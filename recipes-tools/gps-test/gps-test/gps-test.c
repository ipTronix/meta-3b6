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

  
int uartOpen(char* device);
int uartClose(int uart);
int lpccomReset(int uartfd);

/**
 */
int main(int argc, char** argv)
{
  int uart;
  char buffer[256];
  int rb;
  uart = uartOpen("/dev/ttymxc3");
  if(uart<0){
    printf("ERROR while open /dev/ttymxc3\n");
    return -1;
  }
  for(;;){
    rb = read(uart, buffer, sizeof(buffer));
    if(rb>0){
      buffer[rb] = 0;
      printf("%s", buffer);
    }
  }

  uartClose(uart);
  return 0;
}


/**
 */
int uartOpen(char* device)
{
  int             fd;
  struct termios  tty;
  int             baud;
  int             parity;
  int             bit;
  int             stop;
  int             ret;

  baud   = B9600;
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
