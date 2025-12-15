/*
     The code skeleton in this task makes it seem like list length should be hardcoded at
     100. This seems weird as N will have a very little effect on the performance. The only
     place that N is used is in the initalization of the buffer. But since the assignments
     instructions are not clear enough on their own to perform the task, I assume I should
     also use the "clues" given in the code skeleton to perform the task, therefore the
     lists, list1 and list2 are hardcoded at 100 in length.

     I think some people (me included) see code skeletons in other courses as suggestions
     that can be chosen by the students if we use them. Therefore, I think it should be 
     made clear that the instructions in the code skeleton should be followed. 
     If they should, if not it should be made clear that the assignment description on 
     Canvas is the only instruction that should be followed.

     I think it can be a good idea to include the instructions that are in the code
     skeleton, in canvas anyways so that a student does not have to use it unless they
     want to. With the current instructions it is slighly unclear if we need to or not.
*/

#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

typedef struct 
{
     int offset;
     int len;
}
ListEntry;

typedef struct 
{
     // List to process
     ListEntry *list;

     // Length of list to process
     int len;

     // Buffer to write from
     void *buf;

     // File descriptor of file
     int fd;
}
ThreadData;

// This is what should be returned when we calculate the part of the list
// each thread gets, we need a ListEntry array and a len.
typedef struct 
{
     // List to process
     ListEntry *list;

     // Length of list to process
     int len;
}
ListReturn;

void *reader_thread_func(void *data) 
{ 
     
     // @Add code for reader threads
     // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
     // @for each: read bytes_i from offset_i
     ThreadData *r_data = (ThreadData *)data;

     char *buf = (char *)r_data->buf;
     int fd = r_data->fd;
     int len = r_data->len;
     ListEntry *list = r_data->list;

     for(int i = 0; i < len; i++) 
     {
          int offset = list[i].offset;
          int bytes = list[i].len;

          // pread reads 'bytes' from 'fd' at 'offset' into 'buf + offset'
          ssize_t read_bytes = pread(fd, buf + offset, bytes, offset);

          if (read_bytes != bytes) 
          {
               if (read_bytes == -1) 
               {
                    perror("Error reading from file");
               }
          }
     }

     // Important: If you malloc'd 'data' in main to fix the race condition,
     // you must free it here.
     free(data); 

     pthread_exit(0);
}


void *writer_thread_func(void *data) 
{ 
     
     // @Add code for writer threads
     // @Given a list of [offset1, bytes1], [offset2, bytes2], ...
     // @for each: write bytes_i to offset_i
     ThreadData *w_data = (ThreadData *)data;

     // So we can do pointer arithmetic
     char *buf = (char *)w_data->buf;
     int fd = w_data->fd;
     int len = w_data->len;
     ListEntry *list = w_data->list;

     for(int i = 0; i < len; i++) {
          int offset = list[i].offset;
          int bytes = list[i].len;

          // writes 'bytes' bytes sequentially from buffer buf+offset to file descriptor fd with offset 'offset'
          ssize_t written = pwrite(fd, buf + offset, bytes, offset);

          if (written != bytes) {
               perror("Error writing to file");
          }
     }

     free(data);

     pthread_exit(0);
}

int is_in_array(int *arr, int num, int len) 
{
     for(int i = 0; i < len; i++) {
          if(arr[i] == num) {
               return 1;
          }
     }
     return 0;
}

int main(int argc, char *argv[])
{
     if(argc != 3) {
          printf("Need 2 args please");
          return 1;
     }
     int total_bytes = atoi(argv[1]); //number of bytes
     int total_threads = atoi(argv[2]); //number of threads

     srand(time(NULL));

     const int LIST1_REQ_SIZE = 16384;
     const int LIST2_REQ_SIZE = 128;

     const int PAGE_SIZE = 4096;

     const int NR_OF_LISTS = 2;
     const int LIST_LEN = 100;

     // @create a file for saving the data
     int fd = open("test.txt", O_WRONLY | O_CREAT | O_TRUNC, 0644);

     if (fd == -1) 
     {
        perror("Error creating file");
        return 1;
     }
     
     // @allocate a buffer and initialize it
     void *buf = calloc(1, total_bytes);

     // @create two lists of 100 requests in the format of [offset, bytes]
     ListEntry rw_lists[NR_OF_LISTS][LIST_LEN];

     // @List 1: sequtial requests of 16384 bytes, where offset_n = offset_(n-1) + 16384
     // @e.g., [0, 16384], [16384, 16384], [32768, 16384] ...
     // @ensure no overlapping among these requests.
     for(int i = 0; i < LIST_LEN; i++) 
     {
          rw_lists[0][i].offset = i*LIST1_REQ_SIZE;
          rw_lists[0][i].len = LIST1_REQ_SIZE;
     }

     // @List 2: random requests of 128 bytes, where offset_n = random[0,N/4096] * 4096
     // @e.g., [4096, 128], [16384, 128], [32768, 128], etc.
     // @ensure no overlapping among these requests.
     int used_offsets[LIST_LEN];

     for(int i = 0; i < LIST_LEN; i++) 
     {
          // Select an index that is not already used to not get overlap
          int random_index = (rand() % (total_bytes / PAGE_SIZE)) * PAGE_SIZE;
          while(is_in_array(used_offsets, random_index, i)) 
          {
               random_index = (rand() % (total_bytes / PAGE_SIZE)) * PAGE_SIZE;
          }

          // Put new unused index in the used array, since we will use it
          // Note that i will track the index of the last used index in the array
          used_offsets[i] = random_index;

          rw_lists[1][i].offset = random_index;
          rw_lists[1][i].len = LIST2_REQ_SIZE;
     }

     printf("\n============== LIST1 ==============\n");

     // @start timing
     struct timeval start, end;
     gettimeofday(&start, NULL);

     /* Create writer workers and pass in their portion of list1 */

     pthread_t threads[total_threads];
     int cur_index = 0;

     int step = LIST_LEN / total_threads;

     for(int i = 0; i < total_threads; i++) 
     {
          int start_index = cur_index;

          // If its the last thread, it needs to take the remaining requests
          int end_index = (i == total_threads - 1) ? LIST_LEN : cur_index + step;

          ThreadData *data = malloc(sizeof(ThreadData));
          data->list = &rw_lists[0][start_index];
          data->len = end_index - start_index;
          data->buf = buf;
          data->fd = fd;

          pthread_create(&(threads[i]), NULL, writer_thread_func, data);

          cur_index += step;
     }
     
     /* Wait for all writers to finish */ 
     for(int i = 0; i < total_threads; i++)
     {
          pthread_join(threads[i], NULL);
     }
     
     // @close the file 
     close(fd);

     // @end timing 
     gettimeofday(&end, NULL);

     double elapsed_s = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
     double total_mb = (LIST_LEN * LIST1_REQ_SIZE) / (1048576.0);

     //@Print out the write bandwidth
     printf("Write %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", total_mb, total_threads, elapsed_s, total_mb/elapsed_s);
     
     
     // @reopen the file 
     fd = open("test.txt", O_RDONLY);
     if (fd == -1) 
     {
          perror("Error reopening file");
          return 1;
     }
     // @start timing 
     gettimeofday(&start, NULL);

     /* Create reader workers and pass in their portion of list1 */   
     cur_index = 0;

     for(int i = 0; i < total_threads; i++) 
     {
          int start_index = cur_index;

          // If its the last thread, it needs to take the remaining requests
          int end_index = (i == total_threads - 1) ? LIST_LEN : cur_index + step;

          ThreadData *data = malloc(sizeof(ThreadData));
          data->list = &rw_lists[0][start_index];
          data->len = end_index - start_index;
          data->buf = buf;
          data->fd = fd;

          pthread_create(&(threads[i]), NULL, reader_thread_func, data);

          cur_index += step;
     }
     
     /* Wait for all reader to finish */ 
     for(int i = 0; i < total_threads; i++)
     {
          pthread_join(threads[i], NULL);
     }

     // @close the file 
     close(fd);

     // @end timing 
     gettimeofday(&end, NULL);

     elapsed_s = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

     //@Print out the read bandwidth
     printf("Read %f MB, use %d threads, elapsed time %f s, read bandwidth: %f MB/s \n", total_mb, total_threads, elapsed_s, total_mb/elapsed_s);

     printf("\n============== LIST2 ==============\n");

     // @Repeat the write and read test now using List2 
     fd = open("test.txt", O_WRONLY | O_TRUNC);
     if (fd == -1) 
     {
        perror("Error opening file for List 2");
        return 1;
     }

     gettimeofday(&start, NULL);
     
     cur_index = 0;

     for(int i = 0; i < total_threads; i++) 
     {
          int start_index = cur_index;

          // If its the last thread, it needs to take the remaining requests
          int end_index = (i == total_threads - 1) ? LIST_LEN : cur_index + step;

          ThreadData *data = malloc(sizeof(ThreadData));
          data->list = &rw_lists[1][start_index];
          data->len = end_index - start_index;
          data->buf = buf;
          data->fd = fd;

          pthread_create(&(threads[i]), NULL, writer_thread_func, data);

          cur_index += step;
     }
     
     /* Wait for all writers to finish */ 
     for(int i = 0; i < total_threads; i++)
     {
          pthread_join(threads[i], NULL);
     }
     
     // @close the file 
     close(fd);

     gettimeofday(&end, NULL);

     elapsed_s = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
     total_mb = (LIST_LEN * LIST2_REQ_SIZE) / (1048576.0);

     //@Print out the write bandwidth
     printf("Write %f MB, use %d threads, elapsed time %f s, write bandwidth: %f MB/s \n", total_mb, total_threads, elapsed_s, total_mb/elapsed_s);

     fd = open("test.txt", O_RDONLY);
     if (fd == -1) 
     {
          perror("Error reopening file");
          return 1;
     }
     // @start timing 
     gettimeofday(&start, NULL);
     /* Create reader workers and pass in their portion of list1 */   
     cur_index = 0;

     for(int i = 0; i < total_threads; i++) 
     {
          int start_index = cur_index;

          // If its the last thread, it needs to take the remaining requests
          int end_index = (i == total_threads - 1) ? LIST_LEN : cur_index + step;

          ThreadData *data = malloc(sizeof(ThreadData));
          data->list = &rw_lists[1][start_index];
          data->len = end_index - start_index;
          data->buf = buf;
          data->fd = fd;

          pthread_create(&(threads[i]), NULL, reader_thread_func, data);

          cur_index += step;
     }
     
     /* Wait for all reader to finish */ 
     for(int i = 0; i < total_threads; i++)
     {
          pthread_join(threads[i], NULL);
     }

     // @close the file 
     close(fd);

     gettimeofday(&end, NULL);

     elapsed_s = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

     printf("Read %f MB, use %d threads, elapsed time %f s, read bandwidth: %f MB/s \n", total_mb, total_threads, elapsed_s, total_mb/elapsed_s);

     /*free up resources properly */
     free(buf);
     return 0;
}
