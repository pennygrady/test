/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Counter.c
* DESCRIPTION: Application module source file based on AUTOSAR OS standard
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE
* 1.0                        10/AUG/09       Initial version.
*
*********************************************************************************
*/
#ifndef OS_COUNTER_C
#define OS_COUNTER_C
#include "Std_Types.h"

#if (OS_COUNTER_INTERFACE_MODULE == STD_ON)

/**********************************************************************
* Function Prototype��IncrementCounter(CounterType CounterID)
* Service ID: OS_COUNTER_SERVICE_ID_BASE + 0x00
* Sync/Asyn:  Sync
* Re-Entrancy: YES
* Parameters description
*    (in): thCounterID        The Counter to be incremented
*    (out): NO
* Return Value: E_OK:successful
*                E_OS_ID (only in EXTENDED status) :The CounterID was not valid or counter is implemented in hardware and can not be incremented by software.
* Caller: If called from a task, rescheduling may take place
* Call Function��
* Description: Available in all Scalability Classes.
* Author:Jiang.Li
* Time:
***************************************************************************
*/
 StatusType IncrementCounter(CounterType CounterID)
 {
   #if (OS_EXTENDED_STATUS == STD_ON)
    if(CounterID<OSCounters||CounterID>=(OSCounters+OS_COUNTER_NUM)||CounterID->CtrInfo->OsCounterType==HARDWARE)
    {
      OSRETERROR(E_OS_ID, OSServiceId_IncrementCounter, CounterID);
    }
   #endif /*OS_EXTENDED_STATUS*/

   OSCounterTrigger(CounterID);             /*increment counter value*/

   return E_OK;

 }
 /*
 StatusType GetCounterValue (     CounterType  CounterID,     TickRefType  Value )
 {

 }

 StatusType GetElapsedCounterValue (     CounterType  CounterID,     TickRefType   Value,     TickRefType  ElapsedValue )
 {

 }
 */

#endif /*OS_COUNTER_INTERFACE_MODULE*/

#endif /*OS_COUNTER_C*/
