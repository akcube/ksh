#include "libs.h"
#include "builtins.h"

char *builtins[] = {"cd", "pwd", "ls", NULL};
int (*jumptable[])(Command c) = {cd};

/**
 * @brief Check if the command is a builtin command
 */
bool is_builtin(char *name){
	char **builtin = builtins;
	for(;(*builtin)!=NULL; builtin++)
		if(!strcmp(name, *builtin)) return true;
	return false;
}

int exec_builtin(Command c){
	char **builtin = builtins;
	int ret = -1;
	for(int id=0;(*builtin)!=NULL; builtin++, id++)
		if(!strcmp(c.name, *builtin)) ret = (*jumptable[id])(c);
	return ret;
}

int cd(Command c){
	if(c.argc>1){
		throw_error(TOO_MANY_ARGS);
		return -1;
	}
	string newpath;
	if(c.argc==0) newpath = KSH.homedir;
	else {
		replace_tilda(&(c.argv.arr[1]));
		newpath = c.argv.arr[1];
	}

	struct stat sb;
	if(check_perror("cd", stat(newpath, &sb), -1)) return -1;
	if(!S_ISDIR(sb.st_mode)){
		puts("cd: Cannot cd to a file. Path must be a directory.");
		return -1;
	}

	free(KSH.lastdir);
	free(KSH.promptdir);
	KSH.lastdir = KSH.curdir;
	chdir(newpath);
	KSH.curdir = get_cwd();
	KSH.promptdir = get_prompt_dir();

	return 0;	
}