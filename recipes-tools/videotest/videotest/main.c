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
#include <dirent.h>
#include <fcntl.h>
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <sys/time.h>
#include <linux/videodev2.h>

#include "fs.h"
#include "rng.h"
#include "event.h"
#include "out.h"
#include "cap.h"
#include "v4l2.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "VT"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif

#define CAP_NUM 2

//#define CAP_WIDTH  720
//#define CAP_HEIGHT 574
#define CAP_WIDTH  510  //(720/1.414)
#define CAP_HEIGHT 406  //(574/1.414)
//#define CAP_WIDTH  (720/4)
//#define CAP_HEIGHT (574/4)

#define CAP_STD   V4L2_STD_PAL
//#define CAP_STD   V4L2_STD_NTSC
#define CAP_DEF_INPUT   1

#define CAP_FLG_OPENED      0x00000001
#define CAP_FLG_SETUPED     0x00000002
#define CAP_FLG_STARTED     0x00000004
#define CAP_FLG_FIRSTFRAME  0x00000008
#define CAP_FLG_STOPPED     0x00000010
#define CAP_FLG_OPEN        0x40000000
#define CAP_FLG_STOP        0x80000000

/**
 */
typedef struct {
  int               capIdx;
  pthread_t         tid;
  psCapDev          pCap;
  psRng             pRng;
  int               bufIdx;
  uint32_t          flag;
  int               input;
  int               chgin;
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
pthread_t   eventTid;
sTaskArg    capArg[CAP_NUM];

/**
 */
void*   eventThread(void* arg);
void*   capThread(void* arg);
int     capDevOpen(psTaskArg pTask);
int     capDevClose(psTaskArg pTask);

void    bufcpy(uint8_t* dst, uint8_t* src, uint16_t w, uint16_t h, int idx);
psImage imageLoad(char* name);
void    imageFree(psImage pImg);

/**
 */
char *capDevName[] = {
  "/dev/video4",
  "/dev/video5",
  "/dev/video6",
  "/dev/video7"
};

/**
 */
int main(int argc, char **argv)
{
  int                 ret;
  int                 d;
  int                 i;
  char               *outdev = "/dev/video17";
  psOutDev            pOut;
  struct v4l2_buffer *outb;
  psImage             img0;   // No Device Connected
  psImage             img1;   // Input unselected
  psImage             img2;   // Wait device opening
int cs[CAP_NUM];
int os[CAP_NUM];
memset(cs, 0, sizeof(cs));
memset(os, 0, sizeof(os));

  printf("%s "__DATE__" "__TIME__"\n", argv[0]);

  // start capture threads
  for(i=0; i<CAP_NUM; i++){
    memset(&capArg[i], 0, sizeof(sTaskArg));
    capArg[i].capIdx = i;
    capArg[i].input  = CAP_DEF_INPUT;
    capArg[i].flag   = CAP_FLG_STOPPED;
    capArg[i].pRng   = rngCreate(4);
    if(!capArg[i].pRng){
      DBG_ERROR("cap #%d creating rng queue fail\n", i);
      return -1;
    }
    DBG_PRINT("capArg[%d]=%p\n", i, &capArg[i]);
    ret = pthread_create(&capArg[i].tid, NULL, capThread, &capArg[i]);
    if(ret){
      DBG_ERROR("cap #%d start capture thread fail\n", i);
      return -1;
    }
  }

  // Start keyboard event thread
  ret = pthread_create(&eventTid, NULL, eventThread, NULL);
  if(ret){
    DBG_ERROR("ERROR start USB Video devices scan thread\n");
    return -1;
  }

  // Open output device
  pOut = outOpen(outdev);
  if(!pOut){
    DBG_ERROR("error opening %s output device\n", outdev);
    return -1;
  }
  pOut->fmt    = V4L2_PIX_FMT_YUYV;
  pOut->width  =  30+(CAP_WIDTH +30)*CAP_NUM;
  pOut->height = 120+(CAP_HEIGHT+30)*2;
  pOut->disp_top    =    0;
  pOut->disp_left   =    0;
  pOut->disp_width  = 1280;
  pOut->disp_height =  800;
  if(outSetup(pOut)){
    DBG_ERROR("Setup v4l output failed.\n");
    return -1;
  }
  if(outPrepare(pOut)){
    DBG_ERROR("outPrepare failed\n");
    return -1;
  }
  fbOverlaySet();
  if(outStart(pOut)){
    DBG_ERROR("Could not start output stream\n");
    return -1;
  }
  DBG_PRINT("output device %s OK\n", outdev);

  // Load image from file
  img0 = imageLoad("nodev.uyvy");
  img1 = imageLoad("noin.uyvy");
  img2 = imageLoad("wait.uyvy");
  if(!img0 || !img1 || !img2){
    DBG_ERROR("error loading images %p %p %p\n", img0, img1, img2);
    return -1;
  }
  DBG_PRINT("images loaded\n");

  for(;;){
    // get output buffer
    outb = outBufGet(pOut);
    if(outb){
      for(i=0; i<CAP_NUM; i++){
        if(capArg[i].flag & CAP_FLG_STARTED){
          int   temp;
cs[i] = 1;
if(cs[i]!=os[i]){
  DBG_PRINT("Capture #%d started\n", i);
  os[i] = cs[i];
}
          // Get capture buffers from ring queue and enqueue last used buffer
          ret = 1;
          while(ret){
            if((capArg[i].flag&CAP_FLG_STARTED)==0){
              break;
            }
            temp = capArg[i].bufIdx;
            ret = rngGet(capArg[i].pRng, &(capArg[i].bufIdx));
            if(ret==0){
              if(capArg[i].flag & CAP_FLG_FIRSTFRAME){
                struct v4l2_buffer  cap_buf;
                cap_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
                cap_buf.index  = temp;
                cap_buf.memory = V4L2_MEMORY_MMAP;
                if(capArg[i].pCap){
                  ret = ioctl(capArg[i].pCap->fd, VIDIOC_QBUF, &cap_buf);
                }
              }
              capArg[i].flag |= CAP_FLG_FIRSTFRAME;
            }
          }
          if(capArg[i].flag==0){
            continue;
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
          ret = fsFileStat(capDevName[i], FS_TYPE_CHR);
          if(!ret){
cs[i] = 2;
if(cs[i]!=os[i]){
  DBG_PRINT("Capture #%d present\n", i);
  os[i] = cs[i];
}
            // copy 'Wait' image buffer to output
            bufcpy(pOut->buf[outb->index].start,
                   img2->image, img2->bpl, img2->h, i*2+0);
            bufcpy(pOut->buf[outb->index].start,
                   img2->image, img2->bpl, img2->h, i*2+1);
          }else{
cs[i] = 3;
if(cs[i]!=os[i]){
  DBG_PRINT("Capture #%d not present\n", i);
  os[i] = cs[i];
}
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

  DBG_PRINT("#%d:started arg is %p Device %s\n", index, arg, capDevName[index]);

  for(;;){
    if(pTask->flag & CAP_FLG_STOP){
      DBG_PRINT("#%d:acquisition stopped close device.\n",
                index, pTask->input, capDevName[index]);
      capDevClose(pTask);
      pTask->flag = CAP_FLG_STOPPED;
    }
    if(pTask->flag & CAP_FLG_STOPPED){
      continue;
    }
    // verifica che il device sia presente
    ret = fsFileStat(capDevName[index], FS_TYPE_CHR);
    if(ret){
      DBG_PRINT("#%d:device %s not present close it.\n", index, capDevName[index]);
      // se pCap è aperto lo chiude
      if(pTask->pCap){
        DBG_PRINT("#%d:device %s not present close it.\n",
                  index, capDevName[index]);
        capDevClose(pTask);
      }
      usleep(20000);
      continue;
    }
    // se l'ingresso è cambiato chiude pCap e lo riapre
    if(pTask->chgin){
      DBG_PRINT("#%d:input changed to %d close device %s.\n",
                index, pTask->input, capDevName[index]);
      if(pTask->pCap){
        capDevClose(pTask);
      }
      if(pTask->pCap==NULL){
        ret = capDevOpen(pTask);
        if(ret){
          usleep(20000);
          continue;
        }
      }
      pTask->chgin = 0;
    }
    //
    if(pTask->flag & CAP_FLG_OPEN){
      // apre pCap
      DBG_PRINT("#%d:open %s requested.\n", index, capDevName[index]);
      if(pTask->pCap==NULL){
        ret = capDevOpen(pTask);
        if(ret){
          usleep(20000);
          continue;
        }
      }else{
        DBG_ERROR("#%d:pTask->pCap not null in open request\n", index);
      }
    }

    if( (pTask->flag & CAP_FLG_STARTED)==0 || pTask->pCap==NULL ){
      usleep(20000);
      continue;
    }

    // Dequeue capture buffer
    memset(&capture_buf, 0, sizeof(capture_buf));
    capture_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    capture_buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(pTask->pCap->fd, VIDIOC_DQBUF, &capture_buf);
    if(ret<0){
      if(errno==11){
        continue;
      }
      DBG_ERROR("#%d:fd %d VIDIOC_DQBUF capture %s failed (%d) %d %s.\n",
                index, pTask->pCap->fd, pTask->pCap->name, ret,
                errno, strerror(errno));
      continue;
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
 * @brief Thread per l'acquisizione dell'evento da tastiera per la modifica
 *        dell'ingresso.
 */
void* eventThread(void* arg)
{
  char     *name = "/dev/input/event2";
  psEvHnd   pHnd;
  psEvent   pEve;

  DBG_PRINT("started arg is %p\n", arg);

  pHnd = evOpen(name);
  if(!pHnd){
    return NULL;
  }
  for(;;){
    pEve = evGet(pHnd);
    if(pEve){
/*
      DBG_PRINT("eventThread: time %ld.%06ld, type %d, code %d, value %d\n",
                pEve->time.tv_sec, pEve->time.tv_usec,
                pEve->type, pEve->code, pEve->value);
*/
      if(pEve->code==3 && pEve->value==1){
        int i;
        DBG_PRINT("\n====================================\n"
                  "Stop acquisition\n");
        for(i=0; i<CAP_NUM; i++){
          if(!capArg[i].chgin){
            capArg[i].flag = CAP_FLG_STOP;
          }
        }
      }
      if(pEve->code==4 && pEve->value==1){
        int i;
        if(capArg[0].flag==0 || capArg[0].flag & CAP_FLG_STOPPED ){
          DBG_PRINT("\n====================================\n"
                    "Open devices\n");
          for(i=0; i<CAP_NUM; i++){
            capArg[i].flag = CAP_FLG_OPEN;
          }
        }else if(capArg[0].flag & CAP_FLG_STARTED){
          int in;
          in = capArg[0].input? 0: 1;
          DBG_PRINT("\n====================================\n"
                    "Change input to %d\n", in);
          for(i=0; i<CAP_NUM; i++){
            if(!capArg[i].chgin){
              capArg[i].input = in;
              capArg[i].chgin = 1;
              capArg[i].flag  = 0;
            }
          }
        }
      }
    }
  }
  return NULL;
}

/**
 */
int capDevOpen(psTaskArg pTask)
{
  if(pTask->pCap){
    DBG_ERROR("#%d:capOpen device %s already opened.\n",
              pTask->capIdx, pTask->pCap->name);
    return 0;
  }

  DBG_PRINT("#%d:capOpen device %s.\n",
          pTask->capIdx, capDevName[pTask->capIdx]);
  pTask->pCap = capOpen(capDevName[pTask->capIdx]);
  if(!pTask->pCap){
    DBG_ERROR("#%d:capOpen device %s failed.\n",
              pTask->capIdx, pTask->pCap->name);
    return -1;
  }
  pTask->flag = CAP_FLG_OPENED;
  DBG_PRINT("#%d:capOpen device %s fd %d OK.\n",
            pTask->capIdx, pTask->pCap->name, pTask->pCap->fd);
  // impostazione device
  pTask->pCap->width  = CAP_WIDTH;
  pTask->pCap->height = CAP_HEIGHT;
  pTask->pCap->fmt    = V4L2_PIX_FMT_YUYV;
  pTask->pCap->std    = CAP_STD;
  pTask->pCap->input  = pTask->input;
  DBG_PRINT("#%d:capSetup device %s input %d.\n",
            pTask->capIdx, pTask->pCap->name, pTask->pCap->input);
  if(capSetup(pTask->pCap)){
    DBG_ERROR("#%d:capSetup device %s input %d failed.\n",
              pTask->capIdx, pTask->pCap->name, pTask->pCap->input);
    capClose(pTask->pCap);
    pTask->pCap = NULL;
    pTask->flag = 0;
    return -1;
  }
  pTask->flag |= CAP_FLG_SETUPED;

  DBG_PRINT("#%d:capStart device %s.\n",
            pTask->capIdx, pTask->pCap->name);
  if(capStart(pTask->pCap)){
    DBG_ERROR("#%d:Could not start capture stream device %s.\n",
              pTask->capIdx, pTask->pCap->name);
    capClose(pTask->pCap);
    pTask->pCap = NULL;
    pTask->flag = 0;
    return -1;
  }
  pTask->flag |= CAP_FLG_STARTED;
  return 0;
}

/**
 */
int capDevClose(psTaskArg pTask)
{
  if(!pTask->pCap){
    DBG_ERROR("#%d:capOpen already closed.\n", pTask->capIdx);
    return 0;
  }
  DBG_PRINT("#%d:close device.\n", pTask->capIdx);
  pTask->flag = 0;
  rngClr(pTask->pRng);
  capStop(pTask->pCap);
  capClose(pTask->pCap);
  pTask->pCap = NULL;
  return 0;
}

/**
 */
#define LINE_LEN (30+(30+CAP_WIDTH)*CAP_NUM)

void bufcpy(uint8_t* dst, uint8_t* src, uint16_t w, uint16_t h, int idx)
{
  int i;
#if CAP_NUM == 2
  switch(idx){
  case  0: dst += (120*LINE_LEN+30                 )*2; break;
  case  1: dst += (120*LINE_LEN+30+(CAP_WIDTH+30)*1)*2; break;
  case  2: dst += ((120+CAP_HEIGHT+30)*LINE_LEN+30                 )*2; break;
  case  3: dst += ((120+CAP_HEIGHT+30)*LINE_LEN+30+(CAP_WIDTH+30)*1)*2; break;
  default: return;
  }
#elif CAP_NUM == 4
  switch(idx){
  case  0: dst += (120*LINE_LEN+30                 )*2; break;
  case  1: dst += (120*LINE_LEN+30+(CAP_WIDTH+30)*1)*2; break;
  case  2: dst += (120*LINE_LEN+30+(CAP_WIDTH+30)*2)*2; break;
  case  3: dst += (120*LINE_LEN+30+(CAP_WIDTH+30)*3)*2; break;
  case  4: dst += ((120+CAP_HEIGHT+30)*LINE_LEN+30                 )*2; break;
  case  5: dst += ((120+CAP_HEIGHT+30)*LINE_LEN+30+(CAP_WIDTH+30)*1)*2; break;
  case  6: dst += ((120+CAP_HEIGHT+30)*LINE_LEN+30+(CAP_WIDTH+30)*2)*2; break;
  case  7: dst += ((120+CAP_HEIGHT+30)*LINE_LEN+30+(CAP_WIDTH+30)*3)*2; break;
  default: return;
  }
#else
  return;
#endif
  for(i=0; i<h; i++){
    memcpy(dst, src, w);
    src += w;
    dst += LINE_LEN*2;
  }
}

/**
 */
psImage imageLoad(char* name)
{
  psImage   pImg;
  int       i;
  uint16_t  t;
  uint16_t *p;

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
  pImg->w = CAP_WIDTH;
  pImg->h = CAP_HEIGHT;
  pImg->bpl = pImg->w * 2;

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
