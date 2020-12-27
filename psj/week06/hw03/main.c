#include <stdio.h>
#include <unistd.h>

int main(void) {
    int i;
    float y = 3.14;
    fork();
    fork();
    for (;;) {
        for (i = 0; i < 1000000000; i += 1) {
            y *= 0.4;
        }
        printf("pid: %d\n", getpid());
    }
    return 0;
}
