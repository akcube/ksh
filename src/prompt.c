#include "libs.h"
#include "prompt.h"

void parse(char *linebuf){

	char *delim = ";&";
	char *saveptr_p, *saveptr_c;
	char *top = strtok_r(linebuf, delim, &saveptr_p);

	Command command;

	for(;top!=NULL; top=strtok_r(NULL, delim, &saveptr_p)){
		
		init_command(&command);
		char *token = strtok_r(command, " ", &saveptr_c);

		for(;token!=NULL; token=strtok_r(NULL, " ", &saveptr_c)){

		}
	}

}

int prompt(){
	// Setup vars
	string linebuf = NULL;
	size_t bufsize = 0;
	size_t comm_len = 0;

	// Display prompt
    printf("<%s@%s:%s> ", KSH.username, KSH.hostname, KSH.promptdir);

    // Read user command
    comm_len = getline(&linebuf, &bufsize, stdin);
    check_fatal_perror("Prompt", comm_len, -1);
    linebuf[comm_len-1] = '\0'; // Remove the trailing '\n'

    #ifdef DEBUG
    	if(!strcmp("exit", linebuf)) return 0;
    #endif

    if(comm_len > 1) parse(linebuf);

    free(linebuf);
    return 1;
}	