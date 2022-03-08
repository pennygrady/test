/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of Zhejiang University
* All rights reserved.
*
* FILE NAME: OS_CPU_Types.h
* DESCRIPTION: task's type definition header file for MPC5634
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0           Wei.Chen      4/DEC/03       Initial version.   
* 1.1    HangPing.Pan&Jiang.Li 17/JAN/05   
* 1.2           Jiang.Li      17/MAY/05   
*
*********************************************************************************
*/

#ifndef OS_CPU_TYPES_H_
#define OS_CPU_TYPES_H_
/******************************************************************
| includes
| 1) system and project includes
| 2) needed interfaces from external units
| 3) internal and external interfaces from this unit
|-----------------------------------------------------------------*/
#include "Integration.h"
#include "os_core.h"
#if(OS_MODULE == STD_ON)
    #include "Std_Types.h"
    #include "Os_Cfg.h"
    
/*******************************************************************
| typedefs and structures (scope: module-local)
|------------------------------------------------------------------*/
#if (OS_RESOURCE_MODULE == STD_ON)       
    typedef struct OSResCB *OSRCBRef;
#endif 

    typedef uint8 PriorityType;
    typedef sint8 ResPrioType;
    typedef void(*OSTaskEntry)(void);
    
    typedef struct OSTCB *TaskType;
    typedef TaskType *TaskRefType;
    typedef uint8 TaskStateType;
    typedef TaskStateType *TaskStateRefType;
    typedef float32 SecondType;
    
    typedef enum{FULL=0x10,NON} OsTaskSchedule;        /*Schedule method of the tasks*/
    typedef struct{AppModeType OsTaskAppModeRef;} OsTaskAutostart;        /*Application Mode of the autostart tasks*/
   
 #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON) 
    typedef struct TaskResourceLock /* lock time of resource */
    {
      SecondType OSTaskResourceBudget;/*the max lock time*/
      OsResourceIndex OsTaskResourceRef;/*resource mask*/ 
    }TaskResourceLock,* OsTaskResourceLockRef;       
     
    typedef struct
    {
      // SecondType OsTaskAllInterruptLockBudget;/*TODO*/
       SecondType OsTaskExecutionBudget;        
      // SecondType OSTaskOsInterruptLockBudget;/*TODO*/
       SecondType OsTaskTimeFrame; 
       OsTaskResourceLockRef OsTaskResourceLock;
    }OsTaskTimingProtection;
    
 #endif
    
  
    typedef struct 
    {/*task configuration struct*/
        OSTaskEntry PTask; /*Task function pointer*/
        OSSTKRef PStkHigh; /*the top pointer of the task stack*/
        OSSTKRef PStkLow; /* the bottom pointer of the task stack*/
        PriorityType OsTaskPriority; /*original priority of the task*/ 
    #if (OS_RESOURCE_MODULE == STD_ON)
        PriorityType RunPrio;       
    #endif
        
        uint8 OsTaskActivation;       
        OsTaskSchedule TaskSchedule;/*task scheduleing method*/        
        boolean Autostart; /*Autostart or not*/        
        EventMaskType OsTaskEventRef;  /*event list*/
        ResourceMaskType OsTaskResourceRef; /*resource list*/  
        OsTaskAutostart  TaskAutostartMode;       /*in which Application Mode that the task can autostart*/       
    #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON) 
        OsTaskTimingProtection TaskTimingProtection; /*when there are trusted application,then shall do timing protection*/
    #endif       
    
    #if (OS_APPLICATION_MODULE == STD_ON)
         OsApplicationIDTypeRef  OsTaskAccessingApplication;
    #endif
       
    }OsTask,*OsTaskRef,OSTaskCfg;


   typedef struct OSTCB
    {
        OSSTKRef OSTCBStkCur; /* stack pointer  */
        OSTaskEntry PTask;    /* task function entry pointer*/
        OSSTKRef OSTCBStkTop; /* the too pointer of the task*/

        #if (OS_EVENT_MODULE == STD_ON)
            EventMaskType WEvent; /* mask of waited events         */
            EventMaskType SEvent; /* mask of setted events         */
        #endif /*#ifdef OS_EVENT_EN*/

        #if (OS_STACK_MONITORING_MODULE == STD_ON)  
            OSSTKRef OSTCBStkBottom; /* the Bottom pointer of stack                */
        #endif /*(OS_STACK_MONITORING_MODULE)*/

        #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON) 
            OSRCBRef resources; /* head of occupied external resource list           */
        #endif /*if defined(OS_OUT_RES)||defined(OS_RES_SCHEDULER)*/

        #if (OS_RES_INTERNAL == STD_ON)
            TaskType SavedTCB; /*pointer to the original task tcb that saved at this location   */
        #endif
        
        #if (OS_RES_INTERNAL == STD_ON) || (OS_RES_EXTERNAL == STD_ON)||(OS_RES_SCHEDULER == STD_ON)
            PriorityType RunPrio; /* the run priority of the task        */
        #endif /*#if defined(OS_INT_RES)    *//*||defined(OS_OUT_RES)*/    

        PriorityType OrigPrio; /*the original priority of task*/        
        uint8 OSTCBStat; /* status of the task    */


        uint8 OSTCBBitX; /*mask that used to access the ready list X-Index */
        uint8 OSTCBBitY; /*mask that used to access the ready list Y-Index */

      
        TaskIdType OSTCBId; /* Task ID   */
        uint8 MaxAct;              /*max activation*/
        uint8 CurAct;                          /*current activation*/
        #if (OS_PROTECTION_HOOK == STD_ON)
          uint8 InfoBit2;  //the information of the protection hook //TODO
        #endif 
        uint8 InfoBit; /*the information the task:
        1Bit:(lower bit) reserved
        2Bit:reserved
        3Bit:is it the first time of the task to run
        4Bit:Does task has occupied internal resource right now
        5Bit:Scheduling Method
        6Bit:AutoStart
        7Bit:whether task applies for internal resource
        8Bit:extended task*/
        
              EventMaskType OsTaskEventRef;  /*the event list of the task*/
        ResourceMaskType OsTaskResourceRef; /*the resource list of the task*/  
      
       #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)            
        TickType OsTaskExecutionBudget;  
        TickType RemainingExecutionBudget; 
        TickType OsTaskTimeFrame; 
        TickType TimeFrameStart;
        uint32  CrossNum;     
        OsTaskResourceLockRef OsTaskResourceLock;
    #endif 
        
    #if (OS_APPLICATION_MODULE == STD_ON)
        OsApplicationIDTypeRef  OsTaskAccessingApplication;
        OsApplicationIDType     OsTaskOwnershipApp;
    #endif   
    } OSTCB;


      
      typedef struct ReadyListCB     
      {
      
        uint8  *head;        /*the head of the same priority array*/
        uint8  start;   /*the valid start index of the priority array*/
        uint8  count;        /*element num of the same priortiy array*/
        uint8  OSPrioBitX;  /*mask that used to access the ready list X-Index*/
        uint8  OSPrioBitY;  /*mask that used to access the ready list Y-Index*/ 
      
      }ReadyListCB;
      
      typedef struct ReadyListCfg
      {
        uint8 *head;    /*the head of the same priority array*/
        uint8 length;  /*the max array size of the ReadyList*/
      }ReadyListCfg;
            


    
    
    

#endif    /*#if(OS_MODULE == STD_ON)*/
#endif /* OS_CPU_TYPES_H_ */
