/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Hook.c
* DESCRIPTION: OS protection module source file based on AUTOSAR OS standard 
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0          Lvhong.Zhang 4/NOV/09       Initial version.   
*
*********************************************************************************
*/
#ifndef OS_HOOK_C
    #define OS_HOOK_C
#include "Os.h"

extern  uint8 SuspendCount ; /*Suspend/ResumeAllInterupst nesting time*/
extern  uint8 OSSuspendCount ; /*Suspend/ResumeOSInterupst nesting time*/
extern  uint8 AllSuspendCount ; /*Disable/EnableAllInterrupt nesting time*/


#if (OS_ERROR_HOOK == STD_ON )
     boolean OSNestedErrorHook;
    #if( OS_ERROR_PARAM_ACCESS == STD_ON)
        ObjectIDType OS_ObjectId = 0;
    #endif
    
    #if(OS_ERROR_GET_SERVICEID == STD_ON)
        OsServiceIdType OS_ServiceId = 0;
    #endif    
#endif 

#if(OS_PROTECTION_HOOK == STD_ON)

/**********************************************************************
* Function Prototype�� StatusType  ProtectionHook(StatusType error)
* Service ID: Not a user service, so no ID
* Sync/Asyn:  Sync
* Re-Entrancy:  YES
* Parameters description
*    (in): error The error which caused the call to the protection hook
*    (out): None
* Return Value: The return value defines the action the OS shall take after the protection hook 
*                PRO_IGNORE 
*                PRO_TERMINATETASKISR 
*                PRO_TERMINATEAPPL 
*                PRO_TERMINATEAPPL_RESTART 
*                PRO_SHUTDOWN
* Caller: None
* Call Function��
* Description: The protection hook is always called if a serious error occurs.
* Author:Jiang.Li
* Time:2010.11.4
***************************************************************************
*/ 
StatusType  ProtectionHook(StatusType error)
{
        if(error == E_OS_STACKFAULT || error == E_OS_PROTECTION_TIME || error == E_OS_PROTECTION_ARRIVAL || error == E_OS_PROTECTION_LOCKED )
           return   PRO_TERMINATETASKISR;
} 


/**********************************************************************
* Function Prototype�� StatusType OSProtectionHook(uint8 ErrorID, TaskType ptcb)
* Service ID: Not a user service, so no ID
* Sync/Asyn:  Sync
* Re-Entrancy:  YES
* Parameters description
*    (in): error: The error which caused the call to the protection hook
*     (in): ptcb: the task in which the error occur.
*    (out): None
* Return Value: E_OK
* Caller: None
* Call Function��
* Description: The protection hook is always called if a serious error occurs.
* Author:Jiang.Li
* Time:2010.11.4
***************************************************************************
*/ 

 StatusType OSProtectionHook(StatusType Error,TaskType ptcb)
 {
    TaskType curTcb=NULL;
    uint8 *ReadyListPtr;
    uint8 highestPrio;
    ResourceType res; 
    uint8  provalue;
    
      #if (OS_PROTECTION_HOOK == STD_ON)
      
          DEFINECRITICAL();
        provalue= ProtectionHook(Error);   
        curTcb=OSTCBCur;
    
    switch(provalue)
    {    
      case PRO_TERMINATETASKISR: 
      {    
              ENTER_CRITICAL();     
              CLEAR_TASK_EVENT(ptcb);  //OS must clear all set event.
              
              if(WAITING==GET_TASK_STATUS(ptcb)) 
             {
                   SET_TASK_STATUS(ptcb,SUSPENDED);
                DEC_TASK_CUR_ACT(ptcb); /*decrement the activities time of task*/
             }else if(READY ==GET_TASK_STATUS(ptcb))
             {
                DEC_TASK_CUR_ACT(ptcb); /*decrement the activities time of task*/
                    REMOVE_READY_LIST_HEAD(GET_TASK_ORIGINAL_PRIO(ptcb));/*delete task from the head of the ready list*/
                SET_TASK_UNREADY(GET_TASK_ORIGINAL_PRIO(ptcb), ptcb, SUSPENDED);         
            }else if(RUNNING==GET_TASK_STATUS(ptcb)||RUN_READY==GET_TASK_STATUS(ptcb))
             {
                 /*release out resource of the task*/    
                   #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON)              
                         while((OSRCBRef)0!=ptcb->resources)
                         {
                            if(ptcb->resources->NextRes == NULL) 
                        {//when task occupied out resource, then release the head of task ready list whose Prio is equal to internal resource Prio.
                        //Because when wo apply out resource ,we does not handle the internal reource. So, we need to cancle the internal resourece pointer in task ready list.
                            REMOVE_READY_LIST_HEAD(GET_RES_OLD_RUN_PRIO(ptcb->resources));             
                            OS_TASK_TO_UNREADY(GET_RES_OLD_RUN_PRIO(ptcb->resources), ptcb);
                            RESET_ACTUAL_HOLD_INT_RES(ptcb);//when RUNNING==GET_TASK_STATUS(ptcb) 
                        }
                       res = ptcb->resources;
                            SET_RES_UNUSED(res);                 /*set resource free*/
                            RESET_OUT_PRIORITY(ptcb, res);    /*reset task Prio*/
                       DELETE_RES_LIST(ptcb, res);       /*delete resource from the resource list*/  
                      }
                  #else
                     if(RUNNING==GET_TASK_STATUS(ptcb)) 
                     {
                         RELEASE_INT_RES(ptcb);
                     }
                   #endif
                 
                 
                   OS_TASK_SWITCH(curTcb);                          
                   OSPostTaskHook();
                   DEC_TASK_CUR_ACT(ptcb); /*decrement the activities time of task*/                        
                   REMOVE_READY_LIST_HEAD(GET_TASK_ORIGINAL_PRIO(ptcb));/*delete task from the head of the ready list*/
               
                   if(GET_TASK_CUR_ACT(ptcb)==0)
                {/*if the activities time is equal to 0, then set task to suspend status, and update the ready list*/
                    SET_TASK_UNREADY(GET_TASK_ORIGINAL_PRIO(ptcb), ptcb, SUSPENDED);
                }
                else
                {/*if the activities time is not equal to 0, then set task to ready status, and update the ready list*/
                    SET_TASK_READY(GET_TASK_ORIGINAL_PRIO(ptcb),ptcb); 
                }
             
                   if(NOT_IN_ISR)
                   {                           
                        OSTCBOld = OSTCBCur;
                         highestPrio = OS_FIND_HIGHEST_TASK();  /*Find the highest task who is at ready status.*/
                         ReadyListPtr = ReadyListCBTbl[highestPrio].head;        /*Get the task TCB*/
                         OSTCBCur = curTcb = &OSTCBTbl[*(ReadyListPtr+ReadyListCBTbl[highestPrio].start)];     /*update OSTCBCur*/
                       
                         GET_INT_RES(curTcb);
                         OSPreTaskHook();
                         
                       TaskTerminateSwitch(OSTCBOld, OSTCBCur);

                    EXIT_CRITICAL();
                    } 
             }
               
            #if (OS_TIMING_PROTECTION_MODULE ==  STD_ON)
                #if (OS_APPLICATION_MODULE == STD_ON)
                    AppID = GetApplicationID();
                    if(FALSE == ApplicationCB[AppID].OsTrusted ||TRUE == ApplicationCB[AppID].TimingProtection) 
                    {
                        ptcb->RemainingExecutionBudget = ptcb->OsTaskExecutionBudget;//reset RemainingExecutionBudget
                    }
                #endif
            #endif
        
               if(ptcb->InfoBit2 & 0x01U != 0U)
               {
            #if (OS_INTERRUPT_MODULE == STD_ON)
                  ResumeOSInterrupts();
            #endif
               }
               if(ptcb->InfoBit2 & 0x02U != 0U)
               {
            #if (OS_INTERRUPT_MODULE == STD_ON)
                  ResumeAllInterrupts();
            #endif
               }
               if(ptcb->InfoBit2 & 0x04U != 0U)
               {
            #if (OS_INTERRUPT_MODULE == STD_ON)
                  DisableAllInterrupts();
            #endif
               }
            EXIT_CRITICAL();    
        }
        break;
      
      case PRO_TERMINATEAPPL: 
      {
      //TODO
      
      }
      break;
      
      case PRO_TERMINATEAPPL_RESTART: 
      {
      
      //TODO
      }
      break;
      
      case PRO_SHUTDOWN: 
      {
      
         ShutdownOS(Error);
      }
      break;
      
     case PRO_IGNORE:
        {
        
         //do nothing;    
        }
      break;
    }
   
      #else
          ShutdownOS(Error) ;
      #endif
      
      return E_OK; 
 }


#endif

#endif
