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
  int         bufIdx;
  uint32_t    flag;
  int         input;
  int         chgin;
}sTaskArg, *psTaskArg;

/**
 */
typedef struct {
  char        name[128];
  uint16_t    w;
  uint16_t    h;
  uint16_t    bpl;
  uint32_t    size;
  uint8_t    *image;
}sImage, *psImage;

/**
 */
sUsbDevice  usbDevice;
pthread_t   devScanTid;
pthread_t   eventTid;
sTaskArg    capArg[2];

void* devScanThread(void* arg);
void* eventThread(void* arg);
void* capThread(void* arg);
void bufcpy(uint8_t* dst, uint8_t* src, uint16_t w, uint16_t h, int idx);

psImage imageLoad(char* name);
void    imageFree(psImage pImg);

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
  psImage             img0;   // No Device Connected
  psImage             img1;   // Input unselected
  psImage             img2;   // Wait device opening

  printf("%s "__DATE__" "__TIME__"\n", argv[0]);

  // start capture threads
  for(i=0; i<2; i++){
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
  //pOut->width  =  90+720*2; //  90 + 1440 = 1530
  //pOut->height = 180+574*2; // 180 + 1148 = 1328
  pOut->width  =  90+510*2; //  90 + 1440 = 1530
  pOut->height = 180+406*2; // 180 + 1148 = 1328
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

  // Load image from file
  img0 = imageLoad("nodev.uyvy");
  img1 = imageLoad("noin.uyvy");
  img2 = imageLoad("wait.uyvy");

  for(;;){
    // get output buffer
    outb = outBufGet(pOut);
    if(outb){
      for(i=0; i<2; i++){
        if(capArg[i].pCap){
          int   temp;
          // Get capture buffers from ring queue and enqueue last used buffer
          ret = 1;
          while(ret){
            temp = capArg[i].bufIdx;
            ret = rngGet(capArg[i].pRng, &(capArg[i].bufIdx));
            if(ret==0){
              if(capArg[i].flag){
                struct v4l2_buffer  cap_buf;
                cap_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                cap_buf.index  = temp;
                cap_buf.memory = V4L2_MEMORY_MMAP;
                ret = ioctl(capArg[i].pCap->fd, VIDIOC_QBUF, &cap_buf);
              }
              capArg[i].flag |= 0x00000001;
            }
          }
          // copy capture buffer to output
          bufcpy(pOut->buf[outb->index].start,
                capArg[i].pCap->buf[capArg[i].bufIdx].start,
                capArg[i].pCap->bpl, capArg[i].pCap->height,
                i*2+capArg[i].input);
          // copy 'unselected input' image buffer to output
          bufcpy(pOut->buf[outb->index].start,
                img1->image, img1->bpl, img1->h,
                i*2+(capArg[i].input?0:1));
        }else{
          if(i<usbDevice.num){
            // copy 'Wait' image buffer to output
            bufcpy(pOut->buf[outb->index].start,
                   img2->image, img2->bpl, img2->h, i*2+0);
            bufcpy(pOut->buf[outb->index].start,
                   img2->image, img2->bpl, img2->h, i*2+1);
          }else{
            // copy 'NoDevice' image buffer to output
            bufcpy(pOut->buf[outb->index].start,
                   img0->image, img0->bpl, img0->h, i*2+0);
            bufcpy(pOut->buf[outb->index].start,
                   img0->image, img0->bpl, img0->h, i*2+1);
          }
        }
      }
      // put output buffer
      outBufPut(pOut, outb);
    }
  }
  pthread_mutex_destroy(&usbDevice.lock);
  return 0;
}

/**
 * @brief Thread per l'accquisizione dello stream video in ingresso.
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
        rngClr(pTask->pRng);
      }
      pthread_mutex_unlock(&usbDevice.lock);
      continue;
    }
    // se l'ingresso è cambiato chiude pCap
    if(pTask->chgin){
      if(pTask->pCap){
        DBG_PRINT("#%d:input changed close device device %s.\n", index, usbDevice.device[index]);
        capStop(pTask->pCap);
        capClose(pTask->pCap);
        pTask->pCap = NULL;
        rngClr(pTask->pRng);
      }else{
        printf("pTask->pCap is null\n");
      }
      pTask->chgin = 0;
    }
    // se pCap è chiuso lo apre
    if(pTask->pCap==NULL){
      DBG_PRINT("#%d:open device %s.\n", index, usbDevice.device[index]);
      pTask->pCap = capOpen(usbDevice.device[index]);
      if(!pTask->pCap){
        pthread_mutex_unlock(&usbDevice.lock);
        continue;
      }
      // impostazione device
      pTask->pCap->width  = 510;
      pTask->pCap->height = 406;
      pTask->pCap->fmt    = V4L2_PIX_FMT_YUYV;
      pTask->pCap->input  = pTask->input;
      DBG_PRINT("#%d:capSetup device %s input %d.\n", index, pTask->pCap->name, pTask->pCap->input);
      if(capSetup(pTask->pCap)){
        DBG_ERROR("#%d:capSetup device %s input %d failed.\n", index, pTask->pCap->name, pTask->pCap->input);
        capClose(pTask->pCap);
        pTask->pCap = NULL;
        pthread_mutex_unlock(&usbDevice.lock);
        continue;
      }
      DBG_PRINT("#%d:capStart device %s input %d.\n", index, pTask->pCap->name, pTask->pCap->input);
      if(capStart(pTask->pCap)){
        DBG_ERROR("#%d:Could not start capture stream device %s input %d.\n", index, pTask->pCap->name, pTask->pCap->input);
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
    ret = rngPut(pTask->pRng, capture_buf.index);
    if(ret){
      DBG_ERROR("#%d: put buffer overflow\n", index);
      ret = ioctl(pTask->pCap->fd, VIDIOC_QBUF, &capture_buf);
      if(ret){
        DBG_ERROR("#%d:VIDIOC_QBUF\n", index);
      }
    }

  }
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
      printf("====================================\n"
             "trovate %d USB\n", usbDevice.num);
      for(i=0; i<usbDevice.num; i++){
        printf("  %d %s\n", i, usbDevice.device[i]);
      }
      pthread_mutex_unlock(&usbDevice.lock);
      old = ret;
    }
  }
  return NULL;
}

/**
 * @brief Thread per l'acquisizione delll'evento da tastiera per la modifica
 *        dell'ingresso.
 */
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
        int i;
        printf("====================================\n"
               "Change input\n");
        for(i=0; i<2; i++){
          capArg[i].input = capArg[i].input? 0: 1;
          capArg[i].chgin = 1;
          capArg[i].flag  = 0;
        }
      }
    }
  }
  return NULL;
}

/**
 */
void bufcpy(uint8_t* dst, uint8_t* src, uint16_t w, uint16_t h, int idx)
{
  int i;
  switch(idx){
  case  0: dst += (120*1110+30)*2; break;
  case  1: dst += (120*1110+30+510+30)*2; ; break;
  case  2: dst += (150*1110+406*1110+30)*2; break;
  case  3: dst += (150*1110+406*1110+30+510+30)*2; break;
  default: return;
  }
  for(i=0; i<h; i++){
    memcpy(dst, src, w);
    src += w;
    dst += 1110*2;
  }
}

/**
 */
psImage imageLoad(char* name)
{
  psImage pImg;

  pImg = (psImage)malloc(sizeof(sImage));
  if(!pImg){
    return NULL;
  }
  memset(pImg, 0, sizeof(sImage));

  pImg->size = fsFileLoad(name, &pImg->image);
  if(!pImg->size){
    free(pImg);
    return NULL;
  }
  strcpy(pImg->name, name);
  pImg->w = 510;
  pImg->h = 406;
  pImg->bpl = pImg->w * 2;

  int i;
  uint16_t t;
  uint16_t *p;
  p = (uint16_t*)pImg->image;
  for(i=0; i<pImg->size/2; i++){
    t = *p;
    *p = (t>>8)|(t<<8);
    p++;
  }

  return pImg;
}

/**
 */
void imageFree(psImage pImg)
{
  if(pImg){
    if(pImg->image){
      free(pImg->image);
    }
    free(pImg);
  }
}
