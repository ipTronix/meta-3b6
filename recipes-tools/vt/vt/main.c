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
#include "fb.h"
#include "v4l2.h"
#include "blt.h"
#include "grx.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "VT"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif


#define CAP_NUM 4
#define CAP_DEF_INPUT   1

#define CAP_WIDTH       720
#define CAP_HEIGHT      576
#define CAP_STD         V4L2_STD_PAL
//#define CAP_STD         V4L2_STD_NTSC

#define CAP_FLG_OPENED      0x00000001
#define CAP_FLG_SETUPED     0x00000002
#define CAP_FLG_STARTED     0x00000004
#define CAP_FLG_FIRSTFRAME  0x00000008
#define CAP_FLG_STOPPED     0x00000010
#define CAP_FLG_STOP        0x80000000

/**
 */
uint64_t msGet(void)
{
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec*1000 + tv.tv_usec/1000;
}

/**
 */
typedef struct {
  char      dev[32];
//  uint32_t  fmt;
}sCfgCap, *psCfgCap;

/**
 */
typedef struct {
  char      bgfb   [32];
  char      fgfb   [32];
  uint32_t  xres;
  uint32_t  yres;
  uint32_t  bpp ;
  sCfgCap   capture[ 4];
  char      event  [32];
}sConfig, *psConfig;

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
  uint32_t          width;
  uint32_t          height;
  int               chgin;
}sTaskArg, *psTaskArg;

/**
 */
sConfig config = {
  .bgfb       = "/dev/fb0",
  .fgfb       = "/dev/fb1",
  .xres       = 1280,
  .yres       =  800,
  .bpp        =   16,
  .capture[0] = { .dev = "/dev/video3"/*, V4L2_PIX_FMT_YVYU*/ },
  .capture[1] = { .dev = "/dev/video2"/*, V4L2_PIX_FMT_YVYU*/ },
  .capture[2] = { .dev = "/dev/video1"/*, V4L2_PIX_FMT_YUYV*/ },
  .capture[3] = { .dev = "/dev/video0"/*, V4L2_PIX_FMT_YUYV*/ },
  .event      = "/dev/input/event2",
};

/**
 */
pthread_t   eventTid;
sTaskArg    capArg[CAP_NUM];

/**
 */
void*   eventThread(void* arg);
void*   capThread(void* arg);
int     capDevOpen(psTaskArg pTask);
int     capBufGet(psTaskArg pTask, int input, int wait);
int     capDevClose(psTaskArg pTask);

/**
 */
typedef struct {
  int x;
  int y;
  int w;
  int h;
}sRect;

/**
 */
int main(int argc, char **argv)
{
  int                 ret;
  int                 d;
  int                 i;
  psFbHnd             fbHnd;
  psGrxHnd            pGrx;
  psBltHnd            pBlt;
  sRect               winpos[] =  {
                                    {100, 100, 720, 574},
                                    {900,  50, 250, 200},
                                    {900, 300, 250, 200},
                                    {900, 550, 250, 200}
                                  };
  int                 bg_frm_idx;   // background frame index
  int                 bg_frm_ptr;   // background frame memory pointer

  printf("%s "__DATE__" "__TIME__"\n", argv[0]);

  if(argc>1){
    if( memcmp(argv[1], "32", 3)==0 ){config.bpp = 32;} else
    if( memcmp(argv[1], "16", 3)==0 ){config.bpp = 16;}
  }
  printf("%s BPP %d\n", argv[0], config.bpp);

  // start capture threads
  for(i=0; i<CAP_NUM; i++){
    memset(&capArg[i], 0, sizeof(sTaskArg));
    capArg[i].capIdx = i;
    capArg[i].input  = CAP_DEF_INPUT;
    capArg[i].width  = winpos[i].w;
    capArg[i].height = winpos[i].h;
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
    DBG_ERROR("Start keyboard event thread FAIL\n");
    return -1;
  }

  // Open fb1 DISP3 FG, graphic layer
  pGrx = grxOpen(config.fgfb, config.xres, config.yres, config.bpp);
  if(!pGrx){
    DBG_ERROR("grxOpen FAIL\n");
    return -1;
  }
  grxVideoBg(pGrx);
  for(i=0; i<4; i++){
    grxVideoBox(pGrx, winpos[i].x, winpos[i].y, winpos[i].w, winpos[i].h);
  }

  // Open Blitter
  pBlt = bltOpen(config.bpp);
  if(!pGrx){
    DBG_ERROR("bltOpenFAIL\n");
    return -1;
  }

  // Open fb0 DISP3 BG, video stream layer
  fbHnd = fbOpen(config.bgfb);
  fbSet(fbHnd, config.xres, config.yres, config.bpp, 2);
  fbVarShow(fbHnd);

  void* bg_virt;
  bg_virt = fbMemGet(fbHnd);
  memset(bg_virt, 0, fbHnd->size*2);

  fbPan(fbHnd, config.yres);
  bg_frm_idx = 0;
  bg_frm_ptr = fbHnd->phys;

  // main loop
  for(;;){
    for(i=0; i<CAP_NUM; i++){
      if(capArg[i].flag & CAP_FLG_STARTED){
        ret = capBufGet(&capArg[i], CAP_DEF_INPUT, -1);
        if(ret){
          break;
        }
        if(capArg[i].flag==0){
          continue;
        }
        bltCopy(pBlt,
                capArg[i].pCap->buf[capArg[i].bufIdx].offset, bg_frm_ptr,
                winpos[i].x, winpos[i].y,
                winpos[i].w, winpos[i].h);
      }
    }
    // upddate frame buffer
    bltUpdate(pBlt);
    if(bg_frm_idx){
      fbPan(fbHnd, config.yres);
      bg_frm_idx = 0;
      bg_frm_ptr = fbHnd->phys;
    }else{
      fbPan(fbHnd, 0);
      bg_frm_idx = 1;
      bg_frm_ptr = fbHnd->phys + fbHnd->size;
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

  DBG_PRINT("#%d:started arg is %p Device %s\n",
            index, arg, config.capture[index].dev);
  // TODO test per verificare utilizzo CPU DA TOGLIERE
  for(;;){
    ret = capDevOpen(pTask);
    if(ret){
      usleep(20000);
      continue;
    }
    break;
  }
  for(;;){
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
  char     *name = config.event;
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
      DBG_PRINT("eventThread: time %ld.%06ld, type %d, code %d, value %d\n",
                pEve->time.tv_sec, pEve->time.tv_usec,
                pEve->type, pEve->code, pEve->value);
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
          pTask->capIdx, config.capture[pTask->capIdx].dev);
  pTask->pCap = capOpen(config.capture[pTask->capIdx].dev);
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
  //pTask->pCap->fmt    = config.capture[pTask->capIdx].fmt;
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
 * @brief Get capture buffers from ring queue and enqueue last used buffer
 */
int capBufGet(psTaskArg pTask, int input, int wait)
{
  int   temp;
  int   ret;
  int   retval = -1;

  if(input<0 || input>1){
    return -1;
  }
  wait = wait? -1: 0;
  ret = wait;
  while(ret==wait){
    if(pTask->pCap==0 || (pTask->flag&CAP_FLG_STARTED)==0){
      return -1;
    }
    temp = pTask->bufIdx;
    ret = rngGet(pTask->pRng, &(pTask->bufIdx));
    if(ret==0){
      if(pTask->flag & CAP_FLG_FIRSTFRAME){
        struct v4l2_buffer  cap_buf;
        cap_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        cap_buf.index  = temp;
        cap_buf.memory = V4L2_MEMORY_MMAP;
        if(pTask->pCap){
          ret = ioctl(pTask->pCap->fd, VIDIOC_QBUF, &cap_buf);
        }
      }
      pTask->flag |= CAP_FLG_FIRSTFRAME;
      retval = 0;
    }
  }
  return retval;
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
