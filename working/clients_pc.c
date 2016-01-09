//client_PC
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
         	char buf[BUFLEN],cmd[BUFLEN],capture[BUFLEN];
     
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
	int i=0,count;
	char *data[10];
	char *captur[10];
	printf("Select the below option and Enter capture iteration\n1.Validation\n2.Registeration\n");
	scanf("%d%d",&data,&captur);
	sprintf(cmd, "%d\n", data[i]);
	if (sendto(s, cmd, BUFLEN, 0, &si_other, slen)==-1)
	diep("sendto()");
        sprintf(capture, "%d\n", captur[i]);
	if (sendto(s, capture, BUFLEN, 0, &si_other, slen)==-1)
        diep("sendto()");
	printf("Sending packet.. \n");

 	int command,itrs;
	command = atoi(cmd);
	itrs = atoi(capture);
	switch (command) {

                case 1:
                        printf("Validation\n");
                        int j;
                        for(count=0;count<itrs;count++)
                        {
                        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)==-1)
                        diep("recvfrom()");
                        printf("Validation-Data: %s\n\n",buf);
                        }
                        break;

                case 2:
                        printf("Registeration\n");
                        for(count=0;count<itrs;count++)
                        {
                        if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)==-1)
                        diep("recvfrom()");
                        printf("Registeration-Data: %s\n\n",buf);
                        }
                        break;
        }
        
	}

    	close(s);
    	return 0;
  }
