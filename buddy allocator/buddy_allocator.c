
#define _GNU_SOURCE //Enables MAP_ANONYMOUS Flag since it is not a part of the posix standard
#include <stdio.h> //remove this library in future
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>

#define MAGIC_NUMBER 0xBEEF
#define HEAP_SIZE 64
#define FREELIST_SIZE 7

typedef struct header_t{
    uint16_t size;
    uint16_t magic_number;
}header_t;

// size of block_t = 8 bytes
typedef struct block_t{
    struct block_t *next;
}block_t;

void *heap;
// an array of freelist nodes where each index k
// corresponds to free block of size 2^k
// if no free block then NULL
block_t *array_freelist[13]; 

void heap_init(){
    //creating a heap using a system call mmap
    heap = mmap(NULL, HEAP_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);

    if(heap==MAP_FAILED){
        perror("Memory allocation failed");
        exit(1);
    }    

    block_t *freelist = (block_t *)heap;
    freelist->next = NULL;

    // initialize array of freelist
    array_freelist[FREELIST_SIZE-1] = freelist;  //free block of size = 2^12 = 4096 bytes
    for(int i=0;i<=FREELIST_SIZE-2;i++) array_freelist[i]=NULL;
}

void print_freelist(void){
    printf("Freelist:\n");
    for(int i=FREELIST_SIZE-1;i>=3;i--){    
        printf("[Size of Free Block:%d] : ",1<<i);
        block_t *trv = array_freelist[i];
        while(trv!=NULL){
            printf("[Address: %p] -> ",trv);
            trv=trv->next;
        }
        printf("NULL\n");
    }
}

void *umalloc(uint16_t bytes){

    uint16_t request_size = bytes + sizeof(header_t);

    if(request_size>HEAP_SIZE) return NULL;

    uint16_t order = 0;
    for(int i=FREELIST_SIZE-1;i>=3;i--){
        if((1<<i)>=request_size) order = i;
    }

    if(order==0) return NULL;

    if(array_freelist[order]!=NULL){

        block_t *ptr = array_freelist[order];
        array_freelist[order] = ptr->next;

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
            break;
        }
    }

    if(parent_order==0) return NULL;
    
    block_t *parent = array_freelist[parent_order];
    array_freelist[parent_order] = parent->next;

    for(int i=parent_order-1;i>=order;i--){
        block_t *buddy = (block_t *)((char *)parent + (1<<(i)));
        buddy->next = array_freelist[i];
        array_freelist[i] = buddy;
    }

    header_t *hptr = (header_t *)parent;
    hptr->size = (1 << order) - sizeof(header_t);
    hptr->magic_number = MAGIC_NUMBER;

    void *ptr = (void *)((char *)hptr + sizeof(header_t));
    return ptr;
}

void ufree(void *ptrx){
    if(ptrx==NULL) return;

    header_t *hptr = ((header_t *)ptrx)-1;
    if(hptr->magic_number!=MAGIC_NUMBER){
        fprintf(stderr,"ufree(): invalid or double free detected at %p\n",ptrx);
        exit(1);
    }

    // avoiding double free 
    hptr->magic_number = 0;

    uint16_t block_size = hptr->size+sizeof(header_t);
    block_t *ptr = (block_t *)hptr;

    uint16_t order = 0;
    while(block_size!=1){
        order++;
        block_size=block_size>>1;
    }
    // for (int i = order; i < FREELIST_SIZE; i++) {
    //     block_t *buddy = (block_t *)((uintptr_t)ptr ^ (1 << i));
    //     block_t **trv = &array_freelist[i]; // Pointer to pointer for easier removal

    //     while (*trv) {
    //         if (*trv == buddy) {  
    //             // Found buddy, remove from list
    //             *trv = buddy->next;

    //             // Merge blocks
    //             ptr = (ptr < buddy) ? ptr : buddy;
    //             goto continue_merging;
    //         }
    //         trv = &((*trv)->next); // Move to next block
    //     }

    //     // If no buddy found, insert the block and exit
    //     ptr->next = array_freelist[i];
    //     array_freelist[i] = ptr;
    //     return;

    //     continue_merging:;
    // }
 
    for(int i=order;i<FREELIST_SIZE;i++){
        block_t *buddy = (block_t *)(((uintptr_t)ptr)^(1<<i));
        block_t *trv = array_freelist[i];

        if(trv==buddy){
            array_freelist[i]=buddy->next;
            ptr = (ptr < buddy)? ptr:buddy;
            continue;
        }

        while(trv!=NULL && trv->next!=buddy){
            trv=trv->next;
        }

        if(trv!=NULL && trv->next==buddy){
            trv->next = buddy->next;
            ptr = (ptr < buddy)? ptr:buddy;
            continue;
        }

        // no buddy present
        ptr->next = array_freelist[i];
        array_freelist[i] = ptr;
        return;
    }
}
