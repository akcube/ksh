#ifndef __SHELL_ERR_HANDLERS
#define __SHELL_ERR_HANDLERS

#include "libs.h"

void throw_fatal_perror(char *errMsg);
void check_fatal_error(char *errMsg, int retval, int errcode);
void check_error(char *errMsg, int reval, int errcode);
void throw_fatal_error(int ERROR_CODE);

#endif