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

#include "GATMAPI.h"             /* BTPM GATT Manager API Proto./Constants.   */

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Generic Attribute Profile Manager      */
   /* Service.  This function returns zero if successful, or a negative */
   /* return error code if there was an error initializing the Bluetopia*/
   /* Platform Manager Generic Attribute Profile Manager Service.       */
int GATM_Initialize(void);

   /* The following function is responsible for shutting down the       */
   /* Bluetopia Platform Manager Generic Attribute Profile Manager      */
   /* Service.  After this function is called the Bluetooth Platform    */
   /* Manager Generic Attribute Profile Manager service will no longer  */
   /* operate until it is initialized again via a call to the           */
   /* GATM_Initialize() function.                                       */
void GATM_Cleanup(void);

#endif
