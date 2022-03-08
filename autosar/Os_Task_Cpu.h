/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering Center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Task_Cpu.h
* DESCRIPTION: Operation of The Task
* UPDATE HISTORY
* REV      AUTHOR      DATE        DESCRIPTION OF CHANGE                         
* 1.0      LiJie Yu    07/MAR/05   Initial version.   
*
*********************************************************************************
*/


#ifndef OS_TASK_CPU_H
    #define OS_TASK_CPU_H
#include "Os_Cfg.h"
    #if( OS_STACK_MONITORING_MODULE == STD_ON )
        #define STACK_PUSH_SIZE         27/*for MPC5634 stack frame*/
    #endif

    extern uint8 OSRdyGrp; /*ready list index*/
    extern uint8 OSRdyTbl[OS_RDY_TBL_SIZE]; /*Ready list Array*/

 
   extern ReadyListCB ReadyListCBTbl[];
   extern const ReadyListCfg ReadyListCfgTbl[];


        
    /*set masks of the priority: OSPrioBitX and OSPrioBitY*/
     #define SET_PRIO_MASK(pReadyListCB,prio) (pReadyListCB)->OSPrioBitY = OSMapTbl[(prio) >> 3]; (pReadyListCB)->OSPrioBitX = OSMapTbl[(prio) & 0x07]   
    /*Set Task to Ready: first, change the status; second, insert into readyList */   
    #define OS_TASK_TO_READY(prio,PTCB) \
    do{\
    if(ReadyListCBTbl[prio].count==0)\
    {\
      OSRdyGrp|= ReadyListCBTbl[prio].OSPrioBitY;     \
      OSRdyTbl[prio >>3] |= ReadyListCBTbl[prio].OSPrioBitX;  \
    }}while(0)
    /*Set Task To Unready first , delete the task from the readyList, second change the status of task*/
    #define OS_TASK_TO_UNREADY(prio,PTCB)   if ((ReadyListCBTbl[prio].count==0)&&(( OSRdyTbl[(prio) >>3]&=~ReadyListCBTbl[prio].OSPrioBitX) == 0x00))  OSRdyGrp &= ~ReadyListCBTbl[prio].OSPrioBitY
     
  
  
    #define OS_FIND_HIGHEST_TASK() (uint8)((OSUnMapTbl[OSRdyGrp] << 3)+OSUnMapTbl[OSRdyTbl[OSUnMapTbl[OSRdyGrp]]])



    #if( OS_STACK_INIT == STD_ON)
        OSSTKRef OSTaskStkInit(OSTaskEntry task, OSSTKRef ptos); /*Stack Initialation*//*not set OSTCBStkCur, because OSTaskStkInit will change the value of the OSTCBStkCur*/
        #define SET_TASK_STK_INIT(PTCB)     (PTCB)->OSTCBStkCur = OSTaskStkInit((PTCB)->PTask,(PTCB)->OSTCBStkTop); OSStackMonitoring(PTCB)
        #define SET_TASK_STK(PTCB,SP,SB)  (PTCB)->OSTCBStkTop=SP; SET_TASK_STK_BOTTOM(PTCB, SB)
    #else
        #define SET_TASK_STK_INIT(PTCB)     (PTCB)->OSTCBStkCur = (PTCB)->OSTCBStkTop; OSStackMonitoring(PTCB)/*initialize the OSTCBStkCur but not initialize the stack*/
        #define SET_TASK_STK(PTCB,SP,SB)  (PTCB)->OSTCBStkTop=SP; (PTCB)->OSTCBStkCur=SP; SET_TASK_STK_BOTTOM(PTCB, SB)
    #endif /*#if( OS_STACK_INIT == STD_ON)*/


    /*Set the Information of The Task*/
    #if (OS_STACK_MONITORING_MODULE == STD_ON) 
        #define SET_TASK_STK_BOTTOM(PTCB, SB) ((PTCB)->OSTCBStkBottom = (SB))
    #else
          #define SET_TASK_STK_BOTTOM(PTCB, SB)
    #endif         





/*Get Stack Top*/
        #define GET_TASK_STK_TOP(PTCB)     (PTCB)->OSTCBStkTop
/*Save Old Task*/
        #define SET_TASK_OLD(PTCB)   OSTCBOld=(PTCB)


    /*Record the old Task*/
    #define OS_TASK_SWITCH(OldTcb) SET_TASK_OLD(OldTcb)

 extern TaskType OSTCBOld; 

    #if( OS_STACK_INIT == STD_ON) //if need OS_TASK_STK, then every task should load context.         
        #define OSTaskRun(PRunTcb)  SET_TASK_STK_INIT(PRunTcb); TaskRun(PRunTcb)
        #define OSTaskSaveRun(PSavedTcb,PRunTcb) SET_TASK_STK_INIT(PRunTcb); TaskSaveRun(PSavedTcb,PRunTcb)
        #define OSTaskLoadRun(PRunTcb)  TaskLoadRun(PRunTcb)
        #define OSTaskSaveLoadRun(PSavedTcb,PRunTcb) TaskSaveLoadRun(PSavedTcb,PRunTcb) 
        #define OSFirstTaskRun(PRunTcb)             SET_TASK_STK_INIT(PRunTcb);FirstTaskRun(PRunTcb)
    #else
        #define OSTaskRun(PRunTcb)  SET_TASK_STK_INIT(PRunTcb); TaskRun(PRunTcb)
        #define OSTaskSaveRun(PSavedTcb,PRunTcb) SET_TASK_STK_INIT(PRunTcb); TaskSaveRun(PSavedTcb,PRunTcb)
        #define OSTaskLoadRun(PRunTcb)  TaskLoadRun(PRunTcb)
        #define OSTaskSaveLoadRun(PSavedTcb,PRunTcb) TaskSaveLoadRun(PSavedTcb,PRunTcb) 

    #endif 




     #define INIT_RDY_TBL()  \
                uint8 * prdytbl; \
                uint8     i; \
                prdytbl = &OSRdyTbl[0];\
                for (i = 0; i < OS_RDY_TBL_SIZE; i++) *prdytbl++ = 0x00

    /*set Idle To Ready Status*/
    #define IDLE_READY(PTCB)      SET_TASK_READY(0,PTCB)


///////////////////////////
    #define  TaskTerminateSwitch(OSTCBOld, OSTCBCur)\
                if (RUN_READY != GET_TASK_STATUS(OSTCBCur))\
                {\
                    SET_TASK_STATUS(OSTCBCur, RUN_READY);\
                    OSTaskRun(OSTCBCur);\
                }else\
                {\
                    OSTaskLoadRun(OSTCBCur);\
                }

//////////////////////////


#endif /*#ifndef OS_TASK_CPU_H*/
