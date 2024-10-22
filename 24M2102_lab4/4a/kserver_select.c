#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/select.h>

#define PORT 8084
#define MAX_CONNECTIONS 5
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, new_socket, client_socket[MAX_CONNECTIONS], max_sd, sd, activity;
    struct sockaddr_in address;
    fd_set readfds;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char *response = "World";

    for (int i = 0; i < MAX_CONNECTIONS; i++)
    {
        client_socket[i] = 0;
    }

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
        exit(EXIT_FAILURE);
    }

    if (listen(server_fd, MAX_CONNECTIONS) < 0)
    {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", PORT);

    while (1)
    {
        FD_ZERO(&readfds);

        FD_SET(server_fd, &readfds);
        max_sd = server_fd;

        for (int i = 0; i < MAX_CONNECTIONS; i++)
        {
            sd = client_socket[i];
            if (sd > 0)
                FD_SET(sd, &readfds);
            if (sd > max_sd)
                max_sd = sd;
        }

        activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);

        if (FD_ISSET(server_fd, &readfds))
        {
            if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
            {
                perror("Accept failed");
                exit(EXIT_FAILURE);
            }

            printf("New connection, socket fd: %d\n", new_socket);

            for (int i = 0; i < MAX_CONNECTIONS; i++)
            {
                if (client_socket[i] == 0)
                {
                    client_socket[i] = new_socket;
                    printf("Added to list of sockets\n");
                    break;
                }
            }
        }

        for (int i = 0; i < MAX_CONNECTIONS; i++)
        {
            sd = client_socket[i];
            if (FD_ISSET(sd, &readfds))
            {
                if (read(sd, buffer, BUFFER_SIZE) == 0)
                {
                    close(sd);
                    client_socket[i] = 0;
                    printf("Client disconnected\n");
                }
                else
                {
                    printf("Received: %s\n", buffer);
                    send(sd, response, strlen(response), 0);
                    printf("Response sent: World\n");
                }
            }
        }
    }

    return 0;
}
