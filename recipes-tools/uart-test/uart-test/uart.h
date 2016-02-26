/**
 *
 */
#ifndef __UART_H
#define __UART_H
  
int uartOpen(char* device, int bps);
int uartClose(int uart);
int uartRTS(int uart, int rts);
int uartCTS(int uart);
int uartLineGet(int uart, char* line, int size);

#endif /* __UART_H */
