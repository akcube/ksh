#ifndef __SHELL_BUILTINS
#define __SHELL_BUILTINS

bool is_builtin(char *name);
int exec_builtin(Command c);
int cd(Command c);

#endif