/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering Center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Cpu_C.c
* DESCRIPTION: Stack Init, DEC Init
* UPDATE HISTORY
* REV      AUTHOR                  DATE       DESCRIPTION OF CHANGE                         
* 1.0      HaoJie Chen, Li Zhou    07/MAR/11   Initial version.   
*
*********************************************************************************
*/
#include "Os.h"
#include "integration.h"
#include "Integration.h"
#include "Os.h"
#include "Os_Cpu_Types.h"
#include "Platform_Lib.h"
//#include "IntHandle.h"

#if (OS_MODULE  == STD_ON) 
 #if((OS_INTERRUPT_MODULE == STD_ON) || (OS_ALARM_MODULE == STD_ON))
     extern void OSIntExit(void);
 #endif
#endif

extern TaskType OSTCBOld;
extern OsTask OSTaskCfgTbl[];
extern  uint8 ReadyList0[];

 #if (OS_INTERRUPT_MODULE == STD_ON) || (OS_ALARM_MODULE == STD_ON)
       extern uint8 IntNesting ; 
    #endif /*#if defined(OS_EXCEPTION_EN)|| defined(OS_ALARM_EN)*/

static uint32 LR_ADDR = 0;/*save LR value*/
/**********************************************************************
* Function Prototype: OSSTK *OSTaskStkInit (void (*task)(void), OSSTK *ptos)
* Service ID: 
* Sync/Asyn: Asyn
* Re-Entrancy: YES
* Parameters description
*    (in): task: the task that needs to init stack; ptos: the stack pointer of the task
*    (out): 
* Return Value: 
* Caller: SET_TASK_READY
* Call Function: 
* Description: Init the stack of the Corresponding task
* Author: Jerry
* Time: 07/11/19
***************************************************************************
*/
OSSTK *OSTaskStkInit(void(*task)(void), OSSTK *ptos)
{
    OSSTK *stkp;
    stkp = ptos;
    *(stkp)=0x01000000;        /*xPSR*/
    *(--stkp)=task;            /*Entry Point*/
    *(--stkp)=task;    /*R14 (LR) (init value will cause fault if ever used) */
    *(--stkp)=0x12121212;    /*R12*/
    *(--stkp)=0x03030303;    /*R3*/
    *(--stkp)=0x02020202;    /*R2*/
    *(--stkp)=0x01010101;    /*R1*/
    *(--stkp)=0x00000000;    /*R0:argument*//* Remaining registers saved on process stack   */
    *(--stkp)=0x11111111;    /*R11*/
    *(--stkp)=0x10101010;    /*R10*/
    *(--stkp)=0x09090909;    /*R9*/
    *(--stkp)=0x08080808;    /*R8*/
    *(--stkp)=0x07070707;    /*R7*/
    *(--stkp)=0x06060606;    /*R6*/
    *(--stkp)=0x05050505;    /*R5*/
    *(--stkp)=0x04040404;    /*R4*/
    return (stkp);    
}

//////////////////////////////

/**********************************************************************
* Function Prototype: void  OSTaskInit()
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*    (in): None
*    (out): None
* Return Value: None
* Caller: StartOS
* Call Function: 
* Description: Initialize OsTask module
* Author: Jiang Li
* Time: 2005.7.5
***************************************************************************
*/
  void OSTaskInit()
  {
      TaskType Tcb; 
      uint8 index = 1U ,*ReadyListPtr;
      PriorityType tmp_OrigPrio;
      uint16 pos;
    #if (OS_APPLICATION_MODULE == STD_ON)
        uint16 AppID;
    #endif
           
     #if (OS_TASK_NUM > 0U)
        OsTask *TcbCfg = OSTaskCfgTbl;
     #endif /*OS_TASK_NUM>0*/

      INIT_RDY_TBL(); /*ready list initialization*/

      /*creat idle task*/
      Tcb = &OSTCBTbl[0];
      SET_TASK_PROPERTY(Tcb, FULL_SCHED | AUTO_START);
      SET_TASK_ENTRY(Tcb, OSTaskIdle);


      SET_TASK_STK(Tcb, &OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE - 2U], OSTaskIdleStk);
   
        SET_TASK_ID(Tcb, 0U);
      INIT_IDLE_READY_LIST(0U);
      SET_PRIO_MASK(ReadyListCBTbl,0U);        

    
      IDLE_READY(Tcb); 


#if (OS_TASK_NUM > 0U)
      /*initialization of ReadyListCB*/

      for(; index <= OS_MAX_TASK_PRIO; index++)
      {
        INIT_READY_LIST(index);
        SET_PRIO_MASK(&ReadyListCBTbl[index],index);
      }
         
      for (index = 1U; index < OS_NUM_SYS_TASK; index++)      
      {/*set the task information of all tasks*/
          tmp_OrigPrio = TcbCfg->OsTaskPriority;
        
          Tcb = OSTCBTbl + index;            
          SET_TASK_ID(Tcb, index);
          SET_TASK_PROPERTY(Tcb, 0U);
             SET_TASK_ORIGINAL_PRIO(Tcb,tmp_OrigPrio);
             #if (OS_RESOURCE_MODULE == STD_ON)
                #if (OS_RES_INTERNAL == STD_ON)
                      if(TcbCfg->RunPrio != INVALID_RESOURCE_PRIO)
                     {
                         Tcb->InfoBit |=HOLD_INT_RES;
                         SET_TASK_RUNNING_PRIO(Tcb,TcbCfg->RunPrio);
                     }else
                     {    
                         SET_TASK_RUNNING_PRIO(Tcb,tmp_OrigPrio);
                     }
             #else
                         SET_TASK_RUNNING_PRIO(Tcb,tmp_OrigPrio);
                  #endif
             #endif 
      
          SET_TASK_ENTRY(Tcb, TcbCfg->PTask);
          CLEAR_TASK_EVENT(Tcb);
            
          SET_TASK_STK(Tcb, TcbCfg->PStkHigh, TcbCfg->PStkLow);
          
          SET_TASK_MAX_ACT(Tcb,TcbCfg->OsTaskActivation);
          SET_TASK_CUR_ACT(Tcb,0U);

              
          
          
          if(FULL_SCHED==TcbCfg->TaskSchedule)
          {
            Tcb->InfoBit|=FULL_SCHED;         
          }
          
          if(TcbCfg->OsTaskEventRef!=0U)
          {
            Tcb->InfoBit|=EXTENDED;         
          }
          
          #if (OS_PROTECTION_HOOK == STD_ON)
                  Tcb->InfoBit2=0;
          #endif
          
          SET_TASK_EVENT_LIST(Tcb,TcbCfg->OsTaskEventRef);
          SET_TASK_RESOURCE_LIST(Tcb,TcbCfg->OsTaskResourceRef);
          
            #if (OS_APPLICATION_MODULE == STD_ON )
           Tcb->OsTaskAccessingApplication = TcbCfg->OsTaskAccessingApplication;
           AppID = OSCheckObjectOwnership(OBJECT_TASK,index);
              Tcb->OsTaskOwnershipApp = AppID;  
         #endif     /*timing protection*/
         
             #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
              #if (OS_APPLICATION_MODULE == STD_ON )

               if(FALSE == ApplicationCB[AppID].OsTrusted || TRUE == ApplicationCB[AppID].TimingProtection) 
                  {                   
                  Tcb->OsTaskExecutionBudget = (TcbCfg->TaskTimingProtection.OsTaskExecutionBudget) * OS_TICKS_PER_SEC ;  /*TODO,when trusted APP is enable, it should have timing protection module*/
                  Tcb->RemainingExecutionBudget = Tcb->OsTaskExecutionBudget; /*TODO*/
                  Tcb->OsTaskTimeFrame = (TcbCfg->TaskTimingProtection.OsTaskTimeFrame) * OS_TICKS_PER_SEC; /*TODO*/
                  Tcb->TimeFrameStart = 0U;/*TODO*/
                  Tcb->CrossNum = MaxCrossTime;                 
                  Tcb->OsTaskResourceLock =TcbCfg->TaskTimingProtection.OsTaskResourceLock;  /*Resource Lock*/
                  }
               #endif
         #endif 
         
          if (((TcbCfg->Autostart) ==TRUE)&&((TcbCfg->TaskAutostartMode.OsTaskAppModeRef & (ActiveAppMode))!=0U))
          {/*AUTOSTART TASK*/           
           /*increase the activation time */
                   INC_TASK_CUR_ACT(Tcb);
              SET_TASK_READY(tmp_OrigPrio, Tcb);
              INSERT_READY_LIST_REAR(index, tmp_OrigPrio);
          #if ( OS_TIMING_PROTECTION_MODULE ==  STD_ON)
              Tcb->InfoBit |= TASKHAVERUN;
          #endif
          }
          else
          {
              SET_TASK_STATUS(Tcb, SUSPENDED);
          }

          TcbCfg++;
      }
#endif /*OS_TASK_NUM>0*/

  }




