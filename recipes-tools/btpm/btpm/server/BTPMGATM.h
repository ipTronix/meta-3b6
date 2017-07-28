/*****< btpmgatm.h >***********************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMGATM - Generic Attribute Profile Manager for Stonestreet One          */
/*             Bluetooth Protocol Stack Platform Manager.                     */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/16/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMGATMH__
#define __BTPMGATMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTPS.h"             /* Bluetopia API Protypes and Constants.     */

#include "SS1BTGAT.h"            /* Bluetopia GATT API Prototypes/Constants.  */

#include "GATMAPI.h"             /* BTPM GATT Manager API Proto./Constants.   */

   /* The following type is used with the GATM_Update_Data_t structure  */
   /* (which is used with the GATM_NotifyUpdate() function to inform the*/
   /* GATT Manager that an Update needs to be dispatched.               */
typedef enum
{
   utGATTConnectionEvent,
   utGATTServerEvent,
   utGATTClientEvent
} GATM_Update_Type_t;

   /* The following structure represents the container structure for    */
   /* holding all GATM connection event data.                           */
typedef struct _tagGATM_Connection_Event_Data_t
{
   GATT_Connection_Event_Type_t Event_Data_Type;
   Word_t                       Event_Data_Size;
   union
   {
      GATT_Device_Connection_Request_Data_t               GATT_Device_Connection_Request_Data;
      GATT_Device_Connection_Data_t                       GATT_Device_Connection_Data;
      GATT_Device_Connection_Confirmation_Data_t          GATT_Device_Connection_Confirmation_Data;
      GATT_Device_Disconnection_Data_t                    GATT_Device_Disconnection_Data;
      GATT_Device_Buffer_Empty_Data_t                     GATT_Device_Buffer_Empty_Data;
      GATT_Server_Notification_Data_t                     GATT_Server_Notification_Data;
      GATT_Server_Indication_Data_t                       GATT_Server_Indication_Data;
      GATT_Device_Connection_MTU_Update_Data_t            GATT_Device_Connection_MTU_Update_Data;
      GATT_Connection_Service_Database_Update_Data_t      GATT_Connection_Service_Database_Update_Data;
      GATT_Connection_Service_Changed_Read_Data_t         GATT_Connection_Service_Changed_Read_Data;
      GATT_Connection_Service_Changed_Confirmation_Data_t GATT_Connection_Service_Changed_Confirmation_Data;
      GATT_Connection_Service_Changed_CCCD_Read_Data_t    GATT_Connection_Service_Changed_CCCD_Read_Data;
      GATT_Connection_Service_Changed_CCCD_Update_Data_t  GATT_Connection_Service_Changed_CCCD_Update_Data;
   } Event_Data;
} GATM_Connection_Event_Data_t;

   /* The following structure represents the container structure for    */
   /* holding all GATM server event data.                               */
typedef struct _tagGATM_Server_Event_Data_t
{
   GATT_Server_Event_Type_t Event_Data_Type;
   Word_t                   Event_Data_Size;
   union
   {
      GATT_Device_Connection_Data_t            GATT_Device_Connection_Data;
      GATT_Device_Disconnection_Data_t         GATT_Device_Disconnection_Data;
      GATT_Device_Connection_MTU_Update_Data_t GATT_Device_Connection_MTU_Update_Data;
      GATT_Read_Request_Data_t                 GATT_Read_Request_Data;
      GATT_Write_Request_Data_t                GATT_Write_Request_Data;
      GATT_Signed_Write_Request_Data_t         GATT_Signed_Write_Request_Data;
      GATT_Execute_Write_Request_Data_t        GATT_Execute_Write_Request_Data;
      GATT_Execute_Write_Confirmation_Data_t   GATT_Execute_Write_Confirmation_Data;
      GATT_Confirmation_Data_t                 GATT_Confirmation_Data;
   } Event_Data;
} GATM_Server_Event_Data_t;

   /* The following structure represents the container structure for    */
   /* holding all GATT client event data.                               */
typedef struct _tagGATM_Client_Event_Data_t
{
   GATT_Client_Event_Type_t Event_Data_Type;
   Word_t                   Event_Data_Size;
   union
   {
      GATT_Request_Error_Data_t                                GATT_Request_Error_Data;
      GATT_Service_Discovery_Response_Data_t                   GATT_Service_Discovery_Response_Data;
      GATT_Service_Discovery_By_UUID_Response_Data_t           GATT_Service_Discovery_By_UUID_Response_Data;
      GATT_Included_Services_Discovery_Response_Data_t         GATT_Included_Services_Discovery_Response_Data;
      GATT_Characteristic_Discovery_Response_Data_t            GATT_Characteristic_Discovery_Response_Data;
      GATT_Characteristic_Descriptor_Discovery_Response_Data_t GATT_Characteristic_Descriptor_Discovery_Response_Data;
      GATT_Read_Response_Data_t                                GATT_Read_Response_Data;
      GATT_Read_By_UUID_Response_Data_t                        GATT_Read_By_UUID_Response_Data;
      GATT_Read_Long_Response_Data_t                           GATT_Read_Long_Response_Data;
      GATT_Read_Multiple_Response_Data_t                       GATT_Read_Multiple_Response_Data;
      GATT_Write_Response_Data_t                               GATT_Write_Response_Data;
      GATT_Prepare_Write_Response_Data_t                       GATT_Prepare_Write_Response_Data;
      GATT_Execute_Write_Response_Data_t                       GATT_Execute_Write_Response_Data;
      GATT_Exchange_MTU_Response_Data_t                        GATT_Exchange_MTU_Response_Data;
   } Event_Data;
} GATM_Client_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the GATM_NotifyUpdate() function).                           */
typedef struct _tagGATM_Update_Data_t
{
   GATM_Update_Type_t UpdateType;
   union
   {
      GATM_Connection_Event_Data_t ConnectionEventData;
      GATM_Server_Event_Data_t     ServerEventData;
      GATM_Client_Event_Data_t     ClientEventData;
   } UpdateData;
} GATM_Update_Data_t;

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Generic Attribute Profile Manager      */
   /* Service.  This function returns zero if successful, or a negative */
   /* return error code if there was an error initializing the Bluetopia*/
   /* Platform Manager Generic Attribute Profile Manager Service.       */
int GATM_Initialize(GATM_Initialization_Data_t *InitializationInfo);

   /* The following function is responsible for shutting down the       */
   /* Bluetopia Platform Manager Generic Attribute Profile Manager      */
   /* Service.  After this function is called the Bluetooth Platform    */
   /* Manager Generic Attribute Profile Manager service will no longer  */
   /* operate until it is initialized again via a call to the           */
   /* GATM_Initialize() function.                                       */
void GATM_Cleanup(void);

   /* The following function is a utility function that is used to query*/
   /* the GATT Connection Identifier for the specified connection.  This*/
   /* function returns the positive, non-zero, Connection ID on success */
   /* or a negative error code.                                         */
int GATM_Query_Connection_ID(BD_ADDR_t BD_ADDR);

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Generic Attribute Manager of a specific Update      */
   /* Event.  The Generic Attribute Manager can then take the correct   */
   /* action to process the update.                                     */
Boolean_t GATM_NotifyUpdate(GATM_Update_Data_t *UpdateData);

#endif
