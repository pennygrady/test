/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering Center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: PrologueEpilogue.h
* DESCRIPTION: save and restore context
* UPDATE HISTORY
* REV      AUTHOR                  DATE       DESCRIPTION OF CHANGE                         
* 1.0      HaoJie Chen, Li Zhou    07/MAR/11   Initial version.   
*
*********************************************************************************
*/
#ifndef PROLOGUEEPILOGUE_H
 #define PROLOGUEEPILOGUE_H
 

#if (OS_MODULE == STD_ON)
#include "Os_Cfg.h"
#endif

/*#########################################################################
#        Stack Frame Definition
#    This Stack frame definitions have to be congruent with the 
#    Stack Frame usage in Os_Cpu_C.c
#########################################################################*/

#define XR1     4
#define    XR0    8
#define    XSRR0    12
#define    XSRR1    16
#define    XCTR    20
#define    XXER    24
#define    XCR    28
#define    XLR    32
#define    XR3    36
#define    XR4    40
#define    XR5    44
#define    XR6    48
#define    XR7    52
#define    XR8    56
#define    XR9    60
#define    XR10    64
#define    XR11    68
#define    XR12    72
#define    XR25    76
#define    XR26    80
#define    XR27    84
#define    XR28    88
#define    XR29    92
#define    XR30    96
#define    XR31    100
#define    XMSR    104
#define    Total_Offset    108


#define NOFRALLOC_I nofralloc

/*##############################################################################
#
#    TaskPrologue        
#    Task Context protection: the next PC value is store in LR, we must store LR to SRR0 
#   becuuse when rescore the context, the PC value is from SRR0
#
##############################################################################    */        
#define TaskPrologue    NOFRALLOC_I /* Store r28 */;\
            e_stwu    r1, -Total_Offset (r1)/* Create stack frame */;\
            stw r0,  XR0 (r1) /* Store r0 working register  */;\
            mflr    r0;\
            stw        r0,XSRR0(r1) /*#the only difference with ISR2Prologue��store lr to SRR0*/;\
            mfsrr1  r0   ;\
            stw        r0,XSRR1(r1)/* Store SRR1 (before enabling EE)*/;\
            mfcr    r0 ;\
            stw     r0,  XCR (r1)/* Store CR */;\
            mfxer   r0 ;\
            stw     r0,  XXER (r1)/* Store XER */;\
            mfctr   r0  ;\
            stw     r0,  XCTR (r1) /* Store CTR */;\
            mflr    r0  ;\
            stw     r0,  XLR (r1)/* Store LR */; \
            mfmsr     r0  ;\
            stw          r0,  XMSR(r1)/* Store MSR */    ;\
            stw     r3, XR3 (r1) /* Store r3 */;\
            stw     r4, XR4 (r1) /* Store r4 */ ;\
            stw     r5, XR5 (r1) /* Store r5 */ ;\
            stw     r6, XR6 (r1) /* Store r6 */ ; \
            stw     r7, XR7 (r1)  /* Store r7 */;\
            stw     r8, XR8 (r1) /* Store r8 */;    \
            stw     r9, XR9 (r1) /* Store r9 */;\
            stw     r10, XR10 (r1)/* Store r10 */;\
            stw     r11, XR11 (r1) /* Store r11 */;\
            stw     r12, XR12 (r1)  /* Store r12 */;\
            stw     r25, XR25 (r1)  /* Store r25 */;\
            stw     r26, XR26 (r1)  /* Store r26 */  ;\
            stw     r27, XR27 (r1) /* Store r27 */ ;\
            stw     r28, XR28 (r1)  /* Store r28 */;\
            stw     r29, XR29 (r1)      /* Store r29 */     ;\
            stw     r30, XR30 (r1)      /* Store r30 */;\
            stw     r31, XR31 (r1)    /* Store r31 */ 


/*##############################################################################
#
#    TaskEpilogue    :
#    Rescore of the task context
#
##############################################################################            */
#define TaskEpilogue       lwz     r0,  XLR (r1)      /* Restore LR */;\
        mtlr    r0;\
        lwz     r0,  XCTR (r1)      /* Restore CTR */;\
        mtctr   r0;\
        lwz     r0,  XXER (r1)      /* Restore XER */;\
        mtxer   r0;\
        lwz     r0,  XCR (r1)      /* Restore CR */;\
        mtcrf   0xff, r0;\
        lwz     r3,  XR3 (r1)      /* Restore r3 */;\
        lwz     r4,  XR4 (r1)      /* Restore r4 */;\
        lwz     r5,  XR5 (r1)      /* Restore r5 */;\
        lwz     r6,  XR6 (r1)      /* Restore r6 */;\
        lwz     r7,  XR7 (r1)      /* Restore r7 */;\
        lwz     r8,  XR8 (r1)      /* Restore r8 */;\
        lwz     r9,  XR9 (r1)      /* Restore r9 */;\
        lwz     r10, XR10 (r1)      /* Restore r10 */;\
        lwz     r11, XR11 (r1)      /* Restore r11 */;\
        lwz     r12, XR12 (r1)      /* Restore r12 */;\
        lwz     r25, XR25 (r1)      /* Restore r25 */;\
        lwz     r26, XR26 (r1)      /* Restore r26 */;\
        lwz     r27, XR27 (r1)      /* Restore r27 */;\
        lwz     r28, XR28 (r1)      /* Restore r28 */;\
        lwz     r29, XR29 (r1)      /* Restore r29 */;\
        lwz     r30, XR30 (r1)      /* Restore r30 */;\
        lwz     r31, XR31 (r1)      /* Restore r31 */;\
        lwz     r0,  XSRR0 (r1)      /* Restore SRR0 */;\
        mtsrr0  r0;\
        lwz     r0,  XSRR1 (r1)      /* Restore SRR1 */;\
        mtsrr1  r0;\
        lwz     r0,  XR0 (r1)      /* Restore r0 */;\
        addi    r1, r1, Total_Offset/* Restore space on stack */;\
        rfi    


/*##############################################################################
#
#    ISR_Prologue        
#    ISR_Level Context protection: the next PC value is store in SRR0 
##############################################################################    */
#define ISR_Prologue NOFRALLOC_I;\
    e_stwu    r1, -Total_Offset (r1)    /* Create stack frame */;\
    stw r0,  XR0 (r1)        /* Store r0 working register  */;\
    mfsrr1  r0                /* Store SRR1 (must be done before enabling EE) */;\
    stw     r0,  XSRR1 (r1);\
    mfsrr0  r0 /* Store SRR0 (must be done before enabling EE) */;\
    stw     r0,  XSRR0 (r1);\
    stw     r31, XR31 (r1)      /* Store r31 */;\
    stw     r30, XR30 (r1)      /* Store r30 */;\
    stw     r29, XR29 (r1)      /* Store r29 */;\
    stw     r28, XR28 (r1)      /* Store r28 */;\
    stw     r27, XR27 (r1)      /* Store r27 */;\
    stw     r26, XR26 (r1)      /* Store r26 */;\
    stw     r25, XR25 (r1)      /* Store r25 */;\
    stw     r12, XR12 (r1)      /* Store r12 */;\
    stw     r11, XR11(r1)      /* Store r11 */;\
    stw     r10, XR10 (r1)      /* Store r10 */;\
    stw     r9,  XR9 (r1)      /* Store r9 */;\
    stw     r8,  XR8 (r1)      /* Store r8 */;\
    stw     r7,  XR7 (r1)      /* Store r7 */;\
    stw     r6,  XR6 (r1)      /* Store r6 */;\
    stw     r5,  XR5 (r1)      /* Store r5 */;\
    stw     r4,  XR4 (r1)      /* Store r4 */;\
    stw     r3,  XR3 (r1)    /* Store r3 */;\
    mfcr    r0                  /* Store CR */;\
    stw     r0,  XCR (r1)         ;\
    mfxer   r0                  /* Store XER */;\
    stw     r0,  XXER (r1);\
    mfctr   r0                  /* Store CTR */;\
    stw     r0,  XCTR (r1);\
    mflr    r0                /* Store LR */;\
    stw     r0,  XLR (r1)

/*##############################################################################
#
#    EXCEPTION_Prologue        
#    Restore the context from exception��the next pc value is from srr0
#
##############################################################################    */        
#define EXCEPTION_Prologue NOFRALLOC_I;\
    e_stwu    r1, -Total_Offset (r1)    /* Create stack frame */;\
    stw r0,  XR0 (r1)        /* Store r0 working register  */;\
    mfsrr1  r0                /* Store SRR1 (must be done before enabling EE) */;\
    stw     r0,  XSRR1 (r1);\
    mfsrr0  r0                /* Store SRR0 (must be done before enabling EE) */;\
    stw     r0,  XSRR0 (r1);\
    stw     r3,  XR3 (r1)      /* Store r3 */;\
    stw     r31, XR31 (r1)      /* Store r31 */;\
    stw     r30, XR30 (r1)      /* Store r30 */;\
    stw     r29, XR29 (r1)      /* Store r29 */;\
    stw     r28, XR28 (r1)      /* Store r28 */;\
    stw     r27, XR27 (r1)      /* Store r27 */;\
    stw     r26, XR26 (r1)      /* Store r26 */;\
    stw     r25, XR25 (r1)      /* Store r25 */;\
    stw     r12, XR12 (r1)      /* Store r12 */;\
    stw     r11, XR11(r1)      /* Store r11 */;\
    stw     r10, XR10 (r1)      /* Store r10 */;\
    stw     r9,  XR9 (r1)      /* Store r9 */;\
    stw     r8,  XR8 (r1)      /* Store r8 */;\
    stw     r7,  XR7 (r1)      /* Store r7 */;\
    stw     r6,  XR6 (r1)      /* Store r6 */;\
    stw     r5,  XR5 (r1)      /* Store r5 */;\
    stw     r4,  XR4 (r1)      /* Store r4 */;\
    mfcr    r0                  /* Store CR */;\
    stw     r0,  XCR (r1);\
    mfxer   r0                  /* Store XER */;\
    stw     r0,  XXER (r1);\
    mfctr   r0                  /* Store CTR */;\
    stw     r0,  XCTR (r1);\
    mflr    r0                  /* Store LR */;\
    stw     r0,  XLR (r1)
 

/*##############################################################################
#
#    ISR_Epilogue    :
#    ISR_Level Context Restore: the next PC value is store in SRR0 
#   the difference between ISR_Epilogue and TASK_Epilogue: in TASK_Epilogue, the next pc value is from LR, but in ISR_Epilogue ,the pc value is from SRR0
#   the difference between ISR_Epilogue and EXCEPTION_Epilogue: ISR_Epilogue need to record the ISR Flag.
#
##############################################################################    */        
#define ISR_Epilogue lwz     r0,  XLR (r1)      /* Restore LR */;\
        mtlr    r0;\
        lwz     r0,  XCTR (r1)      /* Restore CTR */;\
        mtctr   r0;\
        lwz     r0,  XXER (r1)      /* Restore XER */;\
        mtxer   r0;\
        lwz     r0,  XCR (r1)      /* Restore CR */;\
        mtcrf   0xff, r0;\
        lwz     r5,  XR5 (r1)      /* Restore r5 */;\
        lwz     r6,  XR6 (r1)      /* Restore r6 */;\
        lwz     r7,  XR7 (r1)      /* Restore r7 */;\
        lwz     r8,  XR8 (r1)      /* Restore r8 */;\
        lwz     r9,  XR9 (r1)      /* Restore r9 */;\
        lwz     r10, XR10 (r1)      /* Restore r10 */;\
        lwz     r11, XR11 (r1)      /* Restore r11 */;\
        lwz     r12, XR12 (r1)      /* Restore r12 */;\
/* the difference between ISR_Epilogue and EXCEPTION_Epilogue: Ensure interrupt flag has finished clearing */;\
mbar    0;\
/* Write 0 to INTC_EOIR, informing INTC to lower priority */;\
li      r3, 0;\
lis     r4, INTC_EOIR@h     /* Load upper half of INTC_EOIR address to r4 */;\
ori     r4, r4, INTC_EOIR@l;\
stw     r3, 0(r4)           /* Write 0 to INTC_EOIR */;\
        /*#Restore Working Registers */;\
        lwz     r3,  XR3 (r1)      /* Restore r3 */;\
        lwz     r4,  XR4 (r1)      /* Restore r4 */;\
        lwz     r25, XR25 (r1)      /* Restore r25 */;\
        lwz     r26, XR26 (r1)      /* Restore r26 */;\
        lwz     r27, XR27 (r1)      /* Restore r27 */;\
        lwz     r28, XR28 (r1)      /* Restore r28 */;\
        lwz     r29, XR29 (r1)      /* Restore r29 */;\
        lwz     r30, XR30 (r1)      /* Restore r30 */;\
        lwz     r31, XR31 (r1)      /* Restore r31 */;\
        lwz     r0,  XSRR0 (r1)      /* Restore SRR0 */;\
        mtsrr0  r0;\
        lwz     r0,  XSRR1 (r1)      /* Restore SRR1 */;\
        mtsrr1  r0;\
        lwz     r0,  XR0 (r1)      /* Restore r0 */;\
        addi    r1, r1, Total_Offset;\
        rfi



/*##############################################################################
#
#    EXCEPTION_Epilogue    :
#    EXCEPTION_Level Context Restore: the next PC value is store in SRR0 
#   the difference between ISR_Epilogue and TASK_Epilogue: in TASK_Epilogue, the next pc value is from LR, but in ISR_Epilogue ,the pc value is from SRR0
#   the difference between ISR_Epilogue and EXCEPTION_Epilogue: ISR_Epilogue need to record the ISR Flag.
#   do not need restore MSR, because rfi will do
##############################################################################            */
#define EXCEPTION_Epilogue lwz     r0,  XLR (r1)      /* Restore LR */;\
        mtlr    r0;\
        lwz     r0,  XCTR (r1)      /* Restore CTR */;\
        mtctr   r0;\
        lwz     r0,  XXER (r1)      /* Restore XER */;\
        mtxer   r0;\
        lwz     r0,  XCR (r1)      /* Restore CR */;\
        mtcrf   0xff, r0;\
        lwz     r3,  XR3 (r1)      /* Restore r3 */;\
        lwz     r4,  XR4 (r1)      /* Restore r4 */;\
        lwz     r5,  XR5 (r1)      /* Restore r5 */;\
        lwz     r6,  XR6 (r1)      /* Restore r6 */;\
        lwz     r7,  XR7 (r1)      /* Restore r7 */;\
        lwz     r8,  XR8 (r1)      /* Restore r8 */;\
        lwz     r9,  XR9 (r1)      /* Restore r9 */;\
        lwz     r10, XR10 (r1)      /* Restore r10 */;\
        lwz     r11, XR11 (r1)      /* Restore r11 */;\
        lwz     r12, XR12 (r1)      /* Restore r12 */;\
        lwz     r25, XR25 (r1)      /* Restore r25 */;\
        lwz     r26, XR26 (r1)      /* Restore r26 */;\
        lwz     r27, XR27 (r1)      /* Restore r27 */;\
        lwz     r28, XR28 (r1)      /* Restore r28 */;\
        lwz     r29, XR29 (r1)      /* Restore r29 */;\
        lwz     r30, XR30 (r1)      /* Restore r30 */;\
        lwz     r31, XR31 (r1)      /* Restore r31 */;\
        lwz     r0,  XSRR0 (r1)      /* Restore SRR0 */;\
        mtsrr0  r0;\
        lwz     r0,  XSRR1 (r1)      /* Restore SRR1 */;\
        mtsrr1  r0;\
        lwz     r0,  XR0 (r1)      /* Restore r0 */;\
        addi    r1, r1, Total_Offset;\
        rfi


#endif
