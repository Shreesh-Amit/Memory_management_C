#include <stdio.h>
#include "embedded_allocator.h"

int main(){
    heap_init(100000);

    void *ptr1 = salloc(1000);
    if(ptr1==NULL){
        fprintf(stderr,"Memory Allocation Failed\n");
    }
    print_freelist();

    void *ptr2 = salloc(500);
    if(ptr2==NULL){
        fprintf(stderr,"Memory Allocation Failed\n");
    }
    print_freelist();

    void *ptr3 = salloc(1000);
    if(ptr3==NULL){
        fprintf(stderr,"Memory Allocation Failed\n");
    }
    print_freelist();

    sfree(ptr1); 
    print_freelist();

    sfree(ptr2);
    print_freelist();

    sfree(ptr3);
    print_freelist();

    return 0;
}