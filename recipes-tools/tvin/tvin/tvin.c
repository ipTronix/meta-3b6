/**
 * @file tvin.c
 * @brief TeraHE TVIN driver test application
 */

/**
 * INCLUDE FILES
 */
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
#include "output.h"

/**
 * ring queue definition
 */
#define RNG_SIZE 8
typedef struct {
  int         idxi;
  int         idxo;
  int         size;
  int         item[RNG_SIZE];
}sRng, *psRng;

/**
 */
int rngPut(psRng pRng, int item)
{
  if(((pRng->idxi+1)%pRng->size)==pRng->idxo){
    return -1;
  }
  pRng->item[pRng->idxi] = item;
  pRng->idxi++;
  pRng->idxi %= pRng->size;
  return 0;
}

/**
 */
int rngGet(psRng pRng, int* item)
{
  if(pRng->idxo==pRng->idxi){
    return -1;
  }
  *item = pRng->item[pRng->idxo];
  pRng->idxo++;
  pRng->idxo %= pRng->size;
  return 0;
}

/**
 */
int rngLoad(psRng pRng)
{
  return (pRng->idxi - pRng->idxo + pRng->size) % pRng->size;
}


/**
 */
typedef struct {
  pthread_t   tid;
  psCapDev    pCap;
  int         capIdx;
  psOutDev    pOut;
}sTaskArg, *psTaskArg;

#define TVIN_CAP_TOT 4
sCapDev   capture[TVIN_CAP_TOT];

sOutDev   output;
psOutDev  pOut = &output;

sRng rng[TVIN_CAP_TOT] =
{
  {.idxi=0, .idxo=0, .size=RNG_SIZE},
  {.idxi=0, .idxo=0, .size=RNG_SIZE},
  {.idxi=0, .idxo=0, .size=RNG_SIZE},
  {.idxi=0, .idxo=0, .size=RNG_SIZE},
};

void* tv_task_mst(void* arg);
void* tv_task_slv(void* arg);

/**
 */
void* tv_task_mst(void* arg)
{
  psTaskArg           pTask = (psTaskArg)arg;
  psCapDev            pCap = pTask->pCap;
  psOutDev            pOut = pTask->pOut;
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

  printf("thread mst:started: output device %s, capture device %s\n",
         pOut->name, pCap->name);
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
    // Dequeue output buffer
    memset(&output_buf, 0, sizeof(output_buf));
    output_buf.type   = V4L2_BUF_TYPE_VIDEO_OUTPUT;
    output_buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(pOut->fd, VIDIOC_DQBUF, &output_buf);
    if(ret<0){
      printf( "thread mst:ERROR:VIDIOC_DQBUF output failed (%d).\n", ret);
      return NULL;
    }
    // set output buffer address
    op = pOut->buf[output_buf.index].start;
    // copy main thread capture buffer to output buffer
    ip = pCap->buf[capture_buf.index].start;
    for(j=0; j<pCap->height; j++){
      memcpy(op, ip, pCap->bpl);
      ip += pCap->bpl;
      op += pCap->bpl<<1;
    }
    // get other threads capture buffer
    for(i=1; i<TVIN_CAP_TOT; i++){
      int temp;
      // Get capture buffers from ring queue and enqueue last used buffer
      ret = 1;
      while(ret){
        temp = capBufIdx[i];
        ret = rngGet(&rng[i], &(capBufIdx[i]));
        if(ret==0){
          if(flag[i]){
            struct v4l2_buffer  cap_buf;
            cap_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            cap_buf.index  = temp;
            cap_buf.memory = V4L2_MEMORY_MMAP;
            ret = ioctl(capture[i].fd, VIDIOC_QBUF, &cap_buf);
          }
          flag[i] |= 0x00000001;
        }
      }
      // copy other threads capture buffer to output buffer
      if(capBufIdx[i]>=0 && capBufIdx[i]<pCap->nbuf){
        ip = capture[i].buf[capBufIdx[i]].start;
        op = pOut->buf[output_buf.index].start;
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
    ret = ioctl(pOut->fd, VIDIOC_QBUF, &output_buf);
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
    }

    // put buffer to ring queue
    ret = rngPut(&rng[pTask->capIdx], capture_buf.index);
    if(ret){
      printf( "thread slv #%d:ERROR: ring[%d] put buffer overflow\n",
              pTask->capIdx, pTask->capIdx);
      ret = ioctl(pCap->fd, VIDIOC_QBUF, &capture_buf);
      if(ret){
        printf( "thread slv #%d:ERROR: VIDIOC_QBUF\n", pTask->capIdx);
      }
    }
  }
  return NULL;
}

/**
 */
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
  char               *out_device;

  printf("%s "__DATE__" "__TIME__"\n", argv[0]);

  cap_device[0] = "/dev/video0";
  cap_device[1] = "/dev/video1";
  cap_device[2] = "/dev/video2";
  cap_device[3] = "/dev/video3";

  for(i=0; i<TVIN_CAP_TOT; i++){
    printf("Try to open capture device %s\n", cap_device[i]);
    if(capture_open(cap_device[i], &capture[i])){
      printf("Unable to open %s\n", cap_device[i]);
      return -1;
    }
    if(capture_setup(&capture[i])){
      printf("Setup v4l capture failed.\n");
      return -1;
    }
  }

  out_device = "/dev/video17";
  if(output_open(out_device, pOut)){
    printf("Unable to open %s\n", cap_device[i]);
    return -1;
  }

  pOut->fmt = V4L2_PIX_FMT_UYVY;
  pOut->width  = capture[0].width  * 2;
  pOut->height = capture[0].height * 2;
  pOut->disp_top    =    0;
  pOut->disp_left   =    0;
  pOut->disp_width  = 1280;
  pOut->disp_height =  800;

  if(output_setup(pOut)<0){
    printf("Setup v4l output failed.\n");
    close(capture[0].fd);
    return -1;
  }
  if(output_prepare(pOut)<0){
    printf("output_prepare failed\n");
    return -1;
  }

  overlay_set();

  if(output_start(pOut) < 0){
    printf("Could not start output stream\n");
    return -1;
  }

  printf("pthread_create master\n");
  i = 0;
  task[i].pCap   = &capture[i];
  task[i].capIdx = 0;
  task[i].pOut   = pOut;
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
  output_stop(pOut);
  output_close(pOut);

  for(i=0; i<TVIN_CAP_TOT; i++){
    capture_close(&capture[i]);
  }
  return 0;
}
