/**
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>

#include "can.h"

/**
 */
psCanHnd canOpen(char* device, int mode)
{
  psCanHnd  pCan;
  int       ret;
  pCan = (psCanHnd)malloc(sizeof(sCanHnd));
  if(!pCan){
    return NULL;
  }
  memset(pCan, 0, sizeof(sCanHnd));
  if(mode){
    pCan->sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
  }else{
    pCan->sock = socket(PF_CAN, SOCK_DGRAM, CAN_BCM);
  }
  if(pCan->sock<0){
    free(pCan);
    return NULL;
  }
  if(device){
    strcpy(pCan->ifr.ifr_name, device);
    ret = ioctl(pCan->sock, SIOCGIFINDEX, &pCan->ifr);
    if(ret){
      close(pCan->sock);
      free(pCan);
      return NULL;
    }
    pCan->addr.can_ifindex = pCan->ifr.ifr_ifindex;
  }else{
    pCan->addr.can_ifindex = 0; // any can interface
  }
  pCan->addr.can_family  = AF_CAN;
  if(mode){
    ret = bind(pCan->sock, (struct sockaddr *)&pCan->addr, sizeof(pCan->addr));
  }else{
    ret = connect(pCan->sock, (struct sockaddr *)&pCan->addr, sizeof(pCan->addr));
  }
  if(ret){
    close(pCan->sock);
    free(pCan);
    return NULL;
  }
  return pCan;
}


int canRecv(psCanHnd pCan, psCanMsg pMsg)
{
  int                 rb;
  struct sockaddr_can addr;
  socklen_t           len = sizeof(addr);
  struct ifreq        ifr;
  int                 ret;

  rb = recvfrom(pCan->sock, &pMsg->frame, sizeof(struct can_frame),
                0, (struct sockaddr*)&addr, &len);

  if(rb!=sizeof(struct can_frame)){
    return -1;
  }
  // get interface name of the received CAN frame
  ifr.ifr_ifindex = addr.can_ifindex;
  ret = ioctl(pCan->sock, SIOCGIFNAME, &ifr);
  if(ret){
    printf("ERROR interface name\n");
  }else{
    strcpy(pMsg->iffrom, ifr.ifr_name);
    printf("Received a CAN frame from interface %s\n", pMsg->iffrom);
  }

  printf("============================================================\n");
  printf( "Received frame from interface %s\n"
          "can_id  %08X\n"//;  // 32 bit CAN_ID + EFF/RTR/ERR flags
          "can_dlc %d\n"// frame payload length in byte (0 .. 8)
          "__pad   %02X\n"//;   // padding
          "__res0  %02X\n"//;  // reserved / padding
          "__res1  %02X\n"//;  // reserved / padding
          "data %02X %02X %02X %02X %02X %02X %02X %02X\n",
          //[8] __attribute__((aligned(8)));
          pMsg->iffrom,
          pMsg->frame.can_id,
          pMsg->frame.can_dlc,
          pMsg->frame.__pad,
          pMsg->frame.__res0,
          pMsg->frame.__res1,
          pMsg->frame.data[0], pMsg->frame.data[1],
          pMsg->frame.data[2], pMsg->frame.data[3],
          pMsg->frame.data[4], pMsg->frame.data[5],
          pMsg->frame.data[6], pMsg->frame.data[7]);
  return 0;
}

int canRead(psCanHnd pCan, psCanMsg pMsg)
{
  fd_set          rdfs;
  struct timeval  timeo;
  size_t          rb;
  int             ret;

  FD_ZERO(&rdfs);
//  FD_SET(0, &rdfs);
  FD_SET(pCan->sock, &rdfs);

  timeo.tv_sec  = 10;
  timeo.tv_usec = 0;//10000 * loop;

  ret = select(pCan->sock+1, &rdfs, NULL, NULL, &timeo);
  if(ret<=0){
    //perror("select");
    //running = 0;
    //continue;
    return -1;
  }

  //gettimeofday(&tv, NULL);
  //currcms = (tv.tv_sec - start_tv.tv_sec) * 100 + (tv.tv_usec / 10000);

  //if (FD_ISSET(0, &rdfs))
  //  running &= handle_keyb(s);

  if(FD_ISSET(pCan->sock, &rdfs)){
    rb = read(pCan->sock, pMsg, sizeof(sCanMsg));
    if(rb<0){
      return -1;
    }
    if(rb!=sizeof(sCanMsg)){
      return -1;
    }
    return 0;
  }
  return -1;
}
/*
    struct sockaddr_can addr;
    struct ifreq ifr;
    socklen_t len = sizeof(addr);
    struct can_frame frame;

    nbytes = recvfrom(s, &frame, sizeof(struct can_frame),
                      0, (struct sockaddr*)&addr, &len);

    // get interface name of the received CAN frame 
    ifr.ifr_ifindex = addr.can_ifindex;
    ioctl(s, SIOCGIFNAME, &ifr);
    printf("Received a CAN frame from interface %s", ifr.ifr_name);

  To write CAN frames on sockets bound to 'any' CAN interface the
  outgoing interface has to be defined certainly.

    strcpy(ifr.ifr_name, "can0");
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_family  = AF_CAN;

    nbytes = sendto(s, &frame, sizeof(struct can_frame),
                    0, (struct sockaddr*)&addr, sizeof(addr));


 struct can_frame {
            canid_t can_id;  // 32 bit CAN_ID + EFF/RTR/ERR flags
            __u8    can_dlc; // frame payload length in byte (0 .. 8)
            __u8    __pad;   // padding
            __u8    __res0;  // reserved / padding
            __u8    __res1;  // reserved / padding
            __u8    data[8] __attribute__((aligned(8)));
    };

    struct sockaddr_can addr;
    struct              ifreq ifr;
    socklen_t           len = sizeof(addr);
    struct can_frame    frame;

    nbytes = recvfrom(s, &frame, sizeof(struct can_frame),
                      0, (struct sockaddr*)&addr, &len);

    // get interface name of the received CAN frame
    ifr.ifr_ifindex = addr.can_ifindex;
    ioctl(s, SIOCGIFNAME, &ifr);
    printf("Received a CAN frame from interface %s", ifr.ifr_name);

  //To write CAN frames on sockets bound to 'any' CAN interface the
  //outgoing interface has to be defined certainly.

    strcpy(ifr.ifr_name, "can0");
    ioctl(s, SIOCGIFINDEX, &ifr);
    addr.can_ifindex = ifr.ifr_ifindex;
    addr.can_family  = AF_CAN;

    nbytes = sendto(s, &frame, sizeof(struct can_frame),
                    0, (struct sockaddr*)&addr, sizeof(addr));

pCan->sock 
*/