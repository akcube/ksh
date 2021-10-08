#include "libs.h"
#include "parsing.h"

/**
 * @brief Initialize all the fields of a Command struct
 * @param command Pointer to the struct we want to initialize
 * @param name Pointer to the name of the command. Note that name is copied
 */
void init_command(Command *command, string name){
    command->argc = 0;
    command->runInBackground = false;
    command->infile = NULL;
    command->outfile = NULL;
    command->valid = true;
    command->append = false;
    create_vector(&(command->argv), 2);
    command->name = check_bad_alloc(strdup(name));
    push_back(&(command->argv), name);
}

/**
 * @brief Destroy and free all the fields of a Command struct
 * @param command Pointer to the struct we want to destroy
 */
void destroy_command(Command *command){
    if(command->name) free(command->name);
    if((command->argv).size) destroy_vector(&(command->argv));
    if(command->infile) free(command->infile);
    if(command->outfile) free(command->outfile);
    command->argc = 0;
    command->valid = false;
    command->runInBackground = false;
    command->append = false;
}

/**
 * @brief Checks for the given delim. If found, returns filepath string and removes
 *        delim + next arg from argstring
 * @details Sets valid to false on parse error
 */
string parseRedirect(Command *command, string argstr, char delim){

    int n = strlen(argstr);
    // Handle redirects first
    int trim_begin = -1;
    string filepath;
    string ret = NULL;
    for(int i=0; i<n; i++){
        // If delim found
        if(argstr[i]==delim){
            trim_begin = i;
            // Trim whitespace if necessary
            while(i<n && (argstr[i]==delim || argstr[i]==' ')) i++;
            // Obtain filepath
            filepath = &argstr[i];
            while(i<n && (isPOSIXFilechar(argstr[i]) || argstr[i]=='~')) i++;
            argstr[i] = '\0';
            if(ret || strlen(filepath) < 1) command->valid = false;
            if(!ret) ret = check_bad_alloc(strdup(filepath));
            // Remove from argstr
            for(int j=trim_begin; j<=i && j<n; j++) argstr[j] = ' ';
        }
    }

    if(ret)
        replace_tilda(&ret);
    return ret;
}

/**
 * @brief Parse all arguments given to a program
 * @details Reads all space separated arguments and pushes them to the
 * character vector in the Command struct
 * 
 * @param command Pointer to the Command struct we're modifying
 * @param argstr string containing space separated arguments 
 */
void parse_args(Command *command, string argstr){

    // Check if redirection is append type
    int n = strlen(argstr);
    for(int i=0; i<n-1; i++)
        if(argstr[i]=='>' && argstr[i+1]=='>') command->append = true;

    // Assign redirection files
    command->infile = parseRedirect(command, argstr, '<');
    command->outfile = parseRedirect(command, argstr, '>');

    // Push args to argv
    char *saveptr;
    char *token = strtok_r(argstr, " \t", &saveptr);
    for(;token!=NULL; token=strtok_r(NULL, " \t", &saveptr)){
        push_back(&(command->argv), token);
        command->argc++;
        replace_tilda(&(command->argv.arr[command->argc]));
    }
    if(!is_builtin(command->name))
        push_back(&(command->argv), NULL);
}

void parsePipe(string cmd){

}

/**
 * @brief Parses the entire line as read from the terminal
 * @details Separates commands by ;, &. Then initializes a Command struct
 * with all parsed data and then calls execute(Command)
 *
 * @param linebuf The entire line read from the terminal
 */
void parse(string linebuf){

    bool empty = true;
    for(char *ptr=linebuf; *ptr; ptr++){
        if(*ptr==' ' || *ptr=='\t' || *ptr=='\n') continue;
        empty = false;
    }

    if(empty) return;
    log_history(linebuf);

    // Create dupl string to easily check what the delim was
    char *dupl = check_bad_alloc(strdup(linebuf));
    
    // If the SIGCHLD handler memsets the linebuf to 0 we lose data required for
    // sequential execution. Make a copy here to prevent that issue.
    char *cpystr = check_bad_alloc(strdup(linebuf));

    char *delim = ";&";
    char *saveptr_p, *saveptr_c;

    // Read a single command, similar to the front of a queue
    char *front = strtok_r(cpystr, delim, &saveptr_p);
    Command command;

    // Iterate and execute all commands in the queue
    for(;front!=NULL; front=strtok_r(NULL, delim, &saveptr_p)){

        // If command is actually a chain of piped commands, handle separately
        bool hasPipe = false;
        for(char *ptr=front; *ptr; ptr++)
            hasPipe |= ((*ptr)=='|');

        if(hasPipe){
            parsePipe(front);
            continue;
        }
        
        // First arg is always the program name
        char *token = strtok_r(front, " \t", &saveptr_c);
        if(!token) continue;

        // Fill in the Command struct with parsed data
        init_command(&command, token);
        token=strtok_r(NULL, delim, &saveptr_c);
        int delim_pos = (int)(saveptr_c-cpystr);
        command.runInBackground = (dupl[delim_pos]=='&');
        if(token) 
            parse_args(&command, token);

        // Execute and cleanup
        execute(&command);
        destroy_command(&command);
    }
    free(dupl);
    free(cpystr);
}
