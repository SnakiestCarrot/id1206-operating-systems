
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define PAGE_REF_STR_LEN 1000

int reference_string[PAGE_REF_STR_LEN];
pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;
bool is_running = true;

typedef struct page
{
    int page_id;
    int reference_bit;
    int access_count;
    struct page *next;
    struct page *prev;
    // @other auxiliary
} Node;

typedef struct page_list
{
    Node *head;
    Node *tail;
    int size;
} PageList;

PageList *active_list;
PageList *inactive_list;

Node *get_page_with_id(PageList *list, int page_id)
{
    Node *current = list->head;
    while (current != NULL) 
    {
        if (current->page_id == page_id) 
        {
            return current;
        }
        current = current->next;
    }
    return NULL; // Not found
}

void init_page_list(PageList *list)
{
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

/*
    Yes, this function could be merged with get_page_with_id, 
    but for clarity I kept them separate. Performance is not a concern in this assignment.
    It will be O(2N) instead of O(N) in worst case, for one list.
*/
bool is_page_in_list(PageList *list, int page_id)
{
    Node *current = list->head;
    while (current != NULL) 
    {
        if (current->page_id == page_id) 
        {
            return true;
        }
        current = current->next;
    }
    return false;
}

void unlink_page(PageList *list, Node *page)
{
    // If page has a previous page in list
    if (page->prev != NULL) 
    {
        // simply unlink it
        page->prev->next = page->next;
    } else { // if not, we are the head
        list->head = page->next;
    }

    // If page has a next page in list
    if (page->next != NULL) 
    {
        // simply unlink it
        page->next->prev = page->prev;
    } else 
    { // if not, we are the tail
        list->tail = page->prev;
    }
    list->size--;
}

void link_page_to_tail(PageList *list, Node *page)
{
    page->next = NULL;
    page->prev = list->tail;

    if (list->tail != NULL) {
        list->tail->next = page;
    } else {
        list->head = page;
    }
    list->tail = page;
    list->size++;
}

Node *create_new_page(int page_id)
{
    Node *new_page = (Node *)malloc(sizeof(Node));
    new_page->page_id = page_id;
    new_page->reference_bit = 1;
    new_page->access_count = 0;
    new_page->next = NULL;
    new_page->prev = NULL;
    return new_page;
}

void free_list_nodes(PageList *list) 
{
    Node *curr = list->head;
    while (curr != NULL) {
        Node *temp = curr;
        curr = curr->next;
        free(temp);
    }
}

void *player_thread_func(void *arg)
{
    const int N = *((int *)arg);
    for(int i = 0; i < PAGE_REF_STR_LEN; i++) 
    {
        usleep(10);

        // acquire list lock, to not collide with checker thread
        pthread_mutex_lock(&list_lock);
        int page_id = reference_string[i];

        if(is_page_in_list(active_list, page_id)) 
        {
            Node *page = get_page_with_id(active_list, page_id);
            page->reference_bit = 1;

            // unlink from current spot
            unlink_page(active_list, page);

            // link to tail of active list, "promote" it
            link_page_to_tail(active_list, page);
        } 
        else if(is_page_in_list(inactive_list, page_id)) 
        {
            Node *page = get_page_with_id(inactive_list, page_id);
            page->reference_bit = 1;
            
            // remove from inactive list current spot
            unlink_page(inactive_list, page);
            
            // add to active list
            link_page_to_tail(active_list, page);
        } 
        else 
        { // "page fault"
            // We create a new page
            Node *page = create_new_page(page_id);

            // add to active list
            link_page_to_tail(active_list, page);
        }

        // if active list is over 70% of N, move 20% to inactive
        if(active_list->size > 0.7*N) 
        {
            int num_pages_to_move = 0.2*active_list->size;
            
            // to ensure at least one page is moved
            if (num_pages_to_move == 0 && active_list->size > 0) num_pages_to_move = 1;
            
            for(int j = 0; j < num_pages_to_move; j++) 
            {
                // move head to inactive list on each iteration
                Node *page = active_list->head;
                unlink_page(active_list, page);
                link_page_to_tail(inactive_list, page);
            }
        }
        pthread_mutex_unlock(&list_lock);
    }

    pthread_exit(0);
}

void *checker_thread_func(void *arg)
{
    const int M = *((int *)arg);
    while(is_running) 
    {
        usleep(M); // M milliseconds

        // acquire list lock, to not collide with player thread
        pthread_mutex_lock(&list_lock);

        // get first page in active list, to start traversal
        Node *current_page = active_list->head;

        // traverse active list
        while(current_page != NULL) 
        {
            if(current_page->reference_bit == 1) 
            {
                current_page->reference_bit = 0;
                current_page->access_count++;
            }
            current_page = current_page->next;
        }
        pthread_mutex_unlock(&list_lock);
    }
    pthread_exit(0);
}

void print_list(PageList *list) 
{
    Node *curr = list->head;
    while(curr != NULL) 
    {
        printf("%d ", curr->page_id);
        curr = curr->next;
    }
    printf("\n");
}

int main(int argc, char *argv[])
{
    if(argc != 3) 
    {
        printf("Usage: %s <N> <M>\n", argv[0]);
        return 1;
    }

    srand(time(NULL));

    active_list = (PageList*)malloc(sizeof(PageList));
    inactive_list = (PageList*)malloc(sizeof(PageList));

    int N = atoi(argv[1]);
    int M = atoi(argv[2]);

    init_page_list(active_list);
    init_page_list(inactive_list);

    // create random page reference string
    // with values between 0 and N-1
    for(int i = 0; i < PAGE_REF_STR_LEN; i++) 
    {
        reference_string[i] = rand() % N;
    }

    /* Create two workers */
    pthread_t player;
    pthread_t checker;

    pthread_create(&player, NULL, player_thread_func, &N); // Pass N as argument
    pthread_create(&checker, NULL, checker_thread_func, &M); // Pass M as argument

    pthread_join(player, NULL);
    is_running = false;

    pthread_join(checker, NULL);

    printf("Page_Id\t\tTotal_Referenced\n");
    //Print out the statistics of page references

    Node *curr = active_list->head;
    while(curr != NULL)
    {
        printf("%d\t\t%d\n", curr->page_id, curr->access_count);
        curr = curr->next;
    }
    
    curr = inactive_list->head;
    while(curr != NULL)
    {
        printf("%d\t\t%d\n", curr->page_id, curr->access_count);
        curr = curr->next;
    }

    printf("Pages in active list: ");
    //Print out the list of pages in active list
    print_list(active_list);
    printf("Pages in inactive list: ");
    //Print out the list of pages in inactive list
    print_list(inactive_list);
    /*free up resources properly */
    free_list_nodes(active_list);
    free_list_nodes(inactive_list);
    free(active_list);
    free(inactive_list);
    return 0;
}
