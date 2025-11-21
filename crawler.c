#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <unistd.h>
#include <curl/curl.h>

// Define a structure for queue elements.
typedef struct URLQueueNode {
    char *url;
    struct URLQueueNode *next;
} URLQueueNode;

// Define a structure for a thread-safe queue.
typedef struct {
    URLQueueNode *head, *tail;
    pthread_mutex_t lock;
} URLQueue;

// Initialize a URL queue.
void initQueue(URLQueue *queue) {
    queue->head = queue->tail = NULL;
    pthread_mutex_init(&queue->lock, NULL);
}

// Add a URL to the queue.
void enqueue(URLQueue *queue, const char *url) {
    URLQueueNode *newNode = malloc(sizeof(URLQueueNode));
    newNode->url = strdup(url);
    newNode->next = NULL;

    pthread_mutex_lock(&queue->lock);
    if (queue->tail) {
        queue->tail->next = newNode;
    } else {
        queue->head = newNode;
    }
    queue->tail = newNode;
    pthread_mutex_unlock(&queue->lock);
}

// Remove a URL from the queue.
char *dequeue(URLQueue *queue) {
    pthread_mutex_lock(&queue->lock);
    if (queue->head == NULL) {
        pthread_mutex_unlock(&queue->lock);
        return NULL;
    }

    URLQueueNode *temp = queue->head;
    char *url = temp->url;
    queue->head = queue->head->next;
    if (queue->head == NULL) {
        queue->tail = NULL;
    }
    free(temp);
    pthread_mutex_unlock(&queue->lock);
    return url;
}

void destroyQueue(URLQueue *queue) {          
    pthread_mutex_lock(&queue->lock);         

    URLQueueNode *cur = queue->head;          
    while (cur) {                              
        URLQueueNode *next = cur->next;       
        free(cur->url);                       
        free(cur);                            
        cur = next;                           
    }                                         

    queue->head = queue->tail = NULL;        
    pthread_mutex_unlock(&queue->lock);       
    pthread_mutex_destroy(&queue->lock);      
}                                             

// Placeholder for the function to fetch and process a URL.
void *fetch_url(void *arg) {
    // Cast arg to your queue or custom data structure.
    URLQueue *queue = (URLQueue *)arg;
    // added a worker loop to the function
    while (1) {
        char *url = dequeue(queue);
        if (url == NULL) break;
        printf("[Thread %lu] Processing URL: %s\n",
            (unsigned long)pthread_self(), url);
        sleep(1);
        free(url);                                      
      }                            

    return NULL;
}

// Main function to drive the web crawler.
int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Usage: %s <starting-url>\n", argv[0]);
        return 1;
    }

    URLQueue queue;
    initQueue(&queue);
    enqueue(&queue, argv[1]);

    
// added a multi-URL enqueue loop    
    for (int i = 1; i < argc; i++) {
        enqueue(&queue, argv[i]);                        
     }     
    const int NUM_THREADS = 4; // Example thread count, adjust as needed.
    pthread_t threads[NUM_THREADS];

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL, fetch_url, (void *)&queue);
    }

    // Join threads after completion.
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    destroyQueue(&queue);
    
    return 0;

}

