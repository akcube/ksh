#include "libs.h"
#include "builtins.h"

char *builtins[] = {"cd", "pwd", "echo", "ls", "repeat", "pinfo", "history", 
					"jobs", "sig", "bg", "fg", "replay", "baywatch", NULL};
int (*jumptable[])(Command *c) = {cd, pwd, echo, ls, repeat, pinfo, history, jobs, sig, bg, fg, replay, baywatch};


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

/**
 * @brief Prints the size of the memory which is dirty.
 * @details Reads information from the `Dirty:` column in /proc/meminfo. 
 * Infinite loop sleeping in intervals of t.
 */
void *bw_dirty(void *interval){
	int t = *((int*)interval);
	char *buf = NULL;
	size_t buf_size = 0;
	while(1){
		FILE *fptr = fopen("/proc/meminfo", "r");

		while(getline(&buf, &buf_size, fptr)){
			if(!strncmp(buf, "Dirty:", 6)){
				printf("%s", &buf[6]);
				break;
			}
		}
		fclose(fptr);
		sleep(t);
	}
}

/**
 * @brief Prints the number of times the CPU(s) has(ve) been interrupted by the 
 * keyboardcontroller (i8042 with IRQ 1).
 * @details Reads 1st line of info in /proc/interrupts. Infinite loop sleeping
 * in intervals of t.
 */
void *bw_interrupt(void *interval){
	int t = *((int*)interval);
	char *buf = NULL;
	size_t buf_size = 0;
	FILE *fptr = fopen("/proc/interrupts", "r");
	getline(&buf, &buf_size, fptr);
	printf(buf);
	fclose(fptr);
	while(1){
		FILE *fptr = fopen("/proc/interrupts", "r");
		bool found = false;
		int n;
		while(!found){
			getline(&buf, &buf_size, fptr);
			n = strlen(buf);
			for(int i=0; i<n-2; i++){
				if(!strncmp(&buf[i], "1:", 2)){
					buf[i] = buf[i+1] = ' ';
					found = true;
					break;
				}
			}
		}

		for(int i=0; i<n; i++){
			if(isspace(buf[i]) || isdigit(buf[i]))
				printf("%c", buf[i]);
			else break;
		}
		printf("\n");

		fclose(fptr);
		sleep(t);
	}
	return 0;
}

/**
 * @brief Prints the pid of the process that was most recently created on the system
 * @details Reads info from the 5th argument in /proc/loadavg. Infinite loop sleeping
 * in intervals of t.
 */
void *bw_newborn(void *interval){ 
	int t = *((int*)interval);
	char buf[4096];

	while(1){
		int fd = open("/proc/loadavg", O_RDONLY);
		read(fd, &buf, 4095);
		char *saveptr, *token;

		token = strtok_r(buf, " ", &saveptr);
		for(int i=1; i<5; i++)
			token = strtok_r(NULL, " ", &saveptr);
		printf(token);
		close(fd);
		sleep(t);
	}
}

#define BAYWATCH_DIRTY 0
#define BAYWATCH_INTERRUPT 1
#define BAYWATCH_NEWBORN 2
void* (*baywatch_jt[])(void *) = {bw_dirty, bw_interrupt, bw_newborn};

/**
 * @brief Similar to the watch command. Will display last ran thread, dirty memory or
 * interrupts from i8042 with IRQ1 as specified by the flag at intervals of `n` seconds
 * until the key `q` is pressed 
 */
int baywatch(Command *c){

	// Function must have 3 arguments
	if(c->argc != 3){
		throw_error(BAD_ARGS); return -1;
	}

	// Parse possible locations of `-n` flag
	int ndex = -1;
	if(!strncmp(c->argv.arr[1], "-n", 2)) ndex = 1;
	else if(!strncmp(c->argv.arr[2], "-n", 2)) ndex = 2;

	if(ndex==-1){
		throw_error(BAD_ARGS); return -1;
	}

	// Get interval time
	int interval = string_to_int(c->argv.arr[ndex+1]);
	if(interval==-1){
		throw_error(BAD_ARGS); return -1;
	}

	// Parse command type
	string type = (ndex==1)?c->argv.arr[3]:c->argv.arr[1];

	int COMMAND = -1;
	if(!strcmp(type, "dirty")) COMMAND = BAYWATCH_DIRTY;
	else if(!strcmp(type, "interrupt")) COMMAND = BAYWATCH_INTERRUPT;
	else if(!strcmp(type, "newborn")) COMMAND = BAYWATCH_NEWBORN;

	if(COMMAND==-1) {
		puts("Invalid command. The options available to you are [dirty, newborn, interrupt].");
		return -1;
	}

	// Create a new thread where the watcher will output contents 
	pthread_t newthread;
	pthread_create(&newthread, NULL, baywatch_jt[COMMAND], &interval);

	// Enable raw mode so we can setup a listener for the `q` key
	enableRawMode();
	char ch;
	// Check for `q` press
	while(read(STDIN_FILENO, &ch, 1)==1){
		if(ch=='q'){
			pthread_cancel(newthread);
			break;
		}
	}
	// Set terminal back to normal
	disableRawMode();
	return 0;
}

#define INTERVAL_BIT (1<<0)
#define PERIOD_BIT (1<<1)
#define COMMAND_BIT (1<<2)
#define ILEN strlen("-interval ")
#define PLEN strlen("-period ")
#define CLEN strlen("-command ")
#define REPLAY_PARSE_SUCCESS(X) (X==7)

/**
 * @brief Util function for replay parser to parse commands.
 * @return 0 on success, -1 on failure.
 */
int __parse_replay_util(string cmd, int pos, int n, int LEN){
	
	int num_end, j, ret;
	// Erase -<flag> from command
	for(j=pos; j<pos+LEN; j++) cmd[j] = ' ';

	// Find the next argument
	while(j < n && cmd[j]==' ') j++;
	if(j==n) return -1; // No argument after -<flag>
	
	// Isolate the next argument and parse it to an int
	num_end = j;
	while(num_end < n && cmd[num_end] != ' ') num_end++;
	cmd[num_end] = '\0';
	ret = string_to_int(&cmd[j]);

	// Erase the number from the string
	for(;j<n && j<=num_end; j++) cmd[j] = ' ';

	return ret;
}

/**
 * @brief Parses the arguments given to replay. Stores interval & period in 
 * pointers passed to the two integers
 * 
 * @return 0 on success, -1 on failure.
 */
int __parse_replay_args(string cmd, int *interval, int *period){
	
	// Holder vars
	int n = strlen(cmd);
	int status = 0;

	// If we see the two flags, obtain integer values and erase from cmd string
	for(int i=0; i<n; i++){
		if(cmd[i] == '-'){
			if(!strncmp(&cmd[i], "-interval ", ILEN)){
				status |= INTERVAL_BIT; 
				*interval = __parse_replay_util(cmd, i, n, ILEN);
				if(*interval==-1) return -1;
			}
			else if(!strncmp(&cmd[i], "-period ", PLEN)){
				status |= PERIOD_BIT;
				*period = __parse_replay_util(cmd, i, n, PLEN);
				if(*period==-1) return -1;
			}
		}
	}

	// Check if -command flag is found. It must be at the beginning after removal of 
	// other flags. If found remove from cmd string.
	for(int i=0; i<n; i++){
		if(cmd[i]==' ') continue;
		if(!strncmp(&cmd[i], "-command ", CLEN)){
			status |= COMMAND_BIT;
			for(int j=i; j<i+CLEN; j++) 
				cmd[j] = ' ';
			break;
		}
		else return -1;
	}

	// If all three flags were found, success
	if(status==7) return 0;
	else return -1;
}

/**
 * @brief Executes a particular command in fixed time interval for a certain period.
 * @details Best explained with an example. `replay -command echo "hi" -interval 3 -period 6`
 * This command will execute echo "hi" command after every 3 seconds until 6 seconds are 
 * elapsed. In this example, echo "hi" command will be executed 2 times, once after 3 seconds 
 * and then after 6 seconds.
 *
 * @return 0 on success. -1 on failure.
 */
int replay(Command *c){

	// Usage: replay -command <command> -interval <time period> -period <time>
	// Requires: min 6 args
	if(c->argc < 6){
		throw_error(TOO_LESS_ARGS); return -1;
	}

	// For this command specifically, it is easier to parse as a complete string
	// rather than as individual arguments. So we will re-construct the string from args.

	int req_len = 0;
	for(int i=1; i<=c->argc; i++)
		req_len += strlen(c->argv.arr[i]) + 1; // +1 for whitespace
	req_len++; // Null char

	// Reconstruct string
	string buf = malloc(sizeof(char)*req_len + 1024);
	strcpy(buf, c->argv.arr[1]);
	for(int i=2; i<=c->argc; i++){
		strcat(buf, " ");
		strcat(buf, c->argv.arr[i]);
	}

	// Parse arguments
	int interval, period;
	if(__parse_replay_args(buf, &interval, &period) == -1){
		throw_error(BAD_PARSE); return -1;
	}

	// I/O redirection for command will get stored in c. Append to the command we will run instead.
	if(c->infile){
		strcat(buf, " < ");
		strcat(buf, c->infile);
	}
	if(c->outfile){
		if(c->append) strcat(buf, " >> ");
		else strcat(buf, " > ");
		strcat(buf, c->outfile);
	}

	// Repeat the command
	for(int t=interval; t<=period; t+=interval){
		sleep(interval);
		parse(buf);
	}

	// Cleanup
	free(buf);
	return 0;
}

/**
 * @brief Brings a running/suspended background process to the foreground and 
 * changes state to running.
 * 
 * @return Suspension / exit code returned by process. -1 on parse failure.
 */
int fg(Command *c){
	// Bad args if command wasn't given exactly one argument.
	// Usage `fg <job_number>`
	if(c->argc != 1){
		throw_error(TOO_LESS_ARGS);
		return -1;
	}

	// Parse job number to int from args
	int64_t job_num = string_to_int(c->argv.arr[1]);
	if(job_num==-1){
		throw_error(BAD_ARGS);
		return -1;
	}

	// Get process id from the given job number
	pid_t pid = get_process_id(job_num, &(KSH.plist.head));
	if(pid == -1){
		printf("Process with job number %ld does not exist.\n", job_num);
		return -1;
	}


	int status; // Holder var	
	// Moves process to foreground
	make_fg_process(pid);
	
	// Send SIGCONT to the process
	if(check_perror("sig", kill(pid, SIGCONT), -1)) return -1;

	// Wait for process to stop / terminate
	waitpid(pid, &status, WUNTRACED);

	// If it was suspended, don't remove from proc list
	if(WIFSTOPPED(status)) status = WSTOPSIG(status);
	// If it was terminated, remove from proc list and return appropriate status
	else if(WIFEXITED(status) || WIFSIGNALED(status)){
		remove_process(pid, &(KSH.plist.head));
		status = (WIFEXITED(status)) ? WEXITSTATUS(status) : WTERMSIG(status);
	}

	// Set parent back to foreground process
	make_fg_parent();
	return status;
}

/**
 * @brief Changes state of a stopped background process to running in the background.
 * @details Usage: `bg <job_num>`. Job number is the sequential unique number allotted to 
 * a process executed by the shell. Can be obtained by calling `jobs`.
 * 
 * @return 0 on success. -1 on failure.
 */
int bg(Command *c){
	// Bad args if command wasn't given exactly one argument.
	// Usage `bg <job_number>`
	if(c->argc != 1){
		throw_error(BAD_ARGS); // Handle errors
		return -1;
	}

	// Parse job number to int from args
	int64_t job_num = string_to_int(c->argv.arr[1]);
	if(job_num==-1){
		throw_error(BAD_ARGS); // Handle errors
		return -1;
	}

	// Get process id from the given job number
	pid_t pid = get_process_id(job_num, &(KSH.plist.head));
	if(pid == -1){
		printf("Process with job number %ld does not exist.\n", job_num); // Handle errors
		return -1;
	}

	// Send SIGCONT to the process
	if(check_perror("sig", kill(pid, SIGCONT), -1)) return -1;
	return 0;
}

/**
 * @brief Sends signal to process by job number
 * @details Job number is the sequential unique number allotted to a process executed
 * by the shell. Can be obtained by calling `jobs`. Command is of type
 * `sig <job_number> <signal_number>`. Send signal `signal_number` to process
 * 
 * @return 0 on success. -1 on failure.
 */
int sig(Command *c){

	// If command doesn't have exactly 2 arguments then we have bad args
	// Usage `sig <job_number> <signal_number>`
	if(c->argc!=2){
		throw_error(BAD_ARGS);
		return -1;
	}

	// Get job number and signal number from args
	int64_t job_num = string_to_int(c->argv.arr[1]);
	int sig_num = string_to_int(c->argv.arr[2]);
	// Handle errors
	if(job_num == -1 || sig_num == -1){
		throw_error(BAD_ARGS);
		return -1;
	}

	// Gets pid of process
	pid_t pid = get_process_id(job_num, &(KSH.plist.head));
	if(pid == -1){
		printf("Process with job number %ld does not exist.\n", job_num); // Handle errors
		return -1;
	}

	// Send signal to process
	if(check_perror("sig", kill(pid, sig_num), -1)) return -1;
	return 0;
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
		for(int argno=1; token; token=strtok_r(NULL, " ", &saveptr), argno++){
			if(argno==STAT_STATUS){
				status = token[0];
				break;
			}
		}
		
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
		if(argno==STAT_VMSIZE) { memory_used = string_to_int(token); break; }
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
	if(c->infile)
		package.infile = check_bad_alloc(strdup(c->infile));
	if(c->outfile)
		package.outfile = check_bad_alloc(strdup(c->outfile));
	package.append = c->append;

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