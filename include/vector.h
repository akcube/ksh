/**
 * This is the code for a string vector implementation in C. It 
 * behaves more like the C++ stack DS than the vector. The goal is 
 * to just implement what is required and not a fully functional
 * vector. 
 */

#ifndef __STRING_VECTOR
#define __STRING_VECTOR

typedef char* string;

typedef struct string_vector{
	string *arr;
	uint32_t size;
	uint32_t table_size;
} string_vector;

void push_back(string_vector*, string);
void pop_back(string_vector*);
string top(string_vector*);
void create_vector(string_vector*, uint32_t n);
void destroy_vector(string_vector*);
void vec_sort(string_vector*, bool);

#define CASE_SENSITIVE_SORT 1
#define CASE_INSENSITIVE_SORT 0

#endif