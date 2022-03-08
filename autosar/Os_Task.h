/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Task.h
* DESCRIPTION: Task module header file based on AUTOSAR OS standard 
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0           Wei.Chen      4/DEC/03       Initial version.   
* 1.1    HangPing.Pan&Jiang.Li 17/JAN/05   
* 1.2           Jiang.Li      17/MAY/05   
*
*********************************************************************************
*/
#ifndef OS_TASK_H
    #define OS_TASK_H

#include "Os_Cpu_Types.h"
/*******************************************************************
| defines and macros (scope: module-local)
|------------------------------------------------------------------*/

    #define OS_NUM_SYS_TASK ((OS_TASK_NUM)+1u)
    #define OS_RDY_TBL_SIZE   (((OS_NUM_SYS_TASK) / 8U) + 1U)   /* size of ready table  */
    #define OS_MAX_TASKS 64U /*Max num of tasks*/             

    /*status macro of the tasks*/
    #define READY            0x00U
    #define WAITING          0x02U
    #define SUSPENDED        0x04U
    #define RUNNING          0x01U
    #define RUN_READY        (uint8)(0x10)   /*Task has run, and entered to RUN_Ready status*/ 
    
    /*whether any task of the system has run*/
    #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
       #define TASKHAVERUN      0x04U
    #endif

    #define INVALID_TASK     (NULL)          /* Invalid task*/    
    #define Preemptive    1U /*Task can be interrupted*/
    #define NoPreemptive 0U /*Task can not be interrupted*/   
    #define FULL_SCHED 0x10U
    #define AUTO_START 0x20U
    #define HOLD_INT_RES 0x40U
    #define EXTENDED 0x80U


    /*get the type of task:extended or stardard*/
    #define GET_TASK_TYPE(PTCB) ((PTCB)->InfoBit & EXTENDED)
    /*get the scheduling method of the task*/
    #define GET_TASK_SCHED(PTCB)  ((PTCB)->InfoBit & FULL_SCHED)
    /*extimate whether the task ocuupied the internal resource*/
    #define GET_TASK_HOLD_INT_RES(PTCB) ((PTCB)->InfoBit & HOLD_INT_RES)
    /*set the information of the task*/
    #define SET_TASK_PROPERTY(PTCB,PROPERTY) (PTCB)->InfoBit=(PROPERTY)
    /*set entry pointer of the task*/
    #define SET_TASK_ENTRY(PTCB,ENTRY) (PTCB)->PTask=(ENTRY)
    /*get the status of the task*/
    #define GET_TASK_STATUS(PTCB)  ((PTCB)->OSTCBStat)
    /*set the ID of the task*/
    #define SET_TASK_ID(PTCB,ID) (PTCB)->OSTCBId=(ID)
    /*set the original priority of the task*/
    #define SET_TASK_ORIGINAL_PRIO(PTCB,origPrio) ((PTCB)->OrigPrio)=origPrio    
     /*set the csa cnt of the task*/

 #if (OS_RES_INTERNAL == STD_ON) || (OS_RES_EXTERNAL == STD_ON)|| (OS_RES_SCHEDULER == STD_ON)
     /*set the run priority of the task*/
    #define SET_TASK_RUNNING_PRIO(PTCB,runPrio) ((PTCB)->RunPrio)=runPrio
  #else  
    #define SET_TASK_RUNNING_PRIO(PTCB,runPrio)
 
 #endif /*(OS_RES_INTERNAL == STD_ON) || (OS_RES_EXTERNAL == STD_ON)||defined(OS_RES_SCHEDULER)*/ 
  
        /*set the event list of the task*/
        #define SET_TASK_EVENT_LIST(PTCB,osTaskEventRef)  ((PTCB)->OsTaskEventRef)=osTaskEventRef
        /* set the resource list of the task */
        #define SET_TASK_RESOURCE_LIST(PTCB,osTaskResourceRef) ((PTCB)->OsTaskResourceRef)=osTaskResourceRef
           
         /*set the max activation time of the task*/
         #define SET_TASK_MAX_ACT(PTCB,maxAct) ((PTCB)->MaxAct)=(maxAct)
         /*set the current activation time of the task*/
         #define SET_TASK_CUR_ACT(PTCB,curAct) ((PTCB)->CurAct)=(curAct)
         
         #define GET_TASK_CUR_ACT(PTCB) ((PTCB)->CurAct)
          
         #define DEC_TASK_CUR_ACT(PTCB) (((PTCB)->CurAct)--)
         
         #define INC_TASK_CUR_ACT(PTCB) (((PTCB)->CurAct)++) 
         
         #define INIT_IDLE_READY_LIST(prio)                \
             ReadyListCBTbl[prio].head = ReadyList0;   \
             ReadyListCBTbl[prio].count = 0U;            \
             ReadyListCBTbl[prio].start = 0U

         #define INIT_READY_LIST(prio)                \
             ReadyListCBTbl[prio].head = ReadyListCfgTbl[((prio)-1)].head; \
             ReadyListCBTbl[prio].count = 0U;            \
             ReadyListCBTbl[prio].start = 0U             

         /*insert to the head of the same priority ready list*/
         #define INSERT_READY_LIST_HEAD(taskId, prio)  \
            ReadyListPtr = ReadyListCBTbl[prio].head; \
            start = ReadyListCBTbl[prio].start;            \
            if(start != 0U )  pos = start-1U;                         \
            else pos = ReadyListCfgTbl[prio-1U].length-1U;\
            ReadyListCBTbl[prio].start = (uint8)pos;              \
            *(ReadyListPtr + pos) = taskId;                                 \
            ReadyListCBTbl[prio].count++
                        
            
         /*insert to the rear of the same priority ready list*/            
         #define INSERT_READY_LIST_REAR(taskId,prio)        \
                 ReadyListPtr=ReadyListCBTbl[prio].head; \
                 pos=(ReadyListCBTbl[prio].start+ReadyListCBTbl[prio].count)%ReadyListCfgTbl[prio-1U].length; \
                 *(ReadyListPtr+pos)=taskId;                                 \
                 ReadyListCBTbl[prio].count++
        
         /*cancel the head task from the same priority ready list*/    
         #define REMOVE_READY_LIST_HEAD(prio)     \
           ReadyListPtr=ReadyListCBTbl[prio].head; \
           ReadyListCBTbl[prio].start=(ReadyListCBTbl[prio].start+1U)%ReadyListCfgTbl[prio-1U].length;     \
           ReadyListCBTbl[prio].count--
         
         /*set the same priority ready list to empty*/
         #define IS_READY_LIST_EMPTY(prio)  \
           (ReadyListCBTbl[prio].count==0U)

         /*estimate whether the corresponding  priority ready list is full*/
         #define IS_READY_LIST_FULL(prio) \
               (ReadyListCBTbl[prio]==ReadyListCfgTbl[prio-1U].length)
        
    /*set the status of the task*/
    #define SET_TASK_STATUS(PTCB,STATUS) (PTCB)->OSTCBStat = (STATUS)
    /*get the id of the task*/
    #define GET_TASK_ID(PTCB)  ((PTCB)->OSTCBId)
    #define GET_TASK_ORIGINAL_PRIO(PTCB) ((PTCB)->OrigPrio)
    /*estimate whether the task is ready,true is YES*/
    #define IS_TASK_READY(PTCB) ( READY==(GET_TASK_STATUS(PTCB)&~RUN_READY) )
  
    #if (OS_RES_INTERNAL == STD_ON)
       /*get the priority the task's internal resource */
        #define GET_TASK_INT_RES(PTCB)  ((PTCB)->RunPrio)
        /*get the saved task TCB of current task tcb*/
        #define GET_TASK_SAVED_TCB(PTCB)  ((PTCB)->SavedTCB)
        /*set the priority of the task*/
        #define SET_TASK_INT_RES(PTcb,ResPrio) ((PTcb)->RunPrio)=ResPrio
        /*save the runPriority tcb in the current task's TCB*/
        #define RESET_TASK_SAVED_TCB(PTCB) \
        OSTCBPrioTbl[GET_TASK_INT_RES(PTCB)]=GET_TASK_SAVED_TCB(PTCB)

    #else /*#ifdef OS_INT_RES*/
        #define RESET_TASK_SAVED_TCB(PTCB)    
        #define SET_TASK_INT_RES(PTcb,ResPrio)
    #endif /*#define OS_INT_RES*/    


    #if ((OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON))
        /*the head of the occupied external resouce list*/
        #define GET_TASK_OUT_RES(PTCB) ((PTCB)->resources)
        /*set the head of the occupied external resource lise*/
        #define SET_TASK_OUT_RES(PTCB,Res) (PTCB)->resources=Res
    #endif /*#if defined(OS_OUT_RES)||defined(OS_RES_SCHEDULER)*/


    #if ((OS_RES_INTERNAL == STD_ON) || (OS_RES_EXTERNAL == STD_ON)||(OS_RES_SCHEDULER == STD_ON))
        #define GET_TASK_RUN_PRIO(PTCB) ((PTCB)->RunPrio)
        #define SET_TASK_RUN_PRIO(PTCB,PRIO) ((PTCB)->RunPrio)=((PriorityType)(PRIO) )
    #else /*#if defined(OS_INT_RES) ||defined(OS_OUT_RES)*/
        #define GET_TASK_RUN_PRIO(PTCB) GET_TASK_ORIGINAL_PRIO(PTCB)
        #define SET_TASK_RUN_PRIO(PTCB,PRIO)
    #endif /*#if defined(OS_INT_RES) ||defined(OS_OUT_RES)*/

    #if (OS_EVENT_MODULE == STD_ON)
        /*clear the event of the task TCB*/
        #define CLEAR_TASK_EVENT(PTCB)  (PTCB)->SEvent=(0U);\
                                         (PTCB)->WEvent=(0U)

        /*set got event of the TASK TCB*/
        #define SET_TASK_SET_EVENT(PTCB,mask)  (PTCB)->SEvent|=(EventMaskType)(mask)

        /*set wait event of the task TCB*/
        #define SET_TASK_WAIT_EVENT(PTCB,mask)  (PTCB)->WEvent|=(EventMaskType)(mask)


        /*get the set event of the task*/
        #define GET_TASK_SET_EVENT(PTCB)  ((PTCB)-> SEvent)

        /*get the waited event of the task*/
        #define GET_TASK_WAIT_EVENT(PTCB)  ((PTCB)->WEvent)

        /*clear the event of the task*/
        #define CLERA_TASK_MASK_EVENT(PTCB,mask)  (PTCB)->SEvent&=(EventMaskType)(~mask);\
                                                                                                    (PTCB)->WEvent&=(EventMaskType)(~mask)

    #else /*#ifdef OS_EVENT_EN*/
        #define CLEAR_TASK_EVENT(PTCB)
        #define SET_TASK_EVENT(PTCB,SEvent)
    #endif /*#ifdef OS_EVENT_EN*/
    
    #define SET_TASK_READY(prio, PTCB)             \
         SET_TASK_STATUS(PTCB,READY); \
         OS_TASK_TO_READY(prio,PTCB)
 
    #define SET_TASK_UNREADY(TaskId,PTCB,Status)  \
         SET_TASK_STATUS(PTCB,Status); \
         OS_TASK_TO_UNREADY(TaskId,PTCB)
          
    #if (OS_STACK_MONITORING_MODULE == STD_OFF)
        #define OSStackMonitoring(PTCB)   
    #else 
        #define OSStackMonitoring(PTCB) OSTaskStkChk(PTCB)
    #endif   

    #define TASK(name) void Func##name(void)
    #define DefineTask(p1,p2,p3,p4,p5,p6) FuncDefineTask(Func##p1,p2,p3,p4,p5,p6)

/*******************************************************************
| external declarations:
| function prototypes (scope: module-exported)
| extern variable  (scope: module-exported)
|------------------------------------------------------------------*/
    extern TaskType OSTCBCur; /* current tcb pointer     */
    extern TaskType OSTCBPrioTbl[(OS_NUM_SYS_TASK)]; /* tcb array based on the priority index */
    extern OSTCB OSTCBTbl[(OS_NUM_SYS_TASK)]; /* TCB Table                  */

/*********************************************************************************************************
       task context switch ��implenmented in  OS_CPU_A.ASM
        1.Save old task��Load new task. used in Schedule��WaitEvent��SetEvent��ReleaseResource
        2.Save old task��not Load new task��used in ActivateTask��Schedule��WaitEvent��ReleaseResource
        3.not Save old task��just Load new task. used in TerminateTask��ChainTask��ISR2
        4.not Save��Load��used in TerminateTask��ChainTask��ISR2
**************************************************************************************************/
      void TaskSaveLoadRun(TaskType PSavedTcb, TaskType PLoadTcb);
      void TaskSaveRun(TaskType PSavedTcb, TaskType PLoadTcb);
      void TaskLoadRun(TaskType PRunTcb);
      void TaskRun(TaskType PRunTcb);
      void FirstTaskRun(TaskType PRunTcb);  //lvjun modified

    #if (OS_ALARM_MODULE == STD_ON)
        void OSActivateTask(TaskType TaskId);
    #endif

/*******************************************************************
| function prototypes (scope: module-local)
|------------------------------------------------------------------*/
void OSTaskInit(void);
uint8 FuncDefineTask(void(*PTask)(void), OSSTKRef PStkHigh, OSSTKRef PStkLow, uint8 Priority, uint8 Property, PriorityType  ResPrio);

/******************************************************************* 
| function prototypes (scope: module-exported) 
|------------------------------------------------------------------*/

    #define DeclareTask(TaskID)  void Func##TaskID(void)
    StatusType ActivateTask(TaskType ptcb);
    StatusType TerminateTask(void);
    StatusType ChainTask(TaskType ptcb);
    StatusType GetTaskID(TaskRefType PTask);
    StatusType GetTaskState(TaskType ptcb, TaskStateRefType state);
    StatusType Schedule(void);
    void OSAppActivateTask(TaskType ptcb, uint16 AppID);

#endif /*#ifndef  OS_TASK_H*/
