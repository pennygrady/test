/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Alarm.h
* DESCRIPTION: OsAlarm module header file based on AUTOSAR OS standard 
* UPDATE HISTORY
* REV         AUTHOR                  DATE       DESCRIPTION OF CHANGE                         
* 1.0         Shuxin.Lin              23/DEC/03        
* 1.1         Hangping.Pan            27/SEP/04        
* 1.2         Xiaonan.Meng&Jiang.Li 1/MAR/05       
* 1.3         Xiaonan.Meng          7/MAR/05    
* 1.4         Luchuan.Xu              7/JUN/05        
*
*********************************************************************************
*/

#ifndef OS_ALARM_H
    #define OS_ALARM_H

/*******************************************************************
| defines and macros (scope: module-local)
|------------------------------------------------------------------*/
    #define AlarmUsed        1U
    #define AlarmUnUsed      0U
    #define ALARM_AUTO_START 1U
    #define INVALID_ALARM     (NULL)          /* invalid alarm*/
    #define INVALID_COUNTER   (OS_COUNTER_NUM+1U)     /*invalid counter*/
        
    /**********************************************************************************
    The following three macros should be defined as follows when the user uses OSMAXALLOWEDVALUE_x
    #define OSMAXALLOWEDVALUE_1 GETMAXALLOWEDVALUE(1)
    If there are more the one, then define OSMAXALLOWEDVALUE_x one by one through by looping 
    to implement the const variants of OSMAXALLOWEDVALUE_1 in OSEK standard
     ***************************************************************
    #define GETMAXALLOWEDVALUE(Index)  OSCounters[Index].CtrInfo->maxallowedvalue
    #define GETTICKSPERBASE(Index)     OSCounters[Index].CtrInfo->ticksperbase
    #define GETMINCYCLE(Index)         OSCounters[Index].CtrInfo->mincycle*/

   
    /*Use ALARMCALLBACK macro to define a callback function of the alarm related*/
    #define ALARMCALLBACK(name) void name(void)    

    #define DeclareAlarm(AlarmIdentifier) extern AlarmType AlarmIdentifier
    /*Change ticks to truly time
    #define OS_TICKS2NS_RTI()
    #define OS_TICKS2US_RTI()
    #define OS_TICKS2MS_RTI()
    #define OS_TICKS2SEC_RTI()
        
    #define OS_TICKS2NS_PIT1()
    #define OS_TICKS2US_PIT1()
    #define OS_TICKS2MS_PIT1()
    #define OS_TICKS2SEC_PIT1()
        
    #define OS_TICKS2NS_PIT2()
    #define OS_TICKS2US_PIT2()
    #define OS_TICKS2MS_PIT2()
    #define OS_TICKS2SEC_PI2()
    
    #define OS_TICKS2NS_PIT3()
    #define OS_TICKS2US_PIT3()
    #define OS_TICKS2MS_PIT3()
    #define OS_TICKS2SEC_PIT3()
    
    #define OS_TICKS2NS_PIT4()
    #define OS_TICKS2US_PIT4()
    #define OS_TICKS2MS_PIT4()
    #define OS_TICKS2SEC_PIT4()
    Change ticks to truly time*/


    #define GET_ALARM_COUNTER_HEAD(PALARM) ((PALARM)->ctr->head)
    #define GET_ALARM_COUNTER_VALUE(PALARM) ((PALARM)->ctr->value)
    #define GET_ALARM_COUNTER_MAXVALUE(PALARM) ((PALARM)->ctr->CtrInfo->OsCounterMaxAllowedValue)
    #define GET_ALARM_COUNTER_MINCYCLE(PALARM) ((PALARM)->ctr->CtrInfo->OsCounterMinCycle)
    #define GET_ALARM_DELTA(PALARM) ((PALARM)->delta)
    #define GET_NEXT_ALARM(PALARM) ((PALARM)->next)
    #define GET_PRV_ALARM(PALARM) ((PALARM)->prev)
    #define GET_ALARM_CYCLE(PALARM) ((PALARM)->cycle)
    #define GET_ALARM_EVENT(PALARM) ((PALARM)->event)
    #define GET_ALARM_TASK(PALARM) ((PALARM)->task)
    /*Get the configuration of the corresponding Alarm*/
    #define GET_ALARM_COUNTER_INFO(PALARM) ((PALARM)->ctr->CtrInfo)
    #define GET_ALARM_STATE(PALARM) ((PALARM)->start)


    #define SET_ALARM_COUNTER_HEAD(PALARM, ALM2SET) (PALARM)->ctr->head = (ALM2SET)
    #define SET_ALARM_USED(PALARM) (PALARM)->start = AlarmUsed
    #define SET_ALARM_UNUSED(PALARM) (PALARM)->start = AlarmUnUsed    
    /*Insert ALM2SET into the alarm list as list head*/
    /*#define SET_ALARM_COUNTER_HEAD(PALARM, ALM2SET) (PALARM)->ctr->head = (ALM2SET)*/
    #define SET_NEXT_ALARM(PALARM, NXTALM) (PALARM)->next = (NXTALM)
    #define SET_PRV_ALARM(PALARM, PRVALM) (PALARM)->prev = (PRVALM)
    #define SET_ALARM_CYCLE(PALARM, CYCLE) (PALARM)->cycle = (CYCLE)
    #define SET_ALARM_DELTA(PALARM, DELTA) (PALARM)->delta = (DELTA)

    #define GET_COUNTER_VALUE(PCTR) ((PCTR)->value)
    #define GET_COUNTER_MAXVALUE(PCTR) ((PCTR)->CtrInfo->OsCounterMaxAllowedValue)
    #define GET_HEAD_ALARM(PCTR) ((PCTR)->head)
    #define GET_COUNTER_CTRDELTA(PCTR) ((PCTR)->ctrDelta)
    
    /* #define GET_COUNTER_REMAIN_TICKS(PCTR) (PCTR)->RemainTicks        */

    #define SET_COUNTER_VALUE(PCTR, VALUE) (PCTR)->value = (VALUE)
    #define COUNTER_VALUE_GROW(PCTR) ((PCTR)->value ++)
    #define COUNTER_RAW_VALUE_GROW(PCTR) ((PCTR)->raw_value ++)
    /* #define REMAIN_TICKS_DECREASE(PCTR);(PCTR)->RemainTicks --*/
    #define SET_HEAD_ALARM(PCTR, HDALARM) (PCTR)->head = (HDALARM)
    #define SET_ALARM_COUNTER_REMAIN_TICKS()
/*******************************************************************
| typedefs and structures (scope: module-local)
|------------------------------------------------------------------*/
    typedef uint8  CounterIndex;
    typedef TickType *TickRefType; /*TickTypeRef*/

    /*Application configuration struct*/
      /* typedef struct OsApplication *OsApplicationRef;*/
    

    /*********************************************************
    The configuration infomation of OS Counter 
     ***********************************************************/
    typedef enum  
    {
        HARDWARE = 0U ,/*hardware counter*/
        SOFTWARE = 1U /*software counter*/
    }OSCounterType ;

    typedef uint8 GptChannelRef;

    typedef struct  
    {
        GptChannelRef OsGptChannelRef ;
    }OsDriver ;

    typedef struct
    {
        TickType    OsConstName; /*Ticks value*/
        float32    OsTimeValue; /* the value of the constant in seconds*/
    }OsTimeConstant,*OsTimeConstantRef;
   

    typedef struct  
    {
        TickType OsCounterMaxAllowedValue; /* Maximum value of ticks */
        TickType OsCounterMinCycle; /*Minimum value of period of alarm*/
        TickType OsCounterTicksPerBase; /* The number of ticks as a specific unit used by counter*/
        OSCounterType OsCounterType ;
        float32 OsSecondsPerTick;/*Time of one hardware tick in seconds.>> not support now*/        
       #if (OS_APPLICATION_MODULE == STD_ON)
            uint8 ApplicationNUM;
            OsApplicationIDTypeRef  OsCounterAccessingApplication;
       #endif 
        
       //OsDriver CounterDrive;/*hardware valid which is related to GPT module, SmartSAR Studio have handled it*/                             
       /*uint8 ConstantNUM;  */                    
       /*OsTimeConstantRef TimeConstant; */  
    }AlarmBaseType,OsCounter, *OsCounterRef;
    typedef AlarmBaseType * AlarmBaseRefType;



    typedef struct AlarmOSCB *AlarmCBRef; /*Two alias, AlarmCBRef is used in internal��AlarmType is used in external*/
    /*********************************************************
    The control infomation of OS Counter
    ***********************************************************/
    typedef struct 
    {
        TickType value;     /*The current ticks, and the range is [0, CtrInfo->maxallowedvalue]*/
        TickType raw_value; /*The result from the requency along with upgrading the original valude to raw_value*/      
        /* TickType RemainTicks; */  /*The remaining ticks*/
        TickType ctrDelta;    /*The ticks of the alarm which should be triggerred earlist in the alarm list mounting on this couter*/
        AlarmCBRef head;      /*List head of the alarm list mounting on this couter*/
        OsCounterRef CtrInfo; /*The configuration infomation of the counter*/
       #if (OS_APPLICATION_MODULE == STD_ON)
        OsApplicationIDTypeRef  OsCounterAccessingApplication;  /*The Application Id array which could access the couter*/
        OsApplicationIDType     OsCounterOwnershipApp;
        uint8    id;       /*id, The API of Application may use it*/
       #endif 
    } CounterCB,*CounterType;
   
    typedef void(*OS_CALLBACK_ENTRY)(void); /*callback function*/

    /*********************************************************
    The configuration infomaiton of the OS Alarm
    ***********************************************************/
    typedef struct 
    {
        EventMaskRefType OsAlarmSetEventRef ; /*The event pointer related to this Alarm */
        TaskIdType osAlarmSetEventTaskRef ;  /*The task Id related to this event*/
    }OsAlarmSetEvent ;
    
    typedef struct 
    {
        TaskIdType OsAlarmActivateTaskRef ; /*The Task pointer related to this Alarm*/
    }OsAlarmActiveTask ;
    
    typedef struct 
    {
        OS_CALLBACK_ENTRY AlarmCallBackName ; /*callback function*/
    }OsAlarmCallBack ;
    
    typedef struct 
    {
        CounterIndex OsAlarmIncrementCounterRef ; /*counter pointer*/
    }OsAlarmIncrementCounter ;   

    typedef struct  
    {
       OsAlarmIncrementCounter IncrementCounterID ;  /*Counter to be increased by this alarm*/
       #if (OS_ALARM_CALLBACK == STD_ON )  
          OsAlarmCallBack  callback ;              /*Callback to be called by this alarm*/
       #endif
       
       #if (OS_EVENT_MODULE == STD_ON)
       OsAlarmSetEvent event ;  /*Event to be set by this alarm*/
       #endif       
       
       TaskIdType  task ;      /*Task to be activated by this alarm*/

    }OsAlarmAction;  
    

    typedef enum 
    {
       ALARM_ABSOLUTE = ABSOLUTE , /*The autostart mode of the alarm is absolute */
       ALARM_RELATIVE = RELATIVE  /*The autostart mode of the alarm is relative */   
    }OsAlarmAutostartType ;
    
    typedef struct  
    {
       TickType OsAlarmAlarmTime ;    /*Ticks value of this value when it is expiried*/
       OsAlarmAutostartType autostarttype ; /*The autostart mode of this alarm*/
       TickType OsAlarmCycleTime ;      /*The period of this alarm*/
       AppModeType OsAlarmAppModeRef ;  /*The App Mode related to this alarm*/
    }OsAlarmAutostart,* OsAlarmAutostartRef;
   
    
    typedef struct   
    {
        uint8 OsAlarmCounterRef;/*Reference to the counterID which will be used by GetAlarmBase function*/
        OsAlarmAction action ; /*The actions of this alarm*/
        OsAlarmAutostartRef autostart ; /*The paramete of the autostart mode*/
       
       #if (OS_APPLICATION_MODULE == STD_ON)
        OsApplicationIDTypeRef  OsAlarmAccessingApplication;
       #endif  
             
    }OsAlarm,OSAlarmCfg;
    typedef OsAlarm *OsAlarmRef;

    /*********************************************************
    The control infomation of OS Alarm
    **********************************************************/
    typedef enum 
    {
      COMMONALARM = COMMON_TYPE,
      SCHETBLALARM = SCHETBL_TYPE,
      RESALARM = RESOURCE_TYPE       
    }OSAlarmApp;
    
    typedef struct AlarmOSCB
    {
        CounterType  ctr;   /*Counter, used by GetAlarmBase function*/
        TickType delta; /*Next value of ticks when the alarm is triggerred*/
        TickType cycle; /*Period*/
        TaskType task; /*Associated tasks. When the alarm is a alarm for resource, then this value corresponds to the task which seize the resource*/
        
        #if (OS_ALARM_CALLBACK == STD_ON)
            OS_CALLBACK_ENTRY AlarmCallBack; /*callback function*/
        #endif /*#ifdef OS_ALARM_CALLBACK_EN*/

        #if (OS_EVENT_MODULE == STD_ON)
            EventMaskRefType event; /*event mask*/
        #endif   /*#ifdef OS_EVENT_EN*/
        
        CounterIndex IncrementCounterID ;  /*Counter to be increased by this alarm*/
        AlarmCBRef next; /*Next alarm node in the alarm list*/
        AlarmCBRef prev; /*Previous alarm node in the alarm list*/
        uint8      start; /*status flag, 0: alarm is unused, 1: alram is used */
        #if (OS_SCHEDULE_TABLE_MODULE == STD_ON)
           ScheduleTableType  ScheduleTableId; /*event mask*/
            //boolean AlarmOrScheduleTable;    /*Whether this Alarm belong to Alarm module or Schedule Table module*/
        #endif   /*#ifdef OS_SCHEDULE_TABLE_MODULE_EN*/
        
        OSAlarmApp  AlarmApp; /*Whether this Alarm belong to CommonAlarm or ScheTblAlarm or ResAlarm*/
        
       #if (OS_APPLICATION_MODULE == STD_ON)
        OsApplicationIDTypeRef  OsAlarmAccessingApplication;
        uint8     id;     /*ID of the Alarm, mainly used in the Application*/
        OsApplicationIDType     OsAlarmOwnershipApp;
       #endif 
       
    } AlarmCB,*AlarmType;

/*******************************************************************
| external declarations:
| function prototypes (scope: module-exported)
| extern variable  (scope: module-exported)
|------------------------------------------------------------------*/
    extern AlarmCB OSAlarms[]; /*Configuration array of Alarm*/
    extern CounterCB OSCounters[]; /*Configuration array of Counter*/
/******************************************************************* 
| function prototypes (scope: module-exported) 
|------------------------------------------------------------------*/    
    void OSAlarmInit(void);
    StatusType GetAlarm(AlarmType AlarmID, TickRefType tick);
    StatusType CancelAlarm(AlarmType AlarmID);
    StatusType GetAlarmBase(AlarmType AlarmID, AlarmBaseRefType Info);
    StatusType SetRelAlarm(AlarmType AlarmID, TickType increment, TickType cycle);
    StatusType SetAbsAlarm(AlarmType AlarmID, TickType start, TickType cycle);
    void OSCounterTrigger(CounterType isr);
    void OSDeleteAlarm(AlarmCBRef alarm);
    void OSAlarmCounterTrigger(CounterType ctr);
#endif
