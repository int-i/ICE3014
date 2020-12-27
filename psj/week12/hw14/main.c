#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

int main(void) {
    char buf[10];
    int f1 = open("f1", O_RDONLY);
    int f2 = open("f2", O_RDONLY);
    printf("f1 and f2 are %d %d\n", f1, f2);
    if (fork() == 0) {
        syscall(17);
        read(f1, buf, 1);
        sleep(2);
        syscall(17);
        write(f2, buf, 1);
    } else {
        sleep(1);
        syscall(17);
        read(f1, buf, 1);
        write(f2, buf, 1);
    }
    return 0;
}
