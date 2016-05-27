/** ***************************************************************************
 * @file blt.c
 * @brief TeraHE Video test
 **************************************************************************** */

/**
 * INCLUDE FILES
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "blt.h"
#include "g2d.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "BLT"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif

/**
 */
psBltHnd bltOpen(int bpp)
{
  psBltHnd  pHnd;
  int       ret;
  int       tmp;

  pHnd = (psBltHnd)malloc(sizeof(sBltHnd));
  if(!pHnd){
    return NULL;
  }
  memset(pHnd, 0, sizeof(sBltHnd));

  // get g2d handle
  ret = g2d_open(&pHnd->g2dHnd);
  if(ret){
    DBG_ERROR("g2d_open fail.\n");
    free(pHnd);
    return NULL;
  }
  ret = g2d_make_current(pHnd->g2dHnd, G2D_HARDWARE_VG);
  if(ret){
    DBG_ERROR("g2d_make_current fail.\n");
    g2d_close(pHnd->g2dHnd);
    free(pHnd);
    return NULL;
  }
/*
 * Video format
 *   G2D_RGB565
 *   G2D_RGBA8888
 *   G2D_RGBX8888
 *   G2D_BGRA8888
 *   G2D_BGRX8888
 *   G2D_NV12
 *   G2D_I420
 *   G2D_YV12
 *   G2D_NV21
 *   G2D_YUYV
 *   G2D_YVYU
 *   G2D_UYVY
 *   G2D_NV16
 */

  pHnd->cap = 0;
  ret = g2d_query_cap(pHnd->g2dHnd, G2D_BLEND, &tmp);
  if(!ret){
    pHnd->cap |= BLT_CAP_BLEND;
  }
  ret = g2d_query_cap(pHnd->g2dHnd, G2D_DITHER, &tmp);
  if(!ret){
    pHnd->cap |= BLT_CAP_DITHER;
  }
  ret = g2d_query_cap(pHnd->g2dHnd, G2D_GLOBAL_ALPHA, &tmp);
  if(!ret){
    pHnd->cap |= BLT_CAP_GL_ALPHA;
  }
  DBG_PRINT("Capabilities %08X.\n", pHnd->cap);

  pHnd->src_fmt  = G2D_YUYV;
  pHnd->src_xres =  720;
  pHnd->src_yres =  574;

  switch(bpp){
  case 16: pHnd->dst_fmt  = G2D_RGB565  ; break;
  case 32: pHnd->dst_fmt  = G2D_RGBA8888; break;
  }
  pHnd->dst_xres = 1280;
  pHnd->dst_yres =  800;

  pHnd->blend    = G2D_ZERO;
  return pHnd;
}

/**
 */
void bltClose(psBltHnd pHnd)
{
  int ret;
  ret = g2d_close(pHnd->g2dHnd);
  if(ret){
    DBG_ERROR("g2d_close FAIL\n");
  }
  free(pHnd);
}

/**
 * Copia il frame acquisito da telecamera (CAP_WIDTH * CAP_HEIGHT)
 * sul frame buffer alle coordinate indicate (x, y)
 * scalando le dimensioni a (w, h)
 *
 */
int bltCopy(psBltHnd pHnd, int src_phys, int dst_phys,
            uint32_t dst_x, uint32_t dst_y, uint32_t dst_w, uint32_t dst_h)
{
  struct g2d_surface  src;
  struct g2d_surface  dst;
  int                 ret;

  src.format       = pHnd->src_fmt;
  src.planes[0]    = src_phys;
  src.left         = 0;
  src.top          = 0;
  src.right        = pHnd->src_xres;
  src.bottom       = pHnd->src_yres;
  src.stride       = pHnd->src_xres;
  src.width        = pHnd->src_xres;
  src.height       = pHnd->src_yres;
  src.blendfunc    = pHnd->blend;
  src.global_alpha = 0;
  src.clrcolor     = 0;
  src.rot          = G2D_ROTATION_0;

  dst.format       = pHnd->dst_fmt;
  dst.planes[0]    = dst_phys;
  dst.left         = dst_x;
  dst.top          = dst_y;
  dst.right        = dst_x+dst_w;
  dst.bottom       = dst_y+dst_h;
  dst.stride       = pHnd->dst_xres;
  dst.width        = pHnd->dst_xres;
  dst.height       = pHnd->dst_yres;
  dst.blendfunc    = G2D_ZERO;
  dst.global_alpha = 0;
  dst.clrcolor     = 0;
  dst.rot          = G2D_ROTATION_0;

  ret = g2d_blit(pHnd->g2dHnd, &src, &dst);
  if(ret==-1){
    DBG_ERROR("g2d_blit fail\n");
    return -1;
  }
  return 0;
}

/**
 */
int bltUpdate(psBltHnd pHnd)
{
  int ret;
  ret = g2d_finish(pHnd->g2dHnd);
  if(ret==-1){
    DBG_ERROR("g2d_finish FAIL\n");
  }
  return ret;
}
