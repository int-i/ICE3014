#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>

int A[8192][8192];

double getUnixTime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (tv.tv_sec + tv.tv_usec / 1.0e6);
}

int main(void) {
    int i, j;
    double start = getUnixTime();
    syscall(31);
    for (i = 0; i < 8192; i += 1) {
        for (j = 0; j < 8192; j += 1) {
            A[j][i] = 3;
        }
    }
    syscall(31);
    double end = getUnixTime();
    double elapsed = end - start;
    printf("the elapsed time: %f\n", elapsed);
    return 0;
}
