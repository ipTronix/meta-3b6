/** ***************************************************************************
 * @file v4l2.c
 * @brief
 **************************************************************************** */
#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <linux/videodev2.h>

#include "fb.h"
#include "mxcfb.h"

#define DEBUG
#ifdef DEBUG
  #define DBG_NAME "FB"
  #define DBG_PRINT(fmt, ...)   printf("%s:%s:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
  #define DBG_ERROR(fmt, ...)   printf("%s:%s:ERROR:"fmt, DBG_NAME, __func__, ##__VA_ARGS__);
#else
  #define DBG_PRINT(fmt, ...)
  #define DBG_ERROR(fmt, ...)
#endif


/**
 */
psFbHnd fbOpen(char* device)
{
  psFbHnd                   pHnd;
  int                       ret;

  pHnd = (psFbHnd)malloc(sizeof(sFbHnd));
  if(!pHnd){
    DBG_ERROR("memory\n");
    return NULL;
  }
  memset(pHnd, 0, sizeof(sFbHnd));

  pHnd->fd = open(device, O_RDWR, 0);
  if(pHnd->fd<0){
    DBG_ERROR("Unable to open %s\n", device);
    free(pHnd);
    return NULL;
  }
  strncpy(pHnd->device, device, sizeof(pHnd->device));

  /* Do Ioctl. Retrieve fixed screen info. */
  ret = ioctl(pHnd->fd, FBIOGET_FSCREENINFO, &pHnd->fix);
  if(ret<0){
    DBG_ERROR("get fixed screen info failed: %s\n", strerror(errno));
    close(pHnd->fd);
    free(pHnd);
    return NULL;
  }
  /* Do Ioctl. Get the variable screen info. */
  ret = ioctl(pHnd->fd, FBIOGET_VSCREENINFO, &pHnd->var);
  if(ret<0){
    DBG_ERROR("Unable to retrieve variable screen info: %s\n", strerror(errno));
    close(pHnd->fd);
    free(pHnd);
    return NULL;
  }
/*
uint32_t ipu_ch;
  ret = ioctl(pHnd->fd, MXCFB_GET_FB_IPU_CHAN, &ipu_ch);
  if(ret<0){
    DBG_ERROR("Unable to retrieve IPU Channel: %s\n", strerror(errno));
    close(pHnd->fd);
    free(pHnd);
    return NULL;
  }
  DBG_PRINT("%s, IPU Channel %08X\n", device, ipu_ch);
*/

  DBG_PRINT("%s, id %s, line_length %d var xres %d, yres %d, bits_per_pixel %d\n",
         device, pHnd->fix.id, pHnd->fix.line_length,
         pHnd->var.xres, pHnd->var.yres, pHnd->var.bits_per_pixel);
  DBG_PRINT("%s, id %s, phys %p\n",
         device, pHnd->fix.id, pHnd->fix.smem_start);
  pHnd->phys = pHnd->fix.smem_start;
  return pHnd;
}

/**
 */
int fbClose(psFbHnd pHnd)
{
  close(pHnd->fd);
  free(pHnd);
  return 0;
}

/**
 * Calculate the size to mmap and Now mmap the framebuffer.
 *
 */
void* fbMemGet(psFbHnd pHnd)
{
  void *ptr;
  int   size;

  size = pHnd->fix.line_length * pHnd->var.yres_virtual;
  ptr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_SHARED, pHnd->fd, 0);
  DBG_PRINT("mmap return %p size %d\n", ptr, size);
  return ptr;
}

/**
 */
/* TODO
void fbClear(psFbHnd pHnd)
{
  int   size;

  size = pHnd->fix.line_length * pHnd->var.yres_virtual;
  DBG_PRINT("clear memory @ %p size %d\n", pHnd->phys, size);
  memset((void*)pHnd->phys, 0, size);
}
*/
/**
 */
int fbSet(psFbHnd pHnd, uint32_t xres, uint32_t yres, uint32_t bpp, uint32_t nb)
{
//  struct mxcfb_gbl_alpha    ga;
  int ret;

  // Get var screen info
  ret = ioctl(pHnd->fd, FBIOGET_VSCREENINFO, &pHnd->var);
  if(ret<0){
    DBG_ERROR("ioctl FBIOGET_VSCREENINFO\n");
    return -1;
  }
  if(nb<1){
    nb = 1;
  }else if(nb>FB_MAX_BUFFER){
    nb = FB_MAX_BUFFER;
  }
  // Set var screen info
  pHnd->var.bits_per_pixel = bpp;
  pHnd->var.xres   = xres;
  pHnd->var.yres   = yres;
  if(bpp==16){
    pHnd->var.nonstd = 0;
#if 0
/*
#define V4L2_PIX_FMT_RGB555  v4l2_fourcc('R','G','B','O') // 16  RGB-5-5-5
#define V4L2_PIX_FMT_RGB565  v4l2_fourcc('R','G','B','P') // 16  RGB-5-6-5
#define V4L2_PIX_FMT_RGB555X v4l2_fourcc('R','G','B','Q') // 16  RGB-5-5-5 BE
*/
    //pHnd->var.nonstd = 0;
    //pHnd->var.nonstd = v4l2_fourcc('R', 'G', 'B', '0');
    //pHnd->var.nonstd = v4l2_fourcc('Y', 'U', 'Y', 'V');
    pHnd->var.nonstd = v4l2_fourcc('R','G','B','P');
/*
    pHnd->var.transp.offset = 15; pHnd->var.transp.length =  1;
    pHnd->var.red.offset    = 10; pHnd->var.red.length    =  5;
    pHnd->var.green.offset  =  5; pHnd->var.green.length  =  5;
    pHnd->var.blue.offset   =  0; pHnd->var.blue.length   =  5;
*/

    pHnd->var.transp.offset =  0; pHnd->var.transp.length =  0;
    pHnd->var.red.offset    = 11; pHnd->var.red.length    =  5;
    pHnd->var.green.offset  =  5; pHnd->var.green.length  =  6;
    pHnd->var.blue.offset   =  0; pHnd->var.blue.length   =  5;
#endif
  }else{
    pHnd->var.nonstd = v4l2_fourcc('B', 'G', 'R', 'A');
    pHnd->var.transp.offset = 24; pHnd->var.transp.length =  8;
    pHnd->var.red.offset    = 16; pHnd->var.red.length    =  8;
    pHnd->var.green.offset  =  8; pHnd->var.green.length  =  8;
    pHnd->var.blue.offset   =  0; pHnd->var.blue.length   =  8;
  }
  pHnd->var.activate     = FB_ACTIVATE_FORCE;
  pHnd->var.xres_virtual = pHnd->var.xres;
  pHnd->var.yres_virtual = pHnd->var.yres * nb;
  pHnd->var.yoffset      = 0;
/*
  // Set global alpha
  ga.enable = 0;
  ga.alpha  = 0;
*/

  // Put var screen info
  ret = ioctl(pHnd->fd, FBIOPUT_VSCREENINFO, &pHnd->var);
  if(ret<0){
    DBG_ERROR("ioctl FBIOPUT_VSCREENINFO\n");
    return -1;
  }
/*
  // Put global alpha
  ret = ioctl(pHnd->fd, MXCFB_SET_GBL_ALPHA, &ga);
  if(ret<0){
    printf("error ioctl MXCFB_SET_GBL_ALPHA\n");
    return -1;
  }
*/
  if(bpp==16){
    /*
    // Set and Put 16 bits color key
    struct mxcfb_color_key color_key;
    color_key.color_key = RGB565TOCOLORKEY(0);
    color_key.enable = 0;
    ret = ioctl(pHnd->fd, MXCFB_SET_CLR_KEY, &color_key);
    if(ret<0){
      printf("Error in setting 16 bits color key\n");
      return -1;
    }
    */
  }else{
    /*
    struct mxcfb_loc_alpha  la;
    // Set local alpha
    la.enable          = 1;
    la.alpha_in_pixel  = 1;
    la.alpha_phy_addr0 = 0;
    la.alpha_phy_addr1 = 0;
    // Put local alpha
    ret = ioctl(pHnd->fd, MXCFB_SET_LOC_ALPHA, &la);
    if(ret<0){
      DBG_ERROR("error ioctl MXCFB_SET_LOC_ALPHA\n");
      return -1;
    }
    */
  }

  // wait for vertical sync
  ioctl(pHnd->fd, MXCFB_WAIT_FOR_VSYNC, 0);

  ret = ioctl(pHnd->fd, FBIOGET_FSCREENINFO, &pHnd->fix);
  if(ret<0){
    DBG_ERROR("get fixed screen info failed: %s\n", strerror(errno));
    return NULL;
  }
  pHnd->size = pHnd->fix.line_length * pHnd->var.yres;
  pHnd->phys = pHnd->fix.smem_start;

  // Set BLANK
  ret = ioctl(pHnd->fd, FBIOBLANK, FB_BLANK_UNBLANK);
  if(ret<0){
    DBG_ERROR("ioctl FB_BLANK_UNBLANK\n");
    return -1;
  }
  return 0;
}

/**
 */
void fbVSyncWait(psFbHnd pHnd)
{
  int ret;

  ret = ioctl(pHnd->fd, MXCFB_WAIT_FOR_VSYNC, 0);
  if(ret<0){
    DBG_ERROR("MXCFB_WAIT_FOR_VSYNC failed: %d %s\n", errno, strerror(errno));
  }
}

/**
 */
int fbPan(psFbHnd pHnd, uint32_t ypan)
{
  int ret;

  pHnd->var.yoffset = ypan;
  ret = ioctl(pHnd->fd, FBIOPAN_DISPLAY, &pHnd->var);
  if(ret<0){
    DBG_ERROR("FBIOPAN_DISPLAY failed: %d %s\n", errno, strerror(errno));
    return -1;
  }
  ret = ioctl(pHnd->fd, MXCFB_WAIT_FOR_VSYNC, 0);
  if(ret<0){
    DBG_ERROR("MXCFB_WAIT_FOR_VSYNC failed: %d %s\n", errno, strerror(errno));
  }
  return 0;
}

/**
 * Set local alpha
 * Put local alpha
 */
int fbGlobalAlpha(psFbHnd pHnd, int ena, int val)
{
  struct mxcfb_gbl_alpha  alpha;
  int                     ret;

  alpha.enable = ena;
  alpha.alpha  = val;
  ret = ioctl(pHnd->fd, MXCFB_SET_GBL_ALPHA, &alpha);
  if(ret<0){
    DBG_ERROR("Set global alpha failed\n");
    return -1;
  }
  DBG_PRINT("global alpha setted to: enable %d alpha %d\n",
          alpha.enable, alpha.alpha);
  return 0;
}
/**
 */
int fbLocalAlpha(psFbHnd pHnd, int ena, int pix)
{
  struct mxcfb_loc_alpha  la;
  int                     ret;

  la.enable          = ena;
  la.alpha_in_pixel  = pix;
  la.alpha_phy_addr0 = 0;
  la.alpha_phy_addr1 = 0;

  ret = ioctl(pHnd->fd, MXCFB_SET_LOC_ALPHA, &la);
  if(ret<0){
    DBG_ERROR("error ioctl MXCFB_SET_LOC_ALPHA\n");
    return -1;
  }
  return 0;
}

#define RGB565TOCOLORKEY(rgb)                              \
      ( ((rgb & 0xf800)<<8)  |  ((rgb & 0xe000)<<3)  |     \
        ((rgb & 0x07e0)<<5)  |  ((rgb & 0x0600)>>1)  |     \
        ((rgb & 0x001f)<<3)  |  ((rgb & 0x001c)>>2)  )

/**
 * Put global alpha
 * Set and Put 16 bits color key
 */
int fbColorKey(psFbHnd pHnd, int ena, uint16_t color)
{
  struct mxcfb_color_key  ck;
/**/
  struct mxcfb_gbl_alpha  ga;
/**/
  int                     ret;
/**/
  ga.enable = 1;
  ga.alpha  = 0xFF;
  ret = ioctl(pHnd->fd, MXCFB_SET_GBL_ALPHA, &ga);
  if(ret<0){
    DBG_ERROR("ioctl MXCFB_SET_GBL_ALPHA\n");
    return -1;
  }
/**/
  ck.color_key = RGB565TOCOLORKEY(color);
  ck.enable    = ena? 1: 0;
  ret = ioctl(pHnd->fd, MXCFB_SET_CLR_KEY, &ck);
  if(ret<0){
    DBG_ERROR("setting 16 bits color key\n");
    return -1;
  }
  return 0;
}

/**
 */
void fbVarShow(psFbHnd pHnd)
{
#if 0
struct fb_fix_screeninfo {
  char id[16];                    /* identification string eg "TT Builtin" */
  unsigned long smem_start;       /* Start of frame buffer mem */
                                  /* (physical address) */
  __u32 smem_len;                 /* Length of frame buffer mem */
  __u32 type;                     /* see FB_TYPE_*                */
  __u32 type_aux;                 /* Interleave for interleaved Planes */
  __u32 visual;                   /* see FB_VISUAL_*              */
  __u16 xpanstep;                 /* zero if no hardware panning  */
  __u16 ypanstep;                 /* zero if no hardware panning  */
  __u16 ywrapstep;                /* zero if no hardware ywrap    */
  __u32 line_length;              /* length of a line in bytes    */
  unsigned long mmio_start;       /* Start of Memory Mapped I/O   */
                                  /* (physical address) */
  __u32 mmio_len;                 /* Length of Memory Mapped I/O  */
  __u32 accel;                    /* Indicate to driver which     */
                                  /*  specific chip/card we have  */
  __u16 capabilities;             /* see FB_CAP_*                 */
  __u16 reserved[2];              /* Reserved for future compatibility */
};
#endif

  printf("fb %s var\n"
        "bits_per_pixel = %d\n"
        "xres           = %d\n"
        "yres           = %d\n"
        "nonstd         = %08X\n"
        "rgba           = %d/%d, %d/%d, %d/%d, %d/%d\n"
        "activate       = %08X\n",
        pHnd->device,
        pHnd->var.bits_per_pixel ,
        pHnd->var.xres           ,
        pHnd->var.yres           ,
        pHnd->var.nonstd         ,
        pHnd->var.red.length     ,
        pHnd->var.red.offset     ,
        pHnd->var.green.length   ,
        pHnd->var.green.offset   ,
        pHnd->var.blue.length    ,
        pHnd->var.blue.offset    ,
        pHnd->var.transp.length  ,
        pHnd->var.transp.offset  ,
        pHnd->var.activate       );

}
