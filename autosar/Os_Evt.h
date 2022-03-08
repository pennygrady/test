/**********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Evt.h
* DESCRIPTION: Event module header file based on AUTOSAR OS standard 
* UPDATE HISTORY
* REV         AUTHOR                  DATE       DESCRIPTION OF CHANGE                         
* 1.0          Yang.Gu                1/DEC/04      Initial version.   
* 1.1          Shuang.Gao&Jiang.Li  1/MAR/05   
* 1.2          Jiang.Li              22/MAY/10   
*
***********************************************************************************/

#ifndef OS_EVT_H
    #define OS_EVT_H

    #define NOSETEVENT  0U
    #define DeclareEvent(Event) extern EventMaskType Event

    typedef struct
    {
      EventMaskType OsEventMask;
    
    }OsEvent;

    #if (OS_ALARM_MODULE == STD_ON)
        void OSSetEvent(TaskType TaskID, EventMaskRefType Mask);
    #endif /*#ifdef OS_ALARM_EN*/

    
    StatusType SetEvent(TaskType TaskID, EventMaskType Mask);
    StatusType ClearEvent(EventMaskType Mask);
    StatusType GetEvent(TaskType TaskID, EventMaskRefType Event);
    StatusType WaitEvent(EventMaskType Mask);


#endif
