#include "libs.h"
#include "builtins.h"

char *builtins[] = {"cd", "pwd", "echo", "ls", NULL};
int (*jumptable[])(Command c) = {cd, pwd, echo, ls};

/**
 * @brief Check if the command is a builtin command
 */
bool is_builtin(char *name){
	char **builtin = builtins;
	for(;(*builtin)!=NULL; builtin++)
		if(!strcmp(name, *builtin)) return true;
	return false;
}


/**
 * @brief Execute a builtin command
 *
 * @param c MUST be a builtin command, or will sigsev. Check with is_builtin first.
 * @return Returns 0 on successful execution. -1 on failure.
 */
int exec_builtin(Command c){
	char **builtin = builtins;
	int ret = -1;
	for(int id=0;(*builtin)!=NULL; builtin++, id++)
		if(!strcmp(c.name, *builtin)) ret = (*jumptable[id])(c);
	return ret;
}

#define BIT_A (1<<0)
#define BIT_L (1<<1)
#define INCLUDE_HIDDEN(X) (X & BIT_A)
#define LIST_FORMAT(X) (X & BIT_L)
#define IGNORE(X, FLAG) (!INCLUDE_HIDDEN(FLAG) && X[0]=='.')

int ls(Command c){
	// Init command state vars
	uint8_t flags = 0;
	string_vector directories;
	create_vector(&directories, 2);

	// Parse Arguments
	for(int i=1; i<=c.argc; i++){
		// If flag argument
		if(c.argv.arr[i][0] == '-'){
			// Iterate over all flags 
			for(char *ptr = c.argv.arr[i] + 1; *ptr; ptr++){
				// Set appropriate bit per flag / throw error
				switch(*ptr){
					case 'l':
						flags |= BIT_L;
					break;
					case 'a':
						flags |= BIT_A;
					break;
					default:
						throw_error(BAD_FLAGS);
						return -1;
				}
			}
		}
		// If not a flag argument, it must be a directory. Populate dir vector
		else{
			push_back(&directories, c.argv.arr[i]);
		}
	}

	// Iterate over all directories
	for(int i=0; i<directories.size; i++){
		if(IGNORE(directories.arr[i], flags)) continue;

	}

	destroy_vector(&directories);
	return 0;
}

/**
 * @brief Builtin implementation of echo
 * @details Does not treat quotes / escape sequence characters specially. Yet.
 * 
 * @return Returns 0 on success. -1 on failure.
 */
int echo(Command c){
	for(int i=1; i<=c.argc; i++)
		check_error(PRINTF_FAIL, printf("%s ", c.argv.arr[i]), -1);
	check_error(PRINTF_FAIL, printf("\n"), -1);
	return 0;
}

/**
 * @brief Builtin implementation of pwd
 * @return 0 on success. -1 on failure.
 */
int pwd(Command c){
	// pwd should have no arguments
	if(c.argc > 0){
		throw_error(TOO_MANY_ARGS);
		return -1;
	}
	puts(KSH.curdir);
	return 0;
}

/**
 * @brief Builtin implementation of cd
 * @details Considers the dir the shell was started in as home dir
 *
 * @return 0 on success. -1 on failure.
 */
int cd(Command c){
	// cd with more than 1 arg is incorrect usage
	if(c.argc>1){
		throw_error(TOO_MANY_ARGS);
		return -1;
	}

	string newpath;
	if(c.argc==0) newpath = KSH.homedir; // Just cd should cd to home dir
	else {
		replace_tilda(&(c.argv.arr[1])); // Get absolute path 
		newpath = c.argv.arr[1];
	}

	// Handle the `cd -` case. Should switch to prev directory
	if(strlen(newpath)==1 && newpath[0]=='-'){
		chdir(KSH.lastdir);
		swapstring(&KSH.lastdir, &KSH.curdir);
		free(KSH.promptdir);
		KSH.promptdir = get_prompt_dir();
		return 0;
	}

	// Error check if valid path & then make sure it is a directory
	struct stat sb;
	if(check_perror("cd", stat(newpath, &sb), -1)) return -1;
	if(!S_ISDIR(sb.st_mode)){
		puts("cd: Cannot cd to a file. Path must be a directory.");
		return -1;
	}

	// Update global shell state
	free(KSH.lastdir);
	free(KSH.promptdir);
	KSH.lastdir = KSH.curdir;
	chdir(newpath);
	KSH.curdir = get_cwd();
	KSH.promptdir = get_prompt_dir();

	return 0;	
}