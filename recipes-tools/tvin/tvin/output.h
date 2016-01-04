/**
 */
#ifndef __OUTPUT_H
#define __OUTPUT_H

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


int output_open(char* name, psOutDev pOut);
int output_prepare(psOutDev pOut);
int output_setup(psOutDev pOut);
int output_start(psOutDev pOut);
int output_stop(psOutDev pOut);
int output_close(psOutDev pOut);

#endif  /* __OUTPUT_H */
