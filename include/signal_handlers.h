#ifndef __SHELL_SIGNAL_HANDLERS
#define __SHELL_SIGNAL_HANDLERS

void setup_sighandler(int SIG, void (*handler)(int, siginfo_t*, void*));
void ksh_sigchld(int SIG, siginfo_t *info, void *);
void ksh_ctrlc(int SIG, siginfo_t *info, void *);
void ksh_ctrlz(int SIG, siginfo_t *info, void *);

#endif