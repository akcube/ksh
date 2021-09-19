/**
 * This file contains a few structs that define the functioning
 * of the shell. Shell is an extern state variable shared across
 * all files. Command will contain all information required to process
 * any single command.
 */

#ifndef __SHELL_INCLUDE
#define __SHELL_INCLUDE

#define MAX_COMMAND_LENGTH 1024

typedef struct history{
	int used;
	char data[20][MAX_COMMAND_LENGTH];
} History;

typedef struct Shell{
	string hostname;
	string username;
	string homedir;
	string curdir;
	string lastdir;
	string promptdir;
	uid_t uid;
	ProcList plist;
	History history;
} Shell;

typedef struct Command{
	string name;
	int argc;
	string_vector argv;
	bool runInBackground;
} Command;

// Declares it and makes it accessible in all files this header is included in
extern Shell KSH;

#endif