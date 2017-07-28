/*****< btpmvsapi.h >**********************************************************/
/*      Copyright 2011 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTVSAPI - Vendor specific functions/definitions/constants used to define  */
/*            a set of vendor specific functions supported for CC256x.        */
/*                                                                            */
/*  Author: Kobi L.                                                           */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/08/11  Kobi L.       Initial creation.                                */
/*   12/16/15  D. Keren      Adding Vendor DDIP command.                      */
/******************************************************************************/
#ifndef __BTVSAPIH__
#define __BTVSAPIH__

#include "SS1BTPS.h"            /* Bluetopia API Prototypes/Constants.        */

   /* The following enumerated type represents the different modulation */
   /* types that may be specified in VS_DRPb_Tester_Con_TX.             */
typedef enum
{
   mtContinuousWave = 0x00,
   mtGFSK           = 0x01,
   mtEDR2           = 0x02,
   mtEDR3           = 0x03,
   mtBLE            = 0x04,
   mtANT            = 0x05
} VS_Modulation_Type_t;

   /* The following enumerated type represents the different Test       */
   /* Patterns that may be specified in VS_DRPb_Tester_Con_TX.          */
typedef enum
{
   tpPN9      = 0x00,
   tpPN15     = 0x01,
   tpZ0Z0     = 0x02,
   tpALL_ONE  = 0x03,
   tpALL_ZERO = 0x04,
   tpF0F0     = 0x05,
   tpFF00     = 0x06,
   tpNOT_USED = 0x07
} VS_Test_Pattern_t;

   /* The following types represent the Minimum and Maximum Frequencies */
   /* that may be specified in the VS_DRPb_Tester_Con_TX,               */
   /* VS_DRPb_Tester_Con_RX, and VS_DRPb_BER_Meter_Start commands.      */
#define VS_FREQUENCY_MINIMUM                             2402
#define VS_FREQUENCY_MAXIMUM                             2480

   /* The following constants represent the Minimum and Maximum Power   */
   /* Levels that may be specified in VS_DRPb_Tester_Con_TX.            */
#define VS_POWER_LEVEL_INDEX_MINIMUM                     0x00
#define VS_POWER_LEVEL_INDEX_MAXIMUM                     0x07
#define VS_POWER_LEVEL_INDEX_PA_OFF                      0x08

   /* The following constants specify the acceptable values for the     */
   /* packet type parameter for the VS_DRPb_Tester_Packet_TX_RX() and   */
   /* VS_DRPb_BER_Meter_Start commands.                                 */
#define VS_PACKET_TYPE_DM1                               0x00
#define VS_PACKET_TYPE_DH1                               0x01
#define VS_PACKET_TYPE_DM3                               0x02
#define VS_PACKET_TYPE_DH3                               0x03
#define VS_PACKET_TYPE_DM5                               0x04
#define VS_PACKET_TYPE_DH5                               0x05
#define VS_PACKET_TYPE_2DH1                              0x06
#define VS_PACKET_TYPE_2DH3                              0x07
#define VS_PACKET_TYPE_2DH5                              0x08
#define VS_PACKET_TYPE_3DH1                              0x09
#define VS_PACKET_TYPE_3DH3                              0x0A
#define VS_PACKET_TYPE_3DH5                              0x0B

   /* The following constants define the value A3DP Roles to the        */
   /* VS_AVPR_Enable() function.                                        */
#define AVRP_A3DP_SOURCE_ROLE                            0
#define AVRP_A3DP_SINK_ROLE                              1

   /* The following defines the list of valid values for the sampling   */
   /* frequencies, channel mode, and alloc method for the codec config  */
   /* parameters used in VS_A3DP_CODEC_Configure().                     */
#define AVRP_SOURCE_PCM                                  0x00
#define AVRP_SOURCE_FM                                   0x01

#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_MASK           0x0F
#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_8K             0x01
#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_11K            0x02
#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_12K            0x03
#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_16K            0x04
#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_22K            0x05
#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_24K            0x06
#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_32K            0x07
#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_44K1           0x08
#define AVRP_AUDIO_FORMAT_PCM_SAMPLE_RATE_48K            0x09

#define AVRP_AUDIO_FORMAT_SBC_SAMPLE_RATE_MASK           0x30
#define AVRP_AUDIO_FORMAT_SBC_SAMPLE_RATE_16K            0x00
#define AVRP_AUDIO_FORMAT_SBC_SAMPLE_RATE_32K            0x10
#define AVRP_AUDIO_FORMAT_SBC_SAMPLE_RATE_44K1           0x20
#define AVRP_AUDIO_FORMAT_SBC_SAMPLE_RATE_48K            0x30

#define AVRP_AUDIO_FORMAT_SBC_MODE_MASK                  0xC0
#define AVRP_AUDIO_FORMAT_SBC_MODE_MONO                  0x00
#define AVRP_AUDIO_FORMAT_SBC_MODE_DUAL_CHANNEL          0x40
#define AVRP_AUDIO_FORMAT_SBC_MODE_STEREO                0x80
#define AVRP_AUDIO_FORMAT_SBC_MODE_JOINT_STEREO          0xC0

#define AVRP_SBC_FORMAT_BLOCK_LENGTH_MASK                0x03
#define AVRP_SBC_FORMAT_BLOCK_LENGTH_4                   0x00
#define AVRP_SBC_FORMAT_BLOCK_LENGTH_8                   0x01
#define AVRP_SBC_FORMAT_BLOCK_LENGTH_12                  0x02
#define AVRP_SBC_FORMAT_BLOCK_LENGTH_16                  0x03

#define AVRP_SBC_FORMAT_ALLOCATION_METHOD_MASK           0x04
#define AVRP_SBC_FORMAT_ALLOCATION_METHOD_LOUDNESS       0x00
#define AVRP_SBC_FORMAT_ALLOCATION_METHOD_SNL            0x04

#define STOP_STREAM_FLAG_FLUSH_DATA                      0x01
#define STOP_STREAM_FLAG_GENERATE_STOP_EVENT             0x02

   /* The following structure type definition specifies the command     */
   /* parameters for the VS_DRPb_Tester_Packet_TX_RX() function.        */
typedef struct _tagVS_DRPb_Tester_Packet_TX_RX_Params_t
{
   Byte_t ACLTXPacketType;
   Byte_t FrequencyMode;
   Word_t TXSingleFrequency;
   Word_t RXSingleFrequency;
   Byte_t ACLTXPacketDataPattern;
   Word_t ACLPacketDataLength;
   Byte_t PowerLevelIndex;
   Byte_t DisableWhitening;
   Word_t PRBS9InitValue;
} VS_DRPb_Tester_Packet_TX_RX_Params_t;

   /* The following structure type definition specifies the command     */
   /* parameters for the VS_DRPb_BER_Meter_Start() function.            */
typedef struct _tagVS_DRPb_BER_Meter_Start_Params_t
{
   Byte_t    ChannelIndex;
   BD_ADDR_t BD_ADDR;
   Byte_t    LT_Address;
   Byte_t    PacketType;
   Word_t    PacketLength;
   Word_t    NumberOfPackets;
   Word_t    PRBS_InitializationValue;
   Byte_t    PollPeriod;
} VS_DRPb_BER_Meter_Start_Params_t;

   /* The following structure type definition specifies the return      */
   /* parameters of the VS_DRP_Read_BER_Meter_Result() function.        */
typedef struct _tagVS_DRP_Read_BER_Meter_Result_Return_Params_t
{
   Byte_t  FinishedAtLeastOneTest;
   Word_t  NumberOfPacketsReceived;
   DWord_t TotalBitsCounted;
   DWord_t NumberOfErrorsFound;
} VS_DRP_Read_BER_Meter_Result_Return_Params_t;

   /* The following structure type definition specifies the return      */
   /* parameters of the VS_Read_Patch_Version() function.               */
typedef struct _tagVS_Read_Patch_Version_Return_Params_t
{
   Byte_t EnabledMask[6];
   Byte_t ReleaseMajor;
   Byte_t ReleaseMinor;
   Byte_t PackageID;
   Byte_t BuildNumber;
} VS_Read_Patch_Version_Return_Params_t;

/* The following structure type definition specifies the command        */
/* parameters of the PM_VS_PCM_Codec_Config() function.                 */
typedef struct _tagVS_PCMChannelConfig_t
{
	Word_t Size;  	/* bits per sample */
	Word_t Offset;  /* offset from fsync */
	Byte_t Edge; 	/* * 0 = rising edge, 1 = falling edge  */
} PCMChannelConfig_t;

typedef struct _tagVS_PCMCodecConfig_t
{
	DWord_t FSyncFreq;      /* Fsync Frequency [Hz] */
	Word_t  PCMFreq;        /* PCM Frequency [KHz] */
	Byte_t  ClockDirection; /* 0 - BT device Clock Master, 1 - BT device is slave */
	Byte_t  FsyncEdge; 	    /* 0 = rising edge, 1 = falling edge  */
	Byte_t  FSyncPolarity;  /* 0 - active high, 1 - active low */
	PCMChannelConfig_t CH1Out;
	PCMChannelConfig_t CH1In;
	PCMChannelConfig_t CH2Out;
	PCMChannelConfig_t CH2In;
} PCMCodecConfig_t;

   /* The following function prototype represents the vendor specific   */
   /* function which is used to change the HCILL parameters that are    */
   /* used for the HCILL Low Power protocol for the Local Bluetooth     */
   /* Device specified by the Bluetooth Protocol Stack that is specified*/
   /* by the Bluetooth Protocol Stack ID.  The second is the            */
   /* InactivityTimeout on the Uart in ms.  If no traffic on Uart lines */
   /* after this time the Controller sends a Sleep Indication.  The     */
   /* third is the RetransmitTimeout (ms) for the Sleep Indication if no*/
   /* Sleep Acknowledgement (from the Host) is received.  The fourth is */
   /* the Controller RTS pulse width during Controller wakeup (specified*/
   /* in us).  This function returns zero if successful or a negative   */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_HCILL_Parameters(Word_t InactivityTimeout, Word_t RetransmitTimeout, Byte_t CTSPulseWidth);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_HCILL_Parameters_t)(Word_t InactivityTimeout, Word_t RetransmitTimeout, Byte_t CTSPulseWidth);
#endif

   /* The following function implements the vendor-specific             */
   /* Sleep_Mode_Configurations command.  Refer to the Bluetooth (WL18xx*/
   /* and WL18xxQ) Vendor-Specific HCI Commands User's Guide for a      */
   /* description of the command's parameters.  This function returns 0 */
   /* on success and a negative error code otherwise.                   */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_Sleep_Mode_Configurations(Byte_t DeepSleepEnable);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_Sleep_Mode_Configurations_t)(Byte_t DeepSleepEnable);
#endif

   /* The following function prototype represents the vendor specific   */
   /* function which is used to change the public Bluetooth Address     */
   /* (also known as a MAC address) for the local Bluetooth Device      */
   /* specified by the Bluetooth Protocol Stack ID.  The second         */
   /* parameter contains the Bluetooth Address to set.  This function   */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_Write_BD_ADDR(BD_ADDR_t BD_ADDR);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_Write_BD_ADDR_t)(BD_ADDR_t BD_ADDR);
#endif

   /* The following function implements the vendor-specific             */
   /* PM_VS_DRPb_Set_Power_Vector command.  Refer to the Bluetooth (WL18xx */
   /* and WL18xxQ) Vendor-Specific HCI Commands User's Guide for a      */
   /* description of the command's parameters.  This function returns 0 */
   /* on success and a negative error code otherwise.                   */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_DRPb_Set_Power_Vector(Byte_t ModulationType, Byte_t *PowerLevels, Byte_t PowerLevelIndex);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_DRPb_Set_Power_Vector_t)(Byte_t ModulationType, Byte_t *PowerLevels, Byte_t PowerLevelIndex);
#endif


   /* The following function is used to write the codec interface       */
   /* parameters and the PCM and Frame Sync frequencies. IsMaster should*/
   /* mpm-zero if Bluetooth device serves as clock master, 0 if it is a */
   /* slave.                                                            */
   /* returns zero if successful or a negative return error code if     */
   /* there was an error.                                               */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_PCM_Codec_Config(PCMCodecConfig_t *CodecConfig);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_PCM_Codec_Config_t)(PCMCodecConfig_t *CodecConfig);
#endif

   /* The following function is used to enable or disable the AVPR      */
   /* features.  This function returns zero if successful or a negative */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_AVPR_Enable(Boolean_t AVPREnable, Boolean_t LoadCode, Byte_t A3DPRole);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_AVPR_Enable_t)(Boolean_t AVPREnable, Boolean_t LoadCode, Byte_t A3DPRole);
#endif

   /* The following function implements the vendor-specific             */
   /* DRPb_Enable_RF_Calibration_Enhanced command.  Refer to the        */
   /* Bluetooth (WL18xx and WL18xxQ) Vendor-Specific HCI Commands User's*/
   /* Guide for a description of the command's parameters.  This        */
   /* function returns 0 on success and a negative error code otherwise.*/
BTPSAPI_DECLARATION int BTPSAPI PM_VS_DRPb_Enable_RF_Calibration_Enhanced(Byte_t Mode, Byte_t PeriodicOptions, DWord_t CalibrationProcedures, Byte_t OverrideTemperatureCondition);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_DRPb_Enable_RF_Calibration_Enhanced_t)(Byte_t Mode, Byte_t PeriodicOptions, DWord_t CalibrationProcedures, Byte_t OverrideTemperatureCondition);
#endif

   /* The following function implements the vendor-specific             */
   /* PM_VS_DRPb_Tester_Con_TX command.  Refer to the Bluetooth (WL18xx and*/
   /* WL18xxQ) Vendor-Specific HCI Commands User's Guide for a          */
   /* description of the command's parameters.  This function returns 0 */
   /* on success and a negative error code otherwise.                   */
BTPSAPI_DECLARATION int BTPSAPI DRPb_Tester_Con_TX(Word_t Frequency, VS_Modulation_Type_t Modulation, VS_Test_Pattern_t TestPattern, Byte_t PowerLevelIndex);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_DRPb_Tester_Con_TX_t)(Word_t Frequency, VS_Modulation_Type_t Modulation, VS_Test_Pattern_t TestPattern, Byte_t PowerLevelIndex);
#endif

   /* The following function implements the vendor-specific             */
   /* VS_DRPb_Tester_Packet_TX_RX command.  Refer to the Bluetooth      */
   /* (WL18xx and WL18xxQ) Vendor-Specific HCI Commands User's Guide for*/
   /* a description of the command's parameters.  This function returns */
   /* 0 on success and a negative error code otherwise.                 */
BTPSAPI_DECLARATION int BTPSAPI DRPb_Tester_Packet_TX_RX(VS_DRPb_Tester_Packet_TX_RX_Params_t *VS_DRPb_Tester_Packet_TX_RX_Params);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_DRPb_Tester_Packet_TX_RX_t)(VS_DRPb_Tester_Packet_TX_RX_Params_t *VS_DRPb_Tester_Packet_TX_RX_Params);
#endif

   /* The following function implements the vendor-specific             */
   /* PM_VS_DRPb_Tester_Con_RX command.  Refer to the Bluetooth (WL18xx and*/
   /* WL18xxQ) Vendor-Specific HCI Commands User's Guide for a          */
   /* description of the command's parameters.  This function returns 0 */
   /* on success and a negative error code otherwise.                   */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_DRPb_Tester_Con_RX(Word_t Frequency, Byte_t RxMode, Byte_t ModulationType);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_DRPb_Tester_Con_RX_t)(Word_t Frequency, Byte_t RxMode, Byte_t ModulationType);
#endif

   /* The following function implements the vendor-specific             */
   /* PM_VS_DRPb_Reset command.  Refer to the Bluetooth (WL18xx and        */
   /* WL18xxQ) Vendor-Specific HCI Commands User's Guide for a          */
   /* description of the command's parameters.  This function returns 0 */
   /* on success and a negative error code otherwise.                   */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_DRPb_Reset();

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_DRPb_Reset_t)();
#endif

   /* The following function implements the vendor-specific             */
   /* PM_VS_DRPb_BER_Meter_Start command.  Refer to the Bluetooth (WL18xx  */
   /* and WL18xxQ) Vendor-Specific HCI Commands User's Guide for a      */
   /* description of the command's parameters.  This function returns 0 */
   /* on success and a negative error code otherwise.                   */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_DRPb_BER_Meter_Start(VS_DRPb_BER_Meter_Start_Params_t *VS_DRPb_BER_Meter_Start_Params);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_DRPb_BER_Meter_Start_t)(VS_DRPb_BER_Meter_Start_Params_t *VS_DRPb_BER_Meter_Start_Params);
#endif

   /* The following function implements the vendor-specific             */
   /* PM_VS_DRP_Read_BER_Meter_Result command.  Refer to the Bluetooth     */
   /* (WL18xx and WL18xxQ) Vendor-Specific HCI Commands User's Guide for*/
   /* a description of the command's parameters.  This function returns */
   /* 0 on success and a negative error code otherwise.                 */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_DRP_Read_BER_Meter_Result(VS_DRP_Read_BER_Meter_Result_Return_Params_t *VS_DRP_Read_BER_Meter_Result_Return_Params);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_DRP_Read_BER_Meter_Result_t)(VS_DRP_Read_BER_Meter_Result_Return_Params_t *VS_DRP_Read_BER_Meter_Result_Return_Params);
#endif

   /* The following function implements the vendor-specific             */
   /* VS_Read_Patch_Version command.  Refer to the Bluetooth (WL18xx and*/
   /* WL18xxQ) Vendor-Specific HCI Commands User's Guide for a          */
   /* description of the command's parameters.  This function returns 0 */
   /* on success and a negative error code otherwise.                   */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_Read_Patch_Version(VS_Read_Patch_Version_Return_Params_t *VS_Read_Patch_Version_Return_Params);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_Read_Patch_Version_t)(VS_Read_Patch_Version_Return_Params_t *VS_Read_Patch_Version_Return_Params);
#endif

   /* The following function is a wrapper for the                       */
   /* PM_VS_DRPb_BER_Meter_Start and PM_VS_DRP_Read_BER_Meter_Result          */
   /* vendor-specific commands .  It implements the procedure for the   */
   /* WL18xx PLT Tests documented on TI's WL18xx Bluetopia PM Bluetooth */
   /* RF Testing Wiki guide.  Refer to the Bluetooth (WL18xx and        */
   /* WL18xxQ) Vendor-Specific HCI Commands User's Guide for a          */
   /* description of the return parameters.  This function returns 0 on */
   /* success and a negative error code otherwise.                      */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_Production_Line_Test(BD_ADDR_t BD_ADDR, VS_DRP_Read_BER_Meter_Result_Return_Params_t *VS_DRP_Read_BER_Meter_Result_Return_Params);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_Production_Line_Test_t)(BD_ADDR_t BD_ADDR, VS_DRP_Read_BER_Meter_Result_Return_Params_t *VS_DRP_Read_BER_Meter_Result_Return_Params);
#endif

   /* The following function is called when an A2DP connection is in the*/
   /* open state to tell the controller the L2CAP parameters of the     */
   /* channel.  This function returns zero if successful or a negative  */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Open_Stream(Byte_t Connection_Handle, Word_t CID, Word_t MTU);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Open_Stream_t)(Byte_t Connection_Handle, Word_t CID, Word_t MTU);
#endif

   /* The following function is used to inform the controller when A2DP */
   /* connection closes.  This function returns zero if successful or a */
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Close_Stream(Byte_t Connection_Handle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Close_Stream_t)(Byte_t Connection_Handle);
#endif

   /* The following function is used to configure the SBC Encoder or    */
   /* Decoder parameters.  This function returns zero if successful or a*/
   /* negative return error code if there was an error.                 */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Codec_Configuration(Byte_t AudioFormat, Byte_t SBCFormat, Byte_t BitPoolSize);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Codec_Configuration_t)(Byte_t AudioFormat, Byte_t SBCFormat, Byte_t BitPoolSize);
#endif

   /* The following function is used to start assisted A2DP streaming.  */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Start_Stream(Byte_t Connection_Handle);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Start_Stream_t)(Byte_t Connection_Handle);
#endif

   /* The following function is used to stop assisted A2DP streaming.   */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Stop_Stream(Byte_t Connection_Handle, Byte_t Flags);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Stop_Stream_t)(Byte_t Connection_Handle, Byte_t Flags);
#endif

   /* The following function is used to open a stream as an A2DP SNK    */
   /* device. This function returns zero if successful or a negative    */
   /* return error code if there was an error.                          */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Sink_Open_Stream(Byte_t Connection_Handle, Word_t CID);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Sink_Open_Stream)(Byte_t Connection_Handle, Word_t CID);
#endif

   /* The following function is used to close a SNK stream previously   */
   /* opened using PM_VS_A3DP_Sink_Open_Stream. This function returns zero */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Sink_Close_Stream();

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Sink_Close_Stream)();
#endif

   /* The following function is used to configure an A3DP device as an  */
   /* A2DP SNK, giving it PCM and SBC parameters. This configuration    */
   /* should be performed after PM_VS_PCM_CodecConfig and PM_VS_AVPR_Enable.  */
   /* This function returns zero if successful or a negative return     */
   /* error code if there was an error.                                 */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Sink_Codec_Configuration(Byte_t AudioFormat, Byte_t SBCFormat);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Sink_Codec_Configuration)(Byte_t AudioFormat, Byte_t SBCFormat);
#endif

   /* The following function is used to change a stream previously      */
   /* opened via PM_VS_A3DP_Sink_Open_Stream to the "Playing" state. This  */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Sink_Start_Stream();

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Sink_Start_Stream)();
#endif

   /* The following function is used to change a stream previously      */
   /* opened via PM_VS_A3DP_Sink_Open_stream to the "Stopped" state.  This */
   /* function returns zero if successful or a negative return error    */
   /* code if there was an error.                                       */
BTPSAPI_DECLARATION int BTPSAPI PM_VS_A3DP_Sink_Stop_Stream();

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PFN_PM_VS_A3DP_Sink_Stop_Stream)();
#endif

   /*
    *  Function Name: SendHciDDIPCommand.
    *  Description: The following function is responsible for sending HCI
    *  	vendor specific command, for changing the Remote Bluetooth device ACL
    *  	priority over scans.
    *
    *  Input: The remote device Bluetooth address.
    *
    *  Output: None
    *
    *  Notes: The function is sent when start playing A2DP stream to
    *  	give priority to the audio data over scans.
    *
    */
 BTPSAPI_DECLARATION int BTPSAPI PM_VS_DDIPCommand(Byte_t best_effort_percentage, Byte_t guaranteed_percentage);

#ifdef INCLUDE_BLUETOOTH_API_PROTOTYPES
   typedef int (BTPSAPI *PM_VS_DDIPCommand_t)(Byte_t Enable, Byte_t Connection_Handle);
#endif

#endif
