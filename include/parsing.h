#ifndef __SHELL_PARSING
#define __SHELL_PARSING

void init_command(Command *command, char *name);
void destroy_command(Command *command);
void parse_args(Command *command, char *argstr);
void parse(char *linebuf);

#endif