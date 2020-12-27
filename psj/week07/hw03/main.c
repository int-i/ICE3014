#include <pthread.h>
#include <stdio.h>

pthread_t t1, t2;
pthread_mutex_t lock;
unsigned long long sum = 0;

void *foo1(void *_) {
    int i, j;
    for (i = 0; i < 20000; i += 1) {
        pthread_mutex_lock(&lock);
        for (j = 0; j < 2000; j += 1) {
            sum += 1;
        }
        pthread_mutex_unlock(&lock);
    }
    printf("thread1 sum: %llu\n", sum);
    return NULL;
}

void *foo1(void *_) {
    int i, j;
    for (i = 0; i < 20000; i += 1) {
        pthread_mutex_lock(&lock);
        for (j = 0; j < 2000; j += 1) {
            sum += 1;
        }
        pthread_mutex_unlock(&lock);
    }
    printf("thread2 sum: %llu\n", sum);
    return NULL;
}

int main(void) {
    pthread_mutex_init(&lock, NULL);
    pthread_create(&t1, NULL, &foo1, NULL);
    pthread_create(&t2, NULL, &foo2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_destroy(&lock);
    return 0;
}
