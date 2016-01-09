#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int main()
{
int sockets[2], child;
        char dat[4] ={1,2,3,4};

if (pipe(sockets) < 0) {
        printf("ZS:1\n");
        perror("opening stream socket pair");
        exit(10);
    }

if ((child = fork()) == -1)
        perror("fork");
    else if (child) {
        char buf[1024];
        printf("ZS:3\n");
        /*  This is still the parent.
            It reads the child's message. */
	close(sockets[1]);
	if (write(sockets[1], dat, sizeof(dat)) < 0)
            perror("writing message");printf("ZS:6\n");
        close(sockets[1]);

	if (read(sockets[0], buf, sizeof(buf)) < 0)
            perror("reading message");printf("ZS:4\n");
        printf("-->%s\n", buf);
        close(sockets[0]);

    }
}

