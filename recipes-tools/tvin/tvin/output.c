/**
 */
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "output.h"

#define DEBUG

/**
 */
int output_open(char* name, psOutDev pOut)
{
  memset(pOut, 0, sizeof(sOutDev));
  pOut->fd = open(name, O_RDWR, 0);
  if(pOut->fd<0){
    printf("capture_open:Unable to open %s\n", name);
    return -1;
  }
  strncpy(pOut->name, name, sizeof(pOut->name));
  pOut->nbuf  = OUT_NUM_BUF;
  pOut->std   = V4L2_STD_PAL;
  pOut->fmt   = V4L2_PIX_FMT_UYVY;
  printf("output_open:Device %s opened fd is %d\n", name, pOut->fd);
  return 0;
}

/**
 */
int output_prepare(psOutDev pOut)
{
  int i;
  struct v4l2_buffer output_buf;

  for(i=0; i<pOut->nbuf; i++){
    memset(&output_buf, 0, sizeof(output_buf));
    output_buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    output_buf.memory = V4L2_MEMORY_MMAP;
    output_buf.index  = i;
    if (ioctl(pOut->fd, VIDIOC_QUERYBUF, &output_buf) < 0) {
      printf("VIDIOC_QUERYBUF error\n");
      return -1;
    }

    pOut->buf[i].length = output_buf.length;
    pOut->buf[i].offset = (size_t) output_buf.m.offset;
    pOut->buf[i].start  = mmap(NULL, pOut->buf[i].length,
                               PROT_READ | PROT_WRITE, MAP_SHARED,
                               pOut->fd, pOut->buf[i].offset);
    if(pOut->buf[i].start == NULL){
      printf("v4l2 tvin test: output mmap failed\n");
      return -1;
    }
    printf("output_prepare buf #%d offset=%p addr=%p len=%08X\n",
           i,
           pOut->buf[i].offset,
           pOut->buf[i].start ,
           pOut->buf[i].length);
  }
  return 0;
}

/**
 * devono essere inizializzate:
 * pOut->fmt
 * tb
 * hflip
 * vflip
 * rotate
 */
int output_setup(psOutDev pOut)
{
  struct v4l2_control         ctrl;
  struct v4l2_format          fmt;
  struct v4l2_framebuffer     fb;
  struct v4l2_cropcap         cropcap;
  struct v4l2_crop            crop;
  struct v4l2_capability      cap;
  struct v4l2_requestbuffers  buf_req;

  if(!ioctl(pOut->fd, VIDIOC_QUERYCAP, &cap)){
    printf("driver=%s, card=%s, bus=%s, version=0x%08x, capabilities=0x%08x\n",
            cap.driver, cap.card, cap.bus_info,
            cap.version,
            cap.capabilities);
  }
#ifdef DEBUG
  struct v4l2_fmtdesc fmtdesc;
  
  fmtdesc.index = 0;
  fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  while(!ioctl(pOut->fd, VIDIOC_ENUM_FMT, &fmtdesc)){
    printf("fmt %s: fourcc = 0x%08x\n",
            fmtdesc.description,
            fmtdesc.pixelformat);
    fmtdesc.index++;
  }
#endif

  memset(&cropcap, 0, sizeof(cropcap));
  cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if(ioctl(pOut->fd, VIDIOC_CROPCAP, &cropcap) < 0){
    printf("get crop capability failed\n");
    close(pOut->fd);
    return -1;
  }

  crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  crop.c.top    = pOut->disp_top;
  crop.c.left   = pOut->disp_left;
  crop.c.width  = pOut->disp_width;
  crop.c.height = pOut->disp_height;
  if(ioctl(pOut->fd, VIDIOC_S_CROP, &crop) < 0){
    printf("set crop failed\n");
    close(pOut->fd);
    return -1;
  }

  // Set rotation
  ctrl.id    = V4L2_CID_ROTATE;
  ctrl.value = pOut->rotate;
  if(ioctl(pOut->fd, VIDIOC_S_CTRL, &ctrl) < 0){
    printf("set ctrl rotate failed\n");
    close(pOut->fd);
    return -1;
  }
  ctrl.id    = V4L2_CID_VFLIP;
  ctrl.value = pOut->vflip;
  if(ioctl(pOut->fd, VIDIOC_S_CTRL, &ctrl) < 0){
    printf("set ctrl vflip failed\n");
    close(pOut->fd);
    return -1;
  }
  ctrl.id    = V4L2_CID_HFLIP;
  ctrl.value = pOut->hflip;
  if (ioctl(pOut->fd, VIDIOC_S_CTRL, &ctrl) < 0) {
    printf("set ctrl hflip failed\n");
    close(pOut->fd);
    return -1;
  }
/*
  if (g_vdi_enable) {
    ctrl.id = V4L2_CID_MXC_MOTION;
    ctrl.value = g_vdi_motion;
    if (ioctl(fd_output_v4l, VIDIOC_S_CTRL, &ctrl) < 0) {
      printf("set ctrl motion failed\n");
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
    printf("set format failed\n");
    return -1;
  }
  if(ioctl(pOut->fd, VIDIOC_G_FMT, &fmt) < 0){
    printf("get format failed\n");
    return -1;
  }
  pOut->imsize = fmt.fmt.pix.sizeimage;
  printf("frame_size %d\n", pOut->imsize);

  memset(&buf_req, 0, sizeof(buf_req));
  buf_req.count  = pOut->nbuf;
  buf_req.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  buf_req.memory = V4L2_MEMORY_MMAP;
  if(ioctl(pOut->fd, VIDIOC_REQBUFS, &buf_req) < 0){
    printf("request buffers failed\n");
    return -1;
  }

  return 0;
}

int output_start(psOutDev pOut)
{
  enum v4l2_buf_type  type;
  int                 i;
  int                 ret;

  printf("enqueue output buffers\n");
  for(i=0; i<pOut->nbuf; i++){
    struct v4l2_buffer  output_buf;
    memset(&output_buf, 0, sizeof(output_buf));
    output_buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    output_buf.memory = V4L2_MEMORY_MMAP;
    output_buf.index  = i;
    printf("VIDIOC_QBUF #%d output.\n", i);
    ret = ioctl(pOut->fd, VIDIOC_QBUF, &output_buf);
    if(ret < 0){
      printf("VIDIOC_QBUF #%d output failed (%d).\n", i, ret);
      return -1;
    }
  }

  printf("start output stream\n");
  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if(ioctl(pOut->fd, VIDIOC_STREAMON, &type) < 0){
    printf("Could not start output stream\n");
    return -1;
  }

  return 0;
}

int output_stop(psOutDev pOut)
{
  enum v4l2_buf_type  type;

  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  ioctl(pOut->fd, VIDIOC_STREAMOFF, &type);

  return 0;
}

int output_close(psOutDev pOut)
{
  int i;

  for(i=0; i<pOut->nbuf; i++){
    munmap(pOut->buf[i].start, pOut->buf[i].length);
  }
  close(pOut->fd);

  return 0;
}
