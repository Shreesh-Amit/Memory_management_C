#include <stdio.h>
#include <unistd.h>  //unix standard library for sysconf and _SC_PAGESIZE
#include <sys/mman.h>
#include <stdlib.h>

int x = 5;

void fn(){
	x = 7;
}

int main(){
	// size_t page_size = sysconf(_SC_PAGESIZE);
	// printf("System Page Size: %zu byte\n", page_size);

	int * ptr = malloc(sizeof(int));
	ptr[0]=1;
	printf("%d\n",ptr[0]);
	free(ptr);
	printf("%d\n",ptr[0]);
	return 0;
}
