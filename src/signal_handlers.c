#include "libs.h"
#include "signal_handlers.h"

void setup_sighandler(int SIG, void (*handler)(int, siginfo_t*, void*)){
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));	
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags =  SA_RESTART | SA_SIGINFO;
	check_fatal_perror("Signal handler", sigaction(SIG, &sa, NULL), -1);

}

void ksh_sigchld(int SIG, siginfo_t *info, void *){
	int status;
    pid_t c_pid;

    bool isBackground = false;
    while ((c_pid = waitpid(-1, &status, WNOHANG | WUNTRACED | WCONTINUED)) > 0) {
        string process_name = get_process_name(c_pid, &(KSH.plist.head));
        isBackground = true;

        if(WIFEXITED(status)){
            printf("\n%s with pid %d exited normally\n", process_name, c_pid);
            remove_process(c_pid, &(KSH.plist.head));
        }
        else if (WIFSTOPPED(status)) {
            printf("\n%s with pid %d suspended normally\n", process_name, c_pid);
        } 
        else {
            printf("\n%s with pid %d did not exit normally\n", process_name, c_pid);
            remove_process(c_pid, &(KSH.plist.head));
        }
    }
    if(isBackground)
    	printf("<%s@%s:%s> ", KSH.username, KSH.hostname, KSH.promptdir);
    
    fflush(stdout);
}