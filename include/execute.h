#ifndef __SHELL_EXECUTE
#define __SHELL_EXECUTE

int execute(Command *c);
int exec_pipe(Pipe *p);

#define READ_END 0
#define WRITE_END 1

#endif