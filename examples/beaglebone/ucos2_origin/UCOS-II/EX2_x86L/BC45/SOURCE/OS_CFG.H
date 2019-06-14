/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-1999, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                   Configuration for Intel 80x86 (Large)
*
* File : OS_CFG.H
* By   : Jean J. Labrosse
*********************************************************************************************************
*/

/*
*********************************************************************************************************
*                                         uC/OS-II CONFIGURATION
*********************************************************************************************************
*/

#define OS_MAX_EVENTS            20    /* Max. number of event control blocks in your application ...  */
                                       /* ... MUST be >= 2                                             */
#define OS_MAX_MEM_PART          10    /* Max. number of memory partitions ...                         */
                                       /* ... MUST be >= 2                                             */
#define OS_MAX_QS                 5    /* Max. number of queue control blocks in your application ...  */
                                       /* ... MUST be >= 2                                             */
#define OS_MAX_TASKS             32    /* Max. number of tasks in your application ...                 */
                                       /* ... MUST be >= 2                                             */

#define OS_LOWEST_PRIO           63    /* Defines the lowest priority that can be assigned ...         */
                                       /* ... MUST NEVER be higher than 63!                            */

#define OS_TASK_IDLE_STK_SIZE   512    /* Idle task stack size (# of 16-bit wide entries)              */

#define OS_TASK_STAT_EN           1    /* Enable (1) or Disable(0) the statistics task                 */
#define OS_TASK_STAT_STK_SIZE   512    /* Statistics task stack size (# of 16-bit wide entries)        */

#define OS_ARG_CHK_EN             1    /* Enable (1) or Disable (0) argument checking                  */
#define OS_CPU_HOOKS_EN           1    /* uC/OS-II hooks are found in the processor port files         */
#define OS_MBOX_EN                1    /* Include code for MAILBOXES                                   */
#define OS_MBOX_DEL_EN            0    /* Include code to  delete MAILBOXES                            */
#define OS_MEM_EN                 0    /* Include code for MEMORY MANAGER (fixed sized memory blocks)  */
#define OS_MUTEX_EN               0    /* Include code for MUTUAL EXCLUSION SEMAPHORES                 */
#define OS_MUTEX_DEL_EN           0    /* Include code to  delete MUTUAL EXCLUSION SEMAPHORES          */
#define OS_Q_EN                   0    /* Include code for QUEUES                                      */
#define OS_Q_DEL_EN               0    /* Include code to  delete MESSAGE QUEUES                       */
#define OS_SEM_EN                 0    /* Include code for SEMAPHORES                                  */
#define OS_SEM_DEL_EN             0    /* Include code to  delete SEMAPHORES                           */
#define OS_TASK_CHANGE_PRIO_EN    0    /* Include code for OSTaskChangePrio()                          */
#define OS_TASK_CREATE_EN         0    /* Include code for OSTaskCreate()                              */
#define OS_TASK_CREATE_EXT_EN     1    /* Include code for OSTaskCreateExt()                           */
#define OS_TASK_DEL_EN            0    /* Include code for OSTaskDel()                                 */
#define OS_TASK_SUSPEND_EN        0    /* Include code for OSTaskSuspend() and OSTaskResume()          */

#define OS_TICKS_PER_SEC        200    /* Set the number of ticks in one second                        */

