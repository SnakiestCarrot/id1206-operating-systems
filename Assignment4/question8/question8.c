
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <string.h>
#include <semaphore.h>
#include <sys/wait.h>

#define SEM_PARENT_WRITTEN "/sem_parent_written"
#define SEM_CHILD_WRITTEN  "/sem_child_written"

#define FILE_NAME "file_to_map.txt"
#define FILE_SIZE 1048576

int main(int argc, char *argv[]) 
{
    // remove unused variable warnings
    (void)argc;
    (void)argv;

    int fd;

    fd = open(FILE_NAME, O_RDWR);
    if (fd == -1) 
    {
        perror("Error opening file, make sure to run generate_file.sh first");
        return 1;
    }

    sem_unlink(SEM_PARENT_WRITTEN);
    sem_unlink(SEM_CHILD_WRITTEN);

    sem_t *sem_p = sem_open(SEM_PARENT_WRITTEN, O_CREAT, 0644, 0);
    sem_t *sem_c = sem_open(SEM_CHILD_WRITTEN,  O_CREAT, 0644, 0);

    if (sem_p == SEM_FAILED || sem_c == SEM_FAILED) {
        perror("Semaphore initialization failed");
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0)
    {
        perror("Fork failed");
        exit(1);
    }

    if(pid == 0) 
    { // Child
        char *child_map = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        char *text_to_write = "01234";
        char *text_to_read = (char *)malloc(6);

        if (child_map == MAP_FAILED)
        {
            perror("child mmap failed");
            exit(1);
        }

        printf("Child process (pid=%d); mmap address: %p\n", getpid(), child_map);

        // write and sync file content
        memcpy(child_map+0, text_to_write, 5);
        msync(child_map+0, 5, MS_SYNC);

        // signal we are done writing
        sem_post(sem_c); 

        // wait for parent to write
        sem_wait(sem_p);

        // read
        memcpy(text_to_read, child_map+4096, 5);
        text_to_read[5] = '\0';
        printf("Child process (pid=%d) read from mmaped_ptr[4096]: %s\n", getpid(), text_to_read);
        exit(0);
    }
    else if(pid > 0)
    { // Parent
        char *parent_map = mmap(NULL, FILE_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        char *text_to_write = "56789";
        char *text_to_read = (char *)malloc(6);

        if (parent_map == MAP_FAILED)
        {
            perror("parent mmap failed");
            exit(1);
        }

        printf("Parent process (pid=%d); mmap address: %p\n", getpid(), parent_map);

        // write and sync file content
        memcpy(parent_map+4096, text_to_write, 5);
        msync(parent_map+4096, 5, MS_SYNC);

        // signal we are done writing
        sem_post(sem_p); 

        // wait for child to write
        sem_wait(sem_c);

        memcpy(text_to_read, parent_map+0, 5);
        text_to_read[5] = '\0';
        printf("Parent process (pid=%d) read from mmaped_ptr[0]: %s\n", getpid(), text_to_read);
        exit(0);
    }

    return 0;
}
