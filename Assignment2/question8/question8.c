#include <pthread.h> 
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
     
int num_threads = 0;
int global_node_id_counter = 0;
pthread_mutex_t stack_mutex;
     
typedef struct node { 
     int node_id;      //a unique ID assigned to each node
     struct node *next;
} Node;

// Structure to pass multiple arguments to thread_func
typedef struct {
    int id;
    int opt; // 0 for Mutex, 1 for CAS
} ThreadConfig;

Node *top = NULL; // top of stack

/* Option 1: Mutex Lock */
void push_mutex() {
     Node *old_node;
     Node *new_node;
     new_node = malloc(sizeof(Node)); 
     if (!new_node) { perror("Malloc failed"); exit(1); }

     // Critical Section Start
     pthread_mutex_lock(&stack_mutex);
     
     new_node->node_id = global_node_id_counter++;

     // update top of the stack below
     old_node = top;
     new_node->next = old_node;
     top = new_node;
     
     pthread_mutex_unlock(&stack_mutex);
     // Critical Section End
}

int pop_mutex() {
     Node *old_node;
     Node *new_node; 
     int popped_id = -1;

     // update top of the stack below
     // Critical Section Start
     pthread_mutex_lock(&stack_mutex);
     
     old_node = top;
     
     if (old_node != NULL) {
          new_node = old_node->next;
          top = new_node;
          popped_id = old_node->node_id;
          free(old_node); 
     }
     
     pthread_mutex_unlock(&stack_mutex);
     // Critical Section End

     return popped_id;
}

/* Option 2: Compare-and-Swap (CAS) */
void push_cas() {
     Node *new_node = malloc(sizeof(Node));
     if (!new_node) { perror("Malloc failed"); exit(1); }

     // For CAS (Lock-Free) atomic increment for the ID
     new_node->node_id = __sync_fetch_and_add(&global_node_id_counter, 1);

     Node *old_top;

     // update top of the stack below
     do {
          old_top = top;              // Snapshot
          new_node->next = old_top;   // Link
          // CAS: If top is still old_top, set it to new_node
     } while (!__sync_bool_compare_and_swap(&top, old_top, new_node));
}

int pop_cas() {
     Node *old_top;
     Node *new_top;
     int popped_id = -1;

     // update top of the stack below
     do {
          old_top = top;
          if (old_top == NULL) {
               return -1; // Stack empty
          }
          new_top = old_top->next;
          // CAS: If top is still old_top, move it to next
     } while (!__sync_bool_compare_and_swap(&top, old_top, new_top));

     popped_id = old_top->node_id;
     
     return popped_id;
}

/* the thread function */
void *thread_func(void *arg) {
     ThreadConfig *config = (ThreadConfig *)arg;
     int opt = config->opt;
     int my_id = config->id;

     if( opt==0 ){
          push_mutex();push_mutex();pop_mutex();pop_mutex();push_mutex();
     }else{
          push_cas();push_cas();pop_cas();pop_cas();push_cas();
     }
     
     free(config); // Clean up arguments
     pthread_exit(0);
}

// Helper to print count
void print_remaining_nodes(char *mode) {
     int count = 0;
     Node *curr = top;
     while (curr) {
          count++;
          curr = curr->next;
     }
     printf("%s: Remaining nodes: %d\n", mode, count);
}


// Helper to clear stack between runs
void clear_stack() {
     while (top) {
          Node *temp = top;
          top = top->next;
          free(temp);
     }
}

int main(int argc, char *argv[])
{
     if (argc < 2) {
          printf("Usage: %s <num_threads>\n", argv[0]);
          return 1;
     }
     num_threads = atoi(argv[1]);

     pthread_t *workers = malloc(num_threads * sizeof(pthread_t));

     /* Option 1: Mutex */ 
     printf("Starting Mutex run with %d threads.\n", num_threads);
     global_node_id_counter = 0; // Reset ID counter
     pthread_mutex_init(&stack_mutex, NULL);
     
     for (int i = 0; i < num_threads; i++) { 
          ThreadConfig *cfg = malloc(sizeof(ThreadConfig));
          cfg->id = i;
          cfg->opt = 0; // Mutex mode
          pthread_create(&workers[i], NULL, thread_func, cfg); 
     }
     for (int i = 0; i < num_threads; i++) 
          pthread_join(workers[i], NULL);

     //Print out all remaining nodes in Stack
     print_remaining_nodes("Mutex");

     clear_stack();
     pthread_mutex_destroy(&stack_mutex);

     /* Option 2: CAS */ 
     printf("Starting CAS run with %d threads.\n", num_threads);
     global_node_id_counter = 0; // Reset ID counter
     
     for (int i = 0; i < num_threads; i++) { 
          ThreadConfig *cfg = malloc(sizeof(ThreadConfig));
          cfg->id = i;
          cfg->opt = 1; // CAS mode
          pthread_create(&workers[i], NULL, thread_func, cfg); 
     }
     for (int i = 0; i < num_threads; i++) 
          pthread_join(workers[i], NULL);

     //Print out all remaining nodes in Stack
     print_remaining_nodes("CAS");
     clear_stack();
     free(workers);

     return 0;
}