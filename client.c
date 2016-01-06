#define SRV_IP "192.168.0.101"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
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
		struct sockaddr_in si_other;
         	int s, i, slen=sizeof(si_other);
         	char buf[BUFLEN];
     
if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
	diep("socket");
    
	memset((char *) &si_other, 0, sizeof(si_other));
	si_other.sin_family = AF_INET;
	si_other.sin_port = htons(PORT);
        
if (inet_aton(SRV_IP, &si_other.sin_addr)==0) 
	{
		fprintf(stderr, "inet_aton() failed\n");
		exit(1);
        }
    
while(1)
	{
	int i=0;
	int *data[10];
	printf("Enter the data\n");
	scanf("%d",&data);
	printf("Sending packet %d\n", data[i]);
	sprintf(buf, "Packet from PC %d\n", data[i]);

	if (sendto(s, buf, BUFLEN, 0, &si_other, slen)==-1)
	diep("sendto()");
          
	if (recvfrom(s, buf, BUFLEN, 0, &si_other, &slen)==-1)
	diep("recvfrom()");
	printf("Aravinth-Received packet from %s:%d\nData: %s\n\n",
		inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port), buf);
        i++;
	}

    	close(s);
    	return 0;
  }
