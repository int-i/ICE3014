#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    char buf[100];
    int x = open("/aa/bb", O_RDONLY);
    int y;
    if (fork() == 0) {
        y = read(x, buf, 10);
        buf[y] = '\0';
        printf("child read %s\n", buf);
    } else {
        y = read(x, buf, 10);
        buf[y] = '\0';
        printf("parent read %s\n", buf);
    }
    return 0;
}
