/*
 * hrt.h
 * 
 * application/kernel interface to HRT driver
 */

#ifndef _HRT_H_
#define _HRT_H_

#ifndef __KERNEL__
#include <sys/ioctl.h>
#endif

#define HRT_MAX_DEVICES 8

/* The offset of the HRT I2C bus control port (0x2000 = 8192 = 2^13) */
#define HRT_CONTROL_REG 0x2000
#define HRT_I2C_REG     0x2001
#define HRT_Y_LOW_REG   0x2002
#define HRT_Y_HIGH_REG  0x2003
#define HRT_IRQ_ENABLE  0x2005

#define HRT_LIVE_CMD          0x91
#define HRT_FREEZE_IMM_CMD    0x5B
#define HRT_FREEZE_NEXT_CMD   0x99

#define IOC_HRT_MAGIC_NUM	'v'

#define IOC_HRT_GO_LIVE		_IO (IOC_HRT_MAGIC_NUM, 2)
#define IOC_HRT_FREEZE_FRAME	_IO (IOC_HRT_MAGIC_NUM, 3)
#define IOC_HRT_WIN_SET_WIDTH	_IOW(IOC_HRT_MAGIC_NUM, 4, int)
#define IOC_HRT_WIN_SET_HEIGHT	_IOW(IOC_HRT_MAGIC_NUM, 5, int)
#define IOC_HRT_WIN_SET_X	_IOW(IOC_HRT_MAGIC_NUM, 6, int)
#define IOC_HRT_WIN_SET_Y	_IOW(IOC_HRT_MAGIC_NUM, 7, int)
#define IOC_HRT_SET_MODE        _IOW(IOC_HRT_MAGIC_NUM, 8, int)
#define IOC_HRT_UPSIDE_DOWN	_IO (IOC_HRT_MAGIC_NUM, 9)


#define HRT_HEIGHT            480

/* NTSC 8-bit greyscale */
#define HRT_GRAY_WIDTH             512
#define HRT_GRAY_HEIGHT            480
#define HRT_GRAY_BYTES_PER_PIXEL   1
#define HRT_GRAY_BYTES_PER_LINE    (HRT_GRAY_WIDTH * HRT_GRAY_BYTES_PER_PIXEL)
#define HRT_GRAY_FRAMESIZE         (HRT_GRAY_WIDTH * HRT_GRAY_HEIGHT * HRT_GRAY_BYTES_PER_PIXEL)


/* NTSC 16-bit  color */

#define HRT_COLOR_WIDTH             640 
#define HRT_COLOR_HEIGHT            480
#define HRT_COLOR_BYTES_PER_PIXEL   2
#define HRT_COLOR_BYTES_PER_LINE    (HRT_COLOR_WIDTH * HRT_COLOR_BYTES_PER_PIXEL)
#define HRT_COLOR_FRAMESIZE         (HRT_COLOR_WIDTH * HRT_COLOR_HEIGHT * HRT_COLOR_BYTES_PER_PIXEL)

/**
 * struct subwindow - describes a region-of-interest, i.e.,
 *  a part of the frame
 */
struct subwindow {
  int width, height, depth;
        int startx, starty;
};

/* A/D registers */
#define HRT_BRIGHTNESS_REG    0x19
#define HRT_CONTRAST_REG      0x13

#define HRT_VID_TYPE         (VID_TYPE_CAPTURE | VID_TYPE_MONOCHROME)


/* Private ioctl's */
#define IOC_HRT_SET_I2CREG           _IOW('v', BASE_VIDIOCPRIVATE + 30, struct i2c_regval)
#define IOC_HRT_GET_I2CREG           _IOR('v', BASE_VIDIOCPRIVATE + 31, struct i2c_regval)
#define IOC_HRT_SET_ROI              _IOW('v', BASE_VIDIOCPRIVATE + 32, struct subwindow)
#define IOC_HRT_GET_ROI              _IOR('v', BASE_VIDIOCPRIVATE + 33, struct subwindow)
#define IOC_HRT_SET_I2CREGS          _IOW('v', BASE_VIDIOCPRIVATE + 34, char *)
#define IOC_HRT_NEXT_MMAP_IS_DIRECT  _IOW('v', BASE_VIDIOCPRIVATE + 35, int)
#define IOC_HRT_GET_MAGIC_MMAP_OFFSET  _IOW('v', BASE_VIDIOCPRIVATE + 36, int)

#endif // _HRT_H_
