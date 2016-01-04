// file:      can_daemon/can_shm.h
// project:   vt3
// copyright: TECHNOLEADER S.R.L. all rights reserved

// be aware that changes to this file must be reflected in the can_shm.h for ALL Linux-based targets!

#ifndef INCLUDE_CAN_SHM
#define INCLUDE_CAN_SHM

// CAN-bus present in device
#define	N_CAN		(2)

/* max number of customer-id stored in the vt3 project */
#define NLicense    (4)

/* max nodes to can net */
#define NodesCanNet (256)

#define SizeBitFieldLogged (NodesCanNet / 8)

typedef struct _vt3_shm_can
{
	// data written by can_daemon and read by vt3_app
    unsigned char	BitFieldLogged[SizeBitFieldLogged];	// Bit Field flag, indicates that a login slot is active
    unsigned char	SA_Slave[N_CAN];		// source address of device on the different interfaces
	unsigned int    bitrate_Slave[N_CAN];	// Can bitrate
	// data written by vt3_app and read by can_daemon
	unsigned int    wdt_vt3_app_count;	// incremented by vt3_app every 50 milliseconds (watchdog)
	word			License[NLicense];	// Licensed customer id
	unsigned char	Level[NLicense];	// Level of access
	/*
	 * This is the new area
	 */
	unsigned int	version;
#ifndef USE_COUNT
	// data for command management
	unsigned char	command;
	unsigned char	command_ack;
	unsigned char	dummy[194]; /* the new area is 200 bytes */
#else
	unsigned char	dummy[196];
#endif
	/*
	 * Add another area to store can_daemon version (increment shared memory version)
	 */
	unsigned int	can_daemon_version;

} vt3_shm_can;

// Share memory Key
#define SHMEM_KEY			((key_t)  7890)

#ifdef USE_COUNT
/*
 *  These are new definition to manage the wdt_vt3_app_count field
 */
#define VT3_APP_COUNT_MASK		0x0000ffff
#define VT3_APP_COUNT_GET(x)	(((x)->wdt_vt3_app_count) & VT3_APP_COUNT_MASK)
#define VT3_APP_COUNT_INC(x)	((x)->wdt_vt3_app_count) = ((((x)->wdt_vt3_app_count) & ~VT3_APP_COUNT_MASK) | ((((x)->wdt_vt3_app_count) + 1) & VT3_APP_COUNT_MASK));
// vt3 command requests
#define VT3_CMD_NO_REQ			0x00000000
#define VT3_CMD_BAD				0x00010000
#define VT3_CMD_UPDATE			0x00100000
#define VT3_CMD_END_UPDATE		0x00200000
#define VT3_CMD_MASK			0x00ff0000
#define VT3_CMD_SET(x, cmd)		((x)->wdt_vt3_app_count) |= cmd;
#define VT3_CMD_RESET(x)		((x)->wdt_vt3_app_count) &= ~VT3_CMD_MASK;
#define VT3_CMD_GET(x)			(((x)->wdt_vt3_app_count) & VT3_CMD_MASK)
#define VT3_CMD_IS_BAD(x)		((((x)->wdt_vt3_app_count) & VT3_CMD_BAD) == VT3_CMD_BAD)
#define VT3_CMD_IS_BAD_RST(x)   (VT3_CMD_GET(x) == VT3_CMD_NO_REQ)
// vt3 command acknowledge
#define VT3_CMD_ACK_EMPTY		0x00000000
#define VT3_CMD_ACK_OK			0x01000000
#define VT3_CMD_ACK_NACK		0x02000000
#define VT3_CMD_ACK_MASK		0xff000000
#define VT3_CMD_ACK_SET(x, ack)	((x)->wdt_vt3_app_count) |= ack;
#define VT3_CMD_ACK_GET(x)		(((x)->wdt_vt3_app_count) & VT3_CMD_ACK_MASK)
#define VT3_CMD_ACK_RST(x)		((x)->wdt_vt3_app_count) &= ~VT3_CMD_ACK_MASK;
#else
#define VT3_VERSION_MAJOR(x)	(((x)->version & 0xff000000) >> 24)
#define VT3_APP_COUNT_GET(x)	((x)->wdt_vt3_app_count)
#define VT3_APP_COUNT_INC(x)	(x)->wdt_vt3_app_count = (x)->wdt_vt3_app_count + 1;
// vt3 command requests
#define VT3_CMD_NO_REQ			0x00
#define VT3_CMD_UPDATE			0x01
#define VT3_CMD_END_UPDATE		0x02
#define VT3_CMD_SET(x, cmd)		(x)->command = cmd;
#define VT3_CMD_RESET(x)		(x)->command = VT3_CMD_NO_REQ;
#define VT3_CMD_GET(x)			((x)->command)
// vt3 command acknowledge
#define VT3_CMD_ACK_EMPTY		0x00
#define VT3_CMD_ACK_OK			0x01
#define VT3_CMD_ACK_NACK		0x02
#define VT3_CMD_ACK_SET(x, ack)	(x)->command_ack = ack;
#define VT3_CMD_ACK_GET(x)		((x)->command_ack)
#define VT3_CMD_ACK_RST(x)		(x)->command_ack = VT3_CMD_ACK_EMPTY;
#endif

#endif

// end of file
