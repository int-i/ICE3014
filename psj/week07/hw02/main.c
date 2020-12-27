#include <pthread.h>
#include <stdio.h>

pthread_t t1, t2;
unsigned long long sum = 0;

void *foo1(void *_) {
    int i, j;
    for (i = 0; i < 20000; i += 1) {
        for (j = 0; j < 2000; j += 1) {
            sum += 1;
        }
    }
    printf("thread1 sum: %llu\n", sum);
    return NULL;
}

void *foo2(void *_) {
    int i, j;
    for (i = 0; i < 20000; i += 1) {
        for (j = 0; j < 2000; j += 1) {
            sum += 1;
        }
    }
    printf("thread2 sum: %llu\n", sum);
    return NULL;
}

int main(void) {
    pthread_create(&t1, NULL, foo1, NULL);
    pthread_create(&t2, NULL, foo2, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}
