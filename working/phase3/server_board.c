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
#define LOCAL_BUFLEN 50


#define CMD_VALIDATION     1
#define CMD_REGISTRATION   2
#define SUCCESS            1
#define FAILURE           -1

#define CAMERA_DEV_NAME       "/dev/video1"	

#define NUMBER_OF_BUFFERS      4


#define CLEAR(x) memset (&(x), 0, sizeof (x))
int datas = 0;

struct camera_buffer {
     void *     start;
     size_t     length;
};

static char *           dev_name        = NULL;
static int              fd              = -1;
struct camera_buffer    **camera_buffers       = NULL;
static unsigned int     n_buffers       = 0;
#define  VIDEO_WIDTH         512;
#define  VIDEO_HEIGHT        480;
static int video_depth=8;

/* Create a thread */
pthread_t threads[NTHREADS];
int thread_args[NTHREADS];
    

int sockets[2],capture_event,itrs,ret;
char itr[BUFLEN],t_buff[LOCAL_BUFLEN],s_buf[BUFLEN],buffers[BUFLEN][BUFLEN];
void readfromfile();
void writetofile();
static int init_mmap(void);
static int read_frame(void);
static int init_device(void);
static void mainloop();
void process_image(char *in_buffer, unsigned int len, char *out_buffer, unsigned int out_len);
static void errno_exit(const char *s);

/********************************************************************
* NAME : void diep(char *s)
*
*DESCRIPTION:
*	For error handling
*
********************************************************************/

int diep(char *s)
{
    perror(s);
    return FAILURE;
}

#if 1
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
               memset(&buffers[index][0],data,200);
               sprintf(t_buff, "%d\n", index);
               // * It writes the data to IPC socket *
               write(sockets[1], t_buff, 200);
               printf("ZS1:IPC data %s %d %d\n", t_buff,data,index);
               index=((index+1)%itrs);
               data++;
          }
     }
}
#endif
    
int main()
{
    struct sockaddr_in si_me, si_other;
    int s, i=1, slen=sizeof(si_other),c_index,count,cmds;
    char buf[LOCAL_BUFLEN];
  
    /* Create a pipe */ 
    pipe(sockets);	

    /* Create a socket */
    if ((s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))==-1)
        return diep("socket");

    memset((char *) &si_me, 0, sizeof(si_me));
    si_me.sin_family = AF_INET;
    si_me.sin_port = htons(PORT);
    si_me.sin_addr.s_addr = htonl(INADDR_ANY);

    /* Socket s should be bound to the address in si_me */	
    if (bind(s, (struct sockaddr *)&si_me, sizeof(si_me))==-1)
        return diep("bind");

    thread_args[i] = i;
	
    pthread_create(&threads[i], NULL, thread1/*mainloop*/, (void *) &thread_args[i]);

	
#ifndef __SIMULATE__
    ret = open_device();

	if(ret == FAILURE)
	{
		return diep("camera device open error");
	}
	
	ret = init_device();
	if(ret == FAILURE)
	{
		return diep("camera init device error");
	}

	ret = init_mmap();
	if(ret == FAILURE)
	{
		return diep("camera init device error");
	}

#endif

    while(1)
    {    	
        printf("Waiting for USER...\n");

        /* Receive a packet from s, that the data should be put into buf */
        if (recvfrom(s, buf, LOCAL_BUFLEN, 0, (struct sockaddr *)&si_other, &slen)==-1)
		{
            continue ;
		}

		/* Convert a string to an integer */
		cmds = atoi(buf);
		
        /* Receive a packet from s, that the data should be put into itr */
        if (recvfrom(s, buf, LOCAL_BUFLEN, 0, (struct sockaddr *)&si_other, &slen)==-1)
            continue;
        
		printf("Option: %s Capture: %s\n",buf,itr);
   
        /* Convert a string to an integer */
        itrs = atoi(buf);
        
        switch (cmds) 
        {
            case CMD_VALIDATION:
                printf("Validation\n");
                capture_event=1;
#ifdef __SIMULATE__
                for(count=0;count<itrs;count++)
                {
                    /* It reads the data from IPC socket */
                    read(sockets[0], t_buff, 200);
                    c_index = atoi(t_buff);
                    
                    //deep process_image (buffers[c_index][0].start);
                    sprintf(s_buf, "%d\n", buffers[c_index][0]);
                    printf("Sending data %s\n\n",s_buf);
                        
					/* Send BUFLEN bytes from s_buf to s */
                    if (sendto(s, s_buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
					{
                        diep("sendto()");
						continue;
					}
                }
                readfromfile();
#else
                for(count=0;count<itrs;count++)
                {
                    /* It reads the data from IPC socket */
                    read(sockets[0], t_buff, 200);
                    c_index = atoi(t_buff);
					
					/* Proccess the camera image */
                   process_image((char *)camera_buffers[c_index].start, camera_buffers[c_index].len
					    out_buffer, &out_buffer_len);
					
					/* Send Processed data to the HOST */
                    if (sendto(s, out_buffer, out_buffer_len, 0, (struct sockaddr *)&si_other, slen)==-1)
					{
                        diep("sendto()");
						continue;
					}
                }
#endif

                capture_event=0;
            break;

            case CMD_REGISTRATION:
                printf("Registeration\n");
                    capture_event=1;
#ifdef __SIMULATE__
                    for(count=0;count<itrs;count++)
                    {
      
                        /* It reads the data from IPC socket */
                        read(sockets[0], t_buff, 200);
                        c_index = atoi(t_buff);

						//deep process_image (buffers[c_index][0].start);
                        sprintf(s_buf, "%d\n", buffers[c_index][0]);
                        printf("Sending data %s\n\n",s_buf);

                        /* Send BUFLEN bytes from s_buf to s */
                        if (sendto(s, s_buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
						{
                            diep("sendto()");
							continue;
						}
                    }
#else
                    for(count=0;count<itrs;count++)
                    {
      
                        /* It reads the data from IPC socket */
                        read(sockets[0], t_buff, 200);
                        c_index = atoi(t_buff);

                        printf("Sending data %s\n\n",s_buf);

						/* Proccess the camera image */
       					registeration_image((char *)camera_buffers[c_index].start, camera_buffers[c_index].len
					    out_buffer, &out_buffer_len);

                        /* Send BUFLEN bytes from s_buf to s */
                        if (sendto(s, s_buf, BUFLEN, 0, (struct sockaddr *)&si_other, slen)==-1)
                        {
						    diep("sendto()");
							continue;
						}
                    }
                    writetofile(out_buffer, out_buffer_len);
#endif
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

void readfromfile(char *buffer, unsigned short len)
{
     FILE *fp;

     /* Open file for reading */
     fp = fopen("ref.txt", "r");

     /* Read and display data */
     fread(buffer, len , 1, fp);
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

void writetofile(char *buffer, unsigned short len)
{
     FILE *fp;
     char c[] = "Zumi solutions";
     char buffer[100];

     /* Open file for writing */
    fp = fopen("ref.txt", "wb+");

	 
     /* Write data to the file */
    fwrite(buffer, len, 1, fp);
    
	fclose(fp);
}
#if 0

/********************************************************************
* NAME : void open_device(void)
*
* DESCRIPTION : 
*       
*
********************************************************************/

static int open_device(void)
{
     struct stat st;

     printf("%s: ZS1:1\n",__func__);

     fd = open (CAMERA_DEV_NAME, O_RDWR /* required */ | O_NONBLOCK, 0);

     if (-1 == fd) 
     {
          fprintf (stderr, "Cannot open '%s': %d, %s\n",
          dev_name, errno, strerror (errno));
		  return FAILURE;
     }
	 return SUCCESS;
}

/********************************************************************
* NAME : void init_device(void)
*
* DESCRIPTION : 
*       
*
********************************************************************/

static int init_device(void)
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
			return FAILURE;
	    } 
        else 
		{
            fprintf (stderr, "VIDIOC_QUERYCAP V4L2 device error\n");
			return FAILURE;
		}
     }
 
     if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) 
     {
          fprintf (stderr, "%s is no video capture device\n", dev_name);
          return FAILURE;
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
     fmt.fmt.pix.width       = VIDEO_WIDTH;//640; 
     fmt.fmt.pix.height      = VIDEO_HEIGHT;//480;
     fmt.fmt.pix.pixelformat = V4L2_PIX_FMT_YUYV;
     fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED; // Zumi need to be checked

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

static int init_mmap(void)
{
     struct v4l2_requestbuffers req;

     CLEAR (req);
     req.count               = NUMBER_OF_BUFFERS;
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

    camera_buffers = calloc (req.count, sizeof (*buffers));

    if (!camera_buffers) 
    {
        fprintf (stderr, "Out of memory\n"); 
        return FAILURE;
    }

     for (n_buffers = 0; n_buffers < req.count; ++n_buffers) 
     {
        struct v4l2_buffer buf;

        CLEAR (buf);

        buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        buf.memory      = V4L2_MEMORY_MMAP;
        buf.index       = n_buffers;

        if (-1 == ioctl (fd, VIDIOC_QUERYBUF, &buf))
		{
		    fprintf (stderr, "VIDIOC_QUERYBUF\n"); 
			return FAILURE;
        }       
	

        camera_buffers[n_buffers].length = buf.length;
        camera_buffers[n_buffers].start =
            mmap (NULL /* start anywhere */,
                    buf.length,
                    PROT_READ | PROT_WRITE /* required */,
                    MAP_SHARED /* recommended */,
                    fd, buf.m.offset);

        if (MAP_FAILED == camera_buffers[n_buffers].start)
		{
		    fprintf (stderr, "mmap error\n"); 
			return FAILURE;
        }
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
				fprintf (stderr, "camera device select Error\n");
                return FAILURE;
            }

            if (0 == r) 
            {
                fprintf (stderr, "camera device select timeout\n");
                return FAILURE;
            }
            if (read_frame() == FAILURE)
                break;
        } 
    }
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
    
	/* Send the capture buffer index to the image processing thread */
	if (capture_event==1)
    {
        sprintf(t_buff, "%d\n", buf.index);

        /* It writes the data to IPC socket */
        write(sockets[1], t_buff, BUFLEN);
    }

	/* Again Queue the buffer into V4L2 */
	if (-1 == ioctl (fd, VIDIOC_QBUF, &buf))
    {
        fprintf (stderr, "VIDIOC_QBUF Error\n");
		return FAILURE;
	}

    return SUCCESS;
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

void process_image(char *in_buffer, unsigned int len, char *out_buffer, unsigned int out_len)
{
}
#endif

