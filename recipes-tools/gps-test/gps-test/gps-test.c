/**
 * GPS Test
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

#include "gpio.h"


int uartOpen(char* device, int bps);
int uartClose(int uart);
int uartLineGet(int uart, char* line, int size);

int nmeaCmdSend(int uart, char* cmd);

/**
 */
int main(int argc, char** argv)
{
  char     *ttydev = "/dev/ttymxc3";
  int       uart;
  char      buffer[256];
  int       rb;
  int       i;
  psGpioHnd gpio_GPS_PWR_EN;
  psGpioHnd gpio_GPS_RESET;
  int       bps;
  char      nmeacmd[80];

  printf("gps test version 1.0 "__DATE__" "__TIME__"\n");

  // init gpio pins
  gpio_GPS_PWR_EN = gpioInit(134, 0, 1);
  if(!gpio_GPS_PWR_EN){
    printf("ERROR while set gpio_GPS_PWR_EN\n");
    //return -1;
  }
  gpio_GPS_RESET  = gpioInit(37, 0, 1);
  if(!gpio_GPS_RESET){
    printf("ERROR while set gpio_GPS_RESET\n");
    //return -1;
  }

  // reset GPS module
  gpioSet(gpio_GPS_RESET , 0);
  gpioSet(gpio_GPS_PWR_EN, 0);
  sleep(1);
  gpioSet(gpio_GPS_PWR_EN, 1);
  gpioSet(gpio_GPS_RESET , 1);
  sleep(2);

  // open UART
  bps = 9600;
  printf("Open uart %s @ %d bps\n", ttydev, bps);
  uart = uartOpen(ttydev, bps);
  if(uart<0){
    printf("ERROR while open %s\n", ttydev);
    return -1;
  }

  // send change baud rate command
  bps = 115200;
  sprintf(nmeacmd, "PMTK251,%d", bps);
  nmeaCmdSend(uart, nmeacmd);

  // close and reopen uart
  printf("Close uart %s and reopen @ %d bps\n", ttydev, bps);
  uartClose(uart);
  uart = uartOpen(ttydev, bps);
  if(uart<0){
    printf("ERROR while open %s @ %d bps\n", ttydev, bps);
    return -1;
  }

  // get data from GPS module
  for(;;){
    rb = uartLineGet(uart, buffer, sizeof(buffer));
    if(rb>0){
      printf("%s\n", buffer);
    }
  }

  uartClose(uart);
  gpioClose(gpio_GPS_PWR_EN);
  gpioClose(gpio_GPS_RESET );
  return 0;
}

#define NMEA_MSG_SIZE 90
/**
 */
int nmeaCmdSend(int uart, char* cmd)
{
  int   i;
  unsigned char chk;
  char  msg[NMEA_MSG_SIZE];
  int   len;
  int   wb;

  chk = 0;
  for(i=0; cmd[i]; i++){
    chk ^= cmd[i];
  }
  len = sprintf(msg, "$%s*%02X\r\n", cmd, chk);
  printf("nmeaCmdSend: %slen = %d\n", msg, len);
  wb = write(uart, msg, len);
  if(wb!=len){
    printf("nmeaCmdSend:ERROR\n");
    return -1;
  }
  return 0;
}

/**
 */
int uartOpen(char* device, int bps)
{
  int             fd;
  struct termios  tty;
  int             baud;
  int             parity;
  int             bit;
  int             stop;
  int             ret;

  switch(bps){
  case   9600: baud =   B9600; break;
  case  19200: baud =  B19200; break;
  case  38400: baud =  B38400; break;
  case 115200: baud = B115200; break;
  default    : baud =   B9600; break;
  }
  parity = IGNPAR;
  bit    = CS8;
  stop   = 0;

  memset(&tty, 0, sizeof(tty));
  tty.c_cflag     = baud | bit | stop | CLOCAL | CREAD;
  tty.c_iflag     = parity;
  tty.c_oflag     = 0;
  tty.c_lflag     = 0;
  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN ] = 1;

  tty.c_cflag &= ~CRTSCTS;

  tty.c_iflag &= ~(IXON | IXOFF | IXANY);

  fd = open(device, O_RDWR|O_NOCTTY);
  if(fd==-1){
    return -1;
  }

  ret = tcflush(fd, TCIFLUSH);
  if(ret){
    close(fd);
    return -1;
  }

  ret = tcsetattr(fd, TCSANOW, &tty);
  if(ret){
    close(fd);
    return -1;
  }
  ret = tcsetattr(fd, TCSAFLUSH, &tty);
  if(ret){
    close(fd);
    return -1;
  }
  return fd;
}

/**
 */
int uartClose(int uart)
{
  return close(uart);
}

/**
 */
int uartLineGet(int uart, char* line, int size)
{
  int   len = 0;
  int   rb;
  char  c;

  for(;;){
    rb = read(uart, &c, 1);
    if(rb>0){
      if(c>=0x20 && c<=0x7F){
        line[len] = c;
        len++;
        if(len>size){
          return -1;
        }
      }else{
        if( (c==0x0A || c==0x0D) && len){
          line[len] = 0;
          len++;
          return len;
        }
      }
    }
  }
  return len;
}
