/*****< devmgri.h >************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright (C) 2016 Texas Instruments Incorporated -  	              */
/*      http://www.ti.com/ All Rights Reserved.  							  */
/*                                                                            */
/*  DEVMGRI - Internal Local Device Manager Interface Implementation for      */
/*            Stonestreet One Bluetooth Protocol Stack Platform Manager.      */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/21/10  D. Lange       Initial creation.                               */
/*   04/19/16  L. Gersi       Adding support for LE SC pairing.               */
/*   07/25/16  D. Horowitz    Adding Link Layer Topology feature.			  */
/*   07/25/16  D. Horowitz    Adding Low duty cycle feature.			      */
/*   07/25/16  D. Horowitz    Adding Bonding and Non Bonding options.		  */
/******************************************************************************/
#ifndef __DEVMGRIH__
#define __DEVMGRIH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */
#include "BTPSKRNL.h"            /* BTPS Kernel Prototypes/Constants.         */

#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "BTPMCFG.h"             /* BTPM Configuration Settings/Constants.    */

#include "BTPMDEVM.h"            /* BTPM Device Manager Prototypes/Constants. */
#include "DEVMGR.h"              /* Device Manager Impl. Prototypes/Constants.*/

   /* The following enumerated type represents the various asynchronous */
   /* operation types that are tracked by this module.                  */
typedef enum
{
   atPairing,
   atServiceDiscovery,
   atPairingServiceDiscovery,
   atDeviceName,
   atAuthenticating,
   atEncryptingAuthenticating,
   atEncrypting,
   atConnecting,
   atConnectingAuthenticating,
   atConnectingEncrypting,
   atDeviceUpdate,
} Asynchronous_Operation_Type_t;

   /* The following structure contains all of the information that is   */
   /* required to perform and track an Asynchronous operation.          */
typedef struct _tagAsynchronous_Operation_t
{
   Asynchronous_Operation_Type_t        AsynchronousOperationType;
   BD_ADDR_t                            BD_ADDR;
   Boolean_t                            Submitted;
   Boolean_t                            ConnectionRequest;
   Boolean_t                            OriginatedDuringPairing;
   Boolean_t                            IOCapsValid;
   GAP_LE_Pairing_Capabilities_t        IOCaps;
   unsigned int                         AsynchronousID;
   unsigned int                         SDPCallbackID;
   unsigned long                        ConnectFlags;
   unsigned long                        PairFlags;
   unsigned int                         ConnectionCount;
   unsigned int                         TimerID;
   struct _tagAsynchronous_Operation_t *NextAsynchronousOperationPtr;
} Asynchronous_Operation_t;

   /* The following structure contains all information that is required */
   /* to track an SDP Record that is associated with a specific Client. */
typedef struct _tagClient_SDP_Info_t
{
   unsigned int                  ClientID;
   unsigned long                 ServiceRecordHandle;
   Boolean_t                     PersistClient;
   struct _tagClient_SDP_Info_t *NextClientSDPInfoPtr;
} Client_SDP_Info_t;

   /* The following structure contains all of the information that is   */
   /* required to perform and track an advertising process.             */
typedef struct _tagAdvertising_Info_t
{
   unsigned int                    ClientID;
   unsigned int                    TimerID;
   DEVM_Advertising_Information_t  AdvertisingInformation;
   struct _tagAdvertising_Info_t  *NextAdvertisingInfoPtr;
} Advertising_Info_t;

   /* The following structure is used to hold information about a remote*/
   /* LE device that has been discovered via an LE Scan Process.        */
typedef struct _tagScanning_Entry_t
{
   GAP_LE_Address_Type_t        AddressType;
   BD_ADDR_t                    RemoteDevice;
   unsigned long                Flags;
   int                          RSSI;
   Word_t                       AdvertisingCRC;
   Word_t                       ScanResponseCRC;
   struct _tagScanning_Entry_t *NextScanningEntryPtr;
} Scanning_Entry_t;

#define SCANNING_ENTRY_FLAGS_ADVERTISING_CRC_VALID       0x00000001
#define SCANNING_ENTRY_FLAGS_SCAN_RESP_CRC_VALID         0x00000002

   /* The following structure contains all of the information stored on */
   /* a current advertising process.                                    */
typedef struct _tagAdvertising_Process_t
{
   unsigned long        CurrentAdvertisingFlags;
   unsigned int         AdvertisingDataLength;
   Advertising_Data_t   AdvertisingData;
   BD_ADDR_t            Peer_BD_ADDR;
   unsigned int         ScanResponseDataLength;
   Scan_Response_Data_t ScanResponseData;
} Advertising_Process_t;

   /* The following structure defines the ALL relevant information about*/
   /* the Local Bluetooth Device.                                       */
typedef struct _tagLocal_Device_Info_t
{
   BD_ADDR_t                           BD_ADDR;
   unsigned int                        DeviceNameLength;
   char                                DeviceName[MAX_NAME_LENGTH + 1];
   Class_of_Device_t                   ClassOfDevice;
   HCI_Version_t                       HCIVersion_t;
   unsigned int                        HCIVersion;
   unsigned int                        HCIRevision;
   unsigned int                        LMPVersion;
   unsigned int                        LMPSubVersion;
   unsigned int                        DeviceManufacturer;
   unsigned long                       DeviceFlags;
   unsigned int                        DiscoveryDurationTimerID;
   GAP_Discoverability_Mode_t          DiscoverabilityMode;
   unsigned int                        DiscoverableTimeout;
   unsigned int                        DiscoverableModeTimerID;
   GAP_Connectability_Mode_t           ConnectabilityMode;
   unsigned int                        ConnectableTimeout;
   unsigned int                        ConnectableModeTimerID;
   GAP_Pairability_Mode_t              PairabilityMode;
   unsigned int                        PairableTimeout;
   unsigned int                        PairableModeTimerID;
   Word_t                              DeviceAppearance;
   unsigned int                        RandomAddressUseCount;
   unsigned int                        DefaultRandomAddressUseCount;
   unsigned int                        ScanningDurationTimerID;
   unsigned int                        ScanningTimeout;
   unsigned int                        ConnectionDurationTimerID;
   unsigned int                        ConnectionTimeout;
   Word_t                              ConnectionScanInterval;
   Word_t                              ConnectionScanWindow;
   Word_t                              ScanInterval;
   Word_t                              ScanWindow;
   Word_t                              AdvertisingIntervalMin;
   Word_t                              AdvertisingIntervalMax;
   BD_ADDR_t                           CurrentResolvableBD_ADDR;
   GAP_LE_Connection_Parameters_t      DefaultConnectionParameters;
   Encryption_Key_t                    LocalIRK;
   Advertising_Process_t               AdvertisingProcess;
   Boolean_t                           UseSCOEnhancedTransportParameters;
   SCO_Enhanced_Transport_Parameters_t SCOEnhancedTransportParameters;
} Local_Device_Info_t;

#define LOCAL_DEVICE_INFO_SIZE                              (sizeof(Local_Device_Info_t))

   /* The following constants are used with the DeviceFlags member of   */
   /* the Local_Device_Info_t to denote information about the Local     */
   /* Device.                                                           */
#define LOCAL_DEVICE_FLAGS_PAIRING_CURRENTLY_ALLOWED        0x00000001
#define LOCAL_DEVICE_FLAGS_DISCOVERY_IN_PROGRESS            0x00000002
#define LOCAL_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY       0x00000004
#define LOCAL_DEVICE_FLAGS_LE_SCANNING_IN_PROGRESS          0x00000008
#define LOCAL_DEVICE_FLAGS_LE_ADVERTISING_IN_PROGRESS       0x00000010
#define LOCAL_DEVICE_FLAGS_LE_CONNECTION_IN_PROGRESS        0x00000040
#define LOCAL_DEVICE_FLAGS_LE_SCANNING_FOR_CONNECTION       0x00000080
#define LOCAL_DEVICE_FLAGS_DEVICE_SUPPORTS_ANT              0x00000100
#define LOCAL_DEVICE_FLAGS_LE_CURRENTLY_ENABLED             0x00000200
#define LOCAL_DEVICE_FLAGS_ANT_CURRENTLY_ENABLED            0x00000400

   /* The following structure defines ALL revelant information about a  */
   /* Remote LE Bluetooth Device.                                       */
typedef struct _tagRemote_LE_Device_Info_t
{
   unsigned long                    DeviceFlags;
   unsigned long                    SecurityOperationFlags;
   BD_ADDR_t                        LastResolvableBD_ADDR;
   unsigned int                     ConnectionCount;
   AddressType_t                    AddressType;
   Word_t                           ConnectionHandle;
   int                              TransmitPower;
   int                              RSSI;
   Word_t                           ServiceChangedHandle;
   Word_t                           DeviceAppearance;
   Advertising_Data_t               AdvertisingData;
   Advertising_Data_t               ScanResponseData;
   unsigned int                     ServiceDataLength;
   unsigned char                   *ServiceData;
   BD_ADDR_t                        ConnectionBD_ADDR;
   unsigned int                     DeviceInfoCount;
   GAP_LE_Encryption_Information_t  RemoteEncryptionInformation;
   GAP_LE_Encryption_Information_t  LocalEncryptionInformation;
   GAP_LE_Identity_Information_t    RemoteIdentityInformation;
   GAP_LE_Signing_Information_t     RemoteSigningInformation;
   DWord_t                          RemoteSignCounter;
   GAP_LE_Signing_Information_t     LocalSigningInformation;
   DWord_t                          LocalSignCounter;
   GAP_LE_Connection_Parameters_t   PreferredConnectionParameters;
   GAP_LE_Connection_Parameters_t   QueuedConnectionParameters;
} Remote_LE_Device_Info_t;

#define REMOTE_LE_DEVICE_INFO_SIZE                          (sizeof(Remote_LE_Device_Info_t))

   /* The following constants are used with the SecurityOperationFlags  */
   /* member of the Remote_LE_Device_Info_t to denote information about */
   /* ongoing security operations.                                      */
#define REMOTE_LE_DEVICE_SECURITY_FLAGS_PAIRING_IN_PROGRESS  0x00000001
#define REMOTE_LE_DEVICE_SECURITY_FLAGS_PAIRING_IN_PROG_MITM 0x00000002
#define REMOTE_LE_DEVICE_SECURITY_FLAGS_REESTABLISHING_SEC   0x00000004
#define REMOTE_LE_DEVICE_SECURITY_FLAGS_ENCRYPTION_IN_PROGR  0x00000008
#define REMOTE_LE_DEVICE_SECURITY_FLAGS_PAIR_REQ_RESP_PEND   0x00000010
#define REMOTE_LE_DEVICE_SECURITY_FLAGS_PAIRING_IN_PROG_SC 	 0x00000020
#define REMOTE_LE_DEVICE_SECURITY_FLAGS_BONDING_ALLOWED      0x00000040

   /* The following constants are used with the DeviceFlags member of   */
   /* the Remote_LE_Device_Info_t to denote information about the       */
   /* specified Remote LE Device.                                       */
#define REMOTE_LE_DEVICE_FLAGS_CURRENTLY_CONNECTED           0x00010000
#define REMOTE_LE_DEVICE_FLAGS_CURRENTLY_ENCRYPTED           0x00020000
#define REMOTE_LE_DEVICE_FLAGS_CURRENTLY_PAIRED              0x00040000
#define REMOTE_LE_DEVICE_FLAGS_PAIRED_WITH_MITM              0x00080000
#define REMOTE_LE_DEVICE_FLAGS_RETRIEVING_SERVICES           0x00100000
#define REMOTE_LE_DEVICE_FLAGS_LOCAL_DEVICE_INITIATED        0x00200000 /* When This flag is set, that means that */
                                                                        /* the local device is Master and the     */
                                                                        /* remote device is Slave.                */
#define REMOTE_LE_DEVICE_FLAGS_UPDATING_DEVICE_INFORMATION   0x00400000
#define REMOTE_LE_DEVICE_FLAGS_UPDATING_DEVICE_LOCK          0x00800000
#define REMOTE_LE_DEVICE_FLAGS_REPORTED_IN_CURRENT_SCAN      0x01000000
#define REMOTE_LE_DEVICE_FLAGS_CONNECTION_UPDATE_QUEUED      0x02000000
#define REMOTE_LE_DEVICE_FLAGS_REMOTE_CONN_PARAMS_IN_USE     0x04000000
#define REMOTE_LE_DEVICE_FLAGS_CONNECTION_UPDATE_IN_PROGRESS 0x08000000
#define REMOTE_LE_DEVICE_FLAGS_CONNECTION_UPDATE_RMT_INIT    0x10000000
#define REMOTE_LE_DEVICE_FLAGS_PAIRED_WITH_SC              	 0x20000000
#define REMOTE_LE_DEVICE_FLAGS_NOT_FIRST_ENCRYPTION          0x40000000

#define REMOTE_LE_DEVICE_FLAGS_LAST_RESOLVABLE_VALID         0x00000001
#define REMOTE_LE_DEVICE_FLAGS_PREFERRED_PARAMETERS_VALID    0x00000002
#define REMOTE_LE_DEVICE_FLAGS_REMOTE_ENCRYPTION_VALID       0x00000004
#define REMOTE_LE_DEVICE_FLAGS_LOCAL_ENCRYPTION_VALID        0x00000008
#define REMOTE_LE_DEVICE_FLAGS_REMOTE_IDENTITY_INFO_VALID    0x00000010
#define REMOTE_LE_DEVICE_FLAGS_REMOTE_SIGNING_VALID          0x00000020
#define REMOTE_LE_DEVICE_FLAGS_LOCAL_SIGNING_VALID           0x00000040
#define REMOTE_LE_DEVICE_FLAGS_RSSI_VALID                    0x00000080
#define REMOTE_LE_DEVICE_FLAGS_TRANSMIT_POWER_VALID          0x00000100
#define REMOTE_LE_DEVICE_FLAGS_ADVERTISING_DATA_VALID        0x00000200
#define REMOTE_LE_DEVICE_FLAGS_SCAN_RESPONSE_DATA_VALID      0x00000400
#define REMOTE_LE_DEVICE_FLAGS_DEVICE_APPEARANCE_VALID       0x00000800
#define REMOTE_LE_DEVICE_FLAGS_SERVICE_CHANGED_HANDLE_VALID  0x00001000
#define REMOTE_LE_DEVICE_FLAGS_SERVICE_INFORMATION_VALID     0x00002000

   /* The following structure defines the ALL relevant information about*/
   /* the a Remote Bluetooth Device.                                    */
typedef struct _tagRemote_Device_Info_t
{
   BD_ADDR_t                                 BD_ADDR;
   unsigned int                              DeviceNameLength;
   char                                      DeviceName[MAX_NAME_LENGTH + 1];
   Class_of_Device_t                         ClassOfDevice;
   unsigned long                             DeviceFlags;
   Byte_t                                    PageScanRepetitionMode;
   Byte_t                                    PageScanPeriodMode;
   Byte_t                                    PageScanMode;
   Word_t                                    ClockOffset;
   int                                       RSSI;
   int                                       TransmitPower;
   unsigned int                              SniffInterval;
   Word_t                                    ConnectionHandle;
   unsigned int                              ServiceDataLength;
   unsigned char                            *ServiceData;
   GAP_IO_Capabilities_t                     IOCapabilities;
   GAP_Authentication_Event_Link_Key_Info_t  LinkKeyInfo;
   DEVM_Remote_Device_Application_Data_t     ApplicationData;
   unsigned int                              ConnectionCount;
   unsigned int                              SDPIdleTimerID;
   Extended_Inquiry_Response_Data_t          EIRData;
   Remote_LE_Device_Info_t                   RemoteLEDeviceInfo;
   struct _tagRemote_Device_Info_t          *NextRemoteDeviceInfoPtr;
} Remote_Device_Info_t;

#define REMOTE_DEVICE_INFO_SIZE                             (sizeof(Remote_Device_Info_t))

   /* The following constants are used with the DeviceFlags member of   */
   /* the Remote_Device_Info_t to denote information about the specified*/
   /* Remote Device.                                                    */
#define REMOTE_DEVICE_FLAGS_CURRENTLY_CONNECTED             0x00010000
#define REMOTE_DEVICE_FLAGS_CURRENTLY_AUTHENTICATED         0x00020000
#define REMOTE_DEVICE_FLAGS_CURRENTLY_ENCRYPTED             0x00040000
#define REMOTE_DEVICE_FLAGS_CURRENTLY_SNIFF_MODE            0x00080000
#define REMOTE_DEVICE_FLAGS_RETRIEVING_NAME                 0x00100000
#define REMOTE_DEVICE_FLAGS_RETRIEVING_SERVICES             0x00200000
#define REMOTE_DEVICE_FLAGS_PAIRING_IN_PROGRESS             0x00400000
#define REMOTE_DEVICE_FLAGS_ENCRYPTION_IN_PROGRESS          0x00800000
#define REMOTE_DEVICE_FLAGS_QUEUE_LOCAL_DEVICE_INITIATED    0x01000000
#define REMOTE_DEVICE_FLAGS_LOCAL_DEVICE_INITIATED          0x02000000

#define REMOTE_DEVICE_FLAGS_DEVICE_NAME_VALID               0x00000001
#define REMOTE_DEVICE_FLAGS_APPLICATION_DATA_VALID          0x00000002
#define REMOTE_DEVICE_FLAGS_SERVICE_INFORMATION_VALID       0x00000004
#define REMOTE_DEVICE_FLAGS_LINK_KEY_VALID                  0x00000008
#define REMOTE_DEVICE_FLAGS_PAGE_INFORMATION_VALID          0x00000010
#define REMOTE_DEVICE_FLAGS_CLOCK_OFFSET_VALID              0x00000020
#define REMOTE_DEVICE_FLAGS_RSSI_VALID                      0x00000040
#define REMOTE_DEVICE_FLAGS_TRANSMIT_POWER_VALID            0x00000080
#define REMOTE_DEVICE_FLAGS_EIR_DATA_VALID                  0x00000100

#define REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_LOW_ENERGY      0x40000000
#define REMOTE_DEVICE_FLAGS_DEVICE_SUPPORTS_BR_EDR          0x80000000

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            BD_ADDR field AND the AsynchronousInfoType field AND   */
   /*            the AsynchronousID field is the same as an entry       */
   /*            already in the list.  When this occurs, this function  */
   /*            returns NULL.                                          */
Asynchronous_Operation_t *AddAsynchronousOperation(Asynchronous_Operation_t **ListHead, Asynchronous_Operation_t *EntryToAdd);

   /* The following function searches the specified Asynchronous        */
   /* Operation List for the specified Bluetooth Device AND the         */
   /* specified operation.  This function returns NULL if either the    */
   /* Asynchronous Operation List Head is invalid, the Bluetooth Device */
   /* Address is invalid, the Asynchronous Operation Type is invalid, or*/
   /* the specified Asynchronous Operation was NOT present in the list. */
Asynchronous_Operation_t *SearchAsynchronousOperation(Asynchronous_Operation_t **ListHead, BD_ADDR_t BD_ADDR, Asynchronous_Operation_Type_t AsynchronousOperationType);

   /* The following function searches the specified Asynchronous        */
   /* Operation List for the specified SDP Callback ID.  This function  */
   /* returns NULL if either the Asynchronous Operation List Head is    */
   /* invalid, the SDP Callback ID is invalid, or the specified         */
   /* Asynchronous Operation was NOT present in the list.               */
Asynchronous_Operation_t *SearchAsynchronousOperationSDPCallbackID(Asynchronous_Operation_t **ListHead, unsigned int SDPCallbackID);

   /* The following function searches the specified Asynchronous        */
   /* Operation List for the specified Bluetooth Device AND the         */
   /* specified operation and removes it from the List.  This function  */
   /* returns NULL if either the Asynchronous Operation List Head is    */
   /* invalid, the Bluetooth Device Address is invalid, the Asynchronous*/
   /* Operation Type is invalid, or the specified Asynchronous Operation*/
   /* was NOT present in the list.  The entry returned will have the    */
   /* Next Entry field set to NULL, and the caller is responsible for   */
   /* deleting the memory associated with this entry by calling         */
   /* FreeAsynchronousOperationMemory().                                */
Asynchronous_Operation_t *DeleteAsynchronousOperation(Asynchronous_Operation_t **ListHead, BD_ADDR_t BD_ADDR, Asynchronous_Operation_Type_t AsynchronousOperationType);

   /* The following function searches the specified Asynchronous        */
   /* Operation List for the specified operation that has the specified */
   /* SDP Callback ID and removes it from the List.  This function      */
   /* returns NULL if either the Asynchronous Operation List Head is    */
   /* invalid, the SDP Callback ID is invalid, or the specified         */
   /* Asynchronous Operation was NOT present in the list.  The entry    */
   /* returned will have the Next Entry field set to NULL, and the      */
   /* caller is responsible for deleting the memory associated with this*/
   /* entry by calling FreeAsynchronousOperationMemory().               */
Asynchronous_Operation_t *DeleteAsynchronousOperationSDPCallbackID(Asynchronous_Operation_t **ListHead, unsigned int SDPCallbackID);

   /* The following function searches the specified Asynchronous        */
   /* Operation List for the specified operation that has the specified */
   /* Timer ID and removes it from the List.  This function returns NULL*/
   /* if either the Asynchronous Operation List Head is invalid, the    */
   /* Timer ID is invalid, or the specified Asynchronous Operation was  */
   /* NOT present in the list.  The entry returned will have the Next   */
   /* Entry field set to NULL, and the caller is responsible for        */
   /* deleting the memory associated with this entry by calling         */
   /* FreeAsynchronousOperationMemory().                                */
Asynchronous_Operation_t *DeleteAsynchronousOperationTimerID(Asynchronous_Operation_t **ListHead, unsigned int TimerID);

   /* This function frees the specified Asynchronous Operation member.  */
   /* No check is done on this entry other than making sure it is NOT   */
   /* NULL.                                                             */
void FreeAsynchronousOperationMemory(Asynchronous_Operation_t *EntryToFree);

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Asynchronous Operation List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
void FreeAsynchronousOperationList(Asynchronous_Operation_t **ListHead);

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            ServiceRecordHandle field AND the ClientID field are   */
   /*            the same as an entry already in the list.  When this   */
   /*            occurs, this function returns NULL.                    */
Client_SDP_Info_t *AddClientSDPInfoEntry(Client_SDP_Info_t **ListHead, Client_SDP_Info_t *EntryToAdd);

   /* The following function searches the specified Client SDP          */
   /* Information Entry List for the first entry that has the specified */
   /* Service Record Handle and returns the entry.  This function       */
   /* returns NULL if either the Client SDP Information List Head is    */
   /* invalid, Service Record Handle, or the specified Client SDP       */
   /* Information entry was NOT present in the list.                    */
Client_SDP_Info_t *SearchClientSDPInfoEntryServiceRecordHandle(Client_SDP_Info_t **ListHead, unsigned long ServiceRecordHandle);

   /* The following function searches the specified Client SDP          */
   /* Information Entry List for the first entry that has the specified */
   /* Client ID and removes it from the List (ignores the Service Record*/
   /* Handle).  This function returns NULL if either the Client SDP     */
   /* Information List Head is invalid, Client ID is invalid, or the    */
   /* specified Client SDP Information entry was NOT present in the     */
   /* list.  The entry returned will have the Next Entry field set to   */
   /* NULL, and the caller is responsible for deleting the memory       */
   /* associated with this entry by calling                             */
   /* FreeClientSDPInfoEntryMemory().                                   */
Client_SDP_Info_t *DeleteClientSDPInfoEntryClientID(Client_SDP_Info_t **ListHead, unsigned int ClientID);

   /* The following function searches the specified Client SDP          */
   /* Information Entry List for the entry that has the specified       */
   /* Service Record Handle and removes it from the List (ignores the   */
   /* Client ID).  This function returns NULL if either the Client SDP  */
   /* Information List Head is invalid, Service Record Handle is        */
   /* invalid, or the specified Client SDP Information entry was NOT    */
   /* present in the list.  The entry returned will have the Next Entry */
   /* field set to NULL, and the caller is responsible for deleting the */
   /* memory associated with this entry by calling                      */
   /* FreeClientSDPInfoEntryMemory().                                   */
Client_SDP_Info_t *DeleteClientSDPInfoEntryServiceRecordHandle(Client_SDP_Info_t **ListHead, unsigned long ServiceRecordHandle);

   /* This function frees the specified Client SDP Information Entry    */
   /* member.  No check is done on this entry other than making sure it */
   /* is NOT NULL.                                                      */
void FreeClientSDPInfoEntryMemory(Client_SDP_Info_t *EntryToFree);

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Client SDP Information List.  Upon return*/
   /* of this function, the Head Pointer is set to NULL.                */
void FreeClientSDPInfoList(Client_SDP_Info_t **ListHead);

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            ClientID field are the same as an entry already in the */
   /*            list.  When this occurs, this function returns NULL.   */
Advertising_Info_t *AddAdvertisingInfoEntry(Advertising_Info_t **ListHead, Advertising_Info_t *EntryToAdd);

   /* The following function searches the specified Advertising         */
   /* Information Entry List for the first entry that has the specified */
   /* Client ID and returns the entry.  This function returns NULL if   */
   /* either the Advertising Information List Head is invalid, or if the*/
   /* specified Client ID entry was NOT present in the list.            */
Advertising_Info_t *SearchAdvertisingInfoEntry(Advertising_Info_t **ListHead, unsigned int ClientID);

   /* The following function searches the specified Advertising         */
   /* Information Entry List for the first entry that has the specified */
   /* Timer ID and returns the entry.  This function returns NULL if    */
   /* either the Advertising Information List Head is invalid, or if the*/
   /* specified Timer ID entry was NOT present in the list.             */
Advertising_Info_t *SearchAdvertisingInfoEntryByTimerID(Advertising_Info_t **ListHead, unsigned int TimerID);

   /* The following function searches the specified Advertising         */
   /* Information Entry List for the first entry that has the specified */
   /* Client ID and removes it from the List.  This function returns    */
   /* NULL if either the Advertising Information List Head is invalid,  */
   /* Client ID is invalid, or the specified Client ID entry was NOT    */
   /* present in the list.  The entry returned will have the Next Entry */
   /* field set to NULL, and the caller is responsible for deleting the */
   /* memory associated with this entry by calling                      */
   /* FreeAdvertisingInfoEntryMemory().                                 */
Advertising_Info_t *DeleteAdvertisingInfoEntry(Advertising_Info_t **ListHead, unsigned int ClientID);

   /* This function frees the specified Advertising Information Entry   */
   /* member.  No check is done on this entry other than making sure it */
   /* is NOT NULL.                                                      */
void FreeAdvertisingInfoEntryMemory(Advertising_Info_t *EntryToFree);

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Advertising Information List.  Upon      */
   /* return of this function, the Head Pointer is set to NULL.         */
void FreeAdvertisingInfoList(Advertising_Info_t **ListHead);

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            RemoteDevice and AddressType field are the same as an  */
   /*            entry already in the list.  When this occurs, this     */
   /*            function returns NULL.                                 */
Scanning_Entry_t *AddScanningInfoEntry(Scanning_Entry_t **ListHead, Scanning_Entry_t *EntryToAdd);

   /* The following function searches the specified Advertising         */
   /* Information Entry List for the first entry that has the specified */
   /* AddressType and BD_ADDR and returns the entry.  This function     */
   /* returns NULL if either the Scanning Information List Head is      */
   /* invalid, or if the specified entry was NOT present in the list.   */
Scanning_Entry_t *SearchScanningInfoEntry(Scanning_Entry_t **ListHead, GAP_LE_Address_Type_t AddressType, BD_ADDR_t RemoteDevice);

   /* This function frees the specified Scanning Information Entry      */
   /* member.  No check is done on this entry other than making sure it */
   /* is NOT NULL.                                                      */
void FreeScanningInfoEntryMemory(Scanning_Entry_t *EntryToFree);

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Scanning Information List.  Upon return  */
   /* of this function, the Head Pointer is set to NULL.                */
void FreeScanningInfoList(Scanning_Entry_t **ListHead);

   /* The following function adds the specified Entry to the specified  */
   /* List.  This function allocates and adds an entry to the list that */
   /* has the same attributes as the Entry passed into this function.   */
   /* This function will return NULL if NO Entry was added.  This can   */
   /* occur if the element passed in was deemed invalid or the actual   */
   /* List Head was invalid.                                            */
   /* ** NOTE ** This function does not insert duplicate entries into   */
   /*            the list.  An element is considered a duplicate if the */
   /*            BD_ADDR field is the same as an entry already in the   */
   /*            list.  When this occurs, this function returns NULL.   */
Remote_Device_Info_t *AddRemoteDeviceInfoEntry(Remote_Device_Info_t **ListHead, Remote_Device_Info_t *EntryToAdd);

   /* The following function searches the specified List for the        */
   /* specified Bluetooth Device Address.  This function returns NULL if*/
   /* either the List Head is invalid, the Bluetooth Device Address is  */
   /* invalid, or the specified Bluetooth Device (based on the Bluetooth*/
   /* Device Address) was not found.                                    */
Remote_Device_Info_t *SearchRemoteDeviceInfoEntry(Remote_Device_Info_t **ListHead, BD_ADDR_t BD_ADDR);

   /* The following function searches the specified List for the        */
   /* specified Bluetooth Device Address Type and Address.  This        */
   /* function returns NULL if either the List Head is invalid, the     */
   /* Bluetooth Device Address is invalid, or the specified Bluetooth   */
   /* Device (based on the Bluetooth Device Address and Address Type)   */
   /* was not found.                                                    */
Remote_Device_Info_t *SearchRemoteDeviceInfoEntryByAddressType(Remote_Device_Info_t **ListHead, AddressType_t AddressType, BD_ADDR_t BD_ADDR);

   /* The following function searches the specified List for the        */
   /* specified Bluetooth Device with the specified Connection Handle.  */
   /* This function returns NULL if either the List Head is invalid, the*/
   /* Bluetooth Connection Handle is invalid, or the specified Bluetooth*/
   /* Device (based on the Bluetooth Connection Handle) was not found.  */
Remote_Device_Info_t *SearchRemoteDeviceInfoEntryConnectionHandle(Remote_Device_Info_t **ListHead, Word_t ConnectionHandle);

   /* The following function searches the specified List for the        */
   /* specified Bluetooth Device with the specified SDP Idle Timer ID.  */
   /* This function returns NULL if either the List Head is invalid, the*/
   /* Timer ID is invalid, or the specified Bluetooth Device (based on  */
   /* the Timer ID) was not found.                                      */
Remote_Device_Info_t *SearchRemoteDeviceInfoEntrySDPIdleTimerID(Remote_Device_Info_t **ListHead, unsigned int TimerID);

   /* The following function searches the specified Remote Bluetooth    */
   /* Device List for the specified Bluetooth Device and removes it from*/
   /* the List.  This function returns NULL if either the Remote        */
   /* Bluetooth Device Information List Head is invalid, the Bluetooth  */
   /* Device Address is invalid, or the specified Remote Bluetooth      */
   /* Device was NOT present in the list.  The entry returned will have */
   /* the Next Entry field set to NULL, and the caller is responsible   */
   /* for deleting the memory associated with this entry by calling     */
   /* FreeRemoteDeviceInfoEntryMemory().                                */
Remote_Device_Info_t *DeleteRemoteDeviceInfoEntry(Remote_Device_Info_t **ListHead, BD_ADDR_t BD_ADDR);

   /* The following function searches the specified Remote Bluetooth    */
   /* Device List for the specified Bluetooth Device (that was last     */
   /* resolvable/visible) and removes it from the List.  This function  */
   /* returns NULL if either the Remote Bluetooth Device Information    */
   /* List Head is invalid, the Bluetooth Device Address is invalid, or */
   /* the specified Remote Bluetooth Device was NOT present in the list.*/
   /* The entry returned will have the Next Entry field set to NULL, and*/
   /* the caller is responsible for deleting the memory associated with */
   /* this entry by calling FreeRemoteDeviceInfoEntryMemory().          */
Remote_Device_Info_t *DeleteRemoteDeviceInfoEntryByLastResolvable(Remote_Device_Info_t **ListHead, BD_ADDR_t BD_ADDR);

   /* This function frees the specified Remote Bluetooth Device         */
   /* Information Entry member.  No check is done on this entry other   */
   /* than making sure it is NOT NULL.                                  */
void FreeRemoteDeviceInfoEntryMemory(Remote_Device_Info_t *EntryToFree);

   /* The following function deletes (and free's all memory) every      */
   /* element of the specified Remote Bluetooth Device Information List.*/
   /* Upon return of this function, the Head Pointer is set to NULL.    */
void FreeRemoteDeviceInfoList(Remote_Device_Info_t **ListHead);

   /* The following function is responsible for reading the Local Device*/
   /* Properties from the Device Manager Local Device Settings Section  */
   /* of the specified Configuration File.  This function will *ONLY*   */
   /* override the settings in the specified Local Device Information   */
   /* structure (first parameter) that it finds.  It will not alter ANY */
   /* other members of this structure.                                  */
void ReadLocalDeviceSettingsFromFile(Local_Device_Info_t *LocalDeviceInfo, char *SettingsFileName);

   /* The following function is responsible for writing the Local Device*/
   /* Properties to the Device Manager Local Device Settings Section of */
   /* the specified Configuration File.                                 */
void WriteLocalDeviceSettingsToFile(Local_Device_Info_t *LocalDeviceInfo, char *SettingsFileName);

   /* The following function is responsible for reading All Remote      */
   /* Device Information Entries from the specified Remote Device File. */
   /* This function populates the global Remote Device List that is     */
   /* passed to this function.  The first parameter to this function    */
   /* specifies the Local Bluetooth Device Address.  This is required to*/
   /* determine if the Link Keys specified in the Device File (specified*/
   /* by the second parameter) are valid or not.  This function returns */
   /* the total number of Remote Devices that were read from the file   */
   /* and added to the specified list.                                  */
int ReadRemoteDevicesFromFile(BD_ADDR_t LocalBluetoothDevice, char *DeviceFileName, Remote_Device_Info_t **ListHead);

   /* The following function is responsible for writing All the Remote  */
   /* Device Information Entries to the specified Remote Device File.   */
   /* This function walks the Remote Device List that is passed to this */
   /* function.  The first parameter to this function specifies the     */
   /* Local Bluetooth Device Address.  This is required to be written,  */
   /* so when read, it can be used to determine if the Link Keys        */
   /* specified in the Device File                                      */
void WriteRemoteDevicesToFile(BD_ADDR_t LocalBluetoothDevice, Boolean_t StoreNonPairedDevices, char *DeviceFileName, Remote_Device_Info_t *ListHead);

#endif
