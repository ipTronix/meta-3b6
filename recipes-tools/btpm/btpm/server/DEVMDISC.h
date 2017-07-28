/*****< devmdisc.h >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  DEVMDISC - Device/Service Discovery Module Type Definitions, Prototypes,  */
/*             and Constants.                                                 */
/*                                                                            */
/*  Author:  Tim Thomas                                                       */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/23/10  T. Thomas      Initial creation.                               */
/******************************************************************************/
#ifndef __DEMVDISCH__
#define __DEVMDISCH__

#include "SS1BTPS.h"             /* BTPS Protocol Stack Prototypes/Constants. */

   /* The following structure is used with the                          */
   /* DEVM_DeviceDiscoveryStatistics() function to determine information*/
   /* about the current device discovery operation.                     */
typedef struct _tagDEVM_Device_Discovery_Statistics_t
{
   unsigned int NumberDevicesDiscovered;
   unsigned int NumberNamesDiscovered;
   unsigned int NumberNamesFailed;
} DEVM_Device_Discovery_Statistics_t;

#define DEVM_DEVICE_DISCOVERY_STATISTICS_SIZE                  (sizeof(DEVM_Device_Discovery_Statistics_t))

   /* The following defines the structure that is used to contain all of*/
   /* the information about a device that has been discovered.          */
typedef struct _tagDEVM_Device_Discovery_Info_t
{
   BD_ADDR_t                        BD_ADDR;
   Class_of_Device_t                ClassOfDevice;
   Word_t                           ClockOffset;
   Byte_t                           PageScanRepetitionMode;
   Byte_t                           PageScanPeriodMode;
   Byte_t                           PageScanMode;
   Boolean_t                        FirstDispatch;
   Boolean_t                        RSSIValid;
   Boolean_t                        EIRDataValid;
   int                              RSSI;
   Boolean_t                        NameValid;
   unsigned int                     DeviceNameLength;
   char                             DeviceName[MAX_NAME_LENGTH + 1];
   Extended_Inquiry_Response_Data_t EIRData;
} DEVM_Device_Discovery_Info_t;

#define DEVM_DEVICE_DISCOVERY_INFO_DATA_SIZE                   (sizeof(DEVM_Device_Discovery_Info_t))

   /* Device Manager Discovery Event API Types.                         */
typedef enum
{
   etDeviceInformationIndication,
   etDeviceDiscoveryInActive
} DEVM_Device_Discovery_Event_Type_t;

   /* The following Device Manager Device Discovery Information         */
   /* Indication Event is dispatched when this module has located a new */
   /* device and the information about the new device is determined.    */
typedef struct _tagDEVM_Device_Discovery_Information_Indication_Data_t
{
   DEVM_Device_Discovery_Info_t DeviceInfo;
} DEVM_Device_Discovery_Information_Indication_Data_t;

#define DEVM_DEVICE_DISCOVERY_INFORMATION_INDICATION_DATA_SIZE (sizeof(DEVM_Device_Discovery_Information_Indication_Data_t))

   /* The following structure represents the container structure for    */
   /* Holding all Device Manager Discovery Event Data Data.             */
typedef struct _tagDEVM_Device_Discovery_Event_Data_t
{
   DEVM_Device_Discovery_Event_Type_t EventType;
   union
   {
      DEVM_Device_Discovery_Information_Indication_Data_t DeviceInformationIndicationData;
   } EventData;
} DEVM_Device_Discovery_Event_Data_t;

#define DEVM_DEVICE_DISCOVERY_EVENT_DATA_SIZE                  (sizeof(DEVM_Device_Discovery_Event_Data_t))

   /* The following declared type represents the Prototype Function for */
   /* a Device Manager Device Discovery Event Data Callback.  This      */
   /* function will be called whenever a Discovery Event occurs that is */
   /* associated with the specified Bluetooth Stack ID.  This function  */
   /* passes to the caller the Bluetooth Stack ID, the Discovery Event  */
   /* Data that occurred and the Discovery Event Callback Parameter that*/
   /* was specified when this Callback was installed.  The caller is    */
   /* free to use the contents of the Discovery Event Data ONLY in the  */
   /* context of this callback.  If the caller requires the Data for a  */
   /* longer period of time, then the callback function MUST copy the   */
   /* data into another Data Buffer.  This function is guaranteed NOT to*/
   /* be invoked more than once simultaneously for the specified        */
   /* installed callback (i.e. this function DOES NOT have be           */
   /* reentrant).  It needs to be noted however, that if the same       */
   /* Callback is installed more than once, then the callbacks will be  */
   /* called serially.  Because of this, the processing in this function*/
   /* should be as efficient as possible.  It should also be noted that */
   /* this function is called in the Thread Context of a Thread that the*/
   /* User does NOT own.  Therefore, processing in this function should */
   /* be as efficient as possible (this argument holds anyway because   */
   /* another Discovery Event will not be processed while this function */
   /* call is outstanding).                                             */
   /* ** NOTE ** This function MUST NOT Block and wait for events that  */
   /*            can only be satisfied by Receiving Discovery Events.   */
   /*            A Deadlock WILL occur because NO Discovery Event       */
   /*            Callbacks will be issued while this function is        */
   /*            currently outstanding.                                 */
typedef void (BTPSAPI *DEVM_Device_Discovery_Callback_t)(unsigned int BluetoothStackID, DEVM_Device_Discovery_Event_Data_t *DiscoveryEventData, unsigned long CallbackParameter);

   /* The following function is responsible for initializing a Discovery*/
   /* Context Layer for the specified Bluetooth Protocol Stack.  This   */
   /* function will allocate and initialize an Discovery Context        */
   /* Information structure associated with the specified Bluetooth     */
   /* Stack ID.  This function returns zero if successful, or a non-zero*/
   /* value if there was an error.                                      */
int DEVM_DeviceDiscoveryInitialize(unsigned int BluetoothStackID);

   /* The following function is responsible for releasing any resources */
   /* that the Discovery Layer that is associated with the Bluetooth    */
   /* Protocol Stack specified by the Bluetooth Stack ID, has allocated.*/
   /* Upon completion of this function, ALL Discovery functions will    */
   /* fail if used on the specified Bluetooth Protocol Stack.           */
void DEVM_DeviceDiscoveryCleanup(void);

   /* The following function is used to initiate the Device Discovery   */
   /* process.  The first parameter is a pointer to the callback        */
   /* function that will receive the device information as it becomes   */
   /* available.  The final parameter is a user defined value that will */
   /* be returned to in the Callback Parameter of the callback function.*/
   /* The user is free to use this for any purpose.  The function       */
   /* returns a negative return value if there was an error and a       */
   /* non-zero value if successful (which denotes the current active    */
   /* state).                                                           */
int DEVM_DeviceDiscoveryStart(DEVM_Device_Discovery_Callback_t DeviceDiscoveryCallback, unsigned long DeviceDiscoveryCallbackParameter);

   /* The following function is used to terminate an ongoing Device     */
   /* Discovery process.  The function returns a negative return value  */
   /* if there was an error and non-negative if successful (zero means  */
   /* that the Discovery is In-active, and positive means that a        */
   /* callback will be dispatched when the discovery is in-active).     */
int DEVM_DeviceDiscoveryStop(void);

   /* The following function is used to temporarily suspend any active  */
   /* Device Discovery process.  This function returns the current      */
   /* Active status of the Discovery process (TRUE means active, FALSE  */
   /* means in-active).                                                 */
Boolean_t DEVM_DeviceDiscoverySuspend(Boolean_t Suspend);

   /* The following function is used to query information about the     */
   /* current Discovery Process.  This function returns zero if         */
   /* successful or a negative return error code if there was an error. */
int DEVM_DeviceDiscoveryStatistics(DEVM_Device_Discovery_Statistics_t *Statistics);

   /* The following function is used to inform the Discovery Module that*/
   /* the Device Name is already known for the specified Device Address.*/
   /* This allows the Discovery Module to prioritize devices with names */
   /* that are not know first, then refetch the device names for devices*/
   /* that the name is already know (in case they changed).             */
void DEVM_DeviceDiscoveryNameAlreadyKnown(BD_ADDR_t BD_ADDR);

#endif
