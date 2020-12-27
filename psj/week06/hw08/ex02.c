#include <pthread.h>
#include <stdio.h>

int y = 0;

void *foo(void *_) {
    y = y + 1;
    printf("thread child: %d\n", y);
    return NULL;
}

int main(void) {
    pthread_t x;
    pthread_create(&x, NULL, foo, NULL);
    y = y + 2;
    printf("thread parent: %d\n", y);
    return 0;
}
