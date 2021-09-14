/**
 * This file contains a list of handy functions which make it
 * easy to throw and handle errors appropriately and from a central
 * location. 
 * Fatal errors can be identified quickly. Fatal errors exit the shell.
 * Custom error messages are stored in c_errlist.
 */

#include "libs.h"
#include "error_handlers.h"

const int elist_sz = 3;
char *c_errlist[3] = { 	
						"System out of memory. Malloc failed.", 
						"Out of bounds access on string_vector.",
						"Initialization failed."};

// Fatal errors exit the process
void throw_fatal_perror(char *errMsg){
	perror(errMsg);
	exit(errno);
}

// Throw error if return code does not match success code
int check_fatal_perror(char *errMsg, int retval, int success){
	if(retval!=success){
		throw_fatal_perror(errMsg);
		return 1;
	}
	return 0;
}

// Handle custom errors
void throw_fatal_error(int ERROR_CODE){
	assert(ERROR_CODE >= 0 && ERROR_CODE < elist_sz);
	puts(c_errlist[ERROR_CODE]);
	exit(ERROR_CODE);
}

int check_fatal_error(int ERROR_CODE, int retval, int success){
	if(retval!=success){
		throw_fatal_error(ERROR_CODE);
		return 1;
	}
	return 0;
}

void* check_bad_alloc(void *mem){
	if(mem==NULL)
		throw_fatal_error(BAD_MALLOC);
	return mem;
}
