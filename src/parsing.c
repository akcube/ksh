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

void parse_args(Command *command, char *argstr){
    char *saveptr;
    char *token = strtok_r(argstr, " ", &saveptr);
    for(;token!=NULL; token=strtok_r(NULL, " ", &saveptr)){
        push_back(&(command->argv), token);
        command->argc++;
    }
}

void parse(char *linebuf){

    char *delim = ";&";
    char *saveptr_p, *saveptr_c;
    char *top = strtok_r(linebuf, delim, &saveptr_p);

    Command command;

    for(;top!=NULL; top=strtok_r(NULL, delim, &saveptr_p)){
        char *token = strtok_r(top, " ", &saveptr_c);
        init_command(&command, token);
        token=strtok_r(NULL, delim, &saveptr_c);
        if(token) 
            parse_args(&command, token);
        
        destroy_command(&command);
    }
}
