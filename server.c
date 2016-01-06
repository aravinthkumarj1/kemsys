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
	char buf[BUFLEN];
    
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
	printf("Waiting for USER...\n");
	if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
	diep("recvfrom()");
	printf("Kumar-Received packet from %s:%d\nData: %s\n\n", 
		inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
	int buffer;
	buffer= *buf;
	printf("Buffer is %d\n",buffer);
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
			}
			break;
		case 12:
			printf("Registeration \n1 time it need to capture \n");

			if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
		        diep("recvfrom()");
        		printf("Kumar-Received packet from %s:%d\n Registeration-Data: %s\n\n",
                		inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
			break;
	}

	if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
	diep("sendto()");
 	}
 
	close(s);
	return 0;
	}
