#include <stdio.h>
#include <unistd.h>

int A[5][1024];

int main(void) {
    int i, j;
    printf("&main: %p, &A: %p, &A[5][1024]: %p, &i: %p, &j: %p", main, &A, &A[5][1024], &i, &j);
    syscall(31);
    for (i = 0; i < 5; i += 1) {
        for (j = 0; j < 1024; j += 1) {
            A[i][j] = 3;
        }
    }
    for(;;);
    return 0;
}
