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

int min(int a, int b) { return (a<b)?a:b; }

/**
 * @brief Swap two strings
 */
void swapstring(string *a, string *b){
    string temp = *a;
    *a = *b;
    *b = temp;
}

/**
 * @brief Converts string to int
 *
 * @return -1 if any on-digit char is encountered. String must be an
 * unsigned int. No errors are thrown on overflow.
 */
int64_t string_to_int(string str){
    uint64_t num = 0;
    for(char *ptr = str; *ptr; ptr++){
        if(*ptr > '9' || *ptr < '0') return -1;
        num = num*10+ (int)(*ptr - '0');
    }
    return num;
}

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
 * @brief Replaces any prefix of the home directory with ~ in given string
 */
void reverse_replace_tilda(string *path_adr){
    string path = *path_adr;
    bool match = true;
    uint32_t len = 0;
    for(char *a=path, *b=KSH.homedir; *a != '\0' && *b != '\0'; a++, b++){
        if(*a != *b) match = false;
        len++;
    }
    if(match && KSH.homedir[len] == '\0'){
        path[0] = '~';
        string dup = check_bad_alloc(strdup(path));
        strcpy(path+1, &dup[len]);
        int overlen = strlen(&dup[len]);
        path[1+overlen] = '\0';
        free(dup);
    }
}

/**
 * @brief Open and read pre-logged history if it exists
 */
void init_history(){
    string hisfile = check_bad_alloc(strdup(HISTORY_NAME));
    replace_tilda(&hisfile);

    FILE *fptr = fopen(hisfile, "rb");
    if(fptr){
        fread(&KSH.history, sizeof(History), 1, fptr);
        fclose(fptr);
    }
    else{
        int fd = open(hisfile, O_RDWR | O_CREAT, 0600);
        close(fd);
        memset(&KSH.history, 0, sizeof(History));
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

    // Initialize history
    init_history();

    // Initialize process list
    init_proclist(&(KSH.plist));

    // Setup signal handlers
    setup_sighandler(SIGCHLD, ksh_sigchld);
    setup_sighandler(SIGINT, ksh_ctrlc);
    setup_sighandler(SIGTSTP, ksh_ctrlz);
}

/**
 * @brief Cleans up all excess resources allocated at init
 * @details Writes history to disk storage and frees up resources
 */
void cleanup(){
    clrscr();

    string hisfile = check_bad_alloc(strdup(HISTORY_NAME));
    replace_tilda(&hisfile);

    FILE *fptr = fopen(hisfile, "rb+");
    if(fptr){
        fwrite(&KSH.history, sizeof(History), 1, fptr);
        fclose(fptr);
    }

    free(hisfile);
    free(KSH.username);
    free(KSH.hostname);
    free(KSH.homedir);
    free(KSH.curdir);
    free(KSH.lastdir);
    free(KSH.promptdir);
    destroy_proclist(&KSH.plist);
}