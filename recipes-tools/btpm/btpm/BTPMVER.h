/*****< btpmver.h >************************************************************/
/*      Copyright                                                             */
/*      All Rights Reserved.                                                  */
/*                                                                            */
/*  BTPMVER - Texas Instruments Bluetooth Protocol Stack Platform Manager     */
/*            Version Information.                                            */
/*                                                                            */
/*  Author:  Ram Malovany                                                     */
/*                                                                            */
/*** MODIFICATION HISTORY *****************************************************/
/*                                                                            */
/*   mm/dd/yy  F. Lastname    Description of Modification                     */
/*   --------  -----------    ------------------------------------------------*/
/*   12/25/14  R. Malovany       Initial creation.                            */
/*   05/05/15  D. Horowitz    Delete platforms defines and macros		 	  */
/******************************************************************************/
#ifndef __BTPMVER_H__
#define __BTPMVER_H__


   /* Bluetooth Protocol Stack Platform Manager Release Number.         */
#ifndef BTPM_MAJOR_VERSION_NUMBER
   #define BTPM_MAJOR_VERSION_NUMBER                            2
#endif

   /* Bluetooth Protocol Stack Platform Manager Revision Number.        */
#ifndef BTPM_MINOR_VERSION_NUMBER
   #define BTPM_MINOR_VERSION_NUMBER                            1
#endif


   /* Constants used to convert numeric constants to string constants   */
   /* (used in MACRO's below).                                          */
#define BTPM_VERSION_NUMBER_TO_STRING(_x)                   #_x
#define BTPM_VERSION_CONSTANT_TO_STRING(_y)                 BTPM_VERSION_NUMBER_TO_STRING(_y)

   /* Bluetooth Protocol Stack Platform Manager Constant Version String */
   /* of the form:                                                      */
   /*    "a.b"                                                      */
   /* where:                                                            */
   /*    a - BTPM_MAJOR_VERSION_NUMBER                                  */
   /*    b - BTPM_MINOR_VERSION_NUMBER                                  */

#define BTPM_VERSION_VERSION_STRING                         BTPM_VERSION_CONSTANT_TO_STRING(BTPM_MAJOR_VERSION_NUMBER) "." BTPM_VERSION_CONSTANT_TO_STRING(BTPM_MINOR_VERSION_NUMBER)

   /* File/Product/Company Name Copyrights and Descriptions.            */
#define BTPM_VERSION_PRODUCT_NAME_STRING                    "Bluetopia Platform Manager"

#define BTPM_VERSION_COMPANY_NAME_STRING                    "Texas Instruments"

#define BTPM_VERSION_COPYRIGHT_STRING                       "Copyright (C) 2014 Texas Instruments"

#endif
