#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    char buf[100];
    int x = open("/aa/bb", O_RDONLY);
    int y = open("/aa/newbb", O_RDONLY);
    syscall(17);
    read(x, buf, 10);
    read(y, buf, 10);
    syscall(17);
    return 0;
}
