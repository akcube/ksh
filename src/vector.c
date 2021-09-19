/**
 * This is the code for a string vector implementation in C. It 
 * behaves more like the C++ stack DS than the vector. The goal is 
 * to just implement what is required and not a fully functional
 * vector. 
 */

#include "libs.h"
#include "vector.h"

/**
 * @brief Resizes vector size as required
 */
void vec_resize(string_vector* v, uint32_t tab_size){
    v->arr = (string*) check_bad_alloc(realloc(v->arr, (tab_size)*(sizeof(string))));
    v->table_size = tab_size;
}

/**
 * @brief Pushes a new string into the vector
 * @details Complexity: amortized O(1) per addition. Linear in 
 * the size of the string for strdup however.
 */
void push_back(string_vector* v, string data){
    if(v->size == v->table_size)
        vec_resize(v, (v->table_size<<1));
    if(data)
        v->arr[v->size++] = check_bad_alloc(strdup(data));
    else
        v->arr[v->size++] = NULL;
}

/**
 * @brief Returns a pointer to the last added element
 */
string top(string_vector* v){
    if(v->size <= 0) throw_fatal_error(OUT_OF_BOUNDS);
    return v->arr[v->size-1];
}

/**
 * @brief Erases the last element in the vector
 */
void pop_back(string_vector* v){
    if(v->size <= 0) throw_fatal_error(OUT_OF_BOUNDS);
    
    v->size--;
    free(v->arr[v->size]);

    if(v->size <= (v->table_size)>>2)
        vec_resize(v, v->table_size>>1);
}

/**
 * @brief Creates the vector and initializes array + state vars
 */
void create_vector(string_vector* v, uint32_t n){
    v->arr = (string*) check_bad_alloc(calloc(n, sizeof(string)));
    v->size = 0;
    v->table_size = n;
}

/**
 * @brief Frees all alloc'd memory and cleans up
 */
void destroy_vector(string_vector *v){
    for(int i=0; i<v->size; i++)
        free(v->arr[i]);
    free(v->arr);
    v->size = v->table_size = 0;
}


/**
 * @brief wrapper around strcasecmp
 */
int case_insensitive_cmpfunc(const void *a, const void *b){
    return strcasecmp(*(const char **)a, *(const char **)b);
}

/**
 * @brief wrapper around strcmp
 */
int case_sensitive_cmpfunc(const void *a, const void *b){
    return strcmp(*(const char **)a, *(const char **)b);
}

/**
 * @brief Sort a string of vectors
 * 
 * @param v Pointer to the vector to be sorted
 * @param casesens Boolean flag for whether the sort should consider case or not
 */
void vec_sort(string_vector *v, bool casesens){
    if(!casesens)
        qsort(v->arr, v->size, sizeof(string), case_insensitive_cmpfunc);
    else
        qsort(v->arr, v->size, sizeof(string), case_sensitive_cmpfunc);
}
