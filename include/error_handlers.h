/**
 * This file contains a list of handy functions which make it
 * easy to throw and handle errors appropriately and from a central
 * location. 
 * Fatal errors can be identified quickly. Fatal errors exit the shell.
 * Custom error messages are stored in c_errlist.
 */
#ifndef __SHELL_ERR_HANDLERS
#define __SHELL_ERR_HANDLERS

#include "libs.h"

#define BAD_MALLOC 0
#define OUT_OF_BOUNDS 1

void throw_fatal_perror(char *errMsg);
int check_fatal_error(char *errMsg, int retval, int success);
int check_error(char *errMsg, int retval, int success);
void throw_fatal_error(int ERROR_CODE);
void* check_bad_alloc(void *mem);

#endif