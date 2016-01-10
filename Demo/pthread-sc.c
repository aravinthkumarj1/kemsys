#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define NTHREADS 2
#define BUFLEN 512
void *thread1(void *iter)
{
	int itrs,index=0,capture_event,data=0;
	char buf[BUFLEN];
	char buffer[BUFLEN][BUFLEN];
	int sockets[2];
	itrs = *((int *) iter);
	while(1)
	{
	printf("ZS:5");
	sleep(2);
	if (capture_event==1)
	{
	memset(&buffer[index][0],data,200);
	data++;
	index=((index+1)%itrs);
	sprintf(buf, "%d\n", buffer[index][0]);
	write(sockets[1], buf, 200);
	}
	}
	return 0;
}
int main()
{
printf("ZS:0");
  pthread_t threads[NTHREADS];
  int thread_args[NTHREADS];
  int rc, i,j,itrs,child;
	char buf[BUFLEN];
	int sockets[2];

	pipe(sockets);
        child = fork();
	printf("ZS:1");
      thread_args[i] = i;
      rc = pthread_create(&threads[i], NULL, thread1, (void *) &thread_args[i]);
	printf("ZS:2");
	for(j=0;j<9;j++){

	if (j==3)
        {
        read(sockets[0], buf, 200);
        int ha;
        ha = atoi(buf);
        printf("-->%d\n", ha);
        printf("ZS:3\n");
        }

	printf("Hai%d\n",j);
	
	sleep(2);
	}
   // }
//	int a;
//	scanf("%d",&a);
  /* wait for threads to finish */
//  for (i=0; i<9; ++i) {
  //  rc = pthread_join(threads[i], NULL);
 // }

  return 1;
}
