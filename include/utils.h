/**
 * This file is meant to contains a bunch of handy dandy typedefs 
 * and simple wrappers for other repetitive tasks that have to be carried 
 * out over multiple files. Ex: Opening files, reading input, etc.
 */

#ifndef __SHELL_UTILS
#define __SHELL_UTILS

typedef char* string;

void init();
void prompt();
string get_cwd();

#endif