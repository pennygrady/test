/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Evt.c
* DESCRIPTION: Event module source file based on AUTOSAR OS standard 
* UPDATE HISTORY
* REV         AUTHOR                  DATE       DESCRIPTION OF CHANGE                         
* 1.0          Yang.Gu                1/DEC/04      Initial version.   
* 1.1          Shuang.Gao&Jiang.Li  1/MAR/05   
* 1.2          Jiang.Li              22/MAY/10   
*
**********************************************************************************
*/

#ifndef OS_EVT_C
    #define OS_EVT_C

        #include "Os.h"
 #if (OS_EVENT_MODULE == STD_ON)

/**********************************************************************
* Function Prototype��SetEvent
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): TaskID: the ID of task
*           Mask: the mask of event
*    (out): 
* Return Value: E_OK:successful
*        E_OS_ID��Invalid TaskID or tcb hasn't been initiallized.
*       E_OS_ACCESS��the task is not the extended task
*       E_OS_STATE��the task in suspended state
* Caller: None
* Call Function��
* Description: Set the task's evet.If the set event is the task wait for,it may cause schedule.
* Author:Jiang.Li
* Time:
***************************************************************************
*/ 
StatusType SetEvent(TaskType ptcb, EventMaskType Mask)
{
    TaskType curTcb = NULL;
    uint8 *ReadyListPtr,prio;
    uint16 pos,taskid,AppID;
        
    #if (OS_TIMING_PROTECTION_MODULE ==  STD_ON)
        uint32 ticks;
    #endif    

    DEFINECRITICAL();

    #if (OS_EXTENDED_STATUS == STD_ON)
        if ((ptcb >= (OSTCBTbl + OS_NUM_SYS_TASK)) || (ptcb < OSTCBTbl) || (((AddrType)ptcb - (AddrType)OSTCBTbl) %
            sizeof(OSTCB) != 0U))
        {
            OSRETERROR(E_OS_ID, OSServiceId_SetEvent, ptcb);
            /*Error��ptcb out of range*/
        }
        if (EXTENDED != GET_TASK_TYPE(ptcb))
        {
             /*Error��the task is not the extended task*/
            OSRETERROR(E_OS_ACCESS, OSServiceId_SetEvent, ptcb);

        }

        if (SUSPENDED == GET_TASK_STATUS(ptcb))
        {
             /*Error,�the task in suspended state*/
            OSRETERROR(E_OS_STATE, OSServiceId_SetEvent, ptcb);
        }
        
        if((Mask & ptcb->OsTaskEventRef)==0U)
        {
             /*Error��the task configurated without using this event*/
            OSRETERROR(E_OS_STATE, OSServiceId_SetEvent, ptcb);
        }
        
        #if ((OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
           AppID = GetApplicationID();
           taskid = ptcb-> OSTCBId;
           if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_TASK,taskid) )
            {
             OSRETERROR(E_OS_ACCESS, OSServiceId_SetEvent, ptcb);
            }
        #endif 
                
    #endif /*#ifdef  OS_EXTENDED_STATUS*/

    ENTER_CRITICAL();
    curTcb = OSTCBCur;
    SET_TASK_SET_EVENT(ptcb, Mask); /*Set the event mask of the task*/

    if ( (WAITING == GET_TASK_STATUS(ptcb)) && ( GET_TASK_SET_EVENT(ptcb) & GET_TASK_WAIT_EVENT(ptcb) ) )
    {
     /*timing protection��caculate the ticks of interval, to see if the timing protection is satisfied.If so activate the task.*/
    #if ( (OS_TIMING_PROTECTION_MODULE ==  STD_ON) && (OS_APPLICATION_MODULE == STD_ON)) 
          AppID = CheckObjectOwnership(OBJECT_TASK,taskid);
          if((FALSE == ApplicationCB[AppID].OsTrusted) ||(TRUE == ApplicationCB[AppID].TimingProtection)) 
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
                    OSRETERROR(E_OS_PROTECTION_ARRIVAL, OSServiceId_SetEvent, ptcb);
                }
             
              ptcb->CrossNum =  MaxCrossTime; /*Update the CrossTime*/
              ptcb->TimeFrameStart =  GET_COUNTER_VALUE(OSCounters);/*Update the TimeFrameStart*/
          }
        
     #endif
   /*timing protection*/

        SET_TASK_STATUS(ptcb, RUN_READY); /*if the event set is the event the task waiting for,change the state of the task to READY*/
        OS_TASK_TO_READY(GET_TASK_ORIGINAL_PRIO(ptcb), ptcb);
        
        prio = GET_TASK_ORIGINAL_PRIO(ptcb);
        INSERT_READY_LIST_REAR(GET_TASK_ID(ptcb), prio);
             
     /******************************************************************
        The algorithm of schedule below can be optimized.        
     **********************************************************************/
        #if (OS_SCHEDULE_POLICY != OS_SCHEDULE_NON)
            if (NOT_IN_ISR
            #if (OS_SCHEDULE_POLICY == OS_SCHEDULE_MIX) /*Mixed schedule with Non-preemptive task*/
                 && FULL_SCHED == GET_TASK_SCHED(OSTCBCur) /*Preemptive tasks*/
            #endif /*#ifdef   OS_MIX_SCHEDULE*/
            )
            {  /*Preemptive Task, Not in ISR and the scheuler is unlocked*/
                if (GET_TASK_ORIGINAL_PRIO(ptcb) > GET_TASK_RUN_PRIO(curTcb))
                 /*if the priority of task with the event is higher than the current running task*/
                {
                     /*Preempt by the task with higher priority*/
                    //RELEASE_INT_RES(curTcb); //OS223:Points of rescheduling 
                    OS_TASK_SWITCH(curTcb);
                    OSPostTaskHook(); /*Call PostTaskHook before the current switch. */
                    OSTCBCur = ptcb;

                    GET_INT_RES(ptcb);/*Get the internal resource for timing protection*/
                    
                    OSPreTaskHook(); /*Call PreTaskHook*/
                    OSStackMonitoring(curTcb);

                    SET_TASK_STATUS(ptcb, RUN_READY); /*Set Status of task*/

                    OSTaskSaveLoadRun(curTcb, ptcb); /*Save the current task and run a saved task*/
                }
            }
        #endif /*#ifndef OS_NON_SCHEDULE*/
    }
    EXIT_CRITICAL();
    return E_OK;

}

/**********************************************************************
* Function Prototype��StatusType ClearEvent(EventMaskType Mask)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): Mask: the mask of event   
*    (out): 
* Return Value: E_OK:successful
*        E_OS_ID��Invalid TaskID or tcb hasn't been initiallized.
*       E_OS_ACCESS��the task is not the extended task
*       E_OS_CALLEVEL��the function is called in ISR
* Caller: None
* Call Function��
* Description: Clear the event according the mask
* Author:Jiang.Li
* Time:
***************************************************************************
   */ 
StatusType ClearEvent(EventMaskType Mask)
{
    DEFINECRITICAL();

    #if (OS_EXTENDED_STATUS == STD_ON)

        if (EXTENDED != GET_TASK_TYPE(OSTCBCur))
        {
            OSRETERROR(E_OS_ACCESS, OSServiceId_ClearEvent, 0U);
            /*Error, the task is not the extended task*/
        }
        if (!NOT_IN_ISR)
        {
            OSRETERROR(E_OS_CALLEVEL, OSServiceId_ClearEvent, 0U);
            /*Error, the function is called in ISR*/
        }
        if((Mask & OSTCBCur->OsTaskEventRef)==0)
        {
             /*Error, the task configurated without using this event*/
            OSRETERROR(E_OS_STATE, OSServiceId_SetEvent, 0U);
        }

    #endif /*#ifdef OS_EXTENDED_STATUS*/
    ENTER_CRITICAL();
    CLERA_TASK_MASK_EVENT(OSTCBCur, Mask); /*Clear the event*/
    EXIT_CRITICAL();
    return E_OK;
}

/**********************************************************************
* Function Prototype��StatusType GetEvent(TaskType ptcb, EventMaskRefType Event)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): TaskID: the ID of task
*           Event: pointer to the event info
*    (out): 
* Return Value: E_OK:successful
*        E_OS_ID��Invalid TaskID or tcb hasn't been initiallized.
*       E_OS_ACCESS��the task is not the extended task
*       E_OS_STATE��the task in suspended state
* Caller: None
* Call Function��
* Description: Return the event status of the given task
* Author:Jiang.Li
* Time:
***************************************************************************
*/ 
StatusType GetEvent(TaskType ptcb, EventMaskRefType Event)
{
   uint16 taskid,AppID;

    #if (OS_EXTENDED_STATUS == STD_ON)

        if ((ptcb >= (OSTCBTbl + OS_NUM_SYS_TASK)) || (ptcb < OSTCBTbl) || (((AddrType)ptcb - (AddrType)OSTCBTbl) % sizeof
            (OSTCB) != 0U))
        {
             /*Error��ptcb out of range*/
            OSRETERROR(E_OS_ID, OSServiceId_GetEvent, ptcb);
        }
        if (EXTENDED != GET_TASK_TYPE(ptcb))
        {
             /*Error, the task is not the extended task*/
            OSRETERROR(E_OS_ACCESS, OSServiceId_GetEvent, ptcb);
        }

        if (SUSPENDED == GET_TASK_STATUS(ptcb))
        {
             /*Error,�the task in suspended state*/
            OSRETERROR(E_OS_STATE, OSServiceId_GetEvent, ptcb);
        }
        
        
       #if ((OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
           AppID = GetApplicationID();
           taskid = ptcb-> OSTCBId;
           if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_TASK,taskid) ) 
           {
            OSRETERROR(E_OS_ACCESS, OSServiceId_GetEvent, ptcb);
           }
        #endif   

    #endif /*#ifdef OS_EXTENDED_STATUS*/

    *Event = GET_TASK_SET_EVENT(ptcb); /*Return the event info of the task*/
    return E_OK;
}


/**********************************************************************
* Function Prototype��StatusType WaitEvent( EventMaskType Mask)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): Mask: the mask of event   
*    (out): 
* Return Value: E_OK:successful
*        E_OS_ACCESS��the task is not the extended task
*       E_OS_RESOURCE��the task is not the extended task
*       E_OS_CALLEVEL��the function is called in ISR
* Caller: None
* Call Function��
* Description: Change the status of task to WAITING for the event until the event is set.
* Author:Jiang.Li
* Time:
***************************************************************************
*/ 
StatusType WaitEvent(EventMaskType Mask)
{
    TaskType curTCB = NULL,ptcb = NULL;
    uint8 *ReadyListPtr,highestPrio;
    uint16 pos;
    uint8 CurPrio;
    
    #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON)
        uint8 i = 0U; /*Used while OutResource enabled*/
    #endif
    DEFINECRITICAL();

    #if (OS_EXTENDED_STATUS == STD_ON)

        if (EXTENDED != GET_TASK_TYPE(OSTCBCur))
        {
             /*Error��the task is not the extended task*/
            OSRETERROR(E_OS_ACCESS, OSServiceId_WaitEvent, 0U);
        }
        if (!NOT_IN_ISR)
        {
             /*Error��the function is called in ISR*/
            OSRETERROR(E_OS_CALLEVEL, OSServiceId_WaitEvent, 0U);
        }
        
        if((Mask & OSTCBCur->OsTaskEventRef)==0)
        {
           /*Error, the task configurated without using this event*/
            OSRETERROR(E_OS_STATE, OSServiceId_SetEvent, ptcb);
        }

        #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON)
            if (NO_RESOURCE != GET_TASK_OUT_RES(OSTCBCur)
            #if (OS_RES_INTERRUPT == STD_ON)
                 || NO_RESOURCE != OSISRLastResource
            #endif /*ifdef   OS_ISR_RES*/
            )
            {
                OSRETERROR(E_OS_RESOURCE, OSServiceId_WaitEvent, 0U);
            }
        #endif /*#ifdef OS_OUT_RES||defined(OS_RES_SCHEDULER)*/

    #endif /* #ifdef OS_EXTENDED_STATUS */

    ENTER_CRITICAL();
    curTCB = OSTCBCur;
    SET_TASK_WAIT_EVENT(curTCB, Mask); /*Set the wait event of the task*/
    if (0U == (GET_TASK_SET_EVENT(curTCB) &Mask))
    {
        /*if the wait event is not set, start a schedule*/
        RELEASE_INT_RES(curTCB); /*Release the Resource,not take timing protection into consideration*/        
        OSPostTaskHook();
        CurPrio = GET_TASK_ORIGINAL_PRIO(curTCB);
        /*delete the tcb from the head of ready list*/
        REMOVE_READY_LIST_HEAD(CurPrio);
        
          
        /*Set the ready status table*/
        SET_TASK_UNREADY(GET_TASK_ORIGINAL_PRIO(curTCB), curTCB, WAITING); /*Change the status of the task to WAITING*/ 
        #if (OS_TIMING_PROTECTION_MODULE ==  STD_ON) 
           curTCB->RemainingExecutionBudget = curTCB->OsTaskExecutionBudget;//Reset the time of RemainingExecutionBudget
        #endif
        SET_TASK_OLD(curTCB);
        /*find the task with the highest priority, start a schedule*/
        
        highestPrio = OS_FIND_HIGHEST_TASK();                                  /*find the task with the highest priority among the tasks in Ready status*/
        ReadyListPtr=ReadyListCBTbl[highestPrio].head    ; /*Point to the Readylist with highest priority*/
     
        OSTCBCur=ptcb=&OSTCBTbl[*(ReadyListPtr+ReadyListCBTbl[highestPrio].start)];/*Update OSTCBCur*/
        GET_INT_RES(ptcb); /*Get internal resource*/
        
        OSPreTaskHook(); /*Call PreTaskHook*/
        OSStackMonitoring(curTCB);
        if (RUN_READY != GET_TASK_STATUS(ptcb))
        {
             /*If the task hasn't ever run once*/
            SET_TASK_STATUS(ptcb, RUN_READY); /*Set the status*/
            OSTaskSaveRun(curTCB, ptcb);
        }
        else
        {
            OSTaskSaveLoadRun(curTCB, ptcb);
        }
      
    }
 /*If the event has already set,there's no need to change the status to WAITING of task*/
    EXIT_CRITICAL();
 
    return E_OK;
}

#if (OS_ALARM_MODULE == STD_ON)
/**********************************************************************
* Function Prototype��void OSSetEvent( TaskType TaskID, EventMaskRefType Mask)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): Mask: the mask of event  
*           TaskID: the ID of task 
*    (out): 
* Return Value: E_OK:successful
*        E_OS_ACCESS��the task is not the extended task
*       E_OS_ID��Invalid TaskID or tcb hasn't been initiallized.
*       E_OS_STATE��the function is called in ISR
* Caller: None
* Call Function��
* Description: Set the task's evet.If the set event is the task wait for,it may cause schedule.
* Author:Jiang.Li
* Time:
****************************************************************************/ 
void OSSetEvent(TaskType ptcb, EventMaskRefType Mask)
{
    TaskType curTcb = NULL;
    uint8 *ReadyListPtr;
    uint16 pos,taskid,AppID;
    
    #if (OS_TIMING_PROTECTION_MODULE ==  STD_ON)
        uint32 ticks;
    #endif

  #if ((OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
     AppID = GetApplicationID();
     taskid = ptcb-> OSTCBId;
     if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_TASK,taskid) )
        return ;
  #endif   

  curTcb = OSTCBCur;

  SET_TASK_SET_EVENT(ptcb,  *Mask); /*Set the event mask of the task*/

  if ((WAITING == GET_TASK_STATUS(ptcb))&&(GET_TASK_SET_EVENT(ptcb) &GET_TASK_WAIT_EVENT(ptcb)))
  {

     /*timing protection��caculate the ticks of interval, to see if the timing protection is satisfied.If so activate the task.*/
    #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
      #if (OS_APPLICATION_MODULE == STD_ON)
         AppID = CheckObjectOwnership(OBJECT_TASK,taskid);
         if((FALSE == ApplicationCB[AppID].OsTrusted) ||(TRUE == ApplicationCB[AppID].TimingProtection)) 
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
              }
           
            ptcb->CrossNum =  MaxCrossTime; /*Update CrossTime*/
            ptcb->TimeFrameStart =  GET_COUNTER_VALUE(OSCounters);/*Update TimeFrameStart*/
         }
        #endif
     #endif
   /*timing protection*/

      SET_TASK_STATUS(ptcb, RUN_READY); /*if the event set is the event the task waiting for,change the state of the task to READY*/
      OS_TASK_TO_READY(GET_TASK_ORIGINAL_PRIO(ptcb), ptcb);
      INSERT_READY_LIST_REAR(GET_TASK_ID(ptcb), GET_TASK_ORIGINAL_PRIO(ptcb));
      /******************************************************************
      The algorithm of schedule below can be optimized.       
       **********************************************************************/
      #if (OS_SCHEDULE_POLICY != OS_SCHEDULE_NON)
          if (NOT_IN_ISR
          #if (OS_SCHEDULE_POLICY == OS_SCHEDULE_MIX) /*Mixed schedule with Non-preemptive task*/
               && FULL_SCHED == GET_TASK_SCHED(OSTCBCur) /*Preemptive tasks*/
          #endif /*#ifdef   OS_MIX_SCHEDULE*/
          )
          {
              /*Preemptive Task, Not in ISR and the scheuler is unlocked*/
              if (GET_TASK_ORIGINAL_PRIO(ptcb) > GET_TASK_RUN_PRIO(curTcb))
               /*if the priority of task with the event is higher than the current running task*/
              {
                   /*Preempt by the task with higher priority*/
                   
                  //RELEASE_INT_RES(curTcb); //OS223:Points of rescheduling 
                  OS_TASK_SWITCH(curTcb);
                  OSPostTaskHook(); /*Call PostTaskHook before the current switch.*/
                  OSTCBCur = ptcb;
                  GET_INT_RES(ptcb);  /*Get the internal resource for timing protection*/
                
                  OSPreTaskHook(); /*Call PreTaskHook*/
                  OSStackMonitoring(curTcb);

                  SET_TASK_STATUS(ptcb, RUN_READY); /*Set Status of task*/

                  OSTaskSaveLoadRun(curTcb, ptcb); /*Save the current task and run a saved task*/
              }
          }

      #endif /*#ifndef OS_NON_SCHEDULE*/
  }

}
  #endif /*#ifdef OS_ALARM_EN*/

#endif /* #ifdef OS_EVENT_EN */

#endif /*#ifdef OS_EVENT_C*/
