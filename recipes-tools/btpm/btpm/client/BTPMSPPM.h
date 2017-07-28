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

#include "SPPMAPI.h"             /* BTPM SPP Manager API Proto./Constants.    */

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Serial Port Profile Manager Service.   */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error initializing the Bluetopia       */
   /* Platform Manager Serial Port Profile Manager Service.             */
int SPPM_Initialize(void);

   /* The following function is responsible for shutting down the       */
   /* Bluetopia Platform Manager Serial Port Profile Manager Service.   */
   /* After this function is called the Bluetooth Platform Manager      */
   /* Serial Port Profile Manager service will no longer operate until  */
   /* it is initialized again via a call to the SPPM_Initialize()       */
   /* function.                                                         */
void SPPM_Cleanup(void);

#endif
