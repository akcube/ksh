/**
 * This file contains commonly used include files that are 
 * used in multiple files across the project. This file exists 
 * merely to reduce the long list of includes that would've otherwise
 * had to be present in multiple files. 
 */

#ifndef __SHELL_LIB_INCL
#define __SHELL_LIB_INCL

// Standard library includes 
#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<assert.h>
#include<string.h>
#include<stdbool.h>
#include<unistd.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<time.h>

// Self-defined include files
#include "error_handlers.h"
#include "utils.h"

#endif