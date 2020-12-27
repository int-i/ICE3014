#include <pthread.h>
#include <stdio.h>

pthread_t t1, t2;
pthread_mutex_t lock1, lock2;

unsigned long long sum1 = 0;
unsigned long long sum2 = 0;

void *foo1(void *_) {
    int i, j;
    for (i = 0; i < 20000; i += 1) {
        pthread_mutex_lock(&lock1);
        for (j = 0; j < 2000; j += 1) {
            sum1 += 1;
        }
        pthread_mutex_unlock(&lock1);
        pthread_mutex_lock(&lock2);
        for (j = 0; j < 2000; j += 1) {
            sum2 += 1;
        }
        pthread_mutex_unlock(&lock2);
    }
    printf("thread1 sum1: %llu\n", sum1);
    printf("thread1 sum2: %llu\n", sum2);
    return NULL;
}

void *foo1(void *_) {
    int i, j;
    for (i = 0; i < 20000; i += 1) {
        pthread_mutex_lock(&lock1);
        for (j = 0; j < 2000; j += 1) {
            sum1 += 1;
        }
        pthread_mutex_unlock(&lock1);
        pthread_mutex_lock(&lock2);
        for (j = 0; j < 2000; j += 1) {
            sum2 += 1;
        }
        pthread_mutex_unlock(&lock2);
    }
    printf("thread2 sum1: %llu\n", sum1);
    printf("thread2 sum2: %llu\n", sum2);
    return NULL;
}

int main(void) {
    pthread_mutex_init(&lock1, NULL);
    pthread_mutex_init(&lock2, NULL);
    pthread_create(&t1, NULL, &foo1, NULL);
    pthread_create(&t2, NULL, &foo2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_destroy(&lock1);
    pthread_mutex_destroy(&lock2);
    return 0;
}
