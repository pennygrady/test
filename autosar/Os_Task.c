/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Task.c
* DESCRIPTION: Task module source file based on AUTOSAR OS standard 
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0           Wei.Chen      4/DEC/03       Initial version.   
* 1.1    HangPing.Pan&Jiang.Li 17/JAN/05   
* 1.2           Jiang.Li      17/MAY/05   
*
*********************************************************************************
*/

#ifndef OS_TASK_C 
    #define OS_TASK_C

#include "Os.h"
#include "Os_Cpu_Types.h"
#include "Os_Res.h"
#include "Platform_Lib.h"
/*******************************************************************
| external declarations:
| function prototypes (scope: module-exported)
| extern variable  (scope: module-exported)
|------------------------------------------------------------------*/    
    
#if (OS_TASK_NUM > 0U)
    extern OsTask OSTaskCfgTbl[];
    extern const ReadyListCfg ReadyListCfgTbl[OS_MAX_TASK_PRIO];
#endif /*OS_TASK_NUM>0*/
    #if (OS_APPLICATION_MODULE == STD_ON)
       extern OsApplicationCB ApplicationCB[OS_APPLICATION_NUM];
    #endif   
    
    #if (OS_TIMING_PROTECTION_MODULE ==  STD_ON)
       extern uint32 MaxCrossTime ;
    #endif 
/*******************************************************************
| global variable definitions (scope: module-exported)
|------------------------------------------------------------------*/   
    
    TaskType OSTCBCur = (TaskType)0U;     
    OSTCB OSTCBTbl[(OS_NUM_SYS_TASK)]; 
    TaskType OSTCBOld = (TaskType)0U;  
    
    /*the control block of the ready list*/
    ReadyListCB ReadyListCBTbl[OS_MAX_TASK_PRIO+1U];      

    uint8 OSRdyGrp = 0U;              
    uint8 OSRdyTbl[OS_RDY_TBL_SIZE];  
    uint8 ReadyList0[1U];

/**********************************************************************
* Function Prototype: StatusType ActivateTask (TaskType ptcb)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): ptcb - pointer to task to be activated
*    (out): None
* Return Value: StatusType - E_OS_ID, E_OS_LIMIT, E_OS_ACCESS, E_OS_PROTECTION_TIME, E_OS_PROTECTION_ARRIVAL, E_OK
* Caller: OsTask Object and Category Two ISR
* Call Function: 
* Description: Activates a suspended task
* Author: Wei Chen
* Time: 2003.11.23
***************************************************************************
*/
    StatusType ActivateTask(TaskType ptcb)
    {
        TaskType oldRunning = NULL;
        uint8 *ReadyListPtr;
        uint16 pos;
        
        #if(OS_APPLICATION_MODULE == STD_ON)
        uint16 taskid,AppID;
        #endif
                
        #if (OS_TIMING_PROTECTION_MODULE ==  STD_ON)
            TickType ticks;   
        #endif
       
        DEFINECRITICAL();

        #if (OS_EXTENDED_STATUS == STD_ON)
            if ((ptcb >= (OSTCBTbl + OS_NUM_SYS_TASK)) || (ptcb < OSTCBTbl) || (((AddrType)ptcb - (AddrType)OSTCBTbl) % sizeof
                (OSTCB) != 0U))
            {  /*TCB overrun*/
                OSRETERROR(E_OS_ID, OSServiceId_ActivateTask, ptcb); 
            }
        #endif /*#ifdef OS_EXTENDED_STATUS*/
        
        ENTER_CRITICAL();   /*it is not allowed to do the same opration to the same task*/      
        
          if((ptcb->InfoBit & EXTENDED) && ptcb->CurAct)
          {         /*the activation is rejected, if it tries to activate the extended task that is not at suspended status */
               EXIT_CRITICAL();
               //OSRETERROR(4, OSServiceId_ActivateTask, ptcb);
          }
          else if(!(ptcb->InfoBit & EXTENDED) && (ptcb->CurAct >= ptcb->MaxAct)) 
          { /*the activation is rejected , if basic tasks's activation times more than the max time*/
                EXIT_CRITICAL();
                //OSRETERROR(4, OSServiceId_ActivateTask, ptcb);
          }
   
        #if ((OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
           AppID = GetApplicationID();
           taskid = ptcb->OSTCBId;
           if( CheckObjectAccess(AppID,OBJECT_TASK,taskid) == NO_ACCESS) 
           {
            EXIT_CRITICAL();
            OSRETERROR(E_OS_ACCESS, OSServiceId_ActivateTask, ptcb);
           }
         #endif 
          

      #if (OS_TIMING_PROTECTION_MODULE ==  STD_ON)
        #if (OS_APPLICATION_MODULE == STD_ON )
            AppID = CheckObjectOwnership(OBJECT_TASK,taskid);
             if(FALSE == ApplicationCB[AppID].OsTrusted ||TRUE == ApplicationCB[AppID].TimingProtection) 
             {
                   if(ptcb->InfoBit & TASKHAVERUN)
                    {
                        if( ptcb->CrossNum == MaxCrossTime)
                        {
                             ticks = GET_COUNTER_VALUE(OSCounters) - ptcb-> TimeFrameStart;
                        } else{
                             ticks = (MaxCrossTime - ptcb->CrossNum ) * GET_COUNTER_MAXVALUE(OSCounters)  - ptcb-> TimeFrameStart + GET_COUNTER_VALUE(OSCounters);
                        }
                        
                        if(ticks < ptcb->OsTaskTimeFrame)
                        {
                            EXIT_CRITICAL();
                           // ProtectionHook(E_OS_PROTECTION_ARRIVAL);
                            #if (OS_PROTECTION_HOOK == STD_ON)
                            OSProtectionHook(E_OS_PROTECTION_TIME,OSTCBCur);
                            #endif
                            OSRETERROR(E_OS_PROTECTION_ARRIVAL, OSServiceId_ActivateTask, ptcb);
                        }
                   }else{
                      ptcb->InfoBit |= TASKHAVERUN;
                   }
                   ptcb->CrossNum =  MaxCrossTime; /*updata CrossTime*/
                   ptcb->TimeFrameStart =  GET_COUNTER_VALUE(OSCounters);/*update TimeFrameStart*/
             }
       #endif
    #endif 
   /*timing protection*/
           
        INC_TASK_CUR_ACT(ptcb);            
        oldRunning = OSTCBCur;

        if(GET_TASK_CUR_ACT(ptcb)==1U)
            SET_TASK_READY(GET_TASK_ORIGINAL_PRIO(ptcb), ptcb); 
        
        INSERT_READY_LIST_REAR(GET_TASK_ID(ptcb), GET_TASK_ORIGINAL_PRIO(ptcb)); 
     

        #if (OS_SCHEDULE_POLICY != OS_SCHEDULE_NON) 
            if (NOT_IN_ISR
            #if (OS_SCHEDULE_POLICY == OS_SCHEDULE_MIX)
                 && (FULL_SCHED == GET_TASK_SCHED(oldRunning)) 
            #endif /*#ifdef   OS_MIX_SCHEDULE*/
            )
            {
                /*it can be occupied, and it not in ISR, and the scheduler isn't locked.*/
                if (GET_TASK_ORIGINAL_PRIO(ptcb) > GET_TASK_RUN_PRIO(oldRunning))          
                {/*the priority of the task which is activated is higher than the current task*/
                    //RELEASE_INT_RES(oldRunning);  //OS223:Points of rescheduling 
                    OSPostTaskHook(); 
                    OS_TASK_SWITCH(oldRunning);
                    OSTCBCur = ptcb;

                    GET_INT_RES(ptcb); /*get internal resource*/
                    
                    SET_TASK_STATUS(ptcb, RUN_READY); 
                    OSPreTaskHook();
                    OSStackMonitoring(oldRunning);
                    OSTaskSaveRun(oldRunning, OSTCBCur);
                }
            }
        #endif /*OS_NON_SCHEDULE */
    

        EXIT_CRITICAL();
        return E_OK;
    }

/**********************************************************************
* Function Prototype: StatusType TerminateTask(void)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): None
*    (out): None
* Return Value: None
* Caller: StartOS
* Call Function: 
* Description: Terminates the excution of the current task
* Author: Wei Chen
* Time: 2003.12.23
***************************************************************************
*/
    StatusType TerminateTask(void)
    {
        TaskType curTcb = NULL;
        uint8 *ReadyListPtr,highestPrio;
    #if (OS_APPLICATION_MODULE == STD_ON)
        uint16 AppID;
    #endif
        DEFINECRITICAL();

        #if (OS_EXTENDED_STATUS == STD_ON)
            #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON)
                if (NO_RESOURCE != GET_TASK_OUT_RES(OSTCBCur)
                #if (OS_RES_INTERRUPT == STD_ON) && (OS_RES_EXTERNAL == STD_ON)
                     || NO_RESOURCE != OSISRLastResource
                #endif /*#if defined(OS_ISR_RES)&&defined(OS_OUT_RES)*/    
                )
                {
                    /*current task have occupied external resource*/
                    OSRETERROR(E_OS_RESOURCE, OSServiceId_TerminateTask, 0U);
                }
            #endif /*defined(OS_OUT_RES)||defined(OS_RES_SCHEDULER)*/

            if (!NOT_IN_ISR)
            {
                /*called in ISR*/
                OSRETERROR(E_OS_CALLEVEL, OSServiceId_TerminateTask, 0U);
            }
        #endif /*ifdef OS_EXTENDED_STATUS*/

        ENTER_CRITICAL();
        curTcb = OSTCBCur;
        
        /* TODO: Release Resource used by Timing Protection, the situation without Timing Protection is not considered. */
        
        RELEASE_INT_RES(curTcb);
        CLEAR_TASK_EVENT(curTcb);
        
        OS_TASK_SWITCH(curTcb);
        OSPostTaskHook();
       
        
         DEC_TASK_CUR_ACT(curTcb);   /*decrease the activation time*/

         REMOVE_READY_LIST_HEAD(GET_TASK_ORIGINAL_PRIO(curTcb));
        
      #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
         #if (OS_APPLICATION_MODULE == STD_ON && ModulesInitTrue == ModulesInitStatus)
             AppID = GetApplicationID();
           if(FALSE == ApplicationCB[AppID].OsTrusted ||TRUE == ApplicationCB[AppID].TimingProtection) 
           {
            curTcb->RemainingExecutionBudget = curTcb->OsTaskExecutionBudget;/* reset RemainingExecutionBudget */
           }
         #endif
      #endif 
        
         /* If the multi-activation is Zero, sets the task to suspended and updates the ready status table. */        
         if(GET_TASK_CUR_ACT(curTcb)==0U)
         {
           SET_TASK_UNREADY(GET_TASK_ORIGINAL_PRIO(curTcb), curTcb, SUSPENDED);
         }
         /* if the multi-activation is greater than Zero, reset the task to ready and updates the ready status table. */
         else     
         {
           SET_TASK_READY(GET_TASK_ORIGINAL_PRIO(curTcb),curTcb); 
         }
         
        if (NOT_IN_ISR)
        {
            /* finds the ready task which has highest priority */          
            highestPrio = OS_FIND_HIGHEST_TASK();         
            /* point to the head of highest priority task ready list */
            ReadyListPtr=ReadyListCBTbl[highestPrio].head    ; 
            /* update the OSTCBOld */
            OSTCBOld = OSTCBCur;
            /* updates the OSTCBCur */
            OSTCBCur=curTcb=&OSTCBTbl[*(ReadyListPtr+ReadyListCBTbl[highestPrio].start)];
            GET_INT_RES(curTcb); /*TODO:timing protection*/
            
            OSPreTaskHook(); /*call PreTaskHook*/

            TaskTerminateSwitch(OSTCBOld, OSTCBCur);
        }
     
        EXIT_CRITICAL();

        return E_OK; /*never go  here*/
    }

/**********************************************************************
* Function Prototype: StatusType ChainTask (TaskType ptcb)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): ptcb - pointer to task to be activated
*    (out): None
* Return Value: StatusType - E_OS_ID, E_OS_RESOURCE, E_OS_CALLEVEL, E_OS_ACCESS, E_OS_LIMIT, E_OK
* Caller: OsTask Level
* Call Function: ReDefineTask, OSSchedule
* Description: stops itself and activates a suspended task
* Author: Wei Chen
* Time: 2003.11.23
***************************************************************************
*/
    StatusType ChainTask(TaskType ptcb)
    {
        TaskType curTcb = NULL;
        uint8 *ReadyListPtr,highestPrio;
        uint16 pos;
    #if (OS_APPLICATION_MODULE == STD_ON)
        uint16 AppID,taskid;
    #endif

        DEFINECRITICAL();

        #if (OS_EXTENDED_STATUS == STD_ON)

            if ((ptcb >= (OSTCBTbl + OS_NUM_SYS_TASK)) || (ptcb < OSTCBTbl) || (((AddrType)ptcb - (AddrType)OSTCBTbl) % sizeof
                (OSTCB) != 0U))
            {
                /* Paramters out of range */

                OSRETERROR(E_OS_ID, OSServiceId_ChainTask, ptcb);
            }
        
        #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON)
            if (NO_RESOURCE != GET_TASK_OUT_RES(OSTCBCur)
            #if (OS_RES_INTERRUPT == STD_ON)
                 || NO_RESOURCE != OSISRLastResource
            #endif /*ifdef   OS_ISR_RES*/
            )
            {
                OSRETERROR(E_OS_RESOURCE, OSServiceId_ChainTask, ptcb);
            }
        #endif /*if defined(OS_OUT_RES)||defined(OS_RES_SCHEDULER)*/
        if (!NOT_IN_ISR)
        /* tests whether be called in ISR */
        {
            OSRETERROR(E_OS_CALLEVEL, OSServiceId_ChainTask, ptcb);
        }
        #endif /*#ifdef OS_EXTENDED_STATUS*/
                
        /* checks for Application requirements. */
        #if (OS_APPLICATION_MODULE == STD_ON )
             taskid = OSTCBCur->OSTCBId;
             AppID = CheckObjectOwnership(OBJECT_TASK,taskid);
             taskid = ptcb->OSTCBId;
             if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_TASK,taskid) ) 
             {
              OSRETERROR(E_OS_ACCESS, OSServiceId_ActivateTask, ptcb);
             }
        #endif 
           
        ENTER_CRITICAL();
         curTcb = OSTCBCur;
        
          /* if is extended task, multi-activation is not permitted. */
          if((ptcb!=curTcb) && (ptcb->InfoBit & EXTENDED) && (ptcb->CurAct))
          {         
               EXIT_CRITICAL();
               //OSRETERROR(4, OSServiceId_ChainTask, ptcb);
          }
          /* if is basic task, multi-activation exceeds tha maximum is not permitted. */
          else if((ptcb!=curTcb) && !(ptcb->InfoBit & EXTENDED) && (ptcb->CurAct >= ptcb->MaxAct)) 
          {
                EXIT_CRITICAL();
                //OSRETERROR(4, OSServiceId_ChainTask, ptcb);
          }

        /* TODO: Release Resource used by Timing Protection, the situation without Timing Protection is not considered. */
        RELEASE_INT_RES(curTcb);
        CLEAR_TASK_EVENT(curTcb);
        /* call post task hook function */
        OSPostTaskHook();

       if(curTcb != ptcb) 
       {          
          /* code to process the stop excution */
         /* Decrease the activated times */
         /* Remove the task from the ready list */
         DEC_TASK_CUR_ACT(curTcb);   
                 
         REMOVE_READY_LIST_HEAD(GET_TASK_ORIGINAL_PRIO(curTcb));
         
      #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON) 
          #if (OS_APPLICATION_MODULE == STD_ON)
              if(FALSE == ApplicationCB[AppID].OsTrusted || FALSE == ApplicationCB[AppID].OsTrusted )
                curTcb->RemainingExecutionBudget = curTcb->OsTaskExecutionBudget;/* resets the RemainingExecutionBudget */
          #endif 
       
      #endif 
       
         /* If the multi-activation is Zero, sets the task to suspended and updates the ready status table. */ 
         if(GET_TASK_CUR_ACT(curTcb)==0U)                                
         {
           SET_TASK_UNREADY(GET_TASK_ORIGINAL_PRIO(curTcb), curTcb, SUSPENDED);
         }
         /* If the multi-activation is greater than Zero, sets the task to ready and updates the ready status table. */ 
         else                                                                    
         {
           SET_TASK_READY(GET_TASK_ORIGINAL_PRIO(curTcb),curTcb); 
         }
         
              
         /* code to prepare the task entry to ready status */
         /* if the activated time is 1 (the task has not run before), sets the task to ready status. */
         INC_TASK_CUR_ACT(ptcb);         
         if(ptcb->CurAct == 1U)
         { 
            SET_TASK_READY(GET_TASK_ORIGINAL_PRIO(ptcb),ptcb);                                    
         }
         
         INSERT_READY_LIST_REAR(GET_TASK_ID(ptcb),GET_TASK_ORIGINAL_PRIO(ptcb));
       }
       else
       {
         /* Remove the task from the ready list */
         REMOVE_READY_LIST_HEAD(GET_TASK_ORIGINAL_PRIO(curTcb));
         INSERT_READY_LIST_REAR(GET_TASK_ID(ptcb),GET_TASK_ORIGINAL_PRIO(ptcb));
         SET_TASK_STATUS(curTcb,READY);
       }
            
        /* save old task, */
        /* finds the highest priority ready task and force to re-schedule */
            
        SET_TASK_OLD(curTcb);
        
       
        highestPrio = OS_FIND_HIGHEST_TASK();                         
        ReadyListPtr=ReadyListCBTbl[highestPrio].head    ; 
        OSTCBCur=ptcb=&OSTCBTbl[*(ReadyListPtr+ReadyListCBTbl[highestPrio].start)];

        /* gets the inner resource */
        /* TODO: gets Resource used by Timing Protection, the situation without Timing Protection is not considered. */
        GET_INT_RES(ptcb); 
        
        /* call the PreTask Hook function */
        OSPreTaskHook(); 
        TaskTerminateSwitch(OSTCBOld, OSTCBCur);

        EXIT_CRITICAL();
        return E_OK; /*avoid of   compiler warning, and it's never run here*/
    }
  
/**********************************************************************
* Function Prototype: StatusType GetTaskID( TaskRefType PTaskID )
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): None
*    (out): PTaskID - pointer to store the returned value.
* Return Value: StatusType - E_OK
* Caller: OsTask Object and Category Two ISR
* Call Function: 
* Description: gets the task id of current running task 
* Author: Jiang Li, HangPing Pan
* Time: 2005.1.19
***************************************************************************
*/  
    StatusType GetTaskID(TaskRefType PTask)
    {
        if ((NULL == OSTCBCur) || (OSTCBCur == OSTCBTbl))        
        {/*current TCB is null, or current task is IDLE task, return INVALID_TASK*/
            *PTask = (TaskType)INVALID_TASK;            
        }
        else
        {
             *PTask = (TaskType)OSTCBCur;        
        }
        return E_OK;
    }

/**********************************************************************
* Function Prototype: StatusType GetTaskState ( TaskType ptcb, TaskStateRefType state )
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): ptcb - pointer to task. 
*    (out): state - pointer to store the returned value.
* Return Value: StatusType - E_OS_ID, E_OK
* Caller: OsTask Object and Category Two ISR
* Call Function: 
* Description: gets the task state
* Author: Jiang Li, HangPing Pan
* Time: 2005.1.19
***************************************************************************
*/
    StatusType GetTaskState(TaskType ptcb, TaskStateRefType state)
    {

        #if (OS_EXTENDED_STATUS == STD_ON)
            if ((ptcb >= (OSTCBTbl + OS_NUM_SYS_TASK)) || (ptcb < OSTCBTbl) || (((AddrType)ptcb - (AddrType)OSTCBTbl) % sizeof
                (OSTCB) != 0U))
            {
                OSRETERROR(E_OS_ID, OSServiceId_GetTaskState, ptcb);
            }
        #endif /*#ifdef OS_EXTENDED_STATUS*/

        if (OSTCBCur == ptcb)
        {
            *state = RUNNING;
        }
        else
        {
            *state = (TaskStateType)(GET_TASK_STATUS(ptcb) &(uint8)(~(RUN_READY))); /*take account of RUN_READY*/
        }
        return E_OK;
    }

/**********************************************************************
* Function Prototype: StatusType Schedule( void )
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): None
*    (out): None
* Return Value: StatusType - E_OK, E_OS_CALLEVEL, E_OS_RESOURCE
* Caller: OsTask module
* Call Function: 
* Description: Force to re-schedule task. If having higher priority ready task, then this task gets the right to execute.
* Author: Jiang Li, HangPing Pan
* Time: 2005.1.13
***************************************************************************
*/
    StatusType Schedule(void)
    {        
        TaskType ptcb,curTcb = NULL;
        uint8 *ReadyListPtr ,highestPrio;
        DEFINECRITICAL();
        #if (OS_EXTENDED_STATUS == STD_ON)
            if (!NOT_IN_ISR)
            {    /*called in ISR*/
                OSRETERROR(E_OS_CALLEVEL, OSServiceId_Schedule, 0U);
            }

            #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON)
                if (NO_RESOURCE != GET_TASK_OUT_RES(OSTCBCur))
                {
                     /*current task occupied external resource*/
                    OSRETERROR(E_OS_RESOURCE, OSServiceId_Schedule, 0U);
                }
            #endif /*ifdef OS_OUT_RESOURCE*/
        #endif /*ifdef OS_EXTENDED_STATUS*/

        ENTER_CRITICAL();
        curTcb = OSTCBCur;

        #if (OS_SCHEDULE_POLICY == OS_SCHEDULE_FULL)
            #if (OS_RES_INTERNAL == STD_ON) 
                if (GET_TASK_ORIGINAL_PRIO(curTcb) == GET_TASK_RUN_PRIO(curTcb))
            #endif /*#ifdef OS_INT_RES */
            {             
                /* if the original priority equals to run priority */
                /* (or means the task not has inner resource) ,not to re-schedule */
                EXIT_CRITICAL();
                return (E_OK);
            }
        #elif (OS_SCHEDULE_POLICY == OS_SCHEDULE_MIX)/*#ifdef OS_FULL_SCHEDULE*/  /*mixed schedule method */
            if ((FULL_SCHED == GET_TASK_SCHED(curTcb)) /* a pre-emptive task */
            #if (OS_RES_INTERNAL == STD_ON) 
                 && (GET_TASK_ORIGINAL_PRIO(curTcb) == GET_TASK_RUN_PRIO(curTcb))
                /* if the original priority equals to run priority */
                /* (or means the task not has inner resource) ,not to re-schedule */
            #endif /*#ifdef OS_INT_RES */
            )
            {
                EXIT_CRITICAL();
                return (E_OK);
            }
        #endif /*#ifdef OS_FULL_SCHEDULE*/

        RELEASE_INT_RES(curTcb); /*TODO:timing protection*/
        /* finds the ready task with highest priority */
        highestPrio = OS_FIND_HIGHEST_TASK();                             
        ReadyListPtr=ReadyListCBTbl[highestPrio].head    ; 
        ptcb=&OSTCBTbl[*(ReadyListPtr+ReadyListCBTbl[highestPrio].start)];
                
        if(ptcb!=curTcb)
   {
 
            OS_TASK_SWITCH(curTcb);
            /* before switch the current task, call PostTask Hook function */
            OSPostTaskHook(); 
            OSTCBCur = ptcb;
            GET_INT_RES(ptcb); /*TODO:timing protection*/
            
            /* call PreTask Hook function*/
            OSPreTaskHook(); 
            OSStackMonitoring(curTcb);
            #if (OS_EVENT_MODULE == STD_ON) /* in BCC1,it is definitedly new task to execute */
                if (RUN_READY != GET_TASK_STATUS(ptcb))
            #endif /*#ifdef OS_EVENT_EN*/
            {                 
                /* if task has not run before, sets the task status to ready */
                /* in CC2, parameter OSTCBCur can not be optimized to local variable. */
                SET_TASK_STATUS(OSTCBCur, RUN_READY);
                OSTaskSaveRun(OSTCBOld, OSTCBCur);
            }
            #if (OS_EVENT_MODULE == STD_ON)
                else
                {
                    OSTaskSaveLoadRun(OSTCBOld, OSTCBCur);
                }
            #endif /*#ifdef OS_EVENT_EN*/

           EXIT_CRITICAL();
        }
        else
        {
            GET_INT_RES(curTcb); /*TODO:timing protection*/
            EXIT_CRITICAL();
        }
        return (E_OK);
    }

    #if (OS_ALARM_MODULE == STD_ON)

/**********************************************************************
* Function Prototype: void OSActivateTask (TaskType ptcb)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): ptcb - pointer to task to be activated
*    (out): None
* Return Value: None
* Caller: NotifyAlarm API of OsAlarm module
* Call Function: 
* Description: Activate a suspended task
* Author: Jiang Li
* Time: 2005.5.15
***************************************************************************
*/
void OSActivateTask(TaskType ptcb)
{
    TaskType oldRunning = NULL;
    uint8 *ReadyListPtr;
    uint16 pos;
#if(OS_APPLICATION_MODULE == STD_ON)
    uint16 taskid, AppID;
#endif
            
       #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
        TickType ticks;   
      #endif

      #if (OS_EXTENDED_STATUS == STD_ON)
          if ((ptcb >= (OSTCBTbl + OS_NUM_SYS_TASK)) || (ptcb < OSTCBTbl) || (((AddrType)ptcb - (AddrType)OSTCBTbl) % sizeof
              (OSTCB) != 0U))
          {
               /*TCB is wrong*/
              return;
          }
      #endif /*#ifdef OS_EXTENDED_STATUS*/    
      
      if((ptcb->InfoBit & EXTENDED) && ptcb->CurAct)
      {         /*the activation is rejected, if it tries to activate the extended task that is not at suspended status */
    
           return;
      }
      else if(!(ptcb->InfoBit & EXTENDED) && ptcb->CurAct >= ptcb->MaxAct) 
      {/*the activation is rejected , if basic tasks's activation times more than the max time*/    
            return;
      }
      
      #if ((OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
       AppID = GetApplicationID();
       taskid = ptcb->OSTCBId;
       if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_TASK,taskid) )
       {
           return ;
       }
     #endif 
      

 #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
 
  #if (OS_APPLICATION_MODULE == STD_ON )
       AppID = CheckObjectOwnership(OBJECT_TASK,taskid);
       if(FALSE == ApplicationCB[AppID].OsTrusted || TRUE == ApplicationCB[AppID].TimingProtection ) 
       {
        if(ptcb->InfoBit & TASKHAVERUN)
              {
                  if( ptcb->CrossNum == MaxCrossTime)
                  {
                       ticks = GET_COUNTER_VALUE(OSCounters) - ptcb-> TimeFrameStart;
                  } else{
                       ticks = (MaxCrossTime - ptcb->CrossNum ) * GET_COUNTER_MAXVALUE(OSCounters)  - ptcb-> TimeFrameStart + GET_COUNTER_VALUE(OSCounters);
                  }
                  
                  if(ticks < ptcb->OsTaskTimeFrame)
                  {
                     
                      #if (OS_PROTECTION_HOOK == STD_ON)
                      OSProtectionHook(E_OS_PROTECTION_TIME,OSTCBCur);
                      #endif
                      return;
                  }
             }else{
                ptcb->InfoBit |= TASKHAVERUN;
             }
             ptcb->CrossNum =  MaxCrossTime; /*update CrossTime*/
             ptcb->TimeFrameStart =  GET_COUNTER_VALUE(OSCounters);/*update TimeFrameStart*/
       }
 #endif
#endif 
/*timing protection*/
      
      INC_TASK_CUR_ACT(ptcb);
     
      oldRunning = OSTCBCur;
      SET_TASK_READY(GET_TASK_ORIGINAL_PRIO(ptcb), ptcb); 
      
      INSERT_READY_LIST_REAR(GET_TASK_ID(ptcb), GET_TASK_ORIGINAL_PRIO(ptcb)); 

      #if (OS_SCHEDULE_POLICY != OS_SCHEDULE_NON) 
          if (NOT_IN_ISR
          #if (OS_SCHEDULE_POLICY == OS_SCHEDULE_MIX)
               && (FULL_SCHED == GET_TASK_SCHED(oldRunning)) 
          #endif /*#ifdef   OS_MIX_SCHEDULE*/
          )
          {
              /*it can be occupied, and it not in ISR, and the scheduler isn't locked.*/
              if (GET_TASK_ORIGINAL_PRIO(ptcb) > GET_TASK_RUN_PRIO(oldRunning))
              {/*the priority of the task which is activated is higher than the current task*/
                  OSPostTaskHook();
                  OS_TASK_SWITCH(oldRunning);
                  OSTCBCur = ptcb;

                  GET_INT_RES(ptcb); /*TODO:timing protection*/
                  
                  SET_TASK_STATUS(ptcb, RUN_READY); 
                  OSPreTaskHook();
                  OSStackMonitoring(oldRunning);
                  OSTaskSaveRun(oldRunning, OSTCBCur); 
              }                                                                          
          }
      #endif /*OS_NON_SCHEDULE */

      
      return ;
}

/**********************************************************************
* Function Prototype: void OSAppActivateTask (TaskType ptcb)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): ptcb - pointer to task to be activated
*    (out): None
* Return Value: None
* Caller: TerminateApplication of Application Module
* Call Function: 
* Description: Activate a suspended task
* Author: SURE
* Time: 2010.4
***************************************************************************
*/
#if (OS_APPLICATION_MODULE == STD_ON)   
void OSAppActivateTask(TaskType ptcb ,uint16 AppID)
{
    TaskType oldRunning = NULL;
        uint8 *ReadyListPtr;
        uint16 pos,taskid;
     #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
      TickType ticks;   
    #endif

    DEFINECRITICAL();

    #if (OS_EXTENDED_STATUS == STD_ON)
        if ((ptcb >= (OSTCBTbl + OS_NUM_SYS_TASK)) || (ptcb < OSTCBTbl) || (((AddrType)ptcb - (AddrType)OSTCBTbl) % sizeof
            (OSTCB) != 0U))
        {   /*TCB overrun*/
            return;
        }
    #endif /*#ifdef OS_EXTENDED_STATUS*/

    ENTER_CRITICAL(); /*it is not allowed to do the same opration to the same task*/    
    
    if((ptcb->InfoBit & EXTENDED) && ptcb->CurAct)
    {          /*the activation is rejected, if it tries to activate the extended task that is not at suspended status */
         EXIT_CRITICAL();
         return;
    }
    else if(!(ptcb->InfoBit & EXTENDED) && ptcb->CurAct >= ptcb->MaxAct)
    {/*the activation is rejected , if basic tasks's activation times more than the max time*/
          EXIT_CRITICAL();
              return;
    }
    
     taskid = ptcb->OSTCBId;
     if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_TASK,taskid) ) 
     {
       return ;
     }
        

   #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
     taskid = ptcb ->OSTCBId;
     AppID = CheckObjectOwnership(OBJECT_TASK,taskid);
     if(FALSE == ApplicationCB[AppID].OsTrusted ||TRUE == ApplicationCB[AppID].TimingProtection)
     {
      if(ptcb->InfoBit & TASKHAVERUN)
            {
                if( ptcb->CrossNum == MaxCrossTime)
                {
                     ticks = GET_COUNTER_VALUE(OSCounters) - ptcb-> TimeFrameStart;
                } else{
                     ticks = (MaxCrossTime - ptcb->CrossNum ) * GET_COUNTER_MAXVALUE(OSCounters)  - ptcb-> TimeFrameStart + GET_COUNTER_VALUE(OSCounters);
                }
                
                if(ticks < ptcb->OsTaskTimeFrame)
                {
                    EXIT_CRITICAL();
                    #if (OS_PROTECTION_HOOK == STD_ON)
                     OSProtectionHook(E_OS_PROTECTION_TIME,OSTCBCur);
                    #endif
                    return;
                }
           }else{
              ptcb->InfoBit |= TASKHAVERUN;
           }
           ptcb->CrossNum =  MaxCrossTime; /*update CrossTime*/
           ptcb->TimeFrameStart =  GET_COUNTER_VALUE(OSCounters);/*update TimeFrameStart*/
     }
  #endif 
/*timing protection*/
    
    INC_TASK_CUR_ACT(ptcb);
   
    oldRunning = OSTCBCur;

    SET_TASK_READY(GET_TASK_ORIGINAL_PRIO(ptcb), ptcb); 
    INSERT_READY_LIST_REAR(GET_TASK_ID(ptcb), GET_TASK_ORIGINAL_PRIO(ptcb)); 

    #if (OS_SCHEDULE_POLICY != OS_SCHEDULE_NON) 
        if (NOT_IN_ISR
        #if (OS_SCHEDULE_POLICY == OS_SCHEDULE_MIX)
             && (FULL_SCHED == GET_TASK_SCHED(oldRunning)) 
        #endif /*#ifdef   OS_MIX_SCHEDULE*/
        )
        {
            /*it can be occupied, and it not in ISR, and the scheduler isn't locked.*/
            if (GET_TASK_ORIGINAL_PRIO(ptcb) > GET_TASK_RUN_PRIO(oldRunning))
            {  /*the priority of the task which is activated is higher than the current task*/              
                OSPostTaskHook();
                OS_TASK_SWITCH(oldRunning);
                OSTCBCur = ptcb;

                GET_INT_RES(ptcb); /*TODO: timing protection*/
                
                SET_TASK_STATUS(ptcb, RUN_READY); 
                OSPreTaskHook();
                OSStackMonitoring(oldRunning);
                OSTaskSaveRun(oldRunning, OSTCBCur); 
            }                                                                          
        }
    #endif /*OS_NON_SCHEDULE */

    EXIT_CRITICAL();
}
#endif

    #endif /*#ifdef OS_ALARM_EN*/
    
#endif /*end of  #ifndef OS_TASK_C*/




