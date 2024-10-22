#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

#define PORT 8085
#define MAX_EVENTS 5
#define BUFFER_SIZE 1024

int main()
{
    int server_fd, new_socket, epoll_fd;
    struct sockaddr_in address;
    struct epoll_event event, events[MAX_EVENTS];
    char buffer[BUFFER_SIZE] = {0};
    char *response = "World";
    int addrlen = sizeof(address);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0)
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

    if (listen(server_fd, 5) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Server is listening on port %d...\n", PORT);

    epoll_fd = epoll_create1(0);
    if (epoll_fd == -1)
    {
        perror("Epoll create failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    event.events = EPOLLIN;
    event.data.fd = server_fd;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event) == -1)
    {
        perror("Epoll_ctl failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    while (1)
    {
        int event_count = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < event_count; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen);
                if (new_socket < 0)
                {
                    perror("Accept failed");
                    continue;
                }

                event.events = EPOLLIN;
                event.data.fd = new_socket;
                epoll_ctl(epoll_fd, EPOLL_CTL_ADD, new_socket, &event);
                printf("New connection: %d\n", new_socket);
            }
            else
            {
                int client_fd = events[i].data.fd;
                read(client_fd, buffer, BUFFER_SIZE);
                printf("Received: %s\n", buffer);
                send(client_fd, response, strlen(response), 0);
                printf("Response sent: World\n");
                close(client_fd);
                epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_fd, NULL);
            }
        }
    }

    close(server_fd);
    return 0;
}
