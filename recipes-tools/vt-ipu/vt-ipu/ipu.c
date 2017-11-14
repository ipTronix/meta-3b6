/**
 * @file ipu.c
 */
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "mxcfb.h"
#include "ipu.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "IPU"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif

static unsigned int fmt_to_bpp(unsigned int pixelformat);

/**
 * device /dev/mxc_ipu
 */
psIpuHnd ipuOpen(char* device, int bpp)
{
  psIpuHnd  pHnd;
  u32       format;
  int       ret;

  pHnd = (psIpuHnd)malloc(sizeof(sIpuHnd));
  if(!pHnd){
    DBG_ERROR("insufficent memory.\n");
    return NULL;
  }
  memset(pHnd, 0, sizeof(sIpuHnd));

  pHnd->fd = open(device, O_RDWR, 0);
  if(pHnd->fd < 0){
    DBG_ERROR("open ipu dev '%s' fail\n", device);
    free(pHnd);
    return NULL;
  }

  if(bpp==32){
    format = v4l2_fourcc('R','G','B','A');
  }else{
    format = IPU_PIX_FMT_RGB565;
  }

  // Default Settings

  pHnd->task.priority = 0;
  pHnd->task.task_id  = 0;
  pHnd->task.timeout  = 1000;

  pHnd->task.input.width              = 1280;
  pHnd->task.input.height             =  800;
  pHnd->task.input.format             = format;
  pHnd->task.input.crop.pos.x         = 0;
  pHnd->task.input.crop.pos.y         = 0;
  pHnd->task.input.crop.w             = 0;
  pHnd->task.input.crop.h             = 0;
  pHnd->task.input.deinterlace.enable = 0;
  pHnd->task.input.deinterlace.motion = 0;

//  pHnd->task.output.paddr      =
  pHnd->task.output.width      = 1280;
  pHnd->task.output.height     = 800;
  pHnd->task.output.format     = format;
  pHnd->task.output.rotate     = 0;
  pHnd->task.output.crop.pos.x = 0;
  pHnd->task.output.crop.pos.y = 0;
  pHnd->task.output.crop.w     = 0;
  pHnd->task.output.crop.h     = 0;

  pHnd->isize = pHnd->task.input.width * pHnd->task.input.height *
                fmt_to_bpp(pHnd->task.input.format)/8;

  /**
   * ioctl(fd_ipu, IPU_ALLOC, &val)
   *   val is the size of allocated buffer as an input and
   *       the physical address of the buffer as the output.
   */
  pHnd->task.input.paddr = pHnd->isize;
  ret = ioctl(pHnd->fd, IPU_ALLOC, &pHnd->task.input.paddr);
  if(ret<0){
    DBG_ERROR("ioctl IPU_ALLOC fail\n");
    close(pHnd->fd);
    free(pHnd);
    return NULL;
  }

  // Map the IPU input buffer
  pHnd->inbuf = mmap(0, pHnd->isize, PROT_READ | PROT_WRITE,
                     MAP_SHARED, pHnd->fd, pHnd->task.input.paddr);
  if(pHnd->inbuf==MAP_FAILED){
    DBG_ERROR("mmap fail\n");
    ioctl(pHnd->fd, IPU_FREE, &pHnd->task.input.paddr);
    close(pHnd->fd);
    free(pHnd);
    return NULL;
  }

  return pHnd;
}

/**
 */
int ipuClose(psIpuHnd pHnd)
{
  munmap(pHnd->inbuf, pHnd->isize);
  ioctl(pHnd->fd, IPU_FREE, &pHnd->task.input.paddr);
  //ioctl(pHnd->fd, IPU_FREE, &pHnd->task.output.paddr);

  close(pHnd->fd);
  free(pHnd);

  return 0;
}

/**
 */
int ipuRotate(psIpuHnd pHnd, uint32_t paddr, int x, int y, int w, int h, int rot)
{
  int ret;

  pHnd->task.output.paddr      = (dma_addr_t)paddr;
  pHnd->task.output.rotate     = rot;
  pHnd->task.output.crop.pos.x = x;
  pHnd->task.output.crop.pos.y = y;
  if( (rot==IPU_ROTATE_90_RIGHT      ) ||
      (rot==IPU_ROTATE_90_RIGHT_VFLIP) ||
      (rot==IPU_ROTATE_90_RIGHT_HFLIP) ||
      (rot==IPU_ROTATE_90_LEFT       ) ){
    pHnd->task.output.crop.w = h;
    pHnd->task.output.crop.h = w;
  }else{
    pHnd->task.output.crop.w = w;
    pHnd->task.output.crop.h = h;
  }

  ret = ioctl(pHnd->fd, IPU_QUEUE_TASK, &pHnd->task);
  if(ret < 0){
    DBG_ERROR("ioctl IPU_QUEUE_TASK fail\n");
    return -1;
  }
  return 0;
}

/**
 */
static unsigned int fmt_to_bpp(unsigned int pixelformat)
{
  unsigned int bpp;

  switch(pixelformat){
  case IPU_PIX_FMT_RGB565:
  /*interleaved 422*/
  case IPU_PIX_FMT_YUYV:
  case IPU_PIX_FMT_UYVY:
  /*non-interleaved 422*/
  case IPU_PIX_FMT_YUV422P:
  case IPU_PIX_FMT_YVU422P:
    bpp = 16;
    break;
  case IPU_PIX_FMT_BGR24:
  case IPU_PIX_FMT_RGB24:
  case IPU_PIX_FMT_YUV444:
//  case IPU_PIX_FMT_YUV444P:
    bpp = 24;
    break;
  case IPU_PIX_FMT_BGR32:
  case IPU_PIX_FMT_BGRA32:
  case IPU_PIX_FMT_RGB32:
  case IPU_PIX_FMT_RGBA32:
  case IPU_PIX_FMT_ABGR32:
    bpp = 32;
    break;
  /*non-interleaved 420*/
  case IPU_PIX_FMT_YUV420P:
  case IPU_PIX_FMT_YVU420P:
  case IPU_PIX_FMT_YUV420P2:
//  case IPU_PIX_FMT_NV12:
//  case IPU_PIX_FMT_TILED_NV12:
    bpp = 12;
    break;
  default:
    bpp = 8;
    break;
  }
  return bpp;
}
