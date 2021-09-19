#include "libs.h"
#include "parsing.h"

/**
 * @brief Initialize all the fields of a Command struct
 * @param command Pointer to the struct we want to initialize
 * @param name Pointer to the name of the command. Note that name is copied
 */
void init_command(Command *command, char *name){
    command->argc = 0;
    command->runInBackground = false;
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
    command->argc = 0;
    command->runInBackground = false;
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

/**
 * @brief Parses the entire line as read from the terminal
 * @details Separates commands by ;, &. Then initializes a Command struct
 * with all parsed data and then calls execute(Command)
 *
 * @param linebuf The entire line read from the terminal
 */
void parse(char *linebuf){

    // Create dup string to easily check what the delim was
    char *dup = check_bad_alloc(strdup(linebuf));
    char *delim = ";&";
    char *saveptr_p, *saveptr_c;

    // Read a single command, similar to the front of a queue
    char *front = strtok_r(linebuf, delim, &saveptr_p);
    Command command;

    // Iterate and execute all commands in the queue
    for(;front!=NULL; front=strtok_r(NULL, delim, &saveptr_p)){
        
        // First arg is always the program name
        char *token = strtok_r(front, " \t", &saveptr_c);

        if(!token) continue;

        // Fill in the Command struct with parsed data
        init_command(&command, token);
        token=strtok_r(NULL, delim, &saveptr_c);
        int delim_pos = (int)(saveptr_c-linebuf);
        command.runInBackground = (dup[delim_pos]=='&');
        if(token) 
            parse_args(&command, token);

        // Execute and cleanup
        execute(&command);
        destroy_command(&command);
    }
    free(dup);
}
