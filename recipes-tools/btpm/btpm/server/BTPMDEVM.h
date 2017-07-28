/*****< btpmdevm.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*      Copyright (C) 2016 Texas Instruments Incorporated -  	              */
/*      http://www.ti.com/ All Rights Reserved.  							  */
/*                                                                            */
/*  BTPMDEVM - Local Device Manager for Stonestreet One Bluetooth Protocol    */
/*             Stack Platform Manager.                                        */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   06/26/10  D. Lange       Initial creation.                               */
/*   04/19/16  L. Gersi       Adding support for LE SC pairing.               */
/*   07/25/16  D. Horowitz    Adding Ping feature.			      			  */
/******************************************************************************/
#ifndef __BTPMDEVMH__
#define __BTPMDEVMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "DEVMAPI.h"             /* BTPM Device Manager API Proto./Constants. */

   /* The following type is used with the DEVM_Update_Data_t structure  */
   /* (which is used with the DEVM_NotifyUpdate() function to inform the*/
   /* Device Manager that an Update needs to be dispatched.             */
typedef enum
{
   utLocalDevicePropertiesChanged,
   utRemoteDeviceStatus,
   utRemoteDeviceAddressChange,
   utRemoteDeviceNameStatus,
   utServiceDiscoveryEvent,
   utAuthenticationEvent,
   utLowEnergyAuthenticationEvent,
   utPairingStatus,
   utAuthenticationStatus,
   utEncryptionStatus,
   utConnectionStatus,
   utAdvertisingTimeout,
   utConnectionParameterUpdateResponse,
   utConnectionParametersUpdated,
   utAuthenticatedPayloadTimeoutExpired   
} DEVM_Update_Type_t;

   /* The following is the event data that is used with the             */
   /* utLocalDevicePropertiesChanged Update Type.                       */
typedef struct _tagDEVM_Local_Device_Properties_Changed_Data_t
{
   Boolean_t DiscoverableModeChange;
   Boolean_t ConnectableModeChange;
   Boolean_t PairableModeChange;
   Boolean_t DiscoveryStateChange;
   Boolean_t LEDiscoveryStateChange;
   Boolean_t LEAdvertisingStateChange;
   Boolean_t LEModeChange;
   Boolean_t LEAddressChange;
} DEVM_Local_Device_Properties_Changed_Data_t;

   /* The following is the event data that is used with the             */
   /* utRemoteDeviceStatus Update Type.                                 */
typedef struct _tagDEVM_Remote_Device_Status_Data_t
{
   BD_ADDR_t     BD_ADDR;
   Boolean_t     Added;
   Boolean_t     Updated;
   unsigned long UpdatedMask;
   Boolean_t     Deleted;
} DEVM_Remote_Device_Status_Data_t;

   /* The following is the event data that is used with the             */
   /* utRemoteDeviceAddressChange Update Type.                          */
typedef struct _tagDEVM_Remote_Device_Address_Changed_Data_t
{
   BD_ADDR_t BD_ADDR;
   BD_ADDR_t PreviousBD_ADDR;
} DEVM_Remote_Device_Address_Changed_Data_t;

   /* The following is the event data that is used with the             */
   /* utRemoteDeviceNameStatus Update Type.                             */
typedef struct _tagDEVM_Remote_Device_Name_Status_Data_t
{
   BD_ADDR_t    BD_ADDR;
   Boolean_t    Success;
   unsigned int AsynchronousID;
} DEVM_Remote_Device_Name_Status_Data_t;

   /* The following is the event data that is used with the             */
   /* utServiceDiscoveryEvent Update Type.                              */
typedef struct _tagDEVM_Service_Discovery_Event_Data_t
{
   BD_ADDR_t    BD_ADDR;
   Boolean_t    LowEnergy;
   Boolean_t    Success;
   unsigned int AsynchronousID;
} DEVM_Service_Discovery_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utAuthenticationEvent Update Type.                                */
typedef struct _tagDEVM_Authentication_Event_Data_t
{
   BD_ADDR_t                       BD_ADDR;
   GAP_Authentication_Event_Type_t AuthenticationType;
   Boolean_t                       Extended;
   union
   {
      Byte_t                                AuthenticationStatus;
      DWord_t                               NumericValue;
      Boolean_t                             Confirmation;
      GAP_Keypress_t                        KeypressType;
      GAP_IO_Capabilities_t                 IOCapabilities;
      DEVM_User_Confirmation_Request_Data_t UserConfirmationRequestData;
   } EventData;
} DEVM_Authentication_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utLowEnergyAuthenticationEvent Update Type.                       */
typedef struct _tagDEVM_Low_Energy_Authentication_Event_Data_t
{
   BD_ADDR_t                          BD_ADDR;
   GAP_LE_Authentication_Event_Type_t GAP_LE_Authentication_Type;
   union
   {
      GAP_LE_Security_Request_t                Security_Request;
      GAP_LE_Confirmation_Request_t            Confirmation_Request;
      GAP_LE_Pairing_Status_t                  Pairing_Status;
      GAP_LE_Security_Establishment_Complete_t Security_Establishment_Complete;
      GAP_LE_Keypress_t                        KeypressType;
   } EventData;
} DEVM_Low_Energy_Authentication_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utPairingStatus Update Type.                                      */
typedef struct _tagDEVM_Pairing_Status_Event_Data_t
{
   BD_ADDR_t    BD_ADDR;
   Boolean_t    LowEnergy;
   unsigned int AsynchronousID;
   Byte_t       AuthenticationStatus;
} DEVM_Pairing_Status_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utAuthenticationStatus Update Type.                               */
typedef struct _tagDEVM_Authentication_Status_Event_Data_t
{
   BD_ADDR_t    BD_ADDR;
   unsigned int AsynchronousID;
   int          Status;
} DEVM_Authentication_Status_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utEncryptionStatus Update Type.                                   */
typedef struct _tagDEVM_Encryption_Status_Event_Data_t
{
   BD_ADDR_t    BD_ADDR;
   unsigned int AsynchronousID;
   int          Status;
} DEVM_Encryption_Status_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utConnectionStatus Update Type.                                   */
typedef struct _tagDEVM_Connection_Status_Event_Data_t
{
   BD_ADDR_t    BD_ADDR;
   unsigned int AsynchronousID;
   int          Status;
} DEVM_Connection_Status_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utAdvertisingTimeout Update Type.                                 */
typedef struct _tagDEVM_Advertising_Timeout_Event_Data_t
{
   unsigned int ClientID;
} DEVM_Advertising_Timeout_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utConnectionParameterUpdateResponse Type.                               */
typedef struct _tagDEVM_Connection_Parameter_Update_Response_Event_Data_t
{
   BD_ADDR_t RemoteDeviceAddress;
   Boolean_t Accepted;
} DEVM_Connection_Parameter_Update_Response_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utConnectionParametersUpdated Type.                               */
typedef struct _tagDDEVM_Connection_Parameters_Updated_Event_Data_t
{
   int                                    Status;
   BD_ADDR_t                              RemoteDeviceAddress;
   GAP_LE_Current_Connection_Parameters_t Current_Connection_Parameters;
} DEVM_Connection_Parameters_Updated_Event_Data_t;

   /* The following is the event data that is used with the             */
   /* utAuthenticatedPayloadTimeoutExpired Type.                        */
typedef struct _tagDEVM_Authentication_Payload_Timeout_Expired_Event_Data_t
{
   BD_ADDR_t    BD_ADDR;
} DEVM_Authentication_Payload_Timeout_Expired_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the DEVM_NotifyUpdate() function).                           */
typedef struct _tagDEVM_Update_Data_t
{
   DEVM_Update_Type_t UpdateType;
   union
   {
      DEVM_Local_Device_Properties_Changed_Data_t               LocalDevicePropertiesChangedData;
      DEVM_Remote_Device_Status_Data_t                          RemoteDeviceStatusData;
      DEVM_Remote_Device_Address_Changed_Data_t                 RemoteDeviceAddressChangedData;
      DEVM_Remote_Device_Name_Status_Data_t                     RemoteDeviceNameStatusData;
      DEVM_Service_Discovery_Event_Data_t                       ServiceDiscoveryEventData;
      DEVM_Authentication_Event_Data_t                          AuthenticationEventData;
      DEVM_Pairing_Status_Event_Data_t                          PairingStatusEventData;
      DEVM_Authentication_Status_Event_Data_t                   AuthenticationStatusEventData;
      DEVM_Low_Energy_Authentication_Event_Data_t               LowEnergyAuthenticationEventData;
      DEVM_Encryption_Status_Event_Data_t                       EncryptionStatusEventData;
      DEVM_Connection_Status_Event_Data_t                       ConnectionStatusEventData;
      DEVM_Advertising_Timeout_Event_Data_t                     AdvertisingTimeoutEventData;
      DEVM_Connection_Parameter_Update_Response_Event_Data_t    ConnectionParameterUpdateResponse;
      DEVM_Connection_Parameters_Updated_Event_Data_t           ConnectionParametersUpdatedEventData;
      DEVM_Authentication_Payload_Timeout_Expired_Event_Data_t  AuthenticationPayloadTimeoutExpiredEventData;      
   } UpdateData;
} DEVM_Update_Data_t;

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Local Device Manager Service.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Local Device Manager Service.                             */
int DEVM_Initialize(DEVM_Initialization_Data_t *InitializationInfo);

   /* The following function is responsible for shutting down the       */
   /* Bluetopia Platform Manager Local Device Manager Service.  After   */
   /* this function is called the Bluetooth Platform Manager Local      */
   /* Device Manager service will no longer operate until it is         */
   /* initialized again via a call to the DEVM_Initialize() function.   */
void DEVM_Cleanup(void);

   /* The following function is provided to allow the caller the ability*/
   /* to queue an event into the Local Device Manager that will be      */
   /* dispatched BACK to this module through the _DEVM_ProcessEvent()   */
   /* function.  This allows the caller the ability to process data in a*/
   /* specific context for serialization purposes.                      */
Boolean_t DEVM_QueueEvent(void *EventData);

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Device Manager of a specific Update Event.  The     */
   /* Device Manager can then take the correct action to process the    */
   /* update.                                                           */
Boolean_t DEVM_NotifyUpdate(DEVM_Update_Data_t *UpdateData);

   /* The following function is a utility function which is used to     */
   /* query the local GATT sign counter and signing key from the Remote */
   /* Device Database.                                                  */
Boolean_t DEVM_QueryLocalSigningKey(BD_ADDR_t RemoteDevice, DWord_t *SignCounter, Encryption_Key_t *CSRK);

   /* The following function is a utility function which is used to     */
   /* query the remote GATT sign counter and signing key from the Remote*/
   /* Device Database.                                                  */
Boolean_t DEVM_QueryRemoteSigningKey(BD_ADDR_t RemoteDevice, DWord_t *SignCounter, Encryption_Key_t *CSRK);

   /* The following function is a utility function which is used to     */
   /* increment the Local GATT Sign Counter in the Remote Device        */
   /* Database.                                                         */
Boolean_t DEVM_IncrementLocalSignCounter(BD_ADDR_t RemoteDevice);

   /* The following function is a utility function which is used to     */
   /* increment the Remote GATT Sign Counter in the Remote Device       */
   /* Database.                                                         */
Boolean_t DEVM_IncrementRemoteSignCounter(BD_ADDR_t RemoteDevice);

#endif
