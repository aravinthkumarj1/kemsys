/*H***********************************************************************************
* FILENAME : client_pc.c
*
* DESCRIPTION : 
*       Sent command to client, Receive data from server
*
* PUBLIC FUNCTIONS :
*       void diep(char *s)
*       void thread1()
*       int readfromfile()
*       int writetofile()
*
* AUTHOR : Zumi Solutions (P) Ltd.,
*
*H*/

/* Server ip address */
#define SRV_IP "192.168.0.101"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#define BUFLEN 1024
#define PORT 9930

/********************************************************************
* NAME : void diep(char *s)
*
*DESCRIPTION:
*       For error handling
*
********************************************************************/
    
void diep(char *s)
{
     perror(s);
     exit(1);
}

 
int main()
{
     struct sockaddr_in si_other;
     int s, i, slen=sizeof(si_other);
     char buf[BUFLEN],cmd[BUFLEN],capture[BUFLEN];

     /* Create a socket */     
     if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
          diep("socket");
    
     memset((char *) &si_other, 0, sizeof(si_other));
     si_other.sin_family = AF_INET;
     si_other.sin_port = htons(PORT);
        
     /* convert a string in dotted-decimal ASCII notation to a binary address */
     if (inet_aton(SRV_IP, &si_other.sin_addr)==0) 
     {
          fprintf(stderr, "inet_aton() failed\n");
          exit(1);
     }
    
     while(1)
     {
          int i=0,count,command,itrs;
          char *data[1];
          char *captur[BUFLEN];
          printf("Select the below option and Enter capture iteration\n1.Validation\n2.Registeration\n");
          scanf("%d%d",&data,&captur);
          sprintf(cmd, "%d\n", data[i]);

          /* Send BUFLEN bytes from cmd to s */
          if (sendto(s, cmd, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
               diep("sendto()");
          sprintf(capture, "%d\n", captur[i]);

          /* Send BUFLEN bytes from capture to s */
          if (sendto(s, capture, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
               diep("sendto()");
          printf("Sending packet.. \n");

          /* Convert a string to an integer */
          command = atoi(cmd);
          itrs = atoi(capture);

          switch (command) 
          {
               case 1:
                    printf("Validation\n");
                    for(count=0;count<itrs;count++)
                    {

                         /* Receive a packet from s, that the data should be put into buf */
                         if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)==-1)
                              diep("recvfrom()");
                         printf("Validation-Data: %s\n\n",buf);
                    }
                    break;

               case 2:
                    printf("Registeration\n");
                    for(count=0;count<itrs;count++)
                    {

                         /* Receive a packet from s, that the data should be put into buf */
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
