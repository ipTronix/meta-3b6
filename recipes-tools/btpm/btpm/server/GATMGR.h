/*****< gatmgr.h >*************************************************************/
/*      Copyright 2012 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  GATMGR - Generic Attribute Profile Manager Implementation for Stonestreet */
/*           One Bluetooth Protocol Stack Platform Manager.                   */
/*                                                                            */
/*  Author:  Tim Cook                                                         */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   10/16/12  T. Cook        Initial creation.                               */
/******************************************************************************/
#ifndef __GATMGRH__
#define __GATMGRH__

#include "BTAPITyp.h"            /* Bluetooth API Type Definitions.           */

   /* The following function is provided to allow a mechanism to        */
   /* initialize the Generic Attribute Manager Implementation.  This    */
   /* function returns zero if successful, or a negative return error   */
   /* code if there was an error initializing the Bluetopia Platform    */
   /* Manager Generic Attribute Profile Manager Implementation.         */
int _GATM_Initialize(unsigned long GATTInitializationFlags);

   /* The following function is responsible for shutting down the       */
   /* Generic Attribute Profile Implementation.  After this function is */
   /* called the Generic Attribute Profile Manager Implementation will  */
   /* no longer operate until it is initialized again via a call to the */
   /* _GATM_Initialize() function.                                      */
void _GATM_Cleanup(void);

   /* The following function is responsible for informing the Generic   */
   /* Attribute Profile Manager Implementation of that Bluetooth Stack  */
   /* ID of the currently opened Bluetooth stack.                       */
void _GATM_SetBluetoothStackID(unsigned int BluetoothStackID);

   /* The following function is responsible for informing GATT Manager  */
   /* Implementation of the Queuing parameters to use.                  */
void _GATM_SetQueueParameters(unsigned int QueueDepth, unsigned int QueueThreshold);

   /* GATM Client APIs.                                                 */

   /* The following function is provided to allow a mechanism for local */
   /* modules to read a specified Attribute on the specified remote     */
   /* device.  The function accepts the ConnectionID of the Connection  */
   /* to read the value from, the Attribute Handle of the attribute on  */
   /* the remote device to read, and the number of bytes into the       */
   /* attribute to read.  This function returns a positive non-zero     */
   /* value if successful, or a negative return error code if there was */
   /* an error.                                                         */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
   /* * NOTE * The Offset parameter can be used to read the specified   */
   /*          attribute at the offset specified by the parameter.      */
int _GATM_ReadValue(unsigned int ConnectionID, Word_t AttributeHandle, unsigned int Offset);

   /* The following function is provided to allow a mechanism for local */
   /* modules to write a specified Attribute on the specified remote    */
   /* device.  The function accepts the ConnectionID of the Connection  */
   /* to write the value to, the Attribute Handle of the attribute on   */
   /* the remote device to write, the length of the data and a pointer  */
   /* to the data to write.  This function returns a positive non-zero  */
   /* value if successful, or a negative return error code if there was */
   /* an error.                                                         */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
int _GATM_WriteValue(unsigned int ConnectionID, Word_t AttributeHandle, unsigned int DataLength, Byte_t *Data);

   /* The following function is provided to allow a mechanism for local */
   /* modules to write using a Prepare Write Request a specified        */
   /* Attribute on the specified remote device.  The function accepts   */
   /* the ConnectionID of the Connection to send the prepare write to,  */
   /* the Attribute Handle of the attribute on the remote device to     */
   /* write, the offset into the value to write and the length of the   */
   /* data and a pointer to the data to write.  This function returns a */
   /* positive non-zero value if successful, or a negative return error */
   /* code if there was an error.                                       */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
int _GATM_PrepareWrite(unsigned int ConnectionID, Word_t AttributeHandle, unsigned int Offset, unsigned int DataLength, Byte_t *Data);

   /* The following function is provided to allow a mechanism for local */
   /* modules to execute prepared writes.  The function accepts the     */
   /* ConnectionID of the connection to send the execute to and a       */
   /* Boolean that specifies if the write is to be Canceled (TRUE) or   */
   /* executed (FALSE).  This function returns a positive non-zero value*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
int _GATM_ExecuteWrite(unsigned int ConnectionID, Boolean_t Cancel);

   /* The following function is provided to allow a mechanism for local */
   /* modules to perform a write without response to a specified        */
   /* Attribute on the specified remote device.  The first parameter to */
   /* this function is the ConnectionID of the device to write to.  The */
   /* second parameter specifies the handle of the attribute that is to */
   /* be written.  The third parameter is specifies the signature       */
   /* counter of the attribute that is to be written (if a signed write */
   /* is being performed).  The fourth parameter is the optional CSRK   */
   /* which must be specified for a signed write to be performed.  The  */
   /* final two parameters specify the length and a pointer to the data */
   /* that is to be written.  This function returns zero if successful, */
   /* or a negative return error code if there was an error.            */
   /* * NOTE * No event is generated by this function.                  */
   /* * NOTE * A Signed Write can only be performed to a previously     */
   /*          paired device.                                           */
int _GATM_WriteValueWithoutResponse(unsigned int ConnectionID, Word_t AttributeHandle, DWord_t SignatureCounter, Encryption_Key_t *CSRK, unsigned int DataLength, Byte_t *Data);

   /* The following function is provided to allow a mechanism for local */
   /* modules to send a Handle Value Confirmation to a Handle Value     */
   /* Indication.  The first parameter to this function is the          */
   /* ConnectionID of the device to send the confirmation to.  The      */
   /* second parameter specifies the TransactionID of the indication    */
   /* that is being responded to.  This function returns zero if        */
   /* successful, or a negative return error code if there was an error.*/
   /* * NOTE * No event is generated by this function.                  */
int _GATM_HandleValueConfirmation(unsigned int ConnectionID, unsigned int TransactionID);

   /* GATM Server APIs.                                                 */

   /* The following function is used to publish a previously registered */
   /* GATT Service.  The first parameter to this function are the       */
   /* ServiceFlags which specify what the service supports.  The second */
   /* parameter specifies the number of attributes need by the function.*/
   /* The third parameter is the pointer to the service table to        */
   /* register.  The final parameter is an input/output parameter that  */
   /* is used to specify a specific handle range to put the service or  */
   /* to return the actual allocated handle range.  On success this     */
   /* function will return the positive non-zero Service ID of the      */
   /* published service or a negative error code on failure.            */
   /* * NOTE * Once this function is called no Includes, Characteristics*/
   /*          or Descriptors can be added to the service.              */
   /* * NOTE * Once this function is called the registered service will */
   /*          be "published" and any connected Client will be able to  */
   /*          discover the service (if it is a primary service).       */
int _GATM_PublishService(Byte_t ServiceFlags, unsigned int NumberOfServiceAttributeEntries, GATT_Service_Attribute_Entry_t *ServiceTable, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

   /* The following function is used to un-publish a previously         */
   /* registered service.  The only parameter to this function is the   */
   /* Service of the previously published GATT Service.  On success this*/
   /* function will return ZERO or a negative error code on failure.    */
int _GATM_DeleteService(unsigned int ServiceID);

   /* The following function is used to send a Handle Value Indication  */
   /* to a specified remote device.  The first parameter is the Service */
   /* ID of the previously published service whose attribute is being   */
   /* indication.  The second parameter is the ConnectionID of the      */
   /* connection to send the handle value indication to.  The third     */
   /* parameter contains the attribute offset of the value that is being*/
   /* indicated.  The final two parameters contain the length of the    */
   /* data and a pointer to the data to indicate.  This function returns*/
   /* a positive non-zero value if successful, or a negative return     */
   /* error code if there was an error.                                 */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the handle value*/
   /*          confirmation that is received in response to this call.  */
int _GATM_HandleValueIndication(unsigned int ServiceID, unsigned int ConnectionID, unsigned int AttributeOffset, unsigned int DataLength, Byte_t *Data);

   /* The following function is used to send a Handle Value Notification*/
   /* to a specified remote device.  The first parameter is the Service */
   /* ID of the previously published service whose attribute is being   */
   /* indication.  The second parameter is the Connection ID of the     */
   /* connection to send the handle value notification to.  The third   */
   /* parameter contains the attribute offset of the value that is being*/
   /* indicated.  The final two parameters contain the length of the    */
   /* data and a pointer to the data to indicate.  On success this      */
   /* function will return ZERO or a negative error code on failure.    */
int _GATM_HandleValueNotification(unsigned int ServiceID, unsigned int ConnectionID, unsigned int AttributeOffset, unsigned int DataLength, Byte_t *Data);

   /* The following function is used to respond with success to a Write */
   /* or Prepare Write Request from a connected client.  The only       */
   /* parameter to this function is the TransactionID of the request    */
   /* that is being responded to.  On success this function will return */
   /* ZERO or a negative error code on failure.                         */
int _GATM_WriteResponse(unsigned int TransactionID);

   /* The following function is used to respond with success to a       */
   /* Execute Write Request from a connected client.  The only parameter*/
   /* to this function is the TransactionID of the request that is being*/
   /* responded to.  On success this function will return ZERO or a     */
   /* negative error code on failure.                                   */
int _GATM_ExecuteWriteResponse(unsigned int TransactionID);

   /* The following function is a utility function that is used to      */
   /* respond to a Read Request from a connected client.  The first     */
   /* parameter to this function is the TransactionID of the request    */
   /* that is being responded to.  The final parameters to this function*/
   /* specify the length and a pointer to the data to respond with.  On */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
int _GATM_ReadResponse(unsigned int TransactionID, unsigned int DataLength, Byte_t *Data);

   /* The following function is a utility function that is used to      */
   /* respond to a Service Changed Read Request from a connected client.*/
   /* The first parameter to this function is the Transaction ID of the */
   /* read request that is being responded to.  The final two parameters*/
   /* specify the handle range that has been affected by a service being*/
   /* added/deleted to/from the GATT database.  On success this function*/
   /* will return ZERO or a negative error code on failure.             */
int _GATM_ServiceChangedReadResponse(unsigned int TransactionID, Word_t StartHandle, Word_t EndHandle);

   /* The following function is a utility function that is used to      */
   /* respond to a Service Changed CCCD Read Request from a connected   */
   /* client.  The first parameter to this function is the Transaction  */
   /* ID of the read request that is being responded to.  The final     */
   /* parameter is the CCCD to respond to the read request with.  On    */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
int _GATM_ServiceChangedCCCDReadResponse(unsigned int TransactionID, Word_t CCCD);

   /* The following function is a utility function that is used to      */
   /* respond to a Service Changed CCCD Read Request from a connected   */
   /* client.  The first parameter to this function is the Transaction  */
   /* ID of the read request that is being responded to.  The final     */
   /* parameter is the CCCD to respond to the read request with.  On    */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
int _GATM_ServiceChangedIndication(unsigned int ConnectionID, GATT_Service_Changed_Data_t *ServiceChangedData);

   /* The following function is a utility function that is used to      */
   /* respond with an error to a Write or a Read Request.  The first    */
   /* parameter to this function is the TransactionID of the request    */
   /* that is being responded to.  The second parameter is the          */
   /* AttributeOffset of the attribute whose access caused the error.   */
   /* The final parameters to this function specifies the Attribute     */
   /* Protocol Error Code to respond with.  On success this function    */
   /* will return ZERO or a negative error code on failure.             */
int _GATM_ErrorResponse(unsigned int TransactionID, unsigned int AttributeOffset, Byte_t ErrorCode);

   /* The following function is used to verify a signed write received  */
   /* from a specified remote device.  The first parameter is the       */
   /* Service ID of the previously published service whose attribute is */
   /* being written.  The second parameter is the attribute offset of   */
   /* the attribute that is being written.  The third and fourth        */
   /* parameters contain the length of the data and a pointer to the    */
   /* data to indicate.  The fifth parameter containes the signature to */
   /* verify.  The final parameter represents the key that the signature*/
   /* should have been created with.  On success this function will     */
   /* return TRUE or FALSE on failure.                                  */
Boolean_t _GATM_VerifySignature(unsigned int ServiceID, unsigned int AttributeOffset, unsigned int AttributeValueLength, Byte_t *AttributeValue, ATT_Authentication_Signature_t *ReceivedSignature, Encryption_Key_t *CSRK);

   /* GATM General API.                                                 */

   /* The following function is a utility function that is used to query*/
   /* the ATT Opcode for a specified Transaction.  The first parameter  */
   /* to this function is the TransactionID of the transaction to query */
   /* the opcode for.  The second parameter is a pointer to return the  */
   /* Opcode into on success.  On success this function will return ZERO*/
   /* or a negative error code on failure.                              */
int _GATM_QueryTransactionOpcode(unsigned int TransactionID, Byte_t *Opcode);

   /* The following function is a utility function that is used to      */
   /* cancel an outstanding transaction.  The first parameter to this   */
   /* function is the TransactionID of the transaction that is being    */
   /* canceled.  On success this function will return ZERO or a negative*/
   /* error code on failure.                                            */
int _GATM_CancelTransaction(unsigned int TransactionID);

   /* The following function is a utility function that is used to      */
   /* determine if the specified handle range can be used to register a */
   /* service in.  The only parameter to this function is the handle    */
   /* range to get the availability of.  On success this function will  */
   /* return TRUE if the handle range is available in the GATT database */
   /* or FALSE otherwise.                                               */
Boolean_t _GATM_Get_Service_Range_Availability(GATT_Attribute_Handle_Group_t *ServiceHandleGroup);

#endif
