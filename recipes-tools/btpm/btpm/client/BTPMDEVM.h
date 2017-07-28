/*****< btpmdevm.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
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
/******************************************************************************/
#ifndef __BTPMDEVMH__
#define __BTPMDEVMH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */
#include "BTTypes.h"             /* Bluetooth Type Definitions.               */

#include "DEVMAPI.h"             /* BTPM Device Manager API Proto./Constants. */

   /* The following function is responsible for initializing the        */
   /* Bluetopia Platform Manager Local Device Manager Service.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Local Device Manager Service.                             */
int DEVM_Initialize(void);

   /* The following function is responsible for shutting down the       */
   /* Bluetopia Platform Manager Local Device Manager Service.  After   */
   /* this function is called the Bluetooth Platform Manager Local      */
   /* Device Manager service will no longer operate until it is         */
   /* initialized again via a call to the DEVM_Initialize() function.   */
void DEVM_Cleanup(void);

#endif
