#include "libs.h"
#include "execute.h"

/**
 * @brief If command requires redirection, setup redirect and save current stdin/out
 * @return 0 on success, -1 on failure
 */
int setup_redirection(Command *c){

	// Special case, replay does not require this
	if(!strcmp(c->name, "replay") || !strcmp(c->name, "repeat")) return 0;
	if(!strcmp(c->name, "baywatch") && (c->infile || c->outfile)) return 2;

	// Required flags for i/o redirection
	int r_flags = O_RDONLY;
	int w_flags = O_WRONLY | O_CREAT | ((c->append)?O_APPEND:O_TRUNC);

	// If i redirect specified setup i redirect
	if(c->infile){
		if(check_perror("KSH", KSH.stdin = open(c->infile, r_flags, 0644), -1)) return -1;
		if(check_perror("KSH", KSH.saved_stdin = dup(STDIN_FILENO), -1)) return -1;
		if(check_perror("KSH", dup2(KSH.stdin, STDIN_FILENO), -1)) return -1;
	}
	// If o redirect specified setup o redirect
	if(c->outfile){
		if(check_perror("KSH", KSH.stdout = open(c->outfile, w_flags, 0644), -1)) return -1;
		if(check_perror("KSH", KSH.saved_stdout = dup(STDOUT_FILENO), -1)) return -1;
		if(check_perror("KSH", dup2(KSH.stdout, STDOUT_FILENO), -1)) return -1;	
	}
	return 0;
}

/**
 * @brief Close fds associated with i/o redirect streams and set stdin/stdout back to default
 */
void cleanup_redirection(){
	if(KSH.saved_stdin != -1){
		check_fatal_perror("KSH", dup2(KSH.saved_stdin, STDIN_FILENO), -1);
		check_perror("KSH", close(KSH.stdin), -1);
		check_perror("KSH", close(KSH.saved_stdin), -1);
		KSH.saved_stdin = KSH.stdin = -1;
	}
	if(KSH.saved_stdout != -1){
		check_fatal_perror("KSH", dup2(KSH.saved_stdout, STDOUT_FILENO), -1);
		check_perror("KSH", close(KSH.stdout), -1);
		check_perror("KSH", close(KSH.saved_stdout), -1);
		KSH.saved_stdout = KSH.stdout = -1;
	}
}

/**
 * @brief Executes all commands in a pipe and sets up the fd pipes to one another
 * 
 * @return 0 on success, -1 on failure
 */
int exec_pipe(Pipe *p){
	
	// Init holder vars
	int x = 0;
	int pfds[2][2];

	// Save stdin and stdout for later
	int ifd = dup(STDIN_FILENO);
	int ofd = dup(STDOUT_FILENO);
	int status = 0;

	// Create the first pipe and make stdout refer to pipe write
	if(check_perror("Pipe", pipe(pfds[x]), -1)) status=-1;
	if(check_perror("Pipe", dup2(pfds[x][WRITE_END], STDOUT_FILENO), -1)) status=-1;
	execute(p->c);
	if(check_perror("Pipe", close(pfds[x][WRITE_END]), -1)) status=-1;

	// For every in between command, make it read from previous commands write end
	// and write to its own pipe write end. Close once used.
	for(p = p->next, x ^= 1; p->next; p = p->next, x ^= 1){
		if(check_perror("Pipe", pipe(pfds[x]), -1)) status=-1;
		if(check_perror("Pipe", dup2(pfds[x^1][READ_END], STDIN_FILENO), -1)) status=-1;
		if(check_perror("Pipe", dup2(pfds[x][WRITE_END], STDOUT_FILENO), -1)) status=-1;
		execute(p->c);
		if(check_perror("Pipe", close(pfds[x^1][READ_END]), -1)) status=-1;
		if(check_perror("Pipe", close(pfds[x][WRITE_END]), -1)) status=-1;
	}

	// Last command of the pipe. Read from previous commands write end and write to stdout.
	// If restoring stdin or stdout fails, throw fatal error and exit.
	if(check_perror("Pipe", dup2(pfds[x^1][READ_END], STDIN_FILENO), -1)) status=-1;
	check_fatal_perror("Pipe", dup2(ofd, STDOUT_FILENO), -1);
	execute(p->c);
	if(check_perror("Pipe", close(pfds[x^1][READ_END]), -1)) status=-1;
	check_fatal_perror("Pipe", dup2(ifd, STDIN_FILENO), -1);

	return status;
}

/**
 * @brief Execute a Command
 * @details Handle builtins and other programs differently. If system
 * command then setup process as foreground process and wait till it 
 * finishes. If background process then execute in background and continue
 * running.
 * 
 * @param c Command struct containing all details of command to execute
 * @return 0 if successful. -1 if failure.
 */
int execute(Command *c){
	
	int retvalue;
	if((retvalue = setup_redirection(c))){
		if(retvalue==2)
			throw_error(BAD_ARGS);
		return -1;
	}

	int status = -1;
	// Check if system command
	if(!is_builtin(c->name)){
		pid_t pid = fork();

		if(check_error(FORK_FAIL, pid, -1)) return -1;

		// Create process group for child and execute program
		if(ISCHILD(pid)){
			// Give process it's own group id and restore default signal handlers
			setpgid(0, 0);
			signal(SIGINT, SIG_DFL);
        	signal(SIGTSTP, SIG_DFL);

			execvp(c->name, c->argv.arr);
			cleanup();
			throw_fatal_error(EXEC_FAIL);
		}
		else{
			// Add background process to list of all open processes
			insert_process(pid, c->name, &(KSH.plist.head));

        	// Run process
			// If foreground process
			if(!c->runInBackground){
				int status;
				
				// Move process to foreground
				make_fg_process(pid);
				// Wait for termination
				waitpid(pid, &status, WUNTRACED);

				// If it was suspended, don't remove from proc list
				if(WIFSTOPPED(status)) status = WSTOPSIG(status);
				// If it was terminated, remove from proc list and return appropriate status
				else if(WIFEXITED(status) || WIFSIGNALED(status)){
					remove_process(pid, &(KSH.plist.head));
					status = (WIFEXITED(status)) ? WEXITSTATUS(status) : WTERMSIG(status);
				}
				// Make parent the foreground process again
				make_fg_parent();
			}
			else{
				if(c->runInBackground) printf("%d\n", pid);
			}
		}
	}
	else{
		status = exec_builtin(c);
	}
	cleanup_redirection();
	return status;
}