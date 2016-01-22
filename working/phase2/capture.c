/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 */
#include <SDL/SDL.h>
#include <SDL/SDL_keysym.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>             /* getopt_long() */
#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>          /* for videodev2.h */
#include <linux/videodev2.h>
#include "../hrt.h"

#define CLEAR(x) memset (&(x), 0, sizeof (x))
char data[3][640*480*2];

typedef enum {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} io_method;

struct buffer {
        void *                  start;
        size_t                  length;
};

static char *           dev_name        = NULL;
static io_method	io		= IO_METHOD_MMAP;
static int              fd              = -1;
struct buffer *         buffers         = NULL;
static unsigned int     n_buffers       = 0;
static int video_width=512;
static int video_height=480;
static int video_depth=8;
#define HRT_DEVICE "/dev/video0"
#define HRT_DEFAULT_X 0
#define HRT_DEFAULT_Y 0

typedef struct _hrtview_t {
	char *hrt_device;		/* path name to the HRT device */
	int   fd;			/* file descriptor to the open HRT device */
	int   width, height, depth;	/* sdl info */
	int   flags;			/* for the display */
	SDL_Surface *surface;		/* the main window surface */
	int   frame_count;
	int   argc;
	char **argv;
	int   paused;
	int   interrupts;
	int   x, y;
	int   dragging;
	int   start_x, start_y;
	int   cur_x, cur_y;	/* mouse stuff */
} hrtview_t;

static hrtview_t hrtview = { 0 };

int init_params(int argc, char **argv)
{
	hrtview.fd     = fd;
	hrtview.frame_count = 0;
	hrtview.argc   = argc;
	hrtview.argv   = argv;
	hrtview.width  = video_width;
	hrtview.height = video_height;
	hrtview.depth  = video_depth;
	hrtview.x      = HRT_DEFAULT_X;
	hrtview.y      = HRT_DEFAULT_Y;
	hrtview.flags  = SDL_SWSURFACE | SDL_DOUBLEBUF;
	hrtview.interrupts = 1;
	hrtview.dragging = 0;
	if (argc > 2) return -1;
	if (argc == 2) hrtview.hrt_device = argv[1];
        else hrtview.hrt_device = (char *)strdup(HRT_DEVICE);
	return 0;
}

static void errno_exit(const char *s)
{
        fprintf (stderr, "%s error %d, %s\n",
                 s, errno, strerror (errno));

        exit (EXIT_FAILURE);
}

static int xioctl(int fd, int request, void *arg)
{
        int r;

        do r = ioctl (fd, request, arg);
        while (-1 == r && EINTR == errno);

        return r;
}

static void process_image(const void *p)
{
  update_frame(p);
  SDL_Delay(15);
}

static int read_frame(void)
{
  struct v4l2_buffer buf;
	unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
    		if (-1 == read (fd, buffers[0].start, buffers[0].length)) {
            		switch (errno) {
            		case EAGAIN:
                    		return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit ("read");
			}
		}

    		process_image (buffers[0].start);

		break;

	case IO_METHOD_MMAP:
		CLEAR (buf);
    buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;

  	if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
      printf("xioctl dqbuf is wrong !!!\n");
        switch (errno) {
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
	  process_image (buffers[buf.index].start);

		if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
			errno_exit ("VIDIOC_QBUF");
		break;

	case IO_METHOD_USERPTR:
    fprintf(stdout, "%s running in userpointer manner!\n", dev_name);
		CLEAR (buf);

    		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    		buf.memory = V4L2_MEMORY_USERPTR;

		if (-1 == xioctl (fd, VIDIOC_DQBUF, &buf)) {
			switch (errno) {
			case EAGAIN:
				return 0;

			case EIO:
				/* Could ignore EIO, see spec. */

				/* fall through */

			default:
				errno_exit ("VIDIOC_DQBUF");
			}
		}

		for (i = 0; i < n_buffers; ++i)
			if (buf.m.userptr == (unsigned long) buffers[i].start)
			   // && buf.length == buffers[i].length)
				break;


		assert (i < n_buffers);

    process_image ((void *) buf.m.userptr);

		if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
			errno_exit ("VIDIOC_QBUF");

		break;
	}

	return 1;
}

static void mainloop(void)
{
  unsigned long val,count;
  time_t elapsed = time(NULL);

  val = count = 100;

  while (count-- > 0) {
    for (;;) {
      fd_set fds;
      struct timeval tv;
      int r;
      
      FD_ZERO (&fds);
      FD_SET (fd, &fds);
      
      /* Timeout. */
      tv.tv_sec = 2;
      tv.tv_usec = 0;
      
      r = select (fd + 1, &fds, NULL, NULL, &tv);
      //      fprintf(stdout, "ret value of select is %d\n", r);
      //      fflush(0);
      
      if (-1 == r) {
        if (EINTR == errno)
          continue;
        errno_exit ("select");
      }

      if (0 == r) {
        fprintf (stderr, "select timeout\n");
        exit (EXIT_FAILURE);
      }
      if (read_frame ())
        break;
    } /* forr */
  }

  elapsed = time(NULL) - elapsed;
  fprintf(stdout, "The frame rate (per second) = %lu\n", val/elapsed);

}

static void stop_capturing(void)
{
        enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd, VIDIOC_STREAMOFF, &type))
			errno_exit ("VIDIOC_STREAMOFF");

		break;
	}
}

static void start_capturing(void)
{
        unsigned int i;
        enum v4l2_buf_type type;

	switch (io) {
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i) {
      struct v4l2_buffer buf;

      CLEAR (buf);

      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_MMAP;
      buf.index       = i;

      if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
        errno_exit ("VIDIOC_QBUF ... !!!");
		}
		
    type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	  if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
	    errno_exit ("VIDIOC_STREAMON");

	  break;

	case IO_METHOD_USERPTR:
    for (i = 0; i < n_buffers; ++i) {
      struct v4l2_buffer buf;

      CLEAR (buf);

      buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory      = V4L2_MEMORY_USERPTR;
			buf.index       = i;
			buf.m.userptr	= (unsigned long) buffers[i].start;
			buf.length      = buffers[i].length;

			if (-1 == xioctl (fd, VIDIOC_QBUF, &buf))
                    		errno_exit ("VIDIOC_QBUF");
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl (fd, VIDIOC_STREAMON, &type))
			errno_exit ("VIDIOC_STREAMON");

		break;
	}
}

static void uninit_device(void)
{
        unsigned int i;

	switch (io) {
	case IO_METHOD_READ:
		free (buffers[0].start);
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i)
			if (-1 == munmap (buffers[i].start, buffers[i].length))
				errno_exit ("munmap");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i)
			free (buffers[i].start);
		break;
	}

	free (buffers);
}
int init_view()
{
	SDL_Color colors[256];
	int i;
	if (SDL_Init(SDL_INIT_VIDEO)) {
		printf("Unable to initialize SDL\n");
		return -1;
	}
	if ((hrtview.surface = 
	     SDL_SetVideoMode(hrtview.width, hrtview.height, hrtview.depth, hrtview.flags)) == NULL) {
		printf("Unable to obtain video mode %dx%dx%d\n",
			hrtview.width, hrtview.height, hrtview.depth);
		return -1;
	}

	/* set the driver's window */
	ioctl(hrtview.fd, IOC_HRT_WIN_SET_X, &hrtview.x);
	ioctl(hrtview.fd, IOC_HRT_WIN_SET_Y, &hrtview.y);
        printf("setting width %d\n", hrtview.width);
	ioctl(hrtview.fd, IOC_HRT_WIN_SET_WIDTH, &hrtview.width);
	ioctl(hrtview.fd, IOC_HRT_WIN_SET_HEIGHT, &hrtview.height);
	/* Intialize the B&W palette */
	for (i = 0; i < 256; i++) {
		colors[i].r = i;	
		colors[i].g = i;	
		colors[i].b = i;	
	}
	SDL_SetColors(hrtview.surface, colors, 0, 256);
	SDL_WM_SetCaption("hrtview", "hrtview");

	return 0;
}

void close_view()
{
	if (hrtview.surface) SDL_FreeSurface(hrtview.surface);
	hrtview.surface = (SDL_Surface *)NULL;
	if (SDL_WasInit(SDL_INIT_VIDEO)) 
		SDL_Quit();
}
void draw_invert_rect()
{
	int x, y;

	if (!hrtview.dragging) return;
	
	y = hrtview.start_y;
	for (x = hrtview.start_x; x < hrtview.cur_x; x++) 
		((unsigned char *)hrtview.surface->pixels)[y*(hrtview.width*(hrtview.depth/8))+x] =
			~((unsigned char *)hrtview.surface->pixels)[y*(hrtview.width*(hrtview.depth/8))+x];
	y = hrtview.cur_y;
	for (x = hrtview.start_x; x < hrtview.cur_x; x++) 
		((unsigned char *)hrtview.surface->pixels)[y*(hrtview.width*(hrtview.depth/8))+x] =
			~((unsigned char *)hrtview.surface->pixels)[y*(hrtview.width*(hrtview.depth/8))+x];
	x = hrtview.start_x;
	for (y = hrtview.start_y; y < hrtview.cur_y; y++) 
		((unsigned char *)hrtview.surface->pixels)[y*(hrtview.width*(hrtview.depth/8))+x] =
			~((unsigned char *)hrtview.surface->pixels)[y*(hrtview.width*(hrtview.depth/8))+x];
	x = hrtview.cur_x;
	for (y = hrtview.start_y; y < hrtview.cur_y; y++) 
		((unsigned char *)hrtview.surface->pixels)[y*(hrtview.width*(hrtview.depth/8))+x] =
			~((unsigned char *)hrtview.surface->pixels)[y*(hrtview.width*(hrtview.depth/8))+x];
}

int update_frame(const void * p)
{
  int result;
  int y, x;
  int i,j;
 

  hrtview.frame_count++;
  printf("buf size = %d\n", hrtview.width * hrtview.height * (hrtview.depth / 8));
  fflush(0);

  if (SDL_MUSTLOCK(hrtview.surface))
   	SDL_LockSurface(hrtview.surface);

  memcpy(hrtview.surface->pixels, (unsigned char *)p, 
         hrtview.width * hrtview.height * (hrtview.depth / 8));

#if 0
	if (hrtview.fd >= 0) {
		/* We expect read() to block waiting for next frame */
 	        printf ("reading\n");
		result = read(hrtview.fd, hrtview.surface->pixels, 
		     hrtview.width * hrtview.height * (hrtview.depth / 8));
 	        printf ("read returned %d\n", result);
                if (result == -1) return -1;

	}
#endif
	/*
	for (y = 0; y < hrtview.height; y++) {
  	        for (x = 0; x < hrtview.width; x++) {	
			((char *)hrtview.surface->pixels)[y*hrtview.width+x] = 
			(char)((hrtview.frame_count%101)*x+(y/hrtview.frame_count)*101)%256;
		}
	}
	*/
	if (hrtview.dragging) 
		draw_invert_rect();
	
	SDL_UnlockSurface(hrtview.surface);
	SDL_Flip(hrtview.surface);	/* should wait for vsync */
	//SDL_SaveBMP(hrtview.surface, "cp2.bmp");
	//exit(0);

	return 0;
}

static void init_read(unsigned int buffer_size)
{
        buffers = calloc (1, sizeof (*buffers));

        if (!buffers) {
                fprintf (stderr, "Out of memory\n");
                exit (EXIT_FAILURE);
        }

	buffers[0].length = buffer_size;
	buffers[0].start = malloc (buffer_size);
 
	if (!buffers[0].start) {
    		fprintf (stderr, "Out of memory\n");
            	exit (EXIT_FAILURE);
	}
}

static void init_mmap(void)
{
	struct v4l2_requestbuffers req;

  CLEAR (req);
  req.count               = 4;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_MMAP;

	if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      fprintf (stderr, "%s does not support memory mapping\n", dev_name);
            exit (EXIT_FAILURE);
    } else 
            errno_exit ("VIDIOC_REQBUFS");
  }

  if (req.count < 2) {
    fprintf (stderr, "Insufficient buffer memory on %s\n", dev_name);
    exit (EXIT_FAILURE);
  }

  buffers = calloc (req.count, sizeof (*buffers));

  if (!buffers) {
    fprintf (stderr, "Out of memory\n"); 
    exit (EXIT_FAILURE);
  }

  for (n_buffers = 0; n_buffers < req.count; ++n_buffers) {
    struct v4l2_buffer buf;

    CLEAR (buf);

    buf.type        = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory      = V4L2_MEMORY_MMAP;
    buf.index       = n_buffers;

    if (-1 == xioctl (fd, VIDIOC_QUERYBUF, &buf))
      errno_exit ("VIDIOC_QUERYBUF");

    buffers[n_buffers].length = buf.length;
    buffers[n_buffers].start =
      mmap (NULL /* start anywhere */,
            buf.length,
            PROT_READ | PROT_WRITE /* required */,
            MAP_SHARED /* recommended */,
            fd, buf.m.offset);

    if (MAP_FAILED == buffers[n_buffers].start)
      errno_exit ("mmap");
  }

}

static void init_userp(unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;
  unsigned int page_size;

  page_size = getpagesize ();
  buffer_size = (buffer_size + page_size - 1) & ~(page_size - 1);

printf("bufzise = %d\n",buffer_size);
  CLEAR (req);

  req.count               = 4;
  req.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory              = V4L2_MEMORY_USERPTR;

  if (-1 == xioctl (fd, VIDIOC_REQBUFS, &req)) {
    if (EINVAL == errno) {
      fprintf (stderr, "%s does not support user pointer i/o\n", dev_name);
      exit (EXIT_FAILURE);
    } else 
      errno_exit ("VIDIOC_REQBUFS");
  }

  buffers = calloc (4, sizeof (*buffers));

  if (!buffers) {
    fprintf (stderr, "Out of memory\n");
    exit (EXIT_FAILURE);
  }

  for (n_buffers = 0; n_buffers < 4; ++n_buffers) {
    buffers[n_buffers].length = buffer_size;
    buffers[n_buffers].start = memalign (/* boundary */page_size, buffer_size);

    if (!buffers[n_buffers].start) {
  	  fprintf (stderr, "Out of memory\n");
      exit (EXIT_FAILURE);
		}
  }

}

static void init_device(void)
{
  struct v4l2_capability cap;
  struct v4l2_cropcap cropcap;
  struct v4l2_crop crop;
  struct v4l2_format fmt;
  unsigned int min;

  memset(&cap,0,sizeof(struct v4l2_capability));
  if (-1 == xioctl (fd, VIDIOC_QUERYCAP, &cap)) {
    if (EINVAL == errno) {
      fprintf (stderr, "%s is no V4L2 device\n", dev_name);
                  exit (EXIT_FAILURE);
    } else 
      errno_exit ("VIDIOC_QUERYCAP");
  }

  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    fprintf (stderr, "%s is no video capture device\n", dev_name);
          exit (EXIT_FAILURE);
  }

	switch (io) {
	  case IO_METHOD_READ:
		  if (!(cap.capabilities & V4L2_CAP_READWRITE)) {
			  fprintf (stderr, "%s does not support read i/o\n", dev_name);
			  exit (EXIT_FAILURE);
		  }
		  break;
	  case IO_METHOD_MMAP:
	  case IO_METHOD_USERPTR:
		  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
			  fprintf (stderr, "%s does not support streaming i/o\n", dev_name);
			  exit (EXIT_FAILURE);
		  } else
		    fprintf(stdout, "%s supports streaming\n", dev_name);
		  break;
	}
  /* Select video input, video standard and tune here. */
	CLEAR (cropcap);

  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

  if (0 == xioctl (fd, VIDIOC_CROPCAP, &cropcap)) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */

    if (-1 == xioctl (fd, VIDIOC_S_CROP, &crop)) {
      switch (errno) {
        case EINVAL:
          /* Cropping not supported. */
          break;
        default:
          /* Errors ignored. */
          break;
      }
    }
  } else {	
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

  if (-1 == xioctl (fd, VIDIOC_S_FMT, &fmt))
          errno_exit ("VIDIOC_S_FMT");

  /* Note VIDIOC_S_FMT may change width and height. */

	/* Buggy driver paranoia. */
	min = fmt.fmt.pix.width * 2;
	if (fmt.fmt.pix.bytesperline < min)
		fmt.fmt.pix.bytesperline = min;
	min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
	if (fmt.fmt.pix.sizeimage < min)
		fmt.fmt.pix.sizeimage = min;

	switch (io) {
	case IO_METHOD_READ:
		init_read (fmt.fmt.pix.sizeimage);
		break;
	case IO_METHOD_MMAP:
		init_mmap ();
    fprintf(stdout, "%s is in memory mapping mode!\n", dev_name);
		break;
	case IO_METHOD_USERPTR:
		init_userp (fmt.fmt.pix.sizeimage);
		break;
	}

}

static void close_device(void)
{
  if (-1 == close (fd))
	  errno_exit ("close");
  fd = -1;

}

static void open_device(void)
{
  struct stat st; 

  if (-1 == stat (dev_name, &st)) {
    fprintf (stderr, "Cannot identify '%s': %d, %s\n",
             dev_name, errno, strerror (errno));
    exit (EXIT_FAILURE);
  }

  if (!S_ISCHR (st.st_mode)) {
    fprintf (stderr, "%s is no device\n", dev_name);
    exit (EXIT_FAILURE);
  }

  fd = open (dev_name, O_RDWR /* required */ | O_NONBLOCK, 0);

  if (-1 == fd) {
    fprintf (stderr, "Cannot open '%s': %d, %s\n",
             dev_name, errno, strerror (errno));
    exit (EXIT_FAILURE);
  }
}

static void usage(FILE *fp, int argc, char ** argv)
{
  fprintf (fp,
    "Usage: %s [options]\n\n"
    "Options:\n"
    "-d | --device name   Video device name [/dev/video]\n"
    "-h | --help          Print this message\n"
    "-m | --mmap          Use memory mapped buffers\n"
    "-r | --read          Use read() calls\n"
    "-u | --userp         Use application allocated buffers\n"
    "-c | --color         Set video to color mode\n"
    "-g | --grey          Set video to grey mode\n"
    "",
		argv[0]);

}

static const char short_options [] = "d:hmrucg";

static const struct option long_options [] = {
  { "device",     required_argument,      NULL,           'd' },
  { "help",       no_argument,            NULL,           'h' },
  { "mmap",       no_argument,            NULL,           'm' },
  { "read",       no_argument,            NULL,           'r' },
  { "userp",      no_argument,            NULL,           'u' },
  { "color",      no_argument,            NULL,           'c' },
  { "grey",       no_argument,            NULL,           'g' },
  { 0, 0, 0, 0 }
};

int main(int argc, char ** argv)
{
  dev_name = "/dev/video0";

  for (;;) {
    int index;
    int c;
    
    c = getopt_long (argc, argv, short_options, long_options, &index);
    if (-1 == c)
            break;
    switch (c) {
    case 0: /* getopt_long() flag */
      break;
    case 'd':
      dev_name = optarg;
      break;
    case 'h':
      usage (stdout, argc, argv);
      exit (EXIT_SUCCESS);
    case 'm':
      io = IO_METHOD_MMAP;
      break;
    case 'r':
      io = IO_METHOD_READ;
      break;
    case 'u':
      io = IO_METHOD_USERPTR;
      break;
    case 'c':
      video_width = 640;
      video_height = 480;
      video_depth = 16;
      break;
    case 'g':
      video_width = 512;
      video_height = 480;
      video_depth = 8;
      break;
    default:
      usage (stderr, argc, argv);
      exit (EXIT_FAILURE);
    }
  } // for

  open_device ();

  init_params(argc, argv);

  init_device ();

  if(init_view()) {
	close_device();
	return -1;
  }		

  start_capturing ();

  mainloop ();

  stop_capturing ();

  close_view();

  uninit_device ();

  close_device ();

  return 0;
}
