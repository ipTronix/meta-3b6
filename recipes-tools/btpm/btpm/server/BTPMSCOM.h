/*****< btpmscom.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMSCOM - SCO Manager for Stonestreet One Bluetooth Protocol Stack       */
/*             Platform Manager.                                              */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/07/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __BTPMSCOMH__
#define __BTPMSCOMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "SCOMAPI.h"             /* BTPM SCO Manager API Prototypes/Constants.*/

   /* The following type is used with the SCOM_Update_Data_t structure  */
   /* (which is used with the SCOM_NotifyUpdate() function to inform the*/
   /* Serial Port Manager that an Update needs to be dispatched.        */
typedef enum
{
   utSCOEvent,
   utSCOServerConnection
} SCOM_Update_Type_t;

typedef struct _tagSCOM_SCO_Event_Data_t
{
   SCO_Event_Type_t EventType;
   union
   {
      SCO_Connect_Result_Event_t ConnectResultEventData;
      SCO_Disconnect_Event_t     DisconnectEventData;
   } EventData;
} SCOM_SCO_Event_Data_t;

typedef struct _tagSCOM_Server_Connection_Event_Data_t
{
   BD_ADDR_t    BD_ADDR;
   unsigned int SCOConnectionID;
} SCOM_Server_Connection_Event_Data_t;

   /* The following structure is the container structure that holds the */
   /* information about what type of Update needs to be dispatched (used*/
   /* with the SCOM_NotifyUpdate() function).                           */
typedef struct _tagSCOM_Update_Data_t
{
   SCOM_Update_Type_t UpdateType;
   union
   {
      SCOM_SCO_Event_Data_t               SCOEventData;
      SCOM_Server_Connection_Event_Data_t SCOServerConnectionEventData;
   } UpdateData;
} SCOM_Update_Data_t;

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager SCO Manager Service.  This function    */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error initializing the Bluetopia Platform Manager SCO*/
   /* Manager Service.                                                  */
int SCOM_Initialize(void);

   /* The following function is responsible for shutting down the       */
   /* Bluetopia Platform Manager SCO Manager Service.  After this       */
   /* function is called the Bluetooth Platform Manager SCO Manager     */
   /* service will no longer operate until it is initialized again via a*/
   /* call to the SCOM_Initialize() function.                           */
void SCOM_Cleanup(void);

   /* The following function is provided to allow the caller the ability*/
   /* to notify the SCO Manager of a specific Update Event.  The SCO    */
   /* Manager can then take the correct action to process the update.   */
Boolean_t SCOM_NotifyUpdate(SCOM_Update_Data_t *UpdateData);

#endif
