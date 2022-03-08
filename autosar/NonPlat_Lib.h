/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering Center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: NonPlat_Lib.h
* DESCRIPTION: The header file for global function.
* UPDATE HISTORY
* REV      AUTHOR      DATE       DESCRIPTION OF CHANGE
* 1.0      Yangzai   05/Jan/11   Initial version.
*
*********************************************************************************
*/
#ifndef NONPLAT_LIB_H
#define NONPLAT_LIB_H
/******************************************************************
| includes
| 1) system and project includes
| 2) needed interfaces from external units
| 3) internal and external interfaces from this unit
|-----------------------------------------------------------------*/
#include "Std_Types.h"
/*******************************************************************
| function prototypes (scope: module-exported)
|------------------------------------------------------------------*/
void Lib_Memcpy(uint8 *ptr1,uint8 *ptr2,uint8 length);
#endif
