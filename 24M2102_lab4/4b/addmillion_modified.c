#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include "common.h"

int account_balance = 0;
pthread_mutex_t mutex;

void *increment(void *arg)
{
    long deposit_amount = (long)arg;
    for (int i = 0; i < deposit_amount; i++)
    {
        pthread_mutex_lock(&mutex);
        account_balance++;
        pthread_mutex_unlock(&mutex);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 2)
    {
        fprintf(stderr, "Usage: %s <number_of_threads>\n", argv[0]);
        return 1;
    }

    int threadNum = atoi(argv[1]);
    if (threadNum <= 0)
    {
        fprintf(stderr, "Number of threads must be positive.\n");
        return 1;
    }

    long total_deposit = 2048;
    long deposit_per_thread = total_deposit / threadNum;

    if (total_deposit % threadNum != 0)
    {
        fprintf(stderr, "Total deposit must be evenly divisible by number of threads.\n");
        return 1;
    }

    pthread_t th[threadNum];

    pthread_mutex_init(&mutex, NULL);

    struct timespec start_time, end_time;
    clock_gettime(CLOCK_MONOTONIC, &start_time);

    for (int i = 0; i < threadNum; i++)
    {
        if (pthread_create(th + i, NULL, &increment, (void *)deposit_per_thread) != 0)
        {
            perror("Failed to create thread");
            return 1;
        }
        printf("Transaction %d has started with deposit: %ld million\n", i, deposit_per_thread);
    }

    for (int i = 0; i < threadNum; i++)
    {
        if (pthread_join(th[i], NULL) != 0)
        {
            return 2;
        }
        printf("Transaction %d has finished\n", i);
    }

    clock_gettime(CLOCK_MONOTONIC, &end_time);
    long time_spent = (end_time.tv_sec - start_time.tv_sec) * 1000 +
                      (end_time.tv_nsec - start_time.tv_nsec) / 1000000;

    printf("Account Balance is : %d million\n", account_balance);
    printf("Time spent: %ld ms\n", time_spent);

    pthread_mutex_destroy(&mutex);

    return 0;
}