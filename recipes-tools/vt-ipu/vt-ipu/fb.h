/** ***************************************************************************
 * @file fb.h
 * @brief
 **************************************************************************** */
#ifndef __FB_H
#define __FB_H

#include <stdint.h>
#include <linux/fb.h>

#define FB_MAX_BUFFER 2

typedef struct {
  char                      device[32];
  int                       fd;
  struct fb_fix_screeninfo  fix;
  struct fb_var_screeninfo  var;
  uint32_t                  size;   // size of single frame buffer
  uint32_t                  phys;
}sFbHnd, *psFbHnd;

psFbHnd fbOpen(char* device);
int   fbClose(psFbHnd pHnd);
void* fbMemGet(psFbHnd pHnd);
void  fbClear(psFbHnd pHnd);

int   fbSet(psFbHnd pHnd, uint32_t xres, uint32_t yres, uint32_t bpp, uint32_t nb);
void  fbVSyncWait(psFbHnd pHnd);
int   fbPan(psFbHnd pHnd, uint32_t ypan);
int   fbGlobalAlpha(psFbHnd pHnd, int ena, int val);
int fbLocalAlpha(psFbHnd pHnd, int ena, int pix);

int   fbColorKey(psFbHnd pHnd, int ena, uint16_t color);
void  fbVarShow(psFbHnd pHnd);

#endif  /* __FB_H */
