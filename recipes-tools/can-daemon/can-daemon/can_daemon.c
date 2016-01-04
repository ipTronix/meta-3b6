// file:      can_daemon/can_daemon.c
// project:   vt3
// copyright: TECHNOLEADER S.R.L. all rights reserved

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/mount.h>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <linux/can/error.h>
#include <net/if.h>

#include "hal.h"
#include "can_daemon.h"
#include "can_shm.h"
#include "version_can_daemon.h"

#include "run_eoloader.h"


/* struct: version_only_for_check_by_script
 * used to check lib version from executable file
 */
typedef struct version_only_for_check_by_script_t
{
   char    		lib_marker_start[128];
   unsigned int version;
   char    		lib_marker_end[128];
} version_only_for_check_by_script_t;

const version_only_for_check_by_script_t version_markers_and_number =
{
    "3B6 library version marker start - 13579246809753108642 - 122014 - 789456123074185296300147258369 - 3B6 library version marker ",
    VERSION_CAN_DAEMON,
    "3B6 library version marker end - 13579246809753108642 - 122014 - 789456123074185296300147258369 - 3B6 library version marker   ",
};


/* internal watchdog */
//@todo ripristinare
#define WDGN
int					wdg_fd = 0;				// file descriptor for watchdog
#ifdef WDG
/* taken from watchdog.h */
#define	WATCHDOG_IOCTL_BASE	'W'
#define	WDIOC_KEEPALIVE		_IOR(WATCHDOG_IOCTL_BASE,   5, int)
#define	WDIOC_SETTIMEOUT    _IOWR(WATCHDOG_IOCTL_BASE,  6, int)
#define	WDIOC_GETTIMEOUT    _IOR(WATCHDOG_IOCTL_BASE,   7, int)
#define	WDIOC_GETTIMELEFT	_IOR(WATCHDOG_IOCTL_BASE,  10, int)
//--GC
#ifdef BOARD_IMX356_SMALL
#define WD_DEV	"/dev/ext_watchdog"
#else
#define WD_DEV	"/dev/watchdog"
#endif // BOARD_IMX356_SMALL
#define WD_DEV_OLD	"/dev/misc/watchdog"
#define WD_TIMEOUT_VALUE 60
#endif

// Max number Challenge in memory
#define NCHALLENGE					  (4)

static int 					CanSoc[N_CAN];			// file descriptor Can0 & Can1
static unsigned char   		SA_Slave[N_CAN];		// Source address
static unsigned int        	bitrate_Slave[N_CAN];   // Can port bitrate
static StMultiPacketRX		Rx_MPacket[N_CAN];		// Buffer Multi Packet Rx
static StChallenge         	Challenge[NCHALLENGE];  // Challenge array structure, number request for login
static FILE 				* pWritefile = NULL;	// Point to Write File
static vt3_shm_can			* pVt3_shm_can;			// Share pointer structure
static int  				Enable_Sleep  = 1;		// Sleep Enable '1' disable '0'
static struct timespec		Start_Sleep;

// BusOff
static unsigned char		Can_BUSOFF[N_CAN];
static struct timespec		busoff_recovery_timer[N_CAN];

// vt3_app watchdog timeout
#define VT3_APP_WATCHDOG_DISABLED     (0)	// watchdog disable
#define VT3_APP_WATCHDOG_ENABLE       (1)   // watchdog enable
#define VT3_APP_WATCHDOG_INIT_LONG    (2)   // watchdog initialized long
#define VT3_APP_WATCHDOG_INIT_SHORT   (3)   // watchdog initialized short

// Time out short watchdog: 5s
#define VT3_APP_WATCHDOG_RELOAD_SHORT (5000)
// Time out long watchdog: 9s
#define VT3_APP_WATCHDOG_RELOAD_LONG  (9000)
// Time out: if vt3_app dead for long time, then allow all customers to login
#define VT3_APP_TIMEOUT_LICENSE  	  (60000)
// Challenge not use Time out
#define VT3_APP_CHALLENGE_TIMEOUT     (1000)


static void send_nack(int CanN, char Da_master, char Pf, char Err, char db2, char db3);
static void transmit_multipacket(unsigned char CanN, unsigned int Pf, char Da_master, const StMultiPacketTX * Txp);
static void run_shell_command(unsigned char CanN, unsigned char Da_master);


static char             vt3_app_watchdog_state = 0; // Sleep watchdog '1' disable '0'
static struct timespec  vt3_app_watchdog_timeout;
static struct timespec  vt3_app_license_timeout;

#define VT3_APP_CMD_STATUS_IDLE			0
#define VT3_APP_CMD_STATUS_UPDATE		1
static char      vt3_app_command_state = VT3_APP_CMD_STATUS_IDLE;

#define PATH_USB				"/mnt/floppy/"
#define PATH_UPDATE				PATH_USB"vt3_update/vt3_app"
#define SHELL_CMD_MOUNT			"mount -t vfat /dev/sda1 "PATH_USB
#define SHELL_CMD_VT3_APP		"cd /usr/apps/3b6; ./vt3_app &"
#define SHELL_CMD_KILL_VT3_APP	"killall vt3_app"
#define SHELL_CMD_KILL_9_VT3_APP	"killall -9 vt3_app"
static const unsigned short CRC16_table[256] = {
  0,  2061,  4122,  6167,  8244,  10297,  12334,  14371,  16488,  18533,  20594,  22655,  24668,
  26705,  28742,  30795,  32976,  35037,  37066,  39111,  41188,  43241,  45310,  47347,  49336,
  51381,  53410,  55471,  57484,  59521,  61590,  63643,  2477,  416,  6583,  4538,  10649,  8596,
  14723,  12686,  18885,  16840,  23007,  20946,  27121,  25084,  31211,  29158,  35197,  33136,
  39271,  37226,  43337,  41284,  47443,  45406,  51477,  49432,  55567,  53506,  59681,  57644,
  63803,  61750,  4954,  6999,  832,  2893,  13166,  15203,  9076,  11129,  21298,  23359,  17192,
  19237,  29446,  31499,  25372,  27409,  37770,  39815,  33680,  35741,  46014,  48051,  41892,
  43945,  54242,  56303,  50168,  52213,  62422,  64475,  58316,  60353,  6903,  4858,  2797,  736,
  15043,  13006,  10969,  8916,  23199,  21138,  19077,  17032,  31403,  29350,  27313,  25276,
  39463,  37418,  35389,  33328,  47635,  45598,  43529,  41476,  55887,  53826,  51797,  49752,
  64123,  62070,  60001,  57964,  9908,  11961,  13998,  16035,  1664,  3725,  5786,  7831,  26332,
  28369,  30406,  32459,  18152,  20197,  22258,  24319,  42596,  44649,  46718,  48755,  34384,
  36445,  38474,  40519,  58892,  60929,  62998,  65051,  50744,  52789,  54818,  56879,  12057,
  10004,  16131,  14094,  3885,  1824,  7991,  5946,  28529,  26492,  32619,  30566,  20293,  18248,
  24415,  22354,  45001,  42948,  49107,  47070,  36861,  34800,  40935,  38890,  61345,  59308,
  65467,  63414,  53141,  51096,  57231,  55170,  13806,  15843,  9716,  11769,  5594,  7639,  1472,
  3533,  30086,  32139,  26012,  28049,  21938,  23999,  17832,  19877,  46398,  48435,  42276,
  44329,  38154,  40199,  34064,  36125,  62806,  64859,  58700,  60737,  54626,  56687,  50552,
  52597,  15427,  13390,  11353,  9300,  7287,  5242,  3181,  1120,  31787,  29734,  27697,  25660,
  23583,  21522,  19461,  17416,  48275,  46238,  44169,  42116,  40103,  38058,  36029,  33968,
  64763,  62710,  60641,  58604,  56527,  54466,  52437,  50392,
  };


// compare timespec numbers
inline static int cmp_ts(struct timespec t1, struct timespec t2)
{
    if ( t1.tv_sec > t2.tv_sec )
        return (+1);
    if ( t1.tv_sec < t2.tv_sec )
        return (-1);
    if ( t1.tv_nsec > t2.tv_nsec )
        return (+1);
    if ( t1.tv_nsec < t2.tv_nsec )
        return (-1);
    return (0);
}

// subtract timespec numbers
inline static struct timespec sub_ts(struct timespec t1, struct timespec t2)
{
    struct timespec result;

    // if t1 > t2
    if ( cmp_ts(t1, t2) > 0 )
    {
        // normal subtract
        if ( t1.tv_nsec >= t2.tv_nsec )
        {
            result.tv_nsec = t1.tv_nsec - t2.tv_nsec;
            result.tv_sec  = t1.tv_sec  - t2.tv_sec;
        }
        // use carry
        else
        {
            result.tv_nsec = t1.tv_nsec - t2.tv_nsec + 1000000000LLU;
            result.tv_sec  = t1.tv_sec  - t2.tv_sec  - 1;
        }
    }
    else
    {
        // if t1 <= t2 always return 0
        result.tv_nsec = 0;
        result.tv_sec  = 0;
    }
    return (result);
}


// period is converted into a timespec number
// In	period in ms
// Out	struct timespec
inline static struct timespec period_ts(int period_ms)
{
	struct timespec period;

    period.tv_sec = period_ms / 1000;
    period.tv_nsec = (period_ms % 1000) * 1000000;

	return (period);
}





/***********************************************************************
*
*	Bus OFF Reset
*
*/
static void reset_busoff(int port_number)
{
	struct timespec  now, sub_time, end_time;

	// busoff_recovery_timer
	clock_gettime(CLOCK_MONOTONIC, &now);
	sub_time = sub_ts(now, busoff_recovery_timer[port_number]);

	/* initialize random generator */
	end_time = period_ts((rand() % 500 + (BUSOFF_RECOVERY_TIMER - 500)));

	if (cmp_ts(sub_time, end_time) > 0)
	{
		printf("BUSOFF on CAN#%d: RESET controller\n", port_number);

		// reload timer
		clock_gettime(CLOCK_MONOTONIC, &busoff_recovery_timer[port_number]);

		// restart CAN controller
		hal_reset_canbus_interface(port_number);

		// clear BUSOFF condition
		Can_BUSOFF[port_number] = false;
	}
}

/***********************************************************************
*
* get_bitfield
*
* 	return bit state
*
*	SA_master	-	Destination address master
*
*	Return:
*			1 : Logged
*		    0 : Not logged
*/
static unsigned char get_bitfield(unsigned char SA_master)
{
	unsigned char i, mod;
	unsigned char status;

	// get byte in array
	i = SA_master / 8;
	// get modulo
	mod = SA_master % 8;

	status = 0x01 & ((pVt3_shm_can->BitFieldLogged[i] >> mod));
	return (status);
}

/***********************************************************************
*
* set_bitfield
*
* 	Set bit state
*
*	SA_master	-	Destination address master
*   Bit			-   True o False
*
*/
static void set_bitfield(unsigned char SA_master, unsigned char Bit)
{
	unsigned char i, mod;
	unsigned char mask;

	// get byte in array
	i = SA_master / 8;
	// get modulo
	mod = SA_master % 8;
	mask = 0x80 >> (7 - mod);

	if (Bit == true)
	{
		// set bit true
		pVt3_shm_can->BitFieldLogged[i] = (pVt3_shm_can->BitFieldLogged[i]  | mask);
	}
	else
	{
		// Set Bit False
		mask = 0xff ^ mask;
		pVt3_shm_can->BitFieldLogged[i] = (pVt3_shm_can->BitFieldLogged[i] & mask);
	}
}


/***********************************************************************
*
* grant_login
*
*	SA_master	-	Destination address master
*
*/
static void grant_login(unsigned char SA_master)
{
	// unsigned char n;
	set_bitfield(SA_master, true);
}

/***********************************************************************
*
* logout
*
*	SA_master	-	Destination address master
*
*/
static void logout(unsigned char SA_master)
{
	set_bitfield(SA_master, false);
}

/***********************************************************************
*
* is_logged_in
*
*	SA_master	-	Destination address master
*
*	Return:
*			1 : is logged
*		    0 : not logged
*/
static char is_logged_in(unsigned char SA_master)
{
	return (get_bitfield(SA_master));
}

/***********************************************************************
*
* set_login_challenge
*
* Set challenge in structure
*
*	Da_master	-	Destination address
*	Chal        -   Challenge to set
*
*	Return:
*			1 : ok store
*		    0 : not store
*/
static char set_login_challenge(unsigned char Da_master, word Chal)
{
	unsigned char n;
	struct timespec  now, sub_time, end_time;

	for ( n = 0; n < NCHALLENGE; ++n )
	{
		if ( Challenge[n].Use == true)
		{
			// elapsed time of this challenge
			clock_gettime(CLOCK_MONOTONIC, &now);
			sub_time = sub_ts(now, Challenge[n].timeout );
			end_time = period_ts(VT3_APP_CHALLENGE_TIMEOUT);

			if (cmp_ts(sub_time, end_time) > 0)
			{
				// elapsed time
				// free slot
				Challenge[n].Use = false;
			}
		}

		if ( Challenge[n].Use == false)
		{
			// use slot
			Challenge[n].Use = true;
			Challenge[n].Da_master = Da_master;
			Challenge[n].Challenge = Chal;
			// load timer
			clock_gettime(CLOCK_MONOTONIC, &Challenge[n].timeout);
			return (1);
		}
	}
	// slot not free
	return (0);
}

/***********************************************************************
*
* find_slot_login_challenge
*
* Get challenge from structure
*
*	Da_master	-	Destination address
*
*	Return:
*			word : return store challenge
*		    0    : not store challenge found
*/
static word find_slot_login_challenge(unsigned char Da_master)
{
	unsigned char n;

	for ( n = 0; n < NCHALLENGE; ++n ) {
		if ((Challenge[n].Use == true) && (Challenge[n].Da_master == Da_master)) {
			// challenge found
			return (Challenge[n].Challenge);
		}
	}

	// challenge not found
	return (0);
}

/***********************************************************************
*
* free_slot_login_challenge
*
* free slot challenge in structure
*
*	Da_master	-	Destination address
*
*/
static void free_slot_login_challenge(unsigned char Da_master)
{
	unsigned char n;

	for ( n = 0; n < NCHALLENGE; ++n ) {
		if ((Challenge[n].Use == true) && (Challenge[n].Da_master == Da_master)) {
			// challenge found
			Challenge[n].Use       = false;
			Challenge[n].Challenge = 0;
		}
	}

}


/***********************************************************************
*
*  CRC16
*
*  		 Data		Pointer char data
*		 Len		Length data
* RETURN:
*		 Crc16 word
*/
static word crc16(unsigned char *data, word len)
{
	word tmp=0x5555;

	int i;
	for (i=0; i<len; i++)
	{
		tmp = ((0x00FF & tmp) << 8)^CRC16_table[((tmp>>8)^(*data++))& 0xFF];
	}
	return (tmp);
}


/***********************************************************************
*
* set_can_baud - sets the CAN bit rate
*
*  CanPortN		Device
*  baud			Bit rate
*
* Changing these registers only possible in Reset mode.
*
* RETURN:
*		0 	-	Set bit rate witch success
*/
static int	set_can_baud(int CanPortN,	int baud)
{
//	Config_par_t  cfg;
//	volatile Command_par_t cmd;
//	int ret;

//	cmd.cmd = CMD_STOP;
//	ret = ioctl(CanSoc[CanPortN], CAN_IOCTL_COMMAND, &cmd);

//	cfg.target = CONF_TIMING;
//	cfg.val1   = baud;
//	ret = ioctl(CanSoc[CanPortN], CAN_IOCTL_CONFIG, &cfg);

//	cmd.cmd = CMD_CLEARBUFFERS;
//	ret = ioctl(CanSoc[CanPortN], CAN_IOCTL_COMMAND, &cmd);

//	cmd.cmd = CMD_START;
//	ret = ioctl(CanSoc[CanPortN], CAN_IOCTL_COMMAND, &cmd);
    return 0;
//	return (ret);
}


//@todo comment
void send_can(int line, canmsg_t const * can_msg)
{
	hal_send_can(CanSoc[line], can_msg);
}


/***********************************************************************
*
* write_file   -   Debug Function file write
*
*	CanN		-	Can port number
*	Da_master	-	Destination address master
*
*/
static void write_file(unsigned char CanN, char Da_master)
{
	canmsg_t 	tx;

	// Response
    tx.can_dlc  = 0;
    tx.can_id   = MASK_COBID | (A_FUNC_FILE_WRITE << 16) | (Da_master << 8) | SA_Slave[CanN];
    tx.can_id   |= CAN_EFF_FLAG;
	// Send the Packet
    send_can(CanN, &tx);


	if (Debug)
		printf("Write file: %s \n", FileToWrite);

	// Open File to append
	if (pWritefile == NULL)
	{
		if((pWritefile = fopen(FileToWrite, "a")) == NULL)
		{
			printf("Sorry, can't open file: %s\n", FileToWrite);
		}
	}

	// Write the PayLoad
	fwrite(Rx_MPacket[CanN].rxPacket, 1, Rx_MPacket[CanN].ByteRx, pWritefile);
}



/***********************************************************************
*
*    -   Debug Function file read
*
*	CanN		-	Can port number
*	rx			-	Pointer to the struct can rx
*	Da_master	-	Destination address
*/
static void read_file(unsigned char CanN, const canmsg_t *rx, char Da_master)
{
	FILE 				*pfile;
	long int			offset;
	long int			BlockRead;
	StMultiPacketTX		MpTx;

	// Open File to read
	if((pfile = fopen(FileToRead, "r")) == NULL){
		printf("Sorry, can't open file: %s", FileToRead);
		send_nack(CanN, Da_master, A_FUNC_FILE_READ, ERR_FILE, 00, 00);
		return;
	}

	offset = rx->data[0] | rx->data[1] << 8 | rx->data[2] << 16 | rx->data[3] << 24;
	BlockRead = rx->data[4] | rx->data[5] << 8;

    // check for not over flow buffer
	if (BlockRead > MaxPayLoad)
		BlockRead = MaxPayLoad;

    if (fseek(pfile, offset, SEEK_SET) == 0)
	{
		if (Debug)
		{
			printf("Read file... \n");
		}

		MpTx.ByteToTransfer =  fread(MpTx.TxPacket, 1, BlockRead, pfile);

		transmit_multipacket(CanN, A_FUNC_FILE_READ, Da_master, &MpTx);
	}
	else
	{
		if (Debug)
		{
			printf("EOF \n");
		}

		MpTx.ByteToTransfer = 0;

		transmit_multipacket(CanN, A_FUNC_FILE_READ, Da_master, &MpTx);
	}

	// Close the file
	fclose(pfile);
}



/***********************************************************************
*
* transmit_multipacket - Send a multi packet
*
*	CanN		-	Can port number
*	Pf			-	Function number
*	Da_master	-	Destination address
*	Tpx			- 	Struct to buffer to send
*
* Return:
*
*/
static void transmit_multipacket(unsigned char CanN, unsigned int Pf, char Da_master, const StMultiPacketTX * Txp)
{
	int		dlc, Index, k;
	int		n, ByteSeek;

	canmsg_t tx;

	ByteSeek = 0;
	Index = 0;

	// check if the packet to transmit if length > 4
	if (Txp->ByteToTransfer > 4)
	{
		// Calculate the number of packet to transmit
		n = ((Txp->ByteToTransfer+9) / 7);
		dlc = 8;
	}
	else
	{
		// Transmit only a one packet
		n = 1;
		dlc = 4 + Txp->ByteToTransfer;
	}

	// Build and send a first packet
	// The first Can frame contains the number of packets and the size
    tx.can_id   =   MASK_COBID | (Pf << 16) | (Da_master << 8) | SA_Slave[CanN];
    tx.can_id   |=  CAN_EFF_FLAG;
    tx.can_dlc  =   dlc;
	tx.data[0]	= 	Index;
	if (n > 255)
		tx.data[1]	= 	0;
	else
		tx.data[1]	= 	n;
	tx.data[2]	= 	(char) (0x00FF & Txp->ByteToTransfer) ;
	tx.data[3]	= 	Txp->ByteToTransfer >> 8;

	// Load the first data
	for(k = 0; k < (dlc - 4); k++)
	{
		tx.data[k+4] = Txp->TxPacket[ByteSeek];
		ByteSeek++;
	}

	// Send the Packet
    send_can(CanN, &tx);

	// Creates the next package
	for ( Index = 0; Index < (n-1); Index ++ )
	{
		tx.data[0] = Index + 1;
		// check if the Bytes transmitted for calculate the dlc
		if ( (Txp->ByteToTransfer - ByteSeek) > 7)
			dlc = 8;
		else
			dlc = (Txp->ByteToTransfer - ByteSeek) + 1;

        tx.can_dlc = dlc;
		for(k = 1; k < dlc; k++)
		{
			tx.data[k] = Txp->TxPacket[ByteSeek];
			ByteSeek ++;
		}

		// Send Packet to Can
        send_can(CanN, &tx);
	}
}



/***********************************************************************
*
* send_nack - Send a nack message
*
*	CanN		-	Can port number
*	Da_master	-	Destinatin address
*	Pf			-	Function number
*	Err			- 	Error code
*/
static void send_nack(int CanN, char Da_master, char Pf, char Err, char db2, char db3)
{
	canmsg_t tx;

	if (Debug)
		printf("Send Nack! \n");

    tx.can_dlc  =   4;
	tx.data[0]	= 	Pf;
	tx.data[1]	= 	Err;
	tx.data[2]	= 	db2;
	tx.data[3]	= 	db3;

    tx.can_id   =   MASK_COBID | (NackMsg << 16) | (Da_master << 8) | SA_Slave[CanN];
    tx.can_id  |=   CAN_EFF_FLAG;
    send_can(CanN, &tx);
}

/***********************************************************************
*
* process_multipacket_message - Management the received the multi-packet
*
*	CanN		-	Can port number
*	Pf			-	Function number
*	rx			-	Pointer to the struct can rx
*	Da_master	-	Destination address
*
* Return:
*			0  -  Not full packet received
*			1  -  Full multi packet received
*/
static int process_multipacket_message(unsigned char CanN, unsigned int Pf, const canmsg_t *rx, char Da_master)
{
	int j;
	struct timespec  now, sub_time, end_time;

	// check if sequence is correct
	if (rx->data[0] != Rx_MPacket[CanN].PacketRx)
	{
		// Send Nack Error Message
		printf("multipacket out of sequence error: received %02X, expected %02X\n", rx->data[0], Rx_MPacket[CanN].PacketRx);
		send_nack(CanN, Da_master,  Pf, ERR_MP_SEQ, rx->data[0], Rx_MPacket[CanN].PacketRx);
		Rx_MPacket[CanN].PacketRx = 0;
		return (-1);
	}

	// If the first packet initialize the variable in the structure at rx
	if (rx->data[0] == 0x00)
	{
		// Init elapsed time
		clock_gettime(CLOCK_MONOTONIC, &Rx_MPacket[CanN].StartTimeOut);
		if (rx->data[1] > 0x00)
			Rx_MPacket[CanN].PacketToRx	= rx->data[1];
		else
			Rx_MPacket[CanN].PacketToRx = 256;

		Rx_MPacket[CanN].ByteToTransfer 	= (rx->data[2] | rx->data[3] << 8);
		Rx_MPacket[CanN].Pf					= Pf;

		if (Rx_MPacket[CanN].ByteToTransfer >= 4)
			Rx_MPacket[CanN].ByteRx = 4;
		else
			Rx_MPacket[CanN].ByteRx = Rx_MPacket[CanN].ByteToTransfer;

		// Load the first data
		for(j = 0; j < Rx_MPacket[CanN].ByteRx; j++)
			Rx_MPacket[CanN].rxPacket[j] = rx->data[j+4];
	}
	else
	{
		// Check that the packet is not in timeout
		clock_gettime(CLOCK_MONOTONIC, &now);
		sub_time = sub_ts(now, Rx_MPacket[CanN].StartTimeOut);
		end_time = period_ts(MULTIPACKETTIMEOUT);

		if (cmp_ts(sub_time, end_time) <= 0)
		{
			// Not in Time Out
			// Init elapsed time
			clock_gettime(CLOCK_MONOTONIC, &Rx_MPacket[CanN].StartTimeOut);
		}
		else
		{
			// Time Out
			// Send Nack Error Message
			send_nack(CanN, Da_master, Pf, ERR_MP_TOUT, 00, 00);
			return (-1);
		}


		// check if the function is not equal stop the recived
		if (Pf != Rx_MPacket[CanN].Pf)
		{
			// Send Nack Error Message
			send_nack(CanN, Da_master, Pf, ERR_MP_PF, Pf, Rx_MPacket[CanN].Pf);
			return (-1);
		}

		// Load the next data to buffer
        for(j = 1; j < rx->can_dlc; j++)
		{
			Rx_MPacket[CanN].rxPacket[Rx_MPacket[CanN].ByteRx] = rx->data[j];
			Rx_MPacket[CanN].ByteRx = Rx_MPacket[CanN].ByteRx + 1;
		}
	}

	// count good packets
	Rx_MPacket[CanN].PacketRx ++;

	if (Debug) {
		printf("PacketToRx		: 0x%x \n",  Rx_MPacket[CanN].PacketToRx);
		printf("PacketRx		: 0x%x \n",  Rx_MPacket[CanN].PacketRx);
		printf("ByteToTransfer	: 0x%x of 0x%x \n",  Rx_MPacket[CanN].ByteToTransfer, Rx_MPacket[CanN].ByteRx);
	}

	// Controls the last packet to received
	if (Rx_MPacket[CanN].PacketRx == Rx_MPacket[CanN].PacketToRx)
	{
		Rx_MPacket[CanN].PacketRx = 0;
		return (1);
	}
	return (0);
}

//@todo comment
static void set_vt3debugger_filter(int socket)
{
    struct can_filter rfilter[2];
    rfilter[0].can_id   = CAN_EFF_MASK; // bit 31 is "extended frame" flag
    rfilter[0].can_mask = 0x1B000000 | ((unsigned)SA_Slave[0] << 8);
    rfilter[1].can_id   = CAN_ERR_MASK; // receive error frames
    rfilter[1].can_mask = 0x20000000;
    int err = setsockopt(socket, SOL_CAN_RAW, CAN_RAW_FILTER, &rfilter, sizeof(rfilter));
    printf("setsockopt(%d, SOL_CAN_RAW, CAN_RAW_FILTER) = %d\n", socket, err);

    can_err_mask_t err_mask = CAN_ERR_BUSOFF;
    err = setsockopt(socket, SOL_CAN_RAW, CAN_RAW_ERR_FILTER, &err_mask, sizeof(err_mask));
    printf("setsockopt(soc, SOL_CAN_RAW, CAN_RAW_ERR_FILTER) returned %d\n", err);

}

static int open_canport(const char * canport)
{
    int soc = -1;
    struct ifreq ifr;
    struct sockaddr_can addr;
    // eror mask option
//    can_err_mask_t err_mask = (  CAN_ERR_BUSOFF );

    /* open socket */
    soc = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	//@todo togliere le print
    printf("socket returned %d\n", soc);
    if(soc < 0)
    {
        return (-1);
    }

    addr.can_family = AF_CAN;
    strcpy(ifr.ifr_name, canport);//,sizeof(ifr.ifr_name));//TODO

    if (ioctl(soc, SIOCGIFINDEX, &ifr) < 0)
    {
    	printf("error: ioctl(soc, SIOCGIFINDEX, ...) failed\n");
        return (-1);
    }

    addr.can_ifindex = ifr.ifr_ifindex;

    fcntl(soc, F_SETFL, O_NONBLOCK);

    if (bind(soc, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
    	printf("error: bind(soc, ...) failed\n");
        return (-1);
    }

    set_vt3debugger_filter(soc);

    printf("socket open: %d\n", soc);
/* send test message
	canmsg_t 	tx;
    tx.can_dlc  = 0x8;
    tx.can_id   = 0x1CA;
    memset(tx.data, 0xAA, 8);
    write(soc, &tx, sizeof(tx));
*/

    return soc;
}


int read_can_msg(int soc,long long timeout_usec,canmsg_t * can_msg)
{
	// wait until 
    int tout_sec = timeout_usec / 1000000;
    int tout_usec = timeout_usec % 1000000;
    struct timeval timeout = {tout_sec, tout_usec};
    fd_set readSet;
    FD_ZERO(&readSet);
    FD_SET(soc, &readSet);
    int n = select(soc+1, &readSet, NULL, NULL, &timeout);
    if (n > 0)
    {
        if (FD_ISSET(soc, &readSet))
        {
            int k = read(soc, can_msg, sizeof(canmsg_t));

            if ( k > 0 )
            {
               int i;
            	printf("rx %08X : ", can_msg->can_id);
            	for(i=0; i<can_msg->can_dlc; i++)
	            	printf("%02X ", can_msg->data[i]);
            	printf("\n");

				/* send test message
            	canmsg_t tx = can_msg[j];
            	tx.can_id = 0x10;
            	send_can(0, &tx);
            	*/

	            return 1;
	        }
        }
    }
    return 0;
}

/***********************************************************************
*
* open_can_devices - Open the communication of the Can 0 and Can 1
*
*/
static void open_can_devices(void)
{
    char 	portname[40];
    int 	port;

	hal_switch_on_canbus();

#if 0 /* this is loaded by init script */
	// make sure the can-bus kernel driver is loaded
	if ( system("modprobe can") != 0 )
	{
		printf("error: 'modprobe can' failed\n");
	}
#endif

    // Open Can ports
	for ( port = 0; port < N_CAN; port ++)
	{
        CanSoc[port] = -1;

		if (bitrate_Slave[port] > 0)
		{
            snprintf(portname, sizeof portname, "can%d", port);
			if (Debug)
            {
                printf("Using CAN device %s\n", portname);
            }

            
            hal_bringup_canbus_interface(port, bitrate_Slave[port] * 1000);
            CanSoc[port] = open_canport(portname);

            if(CanSoc[port] < 0 )
			{
                fprintf(stderr, "Error opening CAN device %s\n", port);
			}
		}

		// init BusOff condition
		Can_BUSOFF[port] = false;

		/* send test message */
		canmsg_t 	tx;
	    tx.can_dlc  = 0x8;
	    tx.can_id   = 0x2CA + port;
	    memset(tx.data, 0xAA, 8);
	    write(CanSoc[port], &tx, sizeof(tx));
	}
}

/***********************************************************************
*
* Check License & Level
*
*/
static char check_license_level(word CustNumber)
{
	int  i;
	struct timespec  now, sub_time, end_time;

	/* 3B6 always pass */
    if ( CustNumber == 0xF3B6 )
    {
        return (255);
    }

	// if vt3_app do not respond for long time,
	// then allow all customer-ID to login with access level 100
	clock_gettime(CLOCK_MONOTONIC, &now);
	sub_time = sub_ts(now, vt3_app_license_timeout);
	end_time = period_ts(VT3_APP_TIMEOUT_LICENSE);
	if (cmp_ts(sub_time, end_time) > 0)
	{
		return (100);
	}

	// scan the table of authorized customer id
	for ( i = 0; i < NLicense; i ++ )
	{
		// back door: if customer id 0xFFFF is present in the table,
		// then allow all customer-ID to login with access level 100
		if (pVt3_shm_can->License[i] == 0xFFFF)
			return (100);

		// scan the license in shared memory
		if ( CustNumber == pVt3_shm_can->License[i] )
		{
			// found the license
			return (pVt3_shm_can->Level[i]);
		}
	}

	// license not found
	return (0);
}


/***********************************************************************
*
* Login  Vt3 Error
*
*	CanN		-	Can port number
*	Da_master	-	ID master
*
*/
static void login_error(unsigned char CanN, char Da_master)
{
	canmsg_t tx;
	word     Chall;	// Challenge

	// random number 0 to 0xFFFF
	Chall   =   rand() % 0xFFFF;
	// store Challenge number
	set_login_challenge(Da_master, Chall);

    tx.can_dlc  =   4;
	tx.data[0]	= 	0;		// Granted access level
	tx.data[1]	= 	0;		// 0 Intel   - 1 Motorola
	tx.data[2]  =	(char) (0x00FF & Chall) ;
	tx.data[3]  =	Chall >> 8;
    tx.can_id   =	MASK_COBID | (A_FUNC_LOGIN << 16) | (Da_master << 8) | SA_Slave[CanN];
    tx.can_id   |=	CAN_EFF_FLAG;
    send_can(CanN, &tx);
}

/***********************************************************************
*
* Login Vt3
*
*	CanN		-	Can port number
*	rx			-	Pointer to the struct can rx
*	Da_master	-	ID master
*
*/
static void login(unsigned char CanN, const canmsg_t *rx, char Da_master)
{
	const char		Pass[]			= "Megara Iblea";
	word			Crc16Ris		= 0;
	unsigned char	stCrc[20];
	canmsg_t		tx;
	unsigned char	AccLevel;
	int				i;
	word			Chall; 			// Challenge

	Chall = find_slot_login_challenge(Da_master);

	// Challenge - crc16
	memset(stCrc, 0, sizeof stCrc);
	stCrc[0] = (char) (0x00FF & Chall);
	stCrc[1] =  Chall >> 8;
	// Load License number
	stCrc[2] = rx->data[0];
	stCrc[3] = rx->data[1];
	// Customer number
	stCrc[4] = rx->data[2];
	stCrc[5] = rx->data[3];
	// Access level
	stCrc[6] = rx->data[6];
	memcpy(stCrc+7, Pass, strlen(Pass));

	if (Debug)
	{
		printf("Login CRC:");
		for (i=0; i<sizeof(stCrc); i++)
			printf(" %02X", stCrc[i]);
		printf("\n");
	}

	// Challenge = 0 initialization value
	if ( Chall != 0 )
	{
		Crc16Ris = crc16(stCrc, 19);

		if (Debug)
		{
			printf("CRC16 Master :  0x%x \n", (rx->data[4] | rx->data[5] << 8));
			printf("CRC16 Slave  :  0x%x \n", Crc16Ris);
		}

		// Crc16 Match
		if (Crc16Ris == (rx->data[4] | rx->data[5] << 8))
		{
			// Get customer access level from shared memory
			AccLevel = check_license_level(rx->data[2] | rx->data[3] << 8);

			// clamp the access level to the one requested by the master
			if ( AccLevel > rx->data[6] )
				AccLevel = rx->data[6];

			if (AccLevel > 0)
			{
				// set login granted
				grant_login(Da_master);

				// free challenge slot
				free_slot_login_challenge(Da_master);

				// access granted
                tx.can_dlc 	= 	4;
				tx.data[0]	= 	AccLevel;
				tx.data[1]	= 	0;	// 0 Intel - 1 Motorola
				tx.data[2]  =   (char) (0x00FF & Chall) ;
				tx.data[3]  =   Chall >> 8;
                tx.can_id   =	MASK_COBID | (A_FUNC_LOGIN << 16) | (Da_master << 8) | SA_Slave[CanN];
                tx.can_id 	|=	CAN_EFF_FLAG;
                send_can(CanN, &tx);
				if (Debug)
					printf("Login granted at level %d\n", tx.data[0]);
				return;
			}
		}

		// free challenge slot
		free_slot_login_challenge(Da_master);
	}

	if (Debug)
		printf("Login FAIL\n");
	login_error(CanN, Da_master);
}



/***********************************************************************
*
* process_debugger_message - process the messages for the debug protocol
*
*	CanN	-	Can port number
*	rx		-	Pointer to the struct can rx
*
*/
static void process_debugger_message(unsigned char CanN, const canmsg_t *rx)
{
	unsigned char  Pf;
    unsigned char  Da_slave;
    unsigned char  Sa_master;
	unsigned char  Ret;

    if ((rx->can_id & 0x03000000) != 0x03000000)
		return;

	// extracts from cob-id the function, Destination address and source address
    Pf = ((rx->can_id & 0x00FF0000) >> 16);
    Da_slave = ((rx->can_id & 0x0000FF00) >> 8);
    Sa_master =  (rx->can_id & 0x000000FF);

	if (Debug) {
		printf("Rx Function:   : 0x%x \n", Pf);
		printf("Rx Destination : 0x%x \n", Da_slave);
		printf("Rx Source      : 0x%x \n", Sa_master);
	}

	// check if the message is for this slave
	if (SA_Slave[CanN] == Da_slave)
	{
		// Close file if command not write file
		if (Pf != R_FUNC_FILE_WRITE && pWritefile != NULL)
		{
			fclose(pWritefile);
			pWritefile = NULL;
		}

		// Login request
		if (Pf == R_FUNC_LOGIN)
		{
			if (Debug)
				printf("Login request \n");

			logout(Sa_master);
			login(CanN, rx, Sa_master);
		}


		if (is_logged_in(Sa_master)) {
			switch (Pf)
			{
				// Shell Command
				case R_FUNC_SHELL:
					Ret = process_multipacket_message(CanN, Pf, rx, Sa_master);
					if (Ret == 1) {
						// Execute the shell command
						run_shell_command(CanN, Sa_master);
						// enable sleep so to reduce CPU load for asynchronous commands
						Enable_Sleep = 1;   // FP20090502
					}
					break;

				// File read
				case R_FUNC_FILE_READ:
					if (Debug)
						printf("File read \n");

					read_file(CanN, rx, Sa_master);
					break;

				// File write
				case R_FUNC_FILE_WRITE:
					if (Debug)
						printf("File write \n");

					// disable sleep so receive multipacket faster
					Enable_Sleep = 0;
					// Get Time
					clock_gettime(CLOCK_MONOTONIC, &Start_Sleep);

					Ret = process_multipacket_message(CanN, Pf, rx, Sa_master);
					if (Ret == 1) {
						write_file(CanN, Sa_master);
					}
					break;
			}
		}
	}
}




/***********************************************************************
*
* run_shell_command - Execute the shell command
*
*	CanN		-	Can port number
*	Da_master	-	Pointer to the struct can rx
*
*/
static void run_shell_command(unsigned char CanN, unsigned char Da_master)
{
	int			Ret;
	canmsg_t	tx;

	// Add terminator to string
	char * cmd = Rx_MPacket[CanN].rxPacket;
	int len = Rx_MPacket[CanN].ByteRx;
	cmd[len] = '\0';

	if (Debug)
		printf("Command: %s\n ", cmd);

	// watchdog disable when command kills vt3_app
	if ( strcmp(cmd, SHELL_CMD_KILL_VT3_APP) == 0 )
	{
		if (Debug)
			printf("disable watchdog: vt3_app\n");
		// disable watchdog
		vt3_app_watchdog_state = VT3_APP_WATCHDOG_DISABLED;
	}

	// watchdog enable when command starts vt3_app
	if ( strcmp(cmd, "/usr/apps/3b6/vt3_app &") == 0 )
	{
		if (Debug)
			printf("enable watchdog: vt3_app\n");
		vt3_app_command_state = VT3_APP_CMD_STATUS_IDLE;
		vt3_app_watchdog_state = VT3_APP_WATCHDOG_ENABLE;
	}

	// TERA-HE ps does not support -ef flags so replace
	char const to_replace[]   = "ps -ef | grep -v grep | grep ";
	char const replace_with[] = "ps     | grep -v grep | grep ";
	if ( memcmp("ps -ef | grep -v grep | grep ", cmd, strlen(to_replace)) == 0 )
	{
		printf("command replaced\n");
		memcpy(cmd, replace_with, strlen(replace_with));
	}

	// Execute the shell command
	printf("system(\"%s\")\n", Rx_MPacket[CanN].rxPacket);
	Ret = system (Rx_MPacket[CanN].rxPacket);
	Ret = WEXITSTATUS(Ret);

	// Send the result of operation
    tx.can_id	=	MASK_COBID | (A_FUNC_SHELL << 16) | (Da_master << 8) | SA_Slave[CanN];
    tx.can_id 	|=	CAN_EFF_FLAG;
    tx.can_dlc 	= 	4;
	memcpy(tx.data, &Ret, 4);
    send_can(CanN, &tx);
}




/**********************************************************************
 *
 * init_watchdog
 *
 * setup the freescale internal watchdog
 *
 */
static void init_watchdog(void)
{
#ifdef WDG
	int timeout;

	/* open wdg driver */
	//--GC: used for CRAMFS
	wdg_fd = open(WD_DEV, O_WRONLY);
	if (wdg_fd < 0)
	{
		// try with the old style device name
		wdg_fd = open(WD_DEV_OLD, O_WRONLY);
	}
	if (wdg_fd < 0)
	{
		printf("Error opening watchdog devices %s or %s\n", WD_DEV, WD_DEV_OLD);
		return;
	}

	/* set timeout: one minute */
	timeout = WD_TIMEOUT_VALUE;
	ioctl(wdg_fd, WDIOC_SETTIMEOUT, &timeout);

	/* read back timeout */
	timeout = 0;
	ioctl(wdg_fd, WDIOC_GETTIMEOUT, &timeout);
	if (timeout != WD_TIMEOUT_VALUE)
		{ printf("warning: can't set watchdog timeout value\n"); }
	else
		{ printf("watchdog timeout set at %d seconds\n", timeout); }
#endif
}


/**********************************************************************
 *
 * trigger_watchdog
 *
 * trigger the freescale internal watchdog
 *
 */
static void trigger_watchdog(void)
{
#ifdef WDG
	/* trigger watchdog all the time */
	if (wdg_fd < 0) { return; }
	ioctl(wdg_fd, WDIOC_KEEPALIVE, 0);
#endif
}




/**********************************************************************
 *
 * vt3_app_watchdog
 *
 * make sure vt3_app is well alive
 *
 */
static void vt3_app_watchdog(void)
{
	static unsigned int  old_watchdog;
	struct timespec  now, sub_time, end_time;
	static unsigned char is_just_started = false;

	/* first run - initialize */
	if (vt3_app_watchdog_state == VT3_APP_WATCHDOG_ENABLE)
	{
		old_watchdog = VT3_APP_COUNT_GET(pVt3_shm_can);
		vt3_app_watchdog_state = VT3_APP_WATCHDOG_INIT_LONG;
		// init timer watchdog
		clock_gettime(CLOCK_MONOTONIC, &vt3_app_watchdog_timeout);

		// init timer license
		clock_gettime(CLOCK_MONOTONIC, &vt3_app_license_timeout);

		// wait for vt3 weakup
		is_just_started = true;
		return;
	}

	/* watchdog is disabled */
	if ( vt3_app_watchdog_state == VT3_APP_WATCHDOG_DISABLED )
	{
		return;
	}

	/* watchdog counter change: this means vt3_app is alive: reload timer */
	if ( old_watchdog != VT3_APP_COUNT_GET(pVt3_shm_can) )
	{
		old_watchdog = VT3_APP_COUNT_GET(pVt3_shm_can);
		vt3_app_watchdog_state = VT3_APP_WATCHDOG_INIT_SHORT;
		// init timer watchdog
		clock_gettime(CLOCK_MONOTONIC, &vt3_app_watchdog_timeout);
		// init timer license
		clock_gettime(CLOCK_MONOTONIC, &vt3_app_license_timeout);

		// vt3 is alive
		is_just_started = false;
	}

	// get now real time
	clock_gettime(CLOCK_MONOTONIC, &now);
	sub_time = sub_ts(now, vt3_app_watchdog_timeout);
	if (vt3_app_watchdog_state == VT3_APP_WATCHDOG_INIT_LONG)
		end_time = period_ts(VT3_APP_WATCHDOG_RELOAD_LONG);			// long time out
	else
		end_time = period_ts(VT3_APP_WATCHDOG_RELOAD_SHORT);		// short time out

	/* timeout */
	if (cmp_ts(sub_time, end_time) > 0)
	{
		// Time out reload vt3_app
		// relaunch vt3_app
		clock_gettime(CLOCK_MONOTONIC, &vt3_app_watchdog_timeout);
		if (is_just_started)
		{
			// vt3 doesn't respond try to load from usb-key
			system(SHELL_CMD_MOUNT);
			// kill vt3_app
			system(SHELL_CMD_KILL_VT3_APP);
			sleep(2);
			// execute the vt3 update app
			if ((system(PATH_UPDATE" &")) == 0)
			{
				vt3_app_command_state = VT3_APP_CMD_STATUS_UPDATE;
			}

			is_just_started = false;
		}
		// only relaunch vt3_app if file is executable
		else if ( access("/usr/apps/3b6/vt3_app", X_OK) == 0 )
		{
			// Time out reload vt3_app
			// relaunch vt3_app
			printf("watchdog timeout: relaunch vt3_app\n");
			// if we have some problem with vt3 update set a NACK
			if (vt3_app_command_state == VT3_APP_CMD_STATUS_UPDATE)
			{
				VT3_CMD_ACK_SET(pVt3_shm_can, VT3_CMD_ACK_NACK);
			}
			// kill vt3_app before relaunch
			system(SHELL_CMD_KILL_VT3_APP);
			sleep(1);
			system(SHELL_CMD_KILL_9_VT3_APP);
			sleep(1);
			// relaunch
			system(SHELL_CMD_VT3_APP);
			vt3_app_watchdog_state = VT3_APP_WATCHDOG_INIT_LONG;
			vt3_app_command_state = VT3_APP_CMD_STATUS_IDLE;
		}
	}
}


/**********************************************************************
 *
 * check_for_update
 *
 * wait for VT3 update requests
 *
 */
static void check_for_commands(void)
{
	int ret;
	/*
	 * A bad command means the vt3_app is old (no command management): pVt3_shm_can->wdt_vt3_app_count
	 * is used totally as a counter variable. Anyway can_daemon reset the bad_command status after a while
	 * beacause a new vt3_app could starts after the old one.
	 */
#ifdef USE_COUNT
	static unsigned char bad_command_status = false;
	if (VT3_CMD_IS_BAD_RST(pVt3_shm_can)) { bad_command_status = false; }
	if (VT3_CMD_IS_BAD(pVt3_shm_can)) { bad_command_status = true; }

	if (bad_command_status) return;
#endif

	// check for command request from VT3
	switch(VT3_CMD_GET(pVt3_shm_can))
	{
	case VT3_CMD_UPDATE:
		// execute command only when in idle state
		if ( vt3_app_command_state != VT3_APP_CMD_STATUS_IDLE) break;

		// clear request
		VT3_CMD_RESET(pVt3_shm_can)

		// mount the USB key, could be already mounted
		ret = system(SHELL_CMD_MOUNT);

		// check the update folder
		if ((ret = access(PATH_UPDATE, R_OK)) != 0)
		{
			printf("vt3 update: access = %d\n", ret);
			VT3_CMD_ACK_SET(pVt3_shm_can, VT3_CMD_ACK_NACK);
			break;
		}

		// kill vt3_app
		if ((ret = system(SHELL_CMD_KILL_VT3_APP)) != 0)
		{
			printf("vt3 update: kill vt3 = %d\n", ret);
			VT3_CMD_ACK_SET(pVt3_shm_can, VT3_CMD_ACK_NACK);
			break;
		}

		// run the update vt3_app
		sleep(2);
		if ((system(PATH_UPDATE" &")) != 0) break;

		// VT3_CMD_ACK_SET(pVt3_shm_can, VT3_CMD_ACK_OK);

		vt3_app_command_state = VT3_APP_CMD_STATUS_UPDATE;
		break;

	case VT3_CMD_END_UPDATE:
		// execute command only when in update state
		if ( vt3_app_command_state != VT3_APP_CMD_STATUS_UPDATE) break;

		// clear request
		VT3_CMD_RESET(pVt3_shm_can)

		// kill vt3_app
		if (system(SHELL_CMD_KILL_VT3_APP))
		{
			VT3_CMD_ACK_SET(pVt3_shm_can, VT3_CMD_ACK_NACK);

			break;
		}

		// run the update vt3_app
		sleep(2);
		system(SHELL_CMD_VT3_APP);
		vt3_app_watchdog_state = VT3_APP_WATCHDOG_INIT_LONG;
		vt3_app_command_state = VT3_APP_CMD_STATUS_IDLE;
		break;

	}
}

/***********************************************************************
*
* candaemon_loop - Read the Can message from buffer Can 0 & Can1
*				infinite loop
*/
static void candaemon_loop(void)
{
    
	int		i, k;
	int  	CanActive = 0;

	const int 	Elapsed_Sleep = 1;		//	Elapsed clock time secs.
	const int	Sleep_Time	  = 50;	 	//  Time of Sleep in ms
	struct timespec  now, sub_time, end_time;

	//  Enabe the vt3 application watchdog
	vt3_app_watchdog_state = VT3_APP_WATCHDOG_ENABLE;

	// Check-in can0
	if (bitrate_Slave[CanActive] == 0)
	{
		CanActive ++;
		// Check-in can1
		if (bitrate_Slave[CanActive] == 0)
		{
			printf("Can not open!");
			return;
		}
	}

	// forever
	while(1)
	{
		/* trigger Freescale internal watchdog */
		//@todo ripristinare
		//trigger_watchdog();

		/* check eoloader scripts in USB */
		//@todo ripristinare
		//run_eoloader(wdg_fd);

		/* make sure vt3_app is alive */
		//@todo ripristinare
		//vt3_app_watchdog();

		/* check for VT3 command requests */
		//@todo ripristinare
		//check_for_commands();

		// try to recover from busoff at intervals
		if ( Can_BUSOFF[CanActive] )
        {
			printf("Bus OFF!!!\n");
            reset_busoff(CanActive);
        }

		// get as many messages as possible
        canmsg_t rx;
        unsigned const rx_count = read_can_msg(CanSoc[CanActive],Sleep_Time*1000LL, &rx);
        if ( rx_count )
        {
		    // busoff error frame
	        if ( (rx.can_id & 0x20000040) == 0x20000040 )
	        {

	            printf("BUSOFF on CAN#%d! \n", CanActive);
	            Can_BUSOFF[CanActive] = true;
	            clock_gettime(CLOCK_MONOTONIC, &busoff_recovery_timer[CanActive]);
	        }

	        // debugger message
	        if ( (rx.can_id & 0x1B000000) == 0x1B000000 )
	        {
	            if (Debug)
	                printf("MSG RX\n");

	            Can_BUSOFF[CanActive] = false;
	            // Decode the can message
	            process_debugger_message(CanActive, &rx);
	        }
	    }
	    else 
		{
			// alternate CAN line
			CanActive ++;
			if (CanActive > (N_CAN - 1))
				CanActive = 0;
		}
	}
}


/***********************************************************************
*
* Load configuration file
*
*/
static void load_config_file(void)
{
	FILE *fp;
	char str[80];
	char DeviceConf[80];
	int  n, baud;
	char *p;

	// Load Configuration file
	for (n =0; n < N_CAN; n++)
	{
		sprintf(DeviceConf, FileCanConf, n);
		if((fp = fopen(DeviceConf, "r")) != NULL)
		{
			while(!feof(fp))
			{
				if(fgets(str, 80, fp))
				{
					// Get the Slave Source address Can
					p = strstr(str, "SA");
					if(p != NULL)
					{
						p = strstr(str, "=");
						SA_Slave[n] = atoi(p+1);
						printf("can%d source address = 0x%x\n", n, SA_Slave[n]);
					}

					// Get bit rate for this port
					p = strstr(str, "bitrate");
					if(p != NULL)
					{
						p = strstr(str, "=");
						baud = atoi(p+1);
						bitrate_Slave[n] = baud;
						printf("can%d bit rate       = %d\n", n, baud);
						set_can_baud(n,	baud);
					}
				}
			}
			fclose(fp);
		}
		else
		{
			// *****************
			// Set default value if lost the file config
			SA_Slave[n] = CAN_SA_SLAVE_DEFAULT;
			baud = CAN_BAUD_DEFAULT;
			bitrate_Slave[n] = baud;
			printf("can%d bit rate       = %d\n", n, baud);
			printf("can%d source address = 0x%x\n", n, SA_Slave[n]);
		}
	}
}

/***********************************************************************
*
* Init Share Memory
*
*/
static void init_shared_memory(void)
{
	int 	shmid;
	int  	n;

	if (Debug)
	{
		printf("Size of vt3_shm_can %d \n", sizeof(vt3_shm_can));
	}
	/*
	 * connect to (and possibly create) the segment.
	 * Size of shared memory is expansed to page size to be ready for furure evolution
	 */
	shmid = shmget(SHMEM_KEY, getpagesize(), 0644 | IPC_CREAT);
	if ( shmid < 0 )
	{
		printf("shmget Error \n");
	}

	// attach to the segment to get a pointer to it:
    pVt3_shm_can = (vt3_shm_can *)shmat(shmid, (void *)0, 0);
	if ( pVt3_shm_can == (vt3_shm_can *)(-1) )
	{
		printf("shmat Error \n");
	}

	// init Bit Field Logged
	//--GC
	memset(pVt3_shm_can->BitFieldLogged, 0, SizeBitFieldLogged);

	// License and Level init to "0"
	// and init Challenge to "0"
	for ( n = 0; n < NLicense; n ++ )
	{
		// License
		pVt3_shm_can->License[n] = 0;
		pVt3_shm_can->Level[n] = 0;
	}

	for ( n = 0; n < NCHALLENGE; n ++ )
	{
		// Challenge
		Challenge[n].Use       = false;
		Challenge[n].Da_master = 0;
		Challenge[n].Challenge = 0;
	}


	// Can id slave init & bitrate
	for (n = 0; n < N_CAN; n++)
	{
		pVt3_shm_can->SA_Slave[n]      = SA_Slave[n];
		pVt3_shm_can->bitrate_Slave[n] = bitrate_Slave[n];
	}

	VT3_CMD_RESET(pVt3_shm_can)
	VT3_CMD_ACK_RST(pVt3_shm_can)
	pVt3_shm_can->version = 0x06010102;
	pVt3_shm_can->can_daemon_version = VERSION_CAN_DAEMON;

}

/***********************************************************************
*
* Main application
*
*/
int main(int argc, char *argv[])
{
	// print version
    printf("can_daemon for TERA He v%d\n", VERSION_CAN_DAEMON);

    // starting directory
    chdir("/usr/apps/3b6");

	// init randomizer
	srand(time(NULL));

	/* --GC
	// Set Path vt3
	if (chdir(Vt3_Path) != 0)
		printf("Error setting default path: %s\n", Vt3_Path);
	 */

	// Load Configuration file
	load_config_file();

	// Open the Can-port
	open_can_devices();

	// Init the shared Memory
	init_shared_memory();

	// init watchdog
	init_watchdog();

	// run loop
	candaemon_loop();

	return (0);
}


// end of file
