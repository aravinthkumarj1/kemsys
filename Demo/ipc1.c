#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    int sockets[2], child;
	char buf[1024];
	const char itr[] ="1";
    /* Create a pipe */
	pipe(sockets);
	child = fork();
	
if (child)
	{
	read(sockets[0], buf, sizeof(buf));
	printf("-->%s\n", buf);
	printf("ZS:3\n");
	}
	else
	{
	printf("zs:1\n");
        write(sockets[1], itr, sizeof(itr));
        printf("ZS:2\n");
	}
	exit(0);
	return 0;
}
