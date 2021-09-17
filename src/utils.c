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

/**
 * @brief Returns the shell prompt path relative to home
 * @details Attempts to match prefix of cwd with home path. If there is complete
 * match, the home prefix is substituted with '~'
 */
string get_prompt_dir(){
    bool match = true;
    uint32_t len = 0;
    char *promptdir = check_bad_alloc(strdup(KSH.curdir));
    for(char *a=KSH.curdir, *b=KSH.homedir; *a != '\0' && *b != '\0'; a++, b++){
        if(*a != *b) match = false;
        len++;
    }
    if(match && KSH.homedir[len] == '\0'){
        assert(len>=1);
        promptdir[len-1] = '~';
        string prefixed = check_bad_alloc(strdup(&promptdir[len-1]));
        free(promptdir);
        return prefixed;
    }
    else return promptdir;
}

/**
 * @brief Replace the ~ in relative paths to absolute path
 * @details Given the address of the pointer, it does the replacement and reallocs
 * memory as required
 */
void replace_tilda(string *path_adr){
    string path = *path_adr;
    if(path[0]=='~'){
        int MAXLEN = strlen(path)+strlen(KSH.homedir)+1;
        string dup = check_bad_alloc(strdup(path));
        *path_adr = realloc(path, MAXLEN);
        strcpy(*path_adr, KSH.homedir);
        strcat(*path_adr, dup+1);
        free(dup);
    }
}

/**
 * @brief Initializes all global dependencies of the shell
 * @details Clears screen, sets up home directory & sets up history tracking
 */
void init(){
	
	clrscr(); // Clear terminal

    // Fill in all the details of our global shell state variable
    KSH.uid = getuid();
    KSH.username = check_bad_alloc(strdup(getpwuid(KSH.uid)->pw_name));
    KSH.hostname = check_bad_alloc(malloc((HOST_NAME_MAX+1)*sizeof(char)));
    check_fatal_error(INIT_FAILED, gethostname(KSH.hostname, HOST_NAME_MAX+1), -1);
    KSH.homedir = get_cwd();
    KSH.curdir = get_cwd();
    KSH.lastdir = get_cwd();
    KSH.promptdir = get_prompt_dir();
}