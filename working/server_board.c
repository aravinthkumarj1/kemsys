//server_board
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
     
#define BUFLEN 512
//#define NPACK 10
#define PORT 9930
    
void diep(char *s)
	{
	perror(s);
	exit(1);
	}
    
int main(void)
	{
	struct sockaddr_in si_me, si_other;
	int s, i, slen=sizeof(si_other);
	char buf[BUFLEN],itr[BUFLEN];
	char buffer[200][200];
  	
	pipe(sockets);
	child = fork();

	if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	diep("socket");
    
	memset((char *) &si_me, 0, sizeof(si_me));
	si_me.sin_family = AF_INET;
	si_me.sin_port = htons(PORT);
	si_me.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if (bind(s, &si_me, sizeof(si_me))==-1)
	diep("bind");
    	
	while(1)	
	{
	sleep(2);
	
	printf("Waiting for USER...\n");
	if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
	diep("recvfrom()");
	 int count,data,capture_event,cmds,itrs,index=0;
        if (recvfrom(s, itr, BUFLEN, 0, &si_other, &slen)==-1)
        diep("recvfrom()");
        printf("Option: %s Capture: %s\n",buf,itr);

        cmds = atoi(buf);
        itrs = atoi(itr);
switch (cmds) {
        case 1:
                printf("Validation\n");
                capture_event=1;
                for(count=0;count<itrs;count++)
                {
                if(capture_event==1)
                {
                memset(&buffer[index][0],data,200);
                data++;
                //gevent = 1;
                }
                printf("Sending data %d\n\n", buffer[index][0]);
                sprintf(buf, "%d\n", buffer[index][0]);
                if (sendto(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
                diep("sendto()");
                index=((index+1)%itrs);
                }
                capture_event=0;
                break;

        case 2:
                printf("Registeration\n");
                capture_event=1;
                for(count=0;count<itrs;count++)
                {
                if(capture_event==1)
                {
                memset(&buffer[index][0],data,200);
                data++;
               // gevent = 1;
                }
                printf("Sending data %d\n\n", buffer[index][0]);
		sprintf(buf, "%d\n", buffer[index][0]);
                if (sendto(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
                diep("sendto()");
		index=((index+1)%itrs);
                }
                capture_event=0;
                break;
}
/*	
	if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
        diep("sendto()");
	
	int buffer;
	buffer= atoi(buf);
	switch (buffer) {
		
		case 11:
			printf("Validation \n5 times it need to capture \n");
			int j;
			for(j=0;j<5;j++)
			{
			if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
        		diep("recvfrom()");
        		printf("Kumar-Received packet from %s:%d\n Validation-Data: %s\n\n",
                		inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf); 
			
			if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
                        diep("sendto()");
			}
			break;
		case 12:
			printf("Registeration \n1 time it need to capture \n");

			if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
		        diep("recvfrom()");
        		printf("Kumar-Received packet from %s:%d\n Registeration-Data: %s\n\n",
                		inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
			if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
        		diep("sendto()");	
			break;
	}

 */	}
	close(s);
	return 0;
	}
