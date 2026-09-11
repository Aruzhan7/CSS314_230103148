/*
 * Lab Practicum 01 - Task 2
 * Compute-bound benchmark: count primes up to LIMIT using N threads.
 * Usage: ./bench <num_threads>
 *
 * Compile (macOS / Linux, no OpenMP needed):
 *   gcc -O2 -pthread bench.c -o bench
 *
 * Run:
 *   ./bench 1
 *   ./bench 2
 *   ./bench 4
 *   ./bench 8
 *   ./bench 16
 *   ./bench 32
 *
 * Run each of these 3 times and record the wall-clock time it prints.
 */

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define LIMIT 5000000L

typedef struct {
    long start;
    long end;
    long local_count;
} thread_arg_t;

int is_prime(long n) {
    if (n < 2) return 0;
    if (n < 4) return 1;
    if (n % 2 == 0) return 0;
    for (long i = 3; i * i <= n; i += 2) {
        if (n % i == 0) return 0;
    }
    return 1;
}

void *worker(void *arg) {
    thread_arg_t *t = (thread_arg_t *)arg;
    long count = 0;
    for (long n = t->start; n < t->end; n++) {
        if (is_prime(n)) count++;
    }
    t->local_count = count;
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <num_threads>\n", argv[0]);
        return 1;
    }

    int num_threads = atoi(argv[1]);
    if (num_threads < 1) {
        fprintf(stderr, "Thread count must be >= 1\n");
        return 1;
    }

    pthread_t *threads = malloc(sizeof(pthread_t) * num_threads);
    thread_arg_t *args = malloc(sizeof(thread_arg_t) * num_threads);

    long chunk = LIMIT / num_threads;

    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    for (int i = 0; i < num_threads; i++) {
        args[i].start = i * chunk + 1;
        args[i].end = (i == num_threads - 1) ? LIMIT : (i + 1) * chunk + 1;
        args[i].local_count = 0;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }

    long total = 0;
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
        total += args[i].local_count;
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_end);

    double elapsed = (ts_end.tv_sec - ts_start.tv_sec) +
                      (ts_end.tv_nsec - ts_start.tv_nsec) / 1e9;

    printf("Threads: %d | Primes found up to %ld: %ld | Time: %.4f s\n",
           num_threads, LIMIT, total, elapsed);

    free(threads);
    free(args);
    return 0;
}
