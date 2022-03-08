/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os.h
* DESCRIPTION: header file containing all module header files
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0           Hangping.Pan   14/SEP/04       Initial version.  
* 1.1          Jiang.Li       20/APR/05
* 1.2         Lvhong.zhang   31/APR/09
*********************************************************************************
*/
#ifndef _OS_H
    #define _OS_H
    
    #include "Os_Cfg.h"
#include "Os_Core.h"
#include "Os_Task.h"
#include "Os_Task_Cpu.h"
#include "Platform_Lib.h"
//    #include "app_cfg.h"
    int CounterRdy;
    int pressed;
    int pressed2;
    int cleared;
    #define NULL_PTR ((void *)0)
    //end adding
    
    #if (OS_ALARM_MODULE == STD_ON)
        #include "Os_Alarm.h"
    #endif
     
    #include "Os_Res.h"  /*Because of the internal resource and out resource, it does not need macro switch*/

    #if (OS_EVENT_MODULE == STD_ON)
        #include "Os_Evt.h"
    #endif

  
    #if (OS_INTERRUPT_MODULE == STD_ON)
        //#include "os_irq.h"
    #endif
       
  

    #if( OS_SCHEDULE_TABLE_MODULE == STD_ON)
       #include  "Os_SchTbl.h"
    #endif
    
    #if (OS_STACK_MONITORING_MODULE == STD_ON )
         #include  "Os_Stack_Monitoring.h"
    #endif     
        
    //#if (OS_HOOK_MODULE == STD_ON) // when OS_HOOK_MODULE is disable,     api can not return errorID which is defined in Os_Hook.h
            #include  "Os_Hook.h"
    //#endif

    #if (OS_APPLICATION_MODULE == STD_ON)
        #include "Os_App.h"
    #endif    
        

#endif
