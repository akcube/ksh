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
#include<pwd.h>
#include<limits.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<time.h>
#include<stdint.h>
#include<dirent.h>
#include<grp.h>

// Self-defined include files
#include "error_handlers.h"
#include "utils.h"
#include "vector.h"
#include "shell.h"
#include "prompt.h"
#include "parsing.h"
#include "execute.h"
#include "builtins.h"
#include "ls.h"

#endif