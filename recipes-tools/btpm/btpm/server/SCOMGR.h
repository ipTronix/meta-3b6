/*****< scomgr.h >*************************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  SCOMGR - SCO Manager Implementation for Stonestreet One Bluetooth         */
/*           Protocol Stack Platform Manager.                                 */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   09/07/10  D. Lange       Initial creation.                               */
/******************************************************************************/
#ifndef __SCOMGRH__
#define __SCOMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the SCO Manager Implementation.  This function returns */
   /* zero if successful, or a negative return error code if there was  */
   /* an error initializing the Bluetopia Platform Manager SCO Manager  */
   /* Implementation.                                                   */
int _SCOM_Initialize(void);

   /* The following function is responsible for shutting down the SCO   */
   /* Manager Implementation.  After this function is called the SCO    */
   /* Manager Implementation will no longer operate until it is         */
   /* initialized again via a call to the _SCOM_Initialize() function.  */
void _SCOM_Cleanup(void);

   /* The following function is responsible for informing the SCO       */
   /* Manager Implementation of that Bluetooth Stack ID of the currently*/
   /* opened Bluetooth stack.                                           */
void _SCOM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is responsible for informing the SCO       */
   /* Manager Implementation whether or not it is to accept any         */
   /* connections.                                                      */
void _SCOM_SetConnectionAcceptMode(Boolean_t AcceptConnections);

   /* The following function is provided to allow a mechanism for local */
   /* modules to open remote SCO connection.  This function returns a   */
   /* positive, non-zero, value if successful, or a negative return     */
   /* error code if there was an error.  If this function is successful,*/
   /* the value that is returned represents the SCO Manager Connection  */
   /* ID of the connection.                                             */
int _SCOM_OpenRemoteConnection(BD_ADDR_t RemoteDevice);

   /* The following function is provided to allow a mechanism for local */
   /* modules to close an active connection for the specified SCO       */
   /* Connection (either server or remote connection).  This function   */
   /* returns zero if successful, or a negative return error code if    */
   /* there was an error.                                               */
   /* * NOTE * This function will NOT Un-Register a local server        */
   /*          connection, it will just disconnect any currently        */
   /*          connected remote device.                                 */
int _SCOM_CloseConnection(unsigned int ConnectionID);

   /* The following function is provided to allow local modules the     */
   /* ability to inform the SCO Manager to ignore incoming connections  */
   /* from the specified device.  This is useful if a particular module */
   /* is tracking SCO connections itself (for example Headset or        */
   /* Handsfree modules).  This function returns TRUE if successful or a*/
   /* FALSE if there was an error.                                      */
Boolean_t _SCOM_AddConnectionToIgnoreList(BD_ADDR_t RemoteDevice);

   /* The following function is provided to allow a mechanism for local */
   /* modules to remove a (previously added) device from the ignore     */
   /* list.  This function accepts as input the device to remove from   */
   /* the list.  This function returns TRUE if successful or FALSE if   */
   /* there was an error.                                               */
   /* * NOTE * If the device specified is a NULL BD_ADDR (all zero's)   */
   /*          then this function will clear the entire ignore list.    */
Boolean_t _SCOM_DeleteConnectionFromIgnoreList(BD_ADDR_t RemoteDevice);

   /* The following function is responsible for configuring the enhanced*/
   /* audio connection parameters.  This function accepts the Bluetooth */
   /* Stack ID of the Bluetooth Stack for which the paramters are being */
   /* specified as it's first parameter.  The second parameter specifies*/
   /* the parameters to use.  This function returns zero if successful  */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * The paramters can only be changed if there are no active */
   /*          SCO connections.  Calling this function with active      */
   /*          connections will cause the function to return an error   */
   /*          and the paramters will not be changed.                   */
Boolean_t _SCOM_SetEnhancedTransportParameters(SCO_Enhanced_Transport_Parameters_t *EnhancedTransportParameters);

#endif
