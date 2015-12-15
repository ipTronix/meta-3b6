/**
 * Copyright 2007-2013 Freescale Semiconductor, Inc. All rights reserved.
 * 
 * The code contained herein is licensed under the GNU General Public
 * License. You may obtain a copy of the GNU General Public License
 * Version 2 or later at the following locations:
 *
 * http://www.opensource.org/licenses/gpl-license.html
 * http://www.gnu.org/copyleft/gpl.html
 *
 * 
 * @file mxc_v4l2_tvin.c
 *
 * @brief Mxc TVIN For Linux 2 driver test application
 *
 */

#ifdef __cplusplus
extern "C"{
#endif

/*============================================================================
                                INCLUDE FILES
============================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <stdint.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <string.h>
#include <malloc.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <asm/types.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <linux/videodev2.h>

#include "capture.h"

/**/
#define RNG_SIZE 4
typedef struct {
  int         idxi;
  int         idxo;
  int         size;
  int         item[RNG_SIZE];
}sRng, *psRng;

int rngPut(psRng pRng, int item)
{
  //if(((pRng->idxi+1)%pRng->size)==pRng->idxo){
  //  return -1;
  //}
  pRng->item[pRng->idxi] = item;
  pRng->idxi++;
  pRng->idxi %= pRng->size;
  return 0;
}

int rngGet(psRng pRng, int* item)
{
  if(pRng->idxo==pRng->idxi){
    return -1;
  }
  if(pRng->idxi){
    pRng->idxo = pRng->idxi-1;
  }else{
    pRng->idxo = pRng->size-1;
  }
  *item = pRng->item[pRng->idxo];
  pRng->idxo++;
  pRng->idxo %= pRng->size;
  return 0;//(pRng->idxi - pRng->idxo + pRng->size) % pRng->size;
}

/**/

#define TFAIL -1
#define TPASS 0

struct testbuffer {
  unsigned char  *start;
  size_t          offset;
  unsigned int    length;
};

char v4l_output_dev[100] = "/dev/video17";
int fd_output_v4l = 0;

int g_fmt = V4L2_PIX_FMT_UYVY;
int g_rotate = 0;
int g_vflip = 0;
int g_hflip = 0;
int g_vdi_enable = 0;
int g_vdi_motion = 0;
int g_tb = 0;
int g_output_num_buffers = 4;
int g_display_width = 0;
int g_display_height = 0;
int g_display_top = 0;
int g_display_left = 0;
int g_frame_size;
int g_frame_period = 33333;
v4l2_std_id g_current_std = V4L2_STD_PAL;

struct testbuffer output_buffers[4];

typedef struct {
  char        name[100];
  int         fd;
}sOutDev, *psOutDev;

typedef struct {
  pthread_t   tid;
  psCapDev    pCap;
  int         capIdx;
  psOutDev    pOut;
}sTaskArg, *psTaskArg;

#define TVIN_CAP_TOT 4
sCapDev   capture[TVIN_CAP_TOT];

sRng rng[TVIN_CAP_TOT] =
{
  {.idxi=0, .idxo=0, .size=RNG_SIZE},
  {.idxi=0, .idxo=0, .size=RNG_SIZE},
  {.idxi=0, .idxo=0, .size=RNG_SIZE},
  {.idxi=0, .idxo=0, .size=RNG_SIZE},
};

void* tv_task_mst(void* arg);
void* tv_task_slv(void* arg);

int process_cmdline(int argc, char **argv);

/**
 */
int output_prepare(void)
{
  int i;
  struct v4l2_buffer output_buf;

  for(i=0; i<g_output_num_buffers; i++){
    memset(&output_buf, 0, sizeof(output_buf));
    output_buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    output_buf.memory = V4L2_MEMORY_MMAP;
    output_buf.index  = i;
    if (ioctl(fd_output_v4l, VIDIOC_QUERYBUF, &output_buf) < 0) {
      printf("VIDIOC_QUERYBUF error\n");
      return TFAIL;
    }

    output_buffers[i].length = output_buf.length;
    output_buffers[i].offset = (size_t) output_buf.m.offset;
    output_buffers[i].start  = mmap(NULL, output_buffers[i].length,
                                    PROT_READ | PROT_WRITE, MAP_SHARED,
                                    fd_output_v4l, output_buffers[i].offset);
    if(output_buffers[i].start == NULL){
      printf("v4l2 tvin test: output mmap failed\n");
      return TFAIL;
    }
    printf("output_prepare buf #%d offset=%p addr=%p len=%08X\n",
           i,
           output_buffers[i].offset,
           output_buffers[i].start ,
           output_buffers[i].length);
  }
  return 0;
}

/**
 */
int output_setup(int width, int height)
{
  struct v4l2_control         ctrl;
  struct v4l2_format          fmt;
  struct v4l2_framebuffer     fb;
  struct v4l2_cropcap         cropcap;
  struct v4l2_crop            crop;
  struct v4l2_capability      cap;
  struct v4l2_fmtdesc         fmtdesc;
  struct v4l2_requestbuffers  buf_req;

  if(!ioctl(fd_output_v4l, VIDIOC_QUERYCAP, &cap)){
    printf("driver=%s, card=%s, bus=%s, "
            "version=0x%08x, "
            "capabilities=0x%08x\n",
            cap.driver, cap.card, cap.bus_info,
            cap.version,
            cap.capabilities);
  }

  fmtdesc.index = 0;
  fmtdesc.type  = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  while(!ioctl(fd_output_v4l, VIDIOC_ENUM_FMT, &fmtdesc)){
    printf("fmt %s: fourcc = 0x%08x\n",
            fmtdesc.description,
            fmtdesc.pixelformat);
    fmtdesc.index++;
  }

  memset(&cropcap, 0, sizeof(cropcap));
  cropcap.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if(ioctl(fd_output_v4l, VIDIOC_CROPCAP, &cropcap) < 0){
    printf("get crop capability failed\n");
    close(fd_output_v4l);
    return TFAIL;
  }

  crop.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  crop.c.top    = g_display_top;//TODO
  crop.c.left   = g_display_left;
  crop.c.width  = g_display_width;
  crop.c.height = g_display_height;
  if(ioctl(fd_output_v4l, VIDIOC_S_CROP, &crop) < 0){
    printf("set crop failed\n");
    close(fd_output_v4l);
    return TFAIL;
  }

  // Set rotation
  ctrl.id    = V4L2_CID_ROTATE;
  ctrl.value = g_rotate; //TODO
  if(ioctl(fd_output_v4l, VIDIOC_S_CTRL, &ctrl) < 0){
    printf("set ctrl rotate failed\n");
    close(fd_output_v4l);
    return TFAIL;
  }
  ctrl.id    = V4L2_CID_VFLIP;
  ctrl.value = g_vflip;//TODO
  if(ioctl(fd_output_v4l, VIDIOC_S_CTRL, &ctrl) < 0){
    printf("set ctrl vflip failed\n");
    close(fd_output_v4l);
    return TFAIL;
  }
  ctrl.id    = V4L2_CID_HFLIP;
  ctrl.value = g_hflip;//TODO
  if (ioctl(fd_output_v4l, VIDIOC_S_CTRL, &ctrl) < 0) {
    printf("set ctrl hflip failed\n");
    close(fd_output_v4l);
    return TFAIL;
  }
/*
  if (g_vdi_enable) {//TODO
    ctrl.id = V4L2_CID_MXC_MOTION;
    ctrl.value = g_vdi_motion; //TODO
    if (ioctl(fd_output_v4l, VIDIOC_S_CTRL, &ctrl) < 0) {
      printf("set ctrl motion failed\n");
      close(fd_output_v4l);
      return TFAIL;
    }
  }
*/
  fb.flags = V4L2_FBUF_FLAG_OVERLAY;
  ioctl(fd_output_v4l, VIDIOC_S_FBUF, &fb);

  memset(&fmt, 0, sizeof(fmt));
  fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  fmt.fmt.pix.width        = width;
  fmt.fmt.pix.height       = height;
  fmt.fmt.pix.pixelformat  = g_fmt;
  fmt.fmt.pix.bytesperline = width;
  fmt.fmt.pix.priv         = 0;
  fmt.fmt.pix.sizeimage    = 0;
  if(g_tb){ //TODO
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED_TB;
  }else{
    fmt.fmt.pix.field = V4L2_FIELD_INTERLACED_BT;
  }
  if(ioctl(fd_output_v4l, VIDIOC_S_FMT, &fmt) < 0){
    printf("set format failed\n");
    return TFAIL;
  }

  if(ioctl(fd_output_v4l, VIDIOC_G_FMT, &fmt) < 0){
    printf("get format failed\n");
    return TFAIL;
  }
  g_frame_size = fmt.fmt.pix.sizeimage; //TODO
  printf("g_frame_size %d\n", g_frame_size);

  memset(&buf_req, 0, sizeof(buf_req));
  buf_req.count  = g_output_num_buffers; //TODO
  buf_req.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  buf_req.memory = V4L2_MEMORY_MMAP;
  if(ioctl(fd_output_v4l, VIDIOC_REQBUFS, &buf_req) < 0){
    printf("request buffers failed\n");
    return TFAIL;
  }

  return 0;
}

int tv_stdCheck()
{
#if 0
//TODO gestione cambio standard
{
v4l2_std_id         std;
char* stdname;

  if(ioctl(pCap->fd, VIDIOC_G_STD, &std)){
    printf("VIDIOC_G_STD failed.\n");
    return NULL;
  }
  if(g_current_std!=std){
    g_current_std = std;
    switch(std){
    case V4L2_STD_PAL : stdname = "V4L2_STD_PAL" ; break;
    case V4L2_STD_NTSC: stdname = "V4L2_STD_NTSC"; break;
    default           : stdname = "UNKNOWN"      ; break;
    }
    printf("tvin:std is changed to %s\n", stdname);

    printf("tvin:capture_stop()\n");
    capture_stop(pCap);
    printf("tvin:capture_setup()\n");
    if(capture_setup(pCap)){
      printf("Setup v4l capture failed.\n");
      return NULL;
    }
/**/
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ioctl(fd_output_v4l, VIDIOC_STREAMOFF, &type);

    printf("tvin:output munmap()\n");
    for(i=0; i<g_output_num_buffers; i++){
      munmap(output_buffers[i].start, output_buffers[i].length);
    }
    printf("tvin:output_setup()\n");
    if(output_setup(pCap->width*2, pCap->height)<0){
      printf("Setup v4l output failed.\n");
      return NULL;
    }
    printf("tvin:output_prepare()\n");
    if(output_prepare()<0){
      printf("output_prepare failed\n");
      return NULL;
    }
    for(i=0; i<g_output_num_buffers; i++){
      struct v4l2_buffer  output_buf;
      memset(&output_buf, 0, sizeof(output_buf));
      output_buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
      output_buf.memory = V4L2_MEMORY_MMAP;
      output_buf.index  = i;
      printf("VIDIOC_QBUF #%d output.\n", i);
      ret = ioctl(fd_output_v4l, VIDIOC_QBUF, &output_buf);
      if(ret < 0){
        printf("VIDIOC_QBUF #%d output failed (%d).\n", i, ret);
        return NULL;
      }
    }
    type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    ioctl(fd_output_v4l, VIDIOC_STREAMOFF, &type);
/**/
    printf("tvin:capture_start()\n");
    capture_start(pCap);
  }
}
#endif
  return 0;
}

/**
 */
void* tv_task_mst(void* arg)
{
  psTaskArg           pTask = (psTaskArg)arg;
  psCapDev            pCap = pTask->pCap;
  struct v4l2_buffer  capture_buf;
  struct v4l2_buffer  output_buf;
  enum v4l2_buf_type  type;
  unsigned char      *ip;
  unsigned char      *op;
  int                 i;
  int                 j;
  int                 ret;
  int                 capBufIdx[TVIN_CAP_TOT];
  unsigned long       flag[TVIN_CAP_TOT];

  printf("thread mst:started\n");
  // set internal variables
  for(i=0; i<TVIN_CAP_TOT; i++){
    capBufIdx[i] = 0;
    flag[i] = 0;
  }
  // Start capture
  if(capture_start(pCap)<0){
    printf("thread mst:ERROR:capture_start %s failed\n", pCap->name);
    return NULL;
  }

  // thread loop
  for(;;){
    // Dequeue capture buffer
    memset(&capture_buf, 0, sizeof(capture_buf));
    capture_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    capture_buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(pCap->fd, VIDIOC_DQBUF, &capture_buf);
    if(ret<0){
      printf( "thread #%d:ERROR:VIDIOC_DQBUF capture %s failed (%d).\n",
              pTask->capIdx, pCap->name, ret);
      return NULL;
    }
//    printf( "thread mst: buffer index %d\n", capture_buf.index);

    // Dequeue output buffer
    memset(&output_buf, 0, sizeof(output_buf));
    output_buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    output_buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(fd_output_v4l, VIDIOC_DQBUF, &output_buf);
    if(ret<0){
      printf( "thread mst:ERROR:VIDIOC_DQBUF output failed (%d).\n", ret);
      return NULL;
    }
    // set output buffer address
    op = output_buffers[output_buf.index].start;
    // copy main thread capture buffer
    ip = pCap->buf[capture_buf.index].start;
    for(j=0; j<pCap->height; j++){
      memcpy(op, ip, pCap->bpl);
      ip += pCap->bpl;
      op += pCap->bpl<<1;
    }
    // get other threads capture buffer
    for(i=1; i<TVIN_CAP_TOT; i++){
      ret = rngGet(&rng[i], &(capBufIdx[i]));
      if(ret){
//        printf( "thread mst: ring[%d] is empty\n", i);
      }else{
//        printf( "thread mst: ring[%d] return index %d\n", i, capBufIdx[i]);
        flag[i] |= 0x00000001;
      }
      // copy other threads capture buffer
      if(capBufIdx[i]>=0 && capBufIdx[i]<pCap->nbuf){
        ip = capture[i].buf[capBufIdx[i]].start;
        op = output_buffers[output_buf.index].start;
        switch(i){
        case 0: break;
        case 1: op += pCap->bpl; break;
        case 2: op += pCap->imsize*2; break;
        case 3: op += pCap->imsize*2 + pCap->bpl; break;
        }
        for(j=0; j<pCap->height; j++){
          memcpy(op, ip, pCap->bpl);
          ip += pCap->bpl;
          op += pCap->bpl<<1;
        }
      }
    }
    // Enqueue output buffer
    ret = ioctl(fd_output_v4l, VIDIOC_QBUF, &output_buf);
    if(ret<0){
      printf("thread mst: VIDIOC_QBUF output failed (%d)\n", ret);
      return NULL;
    }
    // Enqueue main capture buffer
    ret = ioctl(pCap->fd, VIDIOC_QBUF, &capture_buf);
    if(ret<0){
//      printf( "thread mst: VIDIOC_QBUF capture[0] index %d failed (%d)\n",
//              capBufIdx[i], ret);
      return NULL;
    }
    // Enqueue other threads capture buffer
    for(i=1; i<TVIN_CAP_TOT; i++){
      if(flag[i]&0x00000001){
        if(capBufIdx[i]>=0 && capBufIdx[i]<capture[i].nbuf){
//          printf( "thread mst: QBUF capture[%d] fd %d index %d\n",
//                  i, capture[i].fd, capBufIdx[i]);
          capture_buf.index  = capBufIdx[i];
          ret = ioctl(capture[i].fd, VIDIOC_QBUF, &capture_buf);
          if(ret<0){
            printf( "thread mst:ERROR: VIDIOC_QBUF capture[%d] index %d failed (%d %s)\n",
                    i, capBufIdx[i], errno, strerror(errno));
            return NULL;
          }
        }
        flag[i] &= ~0x00000001;
      }
    }
  }
  return NULL;
}


/**
 */
void* tv_task_slv(void* arg)
{
  psTaskArg           pTask = (psTaskArg)arg;
  psCapDev            pCap = pTask->pCap;
  struct v4l2_buffer  capture_buf;
  enum v4l2_buf_type  type;
  int                 ret;

  printf("thread slv #%d:started: capture device %s\n",
         pTask->capIdx, pCap->name);

  // Start capture
  if(capture_start(pCap)<0){
    printf("thread slv #%d:ERROR:capture_start %s failed\n",
           pTask->capIdx, pCap->name);
    return NULL;
  }

  // thread loop
  for(;;){
    // Dequeue capture buffer
    memset(&capture_buf, 0, sizeof(capture_buf));
    capture_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    capture_buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(pCap->fd, VIDIOC_DQBUF, &capture_buf);
    if(ret<0){
      printf( "thread slv #%d:ERROR:VIDIOC_DQBUF capture %s failed (%d).\n",
              pTask->capIdx, pCap->name, ret);
      return NULL;
    }

//    printf( "thread slv #%d: rngPut index %d\n",
//            pTask->capIdx, capture_buf.index);
    ret = rngPut(&rng[pTask->capIdx], capture_buf.index);
    if(ret){
      printf( "thread slv #%d:ERROR: ring[%d] put buffer overflow\n",
              pTask->capIdx, pTask->capIdx);
    }
  }
  return NULL;
}

#define MXCFB_SET_GBL_ALPHA _IOW('F', 0x21, struct mxcfb_gbl_alpha)
struct mxcfb_gbl_alpha {
  int enable;
  int alpha;
};

/**
 */
int overlay_set(void)
{
  int                     fd;
  char                   *device = "/dev/fb0";
  struct mxcfb_gbl_alpha  alpha;
  int ret;

  fd = open(device, O_RDWR);
  if(fd<0){
    printf("Unable to open frame buffer %s\n", device);
    return -1;
  }
  alpha.enable = 1;
  alpha.alpha  = 0;
  ret = ioctl(fd, MXCFB_SET_GBL_ALPHA, &alpha);
  if(ret<0){
    printf("Set global alpha failed\n");
    close(fd);
    return -1;
  }
  close(fd);
  printf( "overlay_set: global alpha setted to: enable %d alpha %d\n",
          alpha.enable, alpha.alpha);
  return 0;
}

/**
 */
int main(int argc, char **argv)
{
  enum v4l2_buf_type  type;
  sTaskArg            task[TVIN_CAP_TOT];
  int                 i;
  int                 ret;
  char               *cap_device[4];

  printf("%s "__DATE__" "__TIME__"\n", argv[0]);

  if (process_cmdline(argc, argv) < 0) {
    return TFAIL;
  }

  cap_device[0] = "/dev/video0";
  cap_device[1] = "/dev/video1";
  cap_device[2] = "/dev/video2";
  cap_device[3] = "/dev/video3";
  for(i=0; i<TVIN_CAP_TOT; i++){
    printf("Try to open capture device %s\n", cap_device[i]);
    if(capture_open(cap_device[i], &capture[i])){
      printf("Unable to open %s\n", cap_device[i]);
      return TFAIL;
    }
    if(capture_setup(&capture[i])){
      printf("Setup v4l capture failed.\n");
      return TFAIL;
    }
  }

  if ((fd_output_v4l = open(v4l_output_dev, O_RDWR, 0)) < 0) {
    printf("Unable to open %s\n", v4l_output_dev);
    return TFAIL;
  }
  if(output_setup(capture[0].width*2, capture[0].height*2) < 0){
    printf("Setup v4l output failed.\n");
    close(capture[0].fd);
    return TFAIL;
  }
  if (output_prepare() < 0) {
    printf("output_prepare failed\n");
    return TFAIL;
  }

  overlay_set();

  for(i=0; i<g_output_num_buffers; i++){
    struct v4l2_buffer  output_buf;
    memset(&output_buf, 0, sizeof(output_buf));
    output_buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    output_buf.memory = V4L2_MEMORY_MMAP;
    output_buf.index  = i;
    printf("VIDIOC_QBUF #%d output.\n", i);
    ret = ioctl(fd_output_v4l, VIDIOC_QBUF, &output_buf);
    if(ret < 0){
      printf("VIDIOC_QBUF #%d output failed (%d).\n", i, ret);
      return TFAIL;
    }
  }

  printf("start output stream\n");
  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  if(ioctl(fd_output_v4l, VIDIOC_STREAMON, &type) < 0){
    printf("Could not start output stream\n");
    return TFAIL;
  }
  printf("pthread_create master\n");
  i = 0;
  task[i].pCap   = &capture[i];
  task[i].capIdx = 0;
  ret = pthread_create(&task[i].tid, NULL, tv_task_mst, &task[i]);
  if(ret){
    printf("ERROR pthread_create master\n");
    goto exit;
  }
  for(i=1; i<TVIN_CAP_TOT; i++){
    printf("pthread_create slave #%d\n", i);
    task[i].pCap   = &capture[i];
    task[i].capIdx = i;
    ret = pthread_create(&task[i].tid, NULL, tv_task_slv, &task[i]);
    if(ret){
      printf("ERROR pthread_create #%d\n", i);
      goto exit;
    }
  }

  for(;;){
    sleep(1);
  }

exit:
  type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
  ioctl(fd_output_v4l, VIDIOC_STREAMOFF, &type);

  for (i=0; i<g_output_num_buffers; i++) {
    munmap(output_buffers[i].start, output_buffers[i].length);
  }

  close(fd_output_v4l);
  for(i=0; i<TVIN_CAP_TOT; i++){
    capture_close(&capture[i]);
  }
  return 0;
}

/**
 */
int process_cmdline(int argc, char **argv)
{
  int i;

  for (i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-ow") == 0) {
      g_display_width = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-oh") == 0) {
      g_display_height = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-ot") == 0) {
      g_display_top = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-ol") == 0) {
      g_display_left = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-r") == 0) {
      g_rotate = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-f") == 0) {
      i++;
      g_fmt = v4l2_fourcc(argv[i][0], argv[i][1], argv[i][2], argv[i][3]);
      if ((g_fmt != V4L2_PIX_FMT_NV12) &&
          (g_fmt != V4L2_PIX_FMT_UYVY) &&
          (g_fmt != V4L2_PIX_FMT_YUYV) &&
          (g_fmt != V4L2_PIX_FMT_YUV420)) {
          printf("Default format is used: UYVY\n");
      }
    }
    else if (strcmp(argv[i], "-m") == 0) {
      g_vdi_enable = 1;
      g_vdi_motion = atoi(argv[++i]);
    }
    else if (strcmp(argv[i], "-tb") == 0) {
      g_tb = 1;
    }
    else if (strcmp(argv[i], "-help") == 0) {
      printf("MXC Video4Linux TVin Test\n\n" \
          "Syntax: mxc_v4l2_tvin.out\n" \
          " -ow <capture display width>\n" \
          " -oh <capture display height>\n" \
          " -ot <display top>\n" \
          " -ol <display left>\n" \
          " -r <rotation> -c <capture counter> \n"
          " -m <motion> 0:medium 1:low 2:high, 0-default\n"
          " -tb top field first, bottom field first-default\n"
          " -f <format, only YU12, YUYV, UYVY and NV12 are supported> \n"
          " -d <device> \n");
      return TFAIL;
    }
  }
  if ((g_display_width == 0) || (g_display_height == 0)) {
    printf("Zero display width or height\n");
    return TFAIL;
  }
  return 0;
}
