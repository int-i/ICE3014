#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    char pathname[50];
    char *argv[2];

    for (;;) {
        printf("$ ");
        scanf("%s", pathname);
        argv[0] = pathname;
        argv[1] = NULL;

        if (fork() == 0) { // child
            printf("I am child to execute %s\n", pathname);
            if (execve(pathname, argv, NULL) < 0) {
                perror("exec");
                exit(EXIT_FAILURE);
            }
        } else {
            wait(NULL);
        }
    }
    return 0;
}
