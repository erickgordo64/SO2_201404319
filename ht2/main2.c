#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define MAX_NUMBER 100000
#define NUM_THREADS 2

long long sum = 0;
pthread_mutex_t mutex_sum = PTHREAD_MUTEX_INITIALIZER;

void *calculate_sum_part(void *thread_id) {
    long long start = (*((int *)thread_id)) * (MAX_NUMBER / NUM_THREADS) + 1;
    long long end = (*((int *)thread_id) + 1) * (MAX_NUMBER / NUM_THREADS);
    long long partial_sum = 0;

    for (long long i = start; i <= end; i++) {
        partial_sum += i;
    }

    pthread_mutex_lock(&mutex_sum);
    sum += partial_sum;
    pthread_mutex_unlock(&mutex_sum);

    pthread_exit(NULL);
}

int main() {
    clock_t start_time, end_time;
    double total_time;

    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    start_time = clock();

    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        pthread_create(&threads[i], NULL, calculate_sum_part, (void *)&thread_ids[i]);
    }

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    end_time = clock();
    total_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    printf("Suma utilizando dos hilos: %lld\n", sum);
    printf("Tiempo de ejecución: %f segundos\n", total_time);

    return 0;
}
