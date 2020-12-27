#include <stdio.h>
#include <unistd.h>

int main(void) {
    int x = fork();
    printf("x: %d\n", x);
    return 0;
}
