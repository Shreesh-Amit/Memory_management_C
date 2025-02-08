#define _GNU_SOURCE //Enables MAP_ANONYMOUS Flag since it is not a part of the posix standard
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>  
#include <sys/mman.h>

#define MAGIC_NUMBER 1234567

typedef struct node_t{
    size_t size;
    struct node_t *next;
}node_t;

typedef struct header_t{
    int size;
    int magic_number;
}header_t;

void *heap;
node_t *freelist = NULL;

void print_freelist(void){
    node_t *trv = freelist;
    printf("Freelist:\n");
    while(trv){
        printf("[Address: %p] -> [Size: %lu]\n",trv,trv->size);
        trv=trv->next;
    }
}

node_t *insertionSort(node_t *head,node_t *node) {

    if(head==NULL||head>=node){
        node->next = head;
        return node;
    }

    node_t *trv = head;
    while(trv->next!=NULL && trv->next < node){
        trv=trv->next;
    }
    node->next = trv->next;
    trv->next = node;
 
    return head;
}

void * salloc(size_t size){

        if(size==0) return NULL;

        node_t *previous = NULL;
        node_t *current = freelist;

        size_t request_size = size + sizeof(header_t);
        printf("Required size = %lu\n",request_size);

        while(current!=NULL){
            if(request_size <= current->size){

                // save the fields of current free list node
                size_t available_space = current->size;
                node_t *next_neighbour = current->next;

                // adding the header to the heap
                header_t * ptr = (header_t *)current;
                ptr->size = size;
                ptr->magic_number = MAGIC_NUMBER;

                // the actual pointer to memory location returned by salloc
                ptr = (void *)((char *)ptr + sizeof(header_t));

                // updating the freelist node
                current = (node_t *)((char *)ptr+size);
                current->size = available_space-request_size;
                current->next = next_neighbour;

                if(previous!=NULL) previous->next = current;
                else freelist = current;

                return ptr;
            }
            previous = current;
            current = current->next;
        }

        return NULL;
        
}

void sfree(void* ptr){

    if(ptr==NULL) return;

    // check if pointer passed is correct
    header_t *hptr =((header_t *)ptr - 1);
    if(hptr->magic_number!=MAGIC_NUMBER){
        fprintf(stderr,"sfree(): invalid or double free detected at %p\n",ptr);
        exit(1);
    }

    // making sure the pointer is not doubled freed
    hptr->magic_number = 0;

    // calculate the amount of empty space
    size_t available_space = sizeof(header_t) + hptr->size;

    // will simply delete the available memory
    // [IMPROVE: BY JOINING WITH ADJACENT BLOCKS IF POSSIBLE OR MAKE AS PADDING]
    if(available_space<=sizeof(node_t)) return;

    node_t *node = (node_t *)hptr; 
    node->size = available_space-sizeof(node_t);

    // sorting the freelist for colaescing
    freelist = insertionSort(freelist,node);
    

    node_t *curr = freelist->next;
    node_t *prev = freelist;

    // coalescing
    while(curr!=NULL){
        node_t *offset = (node_t *)((char *)prev + sizeof(node_t) + prev->size );
        if(offset == curr){
            prev->size = prev->size+sizeof(node_t)+curr->size;
            prev->next = curr->next;
            curr       = prev->next;
        }else{
            prev=curr;
            curr=curr->next;
        }
    }
    return;
}


void heap_init(size_t size){
    //creating a heap using a system call mmap
    heap = mmap(NULL, size, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);

    if(heap==MAP_FAILED){
        perror("Memory allocation failed");
        exit(1);
    }    

    // printf("Heap of size %d bytes is allocated at %p\n",HEAP_SIZE,heap);

    // initializing the freelist
    freelist = heap;
    freelist->size = size - sizeof(node_t);

    // no next free block 
    freelist->next = NULL; 

    // printf("Remaining space in heap after first node of freelist:%d\n",freelist->size);
}