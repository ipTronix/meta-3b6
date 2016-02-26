/**
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/serial.h>

int uartOpen(char* device, int bps);
int uartClose(int uart);
int uartLineGet(int uart, char* line, int size);

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
/*
    No parity (8N1):

    options.c_cflag &= ~PARENB
    options.c_cflag &= ~CSTOPB
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;

    Even parity (7E1):

    options.c_cflag |= PARENB
    options.c_cflag &= ~PARODD
    options.c_cflag &= ~CSTOPB
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;

    Odd parity (7O1):

    options.c_cflag |= PARENB
    options.c_cflag |= PARODD
    options.c_cflag &= ~CSTOPB
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS7;

    Space parity is setup the same as no parity (7S1):

    options.c_cflag &= ~PARENB
    options.c_cflag &= ~CSTOPB
    options.c_cflag &= ~CSIZE;
    options.c_cflag |= CS8;
 
 */

/*
     options.c_cflag |= CNEW_RTSCTS;    // Also called CRTSCTS

Similarly, to disable hardware flow control:

    options.c_cflag &= ~CNEW_RTSCTS;

 */
  memset(&tty, 0, sizeof(tty));
  tty.c_cflag     = baud | bit | stop | CLOCAL | CREAD;
  tty.c_iflag     = parity;
  tty.c_oflag     = 0;
  tty.c_lflag     = 0;
  tty.c_cc[VTIME] = 0;
  tty.c_cc[VMIN ] = 1;

  tty.c_cflag &= ~CRTSCTS;
//  tty.c_cflag |= CRTSCTS;

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
int uartRTS(int uart, int rts)
{
  int flags;
  int ret;
 
  ret = ioctl(uart, TIOCMGET, &flags);
  if(ret){
    return -1;
  }
  if(rts){
    flags |=  TIOCM_RTS;
  }else{
    flags &= ~TIOCM_RTS;
  }
  ret = ioctl(uart, TIOCMSET, &flags);
  if(ret){
    return -1;
  }
  return 0;
}

/**
 */
int uartCTS(int uart)
{
  int flags;
  int ret;
  ret = ioctl(uart, TIOCMGET, &flags);
  if(ret){
    return -1;
  }
  return (flags & TIOCM_CTS)? 1: 0;
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
