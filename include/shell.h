#ifndef __SHELL_INCLUDE
#define __SHELL_INCLUDE

#include "libs.h"

typedef struct Shell{
	string hostname;
	string username;
	string homedir;
	string curdir;
	string lastdir;
} Shell;

typedef struct Command{
	string name;
	int argc;
	
} Command;

#endif