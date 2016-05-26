/**
 * LPC Com
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
int lpcCmdSend(int uartfd, char* cmd);

/**
 */
int main(int argc, char** argv)
{
  int  lpc_uart;
  char cmd[32];
  char rxbuff[64];
  char c;
  int  rxidx = 0;

  printf("lpc-com Ver:0.0 "__DATE__" "__TIME__"\n");
  cmd[0] = 0;
  if(argc>1){
    int i;
    int l;
    l = sprintf(cmd, "%s", argv[1]);
    for(i=2; i<argc; i++){
      l += sprintf(cmd+l, " %s", argv[i]);
    }
  }
  lpc_uart = uartOpen("/dev/ttymxc4");
  if(lpc_uart<0){
    return -1;
  }
  if(cmd[0]){
    printf("Send command [%%%s$] to LPC\n", cmd);
    lpcCmdSend(lpc_uart, cmd);
  }
/*
  do{
    if(read(lpc_uart, &c, 1)>0){
      if(c == '%'){
        rxidx = 0;
      }else if(c != '$'){
        rxbuff[rxidx] = c;
        if (rxidx<63)
          rxidx++;
      }else{
        rxbuff[rxidx] = 0;
        printf("%s\n", rxbuff);
      }
    }
  }while(1);
*/
  uartClose(lpc_uart);
  return 0;
}

/**
 */
int lpcCmdSend(int uartfd, char* cmd)
{
  int wb;
  int lc;

  lc = strlen(cmd);

  wb = write(uartfd, "%", 1);
  if(wb!=1){ return -1; }

  wb = write(uartfd, cmd, lc);
  if(wb!=lc){ return -1; }

  wb = write(uartfd, "$", 1);
  if(wb!=1){ return -1; }

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

  baud   = B115200;
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
