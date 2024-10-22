#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024

// Function to handle each client connection
void *handle_client(void *socket_desc)
{
    int new_socket = *(int *)socket_desc;
    char buffer[BUFFER_SIZE] = {0};
    char *response = "World";

    // Read message from client
    read(new_socket, buffer, BUFFER_SIZE);
    printf("Received: %s\n", buffer);

    // Send response to client
    send(new_socket, response, strlen(response), 0);
    printf("Response sent: World\n");

    // Close the connection
    close(new_socket);
    free(socket_desc); // Free the allocated memory for socket descriptor
    return NULL;
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Bind the socket to the port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

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

    printf("Server is listening on port %d...\n", PORT);

    while (1)
    {
        // Accept a connection from a client
        int *new_socket_ptr = malloc(sizeof(int)); // Allocate memory for the socket descriptor
        if ((*new_socket_ptr = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed");
            free(new_socket_ptr); // Free memory in case of failure
            continue;
        }

        // Create a new thread for each client connection
        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_socket_ptr) != 0)
        {
            perror("Thread creation failed");
            free(new_socket_ptr); // Free memory if thread creation fails
        }

        pthread_detach(thread_id); // Detach the thread to clean up resources when it finishes
    }

    // Clean up (not reached in this simple example)
    close(server_fd);
    return 0;
}
