/*
*********************************************************************************
* Copyright (c) 2011,Embedded System Engineering center of  Zhejiang University
* All rights reserved.
*
* FILE NAME: Platform_Lib.h
* DESCRIPTION��CPU related code
* UPDATE HISTORY
* REV      AUTHOR      DATE       DESCRIPTION OF CHANGE                         
* 1.0      JACK YAO    07/MAR/11   Initial version.   
*
*********************************************************************************
*/

#ifndef CRITICAL_H
#define CRITICAL_H
   
#define DEFINECRITICAL() static unsigned int cpu_sr=0U

/*Save MSR, and Disable Interrupts, enter critical region*/
#define ENTER_CRITICAL() cpu_sr = CPU_SR_Save()

/*Restor MSR, and enable interrupts, exit critical region*/ 
#define EXIT_CRITICAL()  CPU_SR_Restore(cpu_sr)
    
   

#endif
