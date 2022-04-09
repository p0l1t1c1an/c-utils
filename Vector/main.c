
// Simple Testing program

#include <stdio.h>
#include "include/vector.h"

void set_to_index(size_t index, void *data)
{
    int *i = data;
    *i = index;
}

void print(size_t index, void *data)
{
    int *i = data;
    printf("%lu: %d\n", index, *i);
}

int remove_test(size_t index, void *data)
{
    int *i = data;
    return *i == 15 || index == 10;
}

int main(void)
{
    int arr[20] = {0};
    struct vector v = vector_from_array(arr, sizeof(arr)/sizeof(arr[0]), sizeof(arr[0]));
    
    v.vtable->enumerate(&v, set_to_index);
 
    int i = 15;
    v.vtable->push(&v, &i);
    
    printf("\nENUMERATE: \n");
    v.vtable->enumerate(&v, print);

    v.vtable->filter(&v, remove_test);
    
    printf("\nFILTER & ENUMERATE: \n");
    v.vtable->enumerate(&v, print);
    
    v.vtable->dtor(&v);
    return 0;
}

