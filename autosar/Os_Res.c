
/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Res.c
* DESCRIPTION: Resource module source file based on AUTOSAR OS standard 
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0          Wei.Chen     4/DEC/03       Initial version.   
* 1.1          Jiang.Li    20/JAN/04   
*
*********************************************************************************
*/
#ifndef OS_RES_C 
    #define OS_RES_C

  #include "Os.h"

    #if (OS_RESOURCE_MODULE == STD_ON)

        #if (OS_RES_EXTERNAL == STD_ON||OS_RES_INTERNAL == STD_ON) /*if user use resource,no matter external res or internal res*/

            extern OsResource OSResourceCfgTbl[]; 
            
            #if (OS_RES_INTERNAL == STD_ON)
                    OSRCBInternal OSResourcesInternal[OS_RES_INTERNAL_NUM]; /*Array of internal resource control structure*/
            #endif 
         
            #if (OS_RES_SCHEDULER == STD_OFF)/*Did not use scheduler resource*/
                #if (OS_RES_EXTERNAL == STD_ON)
                    OSRCB OSResources[OS_RES_EXTERNAL_NUM]; /*Array of external resource control structure*/
                #endif
                
                #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)     
                   AlarmCB OSResourceAlarm[OS_RES_EXTERNAL_NUM]; /*The external resource will use alarm to control the time so that task cannot lock shared res too long*/
                   extern void OSInsertAlarm(AlarmCBRef alarm); 
                   extern void OSDeleteAlarm(AlarmCBRef alarm);  
                #endif 
            #else /*ifndef OS_SCHEDULER*/

                #if (OS_RES_EXTERNAL == STD_ON)
                   OSRCB   OSResources[OS_RES_EXTERNAL_NUM + 1U]; /*Array of external resource control structure*/
                   ResourceType RES_SCHEDULER = &OSResources[OS_RES_EXTERNAL_NUM];
                #else                    
                    OSRCB OSResources[1];
                    ResourceType RES_SCHEDULER = OSResources;                          
                #endif
               
                #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)     
                   AlarmCB OSResourceAlarm[OS_RES_EXTERNAL_NUM+1U];  /*The external resource will use alarm to control the time so that task cannot lock shared res too long*/
                   extern void OSInsertAlarm(AlarmCBRef alarm); 
                   extern void OSDeleteAlarm(AlarmCBRef alarm);  
                #endif 

            #endif /*#ifndef OS_RES_SCHEDULER*/
         
    
        #elif (OS_RES_SCHEDULER == STD_ON)/*only scheduler resource*/
            OSRCB OSResources[1];
            ResourceType RES_SCHEDULER = OSResources;
            #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)                           
                     AlarmCB OSResourceAlarm[1U];
                     extern void OSInsertAlarm(AlarmCBRef alarm); 
                     extern void OSDeleteAlarm(AlarmCBRef alarm);  
            #endif 
        #endif 


        #if ((OS_RES_EXTERNAL == STD_ON)||(OS_RES_INTERNAL == STD_ON))
            // ResPrioType OSResPrioTbl[OS_RES_NUM]; /*OSResPrioTbl use for record the priority of the resource*/
            #if (OS_RES_INTERRUPT == STD_ON)
                OSRCBRef OSISRLastResource = NO_RESOURCE; /*use for record the list head of the res list*/
                #define ISR_LOCK_SCHEDULER  0x10U /*use for lock scheduler resource*/
            #endif /*ifdef OS_ISR_RES*/
        #endif /*#ifdef OS_OUT_RES */
        


/**********************************************************************
* Function Prototype��void GET_INT_RES(TaskType curTcb)
* Service ID: 
* Sync/Asyn:  Sync
* Re-Entrancy: YES
* Parameters description
*    (in): ptcb   the task control structure     
*    (out): NO
* Return Value: 
* Caller: The function  which activate a task 
* Call Function��
* Description: The task whcih configure a internal resource gain the internal resource
* Author:
* Time:
***************************************************************************
*/  

#if (OS_RES_INTERNAL == STD_ON)
void GET_INT_RES(TaskType ptcb)
{/*timing protection:resource block budget*/
   #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
     
     uint32 increment;
     AlarmCBRef ResourceAlarm;
     OsTaskResourceLockRef TaskResLockRef;
   #endif
    uint8 *ReadyListPtr;
    uint16 start,pos,j,k,taskid,AppID; 
   
   if((HOLD_INT_RES==GET_TASK_HOLD_INT_RES(ptcb))&&(!GET_ACTUAL_HOLD_INT_RES(ptcb))&&(GET_TASK_ORIGINAL_PRIO(ptcb) <= GET_TASK_RUN_PRIO(ptcb)))
   {/*set the priority of internal resources*/
      
      if((GET_TASK_ORIGINAL_PRIO(ptcb) != GET_TASK_RUN_PRIO(ptcb)) //if the priority of the curTcb is equal to the priority of the internal reousre, it does nothing.
         #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON) /*External resource or scheduler resource*/
           && (ptcb->resources ==NULL)////if curTcb has occupied out resource
         #endif
        )
      {
         OS_TASK_TO_READY(GET_TASK_RUN_PRIO(ptcb), ptcb); 
         INSERT_READY_LIST_HEAD(GET_TASK_ID(ptcb), GET_TASK_RUN_PRIO(ptcb));                
      }
      SET_ACTUAL_HOLD_INT_RES(ptcb);
   }  
   
   if(GET_ACTUAL_HOLD_INT_RES(ptcb)) 
   {   
       
 #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
           /*The following set the corresponding alarm*/
          for(j=0U;j<OS_RES_NUM;j++)
         {
             if(ptcb->OsTaskResourceRef & (0x00000001U<< j))
             {
                if(OSResourceCfgTbl[j].ResProperty == INTERNAL) 
                {
                 #if (OS_APPLICATION_MODULE == STD_ON)
                    for(k=0U;k<OS_RES_INTERNAL_NUM;k++)
                    {    
                        if(OSResourcesInternal[k].ResId == j) 
                        {
                            break;
                        }                            
                    }
                 #endif
                  break; 
                }
             }
         }
         
             
       #if (OS_APPLICATION_MODULE == STD_ON)
          taskid = ptcb->OSTCBId;
          AppID = CheckObjectOwnership(OBJECT_TASK,taskid);

          if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_RESOURCE,k))
              return ;
          
          if((FALSE == ApplicationCB[AppID].OsTrusted) ||(TRUE == ApplicationCB[AppID].TimingProtection))
           {
             ResourceAlarm =  &OSResourceAlarm[j];
                ResourceAlarm->start = AlarmUsed;
                ResourceAlarm->ctr = &OSCounters;
                ResourceAlarm->task =  ptcb;
                
                TaskResLockRef = ptcb->OsTaskResourceLock;
                while(TaskResLockRef->OsTaskResourceRef != j) /*Here requires the user to configure the lock time of the resources , or in a loop*/
                {                   
                   TaskResLockRef++;
                }
                
               increment = (TaskResLockRef->OSTaskResourceBudget) * OS_TICKS_PER_SEC;
               increment = increment / (ResourceAlarm->ctr->CtrInfo->OsCounterTicksPerBase);
                ResourceAlarm->delta = increment <= GET_ALARM_COUNTER_MAXVALUE(ResourceAlarm) - GET_ALARM_COUNTER_VALUE(ResourceAlarm) ? increment +
                 GET_ALARM_COUNTER_VALUE(ResourceAlarm): increment - (GET_ALARM_COUNTER_MAXVALUE(ResourceAlarm) - GET_ALARM_COUNTER_VALUE(ResourceAlarm))-1U;
             ResourceAlarm->AlarmApp = RESOURCE_TYPE;
                
             OSInsertAlarm(ResourceAlarm);
          }
         #endif
  #endif 
  }
    
}      

  
  /**********************************************************************
* Function Prototype��void RELEASE_INT_RES(TaskType curTcb)
* Service ID: 
* Sync/Asyn:  Sync
* Re-Entrancy: YES
* Parameters description
*    (in): curTcb   the task control structure     
*    (out): 
* Return Value: 
* Caller: The function  which  task lost scheduler
* Call Function��
* Description: The task whcih configure a internal resource release the internal resource when lost scheduler
* Author:
* Time:
**************************************************************************************/

void RELEASE_INT_RES(TaskType curTcb)
{ /*timing protection:resource block budget*/
   
   #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
     AlarmCBRef ResourceAlarm;          
   #endif
   
     uint8 *ReadyListPtr;
         uint16 pos,k,j,taskid,AppID;

    if(GET_ACTUAL_HOLD_INT_RES(curTcb)) 
    {        
        if((GET_TASK_ORIGINAL_PRIO(curTcb) != GET_TASK_RUN_PRIO(curTcb)) //if the priority of the curTcb is equal to the priority of the internal reousre, it does nothing.
           #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON)  /*External resource or scheduler resource*/
              && (curTcb->resources ==NULL)//if curTcb has occupied out resource, it does not remove the tcb pointer

           #endif
        )
       {               
            REMOVE_READY_LIST_HEAD(GET_TASK_RUN_PRIO(curTcb));  
            pos=GET_TASK_RUN_PRIO(curTcb);              
            OS_TASK_TO_UNREADY(pos, curTcb);
       }
             RESET_ACTUAL_HOLD_INT_RES(curTcb);
     
      #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)   
              /*The following delete the alarm*/
             for(j=0U;j<OS_RES_NUM;j++)
             {
                 if(curTcb->OsTaskResourceRef & (0x00000001U<< j))
                 {
                    if(OSResourceCfgTbl[j].ResProperty == INTERNAL)
                     #if (OS_APPLICATION_MODULE == STD_ON)  
                         for(k=0U;k<OS_RES_INTERNAL_NUM;k++)
                        {   
                            if(OSResourcesInternal[k].ResId == j) 
                            {
                                break;                                 
                            }                            
                        } 
                      #endif                       
                      break;
                 }
             }

             ResourceAlarm =  &OSResourceAlarm[j];
             #if (OS_APPLICATION_MODULE == STD_ON)
              taskid = curTcb->OSTCBId;
              AppID = CheckObjectOwnership(OBJECT_TASK,taskid);

              if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_RESOURCE,k))
                  return ;
              
              if((FALSE == ApplicationCB[AppID].OsTrusted) ||(TRUE == ApplicationCB[AppID].TimingProtection))
               {                
                    if(ResourceAlarm->start == AlarmUsed)
                    {/*Delete the alarm*/
                        OSDeleteAlarm(ResourceAlarm);
                    } 
              }
              #endif                 
     #endif       
    
  }
}
    
#endif/*OS_RES_INTERNAL == STD_ON*/   


   
/**********************************************************************
* Function Prototype��void ResourceInit(void)
* Service ID: 
* Sync/Asyn:  Sync
* Re-Entrancy: YES
* Parameters description
*    (in):     
*    (out): 
* Return Value: 
* Caller: StartOs
* Call Function��
* Description: Initialize resources
* Author:Jiang Li
* Time:2005.1.13
**************************************************************************************/

void ResourceInit(void)
{
    #if (OS_RESOURCE_MODULE == STD_ON) && ((OS_RES_EXTERNAL == STD_ON) || (OS_RES_INTERNAL == STD_ON))

        uint8 i = 0U,j=0U,k=0U;             
        #if ( OS_RES_EXTERNAL == STD_ON || OS_RES_SCHEDULER == STD_ON || OS_RES_INTERNAL == STD_ON)
        for (i = 0U; i < OS_RES_NUM; i++)
        {
             #if ( OS_RES_EXTERNAL == STD_ON || OS_RES_SCHEDULER == STD_ON)
             /*Initialize out resource control block*/
                 if(INTERNAL!=OSResourceCfgTbl[i].ResProperty)        
                 {
                        //OSResources[j].priority = OSResPrioTbl[i];
                        OSResources[j].priority = OSResourceCfgTbl[i].ResPrio;
                        OSResources[j].SavedTCB = (TaskType)0;
                        OSResources[j].NextRes  = (OSRCBRef)0;
                        OSResources[j].ResId = j;
                        #if (OS_APPLICATION_MODULE == STD_ON)
                             OSResources[j].OsResourceAccessingApplication = OSResourceCfgTbl[i].OsResourceAccessingApplication;
                             OSResources[j].OsResOwnershipApp = OSCheckObjectOwnership(OBJECT_RESOURCE,i);
                        #endif
            
                        SET_RES_UNUSED(&OSResources[j]);
                         
                        #if (OS_RES_INTERRUPT == STD_ON) 
                            OSResources[i].SavedMask = 0x00U; 
                        #endif /*ifdef OS_ISR_RES*/ 
                        j++;
                 }
            #endif
                     
            #if ( OS_RES_INTERNAL == STD_ON)                    
                 if(INTERNAL==OSResourceCfgTbl[i].ResProperty)    
                  { 
                       OSResourcesInternal[k].priority = OSResourceCfgTbl[i].ResPrio;
                       OSResourcesInternal[k].ResId = i;
                        
                       #if (OS_APPLICATION_MODULE == STD_ON)
                           OSResourcesInternal[k].OsResourceAccessingApplication = OSResourceCfgTbl[i].OsResourceAccessingApplication;
                              OSResourcesInternal[k].OsResOwnershipApp = OSCheckObjectOwnership(OBJECT_RESOURCE,i);
                        #endif            
                         k++;
            
                  }
            #endif
                      
        }
        #endif

    #endif /*#ifdef     OS_RESOURCE_MODULE */
    

    
    #if (OS_RES_SCHEDULER == STD_ON)       
        RES_SCHEDULER->priority=OS_MAX_TASK_PRIO; 
        RES_SCHEDULER->SavedTCB = (TaskType)0;
        RES_SCHEDULER->NextRes = (OSRCBRef)0;
        SET_RES_UNUSED(RES_SCHEDULER);
        #if (OS_RES_INTERRUPT == STD_ON) 
            RES_SCHEDULER->SavedMask = 0x00U; 
        #endif /*ifdef OS_ISR_RES*/
        
        #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
            #if ((OS_RES_EXTERNAL == STD_ON)||(OS_RES_INTERNAL == STD_ON)) 
               RES_SCHEDULER->ResId = OS_RES_NUM; 
            #else
               RES_SCHEDULER->ResId = 0U; 
            #endif
        #endif
    #endif /*ifdef OS_RES_SCHEDULER*/
}

#if (OS_RES_EXTERNAL == STD_ON)


/**********************************************************************
* Function Prototype��StatusType GetOutResource( ResourceType res )
* Service ID: 
* Sync/Asyn:  Sync
* Re-Entrancy: YES
* Parameters description
*    (in): res   the resource which task will get  
*    (out): 
* Return Value: E_OK:
*                E_OS_ID�rres is not valid
*               E_OS_ACCESS�rres had been used or the priority of the task is higher than the res priority
* Caller: 
* Call Function��
* Description: This function only applies to external resources
* Author:Jiang Li
* Time:2005.5.23
**************************************************************************************/
    StatusType GetOutResource(ResourceType res)
    {
                  uint8 *ReadyListPtr;
                  uint16 start,pos,resid,AppID;
              
    #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
                  uint32 increment;
          AlarmCBRef ResourceAlarm;
          OsTaskResourceLockRef TaskResLockRef;
    #endif
       
        DEFINECRITICAL();

        #if (OS_EXTENDED_STATUS == STD_ON)
            if (((OSResources + OS_RES_EXTERNAL_NUM) <= res) || (res < OSResources) || (((AddrType)res - (AddrType)OSResources)
                % sizeof(OSRCB) != 0U))
            {
                 /*res is valid*/
                OSRETERROR(E_OS_ID, OSServiceId_GetResource, res);
            }
            
            if(((0x00000001U<<res->ResId) & OSTCBCur->OsTaskResourceRef)==0U)
            {
                 /*task had no access to res*/
                OSRETERROR(E_OS_ID, OSServiceId_GetResource, res);
            }
            
            if (OUT_RES_USED == GET_RES_USED(res))
            {
                 /*res had been used*/
                OSRETERROR(E_OS_ACCESS, OSServiceId_GetResource, res);
            }
            #if (OS_RES_INTERRUPT == STD_OFF)  
                
                if ((GET_TASK_ORIGINAL_PRIO(OSTCBCur) > GET_RES_PRIO(res)) || (!NOT_IN_ISR))
                {
                    OSRETERROR(E_OS_ACCESS, OSServiceId_GetResource, res);
                }
            #else 
                if ((GET_RES_PRIO(res) > 0U) 
                 && ((GET_TASK_ID(OSTCBCur) > GET_RES_PRIO(res)) || (!NOT_IN_ISR)))
                {
                    OSRETERROR(E_OS_ACCESS, OSServiceId_GetResource, res);
                }
                else if ((GET_RES_PRIO(res) <= 0U) 
                 && (!NOT_IN_ISR) && (GET_ISR_PRIO() >  - (GET_RES_PRIO(res))))
                {
                    OSRETERROR(E_OS_ACCESS, OSServiceId_GetResource, res);
                }
            #endif /*ifndef OS_ISR_RES*/

           /*Application*/ 
           #if (OS_APPLICATION_MODULE == STD_ON)
               AppID = GetApplicationID();
               resid = res-> ResId;
               if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_RESOURCE,resid) ) 
               {
                OSRETERROR(E_OS_ACCESS, OSServiceId_GetResource, ptcb);
               }
            #endif 
            
      #endif /*ifdef OS_EXTENDED_STATUS*/     
        
        ENTER_CRITICAL();

        SET_RES_USED(res);
        #if (OS_RES_INTERRUPT == STD_ON)
            if (GET_RES_PRIO(res) < 0U)
            {
                #if (OS_INTERRUPT_MODULE == STD_ON) 
                    DISABLE_LOW_PRIO_ISR( - (GET_RES_PRIO(res))); /*mask resources whose priorities were lower than res*/
                    SET_RES_ISR_MASK(res, ISR_MASK); /*Set mask of the res*/
                #endif /*ifdef OS_EXCEPTION_EN*/
                ADD_RES_ISR_LIST(res);
                IntNesting |= ISR_LOCK_SCHEDULER;/* set IntNesting*/
            }
            else
        #endif /*ifdef  OS_ISR_RES*/
        {
            SET_OUT_PRIORITY(res, OSTCBCur); /*Change the priority of the task*/
            ADD_RES_LIST(OSTCBCur, res);    
        }
       
        
        /*timing protection:resource block budget*/
   #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
      #if (OS_APPLICATION_MODULE == STD_ON) 
        if( (FALSE == ApplicationCB[AppID].OsTrusted) ||(TRUE == ApplicationCB[AppID].TimingProtection)) 
        {
             ResourceAlarm =  &OSResourceAlarm[res->ResId];
                ResourceAlarm->start = AlarmUsed;
                ResourceAlarm->ctr = &OSCounters;
                ResourceAlarm->task =  OSTCBCur;
                
                TaskResLockRef = OSTCBCur->OsTaskResourceLock;
                while(TaskResLockRef->OsTaskResourceRef !=  res->ResId)/*Here requires the user to configure the lock time of the resources , or in a loop*/
                {
                   
                   TaskResLockRef++;
                }
                
               increment = (TaskResLockRef->OSTaskResourceBudget) * OS_TICKS_PER_SEC;
               increment = increment / (ResourceAlarm->ctr->CtrInfo->OsCounterTicksPerBase);
                ResourceAlarm->delta = increment <= (GET_ALARM_COUNTER_MAXVALUE(ResourceAlarm) - GET_ALARM_COUNTER_VALUE(ResourceAlarm))? increment +
                 GET_ALARM_COUNTER_VALUE(ResourceAlarm): increment - (GET_ALARM_COUNTER_MAXVALUE(ResourceAlarm) - GET_ALARM_COUNTER_VALUE(ResourceAlarm))-1U;
             ResourceAlarm->AlarmApp = RESOURCE_TYPE;
                
             OSInsertAlarm(ResourceAlarm);
        }
        #endif/*OS_APPLICATION_MODULE == STD_ON*/
       #endif        
        /*timing protection:resource block budget*/
        
        EXIT_CRITICAL();
        return E_OK;
    }

     
/**********************************************************************
* Function Prototype��StatusType ReleaseOutResource( ResourceType res )
* Service ID: 
* Sync/Asyn:  Sync
* Re-Entrancy: YES
* Parameters description
*    (in): res   the resource which task will release  
*    (out): 
* Return Value: E_OK:
*                E_OS_ID:res is not valid
*               E_OS_ACCESS:the priority of the task is higher than the res priority
*               E_OS_NOFUNC:res had not been used or it must release other resources firstly 
* Caller: 
* Call Function��
* Description: This function only applies to external resources
* Author:Jiang Li/Liji Yu
* Time:2005.12.13
**************************************************************************************/
    StatusType ReleaseOutResource(ResourceType res)
    {

        
        TaskType curTcb = NULL, ptcb = NULL;
        uint8 *ReadyListPtr, highestPrio,resid,AppID;
        DEFINECRITICAL();
                
     #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
        AlarmCBRef ResourceAlarm;
     #endif

        #if (OS_EXTENDED_STATUS == STD_ON)
            if (((OSResources + OS_RES_EXTERNAL_NUM) <= res) || (res < OSResources) || (((AddrType)res - (AddrType)OSResources)
                % sizeof(OSRCB) != 0U))
            {
                OSRETERROR(E_OS_ID, OSServiceId_ReleaseResource, res);
            }
             
            if ((OUT_RES_NOT_USED == GET_RES_USED(res)))
            {
                OSRETERROR(E_OS_NOFUNC, OSServiceId_ReleaseResource, res);
            }

            if(((0x00000001U<<res->ResId)&OSTCBCur->OsTaskResourceRef)==0U)
            {
                OSRETERROR(E_OS_ID, OSServiceId_GetResource, res);
            }

            #if (OS_RES_INTERRUPT == STD_OFF) 
                if ((!NOT_IN_ISR) || (GET_RES_PRIO(res) < GET_TASK_ORIGINAL_PRIO(OSTCBCur)))
                {
                    OSRETERROR(E_OS_ACCESS, OSServiceId_ReleaseResource, res);
                }
                if (GET_TASK_OUT_RES(OSTCBCur) != res)
                {
                    OSRETERROR(E_OS_NOFUNC, OSServiceId_ReleaseResource, res);
                }
            #else 
                if (GET_RES_PRIO(res) > 0U)
                {
                    if ((!NOT_IN_ISR) || (GET_RES_PRIO(res) < GET_TASK_ORIGINAL_PRIO(OSTCBCur)))
                    {
                        OSRETERROR(E_OS_ACCESS, OSServiceId_ReleaseResource, res);
                    }
                    else if (GET_TASK_OUT_RES(OSTCBCur) != res)
                    {
                        OSRETERROR(E_OS_ACCESS, OSServiceId_ReleaseResource, res);
                    }

                }
                else
                {
                    if (OSISRLastResource != res)
                    {
                        OSRETERROR(E_OS_NOFUNC, OSServiceId_ReleaseResource, res);
                    }
                }
            #endif /*ifndef OS_ISR_RES*/
            
            /*Application*/ 
       #if (OS_APPLICATION_MODULE == STD_ON)
           AppID = GetApplicationID();
           resid = res-> ResId;
           if( NO_ACCESS == CheckObjectAccess(AppID,OBJECT_RESOURCE,resid) ) 
           {
            OSRETERROR(E_OS_ACCESS, OSServiceId_ReleaseResource, ptcb);
           }
        #endif  

        #endif /*ifdef OS_EXTENDED_STATUS*/
        
        ENTER_CRITICAL();
        curTcb = OSTCBCur;
        
        /****************************************************************/
     #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
        
         #if (OS_APPLICATION_MODULE == STD_ON) 
            if((FALSE == ApplicationCB[AppID].OsTrusted) ||(TRUE == ApplicationCB[AppID].TimingProtection)) 
            {
                 ResourceAlarm =  &OSResourceAlarm[res->ResId];
                    if(ResourceAlarm->start == AlarmUsed)
                    {/* delete alarm */
                        OSDeleteAlarm(ResourceAlarm);
                    } 
            }
            #endif
     #endif
        /****************************************************************/        
        SET_RES_UNUSED(res);                 
        
        #if (OS_RES_INTERRUPT == STD_ON)
            if (GET_RES_PRIO(res) <= 0U)
            {
                RUSUME_ISR(GET_RES_ISR_MASK(res));     /*Restore interrupt mask*/
                OSISRLastResource = GET_RES_NEXT(res); /*Get next resource*/
                IntNesting &= ~ISR_LOCK_SCHEDULER;     /*release scheduler resource*/
            }
            else
            {
            #endif /*ifdef  OS_ISR_RES*/            
            RESET_OUT_PRIORITY(curTcb, res); /*Reset priority*/            
            DELETE_RES_LIST(curTcb, res);
            #if (OS_RES_INTERRUPT == STD_ON)
            }
        #endif /*ifdef  OS_ISR_RES*/

        #if (OS_SCHEDULE_POLICY != OS_SCHEDULE_NON)
            if (NOT_IN_ISR
            #if (OS_SCHEDULE_POLICY == OS_SCHEDULE_MIX)
                 && FULL_SCHED == GET_TASK_SCHED(curTcb) 
            #endif /*#ifdef   OS_MIX_SCHEDULE*/
            )
            {     /*Can preempt and not in ISR and the scheduler resource is not locked*/
                 
                  highestPrio = OS_FIND_HIGHEST_TASK();         /*Find the highest priority*/
                  ReadyListPtr=ReadyListCBTbl[highestPrio].head    ; /*Find the head of the ready list*/
                  ptcb=&OSTCBTbl[*(ReadyListPtr+ReadyListCBTbl[highestPrio].start)];/*Update OSTCBCur*/
                  
                      if(ptcb!=curTcb)
                {
                     /*Preempted by high priority*/                  
                    OS_TASK_SWITCH(curTcb);
                    OSPostTaskHook(); 
                    OSTCBCur = ptcb;
                    GET_INT_RES(ptcb);/*Get internal resource*/
                    
                    OSPreTaskHook(); 
                    OSStackMonitoring(curTcb);

                    #if (OS_EVENT_MODULE == STD_ON) 
                        if (RUN_READY != GET_TASK_STATUS(ptcb))
                    #endif /*#ifdef OS_EVENT_EN*/
                    {
                         /*Task is  first time to run*/
                        SET_TASK_STATUS(ptcb, RUN_READY); /*Set the status of the task*/
                        OSTaskSaveRun(curTcb, ptcb);

                    }
                    #if (OS_EVENT_MODULE == STD_ON) 
                        else
                        {
                            OSTaskSaveLoadRun(curTcb, ptcb);
                        }
                    #endif /*#ifdef OS_EVENT_EN*/
                }
            }
        #endif /*OS_NULL_SCHEDULE */

        EXIT_CRITICAL();
        return (E_OK);
    }

     
/**********************************************************************
* Function Prototype�void  OSReleaseAppOutResource(ResourceType res,uint8 AppID)
* Service ID: 
* Sync/Asyn:  Sync
* Re-Entrancy: YES
* Parameters description
*    (in): res   the resource which task will release  
*          AppID Application ID
*    (out): 
* Return Value: 
* Caller: 
* Call Function��
* Description: This function only applies to external resources
* Author:
* Time:
**************************************************************************************/
#if (OS_APPLICATION_MODULE == STD_ON)
void  OSReleaseAppOutResource(ResourceType res,uint8 AppID)
{
      DEFINECRITICAL();
      TaskType curTcb = NULL,ptcb = NULL;
            uint8 *ReadyListPtr,highestPrio;
            
     #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
        AlarmCBRef ResourceAlarm;
     #endif
     
        ENTER_CRITICAL();
        curTcb = OSTCBCur;
        
       #if (OS_EXTENDED_STATUS == STD_ON)
          if (((OSResources + OS_RES_EXTERNAL_NUM) <= res) || (res < OSResources) || (((AddrType)res - (AddrType)OSResources)
              % sizeof(OSRCB) != 0U))
          {
               /*res was not valid*/
                return ;
          }
           
          if ((OUT_RES_NOT_USED == GET_RES_USED(res)))
          {
               /*res had been used*/
                return ;
          }
          if(((0x00000001U<<res->ResId)&OSTCBCur->OsTaskResourceRef)==0U)
          {
               /*Task had no access to the res*/
                return ;
          }
       #endif
          
       /****************************************************************/
     #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
        
            if(FALSE == ApplicationCB[AppID].OsTrusted ||TRUE == ApplicationCB[AppID].TimingProtection) 
            {
                 ResourceAlarm =  &OSResourceAlarm[res->ResId];
                    if(ResourceAlarm->start == AlarmUsed)
                    {/* delete alarm*/
                        OSDeleteAlarm(ResourceAlarm);
                    } 
            }
     #endif
        /****************************************************************/
        SET_RES_UNUSED(res);                 
        EXIT_CRITICAL();
}             
 #endif
#endif /*ifdef OS_OUT_RES*/

#if (OS_RES_SCHEDULER == STD_ON) 
/**********************************************************************
* Function Prototype��StatusType GetSchedulerResource( ResourceType res )
* Service ID: 
* Sync/Asyn:  Sync
* Re-Entrancy: YES
* Parameters description
*    (in): res   the resource which task will get  
*    (out): 
* Return Value: E_OK:
*                E_OS_ID�rres is not valid
*               E_OS_ACCESS�rres had been used or the priority of the task is higher than the res priority
* Caller: 
* Call Function��
* Description: This function only applies to scheduler resources
* Author:Jiang Li
* Time:2005.5.23
**************************************************************************************/
StatusType GetSchedulerResource(ResourceType res)
{
            uint8 *ReadyListPtr;
            uint16 start;
            uint16 pos;

#if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
            uint32 increment;
      AlarmCBRef ResourceAlarm;
      OsTaskResourceLockRef TaskResLockRef;
#endif

    TaskType curTcb = NULL;

    DEFINECRITICAL();
    #if (OS_EXTENDED_STATUS == STD_ON)
        if (RES_SCHEDULER != res)
         /*RES_SCHEDULERΪOS_RES_EXTERNAL_NUM*/
        {
             /*res is not RES_SCHEDULER��*/
            OSRETERROR(E_OS_ID, OSServiceId_GetResource, res);
        }
        if ((OUT_RES_USED == GET_RES_USED(res)) || (!NOT_IN_ISR))
        {
             /*res had been used or in ISR*/
            OSRETERROR(E_OS_ACCESS, OSServiceId_GetResource, res);
        }
    #endif /*ifdef OS_EXTENDED_STATUS*/
    ENTER_CRITICAL();
    curTcb = OSTCBCur;
    SET_OUT_PRIORITY(res, curTcb); /*Change the current task priority*/
    #if (OS_EXTENDED_STATUS == STD_ON)
        SET_RES_USED(res);
    #endif /*ifdef OS_EXTENDED_STATUS*/
    ADD_RES_LIST(curTcb, res); /*Put res in the first address of the  resource list*/   
    
    /*timing protection:resource block budget*/
#if (OS_TIMING_PROTECTION_MODULE ==  STD_ON)
     ResourceAlarm =  &OSResourceAlarm[res->ResId];
        ResourceAlarm->start = AlarmUsed;
        ResourceAlarm->ctr = &OSCounters;
        ResourceAlarm->task =  OSTCBCur;
        
        TaskResLockRef = OSTCBCur->OsTaskResourceLock;
        while(TaskResLockRef->OsTaskResourceRef !=  res->ResId)    /*Here requires the user to configure the lock time of the resources , or in a loop*/
        {
           
            TaskResLockRef++;
        }        
       increment = (TaskResLockRef->OSTaskResourceBudget) * OS_TICKS_PER_SEC;
       increment = increment / (ResourceAlarm->ctr->CtrInfo->OsCounterTicksPerBase);
        ResourceAlarm->delta = increment <= GET_ALARM_COUNTER_MAXVALUE(ResourceAlarm) - GET_ALARM_COUNTER_VALUE(ResourceAlarm) ? increment +
         GET_ALARM_COUNTER_VALUE(ResourceAlarm): increment - (GET_ALARM_COUNTER_MAXVALUE(ResourceAlarm) - GET_ALARM_COUNTER_VALUE(ResourceAlarm))-1U;
     ResourceAlarm->AlarmApp = RESOURCE_TYPE;
        
     OSInsertAlarm(ResourceAlarm);
#endif        
    /*timing protection:resource block budget*/
    EXIT_CRITICAL();
    return (E_OK);

}
     
 /**********************************************************************
* Function Prototype��StatusType ReleaseSchedulerResource( ResourceType res )
* Service ID: 
* Sync/Asyn:  Sync
* Re-Entrancy: YES
* Parameters description
*    (in): res   the scheduler resource which task will release  
*    (out): 
* Return Value: E_OK:
*                E_OS_ID:res is not valid
*               E_OS_ACCESS:the priority of the task is higher than the res priority
*               E_OS_NOFUNC:res had not been used or it must release other resources firstly 
* Caller: 
* Call Function��
* Description: This function only applies to scheduler resources
* Author:Jiang Li/Liji Yu
* Time:2005.12.13
**************************************************************************************/
StatusType ReleaseSchedulerResource(ResourceType res)
{
    TaskType ptcb = NULL;
    TaskType curTcb = NULL;
    
    uint8 *ReadyListPtr;
    uint8 highestPrio;
 
 #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
    AlarmCBRef ResourceAlarm;
 #endif 

    DEFINECRITICAL();

    #if (OS_EXTENDED_STATUS == STD_ON)
        if (RES_SCHEDULER != res)
        {
             /*res is not RES_SCHEDULER��*/
            OSRETERROR(E_OS_ID, OSServiceId_ReleaseResource, res);
        }
        if ((OUT_RES_NOT_USED == GET_RES_USED(res)) || (GET_TASK_OUT_RES(OSTCBCur) != res))
        {
             /*res had not been used or it was not the latest resource of the current task*/
             OSRETERROR(E_OS_NOFUNC, OSServiceId_ReleaseResource, res);
        }
        if ((!NOT_IN_ISR))
        {
            OSRETERROR(E_OS_ACCESS, OSServiceId_ReleaseResource, res);
        }
    #endif /*ifdef OS_EXTENDED_STATUS*/           
    
    ENTER_CRITICAL();
    curTcb = OSTCBCur;
    
    RESET_OUT_PRIORITY(curTcb, res); /*Reset task priority*/
  
    /****************************************************************/
 #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
     ResourceAlarm =  &OSResourceAlarm[res->ResId];
        if(ResourceAlarm->start == AlarmUsed)
        {/*delete alarm*/
            OSDeleteAlarm(ResourceAlarm);
        } 
 #endif
    /****************************************************************/ 
    SET_RES_UNUSED(res);
    DELETE_RES_LIST(curTcb, res);

    #if (OS_SCHEDULE_POLICY != OS_SCHEDULE_NON)
        if (NOT_IN_ISR
        #if (OS_SCHEDULE_POLICY == OS_SCHEDULE_MIX)
             && FULL_SCHED == GET_TASK_SCHED(curTcb) 
        #endif /*#ifdef   OS_MIX_SCHEDULE*/
        )
        {
              highestPrio = OS_FIND_HIGHEST_TASK();           /*Find the highest priority*/
              ReadyListPtr=ReadyListCBTbl[highestPrio].head    ; /*Find the head of the ready list*/
              ptcb=&OSTCBTbl[*(ReadyListPtr+ReadyListCBTbl[highestPrio].start)];/*Update OSTCBCur*/
                      
                    if(ptcb!=curTcb)
            {
                 /*Preempt by high priority*/
              //  RELEASE_INT_RES(curTcb); //OS223:Points of rescheduling 
                OS_TASK_SWITCH(curTcb);
                OSPostTaskHook(); 
                OSTCBCur = ptcb;
                GET_INT_RES(ptcb);/*Get Internal resource*/
                
                OSPreTaskHook(); 
                OSStackMonitoring(curTcb);

                #if (OS_EVENT_MODULE == STD_ON) 
                    if (RUN_READY != GET_TASK_STATUS(ptcb))
                #endif /*#ifdef OS_EVENT_EN*/
                {
                     /*Task is the first time to run*/
                    SET_TASK_STATUS(ptcb, RUN_READY); 

                    OSTaskSaveRun(curTcb, ptcb);
                }
                #if (OS_EVENT_MODULE == STD_ON) 
                    else
                    {
                        OSTaskSaveLoadRun(curTcb, ptcb);

                    }
                #endif /*#ifdef OS_EVENT_EN*/
            }
        }
    #endif /*OS_NULL_SCHEDULE */

    EXIT_CRITICAL();
    return (E_OK);
}

#endif /*ifdef  OS_RES_SCHEDULER*/

    #endif /* #ifdef OS_RESOURCE_EN */
#endif /*#ifndef OS_RES_C*/
