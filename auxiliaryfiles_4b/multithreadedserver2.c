#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8081
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024

// Structure to hold the connection request
typedef struct
{
    int socket;
} request_t;

// Node for the shared queue
typedef struct node
{
    request_t request;
    struct node *next;
} node_t;

// Shared queue
typedef struct
{
    node_t *front;
    node_t *rear;
    int count;
    pthread_mutex_t mutex;
    pthread_cond_t cond;
} queue_t;

queue_t request_queue;

// Function to initialize the queue
void init_queue(queue_t *q)
{
    q->front = NULL;
    q->rear = NULL;
    q->count = 0;
    pthread_mutex_init(&q->mutex, NULL);
    pthread_cond_init(&q->cond, NULL);
}

// Function to enqueue a request
void enqueue(queue_t *q, request_t req)
{
    node_t *new_node = malloc(sizeof(node_t));
    new_node->request = req;
    new_node->next = NULL;

    pthread_mutex_lock(&q->mutex);
    if (q->rear == NULL)
    {
        q->front = new_node;
        q->rear = new_node;
    }
    else
    {
        q->rear->next = new_node;
        q->rear = new_node;
    }
    q->count++;
    pthread_cond_signal(&q->cond);
    pthread_mutex_unlock(&q->mutex);
}

// Function to dequeue a request
request_t dequeue(queue_t *q)
{
    pthread_mutex_lock(&q->mutex);
    while (q->count == 0)
    {
        pthread_cond_wait(&q->cond, &q->mutex);
    }
    node_t *temp = q->front;
    request_t req = temp->request;
    q->front = q->front->next;
    if (q->front == NULL)
    {
        q->rear = NULL;
    }
    q->count--;
    free(temp);
    pthread_mutex_unlock(&q->mutex);
    return req;
}

// Function to handle each client connection
void *worker_thread(void *arg)
{
    while (1)
    {
        request_t req = dequeue(&request_queue);
        char buffer[BUFFER_SIZE] = {0};
        char *response = "World";

        // Read message from client
        read(req.socket, buffer, BUFFER_SIZE);
        printf("Received: %s\n", buffer);

        // Send response to client
        send(req.socket, response, strlen(response), 0);
        printf("Response sent: World\n");

        // Close the connection
        close(req.socket);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <port> <thread_pool_size>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int server_fd, new_socket, thread_pool_size = atoi(argv[2]);
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Initialize the request queue
    init_queue(&request_queue);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(atoi(argv[1]));

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, MAX_CONNECTIONS) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %s...\n", argv[1]);

    // Create the thread pool
    pthread_t threads[thread_pool_size];
    for (int i = 0; i < thread_pool_size; i++)
    {
        pthread_create(&threads[i], NULL, worker_thread, NULL);
    }

    while (1)
    {
        // Accept a connection from a client
        new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
        if (new_socket < 0)
        {
            perror("Accept failed");
            continue;
        }

        request_t req = {.socket = new_socket};
        enqueue(&request_queue, req);
    }

    // Clean up (not reached in this simple example)
    close(server_fd);
    return 0;
}
