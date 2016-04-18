/** ***************************************************************************
 * @file cap.c
 * @brief Video for linux capture handling
 **************************************************************************** */
#ifndef __CAP_H
#define __CAP_H

#include <linux/videodev2.h>

#define CAP_NUM_BUF 3

/**
 */
typedef struct {
  unsigned char  *start;
  size_t          offset;
  unsigned int    length;
}sCapBuf, psCapBuf;

/**
 */
typedef struct {
  char        name[100];
  int         fd;
  int         fmt;
  v4l2_std_id std;

  int         input;
  int         width;
  int         height;
  int         bpl;    //bytes per line
  int         imsize; //image size
  int         nbuf;
  sCapBuf     buf[CAP_NUM_BUF];
}sCapDev, *psCapDev;


psCapDev capOpen(char* name);
int capSetup(psCapDev pCap);
int capStart(psCapDev pCap);
int capStop(psCapDev pCap);
int capInputSet(psCapDev pCap, int input);

v4l2_std_id capStdGet(psCapDev pCap);
int capClose(psCapDev pCap);


struct v4l2_buffer* capBufGet(psCapDev pCap);
int capBufPut(psCapDev pCap, struct v4l2_buffer* pBuf);

#endif /* __CAP_H */
