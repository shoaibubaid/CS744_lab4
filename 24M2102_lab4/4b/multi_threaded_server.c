#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>

#define PORT 8080
#define MAX_CONNECTIONS 4096
#define BUFFER_SIZE 1024

void *handle_client(void *socket_desc)
{
    int new_socket = *(int *)socket_desc;
    char buffer[BUFFER_SIZE] = {0};
    char *response = "World";

    read(new_socket, buffer, BUFFER_SIZE);
    printf("Received: %s\n", buffer);

    send(new_socket, response, strlen(response), 0);
    printf("Response sent: World\n");

    close(new_socket);
    free(socket_desc);
    return NULL;
}

int main()
{
    int server_fd, new_socket;
    struct sockaddr_in address;
    int addrlen = sizeof(address);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CONNECTIONS) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    while (1)
    {

        int *new_socket_ptr = malloc(sizeof(int));
        if ((*new_socket_ptr = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            perror("Accept failed");
            free(new_socket_ptr);
            continue;
        }

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, handle_client, (void *)new_socket_ptr) != 0)
        {
            perror("Thread creation failed");
            free(new_socket_ptr);
        }

        pthread_detach(thread_id);
    }

    close(server_fd);
    return 0;
}