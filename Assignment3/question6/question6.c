
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
    // check for correct number of arguments
    if(argc != 2) {
        printf("Usage: %s <N>\n", argv[0]);
        return 1;
    }

    int N = atoi(argv[1]);
    
    // get page size in bytes, like the task hints at
    size_t page_size = getpagesize();

    printf("The page size is %zu bytes, N is %d and total allocated memory will be %zu bytes\n", page_size, N, N*page_size);

    // make void pointer array for the allocated pages
    void *ptr = malloc(N * page_size);

    if (ptr == NULL) {
        perror("Malloc failed");
        return 1;
    }

    printf("Random memory location: %p\n", ptr);

    free(ptr);
    return 0;
}
