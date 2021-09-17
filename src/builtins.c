#include "libs.h"
#include "builtins.h"

char *builtins[] = {"cd", "pwd", "ls", NULL};

/**
 * @brief Check if the command is a builtin command
 */
bool is_builtin(char *name){
	char **builtin = builtins;
	for(;(*builtin)!=NULL; builtin++)
		if(!strcmp(name, *builtin)) return true;
	return false;
}