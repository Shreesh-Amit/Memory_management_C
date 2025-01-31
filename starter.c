#define _GNU_SOURCE //Enables MAP_ANONYMOUS Flag since it is not a part of the posix standard
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>  //unix standard library for sysconf and _SC_PAGESIZE
#include <sys/mman.h>
#include <string.h>
#include <assert.h>
#define HEAP_SIZE 4096

typedef struct node_t{
    void* addr;
    uint16_t size;
    struct node_t *next;
}node_t;

typedef struct header_t{
    int size;
    int magic_number;
}header_t;

void *heap;
node_t *freelist = NULL;

void print_freelist(){
    node_t *trv = freelist;
    printf("Freelist:\n");
    while(trv){
        printf("Address = %p,Size = %hu\n",trv->addr,trv->size);
        trv=trv->next;
    }
}

void * salloc(uint16_t size){

        if(size==0) return NULL;
        if(freelist==NULL){
            fprintf(stderr,"Heap Exhausted\n");
            exit(1);
        }

        node_t *prev = NULL; // incase a node is needed to be freed
        node_t *trv = freelist;

        uint16_t req_size = size + sizeof(header_t);

        printf("Required size = %hu\n",req_size);
        while(trv!=NULL){
            if(req_size <= trv->size){

                //adding the header to the heap
                header_t * ptr = (header_t *)trv->addr;
                ptr->size = size;
                ptr->magic_number = 1234567;

                //giving the actual pointer to memory location
                ptr = (void *)((char *)ptr + sizeof(header_t));

                if(req_size < trv->size){
                    //type casting addr to char * so that it can size can move it by 1 byte each
                    trv->addr = (char *)trv->addr + req_size;
                    trv->size -= req_size;
                }else{
                    //if size of freespace is equal free the node
                    if(prev!=NULL) prev->next = trv->next;
                    else freelist = trv->next;
                    free(trv);
                }
                return ptr;
            }
            prev=trv;
            trv=trv->next;
        }

        return NULL;
        
}

void sfree(void* ptr){

    if(ptr==NULL) return;

    //check if pointer passed is correct
    header_t *hptr =((header_t *)ptr - 1);
    assert(hptr->magic_number==1234567);

    //making sure the pointer is not doubled frred
    hptr->magic_number = 0;

    //calculate the amount of empty space
    uint16_t empty_space = sizeof(header_t) + hptr->size;

    //updating the freelist
    node_t *node = (node_t *)malloc(sizeof(node_t)); 
    node->addr = hptr;
    node->size = empty_space;
    node->next = freelist;
    freelist = node;

    return;
}


void heap_init(){
    //creating a heap using a system call mmap
    heap = mmap(NULL, HEAP_SIZE, PROT_READ|PROT_WRITE, MAP_PRIVATE|MAP_ANONYMOUS , -1, 0);

    if(heap==MAP_FAILED){
        perror("Memory allocation failed");
        exit(1);
    }    

    printf("Heap of size %d bytes is allocated at %p\n",HEAP_SIZE,heap);

    freelist = malloc(sizeof(node_t));
    freelist->addr = heap;
    freelist->size = HEAP_SIZE;
    freelist->next = NULL;

    printf("Remaining space in heap after first node of freespace:%d\n",freelist->size);
}

int main(){
    heap_init();
    // printf("size of header_t: %zu\n",sizeof(header_t));
    // char cmd[100];
    // uint16_t req;

    // while(1){
    //     printf("Enter ur command salloc,sfree,quit:");
    //     scanf("%s",cmd);
    //     if(!strcmp(cmd,"salloc")){
    //         scanf("%hu",&req);
    //         void *ptr = salloc(req);
    //         if(ptr==NULL){
    //             fprintf(stderr,"Memory Allocation Failed\n");
    //             exit(1);
    //         }

    //     }else if(!strcmp(cmd,"quit")){
    //         printf("Quitting ....\n");
    //         break;
    //     }else{
    //         printf("Wrong Command ....\n");
    //     }
    // }

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