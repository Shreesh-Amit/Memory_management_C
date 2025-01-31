#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

typedef struct node_t{
    int size;
    struct node_t *next;
}node_t;

typedef struct header_t{
    int size;
    int magic_number;
}header_t;

void * umalloc(uint32_t size){

}

void ufree(void* ptr){

}

int main(){
    void * page = malloc(100);
    char cmd[100];
    
}