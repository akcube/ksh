/**
 * This file contains a list of handy functions which make it
 * easy to throw and handle errors appropriately and from a central
 * location. 
 * Fatal errors can be identified quickly. Fatal errors exit the shell.
 * Custom error messages are stored in c_errlist.
 */

#include "error_handlers.h"

const int elist_sz = 1;
char *c_errlist[1];

// Fatal errors exit the process
void throw_fatal_perror(char *errMsg){
	perror(errMsg);
	exit(errno);
}

// Throw error if return code matches error code
void check_fatal_error(char *errMsg, int retval, int errcode){
	if(retval==errcode)
		throw_fatal_perror(errMsg);
}

void check_error(char *errMsg, int retval, int errcode){
	if(retval==errcode)
		perror(errMsg);
}

// Handle custom errors
void throw_fatal_error(int ERROR_CODE){
	assert(ERROR_CODE >= 0 && ERROR_CODE < elist_sz);
	puts(c_errlist[ERROR_CODE]);
	exit(ERROR_CODE);
}