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
#include <linux/videodev2.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <assert.h>
#include <getopt.h> 
#include <fcntl.h>
#include <asm/types.h> 
#include <sys/select.h>

#define BUFLEN 1024
#define PORT 9930
#define NTHREADS 1024

#define CLEAR(x) memset (&(x), 0, sizeof (x))
//char data[3][640*480*2];
int datas = 0;

struct buffer {
     void *     start;
     size_t     length;
     size_t     data;
};

static char *           dev_name        = NULL;
static int              fd              = -1;
struct buffer **buffers         = NULL;
static unsigned int     n_buffers       = 0;
static int video_width=512;
static int video_height=480;
static int video_depth=8;


int sockets[2],capture_event,itrs;
char itr[BUFLEN],t_buff[BUFLEN],s_buf[BUFLEN]/*,buffers[BUFLEN][BUFLEN]*/;
void readfromfile();
void writetofile();
static void init_mmap(void);
static int read_frame(void);
static void init_device(void);
static void mainloop();
void process_image();
static void errno_exit(const char *s);

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
/*
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

//               * It writes the data to IPC socket *
               write(sockets[1], t_buff, 200);
               printf("ZS1:IPC data %s %d %d\n", t_buff,data,index);
               index=((index+1)%itrs);
               data++;
          }
     }
}
*/
    
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
     pthread_create(&threads[i], NULL, mainloop, (void *) &thread_args[i]);

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
                         process_image (buffers[c_index][0].start);
                         sprintf(s_buf, "%d\n", buffers[c_index][0]);
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
                         process_image (buffers[c_index][0].start);
                         sprintf(s_buf, "%d\n", buffers[c_index][0]);
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

/********************************************************************
* NAME : void init_device(void)
*
* DESCRIPTION : 
*       
*
********************************************************************/

static void init_device(void)
{
     struct v4l2_capability cap;
     struct v4l2_cropcap cropcap;
     struct v4l2_crop crop;
     struct v4l2_format fmt;
     unsigned int min;
     unsigned int i;
     enum v4l2_buf_type type;

     memset(&cap,0,sizeof(struct v4l2_capability));
     if (-1 == ioctl (fd, VIDIOC_QUERYCAP, &cap)) 
     {
          if (EINVAL == errno) 
          {
               fprintf (stderr, "%s is no V4L2 device\n", dev_name);
               exit (EXIT_FAILURE);
          } 
          else 
               errno_exit ("VIDIOC_QUERYCAP");
     }
 
     if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
     {
          fprintf (stderr, "%s is no video capture device\n", dev_name);
          exit (EXIT_FAILURE);
     }

     /* Select video input, video standard and tune here. */
     CLEAR (cropcap);
     cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     if (0 == ioctl (fd, VIDIOC_CROPCAP, &cropcap)) 
     {
          crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          crop.c = cropcap.defrect; /* reset to default */
          if (-1 == ioctl (fd, VIDIOC_S_CROP, &crop)) 
          {
               switch (errno) 
               {
                    case EINVAL:

                         /* Cropping not supported. */
                         break;

                    default:

                         /* Errors ignored. */
                         break;
               }
          }
     } 
     else 
     {	
          /* Errors ignored. */
          fprintf(stdout, "%s does not support VIDIOC_CROPCAP!!!\n", dev_name);
     }

     CLEAR (fmt);

     fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     //fmt.fmt.pix.width       = HRT_DEFAULT_WIDTH;//640; 
     //fmt.fmt.pix.height      = HRT_DEFAULT_HEIGHT;//480;
     fmt.fmt.pix.width       = video_width;//640; 
     fmt.fmt.pix.height      = video_height;//480;
     //hrtview.depth  =          video_depth;
     fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
     fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;

     if (-1 == ioctl (fd, VIDIOC_S_FMT, &fmt))
          errno_exit ("VIDIOC_S_FMT");

     /* Note VIDIOC_S_FMT may change width and height. */

     /* Buggy driver paranoia. */
     min = fmt.fmt.pix.width * 2;
     if (fmt.fmt.pix.bytesperline < min)
          fmt.fmt.pix.bytesperline = min;

     min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
     if (fmt.fmt.pix.sizeimage < min)
          fmt.fmt.pix.sizeimage = min;

     init_mmap ();
     fprintf(stdout, "%s is in memory mapping mode!\n", dev_name);
    
     /* start capturing */ 
     for (i = 0; i < n_buffers; ++i)
     {
          struct v4l2_buffer buf;

          CLEAR (buf);

          buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          buf.memory      = V4L2_MEMORY_MMAP;
          buf.index       = i;

          if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))
               errno_exit ("VIDIOC_QBUF ... !!!");
     }

     type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     if (-1 == ioctl (fd, VIDIOC_STREAMON, &type))
          errno_exit ("VIDIOC_STREAMON");
}

/********************************************************************
* NAME : void init_mmap(void)
*
* DESCRIPTION : 
*      Set the number of buffer 
*
********************************************************************/

static void init_mmap(void)
{
     struct v4l2_requestbuffers req;

     CLEAR (req);
     req.count               = 4;
     req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     req.memory              = V4L2_MEMORY_MMAP;

     if (-1 == ioctl (fd, VIDIOC_REQBUFS, &req)) 
     {
          if (EINVAL == errno) 
          {
               fprintf (stderr, "%s does not support memory mapping\n", dev_name);
               exit (EXIT_FAILURE);
          } 
          else 
               errno_exit ("VIDIOC_REQBUFS");
     }

     if (req.count < 2) 
     {
     fprintf (stderr, "Insufficient buffer memory on %s\n", dev_name);
     exit (EXIT_FAILURE);
     }

     buffers = calloc (req.count, sizeof (*buffers));

     if (!buffers) 
     {
          fprintf (stderr, "Out of memory\n"); 
          exit (EXIT_FAILURE);
     }

     for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
     {
          struct v4l2_buffer buf;

          CLEAR (buf);

          buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
          buf.memory      = V4L2_MEMORY_MMAP;
          buf.index       = n_buffers;

          if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf))
               errno_exit ("VIDIOC_QUERYBUF");

          buffers[n_buffers][0].length = buf.length;
          buffers[n_buffers][0].start =
               mmap (NULL /* start anywhere */,
                    buf.length,
                    PROT_READ | PROT_WRITE /* required */,
                    MAP_SHARED /* recommended */,
                    fd, buf.m.offset);

          if (MAP_FAILED == buffers[n_buffers][0].start)
               errno_exit ("mmap");
     }
}

/********************************************************************
* NAME : void mainloop(void)
*
* DESCRIPTION : 
*       
*
********************************************************************/

static void mainloop()
{
     unsigned long val,count;
     time_t elapsed = time(NULL);
     val = count = 100;

     while (count-- > 0) 
     {
          for (;;) 
          {
               fd_set fds;
               struct timeval tv;
               int r;
               FD_ZERO (&fds);
               FD_SET (fd, &fds);
               
              // * Timeout. *
               tv.tv_sec = 2;
               tv.tv_usec = 0;
               
               r = select (fd + 1, &fds, NULL, NULL, &tv);
               if (-1 == r) 
               {
                    if (EINTR == errno)
                         continue;
                    errno_exit ("select");
               }
               if (0 == r) 
               {
                    fprintf (stderr, "select timeout\n");
                    exit (EXIT_FAILURE);
               }
              if (read_frame ())
                    break;
          } 
     }

     elapsed = time(NULL) - elapsed;
     fprintf(stdout, "The frame rate (per second) = %lu\n", val/elapsed);

}

/********************************************************************
* NAME : int read_frame(void)
*
* DESCRIPTION : 
*       Read frame from buffer
*
********************************************************************/

static int read_frame(void)
{
     struct v4l2_buffer buf;
     unsigned int i;

     CLEAR (buf);
     buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
     buf.memory = V4L2_MEMORY_MMAP;

     if (-1 == ioctl (fd, VIDIOC_DQBUF, &buf)) 
     {
          printf("ioctl dqbuf is wrong !!!\n");
          switch (errno) 
          {
               case EAGAIN:
                    return 0;
               case EIO:
                    /* Could ignore EIO, see spec. */
                    /* fall through */
               default:
                    errno_exit ("VIDIOC_DQBUF");
          }
     }

     assert (buf.index < n_buffers);
     printf("index = %d\n",buf.index);
     if (capture_event==1)
     {
          memset(&buffers[buf.index][0],datas,BUFLEN);
          sprintf(t_buff, "%d\n", buf.index);

          /* It writes the data to IPC socket */
          write(sockets[1], t_buff, BUFLEN);
          printf("ZS1:IPC data %s %d %d\n", t_buff,datas,buf.index);
     }

     //process_image (buffers[buf.index].start);

     if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))
          errno_exit ("VIDIOC_QBUF");

     return 1;
}

/********************************************************************
* NAME : static void errno_exit(const char *s)
*
* DESCRIPTION : 
*	Error handling       
*
********************************************************************/

static void errno_exit(const char *s)
{
        fprintf (stderr, "%s error %d, %s\n",
                 s, errno, strerror (errno));

        exit (EXIT_FAILURE);
}

/********************************************************************
* NAME : void process_image()
*
* DESCRIPTION : 
*       
*
********************************************************************/

void process_image()
{
}
