/*H***********************************************************************************
* FILENAME : server_board.c
*
* DESCRIPTION :	
*	Receive command from client, Corresponding data will sent to client.
*
* PUBLIC FUNCTIONS :
*	void diep(char *s)
*	void thread1()
*	int readfromfile()
*	int writetofile()
*
* AUTHOR : Zumi Solutions (P) Ltd.,
*
*H*/

#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
//#include <pthread.h>
     
#define BUFLEN 1024
#define PORT 9930
#define NTHREADS 1024

int sockets[2],capture_event,itrs;
char itr[BUFLEN],t_buff[BUFLEN],s_buf[BUFLEN],buffer[BUFLEN][BUFLEN];
void readfromfile();
void writetofile();

/********************************************************************
* NAME : void diep(char *s)
*
*DESCRIPTION:
*	For error handling
*
********************************************************************/

void diep(char *s)
{
     perror(s);
     exit(1);
}

/********************************************************************
* NAME : void thread1()
*
*DESCRIPTION:	
*	To check capture_event every 2sec.
*	Captured data [index] write to IPC socket.
*
********************************************************************/

void thread1()
{
     int data=1,index=0;
     while(1)
     {
          sleep(2);
          if (capture_event==1)
          {
               memset(&buffer[index][0],data,200);
               sprintf(t_buff, "%d\n", index);

               /* It writes the data to IPC socket */
               write(sockets[1], t_buff, 200);
               printf("ZS1:IPC data %s %d %d\n", t_buff,data,index);
               index=((index+1)%itrs);
               data++;
          }
     }
}

    
int main()
{
     struct sockaddr_in si_me, si_other;
     int s, i=1, slen=sizeof(si_other),c_index,count,cmds;
     char buf[BUFLEN];
  
     /* Create a pipe */ 
     pipe(sockets);	

     /* Create a socket */
     if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
          diep("socket");

     memset((char *) &si_me, 0, sizeof(si_me));
     si_me.sin_family = AF_INET;
     si_me.sin_port = htons(PORT);
     si_me.sin_addr.s_addr = htonl(INADDR_ANY);

     /* Socket s should be bound to the address in si_me */	
     if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me))==-1)
          diep("bind");

     /* Create a thread */
     pthread_t threads[NTHREADS];
     int thread_args[NTHREADS];
     thread_args[i] = i;
     pthread_create(&threads[i], NULL, thread1, (void *) &thread_args[i]);

     while(1)
     {    	
          printf("Waiting for USER...\n");

          /* Receive a packet from s, that the data should be put into buf */
          if (recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)==-1)
               diep("recvfrom()");

          /* Receive a packet from s, that the data should be put into itr */
          if (recvfrom(s, itr, BUFLEN, 0, (struct sockaddr *)&si_other, &slen)==-1)
               diep("recvfrom()");
          printf("Option: %s Capture: %s\n",buf,itr);
   
          /* Convert a string to an integer */
          cmds = atoi(buf);
          itrs = atoi(itr);
        
          switch (cmds) 
          {
               case 1:
                    printf("Validation\n");
                    capture_event=1;
                    for(count=0;count<itrs;count++)
                    {

                         /* It reads the data from IPC socket */
                         read(sockets[0], t_buff, 200);
                         c_index = atoi(t_buff);
                         sprintf(s_buf, "%d\n", buffer[c_index][0]);
                         printf("Sending data %s\n\n",s_buf);
                      
                         /* Send BUFLEN bytes from s_buf to s */
                         if (sendto(s, s_buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
                              diep("sendto()");
                    }
                    readfromfile();
                    capture_event=0;
                    break;

               case 2:
                    printf("Registeration\n");
                    capture_event=1;
                    for(count=0;count<itrs;count++)
                    {
      
                         /* It reads the data from IPC socket */
                         read(sockets[0], t_buff, 200);
                         c_index = atoi(t_buff);
                         sprintf(s_buf, "%d\n", buffer[c_index][0]);
                         printf("Sending data %s\n\n",s_buf);

                         /* Send BUFLEN bytes from s_buf to s */
                         if (sendto(s, s_buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
                              diep("sendto()");
                    }
                    writetofile();
                    capture_event=0;
                    break;
          }
     }
     close(s);
     return 0;
}

/********************************************************************
* NAME : void readfromfile()
*
* DESCRIPTION : 
*	Print the data from file
*
********************************************************************/

void readfromfile()
{
     FILE *fp;
     char buffer[10000];

     /* Open file for reading */
     fp = fopen("ref.txt", "r");

     /* Read and display data */
     fread(buffer, sizeof(buffer), 1, fp);
     printf("%s\n", buffer);
     fclose(fp);
}

/********************************************************************
* NAME : void writetofile()
*
* DESCRIPTION : 
*	Write data to the file
*
********************************************************************/

void writetofile()
{
     FILE *fp;
     char c[] = "Zumi solutions";
     char buffer[100];

     /* Open file for writing */
     fp = fopen("ref.txt", "w");

     /* Write data to the file */
     fwrite(c, strlen(c) + 1, 1, fp);
     fclose(fp);
}

