#include "libs.h"
#include "execute.h"

/**
 * @brief If command requires redirection, setup redirect and save current stdin/out
 * @return 0 on success, -1 on failure
 */
int setup_redirection(Command *c){

	// Required flags for i/o redirection
	int r_flags = O_RDONLY;
	int w_flags = O_WRONLY | O_CREAT | ((c->append)?O_APPEND:O_TRUNC);

	// If i redirect specified setup i redirect
	if(c->infile){
		if(check_perror("KSH", KSH.saved_stdin = dup(STDIN_FILENO), -1)) return -1;
		if(check_perror("KSH", KSH.stdin = open(c->infile, r_flags, 0644), -1)) return -1;
		if(check_perror("KSH", dup2(KSH.stdin, STDIN_FILENO), -1)) return -1;
	}
	// If o redirect specified setup o redirect
	if(c->outfile){
		if(check_perror("KSH", KSH.saved_stdout = dup(STDOUT_FILENO), -1)) return -1;
		if(check_perror("KSH", KSH.stdout = open(c->outfile, w_flags, 0644), -1)) return -1;
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

int exec_pipe(Pipe *p){
	
	int x = 0;
	int pfds[2][2];

	int ifd = dup(STDIN_FILENO);
	int ofd = dup(STDOUT_FILENO);
	int dummy;

	if(check_perror("Pipe", pipe(pfds[x]), -1)) dummy=-1;
	if(check_perror("Pipe", dup2(pfds[x][WRITE_END], STDOUT_FILENO), -1)) dummy=-1;
	execute(p->c);
	if(check_perror("Pipe", close(pfds[x][WRITE_END]), -1)) dummy=-1;

	for(p = p->next, x ^= 1; p->next; p = p->next, x ^= 1){
		if(check_perror("Pipe", pipe(pfds[x]), -1)) dummy=-1;
		if(check_perror("Pipe", dup2(pfds[x^1][READ_END], STDIN_FILENO), -1)) dummy=-1;
		if(check_perror("Pipe", dup2(pfds[x][WRITE_END], STDOUT_FILENO), -1)) dummy=-1;
		execute(p->c);
		if(check_perror("Pipe", close(pfds[x^1][READ_END]), -1)) dummy=-1;
		if(check_perror("Pipe", close(pfds[x][WRITE_END]), -1)) dummy=-1;
	}

	if(check_perror("Pipe", dup2(pfds[x^1][READ_END], STDIN_FILENO), -1)) dummy=-1;
	if(check_perror("Pipe", dup2(ofd, STDOUT_FILENO), -1)) dummy=-1;
	execute(p->c);
	if(check_perror("Pipe", close(pfds[x^1][READ_END]), -1)) dummy=-1;
	if(check_perror("Pipe", dup2(ifd, STDIN_FILENO), -1)) dummy=-1;


	return dummy;
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
	
	if(setup_redirection(c)) return -1;

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
        	// Run process
			// If foreground process
			if(!c->runInBackground){
				int status;
				// Set process group in parent as well to avoid race condition
				setpgid(pid, 0);

				// Ignore TTIN & TTOUT signals while it is in foreground & give it foreground gpid
				signal(SIGTTIN, SIG_IGN);
				signal(SIGTTOU, SIG_IGN);	
				tcsetpgrp(STDIN_FILENO, pid);

				// Wait for termination
				waitpid(pid, &status, WUNTRACED);

				// Set parent back to foreground process gid
				tcsetpgrp(STDIN_FILENO, getpgid(0));	

				// Set TTIN & TTOUT handlers back to default
				signal(SIGTTIN, SIG_DFL);
				signal(SIGTTOU, SIG_DFL);
			}
			else{
				if(c->runInBackground) printf("%d\n", pid);
				// Add background process to list of open background processes
				insert_process(pid, c->name, &(KSH.plist.head));
			}
		}
		status = 1;
	}
	else{
		status = exec_builtin(c);
	}
	cleanup_redirection();
	return status;
}