#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int main()
{
    int sockets[2], child;
	char buf[1024],buff[1024];
	const char itr[2] ={4,2};
	int data = 2;
    /* Create a pipe */
	pipe(sockets);
	child = fork();
	while(1)
{
	printf("zs:6");
	break;	
}
if (child)
	{
	read(sockets[0], buf, 200);
	int ha;
	ha = atoi(buf);
	printf("-->%d%d\n", ha,data);
	printf("ZS:3\n");
	}
	else
	{
	printf("zs:1\n");
	data++;
	sprintf(buf, "%d\n", data);
        write(sockets[1], buf, 200);
        printf("ZS:2 %s\n",buf);
	}
	exit(0);
	return 0;
}
