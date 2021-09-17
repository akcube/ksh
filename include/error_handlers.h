/**
 * This file contains a list of handy functions which make it
 * easy to throw and handle errors appropriately and from a central
 * location. 
 * Fatal errors can be identified quickly. Fatal errors exit the shell.
 * Custom error messages are stored in c_errlist.
 */
#ifndef __SHELL_ERR_HANDLERS
#define __SHELL_ERR_HANDLERS

#define BAD_MALLOC 0
#define OUT_OF_BOUNDS 1
#define INIT_FAILED 2
#define FORK_FAIL 3
#define EXEC_FAIL 4
#define TOO_MANY_ARGS 5
#define PRINTF_FAIL 6

void throw_fatal_perror(char *errMsg);
int check_fatal_perror(char *errMsg, int retval, int success);
int check_error(int ERROR_CODE, int retval, int success);
int check_perror(char *errMsg, int retval, int success);
void throw_fatal_error(int ERROR_CODE);
void throw_error(int ERROR_CODE);
int check_fatal_error(int ERROR_CODE, int retval, int success);
void* check_bad_alloc(void *mem);

#endif