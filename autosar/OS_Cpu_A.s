    .syntax        unified
    .arch        armv7-m
    .cpu        cortex-m4
    .thumb


/* ---------------------------------------------------- */

 .global OSTCBCur
 .global OSTCBOld

 .global FirstTaskRun
 .global TaskSaveLoadRun
 .global TaskSaveRun
 .global TaskLoadRun
 .global TaskRun
 .global PendSV_Handler
 .global CPU_SR_Save
 .global CPU_SR_Restore
 .type PendSV_Handler, %function
 .type CPU_SR_Save, %function
 .type CPU_SR_Restore, %function

 .extern OSTCBStkCur
 .extern SystemSTKTopPtr
 .extern OSTCBCur

 .extern LPIT_IRQHandler



.equ NVIC_INT_CTRL,        0xE000ED04
.equ NVIC_SYSPRI14,        0xE000ED22
.equ NVIC_PENDSV_PRI,    0xFF
.equ NVIC_PENDSVSET,    0x10000000

TaskSaveLoadRun:
    LDR     R2, =NVIC_INT_CTRL
    LDR     R3, =NVIC_PENDSVSET
    STR     R3, [R2]

    BX      LR

TaskSaveRun:
    LDR     R2, =NVIC_INT_CTRL
    LDR     R3, =NVIC_PENDSVSET
    STR     R3, [R2]

    BX      LR

TaskLoadRun:
    LDR     R2, =OSTCBOld
    MOVS     R3, #0
    STR        R3, [R2]

    LDR     R2, =NVIC_INT_CTRL
    LDR     R3, =NVIC_PENDSVSET
    STR     R3, [R2]

    BX        LR

TaskRun:
    LDR     R2, =OSTCBOld
    MOVS     R3, #0
    STR        R3, [R2]

    LDR     R2, =NVIC_INT_CTRL
    LDR     R3, =NVIC_PENDSVSET
    STR     R3, [R2]

    BX        LR

FirstTaskRun:
    CPSID   I
    LDR     R2, =NVIC_SYSPRI14
    LDR     R3, =NVIC_PENDSV_PRI
    STRB    R3, [R2]

    MOVS    R2, #0
    MSR        PSP, R2

    LDR     R2, =SystemSTKTopPtr
    LDR     R3, [R2]
       MSR     MSP, R3

    LDR     R2, =NVIC_INT_CTRL
    LDR     R3, =NVIC_PENDSVSET
    STR     R3, [R2]

    CPSIE   I

OSStartHang:
    B       OSStartHang


PendSV_Handler:
    CPSID   I

    LDR     R0, =OSTCBOld
    LDR        R0, [R0]

    MRS     R2, PSP
    CBZ     R2, PendSVHandler_nosave
    CBZ        R0, PendSVHandler_nosave

    SUBS    R2, R2, #0x20
    STM     R2, {R4-R11}

    STR     R2, [R0]


PendSVHandler_nosave:
    LDR        R2, =OSTCBCur
    LDR        R2, [R2]
    LDR        R2, [R2]
    LDM     R2, {R4-R11}
    ADDS    R2, R2, #0x20

    MSR     PSP, R2
    LDR        R2, =0xFFFFFFFD
    CPSIE   I
    BX      R2

CPU_SR_Save:
    MRS        R0, PRIMASK
    CPSID    I
    BX        LR

CPU_SR_Restore:
    MSR        PRIMASK, R0
    BX        LR

