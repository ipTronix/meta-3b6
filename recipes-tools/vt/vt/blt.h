/** ***************************************************************************
 * @file blt.h
 * @brief
 **************************************************************************** */
#ifndef __BLT_H
#define __BLT_H

#include <stdint.h>

#define BLT_CAP_

#define BLT_CAP_BLEND       0x00000001  // Enable alpha blend in 2D BLT.
#define BLT_CAP_DITHER      0x00000002  // Enable dither in 2D BLT.
#define BLT_CAP_GL_ALPHA    0x00000004  // Enable global alpha in blend.

/**
 */
typedef struct {
  void     *g2dHnd;
  uint32_t  src_fmt;
  uint32_t  src_xres;
  uint32_t  src_yres;
  uint32_t  dst_fmt;
  uint32_t  dst_xres;
  uint32_t  dst_yres;
  uint32_t  blend;
  uint32_t  cap;
}sBltHnd, *psBltHnd;

psBltHnd bltOpen(int bpp);
void bltClose(psBltHnd pHnd);
int bltCopy(psBltHnd pHnd, int src_phys, int dst_phys,
            uint32_t dst_x, uint32_t dst_y, uint32_t dst_w, uint32_t dst_h);
int bltUpdate(psBltHnd pHnd);

#endif /* __BLT_H */
