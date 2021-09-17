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

void init();
string get_cwd();
string get_prompt_dir();
void replace_tilda(string *path_adr);

#endif