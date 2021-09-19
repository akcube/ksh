#include "libs.h"
#include "signal_handlers.h"

/**
 * @brief Helper function to setup signal handlers using sigaction
 * 
 * @param SIG Signal code for which we're installing the handler
 * @param handler Pointer to sigaction handler function
 */
void setup_sighandler(int SIG, void (*handler)(int, siginfo_t*, void*)){
	struct sigaction sa;
	memset(&sa, 0, sizeof(struct sigaction));	
	sa.sa_sigaction = handler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags =  SA_RESTART | SA_SIGINFO;
	check_fatal_perror("Signal handler", sigaction(SIG, &sa, NULL), -1);

}

/**
 * @brief Handles receiving ctrl-c signals and ignores them
 */
void ksh_ctrlc(int SIG, siginfo_t *info, void *){
    char buf[1024];
    write(1, "\n", strlen("\n"));
    sprintf(buf, "<%s@%s:%s> ", KSH.username, KSH.hostname, KSH.promptdir);
    write(1, buf, strlen(buf));
    getline_pt = 0;
    if(getline_inp)
        memset(getline_inp, 0, MAX_COMMAND_LENGTH);
}

/**
 * @brief Handles receiving ctrl-z signals and ignores them
 */
void ksh_ctrlz(int SIG, siginfo_t *info, void *){
    char buf[1024];
    write(1, "\n", strlen("\n"));
    sprintf(buf, "<%s@%s:%s> ", KSH.username, KSH.hostname, KSH.promptdir);
    write(1, buf, strlen(buf));
    getline_pt = 0;
    if(getline_inp)
        memset(getline_inp, 0, MAX_COMMAND_LENGTH);
}

/**
 * @brief Reaps any background processes spawned by the shell and displays to terminal
 */
void ksh_sigchld(int SIG, siginfo_t *info, void *f){

	// Store data received from waitpid
	int status;
    pid_t c_pid;

    // Output information to terminal only if it was a background process
    bool isBackground = false;
    char buf[1024];

    // Reap all children zombie processes & output info about suspended processes as well
    while ((c_pid = waitpid(-1, &status, WNOHANG | WUNTRACED)) > 0) {

    	// Get name from global linked list
        string process_name = get_process_name(c_pid, &(KSH.plist.head));
        isBackground = true; // Set flag to true

        // Handle all R->S states as expected
        if(WIFEXITED(status)){
        	sprintf(buf, "\n%s with pid %d exited normally\n", process_name, c_pid);
            remove_process(c_pid, &(KSH.plist.head));
        }
        else if (WIFSTOPPED(status)) {
            sprintf(buf, "\n%s with pid %d suspended normally\n", process_name, c_pid);
        } 
        else {
            sprintf(buf, "\n%s with pid %d did not exit normally\n", process_name, c_pid);
            remove_process(c_pid, &(KSH.plist.head));
        }
        write(1, buf, strlen(buf));
    }
    // Output prompt again only if interrupted by background process SIGCHLD
    if(isBackground){
    	sprintf(buf, "<%s@%s:%s> ", KSH.username, KSH.hostname, KSH.promptdir);
    	write(1, buf, strlen(buf));
    }
    getline_pt = 0;
    if(getline_inp)
        memset(getline_inp, 0, MAX_COMMAND_LENGTH);
}