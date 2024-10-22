#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <linux/time.h>

#define BUFFER_SIZE 1024

int main(int argc, char *argv[])
{
    if (argc != 4)
    {
        printf("Usage: %s <server_ip> <server_port> <num_requests>\n", argv[0]);
        return -1;
    }

    char *server_ip = argv[1];
    int server_port = atoi(argv[2]);
    int num_requests = atoi(argv[3]);

    int sock;
    struct sockaddr_in server_addr;
    char buffer[BUFFER_SIZE];
    char *message = "Hello";
    struct timespec start, end;
    double total_time = 0.0;

    // Create socket
    for (int i = 0; i < num_requests; i++)
    {
        if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        {
            perror("Socket creation error");
            return -1;
        }

        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(server_port);

        // Convert IPv4 and IPv6 addresses from text to binary form
        if (inet_pton(AF_INET, server_ip, &server_addr.sin_addr) <= 0)
        {
            printf("Invalid address/ Address not supported\n");
            return -1;
        }

        // Connect to server
        if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
        {
            printf("Connection Failed\n");
            return -1;
        }

        // Measure time to send/receive data
        clock_gettime(CLOCK_MONOTONIC, &start);
        send(sock, message, strlen(message), 0);
        read(sock, buffer, BUFFER_SIZE);
        clock_gettime(CLOCK_MONOTONIC, &end);

        // Calculate time difference
        double request_time = (end.tv_sec - start.tv_sec) * 1e9;
        request_time = (request_time + (end.tv_nsec - start.tv_nsec)) * 1e-9;
        total_time += request_time;

        close(sock);
    }

    printf("Average time per request: %lf seconds\n", total_time / num_requests);
    return 0;
}
