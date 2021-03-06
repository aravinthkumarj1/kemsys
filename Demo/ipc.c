#include <stdio.h>
#include <stdlib.h>
void child(int socket) {
    const char hello[] = "hello parent, I am child";
    write(socket, hello, sizeof(hello)); /* NB. this includes nul */
    /* go forth and do childish things with this end of the pipe */
}

void parent(int socket) {
    /* do parental things with this end, like reading the child's message */
    char buf[1024];
    int n = read(socket, buf, sizeof(buf));
    printf("parent received '%.*s'\n", n, buf);
}

void socketfork() {
    int fd[2];
    static const int parentsocket = 0;
    static const int childsocket = 1;
    pid_t pid;

    /* 1. call socketpair ... */
    socketpair(PF_LOCAL, SOCK_STREAM, 0, fd);

    /* 2. call fork ... */
    pid = fork();
    if (pid == 0) { /* 2.1 if fork returned zero, you are the child */
        close(fd[parentsocket]); /* Close the parent file descriptor */
        child(fd[childsocket]);
    } else { /* 2.2 ... you are the parent */
        close(fd[childsocket]); /* Close the child file descriptor */
        parent(fd[parentsocket]);
    }
    exit(0); /* do everything in the parent and child functions */
}
