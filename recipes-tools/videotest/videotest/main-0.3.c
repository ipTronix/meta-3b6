/** ***************************************************************************
 * @file main.c
 * @brief TeraHE Video test
 **************************************************************************** */

/**
 * INCLUDE FILES
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

// TODO spostare in fs.c fs.h
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/videodev2.h>

#include "fs.h"
#include "rng.h"
#include "event.h"
#include "out.h"
#include "cap.h"
#include "v4l2.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME ""
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif


/**
 */
typedef struct {
  pthread_mutex_t   lock;
  int               num;
  char              device[32][32];
}sUsbDevice, *psUsbDevice;

/**
 */
typedef struct {
  int         capIdx;
  pthread_t   tid;
  psCapDev    pCap;
  psRng       pRng;
//  psOutDev    pOut;
}sTaskArg, *psTaskArg;


/**
 */
typedef struct {
  unsigned char  *start;
  int             width;
  int             height;
  int             bpl;    //bytes per line
  int             imsize; //image size
}sUsbFrame, *psUsbFrame;

sUsbDevice  usbDevice;
sUsbFrame   usbFrame[4];

/**
 */
pthread_t   devScanTid;
pthread_t   eventTid;


void* devScanThread(void* arg);
void* eventThread(void* arg);
void* capThread(void* arg);
int getUsbFrame(char* device, int input, psUsbFrame pBuf);
void bufcpy(uint8_t* dst, uint8_t* src, uint16_t w, uint16_t h, int idx);


/**
 */
int main(int argc, char **argv)
{
  int                 ret;
  int                 d;
  int                 i;
  char               *outdev = "/dev/video17"; //TODO da file config o da command line
  psOutDev            pOut;
  struct v4l2_buffer *outb;
  uint8_t            *page1_buf;
  int                 page1_len;

  sTaskArg            capArg[2];
  int                 capBufIdx[2];
  uint32_t            flag[2];

  printf("%s "__DATE__" "__TIME__"\n", argv[0]);

  //
  for(i=0; i<4; i++){
    usbFrame[i].width  = 720;
    usbFrame[i].height = 574;
    usbFrame[i].bpl    = usbFrame[i].width * 2;
    usbFrame[i].imsize = usbFrame[i].bpl * usbFrame[i].height;
    usbFrame[i].start = (unsigned char*)malloc(usbFrame[i].imsize);
    if(!usbFrame[i].start){
      DBG_ERROR("memory\n");
      return -1;
    }
  }

  // start capture threads
  for(i=0; i<2; i++){
    capBufIdx[i] = 0;
    flag[i] = 0;

    memset(&capArg[i], 0, sizeof(sTaskArg));
    capArg[i].capIdx = i;
    capArg[i].pRng = rngCreate(8);
    if(!capArg[i].pRng){
      DBG_ERROR("creating rng queue fail\n");
      return -1;
    }
    DBG_PRINT("&capArg[i]=%p\n", &capArg[i]);
    ret = pthread_create(&capArg[i].tid, NULL, capThread, &capArg[i]);
    if(ret){
      DBG_ERROR("start capture thread fail\n");
      return -1;
    }
  }

  // start USB Video devices scan thread
  ret = pthread_mutex_init(&usbDevice.lock, NULL);
  if(ret){
    printf("mutex init failed\n");
    return -1;
  }
  ret = pthread_create(&devScanTid, NULL, devScanThread, NULL);
  if(ret){
    printf("ERROR start USB Video devices scan thread\n");
    return -1;
  }
  ret = pthread_create(&eventTid, NULL, eventThread, NULL);
  if(ret){
    printf("ERROR start USB Video devices scan thread\n");
    return -1;
  }

  // Open output device
  pOut = outOpen(outdev);
  if(!pOut){
    printf("error opening %s output device\n", outdev);
    return -1;
  }
  pOut->fmt = V4L2_PIX_FMT_YUYV;
  pOut->width  =  90+720*2; //  90 + 1440 = 1530
  pOut->height = 180+574*2; // 180 + 1148 = 1328
  pOut->disp_top    =    0;
  pOut->disp_left   =    0;
  pOut->disp_width  = 1280;
  pOut->disp_height =  800;
  if(outSetup(pOut)){
    printf("Setup v4l output failed.\n");
    return -1;
  }
  if(outPrepare(pOut)){
    printf("output_prepare failed\n");
    return -1;
  }
  fbOverlaySet();
  if(outStart(pOut)){
    printf("Could not start output stream\n");
    return -1;
  }

  // Load default image
  page1_len = fsFileLoad("capPage.yuyv", &page1_buf);

  for(;;){
    // get output buffer
    outb = outBufGet(pOut);
    if(outb){
      int                 temp;
i = 0;
      if(capArg[i].pCap){
      // Get capture buffers from ring queue and enqueue last used buffer
      ret = 1;
      while(ret){
        temp = capBufIdx[i];
        ret = rngGet(capArg[i].pRng, &(capBufIdx[i]));
        if(ret==0){
printf("acquisito buffer %d %p\n", capBufIdx[i], capArg[i].pCap->buf[capBufIdx[i]].start);
          if(flag[i]){
            struct v4l2_buffer  cap_buf;
            cap_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
            cap_buf.index  = temp;
            cap_buf.memory = V4L2_MEMORY_MMAP;
printf("riaccodamento buffer %d\n", temp);
            ret = ioctl(capArg[i].pCap->fd, VIDIOC_QBUF, &cap_buf);
          }
          flag[i] |= 0x00000001;  //TODO va azzerato ad ogni chiusura riapertura e cambio ingresso
        }
      }
      // copy capture buffer to output buffer
printf("copia del buffer %d %p\n", capBufIdx[i], capArg[i].pCap->buf[capBufIdx[i]].start);
      bufcpy(pOut->buf[outb->index].start,
             capArg[i].pCap->buf[capBufIdx[i]].start,
             capArg[i].pCap->bpl, capArg[i].pCap->height, i);
      }else{
        //printf("capArg[i].pCap is NULL\n");
      }
/*
      int idx;
      pthread_mutex_lock(&usbDevice.lock);
      for(d=0; d<usbDevice.num; d++){
        for(i=0; i<2; i++){
          idx = i+d*2;
          ret = getUsbFrame(usbDevice.device[d], i, &usbFrame[idx]);
          if(!ret){
            bufcpy(pOut->buf[outb->index].start,
                  usbFrame[idx].start,
                  usbFrame[idx].bpl, usbFrame[idx].height, idx);
          }
        }
      }
      pthread_mutex_unlock(&usbDevice.lock);

      bufcpy(pOut->buf[outb->index].start, page1_buf, 720*2, 574, 2);
      bufcpy(pOut->buf[outb->index].start, page1_buf, 720*2, 574, 3);
*/
      // put output buffer
      outBufPut(pOut, outb);
    }
  }
  pthread_mutex_destroy(&usbDevice.lock);
  return 0;
}

/**
 */
void* capThread(void* arg)
{
  psTaskArg           pTask = (psTaskArg)arg;
  int                 index = pTask->capIdx;
  struct v4l2_buffer  capture_buf;
  enum v4l2_buf_type  type;
  int                 ret;

  DBG_PRINT("#%d:started arg is %p\n", index, arg);

  for(;;){
    pthread_mutex_lock(&usbDevice.lock);
    if(index>=usbDevice.num){
      // se pCap è aperto lo chiude
      if(pTask->pCap){
        DBG_PRINT("#%d:close device.\n", index);
        capStop(pTask->pCap);
        capClose(pTask->pCap);
        pTask->pCap = NULL;
      }
      pthread_mutex_unlock(&usbDevice.lock);
      continue;
    }
    // se pCap è chiuso lo apre
    if(pTask->pCap==NULL){
      DBG_PRINT("#%d:apertura device %s.\n", index, usbDevice.device[index]);
      pTask->pCap = capOpen(usbDevice.device[index]);
      if(!pTask->pCap){
        pthread_mutex_unlock(&usbDevice.lock);
        continue;
      }
      // impostazione device
      pTask->pCap->fmt   = V4L2_PIX_FMT_YUYV;
      pTask->pCap->input = 0; // TODO input;
      DBG_PRINT("capSetup device %s input %d.\n", pTask->pCap->name, pTask->pCap->input);
      if(capSetup(pTask->pCap)){
        DBG_ERROR("capSetup device %s input %d failed.\n", pTask->pCap->name, pTask->pCap->input);
        capClose(pTask->pCap);
        pTask->pCap = NULL;
        pthread_mutex_unlock(&usbDevice.lock);
        continue;
      }
      DBG_PRINT("capStart device %s input %d.\n", pTask->pCap->name, pTask->pCap->input);
      if(capStart(pTask->pCap)){
        DBG_ERROR("Could not start capture stream device %s input %d.\n", pTask->pCap->name, pTask->pCap->input);
        capClose(pTask->pCap);
        pTask->pCap = NULL;
        pthread_mutex_unlock(&usbDevice.lock);
        continue;
      }
    }
    pthread_mutex_unlock(&usbDevice.lock);

    // Dequeue capture buffer
    memset(&capture_buf, 0, sizeof(capture_buf));
    capture_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    capture_buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(pTask->pCap->fd, VIDIOC_DQBUF, &capture_buf);
    if(ret<0){
      DBG_ERROR("#%d:VIDIOC_DQBUF capture %s failed (%d).\n",
                index, pTask->pCap->name, ret);
    }
    // put buffer to ring queue
    DBG_PRINT("#%d:put buffer to ring queue.\n", index);
    ret = rngPut(pTask->pRng, capture_buf.index);
    if(ret){
      DBG_ERROR("#%d: put buffer overflow\n", index);
      ret = ioctl(pTask->pCap->fd, VIDIOC_QBUF, &capture_buf);
      if(ret){
        DBG_ERROR("#%d:VIDIOC_QBUF\n", index);
      }
    }

  }

/*
  // Start capture
  if(capStart(pCap)<0){
    printf("thread slv #%d:ERROR:capture_start %s failed\n",
           index, pCap->name);
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
              index, pCap->name, ret);
    }

    // put buffer to ring queue
    //ret = rngPut(&rng[index], capture_buf.index);
    ret = rngPut(pTask->pRng, capture_buf.index);
    if(ret){
      printf( "thread slv #%d:ERROR: ring[%d] put buffer overflow\n",
              index, index);
      ret = ioctl(pCap->fd, VIDIOC_QBUF, &capture_buf);
      if(ret){
        printf( "thread slv #%d:ERROR: VIDIOC_QBUF\n", index);
      }
    }
  }
*/
  return NULL;
}


/**
 */
void* devScanThread(void* arg)
{
  int         ret;
  int         old;
  char        buf[32*32];
  char       *dev[32];
  int         i;
  sV4l2Info   inf;

  for(i=0; i<32; i++){
    dev[i] = buf + i*32;
  }
  old = 0;
  for(;;){
    ret = fsDirGet("/dev/", "video", dev, 32);
    if(ret!=old){
      pthread_mutex_lock(&usbDevice.lock);
      usbDevice.num = 0;
      for(i=0; i<ret; i++){
        v4l2InfoGet(dev[i], &inf);
        if(inf.num_input==2 && memcmp(inf.bus_info,"usb", 3)==0){
          //v4l2InfoShow(&inf);
          strcpy(usbDevice.device[usbDevice.num], inf.device);
          usbDevice.num++;
        }
      }
      printf("====================================\n");
      printf("trovate %d USB\n", usbDevice.num);
      for(i=0; i<usbDevice.num; i++){
        printf("  %d %s\n", i, usbDevice.device[i]);
      }
      pthread_mutex_unlock(&usbDevice.lock);
      old = ret;
    }
  }
  return NULL;
}

void* eventThread(void* arg)
{
  char     *name = "/dev/input/event2";
  psEvHnd   pHnd;
  psEvent   pEve;

  pHnd = evOpen(name);
  if(!pHnd){
    return NULL;
  }
  for(;;){
    pEve = evGet(pHnd);
    if(pEve){
      printf("eventThread: time %ld.%06ld, type %d, code %d, value %d\n",
             pEve->time.tv_sec, pEve->time.tv_usec,
             pEve->type, pEve->code, pEve->value);
      if(pEve->code==4 && pEve->value==1){
        //TODO
        printf("==========Cambio sorgente================\n");
      }
    }
  }
  return NULL;
}


/**
 * TODO mettere schermata NO_SIGNAL su errore
 */
int getUsbFrame(char* device, int input, psUsbFrame pBuf)
{
  psCapDev            pCap;
  struct v4l2_buffer *capb;

  DBG_PRINT("capOpen device %s input %d.\n", device, input);
  pCap = capOpen(device);
  if(!pCap){
    DBG_ERROR("error opening capture device %s\n", device);
    return -1;
  }

  DBG_PRINT("capSetup device %s input %d.\n", device, input);
  pCap->fmt   = V4L2_PIX_FMT_YUYV;
  pCap->input = input;
  if(capSetup(pCap)){
    DBG_ERROR("capSetup device %s input %d failed.\n", device, input);
    capClose(pCap);
    return -1;;
  }

  if(capStart(pCap)){
    DBG_ERROR("Could not start capture stream device %s input %d.\n", device, input);
    capClose(pCap);
    return -1;
  }
  DBG_PRINT("capBufGet wait for input %d\n", pCap->input);
  capb = capBufGet(pCap);
  if(capb){
    DBG_PRINT("capBufGet input %d\n", pCap->input);
    memcpy(pBuf->start, pCap->buf[capb->index].start, pBuf->imsize);
    capBufPut(pCap, capb);
  }
  if(capStop(pCap)){
    DBG_ERROR("Could not stop capture stream\n");
    capClose(pCap);
    return -1;
  }
  capClose(pCap);

  return 0;
}

/**
 */
void bufcpy(uint8_t* dst, uint8_t* src, uint16_t w, uint16_t h, int idx)
{
  int i;
  switch(idx){
  case  0: dst += (120*1530+30)*2; break;
  case  1: dst += (120*1530+30+720+30)*2; ; break;
  case  2: dst += (150*1530+574*1530+30)*2; break;
  case  3: dst += (150*1530+574*1530+30+720+30)*2; break;
  default: return;
  }
  for(i=0; i<h; i++){
    memcpy(dst, src, w);
    src += w;
    dst += 1530*2;
  }
}
