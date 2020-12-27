#include <pthread.h>
#include <stdio.h>

void *foo(void *_) {
    printf("hello from child\n");
    return NULL;
}

int main(void) {
    pthread_t thread;
    pthread_create(&thread, NULL, foo, NULL);
    printf("hello from parent\n");
    return 0;
}
