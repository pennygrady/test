/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Hook.h
* DESCRIPTION: source file containing the implements related to OS error mechanism
*             and core functions
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0         Wei.Chen      4/DEC/03       Initial version. 
* 1.1     Hangping.Pan    20/JAN/05 
*
*********************************************************************************
*/

#ifndef OS_HOOK_H
  #define OS_HOOK_H
#include "Os_Cfg.h"
   

    typedef struct 
    {
       boolean OsErrorHook;
       boolean OsPostTaskHook;
       boolean OsPreTaskHook;
       boolean OsProtectionHook;
       boolean OsShutdownHook;
       boolean OsStartupHook;
    }OsHooks;

 
     /***************All errors of API services**********************/
   
    #define E_OS_ACCESS     1U
    #define E_OS_CALLEVEL   2U
    #define E_OS_ID         3U
    #define E_OS_LIMIT      4U
    #define E_OS_NOFUNC     5U
    #define E_OS_RESOURCE   6U
    #define E_OS_STATE      7U
    #define E_OS_VALUE      8U
    #define E_OS_TICKS      9U
    #define E_OS_STACKFAULT 10U
    #define E_OS_PROTECTION_ARRIVAL 11U
    #define E_OS_PROTECTION_LOCKED  12U
    #define E_OS_PROTECTION_TIME    13U
   
  
    //Not used
    //    #define E_OS_SYS_BASE    20/*Internal errors of the operating system*/
    //    #define E_OS_SYS_PARA            ((E_OS_SYS_BASE)+1)/*parameter error*/
    //    #define E_OS_SYS_RESULT            ((E_OS_SYS_BASE)+2) /*result error*/
    //    #define E_OS_SYS_PRIO_EXIST     ((E_OS_SYS_BASE)+3)/*Priority has already exist*/
    //    #define E_OS_INT_ERR            (E_OS_SYS_BASE)+4)/*isr error*/
    //    #define E_OS_STACK              ((E_OS_SYS_BASE)+5) /*stack error*/
    //Not used
/**********************************************************************************
//HOOK Module
----------------------------------------------------------------------------------------*/
#if (STD_OFF == OS_HOOK_MODULE)
         #define OSRETERROR( error, ID, param )   return( error )
         #define OSShutdownHook(Error)
         #define OSPreTaskHook()  
         #define OSPostTaskHook()  
         #define OSStartupHook()  
#else    
    /**********************************************************
    ErrorHook:higher prior than all tasks; not interrupted by ISR2;ERROR HOOK can not called recursively
     ***********************************************************/    
    #if (OS_ERROR_HOOK == STD_ON)
            void ErrorHook(StatusType error);
            extern boolean OSNestedErrorHook;
        #if (OS_ERROR_GET_SERVICEID == STD_ON)
            typedef uint8 OsServiceIdType; /* Service Id Type*/
            extern  OsServiceIdType OS_ServiceId;
            #define OSErrorGetServiceId()  OS_ServiceId
            
            /* OSEK OS Service ID Base*/         
            #define OS_SYSTEM_SERVICE_ID_BASE       0x00U    /* OS*/         
            #define OSServiceId_StartOS                     (OsServiceIdType)( OS_SYSTEM_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_ShutdownOS                  (OsServiceIdType)( OS_SYSTEM_SERVICE_ID_BASE + 0x04 )
            #define OSServiceId_GetActiveApplicationMode    (OsServiceIdType)( OS_SYSTEM_SERVICE_ID_BASE + 0x06 )
            #define OSServiceId_OSCounterTrigger            (OsServiceIdType)( OS_SYSTEM_SERVICE_ID_BASE + 0x08 )

            #define OS_TASK_SERVICE_ID_BASE         0x10U    /* Task */
            #define OSServiceId_ActivateTask                (OsServiceIdType)( OS_TASK_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_TerminateTask               (OsServiceIdType)( OS_TASK_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_ChainTask                   (OsServiceIdType)( OS_TASK_SERVICE_ID_BASE + 0x04 )
            #define OSServiceId_Schedule                    (OsServiceIdType)( OS_TASK_SERVICE_ID_BASE + 0x06 )
            #define OSServiceId_GetTaskID                   (OsServiceIdType)( OS_TASK_SERVICE_ID_BASE + 0x08 )
            #define OSServiceId_GetTaskState                (OsServiceIdType)( OS_TASK_SERVICE_ID_BASE + 0x0A )
            
            #define OS_ISR_SERVICE_ID_BASE          0x20U    /* ISR */
            #define OSServiceId_ResumeAllInterrupts         (OsServiceIdType)( OS_ISR_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_SuspendAllinterrupts        (OsServiceIdType)( OS_ISR_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_ResumeOSInterrupts          (OsServiceIdType)( OS_ISR_SERVICE_ID_BASE + 0x04 )
            #define OSServiceId_SuspendOSinterrupts         (OsServiceIdType)( OS_ISR_SERVICE_ID_BASE + 0x06 )
            #define OSServiceId_EnableAllInterrupts         (OsServiceIdType)( OS_ISR_SERVICE_ID_BASE + 0x08 )
            #define OSServiceId_DisableAllInterrupts        (OsServiceIdType)( OS_ISR_SERVICE_ID_BASE + 0x0A )
            #define OSServiceId_GetISRID                    (OsServiceIdType)( OS_ISR_SERVICE_ID_BASE + 0x0C )
            
            #define OS_RESOURCE_SERVICE_ID_BASE     0x30U   /* Resourece*/
            #define OSServiceId_DefineResource                      (OsServiceIdType)( OS_RESOURCE_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_GetResource                 (OsServiceIdType)( OS_RESOURCE_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_ReleaseResource             (OsServiceIdType)( OS_RESOURCE_SERVICE_ID_BASE + 0x04 )

            #define OS_EVENT_SERVICE_ID_BASE        0x40U    /* event*/
            #define OSServiceId_SetEvent                    (OsServiceIdType)( OS_EVENT_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_ClearEvent                  (OsServiceIdType)( OS_EVENT_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_GetEvent                    (OsServiceIdType)( OS_EVENT_SERVICE_ID_BASE + 0x04 )
            #define OSServiceId_WaitEvent                   (OsServiceIdType)( OS_EVENT_SERVICE_ID_BASE + 0x06 )

            #define OS_ALARM_SERVICE_ID_BASE        0x50U   /* ALARM */
            #define OSServiceId_GetAlarmBase                (OsServiceIdType)( OS_ALARM_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_GetAlarm                    (OsServiceIdType)( OS_ALARM_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_SetRelAlarm                 (OsServiceIdType)( OS_ALARM_SERVICE_ID_BASE + 0x04 )
            #define OSServiceId_SetAbsAlarm                 (OsServiceIdType)( OS_ALARM_SERVICE_ID_BASE + 0x06 )
            #define OSServiceId_CancelAlarm                 (OsServiceIdType)( OS_ALARM_SERVICE_ID_BASE + 0x08 )

            #define OS_SCHEDULETABLE_SERVICE_ID_BASE 0x60U   /*Schedule Table*/    
            #define OSServiceId_StartScheduleTableRel       (OsServiceIdType)( OS_SCHEDULETABLE_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_StartScheduleTableAbs       (OsServiceIdType)( OS_SCHEDULETABLE_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_StopScheduleTable           (OsServiceIdType)( OS_SCHEDULETABLE_SERVICE_ID_BASE + 0x04 )
            #define OSServiceId_NextScheduleTable           (OsServiceIdType)( OS_SCHEDULETABLE_SERVICE_ID_BASE + 0x06 )
            #define OSServiceId_StartScheduleTableSynchron  (OsServiceIdType)( OS_SCHEDULETABLE_SERVICE_ID_BASE + 0x08 )
            #define OSServiceId_SyncScheduleTable           (OsServiceIdType)( OS_SCHEDULETABLE_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_SetScheduleTableAsync       (OsServiceIdType)( OS_SCHEDULETABLE_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_GetScheduleTableStatus      (OsServiceIdType)( OS_SCHEDULETABLE_SERVICE_ID_BASE + 0x04 )

            #define OS_COUNTER_SERVICE_ID_BASE      0x70U    /* counter*/
            #define OSServiceId_IncrementCounter            (OsServiceIdType)( OS_COUNTER_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_GetCounterValue                (OsServiceIdType)( OS_COUNTER_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_GetElapsedCounterValue      (OsServiceIdType)( OS_COUNTER_SERVICE_ID_BASE + 0x04 )

            #define OS_HOOK_SERVICE_ID_BASE         0x80U    /* HOOK*/
            #define OSServiceId_ErrorHook                   (OsServiceIdType)( OS_HOOK_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_PreTaskHook                 (OsServiceIdType)( OS_HOOK_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_PostTaskHook                (OsServiceIdType)( OS_HOOK_SERVICE_ID_BASE + 0x04 )
            #define OSServiceId_StartupHook                 (OsServiceIdType)( OS_HOOK_SERVICE_ID_BASE + 0x06 )
            #define OSServiceId_ShutdownHook                (OsServiceIdType)( OS_HOOK_SERVICE_ID_BASE + 0x08 )         
            
            #define OS_APPLICATION_SERVICE_ID_BASE   0x90U   /*Application*/        
            #define OSServiceId_TerminateApplication         (OsServiceIdType)( OS_APPLICATION_SERVICE_ID_BASE + 0x00 )
            #define OSServiceId_GetApplicationID             (OsServiceIdType)( OS_APPLICATION_SERVICE_ID_BASE + 0x02 )
            #define OSServiceId_CheckObjectAccess            (OsServiceIdType)( OS_APPLICATION_SERVICE_ID_BASE + 0x04 )
            #define OSServiceId_CheckObjectOwnership         (OsServiceIdType)( OS_APPLICATION_SERVICE_ID_BASE + 0x06 )
        #endif       
            

        /**********************************************************************
        OS_ERROR_PARAM_ACCESS:
        The macro to access the first parameter of a system service is mandatory if the parameter is an object identifier. 
         ***********************************************************************/
        #if (OS_ERROR_PARAM_ACCESS == STD_ON)
            typedef uint32 ObjectIDType;
            extern ObjectIDType OS_ObjectId;
            
            /*OS*/
            #define OSError_StartOS_Mode()              (AppModeType)OS_ObjectId
            /* TASK */
            #define OSError_ActivateTask_TaskID()       (TaskType)OS_ObjectId
            #define OSError_ChainTask_TaskID()          (TaskType)OS_ObjectId
            #define OSError_GetTaskState_TaskID()       (TaskType)OS_ObjectId
            /* Resource*/
            #define OSError_GetResource_ResID()         (ResourceType)OS_ObjectId
            #define OSError_ReleaseResource_ResID()     (ResourceType)OS_ObjectId
            /* EVENT*/
            #define OSError_SetEvent_TaskID()           (TaskType)OS_ObjectId
            #define OSError_GetEvent_TaskID()           (TaskType)OS_ObjectId
            /* ALARM */
            #define OSError_GetAlarmBase_AlarmID()      (AlarmType)OS_ObjectId
            #define OSError_GetAlarm_AlarmID()          (AlarmType)OS_ObjectId
            #define OSError_SetRelAlarm_AlarmID()       (AlarmType)OS_ObjectId
            #define OSError_SetAbsAlarm_AlarmID()       (AlarmType)OS_ObjectId
            #define OSError_CancelAlarm_AlarmID()       (AlarmType)OS_ObjectId
        #endif


        #if (OS_ERROR_PARAM_ACCESS == STD_ON)

            #if (OS_ERROR_GET_SERVICEID == STD_ON)
                #define OSSetServiceId(ID, param)  OS_ServiceId = ID; OS_ObjectId = (ObjectIDType) param
            #else 
                #define OSSetServiceId(ID, param)  OS_ObjectId = (ObjectIDType) param
            #endif 
            
        #else 
        
            #if (OS_ERROR_GET_SERVICEID == STD_ON) 
                #define OSSetServiceId(ID, param) OS_ServiceId = ID
            #else 
                #define OSSetServiceId(ID, param)
            #endif 

        #endif /*define OS_ERROR_PARAM_ACCESS*/

        #define OSRETERROR( error, ID, param )\
                            if(OSNestedErrorHook == FALSE)\
                            {\
                                DISABLE();\
                                OSNestedErrorHook = TRUE;\
                                OSSetServiceId(ID, param);\
                                ErrorHook(error);\
                                OSNestedErrorHook = FALSE;\
                                ENABLE();\
                            }\
                            return( error )                            
       
    #else /*OS_ERROR_HOOK*/
        #define OSRETERROR( error, ID, param )   return( error)
    #endif 

    /**********************************************************
    PreTaskHook
     ***********************************************************/
  
    #if (STD_ON == OS_PRE_TASK_HOOK)
           void PreTaskHook(); /*OSPreTaskHook call PreTaskHook��Os_Core.c��*/
        #define OSPreTaskHook()                PreTaskHook()
       
    #else 
        #define OSPreTaskHook()  
    #endif 

    /**********************************************************
    PostTaskHook
     ***********************************************************/
    #if (STD_ON == OS_POST_TASK_HOOK)
        void PostTaskHook(void);
           #define OSPostTaskHook()             PostTaskHook()
    #else 
       #define OSPostTaskHook()  
    #endif 


    /**********************************************************
    StartupHook
     ***********************************************************/
    #if ( STD_ON== OS_STARTUP_HOOK )
        void StartupHook(void);        
        #define OSStartupHook()    StartupHook()
    #else 
        #define OSStartupHook()  
    #endif 

    /**********************************************************
    ShutdownHook
     ***********************************************************/
    #if ( STD_ON == OS_SHUTDOWN_HOOK )
       void ShutdownHook(StatusType error);       
           #if(ECUM_MODULE == STD_ON)
            #define OSShutdownHook( error )\
            do             \
            {         \
                ShutdownHook(error);\
                EcuM_Shutdown();\
            }while(0U) 
        #else
            #define OSShutdownHook( error )\
            do             \
            {         \
                ShutdownHook(error);\
            }while(0U) 
        #endif
    #else 
        #define OSShutdownHook( error)  
    #endif 
        
    /**********************************************************
     ProtectionHook
     ***********************************************************/
      #if (OS_PROTECTION_HOOK == STD_ON)
         #define PRO_IGNORE                1
         #define PRO_TERMINATETASKISR      2
         #define PRO_TERMINATEAPPL         3
         #define PRO_TERMINATEAPPL_RESTART 4
         #define PRO_SHUTDOWN              5
         StatusType ProtectionHook(StatusType error);
         StatusType OSProtectionHook(StatusType Error,TaskType ptcb);   
    #endif


  
#endif  

#endif
    
