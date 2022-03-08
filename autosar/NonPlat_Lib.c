/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering Center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: NonPlat_Lib.c
* DESCRIPTION: The source file for global function.
* UPDATE HISTORY
* REV      AUTHOR      DATE       DESCRIPTION OF CHANGE
* 1.0      Yangzai   05/Jan/11   Initial version.
*
*********************************************************************************
*/
/******************************************************************
| includes
| 1) system and project includes
| 2) needed interfaces from external units
| 3) internal and external interfaces from this unit
|-----------------------------------------------------------------*/
#include "NonPlat_Lib.h"
/**********************************************************************
* Function Prototype:void Lib_Memcpy(uint8 *dest,uint8 *src,uint8 length)
* Service ID:
* Sync/Asyn:
* Re-Entrancy:
* Parameters description
*    (in):ptr1,ptr2,length
*    (out):
* Return Value:
* Caller:
* Call Function:
* Description: copy data from src to dest with size equels length
* Author:
* Time:
***************************************************************************
*/
void Lib_Memcpy(uint8 *dest,uint8 *src,uint8 length)
{
    uint8 i = 0U;
    for(i = 0U; i < length; i++)
    {
       dest[i] = src[i];
    }
    return ;
}
