/**
 */

#include <stdio.h>
#include <unistd.h>

#include "uart.h"

/**
 */
int main(int argc, char** argv)
{
  int   uart;
  char *device = "/dev/ttymxc0";
  //char *device = "/dev/ttyUSB0";
  int   baud = 115200;
int cts;
  printf("uart test "__DATE__" "__TIME__"\n");
  uart = uartOpen(device, baud);
  if(uart<0){
    printf("ERROR: while opening %s\n", device);
    return -1;
  }
  
  for(;;){
    printf("RTS LOW\n");
    uartRTS(uart, 0);
    printf("CTS is %s\n", uartCTS(uart)? "HIGH": "LOW");
    
    sleep(1);
    
    printf("RTS HIGH\n");
    uartRTS(uart, 1);
    cts = uartCTS(uart);
    if(cts<0){
      printf("ERROR while reading CTS\n");
    }else{
      printf("CTS is %s\n", cts? "HIGH": "LOW");
    }
    sleep(1);

  }
  
  uartClose(uart);
  return 0;
}