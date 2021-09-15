/**
 * This file is meant to contains a bunch of handy dandy typedefs 
 * and simple wrappers for other repetitive tasks that have to be carried 
 * out over multiple files. Ex: Opening files, reading input, etc.
 */
#include "libs.h"
#include "utils.h"

/**
 * @brief Somewhat portable clrscr()
 * @details uses conio.h's clrscr() on windows machines and the ANSI POSIX
 * demanding escape sequence on other machines. 
 */
#ifdef _WIN32
#include <conio.h>
#else
#include <stdio.h>
#define clrscr() printf("\e[1;1H\e[2J")
#endif

Shell KSH;

/**
 * @brief Returns a pointer to a string containing cwd
 * @return String: cwd. Caller must free.
 */
string get_cwd(){
    string cwd = check_bad_alloc(malloc(PATH_MAX*sizeof(char)));
    check_bad_alloc(getcwd(cwd, PATH_MAX));
    return cwd;
}

string get_prompt_dir(){
    bool match = true;
    uint32_t len = 0;
    char *promptdir = check_bad_alloc(strdup(KSH.curdir));
    for(char *a=KSH.curdir, *b=KSH.homedir; *a != '\0' && *b != '\0'; a++, b++){
        if(*a != *b) match = false;
        len++;
    }
    if(match && KSH.curdir[len] == '\0'){
        promptdir[len-1] = '~';
        string prefixed = check_bad_alloc(strdup(&promptdir[len-1]));
        free(promptdir);
        return prefixed;
    }
    else return promptdir;
}

/**
 * @brief Initializes all global dependencies of the shell
 * @details Clears screen, sets up home directory & sets up history tracking
 */
void init(){
	
	clrscr(); 

    KSH.uid = getuid();
    KSH.username = check_bad_alloc(strdup(getpwuid(KSH.uid)->pw_name));
    KSH.hostname = check_bad_alloc(malloc((HOST_NAME_MAX+1)*sizeof(char)));
    check_fatal_error(INIT_FAILED, gethostname(KSH.hostname, HOST_NAME_MAX+1), 0);
    KSH.homedir = get_cwd();
    KSH.curdir = get_cwd();
    KSH.lastdir = get_cwd();
    KSH.promptdir = get_prompt_dir();
}

void prompt(){
    printf("<%s@%s:%s>", KSH.username, KSH.hostname, KSH.promptdir);
}