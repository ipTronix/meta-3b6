/** ***************************************************************************
 * @file out.c
 * @brief Video for linux output handling
 **************************************************************************** */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "out.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "OUT"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif

/**
 */
uintptr_t vtop(uintptr_t vaddr)
{
  FILE     *pagemap;
  uintptr_t paddr = 0;
  int       offset = (vaddr / sysconf(_SC_PAGESIZE)) * sizeof(uint64_t);
  uint64_t  e;

  // https://www.kernel.org/doc/Documentation/vm/pagemap.txt
  if ((pagemap = fopen("/proc/self/pagemap", "r"))) {
    if (lseek(fileno(pagemap), offset, SEEK_SET) == offset) {
      if (fread(&e, sizeof(uint64_t), 1, pagemap)) {
        if (e & (1ULL << 63)) { // page present ?
          paddr = e & ((1ULL << 54) - 1); // pfn mask
          paddr = paddr * sysconf(_SC_PAGESIZE);
          // add offset within page
          paddr = paddr | (vaddr & (sysconf(_SC_PAGESIZE) - 1));
        }
      }
    }
    fclose(pagemap);
  }
  return paddr;
}

/**
 */
psOutDev outOpen(char* name)
{
  psOutDev  pOut;

  pOut = (psOutDev)malloc(sizeof(sOutDev));
  if(!pOut){
    return NULL;
  }
  memset(pOut, 0, sizeof(sOutDev));
  pOut->fd = open(name, O_RDWR, 0);
  if(pOut->fd<0){
    DBG_ERROR("Unable to open %s\n", name);
    free(pOut);
    return NULL;
  }
  strncpy(pOut->name, name, sizeof(pOut->name));
  pOut->nbuf  = OUT_NUM_BUF;
  pOut->std   = V4L2_STD_PAL;
  pOut->fmt   = V4L2_PIX_FMT_UYVY;
  DBG_PRINT("Device %s opened fd is %d\n", name, pOut->fd);
  return pOut;
}

/**
 * devono essere inizializzate:
 * pOut->fmt
 * tb
 * hflip
 * vflip
 * rotate
 */
int outSetup(psOutDev pOut)
{
  struct v4l2_control         ctrl;
  struct v4l2_format          fmt;
  struct v4l2_framebuffer     fb;
  struct v4l2_cropcap         cropcap;
  struct v4l2_crop            crop;
  struct v4l2_capability      cap;
  struct v4l2_requestbuffers  buf_req;

  if(!ioctl(pOut->fd, VIDIOC_QUERYCAP, &cap)){
    DBG_PRINT("driver=%s, card=%s, bus=%s, version=0x%08x, capabilities=0x%08x\n",
              cap.driver, cap.card, cap.bus_info,
              cap.version,
              cap.capabilities);
  }
#ifdef DEBUG
  struct v4l2_fmtdesc fmtdesc;

  fmtdesc.index = 0;
  fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  while(!ioctl(pOut->fd, VIDIOC_ENUM_FMT, &fmtdesc)){
    DBG_PRINT("fmt %s: fourcc = 0x%08x\n",
              fmtdesc.description,
              fmtdesc.pixelformat);
    fmtdesc.index++;
  }
#endif

  memset(&cropcap, 0, sizeof(cropcap));
  cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if(ioctl(pOut->fd, VIDIOC_CROPCAP, &cropcap) < 0){
    DBG_ERROR("get crop capability failed\n");
    close(pOut->fd);
    return -1;
  }

  crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  crop.c.top    = pOut->disp_top;
  crop.c.left   = pOut->disp_left;
  crop.c.width  = pOut->disp_width;
  crop.c.height = pOut->disp_height;
  if(ioctl(pOut->fd, VIDIOC_S_CROP, &crop) < 0){
    DBG_ERROR("set crop failed\n");
    close(pOut->fd);
    return -1;
  }

  // Set rotation
  ctrl.id    = V4L2_CID_ROTATE;
  ctrl.value = pOut->rotate;
  if(ioctl(pOut->fd, VIDIOC_S_CTRL, &ctrl) < 0){
    DBG_ERROR("set ctrl rotate failed\n");
    close(pOut->fd);
    return -1;
  }
  ctrl.id    = V4L2_CID_VFLIP;
  ctrl.value = pOut->vflip;
  if(ioctl(pOut->fd, VIDIOC_S_CTRL, &ctrl) < 0){
    DBG_ERROR("set ctrl vflip failed\n");
    close(pOut->fd);
    return -1;
  }
  ctrl.id    = V4L2_CID_HFLIP;
  ctrl.value = pOut->hflip;
  if (ioctl(pOut->fd, VIDIOC_S_CTRL, &ctrl) < 0) {
    DBG_ERROR("set ctrl hflip failed\n");
    close(pOut->fd);
    return -1;
  }
/*
  if (g_vdi_enable) {
    ctrl.id = V4L2_CID_MXC_MOTION;
    ctrl.value = g_vdi_motion;
    if (ioctl(fd_output_v4l, VIDIOC_S_CTRL, &ctrl) < 0) {
      DBG_ERROR("set ctrl motion failed\n");
      close(fd_output_v4l);
      return -1;
    }
  }
*/
  fb.flags = V4L2_FBUF_FLAG_OVERLAY;
  ioctl(pOut->fd, VIDIOC_S_FBUF, &fb);

  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  fmt.fmt.pix.width        = pOut->width;
  fmt.fmt.pix.height       = pOut->height;
  fmt.fmt.pix.pixelformat  = pOut->fmt;
  fmt.fmt.pix.bytesperline = pOut->width;
  fmt.fmt.pix.priv         = 0;
  fmt.fmt.pix.sizeimage    = 0;
  if(pOut->tb){
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED_TB;
  }else{
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED_BT;
  }
  if(ioctl(pOut->fd, VIDIOC_S_FMT, &fmt) < 0){
    DBG_ERROR("set format failed\n");
    return -1;
  }
  if(ioctl(pOut->fd, VIDIOC_G_FMT, &fmt) < 0){
    DBG_ERROR("get format failed\n");
    return -1;
  }
  pOut->imsize = fmt.fmt.pix.sizeimage;
  DBG_PRINT("frame_size %d\n", pOut->imsize);

#if 0
  int i;
  for(i=0; i<pOut->nbuf; i++){
    pOut->buf[i].start = malloc(pOut->imsize);
    if(!pOut->buf[i].start){
      DBG_ERROR("allocation for V4L2_MEMORY_USERPTR buffers failed\n");
      return -1;
    }
    pOut->buf[i].offset = vtop(pOut->buf[i].start);
DBG_PRINT(">>>>>>>> V4L2_MEMORY_USERPTR %d start %p, offset %p,  phys %p\n",
          i, pOut->buf[i].start, pOut->buf[i].offset, vtop(pOut->buf[i].start));
  }
#endif
  memset(&buf_req, 0, sizeof(buf_req));
  buf_req.count  = pOut->nbuf;
  buf_req.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
#if 0
  buf_req.memory = V4L2_MEMORY_USERPTR;
#else
  buf_req.memory = V4L2_MEMORY_MMAP;
#endif
  if(ioctl(pOut->fd, VIDIOC_REQBUFS, &buf_req) < 0){
    DBG_ERROR("request buffers failed\n");
    return -1;
  }

  return 0;
}

/**
 */
int outPrepare(psOutDev pOut)
{
#if 0
#else
  int                 i;
  struct v4l2_buffer  output_buf;

  for(i=0; i<pOut->nbuf; i++){
    memset(&output_buf, 0, sizeof(output_buf));
    output_buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    output_buf.memory = V4L2_MEMORY_MMAP;
    output_buf.index  = i;
    if (ioctl(pOut->fd, VIDIOC_QUERYBUF, &output_buf) < 0) {
      DBG_ERROR("VIDIOC_QUERYBUF failed\n");
      return -1;
    }

    pOut->buf[i].length = output_buf.length;
    pOut->buf[i].offset = (size_t) output_buf.m.offset;
    pOut->buf[i].start  = mmap(NULL, pOut->buf[i].length,
                               PROT_READ | PROT_WRITE, MAP_SHARED,
                               pOut->fd, pOut->buf[i].offset);
    if(pOut->buf[i].start == NULL){
      DBG_ERROR("v4l2 tvin test: output mmap failed\n");
      return -1;
    }
    DBG_PRINT("buf #%d offset=%p addr=%p len=%08X\n",
           i,
           pOut->buf[i].offset,
           pOut->buf[i].start ,
           pOut->buf[i].length);
  }
#endif
  return 0;
}

/**
 */
int outStart(psOutDev pOut)
{
  enum v4l2_buf_type  type;
  int                 i;
  int                 ret;

  DBG_PRINT("enqueue output buffers\n");
  for(i=0; i<pOut->nbuf; i++){
    struct v4l2_buffer  output_buf;
    memset(&output_buf, 0, sizeof(output_buf));
    output_buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
#if 0
    output_buf.memory = V4L2_MEMORY_USERPTR;
    output_buf.m.userptr = (unsigned long)pOut->buf[i].start;
#else
    output_buf.memory = V4L2_MEMORY_MMAP;
#endif
    output_buf.index  = i;
    DBG_PRINT("VIDIOC_QBUF #%d output.\n", i);
    ret = ioctl(pOut->fd, VIDIOC_QBUF, &output_buf);
    if(ret < 0){
      DBG_ERROR("VIDIOC_QBUF #%d output failed (%d) %d %s.\n",
                i, ret, errno, strerror(errno));
      return -1;
    }
  }

  DBG_PRINT("start output stream\n");
  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if(ioctl(pOut->fd, VIDIOC_STREAMON, &type) < 0){
    DBG_ERROR("Could not start output stream\n");
    return -1;
  }

  return 0;
}

/**
 */
int outStop(psOutDev pOut)
{
  enum v4l2_buf_type  type;

  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  ioctl(pOut->fd, VIDIOC_STREAMOFF, &type);

  return 0;
}

/**
 */
int outClose(psOutDev pOut)
{
  int i;

  for(i=0; i<pOut->nbuf; i++){
#if 0
    free(pOut->buf[i].start);
#else
    munmap(pOut->buf[i].start, pOut->buf[i].length);
#endif
  }
  close(pOut->fd);
  free(pOut);
  return 0;
}

/**
 */
struct v4l2_buffer* outBufGet(psOutDev pOut)
{
  static struct v4l2_buffer buf;
  int ret;
  memset(&buf, 0, sizeof(struct v4l2_buffer));
  buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  buf.memory = V4L2_MEMORY_MMAP;
  ret = ioctl(pOut->fd, VIDIOC_DQBUF, &buf);
  if(ret<0){
    DBG_ERROR( "outBufGet:ERROR:VIDIOC_DQBUF failed (%d).\n", ret);
    return NULL;
  }
  return &buf;
}

/**
 */
int outBufPut(psOutDev pOut, struct v4l2_buffer* pBuf)
{
  // Enqueue output buffer
  int ret;
  ret = ioctl(pOut->fd, VIDIOC_QBUF, pBuf);
  if(ret<0){
    DBG_ERROR("outBufPut: VIDIOC_QBUF failed (%d)\n", ret);
    return -1;
  }
  return 0;
}
