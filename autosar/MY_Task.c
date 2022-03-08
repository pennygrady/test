/*
 * MY_Task.c
 *
 *  Created on: Jul 16, 2019
 *      Author: Jerry
 */


/******************** Task body *******************/
#include <Platform_Lib.h>
#include "main.h"
#include "Os_Task.h"
#include "../../asw/pkg/sys_print/sys_print.h"
#include "../../asw/vcm/vcm.h"
#include "../../asw/vcfg/carpara.h"
#include "lwip.h"
#include "../../asw/PFAL/pfal.h"
#include "../../asw/PowerCtrl/PowerCtrl.h"

/* 低优先级任务，保留*/
TASK(OsTask_0)
{
    DEFINECRITICAL();
//    PKG_Print(_TRACE, "task 0 start\r\n");
//    PKG_Print(_TRACE, "task 1 end\r\n");
    (void) TerminateTask();
}

/* 低优先级任务，传感器数据读取 */
TASK(OsTask_1)
{
    DEFINECRITICAL();
    if(VCFG_IsInitOk())
    {
        /* zrDebug */
        printf("Taks 1Run \n");
        /* zrDebug */

        /* 传感器数据读取模块 */
        SIC_TickHdl();
    }
   (void) TerminateTask();
}

/* 安全处理处理底盘时间，优先级较高 */
TASK(OsTask_2)
{
    DEFINECRITICAL();

    if(VCFG_IsInitOk())
    {
        /* 安全模块周期执行任务 */
        SftySchMain();
        sftyMonitorMain();
    //    Testlog2Buffer();
    }
    (void) TerminateTask();
}


/* 车辆控制，优先级最高 */
TASK(OsTask_3)
{
    DEFINECRITICAL();

    if(VCFG_IsInitOk())
    {
        VCM_APP();
        VCM_TickHdl();
    }

   (void) TerminateTask();
}

/* 通信管理 */
TASK(OsTask_4)
{
    DEFINECRITICAL();
//    PKG_Print(_TRACE, "task 4 start\r\n");

//    PKG_Print(_TRACE, "lwip start\r\n");
    MX_LWIP_Process();
//    PKG_Print(_TRACE, "lwip end\r\n");
    COM_TickHdl();//控制连接监测

    //电源监测
//    ACUExceptionMonitor();

//    COM_UART_TxTickHdl();
//    comm_EvTickHdl();//必要时主动调用lwip的poll
//    PKG_Print(_TRACE, "task 4 end\r\n");
    (void) TerminateTask();
}

/* 车辆配置*/
TASK(OsTask_5)
{
    DEFINECRITICAL();

    PWRCTRL_StageMachineRunning();

    (void) TerminateTask();
}

TASK(OsTask_6)
{
    DEFINECRITICAL();
//    PKG_Print(_TRACE, "task 6 start\r\n");
//    PKG_Print(_TRACE, "task 6 end\r\n");
    (void) TerminateTask();
}

