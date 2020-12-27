#include <stdio.h>
#include <stdlib.h>

int x;
int y[10000];

int main(void) {
    int k;
    int *pk = (int *) malloc(sizeof(int));
    printf("hw01_01. &main: %p, &x: %p, &y: %p, &y[9999]: %p, &k: %p, &pk: %p, pk: %p\n", main, &x, &y, &y[9999], &k, &pk, pk);
    for (;;);
    return 0;
}
