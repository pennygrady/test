/**
    @file        Platform_Types.h
    @version     1.0.1

    @brief       AUTOSAR Base - Platform dependend data type definitions.
    @details     AUTOSAR platform types header file. It contains all
                 platform dependent types and symbols. Those types must be abstracted in order to
                 become platform and compiler independent.

    Project      : AUTOSAR 3.2 MCAL
    Platform     : PA
    Peripheral   : XPC560XB
    Dependencies : none

    Autosar Version       : 3.2.0
    Autosar Revision      : ASR_REL_3_2_REV_0002
    Autosar Conf. Variant :
    Software Version      : 1.0.1
    Build Version         : MPC560xB_MCAL_1_0_1_RTM_ASR_REL_3_2_REV_0002_20130906

    (c) Copyright 2006-2013 Freescale Semiconductor Inc. & STMicroelectronics
    All Rights Reserved.
*/
/*==================================================================================================
==================================================================================================*/

/**
@file        Platform_Types.h
@remarks Covers PLATFORM003,PLATFORM001
@remarks Implements DBASE05000
*/

/**
@file        Platform_Types.h
@remarks Covers PLATFORM002

         It is not allowed to add any extension to this file. Any extension invalidates the AUTOSAR
         conformity
*/

#ifndef PLATFORM_TYPES_H
#define PLATFORM_TYPES_H

/*==================================================================================================
                                         INCLUDE FILES
 1) system and project includes
 2) needed interfaces from external units
 3) internal and external interfaces from this unit
==================================================================================================*/

/*==================================================================================================
                               SOURCE FILE VERSION INFORMATION
==================================================================================================*/
/**
@{
@brief Parameters that shall be published within the platform types header file and also in the
       module's description file
@remarks Covers PLATFORM012
@remarks Implements DBASE05028
*/
#define PLATFORM_VENDOR_ID          43
#define PLATFORM_AR_MAJOR_VERSION   2
#define PLATFORM_AR_MINOR_VERSION   3
#define PLATFORM_AR_PATCH_VERSION   1
#define PLATFORM_SW_MAJOR_VERSION   1
#define PLATFORM_SW_MINOR_VERSION   0
#define PLATFORM_SW_PATCH_VERSION   1
/**@}*/
/*==================================================================================================
                                      FILE VERSION CHECKS
==================================================================================================*/

/*    Basic types    */
typedef unsigned char   boolean;


typedef signed char     sint8;
typedef unsigned char   uint8;
typedef signed short    sint16;
typedef unsigned short  uint16;
typedef signed long     sint32;
typedef unsigned long   uint32;

typedef signed long     sint8_least;
typedef unsigned long   uint8_least;
typedef signed long    sint16_least;
typedef unsigned long  uint16_least;
typedef signed long      sint32_least;
typedef unsigned long    uint32_least;
typedef float           float32;
typedef double          double64;
typedef double          float64;

typedef uint16          AddrType; /*Address Type*/
typedef unsigned int    OSSTK; /*Stack Type*/
typedef OSSTK *OSSTKRef;

#ifndef NULL
#define NULL   ((void *) 0)
#endif

 
   #ifndef ABSOLUTE 
      #define ABSOLUTE 0u
    #endif 
    
    #ifndef RELATIVE
      #define RELATIVE 1u
    #endif 
    
    #ifndef SYNCHRON 
      #define SYNCHRON 2u
    #endif
    
    #ifndef IMPLICIT 
      #define IMPLICIT 0u
    #endif
    
    #ifndef EXPLICIT
        #define EXPLICIT 1u
    #endif

#ifndef MACRO_TRUE
#define MACRO_TRUE     1u
#endif

#ifndef MACRO_FALSE
#define MACRO_FALSE    0u
#endif
 /* constant  */    

#ifndef    TRUE
#define    TRUE    ((boolean)1u)
#endif    /*    TRUE    */

#ifndef    FALSE
#define    FALSE    ((boolean)0u)
#endif    /*    FALSE    */

/*Cpu Length*/
#define    CPU_TYPE_8    8u
#define    CPU_TYPE_16    16u
#define    CPU_TYPE_32    32u

/*Bit Sort*/
#define    MSB_FIRST    0u
#define    LSB_FIRST    1u

/* Byte Sort*/
#define HIGH_BYTE_FIRST 0u
#define LOW_BYTE_FIRST  1u

#define    CPU_TYPE        CPU_TYPE_32
#define    CPU_BIT_ORDER    MSB_FIRST
#define    CPU_BYTE_ORDER    HIGH_BYTE_FIRST

#endif    /*    _PLATFORM_TYPE_H    */
