/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                          (c) Copyright 1992-2000, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
* File : uCOS_II.C
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

#define  OS_GLOBALS                           /* Declare GLOBAL variables                              */
#include "../UCOS-INCLUDE/INCLUDES.H"

#define  OS_MASTER_FILE                       /* Prevent the following files from including includes.h */
#include "./ETC/OS_CORE.C"
#include "./ETC/OS_MBOX.C"
#include "./ETC/OS_MEM.C"
#include "./ETC/OS_MUTEX.C"
#include "./ETC/OS_Q.C"
#include "./ETC/OS_SEM.C"
#include "./ETC/OS_TASK.C"
#include "./ETC/OS_TIME.C"
