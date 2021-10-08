/**
 * This file is meant to contains a bunch of handy dandy typedefs 
 * and simple wrappers for other repetitive tasks that have to be carried 
 * out over multiple files. Ex: Opening files, reading input, etc.
 */

#ifndef __SHELL_UTILS
#define __SHELL_UTILS

typedef char* string;

#define ISPARENT(X) X
#define ISCHILD(X) !X

#define STAT_STATUS 3
#define STAT_PGRPID 5
#define STAT_VMSIZE 23

#define HISTORY_SIZE 20
#define DEFAULT_HIS_OUTPUT 10

#define HISTORY_NAME "~/.ksh_history"

void init();
string get_cwd();
string get_prompt_dir();
void replace_tilda(string *path_adr);
void reverse_replace_tilda(string *path_adr);
void swapstring(string *a, string *b);
int64_t string_to_int(string str);
int min(int a, int b);
int max(int a, int b);
void cleanup();
bool isPOSIXFilechar(char c);

#endif