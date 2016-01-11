#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
int sockets[2], child;
        char buf[1024];
        const char itr[2] ={4,5};
	
int main()
{
	pipe(sockets);
	child = fork();
        printf("zs:6");
/*if (child)
        {
        read(sockets[0], buf, 200);
        int ha;
        ha = atoi(buf);
        printf("-->%d\n", ha);
        printf("ZS:3\n");
        }
        else
        {
        printf("zs:1\n");
        sprintf(buf, "%d\n", itr[0]);
        write(sockets[1], buf, 200);
        printf("ZS:2\n");
        }*/
if (child)
{
	reads();
	}
	else{
//	writes();
}
        exit(0);
        return 0;
}

/*void writes()
{
        printf("zs:1\n");
        sprintf(buf, "%d\n", itr[0]);
        write(sockets[1], buf, 200);
        printf("ZS:2\n");
        }
*/
void reads()
{
        read(sockets[0], buf, 200);
        int ha;
        ha = atoi(buf);
        printf("-->%d\n", ha);
        printf("ZS:3\n");
        }
	

