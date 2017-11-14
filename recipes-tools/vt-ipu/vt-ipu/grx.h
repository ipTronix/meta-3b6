/** ***************************************************************************
 * @file grx.h
 * @brief
 **************************************************************************** */
#ifndef __GRX_H
#define __GRX_H

#include <stdint.h>

/**
 */
typedef struct {
  psFbHnd   fbHnd;
  char      device[32];
  uint32_t  xres;
  uint32_t  yres;
  uint32_t  bpp;
  void     *fbMem;
}sGrxHnd, *psGrxHnd;

/**
 */
psGrxHnd grxOpen(char* device, uint32_t xres, uint32_t yres, uint32_t bpp);
int grxClose(psGrxHnd pHnd);
void grxBar(psGrxHnd pHnd, uint32_t x, uint32_t y, uint32_t w, uint32_t h, uint32_t c);

int grxVideoBg(psGrxHnd pHnd);
int grxVideoBox(psGrxHnd pHnd, uint32_t x, uint32_t y, uint32_t w, uint32_t h);

#endif /* __GRX_H */