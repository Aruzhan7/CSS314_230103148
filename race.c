/*
 * Lab Practicum 01 - Task 3
 * 10 threads each increment a shared global counter 1,000,000 times.
 * Expected total: 10 * 1,000,000 = 10,000,000
 *
 * Usage:
 *   ./race unlocked   -> runs WITHOUT mutex (race condition, corrupted result)
 *   ./race locked      -> runs WITH mutex (correct result, but slower)
 *
 * Compile:
 *   gcc -O2 -pthread race.c -o race
 *
 * For Q3 table: run "./race unlocked" 10 times, record the output each time.
 * For Q3.2: run "./race locked" once or a few times, record the time,
 * and compare to the average unlocked time.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>

#define NUM_THREADS 10
#define INCREMENTS_PER_THREAD 1000000L

volatile long shared_counter = 0;
pthread_mutex_t lock;
int use_lock = 0;

void *increment_unlocked(void *arg) {
    for (long i = 0; i < INCREMENTS_PER_THREAD; i++) {
        shared_counter++;   // NOT atomic: Load -> Add -> Store, can interleave
    }
    return NULL;
}

void *increment_locked(void *arg) {
    for (long i = 0; i < INCREMENTS_PER_THREAD; i++) {
        pthread_mutex_lock(&lock);
        shared_counter++;
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    if (argc != 2 || (strcmp(argv[1], "unlocked") != 0 && strcmp(argv[1], "locked") != 0)) {
        fprintf(stderr, "Usage: %s <unlocked|locked>\n", argv[0]);
        return 1;
    }

    use_lock = (strcmp(argv[1], "locked") == 0);
    shared_counter = 0;

    if (use_lock) {
        pthread_mutex_init(&lock, NULL);
    }

    pthread_t threads[NUM_THREADS];

    struct timespec ts_start, ts_end;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);

    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_create(&threads[i], NULL,
                        use_lock ? increment_locked : increment_unlocked,
                        NULL);
    }
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    clock_gettime(CLOCK_MONOTONIC, &ts_end);
    double elapsed_ms = ((ts_end.tv_sec - ts_start.tv_sec) * 1000.0) +
                         ((ts_end.tv_nsec - ts_start.tv_nsec) / 1e6);

    long expected = (long)NUM_THREADS * INCREMENTS_PER_THREAD;
    long error = expected - shared_counter;

    printf("Mode: %-9s | Measured: %ld | Expected: %ld | Error: %ld | Time: %.3f ms\n",
           argv[1], shared_counter, expected, error, elapsed_ms);

    if (use_lock) {
        pthread_mutex_destroy(&lock);
    }
    return 0;
}
