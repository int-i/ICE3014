#include <stdio.h>
#include <unistd.h>

int y = 0;

int main(void) {
    int x = fork();
    if (x == 0) {
        y = y + 1;
        printf("process child: %d\n", y);
    } else {
        y = y + 2;
        printf("process parent: %d\n", y);
    }
    return 0;
}
