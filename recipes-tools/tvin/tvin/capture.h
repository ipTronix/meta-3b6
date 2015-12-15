/**
 */
#ifndef __CAPTURE_H
#define __CAPTURE_H

#define CAP_NUM_BUF 5

/**
 */
typedef struct {
  unsigned char  *start;
  size_t          offset;
  unsigned int    length;
}sBuf, psBuf;

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
  sBuf        buf[CAP_NUM_BUF];
}sCapDev, *psCapDev;

int capture_open(char* name, psCapDev pCap);
int capture_setup(psCapDev pCap);
int capture_start(psCapDev pCap);
int capture_stop(psCapDev pCap);
v4l2_std_id capture_std_get(psCapDev pCap);
int capture_close(psCapDev pCap);

#endif /* __CAPTURE_H */
