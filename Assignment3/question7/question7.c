#define _GNU_SOURCE 
#include <malloc.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <time.h>

// Huge page size is 2MB
#define HUGE_PAGE_SIZE (2 * 1024 * 1024) 

int main(int argc, char** argv){

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <num_pages>\n", argv[0]);
        fprintf(stderr, "Option: <0> for normal pages (default), <1> for huge pages\n");
        return 1;
    }

    struct timespec start,end;
        
    int num_pages = atoi(argv[1]);
    int page_size = getpagesize ();

    int use_huge_page = 0; // Default to Normal Pages
    if (argc >= 3) {
        use_huge_page = atoi(argv[2]);
    }

    size_t length = num_pages * page_size;
    
    printf("Allocating %d pages of %d bytes \n", num_pages, page_size);

    char *addr;
    
    // @Add the start of Timer here
    clock_gettime(CLOCK_MONOTONIC, &start);

    int flags = MAP_PRIVATE | MAP_ANONYMOUS;

  if (use_huge_page) {
      printf("Using Option: HUGE PAGES\n");
      flags |= MAP_HUGETLB; 
  } else {
      printf("Using Option: NORMAL PAGES\n");
  }

    addr = (char*) mmap(NULL, length, PROT_READ | PROT_WRITE, flags, -1, 0);
    

    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(1);
    }

    //the code below updates the pages
    char c = 'a';
    for(int i=0; i<num_pages; i++){
        addr[i*page_size] = c;
        c ++;
    }

    // @Add the end of Timer here
    clock_gettime(CLOCK_MONOTONIC, &end);
    
    // @Add printout of elapsed time in cycles
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1e9;
    printf("Time elapsed: %.9f seconds\n", elapsed);
    
    
    for(int i=0; (i<num_pages && i<16); i++){
        printf("%c ", addr[i*page_size]);
    }
    printf("\n");
    
    munmap(addr, length);
  
}