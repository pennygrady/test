/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering Center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Core.h
* DESCRIPTION: header file containing the declarations related to OS error mechanism
*             and core constants macro definitions, and core functions' declaration      
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0         Wei.Chen      4/DEC/03       Initial version. 
* 1.1     Hangping.Pan    20/JAN/05 
*
*********************************************************************************
*/
#ifndef OS_CORE_H
#define OS_CORE_H
    
    #include "Std_Types.h"

    /*Default APPMODE.*/
    #define OSDEFAULTAPPMODE (AppModeType)1     
    /*Lowest priority.*/
    #define OS_LOWEST_PRIO      0U
    /*Id of Idle Task.*/
    #define OS_TASK_IDLE_ID     (OS_LOWEST_PRIO)    
    /*Priority of Idle Task.*/
    #define OS_IDLE_PRIO       (OS_LOWEST_PRIO)         
    //Alarm Type
    #define COMMON_TYPE      0x00U
    #define SCHETBL_TYPE     0x01U
    #define RESOURCE_TYPE    0x02U
    

    
    #if (OS_INTERRUPT_MODULE == STD_ON) || (OS_ALARM_MODULE == STD_ON)
        extern uint8 IntNesting;
        #define NOT_IN_ISR  (0U == IntNesting)
    #else
        #define NOT_IN_ISR  TRUE
    #endif 

    typedef uint32 AppModeType; /* type definition of Application mode */
    typedef uint32 EventMaskType;
    typedef EventMaskType *EventMaskRefType;
    typedef uint16 ObjectAccessType;
    typedef uint16 ApplicationType;
    typedef uint16 ObjectTypeType;
    typedef uint16 RestartType;  
      
    typedef uint16 OsApplicationIDType, *OsApplicationIDTypeRef;
    typedef uint16 OsAlarmIDType ,*OsAlarmIDTypeRef;
    typedef uint16 OsCounterIDType ,*OsCounterIDTypeRef;
    typedef uint16 OsIsrIDType ,*OsIsrIDTypeRef;
    typedef uint16 OsScheduleTableIDType ,*OsScheduleTableIDTypeRef;
   
    typedef uint8 TaskIdType,*TaskIdTypeRef ;
    typedef uint16 OsResourceIndex,*OsResourceIndexRef;       /*Index of resource.*/

    typedef uint32 ResourceMaskType;
    typedef ResourceMaskType *ResourceMaskRefType;

    typedef uint32 TickType;
    typedef uint8 ScheduleTableType;
    
    
    extern AppModeType ActiveAppMode; 
    /*System stack.*/
    #if (OS_INTERRUPT_MODULE == STD_ON) || (OS_ALARM_MODULE == STD_ON)
        extern OSSTKRef SystemSTKTopPtr;
    #endif 
    extern uint8 const OSMapTbl[];
    extern uint8 const OSUnMapTbl[];
    extern OSSTK OSTaskIdleStk[];

    
    void StartOS(AppModeType mode);
    AppModeType GetActiveApplicationMode(void);
    void ShutdownOS(StatusType Error);
    void OSTaskIdle(void);


#endif /*ifndef     OS_CORE_H*/
