
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

// Calculates difference in seconds between 2 time spec structs
#define TIMESPEC_DIFF(t1, t2) \
    ((double)(t2.tv_sec - t1.tv_sec) + (double)(t2.tv_nsec - t1.tv_nsec) / 1000000000.0)

#define READ_END 0
#define WRITE_END 1

void generate_random_doubles(double *array, int len) {
    for (int i = 0; i < len; i++) {
        array[i] = (double)rand() / (double)RAND_MAX;
    }
}

double calculate_sum_with_processes(double *array, int len) {
    pid_t child_pid1, child_pid2;

    int pipe_c1[2]; 
    int pipe_c2[2];
    double buffer1[1];
    double buffer2[1];

    pipe(pipe_c1);
    pipe(pipe_c2);
    
    struct timespec start_time, end_time;
    double execution_time;

    clock_gettime(CLOCK_MONOTONIC, &start_time);

    child_pid1 = fork();
    
    if(child_pid1 == 0) { // child 1
        int local_length = len/2;
        double sum = 0.0;

        for(int i=0; i < local_length; i++) {
            sum += array[i];
        }

        printf("Child 1: %f\n", sum);
        write(pipe_c1[WRITE_END], &sum, sizeof(double));
        close(pipe_c1[WRITE_END]);
        exit(0);
    } 
    else if(child_pid1 > 0) { // parent
        child_pid2 = fork();

        if(child_pid2 == 0) { // child 2
            int local_length = len/2;
            
            double sum = 0.0;

            for(int i = local_length; i < len; i++) {
                sum += array[i];
            }

            printf("Child 2: %f\n", sum);
            write(pipe_c2[WRITE_END], &sum, sizeof(double));
            close(pipe_c2[WRITE_END]);
            exit(0);
        } 
        else if(child_pid2 > 0) { // parent again
            close(pipe_c1[WRITE_END]);
            close(pipe_c2[WRITE_END]);

            read(pipe_c1[READ_END], buffer1, sizeof(double));
            read(pipe_c2[READ_END], buffer2, sizeof(double));

            wait(NULL);
            wait(NULL);

            printf("Received %f from child 1\n", buffer1[0]);
            printf("Received %f from child 2\n", buffer2[0]);

            double process_sum = buffer1[0] + buffer2[0];
            printf("Sum calculated with processes:\t%f\n", process_sum);
            
            clock_gettime(CLOCK_MONOTONIC, &end_time);
            execution_time = TIMESPEC_DIFF(start_time, end_time);

            double sum = 0.0;
            for(int i = 0; i < len; i++) {
                sum += array[i];
            }
            printf("Parent control sum:\t\t%f\n", sum);
            printf("Exec time: %f\n", execution_time);
        }
    }
    return 0.0;
}

int main(int argc, char *argv[]) {
    if(argc != 2) {
        char *error_message = "Please provide an argument for the number of elements in the array.\nLike this:\n\t./question8 10\n";
        printf("%s", error_message);
        return 0;
    }

    const int N = atoi(argv[1]);
    printf("Input array size: %d\n", N);

    srand(time(NULL));

    double *random_array = (double *)malloc(N * sizeof(double));

    generate_random_doubles(random_array, N);

    calculate_sum_with_processes(random_array, N);

    free(random_array);
    return 0;
}
