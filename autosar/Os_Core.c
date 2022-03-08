/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering Center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Core.c
* DESCRIPTION: source file containing the implements related to OS error mechanism
*             and core functions
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0         Wei.Chen      4/DEC/03       Initial version. 
* 1.1     Hangping.Pan    20/JAN/05 
*
*********************************************************************************
*/
#ifndef OS_CORE_C 
#define OS_CORE_C

#include "Os.h"
#include "Os_Res.h"
#include "Platform_Lib.h"
#include "Std_Types.h"

#if(ECUM_MODULE == STD_ON)
    //#include "EcuM.h"
#endif

    #if (DEC_MODULE == STD_ON)
    extern void initDEC(void);
    #endif
  
 
    #if (OS_INTERRUPT_MODULE == STD_ON) || (OS_ALARM_MODULE == STD_ON)
        uint8 IntNesting = 0U; //the changing instruction lwz to lbz
    #endif /*#if defined(OS_EXCEPTION_EN)|| defined(OS_ALARM_EN)*/
  
  /*Current activated APPMODE.*/
  AppModeType ActiveAppMode = OSDEFAULTAPPMODE; 

  /*States machine of current module.*/
  uint8  ModulesInitStatus = 0U; 
  uint8  ModulesInitTrue = 1U;


  OSSTK OSTaskIdleStk[OS_TASK_IDLE_STK_SIZE];  
  OSSTK SystemSTK[OS_STK_SYSTEM_SIZE];    
  OSSTKRef SystemSTKTopPtr = &SystemSTK[OS_STK_SYSTEM_SIZE-2U];
  
  
  /*Starts task of highest priority.*/  
  #define OSStartFirstTask(curTcb) OSFirstTaskRun(curTcb)
  
  /*OSMapTbl*/
  uint8 const OSMapTbl[] =
  {
      0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U
  };

  /*Finds the most significant bit in a number between 0 to 255;ex, the most significant bit
       of 17(0b10001) is 5. It cans be get fast by this table.*/  
    uint8 const OSUnMapTbl[] =
  {
      0U, 0U, 1U, 1U, 2U, 2U, 2U, 2U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U,
          5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U,
          6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U,
          6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
          7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
          7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
          7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U
  };

/**********************************************************************
* Function Prototype��void  OSTaskIdle (void)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*  (in): None
*  (out): None
* Return Value: None
* Caller: OSInit
* Call Function: 
* Description:      
* Author: Jiang Li
* Time:     17/JAN/2005  
**********************************************************************/
  void OSTaskIdle(void)
  {
      for (;;){
      
      }
  }


/**********************************************************************
* Function Prototype��void  StartOS (AppModeType mode)
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*  (in): mode - AppModeType to start
*  (out): None
* Return Value: None
* Caller: main()
* Call Function: OSStartupHook()
* Description:  This function is to start OS.      
* Author: Jiang Li
* Time:     23/JAN/2005  
**********************************************************************/
  void StartOS(AppModeType mode)
  {
      TaskType curTcb = NULL;
      uint8 *ReadyListPtr;
      uint8 highestPrio;
          
      //DEFINECRITICAL(); //do not need, it must disable interrupt before the first task run.
      {
        //HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_RESET);
        //HAL_Delay(1000);
        //HAL_GPIO_WritePin(GPIOG, GPIO_PIN_2, GPIO_PIN_SET);
     }
      if (OS_NUM_SYS_TASK > OS_MAX_TASKS)
       /*Check whether exceeds maximum tasks, which is system supported */
      {
          return ;
      }
      
      /*Only needs to call entry critical, out of critical area is automatic. */
      //ENTER_CRITICAL();
      
      ActiveAppMode = mode;
      
       #if (OS_APPLICATION_MODULE == STD_ON)
         OSApplicationInit();
      #endif
      
      OSTaskInit();
 

      #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON)
          ResourceInit();
      #endif /*#if defined(OS_OUT_RES)||defined(OS_RES_SCHEDULER)*/
      #if (OS_ALARM_MODULE == STD_ON)
          OSAlarmInit();
      #endif /*#ifdef OS_ALARM_EN*/
      #if (OS_ISR_NUM > 0)
          OSISRInit();
      #endif /*#ifdef OS_EXCEPTION_EN*/
         
      #if (OS_SCHEDULE_TABLE_MODULE == STD_ON)
         OSScheduleTableInit();
      #endif
          
      
      OSStartupHook();
  
      /*Searchs map table to find the idle task of highest priority */
      highestPrio = OS_FIND_HIGHEST_TASK();                                  
    
      /*Points to head of ReadyList*/        
      ReadyListPtr=ReadyListCBTbl[highestPrio].head    ; 
      /*Updates OSTCBCur.*/
      OSTCBCur=curTcb=&OSTCBTbl[*(ReadyListPtr+ReadyListCBTbl[highestPrio].start)];
      /*gets internal resource.*/
      GET_INT_RES(curTcb); 
      
      OSPreTaskHook();
      SET_TASK_STATUS(curTcb, RUN_READY);
 
      ModulesInitStatus = ModulesInitTrue;

      OSStartFirstTask(curTcb);  
      while(1)
      {

      }
          
  }

/**********************************************************************
* Function Prototype AppModeType GetActiveApplicationMode ( void )
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*  (in): None
*  (out): None
* Return Value: Return activated AppModeType
* Caller: Anywhere, excepts ISR of Category One and CALLBACK of Alarm.  
* Call Function: None
* Description:  This function is to get the current APPMODE.
* Author: Jiang Li
* Time:     17/JAN/2005  
**********************************************************************/
  AppModeType GetActiveApplicationMode(void)
  {
      return ActiveAppMode;
  }
   
/**********************************************************************
* Function Prototype��void ShutdownOS ( StatusType Error )
* Service ID: 
* Sync/Asyn: 
* Re-Entrancy: 
* Parameters description
*  (in): Error - Error Status
*  (out): None
* Return Value: None
* Caller: TASK, ISR of Category Two, ErrorHook, StartupHook
* Call Function: None
* Description:  This function is to shutdown OS.
* Author: Xiaolong Shu
* Time:     15/MAY/2006  
**********************************************************************/
  void ShutdownOS(StatusType Error)
  {
      DEFINECRITICAL();
      
      ModulesInitStatus = 0U;
     
      ENTER_CRITICAL();
      
      OSShutdownHook(Error);      
      while (1);
      
      EXIT_CRITICAL();


  }




#endif /*#ifndef OS_CORE_C*/
