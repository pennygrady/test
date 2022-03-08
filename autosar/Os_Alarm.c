/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Alarm.c
* DESCRIPTION: OsAlarm module source file based on AUTOSAR OS standard 
* UPDATE HISTORY
* REV         AUTHOR                  DATE       DESCRIPTION OF CHANGE                         
* 1.0         Shuxin.Lin              23/DEC/03        
* 1.1         Hangping.Pan            27/SEP/04        
* 1.2         Xiaonan.Meng&Jiang.Li 1/MAR/05       
* 1.3         Xiaonan.Meng          7/MAR/05    
* 1.4         Luchuan.Xu              7/JUN/05        
* 1.5         kun.li              4/July/13      modify OSAlarmInit
*********************************************************************************
*/

#ifndef OS_ALARM_C
    #define OS_ALARM_C
/******************************************************************
| includes
| 1) system and project includes
| 2) needed interfaces from external units
| 3) internal and external interfaces from this unit
|-----------------------------------------------------------------*/

#include "NonPlat_Lib.h"
#include "Os.h"
#if (OS_ALARM_MODULE == STD_ON)   
/*******************************************************************
| external declarations:
| function prototypes (scope: module-exported)
| extern variable  (scope: module-exported)
|------------------------------------------------------------------*/


extern OsCounter OSCounterCfgTbl[];   /*Counter configuration structure array*/
extern OsAlarm OSAlarmCfgTbl[];  /*Alarm configuration structure array*/

#if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
    extern uint32 MaxCrossTime;
    #if (OS_RES_SCHEDULER == STD_ON)
      extern AlarmCB OSResourceAlarm[OS_RES_NUM+1U];
    #else 
      extern AlarmCB OSResourceAlarm[OS_RES_NUM];
    #endif
#endif

#if (OS_SCHEDULE_TABLE_MODULE == STD_ON)   
   extern boolean OSProcessScheduleTablesAlarm(AlarmCBRef SchTblAlarm,AlarmCBRef *tempNextSchTblAlarm);
   extern void OSNotifyScheduleTables(AlarmCBRef SchTblAlarm);
#endif /*OS_SCHEDULE_TABLE_MODULE*/
    
/*******************************************************************
| global variable definitions (scope: module-local)
|------------------------------------------------------------------*/    
AlarmCB OSAlarms[OS_ALARM_NUM]; /*Control structure of OS Alarm*/
CounterCB OSCounters[OS_COUNTER_NUM]; /*Control structure of OS Counter*/    

/*******************************************************************
| function prototypes (scope: module-local)
|------------------------------------------------------------------*/
void OSCheckAlarms(CounterType ctrId);
void OSNotifyAlarm(AlarmCBRef alarm);
void OSInsertAlarm(AlarmCBRef alarm);
/******************************************************************* 
| function implementations (scope: module-exported) 
|------------------------------------------------------------------*/

/**********************************************************************
* Function Prototype: void OSAlarmInit(void)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): 
*    (out): 
* Return Value: 
* Caller: StartOS
* Call Function: 
* Description: Initialize OsAlarm module
* Author: Jiang Li
* Time: 2005.7.6
***************************************************************************
*/
void OSAlarmInit()
{
    AlarmType alarm = OSAlarms;
    OsAlarm *alarmCfg = OSAlarmCfgTbl;
    CounterType counter = OSCounters;
    AlarmBaseRefType counterCfg = OSCounterCfgTbl;
    TaskType ptcb;
    uint8 *ReadyListPtr;
    uint16 pos=0,i=0U;
#if(OS_APPLICATION_MODULE == STD_ON)
    uint16 AppID, CounterID;
#endif
    
    for (i=0U; i < OS_COUNTER_NUM; i++)
    {
        counter = OSCounters + i;
        counterCfg = OSCounterCfgTbl + i ; 
         /*Initialize Counter*/
        counter->CtrInfo = counterCfg;
        counter->value = 0U;
        counter->raw_value = 0U;
        counter->head = NULL;
          /*Application*/
        #if (OS_APPLICATION_MODULE == STD_ON)
        counter->OsCounterAccessingApplication = counterCfg->OsCounterAccessingApplication;
        counter->id = i;
        counter->OsCounterOwnershipApp = OSCheckObjectOwnership(OBJECT_ALARM,i);        
        #endif
    }
       
   
    for (i=0U; i <  OS_ALARM_NUM; i++)
    {
         /*Initialize Alarm*/
        alarm = OSAlarms + i;
        alarmCfg = OSAlarmCfgTbl + i;
        alarm->start = AlarmUnUsed;
        alarm->AlarmApp = COMMONALARM;

        alarm->ctr =  &OSCounters[alarmCfg->OsAlarmCounterRef];
      
    #if (OS_APPLICATION_MODULE ==STD_ON )    /*if Application module is contained, check if all tasks are valid*/ 
        alarm->OsAlarmAccessingApplication = alarmCfg->OsAlarmAccessingApplication;
        AppID = OSCheckObjectOwnership (OBJECT_ALARM,i);
        alarm->OsAlarmOwnershipApp = AppID;
        ptcb = &OSTCBTbl[alarmCfg->action.task] ;
        if(ACCESS == CheckObjectAccess(AppID,OBJECT_TASK,ptcb->OSTCBId)) 
        {
          alarm->task = ptcb;
        }

         
      #if (OS_EVENT_MODULE == STD_ON)
        if((alarmCfg->action.event.OsAlarmSetEventRef)!=NULL) 
        {          
           ptcb = &OSTCBTbl[alarmCfg->action.event.osAlarmSetEventTaskRef] ;
           if(ACCESS == CheckObjectAccess(AppID,OBJECT_TASK,ptcb->OSTCBId)) 
           {
             alarm->event = alarmCfg->action.event.OsAlarmSetEventRef;
             alarm->task = ptcb;

           }
        }
      #endif 
        
       alarm->id = i;
       CounterID=  alarmCfg->action.IncrementCounterID.OsAlarmIncrementCounterRef;
       if(ACCESS == CheckObjectAccess(AppID,OBJECT_COUNTER,CounterID)) 
          alarm->IncrementCounterID =  CounterID;
       else
          alarm->IncrementCounterID = INVALID_COUNTER;

    #endif
 
      #if (OS_APPLICATION_MODULE ==STD_OFF )
          alarm->task = &OSTCBTbl[alarmCfg->action.task];
          
         #if (OS_EVENT_MODULE == STD_ON)
          if((alarmCfg->action.event.OsAlarmSetEventRef)!=NULL) 
          {
            alarm->event = alarmCfg->action.event.OsAlarmSetEventRef;
            alarm->task = &OSTCBTbl[alarmCfg->action.event.osAlarmSetEventTaskRef];
          }
         #endif 
                  
          alarm->IncrementCounterID =  alarmCfg->action.IncrementCounterID.OsAlarmIncrementCounterRef;

      #endif /*#ifdef OS_EVENT_EN*/

        #if (OS_ALARM_CALLBACK == STD_ON)
            alarm->AlarmCallBack = alarmCfg->action.callback.AlarmCallBackName;
        #endif /*#ifdef OS_ALARM_CALLBACK_EN*/
        
        
        if ((alarmCfg->autostart != NULL)&&(ActiveAppMode & alarmCfg->autostart->OsAlarmAppModeRef))
         /* If alarm is self-starting, set the delta and the cycle */
        {
            alarm->start = AlarmUsed;
            alarm->cycle = alarmCfg->autostart->OsAlarmCycleTime;

            if(((0U == alarmCfg->autostart->OsAlarmAlarmTime) && (ALARM_RELATIVE == alarmCfg->autostart->autostarttype))||((GET_ALARM_COUNTER_VALUE(alarm) == alarmCfg->autostart->OsAlarmAlarmTime) && (ALARM_ABSOLUTE == alarmCfg->autostart->autostarttype)))
            {
                /* If task need to be started immediately*/
                if(alarm->IncrementCounterID != INVALID_COUNTER)
                {
                    OSAlarmCounterTrigger(&OSCounters[alarm->IncrementCounterID]);
               } else{
                
               #if (OS_ALARM_CALLBACK == STD_ON)
                  if (alarm->AlarmCallBack != NULL)
                  {
                     alarm->AlarmCallBack();
                  }
                  else
                  {
               #endif

               #if (OS_EVENT_MODULE == STD_ON)
                   if (GET_ALARM_EVENT(alarm) != NULL)
                   {
                      OSSetEvent(GET_ALARM_TASK(alarm), (GET_ALARM_EVENT(alarm)));
                   }
                   else
                   {
               #endif
               #if(OS_TASK_NUM > 0U)
                      ptcb = GET_ALARM_TASK(alarm);                  
                      SET_TASK_READY(GET_TASK_ORIGINAL_PRIO(ptcb), ptcb); /*Set task status and put it in the ready state*/
                      INC_TASK_CUR_ACT(ptcb);
                      INSERT_READY_LIST_REAR(GET_TASK_ID(ptcb), GET_TASK_ORIGINAL_PRIO(ptcb)); /*Insert task into the ready queue*/
                 #endif 
                 
                #if (OS_EVENT_MODULE == STD_ON)
                   }
                #endif

                #if (OS_ALARM_CALLBACK == STD_ON)

                  }
                #endif
               }
                            
                 /*If it is a cycle Alarm*/                
               if(0U != GET_ALARM_CYCLE(alarm)) 
               {/*If it is not a one-time alarm, update the delta value, and insert the alarm in the used alarm list*/                  
                   alarm->delta = GET_ALARM_CYCLE(alarm) <= GET_ALARM_COUNTER_MAXVALUE(alarm)- GET_ALARM_COUNTER_VALUE(alarm) ? GET_ALARM_CYCLE(alarm) + \
                   GET_ALARM_COUNTER_VALUE(alarm) : GET_ALARM_CYCLE(alarm)-(GET_ALARM_COUNTER_MAXVALUE(alarm)-GET_ALARM_COUNTER_VALUE(alarm))-1U;

                 OSInsertAlarm(alarm);                 
               }
            }else{/* If alarm can not start immediately*/
                if(ALARM_RELATIVE == alarmCfg->autostart->autostarttype)
                {
                    alarm->delta = GET_ALARM_CYCLE(alarm) <= GET_ALARM_COUNTER_MAXVALUE(alarm)- GET_ALARM_COUNTER_VALUE(alarm) ? GET_ALARM_CYCLE(alarm) + \
                               GET_ALARM_COUNTER_VALUE(alarm) : GET_ALARM_CYCLE(alarm)-(GET_ALARM_COUNTER_MAXVALUE(alarm)-GET_ALARM_COUNTER_VALUE(alarm))-1U;
                }
                else
                {
                    alarm->delta = alarmCfg->autostart->OsAlarmAlarmTime;
                }
                OSInsertAlarm(alarm);
            }
        }
    }
}
/**********************************************************************
* Function Prototype: void OSInsertAlarm(AlarmCBRef alarm)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): alarm
*    (out): 
* Return Value: 
* Caller: SetAbsAlarm,SetRelAlarm,DefineAutoAlarm
* Call Function: 
* Description: According to the order of the next trigger moment, insert the alarm into the alarm list which is mounted on the related counter,
* and set the alarm status to be activated. If the alarm is the new list head, then update the ctrDelta of the counter.
 
* Author: Luchuan Xu
* Time: 2005.6
***************************************************************************
*/
void OSInsertAlarm(AlarmCBRef alarm)
{
    
    AlarmCBRef temp, last,head;
    TickType value;
#if(OS_APPLICATION_MODULE == STD_ON)
    uint16 AppID;
#endif
    
  #if( (OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
     AppID = GetApplicationID();
     if((alarm->AlarmApp == COMMON_TYPE) && (NO_ACCESS == CheckObjectAccess(AppID,OBJECT_ALARM,alarm->id)))
        return ;
  #endif
  
    head = GET_ALARM_COUNTER_HEAD(alarm);
    value = GET_ALARM_COUNTER_VALUE(alarm);
    SET_ALARM_USED(alarm);

    /*If the list is empty, then regard directly to the alarm as a linked list head*/
    if (head == NULL)
    {
        SET_ALARM_COUNTER_HEAD(alarm, alarm);
        SET_NEXT_ALARM(alarm, NULL);
        SET_PRV_ALARM(alarm, NULL);

        /*If list head is changed, then update ctrDelta*/
        alarm->ctr->ctrDelta = GET_ALARM_DELTA(alarm);       
        return ;
    }

    /*Insert alarm into the alarm list*/
    temp = head;
    last = NULL; /*use 'last' to save previous node of the 'temp' node */
    /*1). find the first node will be triggered after the 'alarm' node, store it in 'temp' node*/
    while (temp != NULL)
    {
        if (GET_ALARM_DELTA(temp) < value)
        {
            if (GET_ALARM_DELTA(alarm) < value)
            {
                if (GET_ALARM_DELTA(temp) >= GET_ALARM_DELTA(alarm))
                {
                    break; /*find*/
                }
            }else  /* alarm->delta > value*/
            {
                break; /*find*/
            }
        }else  /* temp->delta > value*/
        {
            if (GET_ALARM_DELTA(alarm) > value)
            {
                if (GET_ALARM_DELTA(temp) >= GET_ALARM_DELTA(alarm))
                {
                    break; /*find*/
                }
            }
        }
        last = temp;
        temp = GET_NEXT_ALARM(temp);
    }

    /*2). Inserted the 'temp' node in front of the 'alarm' node*/
    alarm->prev = last;
    if (last)
    {
        last->next = alarm;
    }

    alarm->next = temp;
    if (temp)
    {
        temp->prev = alarm;
    }

    /*If the 'last' node is empty, then the 'alarm' will be inserted into the list as the list head, then ctrDelta need to be updated.*/
    if (!last)
    {
        SET_ALARM_COUNTER_HEAD(alarm, alarm);
        alarm->ctr->ctrDelta = GET_ALARM_DELTA(alarm);
    }  
}
/**********************************************************************
* Function Prototype: void OSDeleteAlarm(AlarmCBRef alarm)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): alarm
*    (out): 
* Return Value: 
* Caller: CancelAlarm
* Call Function: 
* Description: Delete the alarm from the alarm list which is mounting on the counter which the alarm belong to, and set the alarm status to be inactivated;
* If the alarm which will be deleted is the head list, then update the ctrDelta value of the Counter.
* Author: Luchuan Xu
* Time: 2005.6
***************************************************************************
*/
void OSDeleteAlarm(AlarmCBRef alarm)
{
#if(OS_APPLICATION_MODULE == STD_ON)
    uint16 AppID;
#endif
    DEFINECRITICAL();

    ENTER_CRITICAL();
      
    #if( (OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
     AppID = GetApplicationID();
     if((alarm->AlarmApp == COMMON_TYPE) && (NO_ACCESS == CheckObjectAccess(AppID,OBJECT_ALARM,alarm->id)))
        return ;
    #endif
    
    if (alarm == GET_ALARM_COUNTER_HEAD(alarm))
    {
         /*'alarm' is the list head*/
         
        /*remove the alarm from the alarm list*/
        SET_ALARM_COUNTER_HEAD(alarm, GET_NEXT_ALARM(alarm));
        
        /* If there is one more alarm, the next node will become the new list head, then update ctrDelta */

        if (GET_NEXT_ALARM(alarm))
        {
            SET_PRV_ALARM(GET_NEXT_ALARM(alarm), NULL);
            alarm->ctr->ctrDelta = GET_ALARM_DELTA(GET_NEXT_ALARM(alarm));
        }
    }
    else
    {
         /*remove the alarm from the alarm list*/
        SET_NEXT_ALARM(GET_PRV_ALARM(alarm), GET_NEXT_ALARM(alarm));
        if (GET_NEXT_ALARM(alarm))
        {
            SET_PRV_ALARM(GET_NEXT_ALARM(alarm), GET_PRV_ALARM(alarm));
        }
    }

    SET_NEXT_ALARM(alarm, NULL);
    SET_PRV_ALARM(alarm, NULL);
    SET_ALARM_UNUSED(alarm); /*reset the alarm to be unused*/
    EXIT_CRITICAL();

}
/**********************************************************************
* Function Prototype: void OSCounterTrigger(CounterType ctr)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): ctr
*    (out): 
* Return Value: 
* Caller:DECHandler(OS_CPU_C.c)
* Call Function: 
* Description: Add 1 to the counter value. Check alarms if triggerred at this moment.
* Author: Hangping Pan, Xiaonan Meng, Luchuan Xu
* Time: 2005.6
***************************************************************************
*/
void OSCounterTrigger(CounterType ctr)
{
    COUNTER_RAW_VALUE_GROW(ctr);
    CounterRdy = 0; //add by jerry
    if(ctr->CtrInfo->OsCounterTicksPerBase == ctr->raw_value)
    {     
        CounterRdy = 1; //add by jerry
        if(GET_COUNTER_VALUE(ctr) == GET_COUNTER_MAXVALUE(ctr)) 
        {
            SET_COUNTER_VALUE(ctr, 0U);
            #if (OS_TIMING_PROTECTION_MODULE ==  STD_ON)
                ++MaxCrossTime;
            #endif
        } 
        else 
        {
            COUNTER_VALUE_GROW(ctr);
        }                  
        ctr->raw_value = 0U;             
                 
         
        #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)             
        if(IntNesting==1U)
        {
            if(--OSTCBCur->RemainingExecutionBudget==0U)
            {
              /*Execution time has been out of budget*/
              /*tectionHook( E_OS_PROTECTION_TIME);*/
              #if (OS_PROTECTION_HOOK == STD_ON)
              OSProtectionHook(E_OS_PROTECTION_TIME,OSTCBCur);
              #endif             
            }
        }
        #endif
   
        if (GET_HEAD_ALARM(ctr) != NULL)
        /* Check whether there still are enabled alarms */
        {
           
            if (GET_COUNTER_CTRDELTA(ctr) == GET_COUNTER_VALUE(ctr))
            {
           
                /*At least one alarm will be triggered, so perform checking operation, update ctrDelta */
                OSCheckAlarms(ctr);
            }
        }
    }    
    return ;    

}
/**********************************************************************
* Function Prototype: void OSAlarmCounterTrigger(CounterType ctr)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): ctr
*    (out): 
* Return Value: 
* Caller: DECHandler(OS_CPU_C.c)
* Call Function: 
* Description: Add 1 to the counter value. Check alarms if triggerred at this moment.
* Author: Luchuan Xu
* Time: 2005.6
***************************************************************************
*/ 
void OSAlarmCounterTrigger(CounterType ctr)
{
     
     if(GET_COUNTER_VALUE(ctr) == GET_COUNTER_MAXVALUE(ctr)) 
     {
         SET_COUNTER_VALUE(ctr, 0U);
      #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
            ++MaxCrossTime;
     #endif
     } else{           
          COUNTER_VALUE_GROW(ctr);  
     }

    if (GET_HEAD_ALARM(ctr) != NULL)
    /* Check whether there still are enabled alarms */
    {
        if (GET_COUNTER_CTRDELTA(ctr) == GET_COUNTER_VALUE(ctr))
        {
            /*At least one alarm will be triggered, so perform checking operation, update ctrDelta */
            OSCheckAlarms(ctr);
        }
    }
    
    #if (OS_TIMING_PROTECTION_MODULE ==  STD_ON)             
     if(IntNesting==1U)
     {
        if(--OSTCBCur->RemainingExecutionBudget==0U)
        {
          /*Excution time has been out of budget*/
          /* ProtectionHook( E_OS_PROTECTION_TIME);*/
          #if (OS_PROTECTION_HOOK == STD_ON)
          OSProtectionHook(E_OS_PROTECTION_TIME,OSTCBCur);
          #endif        
        }
    }
  #endif  
}
/**********************************************************************
* Function Prototype: void OSCheckAlarms(CounterType ctr)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): ctr
*    (out): 
* Return Value: 
* Caller: OSISRCounterTrigger
* Call Function: 
* Description: trigger all the expired alarms which are mounting on the counter 
* specified by the ctr  
* Author: Luchuan Xu
* Time: 2005.6
***************************************************************************
*/
void OSCheckAlarms(CounterType ctr)
{

    AlarmCBRef alarmRef,alarmRef2;
    AlarmCBRef temp,temp2,last,head = NULL;
    TickType value = GET_COUNTER_VALUE(ctr);

#if (OS_SCHEDULE_TABLE_MODULE == STD_ON)
    AlarmCBRef tempNextSchTblAlarm;  /*If next schedule table start, then the variant is used to store the corresponding ScheduleTable Alarm */

#endif /*OS_SCHEDULE_TABLE_MODULE*/ 

    alarmRef = alarmRef2 = GET_HEAD_ALARM(ctr);

    /*Traverse the front k-triggered alarms, and triggers it (here k >= 1) */
    while (alarmRef != NULL)
    {
        if (GET_ALARM_DELTA(alarmRef) != value)
        {
            break;
        }
         
        /*Trigger the alarm, if it is a cycle alarm, then update its delta value (the next time to be triggerring) */

      #if (OS_SCHEDULE_TABLE_MODULE == STD_ON )
        if(alarmRef->AlarmApp == COMMONALARM)
      #endif /*OS_SCHEDULE_TABLE_MODULE*/   
           { 
                   OSNotifyAlarm(alarmRef);
           }
               
      #if (OS_SCHEDULE_TABLE_MODULE == STD_ON )  
                else 
                {
                #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
                   if(alarmRef->AlarmApp == RESALARM)
                   {/*block resource has been out of budget, the the hook need to be called, and report this error*/
                    /* id =  alarmRef > OSResourceAlarm ? alarmRef - OSResourceAlarm : OSResourceAlarm - alarmRef;
                     id = id/sizeof(AlarmCB);The resouce Id which has been preempted longer than budget. The type of resources have to be considerred*/
                    
                    /* ProtectionHook(E_OS_PROTECTION_LOCKED); hook????�?????d???????*/
                     #if (OS_PROTECTION_HOOK == STD_ON)
                          OSProtectionHook(E_OS_PROTECTION_TIME,OSTCBCur);
                       #endif
                    /*OSRETERROR(E_OS_PROTECTION_LOCKED, OSServiceId_CheckAlarms, ptcb);                          */
                   } 
                #endif
                
                #if (OS_SCHEDULE_TABLE_MODULE == STD_ON )
                   if(alarmRef->AlarmApp == SCHETBLALARM)
                   {
                     OSNotifyScheduleTables(alarmRef);
                   }
                #endif
                }
      #endif /*OS_SCHEDULE_TABLE_MODULE*/                       
      alarmRef = GET_NEXT_ALARM(alarmRef);
    }
    
    /*the pointer point the list head of the un-triggered alarms*/
    head = alarmRef;
    /*The second traversal, find the k-front alarms latest triggerred, then insert them into the un-triggerred alarm list*/
    while (alarmRef2 != alarmRef)
    {
        /*save alrmRef2->next into temp2*/
        temp2 = GET_NEXT_ALARM(alarmRef2);

     #if (OS_SCHEDULE_TABLE_MODULE == STD_ON ) 
      if(COMMONALARM==alarmRef2->AlarmApp || RESALARM ==alarmRef2->AlarmApp)
      {
     #endif  
        if (GET_ALARM_CYCLE(alarmRef2) == 0U)
        {
            /*Just remove the one-time alarm directly*/
            SET_NEXT_ALARM(alarmRef2, NULL);
            SET_PRV_ALARM(alarmRef2, NULL);
            SET_ALARM_UNUSED(alarmRef2);

            alarmRef2 = temp2;
            continue;
        }
     #if (OS_SCHEDULE_TABLE_MODULE == STD_ON ) 
      }                
     #endif /*OS_SCHEDULE_TABLE_MODULE*/ 
      
      #if (OS_SCHEDULE_TABLE_MODULE == STD_ON)  
            if(SCHETBLALARM==alarmRef2->AlarmApp)
      {
         tempNextSchTblAlarm =NULL;
         if(FALSE==OSProcessScheduleTablesAlarm(alarmRef2,&tempNextSchTblAlarm))
          { 
              /*The FinalDelay of the Schedule Table has been expired, and no new Schedule Table startd, so simply delete the alarm*/
            alarmRef2 = temp2; 
            continue;
          }
         else if(tempNextSchTblAlarm!=NULL) 
         { 
             /*The FinalDelay of the Schedule Table has been expired, and there is new Schedule Table to be startd, so delete the alarm related to the older 
             Schedule Table, and insert the new alarm which is related to the new Schedule Table*/  
            alarmRef2 = tempNextSchTblAlarm; 
         }
      }
      #endif /*OS_SCHEDULE_TABLE_MODULE*/   

        /*If the un-triggerred list is empty, then simply set the alarmRef2 to be the list head*/
        if (head == NULL)
        {
            head = alarmRef2;
            SET_NEXT_ALARM(alarmRef2, NULL);
            SET_PRV_ALARM(alarmRef2, NULL);

            alarmRef2 = temp2;
            continue;
        }

        /*Insert alarmRef2 to the list*/
        temp = head;
        last = NULL;
        /*Search the first node which will be triggerred after the aklarmRef2*/        
        while (temp != NULL)
        {
            if (GET_ALARM_DELTA(temp) < value)
            {
                if (GET_ALARM_DELTA(alarmRef2) < value)
                {
                    if (GET_ALARM_DELTA(temp) >= GET_ALARM_DELTA(alarmRef2))
                    {
                        break;
                    }
                }else /* alarmRef2 > value*/
                {
                    break;
                }
            }else /* temp->delta >value*/
            {
                if (GET_ALARM_DELTA(alarmRef2) > value)
                {
                    if (GET_ALARM_DELTA(temp) >= GET_ALARM_DELTA(alarmRef2))
                    {
                        break;
                    }
                }
            }
            last = temp;
            temp = GET_NEXT_ALARM(temp);
        }

        alarmRef2->prev = last;
        if (last)
        {
            last->next = alarmRef2;
        }

        alarmRef2->next = temp;
        if (temp)
        {
            temp->prev = alarmRef2;
        }

        if (!last)
        {
            head = alarmRef2;
        }
        /*Update alarmRef2 with the new alarm node*/
        alarmRef2 = temp2;
    }

    SET_HEAD_ALARM(ctr, head);
    if (head)
    {
        ctr->ctrDelta = GET_ALARM_DELTA(head);
    }
}
/**********************************************************************
* Function Prototype: void OSNotifyAlarm(AlarmCBRef alarm)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): alarm
*    (out): 
* Return Value: 
* Caller: OSCheckAlarms
* Call Function: SetEvent, ActivateTask, AlarmCallBack 
* Description: Perform the actions specified by this Alarm, then reset the delta value
* Author: Hangping Pan, Xiaonan Meng, Luchuan Xu
* Time: 2005.6
***************************************************************************
*/
void OSNotifyAlarm(AlarmCBRef alarm)
{
    TickType increment;
     if(alarm->IncrementCounterID != INVALID_COUNTER) 
     {
          OSAlarmCounterTrigger(&OSCounters[alarm->IncrementCounterID]);
     }else{
      
    #if (OS_ALARM_CALLBACK == STD_ON)
        if (alarm->AlarmCallBack != NULL)
        {
            alarm->AlarmCallBack();
        }
        else
        {
        #endif

        #if (OS_EVENT_MODULE == STD_ON)
            if (GET_ALARM_EVENT(alarm) != NULL)
            {
                OSSetEvent(GET_ALARM_TASK(alarm), (GET_ALARM_EVENT(alarm)));
            }
            else
            {
            #endif
            #if(OS_TASK_NUM > 0U)

            OSActivateTask(GET_ALARM_TASK(alarm));
            #endif
            
            #if (OS_EVENT_MODULE == STD_ON)
            }
        #endif

        #if (OS_ALARM_CALLBACK == STD_ON)
        }
    #endif
    }
    if (GET_ALARM_CYCLE(alarm) != 0U)
    {        
        increment = GET_ALARM_CYCLE(alarm) ;
        alarm->delta = increment <= GET_ALARM_COUNTER_MAXVALUE(alarm)- GET_ALARM_DELTA(alarm) ? increment+ \
        GET_ALARM_COUNTER_VALUE(alarm):increment-(GET_ALARM_COUNTER_MAXVALUE(alarm)-GET_ALARM_DELTA(alarm))-1U;
    }
    return ;
} 
/**********************************************************************
* Function Prototype: StatusType GetAlarmBase(AlarmType AlarmID, AlarmBaseRefType Info)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): AlarmID
*    (out): Info
* Return Value: E_OK, E_OS_ID 
* Caller:
* Call Function: 
* Description: Get the status of the counter specified by Alarm
* Author: Hangping Pan, Xiaonan Meng, Luchuan Xu
* Time: 2005.6
***************************************************************************
*/   
StatusType GetAlarmBase(AlarmType Alarm, AlarmBaseRefType Info)
{
#if(OS_APPLICATION_MODULE == STD_ON)
    uint16 AppID;
#endif
    #if (OS_EXTENDED_STATUS == STD_ON)
        if ((Alarm >= (OSAlarms + OS_ALARM_NUM)) || (Alarm < OSAlarms) || (((AddrType)Alarm - (AddrType)OSAlarms) %
            sizeof(AlarmCB)) != 0U)
         /* Check if the alarm is still valid*/
        {
            OSRETERROR(E_OS_ID, OSServiceId_GetAlarmBase, Alarm);
        }
    #endif
    
   #if( (OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
     AppID = GetApplicationID();
     if((Alarm->AlarmApp == COMMON_TYPE) && (NO_ACCESS == CheckObjectAccess(AppID,OBJECT_ALARM,Alarm->id))) 
     {
      OSRETERROR(E_OS_ACCESS, OSServiceId_GetAlarmBase, Alarm);
     }
   #endif
    /*Return the infomation of the counter */
     Lib_Memcpy((uint8*)Info, (uint8 *)(GET_ALARM_COUNTER_INFO(Alarm)), sizeof( AlarmBaseType));
    return E_OK;
}
/**********************************************************************
* Function Prototype: StatusType GetAlarm(AlarmType AlarmID, TickRefType tick)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): AlarmID
*    (out): tick
* Return Value: E_OK, E_OS_NOFUNC, E_OS_ID
* Caller:
* Call Function: 
* Description: Get the status and the remainning ticks of the alarm specified by the AlarmId
* Author: Hangping Pan, Xiaonan Meng, Luchuan Xu
* Time: 2005.6
***************************************************************************
*/
StatusType GetAlarm(AlarmType Alarm, TickRefType Tick)
{
#if(OS_APPLICATION_MODULE == STD_ON)
    uint16 AppID;
#endif
  #if( (OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
     AppID = GetApplicationID();
     if((Alarm->AlarmApp == COMMON_TYPE) && (NO_ACCESS == CheckObjectAccess(AppID,OBJECT_ALARM,Alarm->id))) 
     {
      OSRETERROR(E_OS_ACCESS, OSServiceId_GetAlarm, Alarm);
     }
  #endif
    #if (OS_EXTENDED_STATUS == STD_ON)
        if ((Alarm >= (OSAlarms + OS_ALARM_NUM)) || (Alarm < OSAlarms) || (((AddrType)Alarm - (AddrType)OSAlarms) %
            sizeof(AlarmCB)) != 0U)
        {
            OSRETERROR(E_OS_ID, OSServiceId_GetAlarm, Alarm);
        }

        if (GET_ALARM_STATE(Alarm) == AlarmUnUsed)
        {
            OSRETERROR(E_OS_NOFUNC, OSServiceId_GetAlarm, Alarm); /*the alarm is not used*/
        }
    #endif /*ifdef OS_EXTENDED_STATUS */

    *Tick = GET_ALARM_DELTA(Alarm) >= GET_ALARM_COUNTER_VALUE(Alarm) ? GET_ALARM_DELTA(Alarm) - GET_ALARM_COUNTER_VALUE
            (Alarm): GET_ALARM_COUNTER_MAXVALUE(Alarm) - GET_ALARM_COUNTER_VALUE(Alarm)+GET_ALARM_DELTA(Alarm);

    return E_OK;
} /* End of GetAlarm */  
/**********************************************************************
* Function Prototype: StatusType SetRelAlarm(AlarmType AlarmID, TickType increment, TickType cycle)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): Alarm, increment, cycle
*    (out): 
* Return Value: E_OK, E_OS_STATE, E_OS_ID, E_OS_VALUE
* Caller:
* Call Function: OSInsertAlarm, OSNotifyAlarm
* Description: Initial the Alarm specified by AlarmId to be a relative alarm, and set tick and increment
* Author: Hangping Pan, Xiaonan Meng, Luchuan Xu
* Time: 2005.6
***************************************************************************
*/
StatusType SetRelAlarm(AlarmType Alarm, TickType increment, TickType cycle)
{
#if(OS_APPLICATION_MODULE == STD_ON)
    uint16 AppID;
#endif
    DEFINECRITICAL();
    
  #if( (OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
     AppID = GetApplicationID();
     if((Alarm->AlarmApp == COMMON_TYPE) && (NO_ACCESS == CheckObjectAccess(AppID,OBJECT_ALARM,Alarm->id))) 
     {      
      OSRETERROR(E_OS_ACCESS, OSServiceId_SetRelAlarm, Alarm);
     }
  #endif
  
    #if (OS_EXTENDED_STATUS == STD_ON)
        if ((Alarm >= (OSAlarms + OS_ALARM_NUM)) || (Alarm < OSAlarms) || (((AddrType)Alarm - (AddrType)OSAlarms) %
            sizeof(AlarmCB)) != 0U)
        {
            OSRETERROR(E_OS_ID, OSServiceId_SetRelAlarm, Alarm); /* Alarm is out of the range */
        }
        if ((increment > GET_ALARM_COUNTER_MAXVALUE(Alarm)) || ((cycle > GET_ALARM_COUNTER_MAXVALUE(Alarm)) || (cycle !=
            0U && cycle < GET_ALARM_COUNTER_MINCYCLE(Alarm))))
        {
            OSRETERROR(E_OS_VALUE, OSServiceId_SetRelAlarm, Alarm); /*increment or cycle is out of the range*/
        } 
    #endif /*ifdef OS_EXTENDED_STATUS*/

    /*test and set operation*/
    ENTER_CRITICAL();
    if (GET_ALARM_STATE(Alarm) == AlarmUsed)
    {
        EXIT_CRITICAL();
        OSRETERROR(E_OS_STATE, OSServiceId_SetRelAlarm, Alarm); /* the alarm is used by other tasks */
    }
    EXIT_CRITICAL();
    

    /*If the increment is zero, the alarm will be triggerred immediately*/

    if((increment==0U) && (cycle == 0U))
    {
      OSNotifyAlarm(Alarm);             /*Trigger the alarm*/
      return E_OK;
    
    }
    else if((increment==0U) && (cycle!=0U))
    {
        ENTER_CRITICAL();
        SET_ALARM_CYCLE(Alarm, cycle);
      
      OSNotifyAlarm(Alarm);                     /*Trigger the alarm*/
      
      OSInsertAlarm(Alarm);
      EXIT_CRITICAL();      
    }
    else
    {
      
        ENTER_CRITICAL();
      Alarm->delta = increment <= GET_ALARM_COUNTER_MAXVALUE(Alarm) - GET_ALARM_COUNTER_VALUE(Alarm) ? increment +
                GET_ALARM_COUNTER_VALUE(Alarm): increment - (GET_ALARM_COUNTER_MAXVALUE(Alarm) - GET_ALARM_COUNTER_VALUE(Alarm))-1U;
      SET_ALARM_CYCLE(Alarm, cycle);
    
      OSInsertAlarm(Alarm);
      
      EXIT_CRITICAL();
    }  
    return E_OK;

} /* End of SetRelAlarm */
/**********************************************************************
* Function Prototype: StatusType SetAbsAlarm( AlarmType Alarm, TickType start, TickType cycle )
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): Alarm, start, cycle
*    (out): 
* Return Value: E_OK, E_OS_STATE, E_OS_NOFUNC, E_OS_ID, E_OS_VALUE
* Caller:
* Call Function: OSInsertAlarm
* Description: Initial the Alarm specified by AlarmId to be an absolute Alarm, and set tick and increment
* Author: Hangping Pan, Xiaonan Meng, Luchuan Xu
* Time: 2005.6
***************************************************************************
*/
StatusType SetAbsAlarm(AlarmType Alarm, TickType start, TickType cycle)
{
#if(OS_APPLICATION_MODULE == STD_ON)
    uint16 AppID;
#endif
    DEFINECRITICAL(); 
    
  #if( (OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
     AppID = GetApplicationID();
     if((Alarm->AlarmApp == COMMON_TYPE) && (NO_ACCESS == CheckObjectAccess(AppID,OBJECT_ALARM,Alarm->id))) 
     {
      OSRETERROR(E_OS_ACCESS, OSServiceId_SetAbsAlarm, Alarm);
     }
  #endif
  
      #if (OS_EXTENDED_STATUS == STD_ON)
        if ((Alarm >= (OSAlarms + OS_ALARM_NUM)) || (Alarm < OSAlarms) || (((AddrType)Alarm - (AddrType)OSAlarms) %
            sizeof(AlarmCB)) != 0U)
        {
             /* Alarm is out of range*/
            OSRETERROR(E_OS_ID, OSServiceId_SetAbsAlarm, Alarm);
        }
    #endif

    #if (OS_EXTENDED_STATUS == STD_ON)
        if ((cycle > GET_ALARM_COUNTER_MAXVALUE(Alarm)) || (cycle != 0U && cycle < GET_ALARM_COUNTER_MINCYCLE(Alarm)) ||
            (start > GET_ALARM_COUNTER_MAXVALUE(Alarm)))
        {
            OSRETERROR(E_OS_VALUE, OSServiceId_SetAbsAlarm, Alarm); /* start or cycle is out of range */
        }
    #endif
    /*test and set operation*/
    ENTER_CRITICAL();
    if (GET_ALARM_STATE(Alarm) == AlarmUsed)
    {
        EXIT_CRITICAL();
        OSRETERROR(E_OS_STATE, OSServiceId_SetAbsAlarm, Alarm); /*The alarm has been used by another task */
    }
    EXIT_CRITICAL();
    /* When the start value equals the current value of the Counter, then special treatment is needed, just trigger the alarm immediately*/
    if ((start == GET_ALARM_COUNTER_VALUE(Alarm)) && (cycle==0U))
    {
    
        OSNotifyAlarm(Alarm); /*Trigger the alarm*/
        /*one-time alarm*/
        return E_OK;
    }
    else if((start == GET_ALARM_COUNTER_VALUE(Alarm)) && (cycle!=0U))
    {
           ENTER_CRITICAL();
       SET_ALARM_CYCLE(Alarm, cycle);
       OSNotifyAlarm(Alarm);
       OSInsertAlarm(Alarm);
           EXIT_CRITICAL();
    }
   else
   {
    ENTER_CRITICAL();    
    SET_ALARM_DELTA(Alarm, start);
    SET_ALARM_CYCLE(Alarm, cycle);
    OSInsertAlarm(Alarm);    
    EXIT_CRITICAL();
   }

    return E_OK;

} /* End of SetAbsAlarm */

/**********************************************************************
* Function Prototype: StatusType CancelAlarm(AlarmType AlarmID)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): AlarmID
*    (out): 
* Return Value: E_OK, E_OS_NOFUNC, E_OS_ID
* Caller:
* Call Function: OSDeleteAlarm
* Description: Cancel the alarm specified by AlarmID
* Author: Hangping Pan, Xiaonan Meng, Luchuan Xu
* Time: 2005.6
***************************************************************************
*/
StatusType CancelAlarm(AlarmType Alarm)
{

  #if( (OS_APPLICATION_MODULE == STD_ON) && (ModulesInitTrue == ModulesInitStatus))
    uint16 AppID;
    AppID = GetApplicationID();
     if((Alarm->AlarmApp == COMMON_TYPE) && (NO_ACCESS == CheckObjectAccess(AppID,OBJECT_ALARM,Alarm->id))) 
     {
      OSRETERROR(E_OS_ACCESS, OSServiceId_CancelAlarm, Alarm);
     }
  #endif
  
    #if (OS_EXTENDED_STATUS == STD_ON)
        if ((Alarm >= (OSAlarms + OS_ALARM_NUM)) || (Alarm < OSAlarms) || (((AddrType)Alarm - (AddrType)OSAlarms) %
            sizeof(AlarmCB)) != 0U)
        {
            OSRETERROR(E_OS_ID, OSServiceId_CancelAlarm, Alarm); /* Alarm is out of range*/
        }
    #endif

    if (GET_ALARM_STATE(Alarm) == AlarmUnUsed)
    {
        OSRETERROR(E_OS_NOFUNC, OSServiceId_CancelAlarm, Alarm); /* The alarm is unused*/
    }

    OSDeleteAlarm(Alarm);

    return E_OK;
} /* End of CancelAlarm */

#endif /* #ifdef OS_ALARM_EN */

#endif /*#ifndef  OS_ALARM_C */
