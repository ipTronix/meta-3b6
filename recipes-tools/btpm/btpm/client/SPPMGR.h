/*****< sppmgr.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SPPMGR - Serial Port Profile Manager Implementation for Stonestreet One   */
/*           Bluetooth Protocol Stack Platform Manager.                       */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/16/10  D. Lange       Initial creation.                               */
/*   03/02/16  D. Keren       Add the ISPP_Configuration_Params_t to the MFi  */
/*                            configuration function                          */
/******************************************************************************/
#ifndef __SPPMGRH__
#define __SPPMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

#if BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_SUPPORT_MFI

#include "SS1BTISP.h"            /* Bluetopia MFi API Prototypes/Constants.   */

#endif

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Serial Port Profile Manager Implementation.  This  */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Serial Port Profile Manager Implementation.               */
int _SPPM_Initialize(void);

   /* The following function is responsible for shutting down the Serial*/
   /* Port Profile Implementation.  After this function is called the   */
   /* Serial Port Profile Manager Implementation will no longer operate */
   /* until it is initialized again via a call to the _SPPM_Initialize()*/
   /* function.                                                         */
void _SPPM_Cleanup(void);

   /* The following function is provided to allow a mechanism for local */
   /* modules to register a Local Serial Port Profile Manager Serial    */
   /* Port Server.  This function accepts the RFCOMM/SPP Port to use for*/
   /* the Server, followed by the the Port Flags.  This function returns*/
   /* a positive, non-zero, value if successful, or a negative return   */
   /* error code if there was an error.                                 */
   /* * NOTE * A successful return value represents the Server Port     */
   /*          Handle that can be used with various functions in this   */
   /*          module to specify this local server port.                */
int _SPPM_RegisterServerPort(unsigned int ServerPort, unsigned long PortFlags);

   /* The following function is provided to allow a mechanism for local */
   /* module to accept or reject (authorize) an incoming Server Port    */
   /* connection.  This function returns zero if successful, or a       */
   /* negative return error code if there was an error.                 */
   /* * NOTE * If the connection is accepted, it does NOT mean that the */
   /*          connection is successfully connected.  A Server Port     */
   /*          Opened event will be dispatched to signify the actual    */
   /*          result.                                                  */
int _SPPM_OpenServerPortRequestResponse(unsigned int PortHandle, Boolean_t Accept);

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered Server Port.  This */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _SPPM_UnRegisterServerPort(unsigned int PortHandle);

   /* The following function is provided to allow a mechanism for local */
   /* modules to Register an SDP Service Records for a previously       */
   /* registered Server Port.  This function returns a positive,        */
   /* non-zero, value if successful, or a negative return error code if */
   /* there was an error.  If this function is successful, the value    */
   /* that is returned represents the SDP Service Record Handle of the  */
   /* Service Record that was added to the SDP Database.                */
long _SPPM_RegisterServerPortServiceRecord(unsigned int PortHandle, SPPM_Service_Record_Information_t *ServiceRecordInformation);

   /* The following function is provided to allow a mechanism for local */
   /* modules to open a remote Serial Port Profile connection.  This    */
   /* function returns a positive, non-zero, value if successful, or a  */
   /* negative return error code if there was an error.  If this        */
   /* function is successful, the value that is returned represents the */
   /* Serial Port Profile Manager Port Handle of the connection.        */
int _SPPM_OpenRemotePort(BD_ADDR_t RemoteDevice, unsigned int ServerPort, unsigned long OpenFlags);

   /* The following function is provided to allow a mechanism for local */
   /* modules to close an active connection for the specified Serial    */
   /* Port (either local server or remote).  This function returns zero */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * This function will NOT Un-Register a local server port,  */
   /*          it will just disconnect any currently connected remote   */
   /*          device.                                                  */
int _SPPM_ClosePort(unsigned int PortHandle);

   /* The following function is provided to allow a mechanism for local */
   /* modules to read data from an active connection for the specified  */
   /* Serial Port (either local server or remote).  This function       */
   /* returns the number of bytes read or present in the buffer if      */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * If the final parameters are specified as 0, NULL         */
   /*          (respectively) then this instructs the Serial Port       */
   /*          Profile Manager to return how many data bytes are        */
   /*          currently in the Serial Port buffer.                     */
   /* * NOTE * This function will return zero if no data has been       */
   /*          received.                                                */
int _SPPM_ReadData(unsigned int PortHandle, unsigned int DataLength, unsigned char *DataBuffer);

   /* The following function is provided to allow a mechanism for local */
   /* modules to write data to an active connection for the specified   */
   /* Serial Port (either local server or remote).  This function       */
   /* returns the number of bytes written if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * This function will return the amount of data that was    */
   /*          actually sent.                                           */
int _SPPM_WriteData(unsigned int PortHandle, unsigned int DataLength, unsigned char *DataBuffer);

   /* The following function is provided to allow a mechanism for local */
   /* modules to send a Line Status notification to the specified Serial*/
   /* Port (either local server or remote).  This function returns zero */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
int _SPPM_SendLineStatus(unsigned int PortHandle, unsigned long LineStatusMask);

   /* The following function is provided to allow a mechanism for local */
   /* modules to send a Port Status notification to the specified Serial*/
   /* Port (either local server or remote).  This function returns zero */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
int _SPPM_SendPortStatus(unsigned int PortHandle, SPPM_Port_Status_t *PortStatus);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query whether a particular RFCOMM/SPP Port is currently*/
   /* in use as a local SPP Server. This function returns zero if       */
   /* successful, or a negative return error code if there was an error.*/
int _SPPM_QueryServerPresent(unsigned int ServerPort, Boolean_t *Present);

   /* The following function is provided to allow a mechanism for       */
   /* local modules to locate an RFCOMM/SPP Port which is currently     */
   /* available to be used as a local SPP Server. This function returns */
   /* a positive, non-zero, value if successful, or a negative return   */
   /* error code if there was an error. If this function is successful, */
   /* the value that is returned represents the SPPM Port Number which  */
   /* is available.                                                     */
int _SPPM_FindFreeServerPort(void);

   /* The following function is provided to allow the programmer a means*/
   /* to change the default Transmit and Receive Buffer Sizes.  This    */
   /* function accepts as the input the Port ID of the SPP Port to      */
   /* change the buffer size for, and the next two parameters represent */
   /* the requested Buffer size to change the Receive and Transmit      */
   /* Buffer to (respectively).  The special constant                   */
   /* SPPM_BUFFER_SIZE_CURRENT can be used to specify that the requested*/
   /* Buffer Size (either Transmit and/or Receive) NOT be changed.  This*/
   /* function returns zero if the specified Buffer Size(s) were        */
   /* changed, or a negative return error code if there was an error.   */
   /* * NOTE * This function causes ALL Data in each Buffer to be lost. */
   /*          This function clears the each Data Buffer so that all the*/
   /*          available data buffer is available to be used.           */
int _SPPM_ChangeBufferSize(unsigned int PortHandle, unsigned int ReceiveBufferSize, unsigned int TransmitBufferSize);

#if BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_SUPPORT_MFI

   /* The following function is provided to allow a mechanism to enable */
   /* MFi support in the Serial Port Profile Manager (SPPM).  This      */
   /* function returns zero if MFi was configured within the Serial Port*/
   /* Profile Manager (SPPM) or a negative return error code if there   */
   /* was an error.                                                     */
   /* * NOTE * MFi settings can only be configured once and are global  */
   /*          in nature.                                               */
   /* * NOTE * Simply enabling MFi support by calling this function     */
   /*          does not mean that MFi will be available for all ports   */
   /*          configured.  Each port that is specified by this module  */
   /*          can specify whether or not MFi is allowed/requested      */
   /*          when it is configured/opened.                            */
int _SPPM_ConfigureMFiSettings(SPPM_MFi_Configuration_Settings_t *MFiConfigurationSettings, ISPP_Configuration_Params_t *ISPPConfigurationParams);

#endif

   /* The following function is provided to allow a mechanism to        */
   /* determine if the specified, connected, port is operating in either*/
   /* SPP or MFi mode.  This function returns zero if successful (and   */
   /* the connection type will be filled into the input buffer) or a    */
   /* negative return error code if there was error.                    */
   /* * NOTE * This function can only be called on ports that are       */
   /*          currently connected.                                     */
int _SPPM_QueryConnectionType(unsigned int PortHandle, SPPM_Connection_Type_t *ConnectionType);

   /* The following function is provided to allow a mechanism to respond*/
   /* to an incoming MFi Open Session request.  This function accepts   */
   /* the Session ID to accept/reject and a BOOLEAN value that specifies*/
   /* whether or not the specified Session is to be accected (TRUE) or  */
   /* rejected (FALSE).  This function returns zero if successful or a  */
   /* negative return error code if there was an error.                 */
int _SPPM_OpenSessionRequestResponse(unsigned int PortHandle, Word_t SessionID, Boolean_t Accept);

   /* The following function is provided to allow a mechanism to send   */
   /* preformatted session data packets to a currently connected session*/
   /* (based on the Session ID).  This function accepts the length of   */
   /* data to send followed by the actual packet data that is to be     */
   /* sent.  This function returns a positive, non-zero, value if       */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * A successful return value from this function represents  */
   /*          the Packet ID of the packet that can be used to track the*/
   /*          confirmation status (as well as the ability to cancel the*/
   /*          queued packet via the SPPM_CancelPacket() function.      */
int _SPPM_SendSessionData(unsigned int PortHandle, Word_t SessionID, Word_t SessionDataLength, unsigned char *SessionDataBuffer);

   /* The following function is provided to allow a mechanism to send   */
   /* preformatted non-session data packets to a currently MFi device.  */
   /* This function accepts the Lingo ID, Command ID, and the           */
   /* Transaction ID of the data packet to send, followed by the length */
   /* of data to send followed by the actual packet data that is to be  */
   /* sent.  This function returns a positive, non-zero, value if       */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * Non Session data is considered to be any protocol data   */
   /*          that is represented by one of the defined Lingos (note   */
   /*          this also includes the General Lingo).                   */
   /* * NOTE * A successful return value from this function represents  */
   /*          the Packet ID of the packet that can be used to track the*/
   /*          confirmation status (as well as the ability to cancel the*/
   /*          queued packet via the SPPM_CancelPacket() function.      */
int _SPPM_SendNonSessionData(unsigned int PortHandle, Byte_t Lingo, Byte_t CommandID, Word_t TransactionID, Word_t DataLength, unsigned char *DataBuffer);

	/* The following function is provided to allow a mechanism to send   */
	/* preformatted iAP2 control message packet to a currently MFi       */
	/* device. This function accepts the Port ID, control message ID,    */
	/* control message length and the data, and the           			 */
	/* This function returns a positive, non-zero, value if       		 */
	/* successful, or a negative return error code if there was an error.*/
	/* * NOTE * A successful return value from this function represents  */
	/*          the Packet ID of the packet that can be used to track the*/
	/*          confirmation status (as well as the ability to cancel the*/
	/*          queued packet via the SPPM_CancelPacket() function.      */
int _SPPM_Send_Control_Message(unsigned int PortID, Word_t ControlMessageID, Word_t DataLength, unsigned char *DataBuffer);

   /* The following function is provided to allow a mechanism to cancel */
   /* a previously queued packet that was queued via one of the         */
   /* following functions:                                              */
   /*    SPPM_SendSessionData()                                         */
   /*    SPPM_SendNonSessionData()                                      */
   /* This function accepts the Packet ID of the packet to cancel.  The */
   /* Packet ID is the same Packet ID that was returned from one of the */
   /* above mentioned functions.  This function returns zero if         */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * This function can only cancel packets that have been     */
   /*          queued for transmit, not packets that have already been  */
   /*          transmitted.                                             */
int _SPPM_CancelPacket(unsigned int PortHandle, unsigned int PacketID);

#endif
