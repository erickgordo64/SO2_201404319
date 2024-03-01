#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_NUMBER 100000

long long sum = 0;

void calculate_sum() {
    for (int i = 1; i <= MAX_NUMBER; i++) {
        sum += i;
    }
}

int main() {
    clock_t start_time, end_time;
    double total_time;

    start_time = clock();

    calculate_sum();

    end_time = clock();
    total_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;

    printf("Suma utilizando un solo hilo: %lld\n", sum);
    printf("Tiempo de ejecución: %f segundos\n", total_time);

    return 0;
}
