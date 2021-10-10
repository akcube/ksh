#ifndef __SHELL_BUILTINS
#define __SHELL_BUILTINS

bool is_builtin(char *name);
int exec_builtin(Command *c);
int cd(Command *c);
int pwd(Command *c);
int echo(Command *c);
int repeat(Command *c);
int pinfo(Command *c);
int history(Command *c);
int jobs(Command *c);
int sig(Command *c);
int bg(Command *c);

typedef struct job{
	uint64_t job_num;
	pid_t pid;
	char *name;
	char status;
} job;

#endif