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
#include "out.h"
#include "cap.h"
#include "v4l2.h"

typedef struct {
  unsigned char  *start;
  int             width;
  int             height;
  int             bpl;    //bytes per line
  int             imsize; //image size
}sUsbFrame, *psUsbFrame;

sUsbFrame usbFrame[4];



/**
 */
pthread_t   devScanTid;


void* devScanThread(void* arg);
int getUsbFrame(char* device, int input, psUsbFrame pBuf);


// TODO spostare in out e in cap
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
    printf( "outBufGet:ERROR:VIDIOC_DQBUF failed (%d).\n", ret);
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
    printf("outBufPut: VIDIOC_QBUF failed (%d)\n", ret);
    return -1;
  }
  return 0;
}

/**
 */
struct v4l2_buffer* capBufGet(psCapDev pCap)
{
  static struct v4l2_buffer buf;
  int ret;
  memset(&buf, 0, sizeof(struct v4l2_buffer));
  buf.type   = V4L2_BUF_TYPE_VIDEO_CAPTURE;
  buf.memory = V4L2_MEMORY_MMAP;
  ret = ioctl(pCap->fd, VIDIOC_DQBUF, &buf);
  if(ret<0){
    printf( "capBufGet:ERROR:VIDIOC_DQBUF failed (%d).\n", ret);
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
    printf("capBufPut: VIDIOC_QBUF failed (%d)\n", ret);
    return -1;
  }
  return 0;
}


/**
 */
void bufcpy(uint8_t* dst, uint8_t* src, uint16_t w, uint16_t h, int idx)
{
  int i;
  switch(idx){
  case 0: break;
  case 1: dst += w; break;
  case 2: dst += w*h*2; break;
  case 3: dst += w*h*2 + w; break;
  default: return;
  }
  for(i=0; i<h; i++){
    memcpy(dst, src, w);
    src += w;
    dst += w*2;
  }
}

/**
 */
int main(int argc, char **argv)
{
  /*
  int         ret;
  int         old;
  char        buf[32*32];
  char       *dev[32];
  int         i;
  int         num;
  sV4l2Info   inf;
  char        usbvideodev[32][32];
*/
  int i;
  char               *outdev = "/dev/video17"; //TODO da file config o da command line
  psOutDev            pOut;
  struct v4l2_buffer *outb;

  char               *capdev = "/dev/video4"; //TODO da thread
  psCapDev            pCap[4];
  struct v4l2_buffer *capb;

  uint8_t            *page1_buf;
  int                 page1_len;
int ret;
  printf("%s "__DATE__" "__TIME__"\n", argv[0]);



  for(i=0; i<4; i++){
    usbFrame[i].width  = 720;
    usbFrame[i].height = 576;
    usbFrame[i].bpl    = usbFrame[i].width * 2;
    usbFrame[i].imsize = usbFrame[i].bpl * usbFrame[i].height;
    usbFrame[i].start = (unsigned char*)malloc(usbFrame[i].imsize);
    if(!usbFrame[i].start){
      printf("Errore memoria\n");
      return -1;
    }
  }

  // Output open
  pOut = outOpen(outdev);
  if(!pOut){
    printf("error opening %s output device\n", outdev);
    return -1;
  }
  pOut->fmt = V4L2_PIX_FMT_YUYV;
  pOut->width  = 720*2;//1280;//capture[0].width  * 2;
  pOut->height = 576*2;// 800;//capture[0].height * 2;
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

  //
  if(outStart(pOut)){
    printf("Could not start output stream\n");
    return -1;
  }
/*
  // schermata iniziale
  outb = outBufGet(pOut);
  if(outb){
    printf("load page1.uyvy\n");
    page1_len = fsFileLoad("page1.uyvy", &page1_buf);
    if(page1_len){
      memcpy(pOut->buf[outb->index].start, page1_buf, page1_len);
      free(page1_buf);
    }else{
      printf("Error while loading page1.uyvy\n");
    }
    outBufPut(pOut, outb);
  }else{
    printf("Error outBufGet fail\n");
  }
*/
  //
/*
  // Capture open TODO da spostare
  pCap[0] = capOpen(capdev);
  if(!pCap[0]){
    printf("error opening capture device %s\n", capdev);
    return -1;
  }
  pCap[0]->fmt   = V4L2_PIX_FMT_YUYV;
  pCap[0]->input = 1;
  if(capSetup(pCap[0])){
    printf("capSetup v4l2 capture failed.\n");
    return -1;
  }
  // capture Start
  if(capStart(pCap[0])){
    printf("Could not start capture stream\n");
    return -1;
  }
*/
  for(;;){
    outb = outBufGet(pOut);
    if(outb){
      ret = getUsbFrame("/dev/video4", 0, &usbFrame[0]);
      if(!ret){
        bufcpy(pOut->buf[outb->index].start,
              usbFrame[0].start,
              usbFrame[0].bpl, usbFrame[0].height, 0);
      }
      ret = getUsbFrame("/dev/video4", 1, &usbFrame[0]);
      if(!ret){
        bufcpy(pOut->buf[outb->index].start,
              usbFrame[0].start,
              usbFrame[0].bpl, usbFrame[0].height, 1);
      }

#if 0
//printf("capBufGet\n");
/*
      // Capture open TODO da spostare
      pCap[0] = capOpen(capdev);
      if(!pCap[0]){
        printf("error opening capture device %s\n", capdev);
        //return -1;
        continue;
      }
      pCap[0]->fmt = V4L2_PIX_FMT_YUYV;
      if(capSetup(pCap[0])){
        printf("capSetup v4l2 capture failed.\n");
        //return -1;
        continue;
      }
*/
/**/
/*
  pCap[0]->input = 0;
  if(capSetup(pCap[0])){
    printf("capSetup v4l2 capture failed.\n");
    return -1;
  }*/
//capInputSet(pCap[0], 0);
  pCap[0] = capOpen(capdev);
  if(!pCap[0]){
    printf("error opening capture device %s\n", capdev);
    goto update_out;
  }
  printf("capSetup input 0.\n");
  pCap[0]->fmt   = V4L2_PIX_FMT_YUYV;
  pCap[0]->input = 0;
  if(capSetup(pCap[0])){
    printf("capSetup v4l2 capture failed.\n");
    goto update_out;
  }

      if(capStart(pCap[0])){
        printf("Could not start capture stream\n");
        //return -1;
        goto update_out;
      }
      printf("capBufGet wait for input %d\n", pCap[0]->input);
      capb = capBufGet(pCap[0]);
      if(capb){
        printf("capBufGet input %d\n", pCap[0]->input);
        bufcpy(pOut->buf[outb->index].start,
              pCap[0]->buf[capb->index].start,
              pCap[0]->bpl, pCap[0]->height, 0);
        capBufPut(pCap[0], capb);
      }
      if(capStop(pCap[0])){
        printf("Could not stop capture stream\n");
        //return -1;
        goto update_out;
      }
      capClose(pCap[0]);
/**/
/**/
  pCap[0] = capOpen(capdev);
  if(!pCap[0]){
    printf("error opening capture device %s\n", capdev);
    goto update_out;
  }
  printf("capSetup input 1.\n");
  pCap[0]->fmt   = V4L2_PIX_FMT_YUYV;
  pCap[0]->input = 1;
  if(capSetup(pCap[0])){
    printf("capSetup v4l2 capture failed.\n");
    goto update_out;
  }

//capInputSet(pCap[0], 1);

      if(capStart(pCap[0])){
        printf("Could not start capture stream\n");
        //return -1;
        goto update_out;
      }
      printf("capBufGet wait for input %d\n", pCap[0]->input);
      capb = capBufGet(pCap[0]);
      if(capb){
        printf("capBufGet input %d\n", pCap[0]->input);
        bufcpy(pOut->buf[outb->index].start,
              pCap[0]->buf[capb->index].start,
              pCap[0]->bpl, pCap[0]->height, 1);
        capBufPut(pCap[0], capb);
      }
      if(capStop(pCap[0])){
        printf("Could not stop capture stream\n");
        //return -1;
        goto update_out;
      }
// /**/

      capClose(pCap[0]);

update_out:
#endif
      outBufPut(pOut, outb);
    }
  }
/*
  // TODO spostare in un thread per la verifica delle periferiche video collegate
  for(i=0; i<32; i++){
    dev[i] = buf + i*32;
  }
  old = 0;
  for(;;){
    num = 0;
    ret = fsDirGet("/dev/", "video", dev, 32);
    if(ret!=old){
      for(i=0; i<ret; i++){
        v4l2InfoGet(dev[i], &inf);
        if(inf.num_input==2 && memcmp(inf.bus_info,"usb", 3)==0){
          v4l2InfoShow(&inf);
          strcpy(usbvideodev[num], inf.device);
          num++;
        }
      }
      printf("====================================\n");
      printf("trovate %d USB\n", num);
      for(i=0; i<num; i++){
        printf("%d %s\n", i, usbvideodev[i]);
      }
      old = ret;
    }
  }
*/
  return 0;
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
  int         num;
  sV4l2Info   inf;
  char        usbvideodev[32][32];

  for(i=0; i<32; i++){
    dev[i] = buf + i*32;
  }
  old = 0;
  for(;;){
    num = 0;
    ret = fsDirGet("/dev/", "video", dev, 32);
    if(ret!=old){
      for(i=0; i<ret; i++){
        v4l2InfoGet(dev[i], &inf);
        if(inf.num_input==2 && memcmp(inf.bus_info,"usb", 3)==0){
          //v4l2InfoShow(&inf);
          strcpy(usbvideodev[num], inf.device);
          num++;
        }
      }
      //TODO usbDeviceNum(num);
      printf("====================================\n");
      printf("trovate %d USB\n", num);
      for(i=0; i<num; i++){
        //TODO usbUpdate(dev[i], num);
        printf("  %d %s\n", i, usbvideodev[i]);
      }
      old = ret;
    }
  }
  return NULL;
}


int getUsbFrame(char* device, int input, psUsbFrame pBuf)
{
  psCapDev            pCap;
  struct v4l2_buffer *capb;

printf("capOpen device %s input %d.\n", device, input);
  pCap = capOpen(device);
  if(!pCap){
printf("error opening capture device %s\n", device);
    return -1;
  }
printf("capSetup device %s input %d.\n", device, input);
  pCap->fmt   = V4L2_PIX_FMT_YUYV;
  pCap->input = 0;
  if(capSetup(pCap)){
printf("capSetup device %s input %d failed.\n", device, input);
    capClose(pCap);
    return -1;;
  }

  if(capStart(pCap)){
printf("Could not start capture streamdevice %s input %d.\n", device, input);
    capClose(pCap);
    return -1;
  }
printf("capBufGet wait for input %d\n", pCap->input);
  capb = capBufGet(pCap);
  if(capb){
printf("capBufGet input %d\n", pCap->input);
    memcpy(pBuf->start, pCap->buf[capb->index].start, pBuf->imsize);
    capBufPut(pCap, capb);
  }
  if(capStop(pCap)){
printf("Could not stop capture stream\n");
    capClose(pCap);
    return -1;
  }
  capClose(pCap);


  return 0;
}