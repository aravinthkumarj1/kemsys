#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define DATA "Bright star, would I were steadfast as thou art..."

/*
 *  This program creates a pipe, then forks.
 *  The child communicates to the parent over the pipe.
 *  Notice that a pipe is a one-way communications
 *  device. I can write to the output socket  
 *  (sockets[1], the second socket of the array returned 
 *  by  pipe()) and  read from the input socket (sockets[0]), 
 *  but not vice versa.
 */

main()
{
    int sockets[2], child;

    /* Create a pipe */
    if (pipe(sockets) < 0) {
        perror("opening stream socket pair");
        exit(10);
    }

    if ((child = fork()) == -1)
        perror("fork");
    else if (child) {
        char buf[1024];
	printf("ZS:1");
        /*  This is still the parent.
            It reads the child's message. */
        close(sockets[1]);printf("zs:2");
        if (read(sockets[0], buf, sizeof(buf)) < 0){
            perror("reading message");printf("zs:3");}
        printf("-->%s\n", buf);
        close(sockets[0]);
    } else {
        /* This is the child.
           It writes a message to its parent. */
        close(sockets[0]);printf("ZS:4");
        if (write(sockets[1], DATA, sizeof(DATA)) < 0){
            perror("writing message");printf("ZS:5");}
        close(sockets[1]);printf("ZS:6");
    }
}
