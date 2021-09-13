/**
 * This file is meant to contains a bunch of handy dandy typedefs 
 * and simple wrappers for other repetitive tasks that have to be carried 
 * out over multiple files. Ex: Opening files, reading input, etc.
 */

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

/**
 * @brief Initializes all global dependencies of the shell
 * @details Clears screen, sets up home directory & sets up history tracking
 */
void init(){
	
	clrscr(); 

}