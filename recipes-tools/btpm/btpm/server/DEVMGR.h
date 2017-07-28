/*****< devmgr.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright (C) 2016 Texas Instruments Incorporated -  	              */
/*      http://www.ti.com/ All Rights Reserved.  							  */
/*                                                                            */
/*  DEVMGR - Local Device Manager Interface Implementation for Stonestreet    */
/*           One Bluetooth Protocol Stack Platform Manager.                   */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/01/10  D. Lange       Initial creation.                               */
/*   04/19/16  L. Gersi       Adding support for LE SC pairing.               */
/*   07/25/16  D. Horowitz    Adding Link Layer Topology feature.			  */
/*   07/25/16  D. Horowitz    Adding Low duty cycle feature.			      */
/*   07/25/16  D. Horowitz    Adding Ping feature.			      			  */
/******************************************************************************/
#ifndef __DEVMGRH__
#define __DEVMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Local Device Manager Implementation.  This function*/
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager    */
   /* Local Device Manager Implementation.                              */
int _DEVM_Initialize(DEVM_Initialization_Data_t *InitializationInfo);

   /* The following function is responsible for shutting down the Local */
   /* Device Manager Implementation.  After this function is called the */
   /* Local Device Manager Implementation will no longer operate until  */
   /* it is initialized again via a call to the _DEVM_Initialize()      */
   /* function.                                                         */
void _DEVM_Cleanup(void);

   /* The following function is the function that the Local Device      */
   /* Manager calls when the an event that has been previously queued   */
   /* into the Local Device Manager is being dispatched.  This function */
   /* passes a BOOLEAN flag which says whether or not the callee is     */
   /* process the Event (i.e.  the Local Device Manager is still        */
   /* initialized) or not.  If the first parameter is TRUE then the     */
   /* Local Device Manager Implementation should process the event      */
   /* normally.  If this parameter is FALSE then the Local Device       */
   /* Manager Implementation should NOT process the event and simply    */
   /* free any resources that are contained in the event (if any).  The */
   /* second parameter specifies the a pointer to the Event Data that   */
   /* was queued when the Event was queued into the Local Device        */
   /* Manager.                                                          */
void _DEVM_ProcessEvent(Boolean_t Process, void *EventData);

   /* The following function is the function that the Local Device      */
   /* Manager calls when it detects that a Client has un-registered from*/
   /* the system.  This function is called to allow the Device Manager  */
   /* implementation to clean up any resources that might be            */
   /* specifically associated with the specified Client ID.             */
void _DEVM_ProcessClientUnRegister(unsigned int ClientID);

   /* The following function is responsible for actually Powering up the*/
   /* Bluetooth Device and initializing the Stack.  This function       */
   /* accepts as input a pointer to a variable that is to receive the   */
   /* Bluetooth Stack ID of the stack (if initialized successfully).    */
   /* This function returns zero if the stack was initialized           */
   /* successfully, or a negative return error code if there was an     */
   /* error.                                                            */
int _DEVM_PowerUpDevice(unsigned int *BluetoothStackID);

   /* The following function is responsible for actually Powering down  */
   /* the Bluetooth Device (device stack).                              */
void _DEVM_PowerDownDevice(void);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine the current Local Device Properties of the   */
   /* Local Device.  This function returns zero if successful, or a     */
   /* negative return error code if there was an error.                 */
int _DEVM_QueryLocalDeviceProperties(DEVM_Local_Device_Properties_t *LocalDeviceProperties);

   /* The following function is provided to allow a mechanism for local */
   /* modules to update the current Local Device Properties of the Local*/
   /* Device.  This function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
int _DEVM_UpdateLocalDeviceProperties(unsigned long UpdateMemberFlag, DEVM_Local_Device_Properties_t *LocalDeviceProperties, unsigned long *ChangedMemberMask);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine the current Local Device ID Information of   */
   /* the Local Device.  This function returns zero if successful, or a */
   /* negative return error code if there was an error.                 */
int _DEVM_QueryLocalDeviceIDInformation(DEVM_Device_ID_Information_t *LocalDeviceIDInformation);

   /* The following function function is a utility function which is    */
   /* used to enable the specified feature of the Local Device.  This   */
   /* function accepts as input the specified local device feature to   */
   /* disable.                                                          */
int _DEVM_EnableLocalDeviceFeature(unsigned long Feature);

   /* The following function is a utility function which is used to     */
   /* disable the specified feature of the Local Device.  This function */
   /* accepts as input the specified local device feature to disable.   */
   /* * NOTE * Attempting to disable LE will fail if there are any      */
   /*          active LE connections or if a connection establishment   */
   /*          process for LE is outstanding.                           */
int _DEVM_DisableLocalDeviceFeature(unsigned long Feature);

   /* The following function is a utility function which is used to     */
   /* return the currently active Local Device Features.  This function */
   /* accepts as input a pointer to return the active local device      */
   /* features.                                                         */
int _DEVM_QueryActiveLocalDeviceFeatures(unsigned long *ActiveFeatures);

   /* The following function is provided to allow a mechanism for local */
   /* modules to begin a Device Discovery Process.  Device Discovery, in*/
   /* this context, means inquiry and name discovery.  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
int _DEVM_StartDeviceDiscovery(unsigned long DiscoveryDuration);

   /* The following function is provided to allow a mechanism for local */
   /* modules to stop a currently ongoing Device Discovery Process.     */
   /* Device Discovery, in this context, means inquiry and name         */
   /* discovery.  This function returns zero if successful, or a        */
   /* negative return error code if there was an error.                 */
int _DEVM_StopDeviceDiscovery(void);

   /* The following function is provided to allow a mechanism for local */
   /* modules to begin a Device Scanning Process.  Device Scanning, in  */
   /* this context, means Bluetooth Low Energy Active Scanning.  This   */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _DEVM_StartDeviceScan(unsigned long ScanDuration);

   /* The following function is provided to allow a mechanism for local */
   /* modules to stop a currently ongoing Device Scanning Process.      */
   /* Device Scanning, in this context, means Bluetooth Low Energy      */
   /* Active Scanning.  This function returns zero if successful, or a  */
   /* negative return error code if there was an error.                 */
int _DEVM_StopDeviceScan(void);

   /* The following function is provided to allow a mechanism for local */
   /* modules to begin a Advertising process.  This function returns    */
   /* zero if successful, or a negative return error code if there was  */
   /* an error.                                                         */
int _DEVM_StartAdvertising(unsigned int ClientID, DEVM_Advertising_Information_t *AdvertisingInformation);

   /* The following function is provided to allow a mechanism for local */
   /* modules to stop a currently ongoing Advertising Process.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _DEVM_StopAdvertising(unsigned int ClientID, unsigned long AdvertisingFlags);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the current List of Remote Devices that the      */
   /* Bluetopia Platform Manager services currently knows about. The    */
   /* return value from this function will be the number of addresses   */
   /* that that were copied into the specified buffer (if specified),   */
   /* zero signifies success, but no bytes copied.  A negative value    */
   /* represents a return error code if there was an error.             */
int _DEVM_QueryRemoteDeviceList(unsigned int RemoteDeviceFilter, Class_of_Device_t ClassOfDeviceFilter, unsigned int MaximumNumberDevices, BD_ADDR_t *RemoteDeviceList, unsigned int *TotalNumberDevices);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the device properties of a specific Remote Device*/
   /* (based upon the specified Bluetooth Device Address).  This        */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _DEVM_QueryRemoteDeviceProperties(BD_ADDR_t RemoteDevice, unsigned long QueryFlags, unsigned int AsynchronousID, DEVM_Remote_Device_Properties_t *RemoteDeviceProperties);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the BR/EDR Extended Inquiry Response (EIR) data  */
   /* of a specific Remote Device (based upon the specified Bluetooth   */
   /* Device Address).  This function returns zero if there is no known */
   /* EIR Data for the device, a positive value if there is known EIR   */
   /* data for the device (and it will populated in the supplied        */
   /* buffer), or a negative return error code if there was an error.   */
int _DEVM_QueryRemoteDeviceEIRData(BD_ADDR_t RemoteDevice, Extended_Inquiry_Response_Data_t *EIRData);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the Bluetooth Low Energy Advertising and Scan    */
   /* Response data of a specific Remote Device (based upon the         */
   /* specified Bluetooth Device Address).  This function returns the   */
   /* number of Advertising Reports that were populated (zero, one or   */
   /* two) if successful, or a negative return error code if there was  */
   /* an error.                                                         */
   /* * NOTE * If this function returns zero it means that no           */
   /*          Advertising or Scan Response data is know for the device.*/
   /*          If this function returns one, then it means that         */
   /*          advertising data is known (and is returned in the        */
   /*          supplied Advertising Data buffer).  If this function     */
   /*          returns two it means that both Advertising and Scan      */
   /*          Response data is known (and are returned in the          */
   /*          respective buffers).                                     */
int _DEVM_QueryRemoteDeviceAdvertisingData(BD_ADDR_t RemoteDevice, Advertising_Data_t *AdvertisingData, Advertising_Data_t *ScanResponseData);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the services provided by a specific Remote Device*/
   /* (based upon the specified Bluetooth Device Address).  The value   */
   /* from this function will be the number of bytes that were copied   */
   /* into the specified buffer (if specified), zero signifies success, */
   /* but no bytes copied.  A negative value represents a return error  */
   /* code if there was an error.                                       */
int _DEVM_QueryRemoteDeviceServices(BD_ADDR_t RemoteDevice, unsigned long QueryFlags, unsigned int AsynchronousID, unsigned int ServiceDataBufferSize, unsigned char *ServiceDataBuffer, unsigned int *TotalServiceDataLength);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query whether a particular service is supported by a   */
   /* specific Remote Device (based upon the specified Bluetooth Device */
   /* Address). The return value from this function will a positive     */
   /* non-zero value if the device claims to support the service or zero*/
   /* if the device does NOT support the given service. A negative value*/
   /* represents a return error code if there was an error.             */
   /* * NOTE * This function uses the locally cached copy of SDP record */
   /*          information. If the service records for the given device */
   /*          are not known, this function will return an error. The   */
   /*          caller can check whether services are known using the    */
   /*          DEVM_QueryRemoteDeviceProperties() API and, if necessary,*/
   /*          can update the cached service records for a device using */
   /*          the DEVM_QueryRemoteDeviceServices() API.                */
int _DEVM_QueryRemoteDeviceServiceSupported(BD_ADDR_t RemoteDevice, SDP_UUID_Entry_t ServiceUUID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query which devices advertise support a specific       */
   /* service. specific Remote Device (based upon the specified         */
   /* Bluetooth Device Address). The return value from this function    */
   /* will be the number of Device Addresses copied into the buffer (if */
   /* specified), where zero signifies success but no Device Addresses  */
   /* copied. A negative value represents a return error code if there  */
   /* was an error.                                                     */
   /* * NOTE * This function uses the locally cached copy of SDP record */
   /*          information. If the service records for the given device */
   /*          are not known, that device will not be included in the   */
   /*          results. The caller can check whether services are known */
   /*          using the DEVM_QueryRemoteDeviceProperties() API and, if */
   /*          necessary, can update the cached service records for a   */
   /*          device using the DEVM_QueryRemoteDeviceServices() API.   */
int _DEVM_QueryRemoteDevicesForService(SDP_UUID_Entry_t ServiceUUID, unsigned int MaximumNumberDevices, BD_ADDR_t *RemoteDeviceList, unsigned int *TotalNumberDevices);

   /* The following function is provided to allow a mechanism for       */
   /* local modules to query the Service Classes advertised by a        */
   /* specific Remote Device (based upon the specified Bluetooth Device */
   /* Address). The return value from this function will be the number  */
   /* of Service Classes copied into the buffer (if specified), where   */
   /* zero signifies success but no Service Classes copied. A negative  */
   /* value represents a return error code if there was an error.       */
   /* * NOTE * This function uses the locally cached copy of SDP record */
   /*          information. If the service records for the given device */
   /*          are not known, this function will return an error. The   */
   /*          caller can check whether services are known using the    */
   /*          DEVM_QueryRemoteDeviceProperties() API and, if necessary,*/
   /*          can update the cached service records for a device using */
   /*          the DEVM_QueryRemoteDeviceServices() API.                */
int _DEVM_QueryRemoteDeviceServiceClasses(BD_ADDR_t RemoteDevice, unsigned int MaximumNumberServiceClasses, SDP_UUID_Entry_t *ServiceClassList, unsigned int *TotalNumberServiceClasses);

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually add a specific remote device entry into the   */
   /* list of remote devices that the Bluetopia Platform Manager        */
   /* Services are currently maintaining.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * To specify no Application Data pass NULL for the         */
   /*          Application Data parameter.                              */
int _DEVM_AddRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long AddDeviceFlags, Class_of_Device_t ClassOfDevice, DEVM_Remote_Device_Application_Data_t *ApplicationData);

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually remove a specific remote device entry from the*/
   /* list of remote devices that the Bluetopia Platform Manager        */
   /* Services are currently maintaining.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
int _DEVM_DeleteRemoteDevice(BD_ADDR_t RemoteDevice);

   /* The following function is provided to allow a mechanism for local */
   /* modules to set the Application Data for a specific Remote Device. */
   /* This data will be stored along with the Remote Device Information */
   /* in the list of Remote Devices that the Bluetopia Platform Manager */
   /* Services are currently maintaining.  This function returns zero if*/
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * To specify no Application Data pass NULL for the         */
   /*          ApplicationData parameter.                               */
int _DEVM_UpdateRemoteDeviceApplicationData(BD_ADDR_t RemoteDevice, DEVM_Remote_Device_Application_Data_t *ApplicationData);

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually remove a specific group of remote devices from*/
   /* the list of remote devices that the Bluetopia Platform Manager    */
   /* Services are currently maintaining.  The final two parameters are */
   /* optional and, if specified, specify the size (and a buffer) that  */
   /* is to hold all of the devices that were deleted.  This function   */
   /* returns a non-negative number that represents the number of       */
   /* devices that were deleted (and copied into the list - up to the   */
   /* specified maximum - if specified) if successful, or a negative    */
   /* return error code if there was an error.                          */
int _DEVM_DeleteRemoteDevices(unsigned int DeleteDevicesFilter, unsigned int NumberDeletedDevices, BD_ADDR_t *DeletedDevices);

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually begin a pairing process with the specified    */
   /* remote device.  This function returns zero if successful, or a    */
   /* negative return error code if there was an error.                 */
   /* * NOTE * The pairing process itself is asychronous.  The caller   */
   /*          can determine the status of the Pairing procedure by     */
   /*          waiting for the detRemoteDevicePairingStatus event to    */
   /*          be dispatched.                                           */
   /* * NOTE * Bluetooth Low Energy pairing can only be accomplished    */
   /*          if the device is already connected (i.e. this function   */
   /*          will not attempt to connect to the device for pairing).  */
int _DEVM_PairWithRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long PairFlags, unsigned int AsynchronousID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually cancel an on-going pairing process with the   */
   /* specified remote device.  This function returns zero if           */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * The pairing process itself is asychronous.  The caller   */
   /*          can determine the status of the Pairing procedure by     */
   /*          waiting for the detRemoteDevicePairingStatus event to be */
   /*          dispatched.                                              */
int _DEVM_CancelPairWithRemoteDevice(BD_ADDR_t RemoteDevice);

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually remove all stored pairing information with the*/
   /* specified remote device.  This function returns zero if           */
   /* successful, or a negative return error code if there was an error.*/
int _DEVM_UnPairRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long UnPairFlags);

   /* The following function is provided to allow a mechanism for local */
   /* modules to enable/disable SC Only mode.                           */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _DEVM_EnableSCOnly(Boolean_t EnableSCOnly);

   /* The following function is provided to allow a mechanism for local */
   /* modules to generate new local public and private P256 keys.       */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _DEVM_RegenerateP256LocalKeys(void);

   /* The following function is provided to allow a mechanism for local */
   /* modules to generate OOB local parameters. Used in LE SC pairing.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _DEVM_SC_OOB_Generate_Parameters(SM_Random_Value_t *OOB_Local_Rand_Result, SM_Confirm_Value_t *OOB_Local_Confirm_Result); 

   /* The following function is provided to allow a mechanism for local */
   /* modules to send keyprss notifications. Used in LE SC pairing.  	*/
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _DEVM_SC_Send_Keypress_Notification(BD_ADDR_t BD_ADDR, GAP_LE_Keypress_t Keypress_Notification_Type);
   
   /* The following function is provided to allow a mechanism for local */
   /* modules to actually Authenticate with a remote device (the device */
   /* *MUST* already be connected - i.e. this function will not make a  */
   /* connection and then attempt to authenticate the device).  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _DEVM_AuthenticateRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long AuthenticateFlags, unsigned int AsynchronousID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually force Encryption with a remote device (the    */
   /* device *MUST* already be connected - i.e. this function will not  */
   /* make a connection and then attempt to encrypt the device).  This  */
   /* function function returns zero if successful, or a negative return*/
   /* error code if there was an error.                                 */
   /* * NOTE * Bluetooth Encryption *REQUIRES* that a Link Key be       */
   /*          established between two devices before the link can be   */
   /*          encrypted.  Because of this, the caller *MUST* issue this*/
   /*          function call *AFTER* the link has been authenticated    */
   /*          (either locally or remotely).                            */
int _DEVM_EncryptRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long EncryptFlags, unsigned int AsynchronousID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually connect with a remote device (and optionally  */
   /* specify connection parameters (authentication and encryption).    */
   /* This function function returns zero if successful, or a negative  */
   /* return error code if there was an error.                          */
   /* * NOTE * This function is envisioned to be used to force an       */
   /*          authentication on an outgoing link before a profile is   */
   /*          connected.                                               */
int _DEVM_ConnectWithRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long ConnectFlags, unsigned int AsynchronousID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to actually disconnect a remote device.  This function    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
   /* * NOTE * This function should be used sparingly because it will   */
   /*          force all active service connections to be closed in     */
   /*          addition to forcing a disconnection of the device.       */
int _DEVM_DisconnectRemoteDevice(BD_ADDR_t RemoteDevice, unsigned long DisconnectFlags);

   /* The following function is a utility function which is used to     */
   /* query the local GATT sign counter and signing key from the Remote */
   /* Device Database.                                                  */
Boolean_t _DEVM_QueryLocalSigningKey(BD_ADDR_t RemoteDevice, DWord_t *SignCounter, Encryption_Key_t *CSRK);

   /* The following function is a utility function which is used to     */
   /* query the remote GATT sign counter and signing key from the Remote*/
   /* Device Database.                                                  */
Boolean_t _DEVM_QueryRemoteSigningKey(BD_ADDR_t RemoteDevice, DWord_t *SignCounter, Encryption_Key_t *CSRK);

   /* The following function is a utility function which is used to     */
   /* increment the Local GATT Sign Counter in the Remote Device        */
   /* Database.                                                         */
Boolean_t _DEVM_IncrementLocalSignCounter(BD_ADDR_t RemoteDevice);

   /* The following function is a utility function which is used to     */
   /* increment the Remote GATT Sign Counter in the Remote Device       */
   /* Database.                                                         */
Boolean_t _DEVM_IncrementRemoteSignCounter(BD_ADDR_t RemoteDevice);

   /* The following function is provided to allow a mechanism for the   */
   /* local module to force a connection into sniff mode.  This function*/
   /* returns zero if the request was submitted, a positive value if the*/
   /* link is already in sniff mode, or a negative return error code if */
   /* there was an error.                                               */
int _DEVM_SetRemoteDeviceLinkSniff(BD_ADDR_t RemoteDevice, unsigned int SniffMaxInterval, unsigned int SniffMinInterval, unsigned int SniffAttempt, unsigned int SniffTimeout);

   /* The following function is provided to allow a mechanism for local */
   /* modules to force the current link mode/state to active.  This     */
   /* means that if the current device is in:                           */
   /*    - Park Mode                                                    */
   /*    - Hold Mode                                                    */
   /*    - Sniff Mode                                                   */
   /* then this function will attempt to change the current mode/state  */
   /* to the active mode.  This function returns zero if the request    */
   /* was submitted, a positive value if the link is already in active  */
   /* mode, or a negative return error code if there was an error.      */
int _DEVM_SetRemoteDeviceLinkActive(BD_ADDR_t RemoteDevice);

   /* The following function is provided to allow a mechanism for local */
   /* modules to create an SDP Service Record in the Local Devices SDP  */
   /* Database.  This function returns a positive value if successful or*/
   /* a negative return error code if there was an error.               */
   /* * NOTE * If this function returns success then the return value   */
   /*          represents the actual Service Record Handle of the       */
   /*          created Service Record.                                  */
   /* * NOTE * Upon Client De-registration all Registered SDP Records   */
   /*          (registered by the client) are deleted from the SDP      */
   /*          Database if the PersistClient parameters is FALSE.  If   */
   /*          PersistClient is TRUE the record will stay in the SDP    */
   /*          database until it is either explicitly deleted OR the    */
   /*          device is powered down.                                  */
long _DEVM_CreateServiceRecord(unsigned int ClientID, Boolean_t PersistClient, unsigned int NumberServiceClassUUID, SDP_UUID_Entry_t SDP_UUID_Entry[]);

   /* The following function is provided to allow a mechanism for local */
   /* modules to delete a previously registered SDP Service Record from */
   /* the Local Devices SDP Database.  This function returns zero if    */
   /* successful or a negative return error code if there was an error. */
   /* * NOTE * Upon Client De-registration all Registered SDP Records   */
   /*          (registered by the client) are deleted from the SDP      */
   /*          Database.                                                */
int _DEVM_DeleteServiceRecord(unsigned int ClientID, unsigned long ServiceRecordHandle);

   /* The following function is provided to allow a mechanism for local */
   /* modules to add a Service Record Attribute to a Service Record in  */
   /* the Local Devices SDP Database.  This function returns a zero if  */
   /* successful or a negative return error code if there was an error. */
int _DEVM_AddServiceRecordAttribute(unsigned int ClientID, unsigned long ServiceRecordHandle, unsigned int AttributeID, SDP_Data_Element_t *SDP_Data_Element);

   /* The following function is provided to allow a mechanism for local */
   /* modules to delete a Service Record Attribute from a Service Record*/
   /* in the Local Devices SDP Database.  This function returns a zero  */
   /* if successful or a negative return error code if there was an     */
   /* error.                                                            */
int _DEVM_DeleteServiceRecordAttribute(unsigned int ClientID, unsigned long ServiceRecordHandle, unsigned int AttributeID);

   /* The following function is a utility function that allows the      */
   /* caller to instruct the Local Device Manager to enable/disable     */
   /* Bluetooth Debugging.  This function returns zero if successful, or*/
   /* a negative return error code if there was an error.               */
int _DEVM_EnableBluetoothDebug(Boolean_t Enable, unsigned int DebugType, unsigned long DebugFlags, unsigned int DebugParameterLength, unsigned char *DebugParameter);

   /* The following function is provided to allow a mechanism for the   */
   /* local module to register an Authentication Handler with the Local */
   /* Device Manager.  If successful, the registered client will be     */
   /* notified whenever an Authentication Request needs to be serviced. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * There can ONLY be a SINGLE Registered Authentication     */
   /*          Handler in the entire system at any given time.          */
int _DEVM_RegisterAuthentication(void);

   /* The following function is provided to allow a mechanism for the   */
   /* local module to Un-Register a previously registered Authentication*/
   /* Handler.                                                          */
void _DEVM_UnRegisterAuthentication(void);

   /* The following function is provided to allow a mechanism for the   */
   /* local module that is processing Authentication Requests to respond*/
   /* to an Authentication Request.  This function accepts as input the */
   /* Authentication Response Information to respond with.              */
int _DEVM_AuthenticationResponse(unsigned int ClientID, DEVM_Authentication_Information_t *AuthenticationInformation);

   /* The following function is provided to allow a mechanism for local */
   /* modules to send raw HCI commands to the Bluetooth Device. This    */
   /* function accepts as input the Bluetooth Command OGF and OCF, data */
   /* (with length) to send with the command, and optional buffers in   */
   /* which to store command results. The final parameter indicates     */
   /* whether the function should wait for a response from the Bluetooth*/
   /* Device. If the caller knows that the Device does NOT return data  */
   /* for the given command, this parameter can be set to FALSE and the */
   /* function will return immediately after sending the command. If    */
   /* specified, LengthResult should indicate the maximum amount of data*/
   /* to be stored in BufferResult. Otherwise, the function will block  */
   /* until either a response is received or a timeout expires. This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error.                                       */
int _DEVM_SendRawHCICommand(Byte_t OGF, Word_t OCF, Byte_t CommandLength, Byte_t CommandData[], Byte_t *ResultStatus, Byte_t *ResultLength, Byte_t *ResultBuffer, Boolean_t WaitForResponse);

   /* The following function is provided to allow a mechanism for local */
   /* modules to query the connection handle of an ACL connection.  This*/
   /* function accepts as input the BD_ADDR of the remote device, the   */
   /* second parameter specifies where the returned connection handle   */
   /* will be stored.  This function returns zero if successful, or a   */
   /* negative return error code if there was an error.                 */
int _DEVM_QueryConnectionHandle(BD_ADDR_t RemoteDeviceAddress, Word_t *ConnectionHandle);

   /* The following function is provided to allow a mechanism for local */
   /* modules to send Set Authenticated Payload Timeout. Used in order 	*/
   /* to change the Authenticated payload timeout of the ping. The ping */
   /* will start working automatically after finishing the pairing      */
   /* procedure. This function returns zero if successful, or a negative*/
   /* return error code if there was an error.                          */
int _DEVM_Set_Authenticated_Payload_Timeout(BD_ADDR_t BD_ADDR, Word_t AuthenticatedPayloadTimeout);

   /* The following function is provided to allow a mechanism for local */
   /* modules to send Query Authenticated Payload Timeout. Used in order*/
   /* to query the Authenticated payload timeout of the ping.           */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _DEVM_Query_Authenticated_Payload_Timeout(BD_ADDR_t BD_ADDR, Word_t *AuthenticatedPayloadTimeout);

   /* The following function is provided to allow a mechanism for local */
   /* modules to send Set advertising intervals. Used in order to Set   */
   /* advertising intervals.  This function returns zero if successful, */
   /* or a negative return error code if there was an error.            */
int _DEVM_Set_Advertising_Intervals(Word_t Advertising_Interval_Min, Word_t Advertising_Interval_Max);   

   /* The following function is provided to allow a mechanism for local */
   /* modules to send Set and update connection parameters. Used in     */
   /* order to Set and update the connection parameters and the default */
   /* connection parameters.  This function returns zero if successful, */
   /* or a negative return error code if there was an error.            */
int _DEVM_Set_And_Update_Connection_And_Scan_Parameters(BD_ADDR_t *BD_ADDR, AddressType_t *AddressType, GAP_LE_Connection_Parameters_t *ConnectionParameters, Word_t *ConnectionScanInterval, Word_t *ConnectionScanWindow, Word_t *ScanInterval, Word_t *ScanWindow);


#endif
