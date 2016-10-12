/** ***************************************************************************
 * @file grx.c
 * @brief
 **************************************************************************** */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "fb.h"
#include "grx.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "GRX"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif

/**
 */
#define R32 0x00FF0000
#define G32 0x0000FF00
#define B32 0x000000FF
#define PALETTE32 {0, R32, G32, R32|G32, B32, R32|B32, G32|B32, R32|G32|B32}

#define R16 0xF800
#define G16 0x07E0
#define B16 0x001F
#define W16 R16|G16|B16
#define PALETTE16 {0, R16, G16, R16|G16, B16, R16|B16, G16|B16, W16};


void rect16(psGrxHnd pHnd, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t c);
void rect32(psGrxHnd pHnd, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t c);

/**
 */
psGrxHnd grxOpen(char* device, uint32_t xres, uint32_t yres, uint32_t bpp)
{
  psGrxHnd  pHnd;
  int       ret;

  pHnd = (psGrxHnd)malloc(sizeof(sGrxHnd));
  if(!pHnd){
    return NULL;
  }
  memset(pHnd, 0, sizeof(sGrxHnd));

  strncpy(pHnd->device, device, sizeof(pHnd->device)-1);
  pHnd->fbHnd = fbOpen(device);
  if(!pHnd->fbHnd){
    free(pHnd);
    return NULL;
  }

  pHnd->xres = xres;
  pHnd->yres = yres;
  pHnd->bpp  = bpp ;

  ret = fbSet(pHnd->fbHnd, pHnd->xres, pHnd->yres, pHnd->bpp, 1);
  if(ret){
    DBG_ERROR("grx Open Error\n");
  }
  fbVarShow(pHnd->fbHnd);

  // get framebuffer memory pointer
  pHnd->fbMem = fbMemGet(pHnd->fbHnd);
  if(!pHnd->fbMem){
    DBG_ERROR("fb get memory pointer fail\n");
  }

  if(bpp==16){
    // set color key
    ret = fbColorKey(pHnd->fbHnd, 1, 0);
    if(ret){
      DBG_ERROR("fb set color key fail\n");
    }
    // set Global alpha
    ret = fbGlobalAlpha(pHnd->fbHnd, 1, 0xFF);
    if(ret){
      DBG_ERROR("fb set color key fail\n");
    }
  }else{
    // clear color key
    ret = fbColorKey(pHnd->fbHnd, 0, 0);
    if(ret){
      DBG_ERROR("fb set color key fail\n");
    }
    // set Global alpha
    ret = fbGlobalAlpha(pHnd->fbHnd, 0, 0);
    if(ret){
      DBG_ERROR("fb set color key fail\n");
    }
    ret = fbLocalAlpha(pHnd->fbHnd, 1, 1);
    if(ret){
      DBG_ERROR("fb set Local Alpha fail\n");
    }
  }

  return pHnd;
}

/**
 */
int grxClose(psGrxHnd pHnd)
{
  fbClose(pHnd->fbHnd);
  free(pHnd);
  return 0;
}

/**
 */
int grxVideoBg(psGrxHnd pHnd)
{
  uint32_t  xx;
  uint32_t  yy;

  switch(pHnd->bpp){
  case 16:
    {
      uint16_t *p = (uint16_t*)pHnd->fbMem;
      uint16_t  palette[8] = PALETTE16;

      for(yy=0; yy<pHnd->yres; yy++){
        for(xx=0; xx<pHnd->xres; xx++){
          p[xx + yy * pHnd->xres] = palette[xx/(pHnd->xres/8)];
        }
      }

    }
    break;
  case 32:
    {
      uint32_t *p = (uint32_t*)pHnd->fbMem;
      uint32_t  palette[8] = PALETTE32;

      for(yy=0; yy<pHnd->yres; yy++){
        for(xx=0; xx<pHnd->xres; xx++){
          p[xx + yy * pHnd->xres] = palette[xx/(pHnd->xres/8)] | 0xFF000000;
        }
      }
    }
    break;
  }
  return 0;
}

/**
 */
int grxVideoBox(psGrxHnd pHnd, uint32_t x, uint32_t y, uint32_t w, uint32_t h)
{
  if( ( (x+w) >= pHnd->xres) || ((y+h) >= pHnd->yres) ){
    return -1;
  }
  switch(pHnd->bpp){
  case 16:
    rect16(pHnd, x, y, w, h, 0x0000);
    rect16(pHnd, x+w*1/8, y+h*1/8, w*6/8, h*2/8, R16);
    rect16(pHnd, x+w*1/8, y+h*5/8, w*6/8, h*2/8, W16);
    break;
  case 32:
    rect32(pHnd, x, y, w, h, 0x00000000);
    rect32(pHnd, x+w*1/8, y+h*1/8, w*6/8, h*2/8, 0xFF000000);//nero opaco
    rect32(pHnd, x+w*1/8, y+h*5/8, w*6/8, h*2/8, 0x80000000);//nero 50%
    break;
  }

  return 0;
}


/**
 */
void grxBar(psGrxHnd pHnd, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t c)
{
  uint16_t c16;
  switch(pHnd->bpp){
  case 16:
    c16 = ((c>>8) & 0xF800) |
          ((c>>5) & 0x07E0) |
          ((c>>3) & 0x001F) ;
    rect16(pHnd, x, y, w, h, c16);
    break;
  case 32:
    rect32(pHnd, x, y, w, h, c);
    break;
  }
}


/**
 */
void rect16(psGrxHnd pHnd, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint16_t c)
{
  uint32_t  xx;
  uint32_t  yy;
  uint16_t *p = (uint16_t*)pHnd->fbMem;

  for(yy=y; yy<y+h; yy++){
    for(xx=x; xx<x+w; xx++){
      p[xx + yy * pHnd->xres] = c;
    }
  }
}

/**
 */
void rect32(psGrxHnd pHnd, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t c)
{
  uint32_t  xx;
  uint32_t  yy;
  uint32_t *p = (uint32_t*)pHnd->fbMem;

  for(yy=y; yy<y+h; yy++){
    for(xx=x; xx<x+w; xx++){
      p[xx + yy * pHnd->xres] = c;
    }
  }
}
