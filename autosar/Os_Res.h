/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Os_Res.h
* DESCRIPTION: Resource module header file based on AUTOSAR OS standard 
* UPDATE HISTORY
* REV         AUTHOR          DATE       DESCRIPTION OF CHANGE                         
* 1.0          Wei.Chen     4/DEC/03       Initial version.   
* 1.1          Jiang.Li    20/JAN/04   
*
*********************************************************************************
*/
#ifndef OS_RES_H
    #define OS_RES_H


    #define INVALID_RESOURCE    (NULL)          /* invalid alarm*/
    #define INVALID_RESOURCE_PRIO (0x0U)
   
    /*************************************************************
    Macro operation of internal resource
     ***************************************************************/
   #define ACTUAL_HOLD_INT_RES 0x08U
   #define GET_ACTUAL_HOLD_INT_RES(PTCB) ((PTCB)->InfoBit & ACTUAL_HOLD_INT_RES)
   #define SET_ACTUAL_HOLD_INT_RES(PTCB) ((PTCB)->InfoBit=(PTCB)->InfoBit | ACTUAL_HOLD_INT_RES)
   #define RESET_ACTUAL_HOLD_INT_RES(PTCB) ((PTCB)->InfoBit=(PTCB)->InfoBit &( ~ACTUAL_HOLD_INT_RES) )

 
    #if (OS_RES_INTERNAL == STD_ON) /*internal resource*/
     void  GET_INT_RES(TaskType ptcb);
     void  RELEASE_INT_RES(TaskType ptcb);     
    #else /*#ifdef OS_INT_RES*/
      #define GET_INT_RES(PTCB)
      #define RELEASE_INT_RES(curTcb)    
    #endif /*#ifdef OS_INT_RES*/ 
    
    typedef struct OSResCB *ResourceType; /*resource type*/  
    #if (OS_RES_INTERNAL == STD_ON) 
    typedef struct OSInternalResCB *ResourceInternalType;  
    #endif 
 
/****************************************
 typedef of the resource module
*****************************************/    
 typedef enum{INTERNAL,LINKED,STANDARD}OsResourceProperty;     /*Property of resource*/ 
 
 typedef struct
 {/*struct which is used as the configurator to uers.*/
   OsResourceProperty ResProperty; 
   OsResourceIndex OsResLinkedResRef;/*if the resource is LINKED��?this field is point to the linked resource*/
   ResPrioType ResPrio;
   #if (OS_APPLICATION_MODULE == STD_ON)
     OsApplicationIDTypeRef  OsResourceAccessingApplication;
   #endif    
 }OsResource;   

  #if (OS_RES_INTERNAL == STD_ON)
     typedef struct OSInternalResCB
      {
          ResPrioType priority;     
          OsResourceIndex ResId;   /*internal ID which is the index of the config array.*/
         // uint8 IsUsed;          
         // TaskType SavedTCB;                  
          #if (OS_APPLICATION_MODULE == STD_ON)
            OsApplicationIDTypeRef  OsResourceAccessingApplication;
            OsApplicationIDType     OsResOwnershipApp; 
          #endif 
          
      }OSRCBInternal,*OSRCBInternalRef;
      extern OSRCBInternal OSResourcesInternal[]; 
    #endif  
      
       
    /*************************************************************
        MACRO OPERATION OF RESOURCE MACRO.
     ***************************************************************/

    #if (OS_RES_EXTERNAL == STD_ON) || (OS_RES_SCHEDULER == STD_ON) /*OUT RESOURCE AND SCHEDULER RESOURCE*/

        #define OUT_RES_USED  1U
        #define OUT_RES_NOT_USED  0U
     
        #if (OS_RES_SCHEDULER == STD_ON)            
                    #define OS_RES_SCHEDULER_NUM OS_RES_NUM
        #endif
                 
        #if (OS_RES_INTERRUPT == STD_ON) /*Interrupt resource*/
            typedef uint32 OSISRMaskType; /*MASK type*/
        #endif 
        
        typedef struct OSResCB
        {
            ResPrioType priority; 
            PriorityType OldRunPrio; /*original Prio*/
            OsResourceIndex ResId;   /*Out resource id in all resource, which is the index of the config array.*/
            
            #if (OS_EXTENDED_STATUS == STD_ON)
                uint8 IsUsed; /*0:used 0: unused*/
            #endif 
            
            TaskType SavedTCB; /* TCB pointer of the task*/
            OSRCBRef NextRes; /*next resource in the resource list of the task*/
            
            #if (OS_RES_INTERRUPT == STD_ON) 
                OSISRMaskType SavedMask; /*Interrrupt resource MASK*/
            #endif 
              
           #if (OS_APPLICATION_MODULE == STD_ON)
                OsApplicationIDTypeRef  OsResourceAccessingApplication;
                OsApplicationIDType     OsResOwnershipApp; 
           #endif 
           // AlarmCBRef    alarm;                /*Alarm pointer which is used to timing protection*/
        }OSRCB;
        extern OSRCB OSResources[];
        
        /***********************************************************
         Macro operation of  resource
         *************************************************************/             
        #define GET_RES_PRIO(PRCB) ((PRCB)->priority)        
        #define GET_RES_OLD_RUN_PRIO(PRCB) ((PRCB)->OldRunPrio)
        #define GET_RES_SAVED_TCB(PRCB) ((PRCB)->SavedTCB)
        #define GET_RES_NEXT(PRCB)  ((PRCB)->NextRes)
        #define SET_RES_NEXT(PRCB,next)  (PRCB)->NextRes=next

        /********************************************************************
         * * Prio change when get the out resource
         *********************************************************************/
        #define SET_OUT_PRIORITY(RCB,TCB) \
            do{if(((RCB)->OldRunPrio = GET_TASK_RUN_PRIO(TCB) ) < GET_RES_PRIO(RCB))\
            {\
              OS_TASK_TO_READY((RCB)->priority,TCB);\
              INSERT_READY_LIST_HEAD(GET_TASK_ID(TCB), GET_RES_PRIO(RCB));\
              SET_TASK_RUN_PRIO(TCB,(RCB)->priority);\
            }}while(0)

        /********************************************************************
         * Prio change when release the out resource
         * 1)reset the task prio, restore to the previous Prio before the task get this resource
         * 2)set TCB  to the previous position OSTCBPrioTbl
         *********************************************************************/
        #define RESET_OUT_PRIORITY(TCB, RCB) \
            do{if(( GET_TASK_RUN_PRIO(TCB) ) > GET_RES_OLD_RUN_PRIO(RCB))\
            {\
              REMOVE_READY_LIST_HEAD(GET_RES_PRIO(RCB));\
              OS_TASK_TO_UNREADY(GET_RES_PRIO(RCB),TCB);\
              SET_TASK_RUN_PRIO(TCB,GET_RES_OLD_RUN_PRIO(RCB));\
            }}while(0)

        /*add resource to the resource list of the task*/
        #define ADD_RES_LIST(TCB,RCB) \
            SET_RES_NEXT(RCB,(TCB)->resources);\
            SET_TASK_OUT_RES((TCB),RCB)

        /*delete resource from the resource list of the task*/
        #define DELETE_RES_LIST(Tcb,RCB) (Tcb)->resources=RCB->NextRes

        /***********************************************************
         Macro operation of interrupt resource
         *************************************************************/
        #if (OS_RES_INTERRUPT == STD_ON)
            extern OSRCBRef OSISRLastResource;
            #define SET_RES_ISR_MASK(RCB,mask)  (RCB)->SavedMask=mask
            #define GET_RES_ISR_MASK(RCB)       ((RCB)->SavedMask)
            /*add ISR_Res List*/
            #define ADD_RES_ISR_LIST(RCB) \
            (RCB)->NextRes= OSISRLastResource; \
            OSISRLastResource = (RCB)
        #else 
            #define SET_RES_ISR_MASK(RCB,mask)
        #endif 

        /***********************************************************
         Macro operation under extended status
         *************************************************************/
        #if (OS_EXTENDED_STATUS == STD_ON)
            #define GET_RES_USED(PRCB) ((PRCB)->IsUsed)
            #define SET_RES_USED(PRCB) (PRCB)->IsUsed=OUT_RES_USED
            #define SET_RES_UNUSED(PRCB) (PRCB)->IsUsed=OUT_RES_NOT_USED
        #else
            #define SET_RES_USED(PRCB)
            #define SET_RES_UNUSED(PRCB)
        #endif 

        #define NO_RESOURCE ((OSRCBRef)0) 
        #define DeclareResource(ResId) extern ResourceType ResId
        void ResourceInit(void);
    #endif 
    
    
       /***********************************************************
         Macro operation of Scheduler resource and out resource
         *************************************************************/    
    #if (OS_RES_SCHEDULER == STD_ON) && (OS_RES_EXTERNAL == STD_ON) /*if  Scheduler resource and out resource both are enable*/
        extern ResourceType RES_SCHEDULER; 
        #define GetResource(ResId) \
        (ResId==RES_SCHEDULER)?GetSchedulerResource(RES_SCHEDULER):GetOutResource(ResId)

        #define ReleaseResource(ResId) \
        (ResId==RES_SCHEDULER)?ReleaseSchedulerResource(RES_SCHEDULER):ReleaseOutResource(ResId)
    #endif 

    #if (OS_RES_SCHEDULER == STD_ON) && (OS_RES_EXTERNAL == STD_OFF) /*if only Scheduler resource is enable*/
        extern ResourceType RES_SCHEDULER; 
        #define GetResource(ResId) GetSchedulerResource(ResId)
        #define ReleaseResource(ResId) ReleaseSchedulerResource(ResId)
        StatusType GetSchedulerResource(ResourceType res);
    #endif 

    #if (OS_RES_EXTERNAL == STD_ON) && (OS_RES_SCHEDULER == STD_OFF)/*if only out resource is enable*/
        #define GetResource(ResId) GetOutResource(ResId)
        #define ReleaseResource(ResId) ReleaseOutResource(ResId)
    #endif 

        #if (OS_RES_EXTERNAL == STD_ON)
        StatusType ReleaseOutResource(ResourceType res);
        StatusType GetOutResource(ResourceType res);
        #endif /* (#if (OS_RES_EXTERNAL == STD_ON)) */
#endif /*ifndef     OS_RES_H*/
