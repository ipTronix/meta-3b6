// file:      can_daemon/can_daemon.h
// project:   vt3
// copyright: TECHNOLEADER S.R.L. all rights reserved

#ifndef __CAN_DAEMON_H
#define __CAN_DAEMON_H

#include <linux/can.h>

// Variable type
typedef unsigned int            word;

typedef struct can_frame canmsg_t;

// Path File & application
#define	Vt3_Path			"/usr/apps/3b6"
#define	FileToRead			"/var/tmp/candaemon-fileread"
#define	FileToWrite			"/var/tmp/candaemon-filewrite"
#define	FileCanConf			"/usr/apps/etc/can%d.conf"

#define false   0
#define true    1

// BusOff
#define BUSOFF_RECOVERY_TIMER (1000) /* BusOff Elapsed clock time top ms */

// Debug mode set 1
#define	Debug				0

// Mask Cob ID
#define	MASK_COBID			0x1B000000

// Time out Multi Packet: 1.5s
#define MULTIPACKETTIMEOUT	1500

// Nack error message
#define	NackMsg		        0xFF

// Pay load
#define	MaxPayLoad	        (4 + 255*7)

/*************************************
*
* Error
*
*/
#define	ERR_MP_SEQ			0x01
#define ERR_MP_TOUT			0x02
#define	ERR_MP_MIX			0x03
#define	ERR_OUT_OF_MEMORY	0x04
#define	ERR_FILE			0x05
#define	ERR_MP_PF			0x18

/*************************************
*
* Default value can
*
*/
#define CAN_BAUD_DEFAULT 		250
#define CAN_SA_SLAVE_DEFAULT	18

/*************************************
*
* Function winload
*
* R_	Request
* A_	Answer
*/

#define	R_FUNC_SHELL			0x16
#define	A_FUNC_SHELL			0x17

#define	R_FUNC_FILE_READ		0x0C
#define	A_FUNC_FILE_READ		0x0D

#define R_FUNC_FILE_WRITE		0x0E
#define A_FUNC_FILE_WRITE		0x0F

#define R_FUNC_LOGIN			0x18
#define A_FUNC_LOGIN			0x19

typedef struct {
	char				rxPacket[MaxPayLoad+1];	// Array for received Multi Packet
	unsigned int		PacketRx;				// Packet number of packet received
	unsigned int		PacketToRx;				// PAcket to received
	unsigned int		ByteToTransfer;			// Byte to transfer
	unsigned int		ByteRx;					// Byte received
	unsigned int		Pf;						// Function active
	struct timespec 	StartTimeOut;			// Start Time Out Time
} StMultiPacketRX;

typedef struct {
	unsigned char Use;      			// Bool flag indicates this Challenge is used
	char Da_master; 	    			// Destination ID
	word Challenge;		    			// Number request of login
	struct timespec  timeout;           // Challenge use time out
} StChallenge;

typedef struct {
	char				TxPacket[MaxPayLoad];	// Array for received Multi Packet
	unsigned int		ByteToTransfer;			// Byte to transfer
} StMultiPacketTX;



word 	CRC16(char *data, word len);
void 	ClearCanbuffer(void);
int		set_bitrate(int fd,	int baud);
void 	FileRead(unsigned char CanN, canmsg_t *rx, char Da);
int 	MultiPacketRx(unsigned char CanN, unsigned int Pf, canmsg_t *rx, char Da);
void 	MultiPacketTx(unsigned char CanN, unsigned int Pf, char Da, StMultiPacketTX Txp);
void 	OpenCan(void);
void 	CobID_DeCod(unsigned char CanN, canmsg_t *rx);
void 	Can_daemon_main(void);
void 	Shell_Command(unsigned char CanN, unsigned char Da);

// end of file

#endif /* __CAN_DAEMON_H */
