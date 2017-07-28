/*****< btpmsppm.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMSPPM - Serial Port Profile Manager for Stonestreet One Bluetooth      */
/*             Protocol Stack Platform Manager.                               */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   08/16/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMSPPMH__
#define __BTPMSPPMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SS1BTPS.h"             /* Bluetopia API Protypes and Constants.     */

#include "SPPMAPI.h"             /* BTPM SPP Manager API Proto./Constants.    */

#if BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_SUPPORT_MFI

#include "SS1BTISP.h"            /* Bluetopia MFi API Prototypes/Constants.   */

#endif

   /* The following type is used with the SPPM_Update_Data_t structure  */
   /* (which is used with the SPPM_NotifyUpdate() function to inform the*/
   /* Serial Port Manager that an Update needs to be dispatched.        */
typedef enum
{
   utSPPEvent,
   utSDPEvent
} SPPM_Update_Type_t;

typedef struct _tagSPPM_SPP_Event_Data_t
{
#if BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_SUPPORT_MFI
   ISPP_Event_Type_t EventType;
#else
   SPP_Event_Type_t  EventType;
#endif
   union
   {
      SPP_Open_Port_Indication_Data_t             OpenPortIndicationData;
      SPP_Open_Port_Confirmation_Data_t           OpenPortConfirmationData;
      SPP_Close_Port_Indication_Data_t            ClosePortIndicationData;
      SPP_Port_Status_Indication_Data_t           PortStatusIndicationData;
      SPP_Data_Indication_Data_t                  DataIndicationData;
      SPP_Transmit_Buffer_Empty_Indication_Data_t TransmitBufferEmptyIndicationData;
      SPP_Line_Status_Indication_Data_t           LineStatusIndicationData;
      SPP_Open_Port_Request_Indication_Data_t     OpenPortRequestIndicationData;

#if BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_SUPPORT_MFI
      ISPP_Process_Status_Data_t                  ProcessStatus;
      ISPP_Session_Open_Indication_Data_t         SessionOpenIndication;
      ISPP_Session_Close_Indication_Data_t        SessionCloseIndication;
      ISPP_Session_Data_Indication_Data_t         SessionDataIndication;
      ISPP_Send_Session_Data_Confirmation_Data_t  SendSessionDataConfirmation;
      ISPP_Raw_Data_Indication_Data_t             RawDataIndication;
      ISPP_Send_Raw_Data_Confirmation_Data_t      SendRawDataConfirmation;
#endif
   } EventData;
} SPPM_SPP_Event_Data_t;

typedef struct _tagSPPM_SDP_Event_Data_t
{
   Boolean_t    Success;
   Boolean_t    MFiSupport;
   unsigned int SerialPortID;
} SPPM_SDP_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the SPPM_NotifyUpdate() function).                           */
typedef struct _tagSPPM_Update_Data_t
{
   SPPM_Update_Type_t UpdateType;
   union
   {
      SPPM_SPP_Event_Data_t SPPEventData;
      SPPM_SDP_Event_Data_t SDPEventData;
   } UpdateData;
} SPPM_Update_Data_t;

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Serial Port Profile Manager Service.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error initializing the Bluetopia       */
   /* Platform Manager Serial Port Profile Manager Service.             */
int SPPM_Initialize(SPPM_Initialization_Data_t *InitializationInfo);

   /* The following function is responsible for shutting down the       */
   /* Bluetopia Platform Manager Serial Port Profile Manager Service.   */
   /* After this function is called the Bluetooth Platform Manager      */
   /* Serial Port Profile Manager service will no longer operate until  */
   /* it is initialized again via a call to the SPPM_Initialize()       */
   /* function.                                                         */
void SPPM_Cleanup(void);

   /* The following function is provided to allow the caller the ability*/
   /* to notify the Serial Port Manager of a specific Update Event.  The*/
   /* Serial Port Manager can then take the correct action to process   */
   /* the update.                                                       */
Boolean_t SPPM_NotifyUpdate(SPPM_Update_Data_t *UpdateData);

#endif
