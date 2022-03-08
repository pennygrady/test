/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Std_Types.h
* DESCRIPTION: Defines BSW types, standardized file
* UPDATE HISTORY:
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0         ZhouLi       2011.6.9         Initial version.   
*
*********************************************************************************
*/

#ifndef _STD_TYPES_H
#define _STD_TYPES_H

#include "Platform_Types.h"
//#include "Compiler.h"


/******************************************************************
| defines and macros (scope: module-local)
|-----------------------------------------------------------------*/


#define VENDOR_ID 0x00U

#if(PLATFORM_VENDOR_ID != 43) //43 is MPC563L
#define    ADC_MODULE_ID    (uint16)0X01U
#define    SPI_MODULE_ID    (uint16)0X02U
#define    GPT_MODULE_ID    (uint16)0X03U
#define    PORT_MODULE_ID    (uint16)0X04U
#define    MCU_MODULE_ID    (uint16)0X05U
#define    DIO_MODULE_ID    (uint16)0X06U
#define    PWM_MODULE_ID    (uint16)0X07U
#define    ICU_MODULE_ID    (uint16)0X08U
#define    CAN_MODULE_ID    (uint16)0X09U

#define    FLS_MODULE_ID    (uint16)0X16U
#define WDG_MODULE_ID    (uint16)0X19U
#define FEE_MODULE_ID   (uint16)0X22U
#define DEM_MODULE_ID    (uint16)0x24U
#endif//43 is MPC563L

#define    CANIF_MODULE_ID    (uint16)0X10U
#define    PDUR_MODULE_ID    (uint16)0X11U
#define    COM_MODULE_ID    (uint16)0X12U
#define    CANTP_MODULE_ID    (uint16)0X13U
#define    NVRAM_MODULE_ID    (uint16)0X14U
#define    MEMIF_MODULE_ID    (uint16)0X15U

#define    IOABS_MODULE_ID    (uint16)0X17U
#define DET_MODULE_ID   (uint16)0X18U
#define ECUM_MODULE_ID  (uint16)0X1AU
#define SCHM_MODULE_ID  (uint16)0X1BU

#define WDGIF_MODULE_ID    (uint16)0X20U
#define WDGM_MODULE_ID    (uint16)0X21U



#define DCM_MODULE_ID    (uint16)0x23U


#define    STD_OFF            0 
#define    STD_ON            1

#define STD_ACTIVE        0x01
#define STD_IDLE        0x00

#define STD_HIGH        0x01 
#define STD_LOW            0x00 


#ifndef STATUSTYPEDEFINED
#define STATUSTYPEDEFINED
#endif


typedef unsigned char StatusType; /* OSEK compliance */
#define ISR(FuncName) void FuncName(void) /*ISR Name*/



typedef void(*ISR_FuncPtrType)(void);

#define INVALID_BOOL  FALSE 
#define INVALID_INT   255U 


/*******************************************************************
| typedefs and structures (scope: module-local)
|------------------------------------------------------------------*/

typedef uint8 Std_ReturnType; 
#ifndef E_OK
    #define E_OK            0x00 
    #define E_NOT_OK        0x01
#endif

#define _STATIC_   static 
typedef struct
{
    uint16  vendorID;               /**< @brief vendor ID */
    uint16  moduleID;               /**< @brief BSW module ID */
    uint8   instanceID;             /**< @brief BSW module instance ID */
    uint8   sw_major_version;       /**< @brief BSW module software major version */
    uint8   sw_minor_version;       /**< @brief BSW module software minor version */
    uint8   sw_patch_version;       /**< @brief BSW module software patch version */
} Std_VersionInfoType;



#endif/* _STD_TYPES_H */
