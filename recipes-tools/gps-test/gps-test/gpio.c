/**
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>

#include "gpio.h"

/**
 */
psGpioHnd gpioInit(int gpio, int dir, int val)
{
  psGpioHnd pHnd;
  char  str[8];
  int   wb;
  int   len;

  pHnd = (psGpioHnd)malloc(sizeof(sGpioHnd));
  if(!pHnd){
    return NULL;
  }
  pHnd->gpio = gpio;
  pHnd->fd = open("/sys/class/gpio/export", O_WRONLY);
  if(pHnd->fd<0){
    return NULL;
  }
  len = sprintf(str, "%d", gpio);
  wb = write(pHnd->fd, str, len);
  if(wb!=len){
    //close(pHnd->fd);
    //return NULL;
  }
  close(pHnd->fd);
  
  sprintf(pHnd->path, "/sys/class/gpio/gpio%d/direction", gpio);
  pHnd->fd = open(pHnd->path, O_WRONLY);
  if(pHnd->fd<0){
    // TODO UNEXPORT
    return NULL;
  }
  len = sprintf(str, "%s", (dir==1)? "in": "out");
  wb = write(pHnd->fd, str, len);
  if(wb!=len){
    // TODO UNEXPORT
    close(pHnd->fd);
    return NULL;
  }
  close(pHnd->fd);
  
  sprintf(pHnd->path, "/sys/class/gpio/gpio%d/value", gpio);
  pHnd->fd = open(pHnd->path, O_WRONLY);
  if(pHnd->fd<0){
    // TODO UNEXPORT
    return NULL;
  }
  len = sprintf(str, "%d", val? 1: 0);
  wb = write(pHnd->fd, str, len);
  if(wb!=len){
    // TODO UNEXPORT
    close(pHnd->fd);
    return NULL;
  }
//  close(pHnd->fd);
  /*
  /sys/class/gpio/unexport
echo 37 > /sys/class/gpio/export
echo out > /sys/class/gpio/gpio37/direction
echo 1 > /sys/class/gpio/gpio37/value
*/
  return pHnd;
}

/**
 */
int gpioClose(psGpioHnd pHnd)
{
  char  str[8];
  int   wb;
  int   len;

  close(pHnd->fd);
  
  pHnd->fd = open("/sys/class/gpio/unexport", O_WRONLY);
  if(pHnd->fd<0){
    return -1;
  }
  len = sprintf(str, "%d", pHnd->gpio);
  wb = write(pHnd->fd, str, len);
  close(pHnd->fd);
  if(wb!=len){
    return -1;
  }
  return 0;
}

/**
 */
int gpioSet(psGpioHnd pHnd, int val)
{
  int wb;
  if(val){
    wb = write(pHnd->fd, "1", 1);
  }else{
    wb = write(pHnd->fd, "0", 1);
  }
  return (wb!=1);
}

/**
 */
int gpioGet(psGpioHnd pHnd)
{
  return 0;
}
