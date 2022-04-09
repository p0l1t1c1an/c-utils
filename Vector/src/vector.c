
#include <stdlib.h>
#include <string.h>
#include "include/vector.h"

// Utility program

static size_t 
log2(size_t x) 
{
    size_t result = 0;
    while(x >>= 1)
        ++result;
    return result;
}


// Methods of Vector

// Safe ways to get data array
static void *
data(struct vector *self) 
{
    if (!self->len) return NULL;
    return self->array;
}

static void *
at(struct vector *self, size_t p) 
{
    if (!self->len || p >= self->len) return NULL;
    return data(self) + self->element_size * p;
}

static void *
back(struct vector *self) 
{
    return at(self, self->len-1);
}

// Sizing information
static size_t 
size(struct vector *self) // Gets amount of memory array in use
{   // Not length of array!! 
    return self->cap * self->element_size;
}

// Boolean checks of vector(s)
static int
empty(struct vector *self)  // See if array is empty / len = 0
{
    return self->len == 0;    
}

static int
equals(struct vector *self, struct vector *other) // Checks if to vectors are equal
{
    char *self_array = data(self);
    char *other_array = data(other);

    if(self_array != NULL && other_array != NULL &&
    self->len == other->len && self->element_size == other->element_size)
    {
        size_t i;
        for(i = 0; i < size(self); ++i) // for each byte of array
        {
            if(self_array[i] != other_array[i]) // is each byte equal
                return 0;
        }
        return 1;
    }
    return 0;
}

// Modify the size of the array
static int 
shrink(struct vector *self, size_t n) // Shrink capacity to n elements
{                                      // DANGER: may truncate array 
    if(n < 0){ /* do nothing */ } 
    else if(n == 0)
    {
        if(data(self) != NULL)
            free(self->array);
        
        self->array = NULL;
        self->cap = 0;
        self->len = 0;
    }
    else if(n < self->cap)
    {
        void *new_array = NULL;
        size_t new_size = n * self->element_size;
        if((new_array = realloc(self->array, new_size)) == NULL)
        {
            return 0; // Failed to realloc (array is unchanged)
        }

        self->array = new_array;
        self->cap = n;
        if(n < self->len)
        {
            self->len = n;
        }
    }
    return 1;
}

static int
shrink_to_fit(struct vector *self) // Shrink capacity to match length
{ 
    if(self->cap > self->len)
    {
        return shrink(self, self->len);
    }
    return 1;
}

static int
grow(struct vector *self, size_t n) // Increase capacity to n elements
{
    if(n > self->cap)
    {
        void *new_array = NULL;
        size_t new_size = n * self->element_size;
        if((new_array = realloc(self->array, new_size)) == NULL)
        {
            return 0; // Failed to realloc (array is unchanged)
        }

        self->array = new_array;
        self->cap = n;
    }
    return 1;
}

// Modify individual elements in array
static int 
assign(struct vector *self, const void *data, size_t p) // Copy data to p-th element in vector
{
    if(p < self->len)
    {
        void *dst = at(self, p);
        if(dst == NULL)
            return 0;
        memcpy(dst, data, self->element_size);
        return 1;
    }
    return 0;
}

static int 
push(struct vector *self, const void *data) // Adds data as element at end of vector
{
    if(self->len >= self->cap)
    {
        if(self->cap == 0)
            self->cap = 1;
        if(!grow(self, self->cap * 2))
            return 0;
    }

    self->len += 1;
    return assign(self, data, self->len-1); 
}

static void 
pop(struct vector *self) // Removes last element in vector
{
    if(self->len > 0) self->len -= 1;
}
        
static int 
insert(struct vector *self, const void *data, size_t p) // Inserts data at position p in array
{
    if(p == self->len)
        return push(self, data);
    else if(p < self->len)
    {
        if(self->len >= self->cap)
        {
            if(self->cap == 0)
                self->cap = 1;
            if(!grow(self, self->cap * 2))
                return 0;
        }

        size_t remaining = self->len - p;

        void *start = at(self, p);
        if(start != NULL)
        {
            self->len += 1;
            memmove(start+self->element_size, start, remaining * self->element_size);
            return assign(self, data, p);
        }
    }
    return 0;
}

static int 
remove(struct vector *self, size_t p) // Removes value at position p in array
{
    if(p < self->len)
    {
        if(p == self->len-1)
        {
            pop(self);
            return 1;
        }
        else
        {
            size_t remaining = self->len - p-1;

            void *start = at(self, p);
            if(start != NULL)
            {
                self->len -= 1;
                memmove(start, start+self->element_size, remaining * self->element_size);
                return 1;
            }
        }
    }
    return 0;
}

// Modify multiple elements in array
static int 
push_n(struct vector *self, const void *data, size_t n)  // Adds data array as n elements at end of vector
{
    if(self->len + n > self->cap)
    {
        size_t cap = 1 << (log2((self->len + n)/self->cap) + 1);
        if(!grow(self, self->cap * cap))
            return 0;
    }

    void *start = self->array + self->element_size * self->len;
    memcpy(start, data, self->element_size * n);
    return 1;
}

static int
trunc(struct vector *self, size_t n) // Limits length of array to n elements 
{                                   // Similar to shrink but doesn't change capacity
    if(n < self->len)
    {
        self->len = n;
        return 1;
    }
    return 0;
}

static int 
insert_n(struct vector *self, const void *data, size_t p, size_t n) // Inserts array at position p in array
{
    if(n == 0)
        return 0;
    if(n == 1)
        return insert(self, data, p);
    if(p == self->len)
        return push_n(self, data, n);

    if(p < self->len)
    {
        if(self->len + n >= self->cap)
        {
            size_t cap = 1 << (log2((self->len + n)/self->cap) + 1);
            if(!grow(self, self->cap * cap))
                return 0;
        }

        size_t remaining = self->len - p;
        void *start = at(self, p);

        if(start != NULL)
        {
            self->len += 1;
            memmove(start+self->element_size * n, start, remaining * self->element_size);
            memcpy(start, data, n * self->element_size);
            return 1;
        }
    }
    return 0;
}

static int 
remove_range(struct vector *self, size_t start, size_t stop) // Removes values at positions start to stop (inclusive)
{
    if(start < self->len && stop < self->len)
    {
        if(start > stop)
            return 0;
        if(start == stop)
            return remove(self, start);
        if(stop == self->len-1)
            return trunc(self, start);

        size_t range = stop - start + 1;
        size_t remaining = self->len - start - range;

        void *data = at(self, start);
        if(data != NULL)
        {
            self->len -= range;
            memmove(data, data+range*self->element_size, remaining * self->element_size);
            return 1;
        }
    }
    return 0;
}


static int
swap(struct vector *self, size_t i, size_t j) // Swaps elements i and j in vector
{
    void *buffer = malloc(self->element_size);
    if(buffer != NULL)
    {
        void *first = at(self, i);
        void *second = at(self, j);

        memcpy(buffer, second, self->element_size);
        memcpy(second, first, self->element_size);
        memcpy(first, buffer, self->element_size);

        free(buffer);
        return 1;
    }
    return 0;
}


static int 
clear(struct vector *self) // Removes all elements in vector
{                          // Resets to default capacity
    return shrink(self, 0);
}


// Deconstructor
static void 
dtor(struct vector *self) // Frees vector
{
    clear(self);
}


static struct vector 
clone(struct vector *self) // Clone array
{
    struct vector new = *self;
    new.array = malloc(self->cap * self->element_size);

    if(new.array != NULL)
        memcpy(new.array, self->array, self->cap * self->element_size);
    else
    { // returns empty vector if malloc fails
        new.cap = 0;
        new.len = 0;
    }

    return new;
}

static void 
for_each(struct vector *self, void (*fn) (void *at))
{
    size_t i;
    for(i = 0; i < self->len; ++i)
    {
        fn(at(self, i));
    }
}

static void 
enumerate(struct vector *self, void (*fn) (size_t index, void *at))
{
    size_t i;
    for(i = 0; i < self->len; ++i)
    {
        fn(i, at(self, i));
    }
}

static void 
filter(struct vector *self, int (*fn) (size_t index, void *at))
{
    size_t i;
    for(i = self->len-1; i > 0; --i)
    {
        if(!fn(i, at(self, i)))
            remove(self, i);
    }

    if(!fn(0, at(self, 0)))
        remove(self, 0);
}

static int
map(struct vector *self, size_t new_size, void (*fn) (size_t index, void *src, void *dst))
{
    void *buffer = malloc(new_size * self->cap);
    if(buffer != NULL && self->array != NULL)
    {
        size_t i;
        for(i = 0; i < self->len; ++i)
            fn(i, self->array+self->cap*i, buffer+new_size*i);
       
        free(self->array);
        self->array = buffer;
        self->element_size = new_size;
        return 1;
    }
    return 0;
}

// Static vtable of methods

static const struct vector_vtable v_vtable = 
{
    .data = data,
    .at = at, 
    .back = back,
    .size = size,
    .shrink = shrink,
    .shrink_to_fit = shrink_to_fit,
    .grow = grow,
    .assign = assign,
    .push = push,
    .pop = pop,
    .insert = insert,
    .remove = remove,
    .push_n = push_n,
    .truncate = trunc,
    .insert_n = insert_n,
    .remove_range = remove_range,
    .swap = swap,
    .clear = clear,
    .dtor = dtor,
    .clone = clone,
    .for_each = for_each,
    .enumerate = enumerate,
    .filter = filter,
    .map = map,
};


// Constructors 

struct vector
vector_ctor(size_t e, size_t c) 
{
    struct vector v = 
    {
        .vtable = &v_vtable,
        .element_size = e,
        .cap = c,
        .len = 0,
        .array = malloc(e * c)
    };
    
    if(v.array == NULL)
        v.cap = 0;

    return v;
}

struct vector
default_vector_ctor(size_t e) 
{
    struct vector v =
    {
        .vtable = &v_vtable,
        .element_size = e,
        .cap = 16,
        .len = 0,
        .array = malloc(e*16)
    };
    
    if(v.array == NULL)
        v.cap = 0;

    return v;
}

struct vector
vector_from_array(const void *array, size_t len, size_t e) 
{
    size_t cap = 1 << (log2(len) + 1);
    cap = cap < 16 ? 16 : cap;
    
    struct vector v = 
    {
        .vtable = &v_vtable,
        .element_size = e,
        .cap = 0,
        .len = 0,
        .array = malloc(cap * e)
    };

    if(v.array != NULL)
    {
        v.cap = cap;
        v.len = len;
        memcpy(v.array, array, e*len); 
    }

    return v;
}

