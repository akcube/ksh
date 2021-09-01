//!No idempotency guards on purpose

#include <stdlib.h>
#include <stdint.h>
#include <assert.h>

#ifndef DATA_TYPE
	#error Requires DATA_TYPE to be defined
#endif

#ifndef __MACRO_CONCAT_UTILS
#define __MACRO_CONCAT_UTILS

#define _CAT2(x,y) x ## y
#define CAT2(x,y) _CAT2(x,y)
#define CAT3(x,y,z) CAT2(x,CAT2(y,z))

#endif

#ifndef VECTOR
	#define VECTOR CAT2(vector_, DATA_TYPE)
	#define __VECRESIZE CAT3(__, DATA_TYPE, __vec__resize)
	#define __VECPUSHBACK CAT3(__, DATA_TYPE, __vec__pushback)
	#define __VECGETSIZE CAT3(__, DATA_TYPE, __vec__size)
	#define __VECGET CAT3(__, DATA_TYPE, __vec__get)
	#define __VECSET CAT3(__, DATA_TYPE, __vec__set)
	#define __VECERASE CAT3(__, DATA_TYPE, __vec__erase)
	#define __VECPOPBACK CAT3(__, DATA_TYPE, __vec__popback)
	#define createVector CAT3(create_, DATA_TYPE, _vector)
	#define destroyVector CAT3(destroy_, DATA_TYPE, _vector)
#endif

typedef struct VECTOR{
	DATA_TYPE* arr;
	uint32_t cur_size;
	uint32_t table_size;

	void (*push_back)(struct VECTOR*, DATA_TYPE);
	uint32_t (*size)(struct VECTOR*);
	DATA_TYPE (*pop_back)(struct VECTOR*);
	DATA_TYPE (*get)(struct VECTOR*, uint32_t);
	void (*set)(struct VECTOR*, DATA_TYPE, uint32_t);
	void (*erase)(struct VECTOR*, uint32_t);
} VECTOR;

void createVector(VECTOR* v, uint32_t n);
void destroyVector(VECTOR* v);

void __VECRESIZE(VECTOR* v, uint32_t tab_size){
    v->arr = (DATA_TYPE*) realloc(v->arr, (tab_size)*(sizeof(DATA_TYPE)));
    v->table_size = tab_size;
}

void __VECPUSHBACK(VECTOR* v, DATA_TYPE data){
    
    if(v->cur_size == v->table_size)
        __VECRESIZE(v, (v->table_size<<1));

    v->arr[v->cur_size++] = data;
}

uint32_t __VECGETSIZE(VECTOR* v){
    return v->cur_size;
}

DATA_TYPE __VECGET(VECTOR* v, uint32_t i){
	
	assert(i>=0 && i<v->cur_size);
    
    return v->arr[i];
}

void __VECSET(VECTOR* v, DATA_TYPE data, uint32_t i){

   	assert(i>=0 && i<v->cur_size);

    v->arr[i] = data;
}

void __VECERASE(VECTOR* v, uint32_t ind){
    
    for(uint32_t i=ind;i<v->cur_size-1;i++)
        v->arr[i] = v->arr[i+1];

    v->cur_size--;

    if(v->cur_size <= (v->table_size)>>2)
        __VECRESIZE(v, v->table_size>>1);
}

DATA_TYPE __VECPOPBACK(VECTOR* v){
    assert(v->cur_size>=0);
    DATA_TYPE retVal = v->arr[v->cur_size-1];
    __VECERASE(v, v->cur_size-1);
    return retVal;
}

void createVector(VECTOR* v, uint32_t n){
    v->arr = (DATA_TYPE*)malloc(sizeof(DATA_TYPE)*n);
    assert(v->arr != NULL);

    v->cur_size = 0;
    v->table_size = n;

    v->size = __VECGETSIZE;
    v->push_back = __VECPUSHBACK;
    v->get = __VECGET;
    v->set = __VECSET;
    v->erase = __VECERASE;
    v->pop_back = __VECPOPBACK;
}

void destroyVector(VECTOR *v){
    free(v->arr);
    v->cur_size = v->table_size = 0;
}

#undef DATA_TYPE
#undef VECTOR