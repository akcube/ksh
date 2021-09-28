#include "libs.h"
#include "execute.h"

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
			if(c->runInBackground) printf("%d\n", pid);
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
				// Add background process to list of open background processes
				insert_process(pid, c->name, &(KSH.plist.head));
			}
		}
		return 1;
	}
	else{
		return exec_builtin(c);
	}
}