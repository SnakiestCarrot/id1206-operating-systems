#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define BILLION 1000000000.0

int num_threads;
int array_size = 1000000;
void *thread_func(); /* the thread function */
float RNG();
float *randomArray;
double *local_sums;

int main(int argc, char *argv[])
{
    // seeding the RNG
    srand(time(NULL));

    num_threads = atoi(argv[1]);
    local_sums = (double *) malloc(num_threads * sizeof(double));
    int *thread_ids = (int *) malloc(num_threads * sizeof(int));

    /* Initialize an array of random values */ 
    randomArray = (float *) malloc(array_size * sizeof(float));
    for (int i = 0; i < array_size; i++)
        randomArray[i] = RNG();

    struct timespec start, end;
    struct timespec start_serial, end_serial;
    /* Perform Serial Sum */
    double  sum_serial = 0.0;
    double time_serial = 0.0;

    //Timer Begin
    clock_gettime(CLOCK_MONOTONIC, &start_serial);

    for (int i = 0; i < array_size; i++)
        sum_serial += (double) randomArray[i];
    //Timer End
    clock_gettime(CLOCK_MONOTONIC, &end_serial);

    time_serial = (end_serial.tv_sec - start_serial.tv_sec) + (end_serial.tv_nsec - start_serial.tv_nsec) / BILLION;
    printf("Serial Sum = %.4f, time = %.3f \n", sum_serial, time_serial);

    /* Create a pool of num_threads workers and keep them in workers */ 
    pthread_t *workers;

    workers = (pthread_t *) malloc(num_threads * sizeof(pthread_t)); 

    double time_parallel = 0.0;
    double sum_parallel = 0.0;

    //Timer Begin
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < num_threads; i++) { 
        pthread_attr_t attr;
        pthread_attr_init(&attr);
        thread_ids[i] = i;
        pthread_create(&workers[i], &attr, thread_func, &thread_ids[i]); 
    }

    for (int i = 0; i < num_threads; i++) 
        pthread_join(workers[i], NULL);

    for(int i = 0; i < num_threads; i++)
        sum_parallel += local_sums[i];

    clock_gettime(CLOCK_MONOTONIC, &end);
    //Timer End
    time_parallel = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / BILLION;
    printf("Parallel Sum = %.4f, time = %.3f \n", sum_parallel, time_parallel);

    /*free up resources properly */
    free(workers);
    exit(0);
}

void *thread_func(void *arg) { 
    /* Assign each thread an id so that they are unique in range [0, num_thread -1 ] */
    int *id_ptr = (int *) arg;
    int my_id = *id_ptr;

    int work_size = array_size / num_threads;
    int remainder = array_size % num_threads;
    
    int start_index = (int) work_size * my_id;
    int end_index = start_index + work_size;
   
    /* Perform Partial Parallel Sum Here */
    double my_sum = 0.0;
    
    // if there's a remainder, let the last thread handle it
    if (my_id == num_threads - 1 && remainder > 0) {
        for (int i = start_index; i < array_size; i++)
            my_sum += (double) randomArray[i];
    } else {
        // otherwise, proceed as usual
        for (int i = start_index; i < end_index; i++)
            my_sum += (double) randomArray[i];
    }

    printf("Thread %d sum = %f\n", my_id, my_sum);
    local_sums[my_id] = my_sum;
    pthread_exit(0);
}

// returns a number between 0 and 1
float RNG() {
    return (float) rand() / (float) RAND_MAX;
}
