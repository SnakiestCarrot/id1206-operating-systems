
#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
    int N = atoi(argv[1]);
    
    // get page size in bytes, like the task hints at
    size_t page_size = getpagesize();

    printf("The page size is %ld, N is %d and total allocated memory will be %ld bytes\n", page_size, N, N*page_size);

    // make void pointer array for the allocated pages
    void *allocated_pages[N];

    // allocate N pages of size page_size with malloc
    for(int i = 0; i < N; i++) {
        allocated_pages[i] = malloc(page_size);
    }

    // Print the first memory position as a number to suppress unused warning
    printf("Random memory location: %d\n", allocated_pages);

    return 0;
}
