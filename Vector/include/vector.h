
#ifndef VECTOR_H
#define VECTOR_H 

#define DEFAULT_CAP 16

#include <stddef.h>

struct vector;
struct vector_vtable;

struct vector 
{
    const struct vector_vtable *vtable;
    size_t element_size, len, cap;
    void *array;
};

struct vector_vtable 
{
    // Safe ways to get data array
    void * (*data)  (struct vector *self);        // Gets array head
    void * (*at)    (struct vector *self, size_t p); // Gets array at position p
    void * (*back)  (struct vector *self);        // Gets array tail

    // Sizing information
    size_t (*size) (struct vector *self); // Gets amount of memory array in use

    // Boolean checks of vector(s)
    int (*empty) (const struct vector *self); // See if array is empty / len = 0
    int (*equals) (const struct vector *self, const struct vector *other); // Checks if to vectors are equal

    // Modify the size of the array
    int (*shrink)           (struct vector *self, size_t n); // Shrink to first n elements (shrinks cap with it)
    int (*shrink_to_fit)    (struct vector *self); // Shrink capacity to match length
    int (*grow)             (struct vector *self, size_t n); // Increase capacity to n elements
    
    // Modify individual elements in array
    int (*assign)   (struct vector *self, const void *data, size_t p); // Copy data to p-th element in vector
    int (*push)     (struct vector *self, const void *data);  // Adds data as element at end of vector
    void (*pop)      (struct vector *self);              // Removes last element in vector
    int (*insert)   (struct vector *self, const void *data, size_t p); // Inserts data at position p in array
    int (*remove)   (struct vector *self, size_t p); // Removes value at position p in array
    
    // Modify multiple elements in array
    int (*push_n)   (struct vector *self, const void *data, size_t n);  // Adds data array as n elements at end of vector
    int (*truncate) (struct vector *self, size_t n); // Limits length of array to n elements 
    int (*insert_n) (struct vector *self, const void *data, size_t p, size_t n); // Inserts array at position p in array
    int (*remove_range)   (struct vector *self, size_t start, size_t stop); // Removes values at positions start to stop
    
    int (*swap)     (struct vector *self, size_t i, size_t j); // Swaps elements i and j in vector
    int (*clear)    (struct vector *self); // Removes all elements in vector

    // Deconstructor
    void (*dtor)     (struct vector *self); // Frees vector
    
    struct vector (*clone)    (struct vector *self); // Clone array

    // Things I like from rust iterators
    void (*for_each)    (struct vector *self, void (*fn) (void *at)); // Preforms fn on all elements of vector
    void (*enumerate)   (struct vector *self, void (*fn) (size_t index, void *at)); // For each with index
    void (*filter)      (struct vector *self, int  (*fn) (size_t index, void *at)); // Removes indexes when fn returns 0 
    
    int (*map)  (struct vector *self, size_t new_size, void (*fn) (size_t index, void *src, void *dst));
    // Transforms vector to elements of new size 
    // fn takes old value in src and sets new value at dst
    
    // Potential other methods 
    // concat, contains, ... 
};


// Constructors
// Allocates and initializes vector struct
struct vector
vector_ctor(size_t e, size_t c);

// Construct vector with default capacity
// Capacity == 16 
struct vector
default_vector_ctor(size_t e); 

// Clone Array

// Clones array values and creates a flexible vector
// Capacity is nearest 2^N w/ min of 16
struct vector
vector_from_array(const void *array, size_t len, size_t element_size);

#endif // VECTOR_H

