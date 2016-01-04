#ifndef __CAN_H
#define __CAN_H

#include <sys/time.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/can.h>
#include <linux/can/bcm.h>

/**
 */
typedef struct {
  int                   sock;
  struct sockaddr_can   addr;
  struct ifreq          ifr;
}sCanHnd, *psCanHnd;

/**
 */
typedef struct {
  struct bcm_msg_head   msg_head;
  struct can_frame      frame;
  char                  iffrom[16];
}sCanMsg, *psCanMsg;


/**
 */
psCanHnd canOpen(char* device, int mode);
int canRead(psCanHnd pCan, psCanMsg pMsg);


#endif  /* __CAN_H */