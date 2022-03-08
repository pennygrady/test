/*
 *********************************************************************************
 * Copyright (c) 2011,Embedded System Engineering Center of  Zhejiang University
 * All rights reserved.
 *
 * FILE NAME: IntHandler.c
 * DESCRIPTION:
 * UPDATE HISTORY
 * REV      AUTHOR      DATE       DESCRIPTION OF CHANGE
 * 1.0       ESE       07/MAR/11   Initial version.
 *
 *********************************************************************************
 */

#include "IntHandle.h"
extern uint8 IntNesting;

#if (OS_INTERRUPT_MODULE == STD_ON)

void OSIntExit(void)
{
    TaskType ptcb = NULL;
    TaskType oldRunning = NULL;
    uint8 *ReadyListPtr;
    uint8 highestPrio;

    if (--IntNesting == 0)
    {
#if (OS_SCHEDULE_POLICY != OS_SCHEDULE_FULL)
#if (OS_SCHEDULE_POLICY == OS_SCHEDULE_MIX)
        if (FULL_SCHED != GET_TASK_SCHED(OSTCBCur))
        {
            return; /*Task not support SCHED_METHOD*/
        }
#else
        if (0 != GET_TASK_ORIGINAL_PRIO(OSTCBCur))
        { /*if TASK is idle task, it needs schedule new task.*/
            return; /*Task not support SCHED_METHOD*/
        }
#endif /*#ifdef OS_MIX_SCHEDULE*/
#endif /*#ifndef OS_FULL_SCHEDULE*/

        oldRunning = OSTCBCur;
        highestPrio = OS_FIND_HIGHEST_TASK(); /*Find the highest Priority task who is ready to run*/
        ReadyListPtr = ReadyListCBTbl[highestPrio].head; /*Find the task TCB*/
        ptcb = &OSTCBTbl[*(ReadyListPtr + ReadyListCBTbl[highestPrio].start)];/*update OSTCBCur*/

        if (ptcb != oldRunning)
        {
            /*preempt by the higher Priority task*/
            // RELEASE_INT_RES(oldRunning); //OS223:Points of rescheduling
            OS_TASK_SWITCH(oldRunning); OSPostTaskHook(); /*when task is preempted ��call PostTaskHook*/
            OSTCBCur = ptcb;

            OSPreTaskHook(); /*call PreTaskHook*/
            OSStackMonitoring(oldRunning);
#if (OS_EVENT_MODULE == STD_ON)
            if (RUN_READY != GET_TASK_STATUS(ptcb))
#endif /*#ifdef  OS_EVENT_EN*/
            {
                SET_TASK_STATUS(ptcb, RUN_READY); /*set task to ready who is the first time to run*/
                OSTaskSaveRun(oldRunning, ptcb);
            }
#if (OS_EVENT_MODULE == STD_ON)
            else
            {
                OSTaskLoadRun(ptcb);
            }
#endif /*#ifdef  OS_EVENT_EN*/
        }
    }
}
#endif /* #if (OS_INTERRUPT_MODULE == STD_ON) */

