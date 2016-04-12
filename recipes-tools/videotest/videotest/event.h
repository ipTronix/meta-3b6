/** ***************************************************************************
 * @file event.h
 * @brief
 **************************************************************************** */
#ifndef __EVENT_H
#define __EVENT_H


#include <linux/input.h>


#define BITS_PER_LONG (sizeof(long) * 8)
#define NBITS(x) ((((x)-1)/BITS_PER_LONG)+1)
#define OFF(x)  ((x)%BITS_PER_LONG)
#define BIT(x)  (1UL<<OFF(x))
#define LONG(x) ((x)/BITS_PER_LONG)
#define test_bit(bit, array)  ((array[LONG(bit)] >> OFF(bit)) & 1)

/**
 */
typedef struct {
  int       fd;
  int       version;
  uint16_t  id[4];
  uint32_t  bit[EV_MAX][NBITS(KEY_MAX)];
  char      name[256];
  int       abs[5];
}sEvHnd, *psEvHnd;

/**
 */
typedef struct {
 struct timeval time;
 uint16_t       type;
 uint16_t       code;
 int32_t        value;
}sEvent, *psEvent;

/**
 */
psEvHnd evOpen(char* name);
int evClose(psEvHnd pHnd);
psEvent evGet(psEvHnd pHnd);


#endif /* __EVENT_H */
