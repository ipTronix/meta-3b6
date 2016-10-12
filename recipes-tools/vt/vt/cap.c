/** ***************************************************************************
 * @file cap.c
 * @brief Video for linux capture handling
 **************************************************************************** */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/videodev2.h>

#include <sys/select.h>
#include <sys/time.h>

#include "cap.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "CAP"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
//fprintf(stderr, "%s is no V4L2 device\n", pCap->name);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif

/**
 */
psCapDev capOpen(char* name)
{
  psCapDev pCap;

  pCap = (psCapDev)malloc(sizeof(sCapDev));
  if(!pCap){
    DBG_ERROR("memory\n");
    return NULL;
  }
  memset(pCap, 0, sizeof(sCapDev));
  DBG_PRINT("device %s pCap [%p]\n", name, pCap);

//  pCap->fd = open(name, O_RDWR | O_NONBLOCK, 0);
  pCap->fd = open(name, O_RDWR, 0);
  if(pCap->fd<0){
    DBG_ERROR("Unable to open %s (%d) %s\n", name, errno, strerror(errno));
    free(pCap);
    return NULL;
  }
  strncpy(pCap->name, name, sizeof(pCap->name));
  pCap->nbuf  = CAP_NUM_BUF;
  pCap->std   = V4L2_STD_PAL;
  pCap->fmt   = V4L2_PIX_FMT_UYVY;
  pCap->input = 1;
  DBG_PRINT("Device %s opened fd is %d\n", name, pCap->fd);
  return pCap;
}

/**
 */
int capSetup(psCapDev pCap)
{
  struct v4l2_capability      cap;
  struct v4l2_cropcap         cropcap;
  struct v4l2_crop            crop;
  struct v4l2_format          fmt;
  struct v4l2_requestbuffers  req;
//  struct v4l2_dbg_chip_ident  chip;
  struct v4l2_streamparm      parm;
  v4l2_std_id                 id;
  unsigned int                min;

  if (ioctl(pCap->fd, VIDIOC_QUERYCAP, &cap) < 0) {
    if (EINVAL == errno) {
      DBG_ERROR("%s is no V4L2 device\n", pCap->name);
      return -1;
    } else {
      DBG_ERROR("%s isn not V4L device, unknow error\n", pCap->name);
      return -1;
    }
  }
  if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE)) {
    DBG_ERROR("%s is no video capture device\n", pCap->name);
    return -1;
  }
  if (!(cap.capabilities & V4L2_CAP_STREAMING)) {
    DBG_ERROR("%s does not support streaming i/o\n", pCap->name);
    return -1;
  }
/*
  if (ioctl(pCap->fd, VIDIOC_DBG_G_CHIP_IDENT, &chip)) {
    DBG_ERROR("VIDIOC_DBG_G_CHIP_IDENT failed.\n");
    close(pCap->fd);
    return -1;
  }
  DBG_ERROR("TV decoder chip is %s\n", chip.match.name);
*/
  if (ioctl(pCap->fd, VIDIOC_S_INPUT, &pCap->input) < 0) {
    DBG_ERROR("%s VIDIOC_S_INPUT failed\n", pCap->name);
    close(pCap->fd);
    return -1;
  }

  if (ioctl(pCap->fd, VIDIOC_G_STD, &id) < 0) {
    DBG_ERROR("VIDIOC_G_STD failed\n");
    close(pCap->fd);
    return -1;
  }
  id = pCap->std;
  if (ioctl(pCap->fd, VIDIOC_S_STD, &id) < 0) {
    DBG_ERROR("%s VIDIOC_S_STD failed\n", pCap->name);
    close(pCap->fd);
    return -1;
  }

  /* Select video input, video standard and tune here. */
  memset(&cropcap, 0, sizeof(cropcap));
  cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(ioctl(pCap->fd, VIDIOC_CROPCAP, &cropcap) < 0) {
    crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    crop.c = cropcap.defrect; /* reset to default */

    if(ioctl(pCap->fd, VIDIOC_S_CROP, &crop) < 0) {
      switch (errno) {
        case EINVAL:
          /* Cropping not supported. */
          DBG_ERROR("%s doesn't support crop\n", pCap->name);
          break;
        default:
          /* Errors ignored. */
          break;
      }
    }
  } else {
    /* Errors ignored. */
  }

  parm.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  parm.parm.capture.timeperframe.numerator   = 1;
  parm.parm.capture.timeperframe.denominator = 0;
  parm.parm.capture.capturemode = 0;
  if(ioctl(pCap->fd, VIDIOC_S_PARM, &parm) < 0){
    DBG_ERROR("%s VIDIOC_S_PARM failed\n", pCap->name);
    close(pCap->fd);
    return -1;
  }

  memset(&fmt, 0, sizeof(fmt));
  fmt.type                = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  fmt.fmt.pix.width       = pCap->width;
  fmt.fmt.pix.height      = pCap->height;
  fmt.fmt.pix.pixelformat = pCap->fmt;
  fmt.fmt.pix.field       = V4L2_FIELD_INTERLACED;
  if(ioctl(pCap->fd, VIDIOC_S_FMT, &fmt) < 0){
    DBG_ERROR("%s iformat not supported \n", pCap->name);
    return -1;
  }

  /* Note VIDIOC_S_FMT may change width and height. */

  /* Buggy driver paranoia. */
  min = fmt.fmt.pix.width * 2;
  if(fmt.fmt.pix.bytesperline < min){
    fmt.fmt.pix.bytesperline = min;
  }
  min = fmt.fmt.pix.bytesperline * fmt.fmt.pix.height;
  if(fmt.fmt.pix.sizeimage < min){
    fmt.fmt.pix.sizeimage = min;
  }
  if(ioctl(pCap->fd, VIDIOC_G_FMT, &fmt) < 0){
    DBG_ERROR("VIDIOC_G_FMT failed\n");
    close(pCap->fd);
    return -1;
  }
  pCap->width  = fmt.fmt.pix.width;
  pCap->height = fmt.fmt.pix.height;
  pCap->bpl    = fmt.fmt.pix.bytesperline;
  pCap->imsize = fmt.fmt.pix.sizeimage;

  // request buffers
  memset(&req, 0, sizeof(req));
  req.count  = pCap->nbuf;
  req.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  req.memory = V4L2_MEMORY_MMAP;
  if(ioctl(pCap->fd, VIDIOC_REQBUFS, &req) < 0){
    if(EINVAL==errno){
      DBG_ERROR("%s does not supportmemory mapping\n", pCap->name);
      return -1;
    } else {
      DBG_ERROR("%s does not support memory mapping, unknow error\n", pCap->name);
      return -1;
    }
  }
  if(req.count<2){
    DBG_ERROR("Insufficient buffer memory on %s\n", pCap->name);
    return -1;
  }
  pCap->nbuf = req.count;

  DBG_PRINT("%s:width %d height %d Bpl %d image size %d num buffers %d\n",
            pCap->name,
            pCap->width, pCap->height, pCap->bpl, pCap->imsize, pCap->nbuf);
  return 0;
}

/**
 */
int capStart(psCapDev pCap)
{
  unsigned int        i;
  struct v4l2_buffer  buf;
  enum v4l2_buf_type  type;

  for(i=0; i<pCap->nbuf; i++) {
    if(pCap->buf[i].start){
      munmap(pCap->buf[i].start, pCap->buf[i].length);
      pCap->buf[i].start = 0;
    }
  }

  for(i=0; i<pCap->nbuf; i++){
    memset(&buf, 0, sizeof(buf));
    buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory = V4L2_MEMORY_MMAP;
    buf.index  = i;
    if(ioctl(pCap->fd, VIDIOC_QUERYBUF, &buf) < 0) {
      DBG_ERROR("%s VIDIOC_QUERYBUF error\n", pCap->name);
      return -1;
    }
    pCap->buf[i].length = buf.length;
    pCap->buf[i].offset = (size_t) buf.m.offset;
    pCap->buf[i].start  = mmap(NULL, pCap->buf[i].length,
                               PROT_READ | PROT_WRITE, MAP_SHARED,
                               pCap->fd, pCap->buf[i].offset);
    memset(pCap->buf[i].start, 0xFF, pCap->buf[i].length);
    DBG_PRINT("%s buf #%d offset=%p addr=%p len=%08X\n",
              pCap->name,
              i,
              pCap->buf[i].offset,
              pCap->buf[i].start ,
              pCap->buf[i].length);
  }

  DBG_PRINT("%s:Enqueue buffers\n", pCap->name);
  for(i=0; i<pCap->nbuf; i++){
    memset(&buf, 0, sizeof(buf));
    buf.type     = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    buf.memory   = V4L2_MEMORY_MMAP;
    buf.index    = i;
    buf.m.offset = pCap->buf[i].offset;
    if(ioctl(pCap->fd, VIDIOC_QBUF, &buf) < 0){
      DBG_ERROR("%s VIDIOC_QBUF error\n", pCap->name);
      return -1;
    }
  }

  DBG_PRINT("%s:Start streaming\n", pCap->name);
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  if(ioctl(pCap->fd, VIDIOC_STREAMON, &type) < 0){
    DBG_ERROR("%s VIDIOC_STREAMON error\n", pCap->name);
    return -1;
  }
  DBG_PRINT("%s:DONE\n", pCap->name);
  return 0;
}

/**
 */
int capStop(psCapDev pCap)
{
  unsigned int        i;
  enum v4l2_buf_type  type;

//  DBG_PRINT("pCap [%p]\n", pCap);
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(pCap->fd, VIDIOC_STREAMOFF, &type);
  for(i=0; i<pCap->nbuf; i++) {
    if(pCap->buf[i].start){
      munmap(pCap->buf[i].start, pCap->buf[i].length);
      pCap->buf[i].start = 0;
    }
  }
  DBG_PRINT("%s:DONE\n", pCap->name);
  return 0;
}

/**
 */
int capInputSet(psCapDev pCap, int input)
{
  int ret;

  pCap->input = input;
  ret = ioctl(pCap->fd, VIDIOC_S_INPUT, &pCap->input);
  if(ret){
    DBG_ERROR("VIDIOC_S_INPUT failed\n");
    return -1;
  }
  return 0;
}

/**
 * Standard enumeration
 */
int capStdLst(psCapDev pCap)
{
  struct v4l2_standard   std;
  int                    ret;

  std.index = 0;
  for(;;){
    ret = ioctl(pCap->fd, VIDIOC_ENUMSTD, &std);
    if(ret){
      break;
    }
    printf("%02d - %08X %s %d %d\n",
           std.index,
           std.id,
           std.name,
           std.frameperiod,
           std.framelines);
    std.index++;
  }
  return 0;
}

/**
 */
v4l2_std_id capStdGet(psCapDev pCap)
{
  v4l2_std_id   id;

  if (ioctl(pCap->fd, VIDIOC_G_STD, &id) < 0) {
    DBG_ERROR("VIDIOC_G_STD failed\n");
    return -1;
  }

  printf("std %08X\n", id);

  if( id & V4L2_STD_PAL_B       ){ printf("PAL_B      \n"); }
  if( id & V4L2_STD_PAL_B1      ){ printf("PAL_B1     \n"); }
  if( id & V4L2_STD_PAL_G       ){ printf("PAL_G      \n"); }
  if( id & V4L2_STD_PAL_H       ){ printf("PAL_H      \n"); }
  if( id & V4L2_STD_PAL_I       ){ printf("PAL_I      \n"); }
  if( id & V4L2_STD_PAL_D       ){ printf("PAL_D      \n"); }
  if( id & V4L2_STD_PAL_D1      ){ printf("PAL_D1     \n"); }
  if( id & V4L2_STD_PAL_K       ){ printf("PAL_K      \n"); }

  if( id & V4L2_STD_PAL_M       ){ printf("PAL_M      \n"); }
  if( id & V4L2_STD_PAL_N       ){ printf("PAL_N      \n"); }
  if( id & V4L2_STD_PAL_Nc      ){ printf("PAL_Nc     \n"); }
  if( id & V4L2_STD_PAL_60      ){ printf("PAL_60     \n"); }

  if( id & V4L2_STD_NTSC_M      ){ printf("NTSC_M     \n"); }
  if( id & V4L2_STD_NTSC_M_JP   ){ printf("NTSC_M_JP  \n"); }
  if( id & V4L2_STD_NTSC_443    ){ printf("NTSC_443   \n"); }
  if( id & V4L2_STD_NTSC_M_KR   ){ printf("NTSC_M_KR  \n"); }

  if( id & V4L2_STD_SECAM_B     ){ printf("SECAM_B    \n"); }
  if( id & V4L2_STD_SECAM_D     ){ printf("SECAM_D    \n"); }
  if( id & V4L2_STD_SECAM_G     ){ printf("SECAM_G    \n"); }
  if( id & V4L2_STD_SECAM_H     ){ printf("SECAM_H    \n"); }
  if( id & V4L2_STD_SECAM_K     ){ printf("SECAM_K    \n"); }
  if( id & V4L2_STD_SECAM_K1    ){ printf("SECAM_K1   \n"); }
  if( id & V4L2_STD_SECAM_L     ){ printf("SECAM_L    \n"); }
  if( id & V4L2_STD_SECAM_LC    ){ printf("SECAM_LC   \n"); }

  if( id & V4L2_STD_ATSC_8_VSB  ){ printf("ATSC_8_VSB \n"); }
  if( id & V4L2_STD_ATSC_16_VSB ){ printf("ATSC_16_VSB\n"); }

  return id;
}

/**
 */
int capClose(psCapDev pCap)
{
  enum v4l2_buf_type  type;
  int                 i;

  DBG_PRINT("pCap [%p]\n", pCap);
  type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  ioctl(pCap->fd, VIDIOC_STREAMOFF, &type);
  for(i=0; i<pCap->nbuf; i++) {
    if(pCap->buf[i].start){
      munmap(pCap->buf[i].start, pCap->buf[i].length);
      pCap->buf[i].start = 0;
    }
  }
  close(pCap->fd);
  free(pCap);
  return 0;
}

/**
 */
struct v4l2_buffer* _capBufGet(psCapDev pCap)
{
  static struct v4l2_buffer buf;
  int             ret;
  fd_set          rfds;
  struct timeval  to;

  memset(&buf, 0, sizeof(struct v4l2_buffer));
  buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;



       FD_ZERO(&rfds);
       FD_SET(pCap->fd, &rfds);
       to.tv_sec  = 1;
       to.tv_usec = 0;
  ret = select(pCap->fd+1, &rfds, NULL, NULL, &to);
DBG_PRINT("select return %d to %d %d.\n", ret, to.tv_sec, to.tv_usec);
  if(ret>0){
    ret = ioctl(pCap->fd, VIDIOC_DQBUF, &buf);
    if(ret<0){
      DBG_ERROR("VIDIOC_DQBUF failed (%d).\n", ret);
      return NULL;
    }
  }else{
    DBG_ERROR("VIDIOC_DQBUF timed out.\n");
    return NULL;
  }
  return &buf;
}

/**
 */
int capBufPut(psCapDev pCap, struct v4l2_buffer* pBuf)
{
  // Enqueue output buffer
  int ret;
  ret = ioctl(pCap->fd, VIDIOC_QBUF, pBuf);
  if(ret<0){
    DBG_ERROR("VIDIOC_QBUF failed (%d)\n", ret);
    return -1;
  }
  return 0;
}
