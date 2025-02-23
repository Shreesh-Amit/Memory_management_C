
#define _GNU_SOURCE //Enables MAP_ANONYMOUS Flag since it is not a part of the posix standard
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>  
#include <stdlib.h>
#include <sys/mman.h>

#define MAGIC_NUMBER 0xBEEF
#define HEAP_SIZE 4096
#define FREELIST_SIZE 13

typedef struct header_t{
    uint16_t size;
    uint16_t magic_number;
}header_t;

void *heap;
// an array of freelist nodes where each index k
// corresponds to free block of size 2^k
// if no free block then NULL
void *array_freelist[13]; 

void heap_init(){
    //creating a heap using a system call mmap
    heap = mmap(NULL, HEAP_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);

    if(heap==MAP_FAILED){
        perror("Memory allocation failed");
        exit(1);
    }    

    // initialize array of freelist
    array_freelist[12] = heap;  //free block of size = 2^12 = 4096 bytes
    for(int i=0;i<=11;i++) array_freelist[i]=NULL;
}

void print_freelist(void){
    printf("Freelist:\n");
    for(int i=12;i>=3;i--){    
        printf("[Size of Free Block:%d] -> [Address: %p]\n",1<<i,array_freelist[i]);
    }
}

void *umalloc(uint16_t bytes){

    uint16_t request_size = bytes + sizeof(header_t);

    if(request_size>4096) return NULL;

    uint16_t order;
    for(int i=FREELIST_SIZE-1;i>=3;i--){
        if((1<<i)>=request_size) order = i;
    }

    if(array_freelist[order]!=NULL){

        void *ptr = array_freelist[order];
        array_freelist[order] = NULL;

        header_t *hptr = (header_t *)ptr;
        hptr->size = (1 << order) - sizeof(header_t);
        hptr->magic_number = MAGIC_NUMBER;

        ptr = (void *)((char *)hptr + sizeof(header_t));
        return ptr;
    }
        
    uint16_t parent_order = 0;
    for(int i=order+1;i<FREELIST_SIZE;i++){
        if(array_freelist[i]!=NULL){
            parent_order = i;
        }
    }

    if(parent_order==0) return NULL;
    
    void *parent = array_freelist[parent_order];
    array_freelist[parent_order]=NULL;

    for(int i=parent_order-1;i>=order;i--){
        void *buddy = (void *)((char *)parent + (1<<(i)));
        array_freelist[i] = buddy;
    }

    // for(int i=parent_order;i!=order;i--){        
    //     void *parent_buddy = (void *)((char *)parent + (1<<(parent_order-1)));
    //     parent_order--;
    //     array_freelist[parent_order] = parent_buddy;
    // }


    header_t *hptr = (header_t *)parent;
    hptr->size = (1 << order) - sizeof(header_t);
    hptr->magic_number = MAGIC_NUMBER;

    void *ptr = (void *)((char *)hptr + sizeof(header_t));
    return ptr;
}

void ufree(void *ptrx){
    if(ptrx==NULL) return;

    header_t *hptr = (header_t *)ptrx-1;
    if(hptr->magic_number!=MAGIC_NUMBER){
        fprintf(stderr,"ufree(): invalid or double free detected at %p\n",ptrx);
        exit(1);
    }

    // avoiding double free 
    hptr->magic_number = 0;

    uint16_t block_size = hptr->size+sizeof(header_t);
    void *ptr = (void *)hptr;

    uint16_t order = 0;
    while(block_size!=1){
        order++;
        block_size=block_size>>1;
    }
    
    for(int i=order;i<FREELIST_SIZE;i++){

        // no buddy present
        if(array_freelist[i]==NULL){
            array_freelist[i] = ptr;
            break;
        }

        // else join with buddy
        array_freelist[i] = NULL;
    }
}

int main(){
    heap_init();
    print_freelist();

    void *ptr = umalloc(1020);
    if(ptr==NULL){
        fprintf(stderr,"Memory Allocation Failed");
        exit(1);
    }

    print_freelist();

    void *ptr1 = umalloc(1020);
    if(ptr1==NULL){
        fprintf(stderr,"Memory Allocation Failed");
        exit(1);
    }

    print_freelist();

    void *ptr2 = umalloc(1020);
    if(ptr2==NULL){
        fprintf(stderr,"Memory Allocation Failed");
        exit(1);
    }

    print_freelist();


    // void *ptr3 = umalloc(4092);
    // if(ptr3==NULL){
    //     fprintf(stderr,"Memory Allocation Failed\n");
    //     exit(1);
    // }

    ufree(ptr1);
    print_freelist();

    ufree(ptr2);
    print_freelist();

    ufree(ptr);
    print_freelist();
    return 0;
}