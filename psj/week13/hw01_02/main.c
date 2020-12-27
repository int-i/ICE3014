#include <stdio.h>
#include <stdlib.h>

int x1;

int main(void) {
    int *pk1 = (int *) malloc(sizeof(int));
    printf("hw01_02. &main: %p, &x1: %p\n", main, &x1);
    for (;;);
    return 0;
}
