/*****< btpmsrvr.c >***********************************************************/
/*      Copyright 2010 - 2014 Stonestreet One.                                */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMSRVR - Bluetopia Platform Manager Main Application Entry point for    */
/*             Linux.                                                         */
/*                                                                            */
/*  Author:  Damon Lange                                                      */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   07/07/10  D. Lange        Initial creation.                              */
/******************************************************************************/
#include <stdio.h>

#include "SS1BTPM.h"             /* BTPM API Prototypes and Constants.        */
#include "BTPMSRVR.h"            /* BTPM Main Application Proto./Constants.   */

   /* Main Program Entry Point.                                         */
int main(int argc, char *argv[])
{
   int                                ret_val;
   BTPM_Initialization_Info_t         InitializationInfo;
   BTPM_Debug_Initialization_Data_t   DebugInitializationInfo;
   DEVM_Initialization_Data_t         DeviceManagerInitializationInfo;
   DEVM_Default_Initialization_Data_t DefaultInitializationData;
#if BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_SUPPORT_MFI
   SPPM_Initialization_Data_t         SerialPortManagerInitializationInfo;
#endif
#if 0

   /* This is the bare minimum Server initialization.  Initialize:      */
   /*    - All default values.                                          */
   ret_val = BTPM_Main(NULL, NULL, NULL);

#endif

   /* Let's go ahead and specify a Platform Specific debugging string.  */
   BTPS_MemInitialize(&DebugInitializationInfo, 0, sizeof(DebugInitializationInfo));
   DebugInitializationInfo.PlatformSpecificInitData = "SS1BTPMS";
   /* In order to Enable Version printing, use 				*/
   /* BTPM_DEBUG_LEVEL_FORCE_PRINT					*/
   /* In order to Disable Version printing, use 			*/
   /* BTPM_DEBUG_LEVEL_VERBOSE						*/
   DebugInitializationInfo.PrintVersionFlag = BTPM_DEBUG_LEVEL_FORCE_PRINT;
#if BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_SUPPORT_MFI
   /* Initialize the SPPM Manger initialization  add MFI support*/
   BTPS_MemInitialize(&SerialPortManagerInitializationInfo, 0, sizeof(SerialPortManagerInitializationInfo));
   SerialPortManagerInitializationInfo.ConfigurationFlags |= SPPM_INITIALIZATION_CONFIGURATION_FLAGS_SUPPORT_MFI;
#endif

   /* Initialize the DEVM default initialization parameters.            */
   BTPS_MemInitialize(&DefaultInitializationData, 0, sizeof(DefaultInitializationData));
   
   DefaultInitializationData.InitializationOverrideFlags                                       |= DEVM_INITIALIZATION_DATA_OVERRIDE_FLAGS_DEVM_ALLOW_LE_PARAM_DATA;
   DefaultInitializationData.DefaultLEAllowableParameters.Flags                                 = DEVM_DEFAULT_ALLOWABLE_LE_PARAMETERS_MINIMUM_INTERVALS_VALID;
   DefaultInitializationData.DefaultLEAllowableParameters.Minimum_Minimum_Interval              = BTPM_CONFIGURATION_DEVICE_MANAGER_DEFAULT_MIN_ALLOWABLE_MIN_CONNECTION_INTERVAL;
   DefaultInitializationData.DefaultLEAllowableParameters.Minimum_Maximum_Interval              = BTPM_CONFIGURATION_DEVICE_MANAGER_DEFAULT_MIN_ALLOWABLE_MAX_CONNECTION_INTERVAL;
                                                                                               
   DefaultInitializationData.InitializationOverrideFlags                                       |= DEVM_INITIALIZATION_DATA_OVERRIDE_FLAGS_PAIRABILITY_MODE_SSP;
   DefaultInitializationData.DefaultPairabilityModeSSP                                          = TRUE;
   /* The Default Enhanced SCO parameters initialization. Effective just for WL18xx.  */
   DefaultInitializationData.InitializationOverrideFlags                                       |= DEVM_INITIALIZATION_DATA_OVERRIDE_FLAGS_SCO_ENHANCED_PARAMETERS;
   DefaultInitializationData.DefaultSCOEnhancedTransportParameters.DataPath                     = 1;
   DefaultInitializationData.DefaultSCOEnhancedTransportParameters.mSBCDataPathHCI              = FALSE;
   DefaultInitializationData.DefaultSCOEnhancedTransportParameters.PCMSamplePayloadMSBPosition  = 1;  
   DefaultInitializationData.DefaultSCOEnhancedTransportParameters.TransportUnitSize            = 17;

   /* Initialize the DEVM initialization parameters.                    */
   BTPS_MemInitialize(&DeviceManagerInitializationInfo, 0, sizeof(DeviceManagerInitializationInfo));
   DeviceManagerInitializationInfo.DefaultInitializationData = &DefaultInitializationData;

   /* Now go ahead and initialize the correct global configuration      */
   /* structure.                                                        */
   BTPS_MemInitialize(&InitializationInfo, 0, sizeof(InitializationInfo));
   InitializationInfo.DebugInitializationInfo         = &DebugInitializationInfo;
   InitializationInfo.DeviceManagerInitializationInfo = &DeviceManagerInitializationInfo;
#if BTPM_CONFIGURATION_SERIAL_PORT_MANAGER_SUPPORT_MFI
   InitializationInfo.SerialPortManagerInitializationInfo=&SerialPortManagerInitializationInfo;
#endif
   /* Do nothing other than call the Library entry point.               */
   ret_val = BTPM_Main(&InitializationInfo, NULL, NULL);

   return(ret_val);
}

