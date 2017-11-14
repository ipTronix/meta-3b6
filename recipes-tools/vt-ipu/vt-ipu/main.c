/** ***************************************************************************
 * @file main.c
 * @brief TeraHE Video test with ipu rotation
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
#include "ipu.h"
#include "grx.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "VT"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_WAIT(fmt, ...)    {printf(fmt, ##__VA_ARGS__);getchar();}
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
  #define DBG_WAIT(fmt, ...)
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
  int       input;
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
  char      event  [4][32];
}sConfig, *psConfig;

/**
 */
typedef struct {
  char             *dev;
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
  .capture[0] = { .dev = "/dev/video0", 1},
  .capture[1] = { .dev = "/dev/video1", 1},
  .capture[2] = { .dev = "/dev/video2", 1},
  .capture[3] = { .dev = "/dev/video3", 1},
  .event[0]   = "/dev/input/touchscreen0",
  .event[1]   = "/dev/input/event3",
  .event[2]   = "",
  .event[3]   = "",
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
int     capBufGet(psTaskArg pTask, int wait);
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
  int       ret;
  int       d;
  int       i;
  psFbHnd   fbHnd;
  psGrxHnd  pGrx;
  psBltHnd  pBlt;
  psIpuHnd  pIpu;
  sRect     winpos[] =  {
#if 0
                          {140,  56, 360, 288},
                          {780,  56, 360, 288},
                          {140, 456, 360, 288},
                          {780, 456, 360, 288}
#else
                          {140,  20, 288, 360},
                          {780,  20, 288, 360},
                          {140, 420, 288, 360},
                          {780, 420, 288, 360}
#endif
                        };
  int       bg_frm_idx;   // background frame index
  int       bg_frm_ptr;   // background frame memory pointer
  int       bg_cnt[4] = {2, 2, 2, 2};
  int       cnt;

  printf("%s "__DATE__" "__TIME__"\n", argv[0]);

  if(argc>1){
    if( memcmp(argv[1], "32", 3)==0 ){config.bpp = 32;} else
    if( memcmp(argv[1], "16", 3)==0 ){config.bpp = 16;}
  }
  DBG_PRINT("BPP %d\n", config.bpp);

  // start capture threads
  for(i=0; i<CAP_NUM; i++){
    memset(&capArg[i], 0, sizeof(sTaskArg));
    capArg[i].dev    = config.capture[i].dev;
    capArg[i].capIdx = i;
    capArg[i].input  = config.capture[i].input;
    capArg[i].width  = winpos[i].w;
    capArg[i].height = winpos[i].h;
    capArg[i].pRng   = rngCreate(CAP_NUM_BUF/2);
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

  // Start event thread
  for(i=0; i<2; i++){
    DBG_PRINT("Start %s event thread\n", config.event[i]);
    ret = pthread_create(&eventTid, NULL, eventThread, config.event[i]);
    if(ret){
      DBG_ERROR("Start %s event thread FAIL\n");
      return -1;
    }
  }

  // Open fb1 DISP3 FG, graphic layer
  DBG_PRINT("Open fb1 DISP3 FG, graphic layer\n");
  pGrx = grxOpen(config.fgfb, config.xres, config.yres, config.bpp);
  if(!pGrx){
    DBG_ERROR("grxOpen FAIL\n");
    return -1;
  }
  // draw graphic layer background
  grxVideoBg(pGrx);

  // draw video windows graphic overlay
  for(i=0; i<CAP_NUM; i++){
    grxBar(pGrx, winpos[i].x, winpos[i].y, winpos[i].w, winpos[i].h, 0x00000000);
  }

  // Open Blitter
  DBG_PRINT("Open Blitter\n");
  pBlt = bltOpen(config.bpp);
  if(!pBlt){
    DBG_ERROR("bltOpen FAIL\n");
    return -1;
  }

  // Open fb0 DISP3 BG, video stream layer
  DBG_PRINT("Open fb0 DISP3 BG, video stream layer\n");
  fbHnd = fbOpen(config.bgfb);
  if(!fbHnd){
    DBG_ERROR("Open fb0 DISP3 BG, video stream layer failed\n");
    return -1;
  }
  fbSet(fbHnd, config.xres, config.yres, config.bpp, 2);
  fbVarShow(fbHnd);

DBG_PRINT("fbMemGet\n");
  void* bg_virt;
  bg_virt = fbMemGet(fbHnd);
DBG_PRINT("bg_virt %p size %d\n", bg_virt, fbHnd->size);
  memset(bg_virt, 0, fbHnd->size*2);

DBG_PRINT("fbPan\n");
  fbPan(fbHnd, config.yres);
  bg_frm_idx = 0;
  bg_frm_ptr = fbHnd->phys;
#if 1
  // open ipu device
  pIpu = ipuOpen("/dev/mxc_ipu", config.bpp);
  if(!pIpu){
    DBG_ERROR("IPU open fail\n");
  }
#endif

  // main loop
  cnt = 0;
  for(;;){
    for(i=0; i<CAP_NUM; i++){
      if(capArg[i].flag & CAP_FLG_STARTED){
        if(bg_cnt[i]<2){
          grxBar(pGrx,
                winpos[i].x, winpos[i].y, winpos[i].w, winpos[i].h,
                0x00000000);
          bg_cnt[i]++;
        }
        ret = capBufGet(&capArg[i], -1);
        if( ret || capArg[i].flag==0 ){
          continue;
        }
#if 0
        bltCopy(pBlt,
                capArg[i].pCap->buf[capArg[i].bufIdx].offset, bg_frm_ptr,
                winpos[i].x, winpos[i].y,
                winpos[i].w, winpos[i].h);
#else
        bltCopy(pBlt,
                capArg[i].pCap->buf[capArg[i].bufIdx].offset, (int)pIpu->task.input.paddr,
                0, 0, winpos[i].h*2, winpos[i].w*2);
        bltUpdate(pBlt);
        ipuRotate(pIpu, bg_frm_ptr,
                  winpos[i].x, winpos[i].y, winpos[i].w, winpos[i].h,
                  IPU_ROTATE_90_RIGHT);
#endif
      }else{
        if(bg_cnt[i]){
          uint32_t  xx;
          uint32_t  yy;
          uint32_t  tt;
          uint16_t *p;

          // clear graphic layer
          grxBar(pGrx,
                winpos[i].x, winpos[i].y, winpos[i].w, winpos[i].h,
                0x0000A0A0);

          // clear video layer
          p = bg_virt;
          for(yy=winpos[i].y; yy<winpos[i].y+winpos[i].h; yy++){
            tt = yy * fbHnd->var.xres;
            for(xx=winpos[i].x; xx<winpos[i].x+winpos[i].w; xx++){
              p[xx + tt] = 0;
              p[xx + tt + fbHnd->var.xres*fbHnd->var.yres] = 0;
            }
          }

          bg_cnt[i]--;
        }
      }
    }

#if 0
    // update frame buffer
    bltUpdate(pBlt);
#endif
    if(bg_frm_idx){
      fbPan(fbHnd, config.yres);
      bg_frm_idx = 0;
      bg_frm_ptr = fbHnd->phys;
    }else{
      fbPan(fbHnd, 0);
      bg_frm_idx = 1;
      bg_frm_ptr = fbHnd->phys + fbHnd->size;
    }

    // update graphic
    if(cnt==0){
      grxBar(pGrx, 0, 390, 1280, 20, 0xFF808080);
    }
    grxBar(pGrx, cnt, 390, 80, 20, 0xFFFFFFFF);
    cnt+=80;
    if(cnt>=1280){
      cnt=0;
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
  for(;;){
    // if is set CAP_FLG_STOP close capture device
    if(pTask->flag & CAP_FLG_STOP){
      DBG_PRINT("#%d:acquisition stopped close device.\n", index);
      capDevClose(pTask);
      pTask->flag = CAP_FLG_STOPPED;
    }
    // if is set CAP_FLG_STOPPED skip acquisition
    if(pTask->flag & CAP_FLG_STOPPED){
      continue;
    }
    // if capture device is close open it
    if(pTask->pCap==NULL){
      ret = capDevOpen(pTask);
      if(ret){
        usleep(20000);
        continue;
      }
      DBG_PRINT("#%d:device %s, input %d opened.\n",
                index, config.capture[index].dev, pTask->input);
    }
    // if capture device is no started (CAP_FLG_STARTED is clear) skip acquisition
    if((pTask->flag & CAP_FLG_STARTED)==0){
      DBG_ERROR("#%d:fd %d capture device %s not started\n",
                  index, pTask->pCap->fd, pTask->pCap->name);
      usleep(20000);
      continue;
    }

    // Dequeue capture buffer
    memset(&capture_buf, 0, sizeof(capture_buf));
    capture_buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    capture_buf.memory = V4L2_MEMORY_MMAP;
    ret = ioctl(pTask->pCap->fd, VIDIOC_DQBUF, &capture_buf);
    if(ret<0){
      if(errno!=11){
        DBG_ERROR("#%d:fd %d VIDIOC_DQBUF capture %s failed (%d) %d %s.\n",
                  index, pTask->pCap->fd, pTask->pCap->name, ret,
                  errno, strerror(errno));
      }
      continue;
    }

    // put buffer to ring queue
    ret = rngPut(pTask->pRng, capture_buf.index);
    if(ret){
      //DBG_ERROR("#%d: put buffer overflow\n", index);
      ret = ioctl(pTask->pCap->fd, VIDIOC_QBUF, &capture_buf);
      if(ret){
        DBG_ERROR("#%d:VIDIOC_QBUF\n", index);
      }
    }

  }
  return NULL;
}


/** ***************************************************************************
 *
 *************************************************************************** */

/**
 */
static void capOn(int arg)
{
  if ( capArg[arg].flag == CAP_FLG_STOPPED ) {
    DBG_PRINT("Start capture device %d\n", arg);
    capArg[arg].flag = 0;
  }
}
/**
 */
static void capOff(int arg)
{
  if ( capArg[arg].flag & CAP_FLG_STARTED ) {
    DBG_PRINT("Stop capture device %d\n", arg);
    capArg[arg].flag = CAP_FLG_STOP;
  }
}

/**
 */
int32_t   Brightness = 128;
int32_t   Hue        = 128;
int32_t   Contrast   = 128;
int32_t   Saturation = 128;
int32_t   Sharpness  = 128;

void ctrlset(int arg)
{
  DBG_PRINT("arg %d\n", arg);
  switch(arg){
  case 0: Brightness -= (Brightness==  0)? 0: 1; v4l2Brightness(capArg[2].pCap->fd, Brightness); break;
  case 1: Brightness += (Brightness==255)? 0: 1; v4l2Brightness(capArg[2].pCap->fd, Brightness); break;
  case 2: Contrast   -= (Contrast  ==  0)? 0: 1; v4l2Contrast(capArg[2].pCap->fd, Contrast);break;
  case 3: Contrast   += (Contrast  ==255)? 0: 1; v4l2Contrast(capArg[2].pCap->fd, Contrast);break;
  }
  DBG_PRINT("New value Brightness: %d, Contrast: %d\n", Brightness, Contrast);
}

typedef struct {
  int   x;
  int   y;
  int   w;
  int   h;
  void(*f)(int);
  int   a;
}sEveBox, *psEveBox;

#if 1
sEveBox evebox[] = {
  {  0,   0, 640, 200, capOn , 0},
  {  0, 200, 640, 200, capOff, 0},
  {640,   0, 640, 200, capOn , 1},
  {640, 200, 640, 200, capOff, 1},
  {  0, 400, 640, 200, capOn , 2},
  {  0, 600, 640, 200, capOff, 2},
  {640, 400, 640, 200, capOn , 3},
  {640, 600, 640, 200, capOff, 3},
};
#else
sEveBox evebox[] = {
  {  0, 400, 640, 200, ctrlset, 0},
  {  0, 600, 640, 200, ctrlset, 1},
  {640, 400, 640, 200, ctrlset, 2},
  {640, 600, 640, 200, ctrlset, 3},
};
#endif

/**
 */
void tsEvent(int x, int y)
{
  int i;

  DBG_PRINT("x: %d, y: %d\n", x, y);
  for(i=0; i<sizeof(evebox)/sizeof(sEveBox); i++){
    if( x>evebox[i].x && x<(evebox[i].x + evebox[i].w) &&
        y>evebox[i].y && y<(evebox[i].y + evebox[i].h) ){
      if(evebox[i].f){
        evebox[i].f(evebox[i].a);
        break;
      }
    }
  }
}

/**
 * @brief Thread per l'acquisizione dell'evento da tastiera per la modifica
 *        dell'ingresso.
 */
void* eventThread(void* arg)
{
  char     *name = (char*)arg;
  psEvHnd   pHnd;
  psEvent   pEve;
  int       xe;
  int       ye;

  DBG_PRINT("started input device is %s\n", name);

  pHnd = evOpen(name);
  if(!pHnd){
    return NULL;
  }
  xe = -1;
  ye = -1;
  for(;;){
    pEve = evGet(pHnd);
    if(!pEve){
      continue;
    }
/*
    DBG_PRINT("eventThread: time %ld.%06ld, type %d, code %d, value %d\n",
              pEve->time.tv_sec, pEve->time.tv_usec,
              pEve->type, pEve->code, pEve->value);
*/
    switch(pEve->type){
    case EV_KEY:
      if(pEve->value==1){
        switch(pEve->code){
        case 6:
          //system("md5sum /usr/apps/3b6/* > /tmp/load1");
          break;
        case 4:
          break;
        }
      }
    case EV_ABS:
      switch(pEve->code){
      case ABS_X: xe = pEve->value; break;
      case ABS_Y: ye = pEve->value; break;
      }
      if(xe<0 || ye<0){
        break;
      }
      if(pEve->code==ABS_MT_TRACKING_ID && pEve->value==-1){
        tsEvent(1280*xe/32768, 800*ye/32768);
      }
      break;
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

//  DBG_PRINT("#%d:capOpen device %s.\n",
//          pTask->capIdx, config.capture[pTask->capIdx].dev);
//  pTask->pCap = capOpen(config.capture[pTask->capIdx].dev);
  DBG_PRINT("#%d:capOpen device %s.\n",
          pTask->capIdx, pTask->dev);
  pTask->pCap = capOpen(pTask->dev);
  if(!pTask->pCap){
    DBG_ERROR("#%d:capOpen device %s failed.\n",
              pTask->capIdx, pTask->dev);
    return -1;
  }
  pTask->flag = CAP_FLG_OPENED;
  DBG_PRINT("#%d:capOpen device %s fd %d OK.\n",
            pTask->capIdx, pTask->pCap->name, pTask->pCap->fd);
  // impostazione device
  pTask->pCap->width  = CAP_WIDTH;
  pTask->pCap->height = CAP_HEIGHT;
  //pTask->pCap->fmt    = pTask->fmt;
  pTask->pCap->fmt    = V4L2_PIX_FMT_YUYV;
  //pTask->pCap->fmt    = V4L2_PIX_FMT_UYVY;
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
 * @param pTask pointer to task definition structure
 * @param wait  0 loop until there are buffers in pTask->pRng queue
 *             -1 wait for a buffer in pTask->pRng queue
 */
int capBufGet(psTaskArg pTask, int wait)
{
  int   temp;
  int   ret;
  int   retval = -1;

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
//    usleep(1);
  }
  return retval;
}

/**
 */
int capDevClose(psTaskArg pTask)
{
  int i;

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
