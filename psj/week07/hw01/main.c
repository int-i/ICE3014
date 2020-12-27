#include <stdio.h>
#include <unistd.h>

unsigned long long sum = 0;

int main(void) {
    if (fork() == 0) { // child
        int i, j;
        for (i = 0; i < 20000; i += 1) {
            for (j = 0; j < 2000; j += 1) {
                sum += 1;
            }
        }
        printf("child sum: %llu\n", sum);
    } else { // parent
        int i, j;
        for (i = 0; i < 20000; i += 1) {
            for (j = 0; j < 2000; j += 1) {
                sum += 1;
            }
        }
        printf("“parent sum: %llu\n", sum);
    }
    return 0;
}
