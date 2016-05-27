/** ***************************************************************************
 * @file out.h
 * @brief Video for linux output handling
 **************************************************************************** */
#ifndef __OUTPUT_H
#define __OUTPUT_H

#include <linux/videodev2.h>

#define OUT_NUM_BUF 4

/**
 */
typedef struct {
  unsigned char  *start;
  size_t          offset;
  unsigned int    length;
}sOutBuf, psOutBuf;

/**
 */
typedef struct {
  char        name[100];
  int         fd;
  int         fmt;
  v4l2_std_id std;

  int         width;
  int         height;
  int         disp_top;
  int         disp_left;
  int         disp_width;
  int         disp_height;

  int         rotate;
  int         vflip;
  int         hflip;
  int         tb;

//  int         bpl;    //bytes per line
  int         imsize; //image size
  int         nbuf;
  sOutBuf     buf[OUT_NUM_BUF];
}sOutDev, *psOutDev;

psOutDev outOpen(char* name);
int outSetup  (psOutDev pOut);
int outPrepare(psOutDev pOut);
int outStart  (psOutDev pOut);
int outStop   (psOutDev pOut);
int outClose  (psOutDev pOut);

struct v4l2_buffer* outBufGet(psOutDev pOut);
int outBufPut(psOutDev pOut, struct v4l2_buffer* pBuf);


#endif  /* __OUTPUT_H */
