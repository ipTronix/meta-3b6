// file:      can_daemon/hal.c
// project:   vt3
// copyright: TECHNOLEADER S.R.L. all rights reserved
// Hardware Abstraction Layer

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>

#include "hal.h"

static void do_cmd(char const * cmd)
{
	printf("command: %s\n", cmd);
	int err = system(cmd);
	printf("returned %d\n", err);
}


void hal_switch_on_canbus(void)
{
	char const * commands[] =
	{
		"echo 40 > /sys/class/gpio/export",
		"echo out > /sys/class/gpio/gpio40/direction",
		"echo 0 > /sys/class/gpio/gpio40/value",
		NULL
	};
	int i;
	for ( i = 0; commands[i]; i ++ )
	{
		do_cmd(commands[i]);
	}
}


void hal_bringup_canbus_interface(int port, int bitrate)
{
	char cmd[128];
	snprintf(cmd, sizeof cmd, "ip link set can%d up type can bitrate %d", port, bitrate);
	do_cmd(cmd);
}


void hal_teardown_canbus_interface(int port)
{
	char cmd[128];
	snprintf(cmd, sizeof cmd, "ip link set can%d down", port);
	do_cmd(cmd);
}


void hal_reset_canbus_interface(int port)
{
	char cmd[128];
	snprintf(cmd, sizeof cmd, "ip link set can%d type can restart", port);
	do_cmd(cmd);
}


void hal_send_can(int soc, const canmsg_t * can_msg)
{
  int i;
	printf("tx %08X : ", can_msg->can_id);
    for(i=0; i<can_msg->can_dlc; i++ )
    	printf("%02X ", can_msg->data[i]);
    
    int count = 0;
    while ( 1 ) 
    {
		//@todo usare select
		int err = write(soc, can_msg, sizeof(canmsg_t));
		if ( err == sizeof(canmsg_t) ) 
		{
		    printf(" --> OK\n");
			return;
		}
		printf(".");
		usleep(100);
		count ++;
		if ( count == 1000 )
		{
			printf(" --> error: write to canbus socket %d failed\n", soc);
			return;
		}
    }
}

