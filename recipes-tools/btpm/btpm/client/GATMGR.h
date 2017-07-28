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
   /* initialize the Generic Attribute Profile Manager Implementation.  */
   /* This function returns zero if successful, or a negative return    */
   /* error code if there was an error initializing the Bluetopia       */
   /* Platform Manager Generic Attribute Profile Manager Implementation.*/
int _GATM_Initialize(void);

   /* The following function is responsible for shutting down the       */
   /* Generic Attribute Profile Implementation.  After this function is */
   /* called the Generic Attribute Profile Manager Implementation will  */
   /* no longer operate until it is initialized again via a call to the */
   /* _GATM_Initialize() function.                                      */
void _GATM_Cleanup(void);

   /* GATM Connection APIs.                                             */

   /* The following function is provided to allow a mechanism for       */
   /* modules to register a callback function with the GATT Manager     */
   /* Service.  This Callback will be dispatched by the GATT Manager    */
   /* when various GATT Manager Events occur.  This function returns a  */
   /* non-zero value if successful or a negative return error code if   */
   /* there was an error.                                               */
   /* * NOTE * The return value from this function specifies the GATT   */
   /*          Event Handler ID.  This value can be passed to the       */
   /*          _GATM_UnRegisterGATT_Events() function to Un-Register    */
   /*          the Event Handler.                                       */
int _GATM_RegisterGATTEvents(void);

   /* The following function is provided to allow a mechanism to        */
   /* un-register a previously registered HGATT Manager Event Handler   */
   /* (registered via a successful call to the                          */
   /* _GATM_RegisterGATTEvents() function).  This function accepts      */
   /* input the GATT Event Handler ID (return value from                */
   /* _GATM_RegisterGATTEvents() function).                             */
int _GATM_UnRegisterGATTEvents(unsigned int GATTEventHandlerID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine if there are currently any connected GATT    */
   /* Devices.  This function accepts a pointer to a buffer that will   */
   /* receive any currently connected GATT devices.  The first parameter*/
   /* specifies the maximum number of GATM_Connection_Information_t     */
   /* entries that the buffer will support (i.e.  can be copied into the*/
   /* buffer).  The next parameter is optional and, if specified, will  */
   /* be populated with the total number of connected GATT devices if   */
   /* the function is successful.  The final parameter can be used to   */
   /* retrieve the total number of connected devices (regardless of the */
   /* size of the list specified by the first two parameters).  This    */
   /* function returns a non-negative value if successful which         */
   /* represents the number of connected devices that were copied into  */
   /* the specified input buffer.  This function returns a negative     */
   /* return error code if there was an error.                          */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Connection List Entries, in which case the final         */
   /*          parameter *MUST* be specified.                           */
int _GATM_QueryConnectedDevices(unsigned int MaximumConnectionListEntries, GATM_Connection_Information_t *ConnectionList, unsigned int *TotalNumberConnectedDevices);

   /* GATM Client APIs.                                                 */

   /* The following function is provided to allow a mechanism for local */
   /* modules to read a specified Attribute on the specified remote     */
   /* device.  The function accepts the GATT Event Handler ID of the    */
   /* registered event handler to call with the result of the read, the */
   /* BD_ADDR of the Connection to read the value from, the Attribute   */
   /* Handle of the attribute on the remote device to read, the number  */
   /* of bytes into the attribute to read, and a Boolean which if TRUE  */
   /* specifies that the entire value should be read or if FALSE just   */
   /* the value that will fit into 1 packet.  This function returns a   */
   /* positive non-zero value if successful, or a negative return error */
   /* code if there was an error.                                       */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
   /* * NOTE * The Offset parameter can be used to read the specified   */
   /*          attribute at the offset specified by the parameter.      */
int _GATM_ReadValue(unsigned int GATTEventHandlerID, BD_ADDR_t BD_ADDR, Word_t AttributeHandle, unsigned int Offset, Boolean_t ReadAll);

   /* The following function is provided to allow a mechanism for local */
   /* modules to write a specified Attribute on the specified remote    */
   /* device.  TThe function accepts the GATT Event Handler ID of the   */
   /* registered event handler to call with the result of the read, the */
   /* BD_ADDR of the Connection to read the value from, the Attribute   */
   /* Handle of the attribute on the remote device to write, the length */
   /* of the data and a pointer to the data to write.  This function    */
   /* returns a positive non-zero value if successful, or a negative    */
   /* return error code if there was an error.                          */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the event that  */
   /*          is received in response to this call.                    */
int _GATM_WriteValue(unsigned int GATTEventHandlerID, BD_ADDR_t BD_ADDR, Word_t AttributeHandle, unsigned int DataLength, Byte_t *Data);

   /* The following function is provided to allow a mechanism for local */
   /* modules to perform a write without response to a specified        */
   /* Attribute on the specified remote device.  The function accepts   */
   /* the GATT Event Handler ID of the registered event handler to call */
   /* with the result of the read, the BD_ADDR of the Connection to read*/
   /* the value from, the Attribute Handle of the attribute on the      */
   /* remote device to write, a Boolean that specifies if a Signed Write*/
   /* is to be performed (if TRUE), the length of the data and a pointer*/
   /* to the data to write.  This function returns the number of bytes  */
   /* that were successfully sent to the remote device, or a negative   */
   /* return error code if there was an error.                          */
   /* * NOTE * No event is generated by this function.                  */
   /* * NOTE * A Signed Write can only be performed to a previously     */
   /*          paired device.                                           */
int _GATM_WriteValueWithoutResponse(unsigned int GATTEventHandlerID, BD_ADDR_t BD_ADDR, Word_t AttributeHandle, Boolean_t PerformSignedWrite, unsigned int DataLength, Byte_t *Data);

   /* GATM Server APIs.                                                 */

   /* The following function is provided to allow a mechanism for local */
   /* modules to register a persistent UID (unique ID) for GATM Service */
   /* Registration.  The first parameter to this function is the number */
   /* of attributes that will be registered in the call to              */
   /* GATM_RegisterService().  The second parameter is a pointer to     */
   /* return the newly registered UID.  The final parameter is an       */
   /* optional pointer to store the handle range in the GATT database   */
   /* that the Persistent UID is allocated to (that is the range in the */
   /* GATT database that the persistent service will reside when it has */
   /* been published).  This function returns ZERO on success (in which */
   /* case PersistentUIDResult will contain a valid persistent UID) or a*/
   /* negative error code.                                              */
   /* * NOTE * The purpose of a persistent UID is to ensure that a      */
   /*          service is always registered at the same location in the */
   /*          local GATT database.  As long as the UID is registered   */
   /*          the application that registers it is ensured that no     */
   /*          other service will use it's allocated handle range       */
   /*          regardless of whether or not the service that is located */
   /*          at the handle range is published.                        */
   /* * NOTE * The NumberOfAttributes parameter MUST match the          */
   /*          NumberOfAttributes parameter passed to                   */
   /*          GATM_RegisterService() otherwise GATM_RegisterService()  */
   /*          will return an error code.                               */
int _GATM_RegisterPersistentUID(unsigned int NumberOfAttributes, DWord_t *PersistentUIDResult, GATT_Attribute_Handle_Group_t *ServiceHandleRangeResult);

   /* The following function is a utility function which is used to     */
   /* unregistered the specified persistent UID.  The only parameter to */
   /* this function is the persistent UID that was registered with the  */
   /* successful call to GATM_RegisterPersistentUID().  This function   */
   /* returns ZERO on success or a negative error code.                 */
int _GATM_UnRegisterPersistentUID(DWord_t PersistentUID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to register a GATT service in the GATT database.  The     */
   /* first parameter to this function is a Boolean that specifies if   */
   /* the registered service is to be a primary service (TRUE) or a     */
   /* secondary service (FALSE).  The second parameter specifies the    */
   /* number of attributes that the service is to contain.  The third   */
   /* parameter parameter specifies the UUID of the service to register.*/
   /* The final parameter specifies an optional UUID that is used to    */
   /* ensure that a service is always given the same handle range in the*/
   /* GATT database.  This function returns a positive, non-zero, value */
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * A successful return value represents the Service ID that */
   /*          can be used to add attributes to the registered service  */
   /*          and can be used to publish the service.                  */
   /* * NOTE * A registered service is not visible to remote devices    */
   /*          until it has been published with the                     */
   /*          GATM_PublishService() API.                               */
   /* * NOTE * The NumberOfAttributes parameter must satisfy the        */
   /*          following formula:                                       */
   /*                                                                   */
   /*              NumberOfAttributes = <NumberOfIncludes>            + */
   /*                                   (<NumberOfCharacteristics>*2) + */
   /*                                   <NumberOfDescriptors>           */
   /*                                                                   */
   /*          Where                                                    */
   /*                                                                   */
   /*          <NumberOfIncludes> is the number of include              */
   /*             definitions contained in the service.                 */
   /*          <NumberOfCharacteristics> is the number of               */
   /*             characteristics contained in the service.             */
   /*          <NumberOfDescriptors> is the total number of descriptors */
   /*             contained in the service.                             */
   /*                                                                   */
   /* * NOTE * The PersistentUID parameter is optional.  If non-NULL    */
   /*          then this function will use the handle range in the GATT */
   /*          database that has been allocated for this service.  If   */
   /*          NULL any available handle range in the GATT database will*/
   /*          be used.                                                 */
int _GATM_RegisterService(Boolean_t PrimaryService, unsigned int NumberOfAttributes, GATT_UUID_t *ServiceUUID, DWord_t *PersistentUID);

   /* The following function is used to add an Include Definition to a  */
   /* registered (but not published) GATT Service.  The first parameter */
   /* to this function is the ServiceID of the registered (but not      */
   /* published) service to add the include to.  The second is the      */
   /* Attribute Offset to place the Include Definition at in the service*/
   /* table.  The final parameter of this function is the ServiceID of  */
   /* the service that is to be include.  On success this function will */
   /* return ZERO or a negative error code on failure.                  */
   /* * NOTE * The Flags parameter is currently unused and may be       */
   /*          ignored.                                                 */
   /* * NOTE * The AttributeOffset contains the offset in the service to*/
   /*          place the include definition.  Include Definitions should*/
   /*          always be in the service table before any other          */
   /*          characteristics or descriptors.                          */
   /* * NOTE * To Calculate the AttributeOffset apply the following     */
   /*          formula:                                                 */
   /*                                                                   */
   /*             AttributeOffset = 1 + (NumPrevIncludes * 1) +         */
   /*                               (NumPrevCharacteristics * 2) +      */
   /*                               (NumPrevDescriptors * 1)            */
   /*                                                                   */
   /*          where:                                                   */
   /*                                                                   */
   /*             NumPrevIncludes = The number of previous Include      */
   /*                               Definition that exist in the        */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevCharacteristics = The number of previous       */
   /*                               Characteristics that exist in the   */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevDescriptors = The number of previous           */
   /*                               Descriptors that exist in the       */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /* * NOTE * The final parameter must be the ServiceID of a service   */
   /*          that has ALREADY been registered AND published.          */
int _GATM_AddServiceInclude(unsigned long Flags, unsigned int ServiceID, unsigned int AttributeOffset, unsigned int IncludedServiceServiceID);

   /* The following function is used to add a Characteristic Declaration*/
   /* (and the Characteristic Value attribute) to a registered (but not */
   /* published) GATT Service.  The first parameter to this function is */
   /* the ServiceID of the Service to add the Characteristic Declaration*/
   /* (and the Characteristic Value) to.  The second is the Attribute   */
   /* Offset to place the Characteristic Declaration (the Characteristic*/
   /* Value will be at AttributeOffset+1).  The third parameter is the  */
   /* characteristic properties mask.  The fourth parameter is the      */
   /* Security Properties of the Characteristic.  The fifth parameter to*/
   /* this function is a pointer to the Characteristic Value UUID.  On  */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
   /* * NOTE * To Calculate the AttributeOffset apply the following     */
   /*          formula:                                                 */
   /*                                                                   */
   /*             AttributeOffset = 1 + (NumPrevIncludes * 1) +         */
   /*                               (NumPrevCharacteristics * 2) +      */
   /*                               (NumPrevDescriptors * 1)            */
   /*                                                                   */
   /*          where:                                                   */
   /*                                                                   */
   /*             NumPrevIncludes = The number of previous Include      */
   /*                               Definition that exist in the        */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevCharacteristics = The number of previous       */
   /*                               Characteristics that exist in the   */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevDescriptors = The number of previous           */
   /*                               Descriptors that exist in the       */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
int _GATM_AddServiceCharacteristic(unsigned int ServiceID, unsigned int AttributeOffset, unsigned long CharacteristicPropertiesMask, unsigned long SecurityPropertiesMask, GATT_UUID_t *CharacteristicUUID);

   /* The following function is used to add a Characteristic Descriptor */
   /* to a registered (but not published) GATT Service.  The first      */
   /* parameter to this function is the Service ID of the Service to add*/
   /* the Characteristic Descriptor to.  The second is the Attribute    */
   /* Offset to place the Characteristic Descriptor.  The third         */
   /* parameter is the desciptor properties mask.  The fourth parameter */
   /* is the Security Properties of the Descriptor.  The fifth parameter*/
   /* to this function is a pointer to the Descriptor UUID.  On success */
   /* this function will return ZERO or a negative error code on        */
   /* failure.                                                          */
   /* * NOTE * To Calculate the AttributeOffset apply the following     */
   /*          formula:                                                 */
   /*                                                                   */
   /*             AttributeOffset = 1 + (NumPrevIncludes * 1) +         */
   /*                               (NumPrevCharacteristics * 2) +      */
   /*                               (NumPrevDescriptors * 1)            */
   /*                                                                   */
   /*          where:                                                   */
   /*                                                                   */
   /*             NumPrevIncludes = The number of previous Include      */
   /*                               Definition that exist in the        */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevCharacteristics = The number of previous       */
   /*                               Characteristics that exist in the   */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevDescriptors = The number of previous           */
   /*                               Descriptors that exist in the       */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
int _GATM_AddServiceDescriptor(unsigned int ServiceID, unsigned int AttributeOffset, unsigned long DescriptorPropertiesMask, unsigned long SecurityPropertiesMask, GATT_UUID_t *DescriptorUUID);

   /* The following function is used to add or update semi-static data  */
   /* that is stored in a service table.  The first parameter to this   */
   /* function is the Service ID of the Service to add the semi-static  */
   /* data to.  The second parameter contains the attribute offset of   */
   /* the characteristic or descriptor to update the semi-static data   */
   /* for.  The final two parameters contain the attribute data length  */
   /* length and a pointer to the attribute data to update the          */
   /* semi-static data to.  On success this function will return ZERO or*/
   /* a negative error code on failure.                                 */
   /* * NOTE * Attribute Data can only be added to Characteristic Value */
   /*          or Characteristic Descriptor attributes.                 */
   /* * NOTE * If attribute data is added to a characteristic           */
   /*          value/descriptor value then we will respond internally to*/
   /*          read requests by client's attempting to read the value   */
   /*          (if the client that is reading the value matches the     */
   /*          specified security properties).  Write requests will     */
   /*          never be responded to internally.                        */
   /* * NOTE * To Calculate the AttributeOffset apply the following     */
   /*          formula:                                                 */
   /*                                                                   */
   /*             AttributeOffset = 1 + (NumPrevIncludes * 1) +         */
   /*                               (NumPrevCharacteristics * 2) +      */
   /*                               (NumPrevDescriptors * 1)            */
   /*                                                                   */
   /*          where:                                                   */
   /*                                                                   */
   /*             NumPrevIncludes = The number of previous Include      */
   /*                               Definition that exist in the        */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevCharacteristics = The number of previous       */
   /*                               Characteristics that exist in the   */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
   /*                                                                   */
   /*             NumPrevDescriptors = The number of previous           */
   /*                               Descriptors that exist in the       */
   /*                               service table prior to the attribute*/
   /*                               (Include, Characteristic or         */
   /*                               Descriptor) that is being added.    */
int _GATM_AddServiceAttributeData(unsigned int ServiceID, unsigned int AttributeOffset, unsigned int AttributeDataLength, Byte_t *AttributeData);

   /* The following function is used to publish a previously registered */
   /* GATT Service.  The first parameter to this function is the        */
   /* ServiceID of the previously registered GATT Service.  The second  */
   /* parameter specifies the EventCallbackID of the callback that will */
   /* be dispatched events when a remote device tries to access the     */
   /* published GATT service.  The third parameter is a bit mask that is*/
   /* used to indicate properties of the service.  The final parameter  */
   /* is a pointer to a structure to store the handle range of the      */
   /* registered service if this function is successful.  On success    */
   /* this function will return ZERO or a negative error code on        */
   /* failure.                                                          */
   /* * NOTE * Once this function is called no Includes, Characteristics*/
   /*          or Descriptors can be added to the service.              */
   /* * NOTE * Once this function is called the registered service will */
   /*          be "published" and any connected Client will be able to  */
   /*          discover the service (if it is a primary service).       */
   /* * NOTE * The ServiceHandleRange parameter is optional and may be  */
   /*          set to NULL.                                             */
int _GATM_PublishService(unsigned int ServiceID, unsigned int GATTEventHandlerID, unsigned long ServiceFlags, GATT_Attribute_Handle_Group_t *ServiceHandleRange);

   /* The following function is used to delete a previously registered  */
   /* service.  The only parameter to this function is the Service of   */
   /* the previously registered (and/or published) GATT Service.  On    */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
   /* * NOTE * If this function is called to delete a published GATT    */
   /*          Service then connected Clients will no longer be able to */
   /*          discover the deleted GATT Service.                       */
int _GATM_DeleteService(unsigned int ServiceID);

   /* The following function is provided to allow a mechanism for local */
   /* modules to determine the services that are published.  This       */
   /* function accepts a pointer to a buffer that will receive any      */
   /* currently published GATT services.  The first parameter specifies */
   /* the maximum number of GATM_Service_Information_t entries that the */
   /* buffer will support (i.e.  can be copied into the buffer).  The   */
   /* next parameter is optional and, if specified, will be populated   */
   /* with the total number of published services.  The next parameter  */
   /* is an optional parameter which may be used to only return services*/
   /* with a specific UUID.  The final parameter can be used to retrieve*/
   /* the total number of published services (regardless of the size of */
   /* the list specified by the first two parameters).  This function   */
   /* returns a non-negative value if successful which represents the   */
   /* number of published services that were copied into the specified  */
   /* input buffer.  This function returns a negative return error code */
   /* if there was an error.                                            */
   /* * NOTE * If ServiceUUID is NULL then any published services will  */
   /*          be returned.                                             */
   /* * NOTE * It is possible to specify zero for the Maximum Number of */
   /*          Published Services Entries, in which case the final      */
   /*          parameter *MUST* be specified.                           */
int _GATM_QueryPublishedServices(unsigned int MaximumPublishedServicesListEntries, GATM_Service_Information_t *PublishedServiceList, GATT_UUID_t *ServiceUUID, unsigned int *TotalNumberPublishedServices);

   /* The following function is used to send a Handle Value Indication  */
   /* to a specified remote device.  The first parameter is the ervice  */
   /* ID of the previously published service whose attribute is being   */
   /* indication.  The second parameter is the BD_ADDR of the connected */
   /* device to write the value to.  The third parameter contains the   */
   /* attribute offset of the value that is being indicated.  The final */
   /* two parameters contain the length of the data and a pointer to the*/
   /* data to indicate.  This function returns a positive non-zero value*/
   /* if successful, or a negative return error code if there was an    */
   /* error.                                                            */
   /* * NOTE * The successful return value from this function is the    */
   /*          TransactionID which can be used to track the handle value*/
   /*          confirmation that is received in response to this call.  */
int _GATM_SendHandleValueIndication(unsigned int ServiceID, BD_ADDR_t RemoteDevice, unsigned int AttributeOffset, unsigned int ValueDataLength, Byte_t *ValueData);

   /* The following function is used to send a Handle Value Notification*/
   /* to a specified remote device.  The first parameter is the Service */
   /* ID of the previously published service whose attribute is being   */
   /* indication.  The second parameter is the BD_ADDR of the connected */
   /* device to write the value to.  The third parameter contains the   */
   /* attribute offset of the value that is being indicated.  The final */
   /* two parameters contain the length of the data and a pointer to the*/
   /* data to indicate.  On success this function will return ZERO or a */
   /* negative error code on failure.                                   */
int _GATM_SendHandleValueNotification(unsigned int ServiceID, BD_ADDR_t RemoteDevice, unsigned int AttributeOffset, unsigned int ValueDataLength, Byte_t *ValueData);

   /* The following function is used to respond with success to a Write */
   /* or Prepare Write Request from a connected client.  The only       */
   /* parameter to this function is the RequestID of the request that is*/
   /* being responded to.  On success this function will return ZERO or */
   /* a negative error code on failure.                                 */
int _GATM_WriteResponse(unsigned int RequestID);

   /* The following function is a utility function that is used to      */
   /* respond to a Read Request from a connected client.  The first     */
   /* parameter to this function is the RequestID of the request that is*/
   /* being responded to.  The final parameters to this function specify*/
   /* the length and a pointer to the data to respond with.  On success */
   /* this function will return ZERO or a negative error code on        */
   /* failure.                                                          */
int _GATM_ReadResponse(unsigned int RequestID, unsigned int DataLength, Byte_t *Data);

   /* The following function is a utility function that is used to      */
   /* respond with an error to a Write or a Read Requst.  The first     */
   /* parameter to this function is the RequestID of the request that is*/
   /* being responded to.  The final parameters to this function        */
   /* specifies the Attribute Protocol Error Code to respond with.  On  */
   /* success this function will return ZERO or a negative error code on*/
   /* failure.                                                          */
int _GATM_ErrorResponse(unsigned int RequestID, Byte_t ErrorCode);

#endif
