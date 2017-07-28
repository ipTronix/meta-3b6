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

#endif
