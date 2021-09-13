/**
 * This is the code for a string vector implementation in C. It 
 * behaves more like the C++ stack DS than the vector. The goal is 
 * to just implement what is required and not a fully functional
 * vector. 
 */

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

    v->arr[v->size++] = check_bad_alloc(strdup(data));
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
void createVector(string_vector* v, uint32_t n){
    v->arr = (string*) check_bad_alloc(malloc(sizeof(string)*n));
    v->size = 0;
    v->table_size = n;
}

/**
 * @brief Frees all alloc'd memory and cleans up
 */
void destroyVector(string_vector *v){
    for(int i=0; i<v->size; i++)
        free(v->arr[i]);
    free(v->arr);
    v->size = v->table_size = 0;
}
