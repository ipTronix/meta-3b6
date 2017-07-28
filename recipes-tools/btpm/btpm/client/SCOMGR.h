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

   /* The following function is provided to allow a mechanism for local */
   /* modules to register a Local SCO Manager Server.  This function    */
   /* accepts a flag that specifies whether or not the server will      */
   /* accept incoming connections, and if so, the device that is allowed*/
   /* to connect.  This function returns a positive, non-zero, value if */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * A successful return value represents the Connection ID   */
   /*          that can be used with various functions in this module   */
   /*          to specify this local server connection.                 */
   /* * NOTE * If a specific is not required, and ANY device is to be   */
   /*          specified for the Remote Device (second parameter), then */
   /*          a NULL device address should be specified (all zeros).   */
int _SCOM_RegisterServerConnection(Boolean_t EnableConnection, BD_ADDR_t RemoteDevice);

   /* The following function is provided to allow a mechanism for local */
   /* modules to Un-Register a previously registered Server Connection. */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
int _SCOM_UnRegisterServerConnection(unsigned int ConnectionID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to enable/disable incoming connections for a specified    */
   /* Server Connection.  This function accepts the Server connection   */
   /* for which to enable/disable incoming connections followed by a    */
   /* flag that specifies whether or not the server will accept incoming*/
   /* connections, and if so, the device that is allowed to connect.    */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error.                                 */
   /* * NOTE * If a specific device is not required, and ANY device is  */
   /*          to be specified for the Remote Device (second parameter),*/
   /*          then a NULL device address should be specified (all      */
   /*          zeros).                                                  */
   /* * NOTE * The connection cannot be disabled for an individual      */
   /*          device.  If the second parameter is specified as FALSE   */
   /*          than ALL incoming connections are rejected and the final */
   /*          parameter is ignored.                                    */
int _SCOM_EnableServerConnection(unsigned int ConnectionID, Boolean_t EnableConnection, BD_ADDR_t RemoteDevice);

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
int _SCOM_SetEnhancedTransportParameters(SCO_Enhanced_Transport_Parameters_t *EnhancedTransportParameters);

#endif
