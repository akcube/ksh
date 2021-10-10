#include "libs.h"
#include "builtins.h"

char *builtins[] = {"cd", "pwd", "echo", "ls", "repeat", "pinfo", "history", "jobs", NULL};
int (*jumptable[])(Command *c) = {cd, pwd, echo, ls, repeat, pinfo, history, jobs};

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
int exec_builtin(Command *c){
	char **builtin = builtins;
	int ret = -1;
	for(int id=0;(*builtin)!=NULL; builtin++, id++)
		if(!strcmp(c->name, *builtin)) ret = (*jumptable[id])(c);
	return ret;
}

#define JOBS_BIT_R (1<<0)
#define JOBS_BIT_S (1<<1)
#define INCLUDE_RUNNING(X) (X & JOBS_BIT_R)
#define INCLUDE_STOPPED(X) (X & JOBS_BIT_S)
#define IS_STOPPED(X) (X!='R' && X!='S')
#define IS_RUNNING(X) (X=='R' || X=='S')

/**
 * @brief Util function to parse arguments given to 'jobs' command
 * @details Checks for the -r and -s flags and toggles appropriate bits in flags
 * 
 * @return 0 on success, -1 if bad args are encountered
 */
int __jobs_parse_arguments(Command *c, uint8_t *flags){
	uint8_t f = 0;

	// If no flags passed, default is printing all jobs
	if(c->argc == 0){
		*flags = JOBS_BIT_R | JOBS_BIT_S;
		return 0;
	}
	// Iterate through all args
	for(int i=1; i<=c->argc; i++){
		// Argument must be a flag parameter
		if(c->argv.arr[i][0] == '-'){
			// Iterate through flags & toggle flag bits as necessary / handle errors
			for(char *ptr=c->argv.arr[i]+1; *ptr; ptr++){
				switch(*ptr){
					case 'r':
						f |= JOBS_BIT_R; // Include running jobs
					break;
					case 's':
						f |= JOBS_BIT_S; // Include sleeping jobs
					break;
					default:
						throw_error(BAD_PARSE);
						return -1;
				}
			}
		}
		else{
			throw_error(BAD_FLAGS);
			return -1;
		}
	}
	*flags = f;
	return 0;
}

/**
 * @brief Case insensitive comparator function for sorting jobs by name
 */
int jobs_cmp(const void *a, const void *b){
	job *p = (job*) a;
	job *q = (job*) b;
	return strcasecmp(p->name, q->name);
}

/**
 * @brief Prints a list of all running & sleeping processes with job num and pid
 * 
 * @details Accepts flags -r and -s for including running and sleeping processes
 * respectively. Job number is a sequential number for jobs dispatched by the shell
 * and can be used to uniquely identify processes started by the current instaance of 
 * the shell
 */
int jobs(Command *c){
	// Read flag arguments
	uint8_t flags = 0;
	if(__jobs_parse_arguments(c, &flags)==-1) return -1;

	// Allocate enough space for array to hold the jobs and init iterator vars
	uint32_t list_size = KSH.plist.size(&KSH.plist);
	job *jlist = check_bad_alloc(calloc(list_size, sizeof(job)));
	char buf[4096];
	uint32_t jdex = 0;
	int fd = -1;

	// Pointer to head of proc list
	Process *p = KSH.plist.head;
	for(; p; p=p->next){
		// Query the /proc/pid/stat file for information regarding execution state
		sprintf(buf, "/proc/%d/stat", p->id);
		if(check_perror("Jobs", fd = open(buf, O_RDONLY), -1)) continue;
		if(check_perror("Jobs", read(fd, buf, 4096), -1)) continue;
		
		// Iterate through contents to get the STAT_STATUS argument
		char *saveptr, status;
		char *token = strtok_r(buf, " ", &saveptr);
		for(int argno=1; token; token=strtok_r(NULL, " ", &saveptr), argno++)
			if(argno==STAT_STATUS) 
				status = token[0];
		
		// Include in array if flags agree with state
		if((IS_RUNNING(status) && INCLUDE_RUNNING(flags)) || (IS_STOPPED(status) && INCLUDE_STOPPED(flags))){
			jlist[jdex].name = check_bad_alloc(strdup(p->str));
			jlist[jdex].job_num = p->job_num;
			jlist[jdex].pid = p->id;
			jlist[jdex++].status = status;
		}

		// Close file
		check_perror("Jobs", close(fd), -1);
	}

	// Sort list by name
	qsort(jlist, jdex, sizeof(job), jobs_cmp);

	// Print the list
	for(int i=0; i<jdex; i++){
		printf("[%ld] ", jlist[i].job_num);
		printf("%s ", ((IS_STOPPED(jlist[i].status))?"Stopped":"Running"));
		printf("%s ", jlist[i].name);
		printf("[%d]\n", jlist[i].pid);
	}

	// Cleanup
	for(int i=0; i<jdex; i++)
		free(jlist[i].name);
	free(jlist);

	return 0;	
}

/**
 * @brief Display the last x commands entered
 * @details Default value for x = 20. Required 0 < x <= 20
 */
int history(Command *c){

	// History can have at max 1 argument
	if(c->argc > 1){
		throw_error(TOO_MANY_ARGS);
		return -1;
	}

	// We can at max show 20 / how much ever data we have in history at the moment
	int toshow = min(KSH.history.used, 20);
	if(c->argc==1)
		toshow = min(toshow, (int) string_to_int(c->argv.arr[1]));

	// Handle bad args
	if(toshow <= 0){
		printf("Number must be between 1 and 20.\n"); 
		return -1;
	}

	// Print history
	for(int i=toshow-1; i>=0; i--){
		printf("%s\n", KSH.history.data[i]);
	}
	return 0;
}

/**
 * @brief Display process information
 * @details Reads data from /proc/pid/stat and /proc/pid/exe to display
 * status, activity and executeable location of process with given pid
 * 
 * @return -1 on failure. 0 on success.
 */
int pinfo(Command *c){

	// pinfo can have at most one argument
	if(c->argc > 1){
		throw_error(TOO_MANY_ARGS);
		return -1;
	}

	// if no arguments pid is self pid
	int pid = getpid();
	if(c->argc != 0)
		pid = string_to_int(c->argv.arr[1]);
	if(pid==-1){
		throw_error(BAD_ARGS);
		return -1;
	}

	// We will query /proc/pid/stat to get process information
	int reqlen = 1024; // strlen("/proc/x/stat")
	string query = malloc(reqlen);
	sprintf(query, "/proc/%d/stat", pid);

	// Open /proc/pid/stat
	int fd = open(query, O_RDONLY);
	if(fd < 0){
		printf("Program with pid: %d doesn't exist\n", pid);
		return -1;
	}
	// Read data to buf
	string buf = malloc(1024*8);
	if(check_perror("pinfo", read(fd, buf, 1024*8), -1)) return -1;

	// Setup variables to store the data retrieved
	char status;
	int64_t memory_used = 0;
	char status_activity;
	pid_t pgrp_id = 0;

	// Go through every token in data retrieved from proc, save relevant info
	char *saveptr;
	char *token = strtok_r(buf, " ", &saveptr);
	for(int argno=1; token; token=strtok_r(NULL, " ", &saveptr), argno++){
		if(argno==STAT_STATUS) status = token[0];
		if(argno==STAT_PGRPID) pgrp_id = string_to_int(token);
		if(argno==STAT_VMSIZE) memory_used = string_to_int(token);
	}

	// If pgrpid == foreground group id, set foreground process
	status_activity = (pgrp_id==tcgetpgrp(0)) ? '+':'-';

	// Query /proc/exe for executable path. Handle symlink
	sprintf(query, "/proc/%d/exe", pid);
	int readlen = readlink(query, buf, 1024*8);
	buf[readlen] = 0;
	reverse_replace_tilda(&buf);

	// Print relevant info
	printf("pid -- %d\n", pid);
    printf("Process Status -- %c%c\n", status, status_activity);
    printf("memory -- %ldB\n", memory_used);
    printf("Executable Path -- %s\n", buf);

    // Cleanup
	free(buf);
	free(query);

	// Close file
	if(check_perror("pinfo", close(fd), -1)) return -1;
	return 0;
}

/**
 * @brief Repeats the command given to it 'n' times
 * 
 * @return -1 on failure. 0 on success.
 */
int repeat(Command *c){
	// Usage: repeat 'n' command-name args...
	if(c->argc <= 1){
		throw_error(TOO_LESS_ARGS);
		return -1;
	}
	// Convert 'n' to int type
	int64_t n = string_to_int(c->argv.arr[1]);
	// Cannot repeat < 0 times
	if(n <= 0){
		puts("Please provide a valid integer > 0 after repeat");
		return 0;
	}
	
	// Obtain comand to be repeated from args passed to repeat
	Command package;
	init_command(&package, c->argv.arr[2]);
	for(int i=3; i<=c->argc; i++){
		push_back(&(package.argv), c->argv.arr[i]);
		package.argc++;
	}
	if(!is_builtin(package.name)) push_back(&(package.argv), NULL);

	// Execute the command in a loop n times
	for(int i=0; i<n; i++)
		execute(&package);

	// Cleanup
	destroy_command(&package);
	return 0;
}


/**
 * @brief Builtin implementation of echo
 * @details Does not treat quotes / escape sequence characters specially. Yet.
 * 
 * @return Returns 0 on success. -1 on failure.
 */
int echo(Command *c){
	for(int i=1; i<=c->argc; i++)
		check_error(PRINTF_FAIL, printf("%s ", c->argv.arr[i]), -1);
	check_error(PRINTF_FAIL, printf("\n"), -1);
	return 0;
}

/**
 * @brief Builtin implementation of pwd
 * @return 0 on success. -1 on failure.
 */
int pwd(Command *c){
	// pwd should have no arguments
	if(c->argc > 0){
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
int cd(Command *c){
	// cd with more than 1 arg is incorrect usage
	if(c->argc>1){
		throw_error(TOO_MANY_ARGS);
		return -1;
	}

	string newpath;
	if(c->argc==0) newpath = KSH.homedir; // Just cd should cd to home dir
	else {
		// Get absolute path 
		newpath = c->argv.arr[1];
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