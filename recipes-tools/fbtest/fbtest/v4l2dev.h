/*
 *  V4L2 video capture example
 *
 *  This program can be used and distributed without restrictions.
 *
 *      This program is provided with the V4L2 API
 * see http://linuxtv.org/docs.php for more information
 */

/*#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <getopt.h>

#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
*/

#include <linux/videodev2.h>

#ifndef __V4L2DEV_H
#define __V4L2DEV_H

/**
 */
typedef unsigned char   u08;
typedef   signed char   s08;
typedef unsigned short  u16;
typedef   signed short  s16;
typedef unsigned long   u32;
typedef   signed long   s32;

/**
 */
typedef struct {
  void   *addr;
  size_t  size;
}sDevBuf, *psDevBuf;

/**
 */
typedef struct {
  char      devname[64];
  int       fd;

  u32       cap;
  char      driver[16];
  char      card[32];
  //char    bus_info[32];
  u32       version;
  //u32     device_caps;
/*
  u32       num;
  psDevBuf  buf;
*/

//  int                 dev_type;
  int                 mem_type;
  enum v4l2_buf_type  buf_type;

}sDevHnd, *psDevHnd;


/**
 */
psDevHnd devOpen(char* devname);

/**
 */
void devShow(psDevHnd pDev);

/**
 */
int devClose(psDevHnd pDev);

/**
 */
int devSetTypes(psDevHnd pDev, int mem_type, enum v4l2_buf_type  buf_type);

/**
 * chiama VIDIOC_REQBUFS e VIDIOC_QUERYBUF
 * @param pDev
 * @param type V4L2_BUF_TYPE_VIDEO_CAPTURE o V4L2_BUF_TYPE_VIDEO_OUTPUT
 * @param mem  V4L2_MEMORY_MMAP o V4L2_MEMORY_USERPTR
 */
//TODO
int devBufReq(psDevHnd pDev, int count, psDevBuf pBuf);//int type, int mem, 


#endif  /* __V4L2DEV_H */
