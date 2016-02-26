/**
 */
#ifndef __GPIO_H
#define __GPIO_H

/**
 */
typedef struct {
  int   gpio;
  int   fd;
  char  path[32];
  int   dir;
}sGpioHnd, *psGpioHnd;

psGpioHnd gpioInit(int gpio, int dir, int val);
int gpioClose(psGpioHnd pHnd);
int gpioSet(psGpioHnd pHnd, int val);
int gpioGet(psGpioHnd pHnd);

#endif /* __GPIO_H */
