#include <unistd.h>

int main(void) {
    char *argv[10];
    argv[0] = "/bin/ls";
    argv[1] = NULL;
    execve(argv[0], argv, NULL);
    return 0;
}
