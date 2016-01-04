/**
 * @file v4l2dev.c
 * @brief funzioni supporto per gestione device Video for linux 2
 *  V4L2 video capture example
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include <fcntl.h>              /* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include <linux/videodev2.h>

#include "v4l2dev.h"

#define CLEAR(x) memset(&(x), 0, sizeof(x))

/**
 */
static int xioctl(int fh, int request, void *arg)
{
  int r;

  do {
    r = ioctl(fh, request, arg);
  } while (-1 == r && EINTR == errno);
  return r;
}

/**
 */
psDevHnd devOpen(char* devname)
{
  psDevHnd      pDev;
  struct stat   st;
  int           ret;

  fprintf(stderr, "devOpen '%s'\n", devname);

  pDev = (psDevHnd)malloc(sizeof(sDevHnd));
  if(pDev==NULL){
    return NULL;
  }
  strncpy(pDev->devname, devname, 63);
  ret = stat(devname, &st);
  if (ret  == -1) {
    fprintf(stderr, "Cannot identify '%s': %d, %s\n",
              devname, errno, strerror(errno));
    free(pDev);
    return NULL;
  }
  if (!S_ISCHR(st.st_mode)) {
    fprintf(stderr, "%s is no device\n", devname);
    free(pDev);
    return NULL;
  }

  pDev->fd = open(devname, O_RDWR | O_NONBLOCK, 0);
  if (pDev->fd == -1) {
    fprintf(stderr, "Cannot open '%s': %d, %s\n",
              devname, errno, strerror(errno));
    free(pDev);
    return NULL;
  }
  
  struct v4l2_capability cap;
  ret = xioctl(pDev->fd, VIDIOC_QUERYCAP, &cap);
  if(ret==-1){
    close(pDev->fd);
    free(pDev);
    return NULL;
  }
  pDev->cap = cap.capabilities;
  strncpy(pDev->driver, cap.driver, sizeof(cap.driver)-1);
  strncpy(pDev->card  , cap.card  , sizeof(cap.card  )-1);
  pDev->version = cap.version;

  return pDev;
}

/**
 */
void devShow(psDevHnd pDev)
{
  printf( "Device: %s\n"
          "Driver: %s\n"
          "Card  : %s\n"
          "Ver   : %d\n"
          "Capabilities:\n",
          pDev->devname,
          pDev->driver,
          pDev->card,
          pDev->version);
    fprintf(stderr, "    %s %s %s\n",
            (pDev->cap & V4L2_CAP_VIDEO_CAPTURE)? "V4L2_CAP_VIDEO_CAPTURE": "",
            (pDev->cap & V4L2_CAP_VIDEO_OUTPUT )? "V4L2_CAP_VIDEO_OUTPUT" : "",
            (pDev->cap & V4L2_CAP_STREAMING    )? "V4L2_CAP_STREAMING"    : "");
#if 0

/*
301   * struct v4l2_capability - Describes V4L2 device caps returned by VIDIOC_QUERYCAP
302   *
303   * @driver:       name of the driver module (e.g. "bttv")
304   * @card:         name of the card (e.g. "Hauppauge WinTV")
305   * @bus_info:     name of the bus (e.g. "PCI:" + pci_name(pci_dev) )
306   * @version:      KERNEL_VERSION
307   * @capabilities: capabilities of the physical device as a whole
308   * @device_caps:  capabilities accessed via this particular device (node)
309   * @reserved:     reserved fields for future extensions
310   */
311 struct v4l2_capability {
312         __u8    driver[16];
313         __u8    card[32];
314         __u8    bus_info[32];
315         __u32   version;
316         __u32   capabilities;
317         __u32   device_caps;
318         __u32   reserved[3];
319 };
320
321 /* Values for 'capabilities' field */
322 #define V4L2_CAP_VIDEO_CAPTURE          0x00000001  /* Is a video capture device */
323 #define V4L2_CAP_VIDEO_OUTPUT           0x00000002  /* Is a video output device */
324 #define V4L2_CAP_VIDEO_OVERLAY          0x00000004  /* Can do video overlay */
325 #define V4L2_CAP_VBI_CAPTURE            0x00000010  /* Is a raw VBI capture device */
326 #define V4L2_CAP_VBI_OUTPUT             0x00000020  /* Is a raw VBI output device */
327 #define V4L2_CAP_SLICED_VBI_CAPTURE     0x00000040  /* Is a sliced VBI capture device */
328 #define V4L2_CAP_SLICED_VBI_OUTPUT      0x00000080  /* Is a sliced VBI output device */
329 #define V4L2_CAP_RDS_CAPTURE            0x00000100  /* RDS data capture */
330 #define V4L2_CAP_VIDEO_OUTPUT_OVERLAY   0x00000200  /* Can do video output overlay */
331 #define V4L2_CAP_HW_FREQ_SEEK           0x00000400  /* Can do hardware frequency seek  */
332 #define V4L2_CAP_RDS_OUTPUT             0x00000800  /* Is an RDS encoder */
333
334 /* Is a video capture device that supports multiplanar formats */
335 #define V4L2_CAP_VIDEO_CAPTURE_MPLANE   0x00001000
336 /* Is a video output device that supports multiplanar formats */
337 #define V4L2_CAP_VIDEO_OUTPUT_MPLANE    0x00002000
338 /* Is a video mem-to-mem device that supports multiplanar formats */
339 #define V4L2_CAP_VIDEO_M2M_MPLANE       0x00004000
340 /* Is a video mem-to-mem device */
341 #define V4L2_CAP_VIDEO_M2M              0x00008000
342
343 #define V4L2_CAP_TUNER                  0x00010000  /* has a tuner */
344 #define V4L2_CAP_AUDIO                  0x00020000  /* has audio support */
345 #define V4L2_CAP_RADIO                  0x00040000  /* is a radio device */
346 #define V4L2_CAP_MODULATOR              0x00080000  /* has a modulator */
347
348 #define V4L2_CAP_SDR_CAPTURE            0x00100000  /* Is a SDR capture device */
349 #define V4L2_CAP_EXT_PIX_FORMAT         0x00200000  /* Supports the extended pixel format */
350
351 #define V4L2_CAP_READWRITE              0x01000000  /* read/write systemcalls */
352 #define V4L2_CAP_ASYNCIO                0x02000000  /* async I/O */
353 #define V4L2_CAP_STREAMING              0x04000000  /* streaming I/O ioctls */
354
355 #define V4L2_CAP_DEVICE_CAPS            0x80000000  /* sets device capabilities field */
356

#endif
}

/**
 */
int devClose(psDevHnd pDev)
{
  close(pDev->fd);
  free(pDev);
  return 0;
}

/**
 */
int devSetTypes(psDevHnd pDev, int mem_type, enum v4l2_buf_type  buf_type)
{
//  dev_type = dev_type;
  pDev->mem_type = mem_type;
  pDev->buf_type = buf_type;
  return 0;
}

/**
 * chiama VIDIOC_REQBUFS e VIDIOC_QUERYBUF
 * @param pDev
 * @param type V4L2_BUF_TYPE_VIDEO_CAPTURE o V4L2_BUF_TYPE_VIDEO_OUTPUT
 * @param mem  V4L2_MEMORY_MMAP o V4L2_MEMORY_USERPTR
 */
//TODO
int devBufReq(psDevHnd pDev, int count, psDevBuf pBuf)//int type, int mem, 
{
  struct v4l2_requestbuffers  req;
  int                         ret;
  int                         i;

  if( pDev->mem_type!=V4L2_MEMORY_MMAP    &&
      pDev->mem_type!=V4L2_MEMORY_USERPTR ){
    return -1;
  }

  CLEAR(req);
  req.count  = count;
  req.type   = pDev->buf_type;//V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = pDev->mem_type;//V4L2_MEMORY_MMAP;
  ret = xioctl(pDev->fd, VIDIOC_REQBUFS, &req);
  if(ret){
      printf("%s error VIDIOC_REQBUFS type %d mem %d\n", pDev->devname, pDev->buf_type, pDev->mem_type);
      return -1;//exit(EXIT_FAILURE);
  }
  if (req.count < 2) {
    fprintf(stderr, "init_mmap Insufficient buffer memory on %s\n");
    return -1;//exit(EXIT_FAILURE);
  }
/*
  *pBuf = calloc(req.count, sizeof(sDevBuf));
  if(!*pBuf){
    fprintf(stderr, "Out of memory\n");
    return -1;//exit(EXIT_FAILURE);
  }
*/
  if(pDev->mem_type==V4L2_MEMORY_MMAP){
    for(i=0; i<req.count; i++){
      struct v4l2_buffer buf;

      CLEAR(buf);
      buf.type   = pDev->buf_type;//V4L2_BUF_TYPE_VIDEO_CAPTURE;
      buf.memory = pDev->mem_type;//V4L2_MEMORY_MMAP;
      buf.index  = i;
      ret = xioctl(pDev->fd, VIDIOC_QUERYBUF, &buf);
      if(ret){
        printf("%s error %d VIDIOC_QUERYBUF type %d mem %d\n", ret, pDev->devname, pDev->buf_type, pDev->mem_type);
        return -1;//errno_exit("VIDIOC_QUERYBUF");
      }
      pBuf[i].size = buf.length;
      pBuf[i].addr = mmap(NULL /* start anywhere */,
                           buf.length,
                           PROT_READ | PROT_WRITE /* required */,
                           MAP_SHARED /* recommended */,
                           pDev->fd, buf.m.offset);
      fprintf(stderr, "%d buf addr V: %08X P: %08X\n", i,
              pBuf[i].addr,
              buf.m.offset);

      if(pBuf[i].addr==MAP_FAILED){
        return -1;//errno_exit("mmap");
      }
    }
  }else{
    //USERPTR
    for(i=0; i<req.count; i++){
      struct v4l2_buffer buf;

      CLEAR(buf);
      buf.type     = pDev->buf_type;//V4L2_BUF_TYPE_VIDEO_OUTPUT;
      buf.memory   = pDev->mem_type;//V4L2_MEMORY_USERPTR;
      buf.index    = i;
      buf.length   = pBuf[i].size;
      buf.m.offset = (int)pBuf[i].addr;
      fprintf(stderr, "VIDIOC_QUERYBUF %d buf addr: %08X size: %08X\n", i,
                pBuf[i].addr, pBuf[i].size);
      ret = xioctl(pDev->fd, VIDIOC_QUERYBUF, &buf);
      if(ret){
        printf("%s error %d VIDIOC_QUERYBUF type %d mem %d\n", pDev->devname, ret, pDev->buf_type, pDev->mem_type);
        return -1;//errno_exit("VIDIOC_QUERYBUF");
      }
    }
  }
  return req.count;
}
