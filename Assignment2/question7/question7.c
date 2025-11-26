#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <time.h>

const int NUM_BINS = 30;
int NUM_THREADS;
int ARRAY_SIZE;

double *data_array;      // Array of doubles [0.0, 1.0]
long *serial_histogram; 
long *parallel_histogram;

//Work for each thread and array to store local histogram
typedef struct {
    int start_index; 
    int end_index;
    long *thread_histogram;
} ThreadData;

// Initializes the array with random values in [0.0, 1.0]
void initialize_array(double *arr, int size) {
    srand(time(NULL));
    for (long long i = 0; i < size; i++) {
        // Generate a random double between 0.0 and 1.0
        arr[i] = (double)rand() / (double)RAND_MAX; 
    }
}

// Determines the correct bin index (0 to 29) for a given value
int get_bin_index(double value) {
    if (value >= 1.0) {
        return NUM_BINS - 1; 
    }
    return (int)floor(value * NUM_BINS); 
}

// Prints the histogram counts
void print_histogram(const char *name, long *hist) {
    // Note: Printing only the first 5 and last 5 bins for brevity in a large output
    printf("Histogram (%s :\n", name);
    for (int i = 0; i < NUM_BINS; i++) {
        printf("  Bin %02d (%.4f-%.4f): %ld\n", i, (double)i/NUM_BINS, (double)(i+1)/NUM_BINS, hist[i]); 
    }
}

// Create histogram in serial by main thread
void compute_serial_histogram() {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);

    for (int i = 0; i < ARRAY_SIZE; i++) {
        int bin = get_bin_index(data_array[i]);
        serial_histogram[bin]++;
    }

    clock_gettime(CLOCK_MONOTONIC, &end);
    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;
    printf("\n--- SERIAL HISTOGRAM ---\n");
    print_histogram("Serial", serial_histogram);
    printf("Serial Time: %.6f seconds\n", elapsed);
}

// Thread funtion to create local histogram from assigned work chunck
void *thread_compute_parallel_histogram(void *arg) {
    ThreadData *data = (ThreadData *)arg;

    for (int i = data->start_index; i < data->end_index; i++){
        int bin = get_bin_index(data_array[i]);
        data->thread_histogram[bin]++;
    }

    pthread_exit(NULL);
}

// Create histogram in parallel
void compute_parallell_histogram() {
    //Allocate memory to list of threads and thread data
    pthread_t *threads = malloc(NUM_THREADS * sizeof(pthread_t));
    ThreadData *thread_data = malloc(NUM_THREADS * sizeof(ThreadData));

    //Divide work for each worker thread
    int chunck_size = ARRAY_SIZE / NUM_THREADS;
    int remaining = ARRAY_SIZE % NUM_THREADS;
    int current_start = 0;

    struct timespec start, end;

    //Divide work and create ThreadData
    for (int i = 0; i < NUM_THREADS; i++) {
        //Extra work to first workers if array size is not divisible by number of threads
        int current_end = current_start + chunck_size + (i < remaining ? 1 : 0);

        //Create the data for each thread
        thread_data[i].thread_histogram = calloc(NUM_BINS, sizeof(long));
        thread_data[i].start_index = current_start;
        thread_data[i].end_index = current_end;

        current_start = current_end;
    }

    clock_gettime(CLOCK_MONOTONIC, &start);

    //Start threads
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, thread_compute_parallel_histogram, &thread_data[i]);
    }

    //Wait for all threads to finish
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);

    //Aggregate the result from the threads to one histogram
    for (int i = 0; i < NUM_THREADS; i++) {
        for (int j = 0; j < NUM_BINS; j++) {
            parallel_histogram[j] += thread_data[i].thread_histogram[j];
        }
    }

    double elapsed = (end.tv_sec - start.tv_sec) + (end.tv_nsec - start.tv_nsec) / 1000000000.0;

    printf("\n--- PARALLEL HISTOGRAM (Threads: %d) ---\n", NUM_THREADS);
    print_histogram("Parallel", parallel_histogram);
    printf("Parallel Time: %.6f seconds\n", elapsed);


    //Cleanup
    for (int i = 0; i < NUM_THREADS; i++) {
        free(thread_data[i].thread_histogram); // Cleanup local buffer
    }
    free(threads);
    free(thread_data);
}

int main(int argc, char *argv[]) {
    if(argc != 3) {
        char *error_message = "\nProvide an argument for num of threads and array size\n";
        printf("%s", error_message);
        return 0;
    }

    NUM_THREADS = atoi(argv[1]);
    ARRAY_SIZE = atoi(argv[2]);

    data_array = malloc(ARRAY_SIZE * sizeof(double));
    serial_histogram = calloc(NUM_BINS, sizeof(long));
    parallel_histogram = calloc(NUM_BINS, sizeof(long));

    initialize_array(data_array, ARRAY_SIZE);

    compute_serial_histogram();
    compute_parallell_histogram();


    // cleanup
    free(data_array);
    free(serial_histogram);
    free(parallel_histogram);
    
    return 0;
}