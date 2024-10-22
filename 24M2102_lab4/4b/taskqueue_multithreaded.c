#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>

#define MAX_QUEUE_SIZE 100

long sum = 0;
long odd = 0;
long even = 0;
long min = INT_MAX;
long max = INT_MIN;
bool done = false;

typedef struct
{
    char type;
    long number;
} Task;

Task task_queue[MAX_QUEUE_SIZE];
int front = 0;
int rear = 0;
int count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond_var = PTHREAD_COND_INITIALIZER;

void enqueue(Task task)
{
    task_queue[rear] = task;
    rear = (rear + 1) % MAX_QUEUE_SIZE;
    count++;
}

Task dequeue()
{
    Task task = task_queue[front];
    front = (front + 1) % MAX_QUEUE_SIZE;
    count--;
    return task;
}

bool is_empty()
{
    return count == 0;
}

void processtask(long number)
{
    sleep(number);

    pthread_mutex_lock(&mutex);
    sum += number;
    if (number % 2 == 1)
    {
        odd++;
    }
    else
    {
        even++;
    }
    if (number < min)
    {
        min = number;
    }
    if (number > max)
    {
        max = number;
    }
    pthread_mutex_unlock(&mutex);
}

void *worker(void *arg)
{
    while (true)
    {
        pthread_mutex_lock(&mutex);
        while (is_empty() && !done)
        {
            pthread_cond_wait(&cond_var, &mutex);
        }
        if (done && is_empty())
        {
            pthread_mutex_unlock(&mutex);
            break;
        }
        Task task = dequeue();
        pthread_mutex_unlock(&mutex);

        if (task.type == 'p')
        {
            processtask(task.number);
            printf("Processed task: %ld seconds\n", task.number);
        }
        else if (task.type == 'w')
        {
            sleep(task.number);
            printf("Waited: %ld seconds\n", task.number);
        }
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <infile> <number_of_threads>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    char *fn = argv[1];
    int num_threads = atoi(argv[2]);
    if (num_threads <= 0)
    {
        printf("Number of threads must be positive.\n");
        exit(EXIT_FAILURE);
    }

    pthread_t threads[num_threads];
    for (int i = 0; i < num_threads; i++)
    {
        pthread_create(&threads[i], NULL, worker, NULL);
    }

    FILE *fin = fopen(fn, "r");
    long t;
    fscanf(fin, "%ld\n", &t);
    printf("The number of tasks are : %ld\n", t);
    char type;
    long num;
    while (fscanf(fin, "%c %ld\n", &type, &num) == 2)
    {
        Task task;
        task.type = type;
        task.number = num;

        pthread_mutex_lock(&mutex);
        enqueue(task);
        pthread_cond_signal(&cond_var);
        pthread_mutex_unlock(&mutex);
    }

    fclose(fin);

    pthread_mutex_lock(&mutex);
    done = true;
    pthread_cond_broadcast(&cond_var);
    pthread_mutex_unlock(&mutex);

    for (int i = 0; i < num_threads; i++)
    {
        pthread_join(threads[i], NULL);
    }

    printf("%ld %ld %ld %ld %ld\n", sum, odd, even, min, max);

    return (EXIT_SUCCESS);
}