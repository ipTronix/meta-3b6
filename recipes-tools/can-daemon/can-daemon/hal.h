// file:      can_daemon/hal.h
// project:   vt3
// copyright: TECHNOLEADER S.R.L. all rights reserved
// Hardware Abstraction Layer

//#ifndef CAN_DAEMON_HAL
//#define CAN_DAEMON_HAL
#ifndef __HAL_H
#define __HAL_H

#include "linux/can.h"

typedef struct can_frame canmsg_t;

void hal_switch_on_canbus(void);
void hal_bringup_canbus_interface(int port, int bitrate);
void hal_teardown_canbus_interface(int port);
void hal_reset_canbus_interface(int port);
void hal_send_can(int soc, canmsg_t const * can_msg);


#endif // CAN_DAEMON_HAL