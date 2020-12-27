#include <unistd.h>

int main(void) {
    fork();
    fork();
    for (;;);
    return 0;
}
