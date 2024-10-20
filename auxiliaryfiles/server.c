// create a socket for the server
// bind the server socket with a server address
// make the server socket passive by making it listen
// accept any connections if there are any connection requests present

#define BACKLOG 5
#define BUF_SIZE 1024

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    int server_socket_fd, accepted_fd, ret;
    struct sockaddr_in server_addr, client_addr;
    char *message = "world";
    char buffer[BUF_SIZE] = {0};

    // socket creation completed
    if ((server_socket_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("server socket creation failed.\n");
        return -1;
    }

    // socket binding with an address.
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(8080);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    if (bind(server_socket_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) != 0)
    {
        printf("socket binding failed.\n");
        return -1;
    }

    // listen for new connections
    if (listen(server_socket_fd, BACKLOG) != 0)
    {
        printf("listen failed at server.\n");
        return -1;
    }

    // accept the new connections

    socklen_t client_addr_len = sizeof(client_addr);
    for (;;)
    {
        if ((accepted_fd = accept(server_socket_fd, (struct sockaddr *)&client_addr, &client_addr_len)) < 0)
        {
            printf("unable to accept the connections.\n");
            return -1;
        }

        printf("the connection is accepted with the client at port number %d and IP address %d\n", server_addr.sin_port, server_addr.sin_addr.s_addr);

        memset(buffer, 0, BUF_SIZE);
        ret = recv(accepted_fd, buffer, BUF_SIZE, 0);
        if (ret == -1)
        {
            printf("Failed to receive message\n");
            return -1;
        }
        printf("Received: %s\n", buffer);

        ret = send(accepted_fd, message, strlen(message), 0);
        if (ret == -1)
        {
            printf("Failed to send message\n");
            return -1;
        }
        printf("Sent: %s\n", message);
        close(accepted_fd);
    }

    close(server_socket_fd);
    return 0;
}