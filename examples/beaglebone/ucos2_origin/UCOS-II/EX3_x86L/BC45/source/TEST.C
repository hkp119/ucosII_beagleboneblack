/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                        (c) Copyright 1992-1998, Jean J. Labrosse, Plantation, FL
*                                           All Rights Reserved
*
*                                                 V2.00
*
*                                               EXAMPLE #3
*********************************************************************************************************
*/

#define  EX3_GLOBALS
#include "includes.h"

/*
*********************************************************************************************************
*                                              CONSTANTS
*********************************************************************************************************
*/

#define          TASK_STK_SIZE     512                /* Size of each task's stacks (# of WORDs)       */

#define          TASK_START_ID       0                /* Application tasks                             */
#define          TASK_CLK_ID         1
#define          TASK_1_ID           2
#define          TASK_2_ID           3
#define          TASK_3_ID           4
#define          TASK_4_ID           5
#define          TASK_5_ID           6

#define          TASK_START_PRIO    10                /* Application tasks priorities                  */
#define          TASK_CLK_PRIO      11
#define          TASK_1_PRIO        12
#define          TASK_2_PRIO        13
#define          TASK_3_PRIO        14
#define          TASK_4_PRIO        15
#define          TASK_5_PRIO        16

#define          MSG_QUEUE_SIZE     20                /* Size of message queue used in example         */

/*$PAGE*/
/*
*********************************************************************************************************
*                                              VARIABLES
*********************************************************************************************************
*/

OS_STK           TaskStartStk[TASK_STK_SIZE];         /* Startup    task stack                         */
OS_STK           TaskClkStk[TASK_STK_SIZE];           /* Clock      task stack                         */
OS_STK           Task1Stk[TASK_STK_SIZE];             /* Task #1    task stack                         */
OS_STK           Task2Stk[TASK_STK_SIZE];             /* Task #2    task stack                         */
OS_STK           Task3Stk[TASK_STK_SIZE];             /* Task #3    task stack                         */
OS_STK           Task4Stk[TASK_STK_SIZE];             /* Task #4    task stack                         */
OS_STK           Task5Stk[TASK_STK_SIZE];             /* Task #5    task stack                         */

OS_EVENT        *MsgQueue;                            /* Message queue pointer                         */
void            *MsgQueueTbl[20];                     /* Storage for messages                          */

/*
*********************************************************************************************************
*                                         FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void             TaskStart(void *data);               /* Function prototypes of tasks                  */
void             TaskStartCreateTasks(void);
void             TaskClk(void *data);
void             Task1(void *data);
void             Task2(void *data);
void             Task3(void *data);
void             Task4(void *data);
void             Task5(void *data);

/*$PAGE*/
/*
*********************************************************************************************************
*                                                  MAIN
*********************************************************************************************************
*/

void main (void)
{
    PC_DispClrScr(DISP_BGND_BLACK);                        /* Clear the screen                         */

    OSInit();                                              /* Initialize uC/OS-II                      */

    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */

    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */

    PC_ElapsedInit();                                      /* Initialized elapsed time measurement     */

    strcpy(TaskUserData[TASK_START_ID].TaskName, "StartTask");
    OSTaskCreateExt(TaskStart,
                    (void *)0,
                    &TaskStartStk[TASK_STK_SIZE - 1],
                    TASK_START_PRIO,
                    TASK_START_ID,
                    &TaskStartStk[0],
                    TASK_STK_SIZE,
                    &TaskUserData[TASK_START_ID],
                    0);
    OSStart();                                             /* Start multitasking                       */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                               STARTUP TASK
*********************************************************************************************************
*/

void  TaskStart (void *data)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif
    char       s[80];
    INT16S     key;


    data = data;                                           /* Prevent compiler warning                 */
    PC_DispStr(26,  0, "uC/OS-II, The Real-Time Kernel", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
    PC_DispStr(33,  1, "Jean J. Labrosse", DISP_FGND_WHITE);
    PC_DispStr(36,  3, "EXAMPLE #3", DISP_FGND_WHITE);
    PC_DispStr(0,  9, "Task Name         Counter  Exec.Time(uS)   Tot.Exec.Time(uS)  %Tot.", DISP_FGND_WHITE);
    PC_DispStr(0, 10, "----------------- -------  -------------   -----------------  -----", DISP_FGND_WHITE);
    PC_DispStr(0, 22, "Determining  CPU's capacity ...", DISP_FGND_WHITE);
    PC_DispStr(28, 24, "<-PRESS 'ESC' TO QUIT->", DISP_FGND_WHITE + DISP_BLINK);

    OS_ENTER_CRITICAL();                                   /* Install uC/OS-II's clock tick ISR        */
    PC_VectSet(0x08, OSTickISR);
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    OSStatInit();

    MsgQueue = OSQCreate(&MsgQueueTbl[0], MSG_QUEUE_SIZE); /* Create a message queue                   */

    TaskStartCreateTasks();

    PC_DispStr( 0, 22, "#Tasks          : xxxxx  CPU Usage: xxx %", DISP_FGND_WHITE);
    PC_DispStr( 0, 23, "#Task switch/sec: xxxxx", DISP_FGND_WHITE);
    for (;;) {
        sprintf(s, "%5d", OSTaskCtr);                      /* Display #tasks running                   */
        PC_DispStr(18, 22, s, DISP_FGND_BLUE + DISP_BGND_CYAN);
        sprintf(s, "%3d", OSCPUUsage);                     /* Display CPU usage in %                   */
        PC_DispStr(36, 22, s, DISP_FGND_BLUE + DISP_BGND_CYAN);
        sprintf(s, "%5d", OSCtxSwCtr);                     /* Display #context switches per second     */
        PC_DispStr(18, 23, s, DISP_FGND_BLUE + DISP_BGND_CYAN);
        sprintf(s, "V%1d.%02d", OSVersion() / 100, OSVersion() % 100);
        PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);

        OSCtxSwCtr = 0;                                    /* Clear the context switch counter         */

        if (PC_GetKey(&key)) {                             /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Yes, return to DOS                       */
            }
        }

        OSTimeDly(OS_TICKS_PER_SEC);                       /* Wait one second                          */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                      CREATE APPLICATION TASKS
*********************************************************************************************************
*/

void  TaskStartCreateTasks (void)
{
    strcpy(TaskUserData[TASK_CLK_ID].TaskName, "Clock Task");
    OSTaskCreateExt(TaskClk,
                    (void *)0,
                    &TaskClkStk[TASK_STK_SIZE - 1],
                    TASK_CLK_PRIO,
                    TASK_CLK_ID,
                    &TaskClkStk[0],
                    TASK_STK_SIZE,
                    &TaskUserData[TASK_CLK_ID],
                    0);

    strcpy(TaskUserData[TASK_1_ID].TaskName, "MsgQ Tx Task");
    OSTaskCreateExt(Task1,
                    (void *)0,
                    &Task1Stk[TASK_STK_SIZE - 1],
                    TASK_1_PRIO,
                    TASK_1_ID,
                    &Task1Stk[0],
                    TASK_STK_SIZE,
                    &TaskUserData[TASK_1_ID],
                    0);

    strcpy(TaskUserData[TASK_2_ID].TaskName, "MsgQ Rx Task #1");
    OSTaskCreateExt(Task2,
                    (void *)0,
                    &Task2Stk[TASK_STK_SIZE - 1],
                    TASK_2_PRIO,
                    TASK_2_ID,
                    &Task2Stk[0],
                    TASK_STK_SIZE,
                    &TaskUserData[TASK_2_ID],
                    0);

    strcpy(TaskUserData[TASK_3_ID].TaskName, "MsgQ Rx Task #2");
    OSTaskCreateExt(Task3,
                    (void *)0,
                    &Task3Stk[TASK_STK_SIZE - 1],
                    TASK_3_PRIO,
                    TASK_3_ID,
                    &Task3Stk[0],
                    TASK_STK_SIZE,
                    &TaskUserData[TASK_3_ID],
                    0);

    strcpy(TaskUserData[TASK_4_ID].TaskName, "MboxPostPendTask");
    OSTaskCreateExt(Task4,
                    (void *)0,
                    &Task4Stk[TASK_STK_SIZE - 1],
                    TASK_4_PRIO,
                    TASK_4_ID,
                    &Task4Stk[0],
                    TASK_STK_SIZE,
                    &TaskUserData[TASK_4_ID],
                    0);

    strcpy(TaskUserData[TASK_5_ID].TaskName, "TimeDlyTask");
    OSTaskCreateExt(Task5,
                    (void *)0,
                    &Task5Stk[TASK_STK_SIZE - 1],
                    TASK_5_PRIO,
                    TASK_5_ID,
                    &Task5Stk[0],
                    TASK_STK_SIZE,
                    &TaskUserData[TASK_5_ID],
                    0);
}
/*$PAGE*/

/*
*********************************************************************************************************
*                                               TASK #1
*********************************************************************************************************
*/

void  Task1 (void *data)
{
    char one   = '1';
    char two   = '2';
    char three = '3';


    data = data;
    for (;;) {
        OSQPost(MsgQueue, (void *)&one);
        OSTimeDlyHMSM(0, 0, 1,   0);        /* Delay for 1 second                                      */
        OSQPost(MsgQueue, (void *)&two);
        OSTimeDlyHMSM(0, 0, 0, 500);        /* Delay for 500 mS                                        */
        OSQPost(MsgQueue, (void *)&three);
        OSTimeDlyHMSM(0, 0, 1,   0);        /* Delay for 1 second                                      */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #2
*
* Description: This task waits for messages sent by task #1.
*********************************************************************************************************
*/

void  Task2 (void *data)
{
    INT8U *msg;
    INT8U  err;


    data = data;
    for (;;) {
        msg = (INT8U *)OSQPend(MsgQueue, 0, &err);    /* Wait forever for message                      */
        PC_DispChar(70, 14, *msg, DISP_FGND_YELLOW + DISP_BGND_BLUE);
        OSTimeDlyHMSM(0, 0, 0, 500);                  /* Delay for 500 mS                              */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #3
*
* Description: This task waits for up to 250 mS for a message sent by task #1.
*********************************************************************************************************
*/

void  Task3 (void *data)
{
    INT8U *msg;
    INT8U  err;


    data = data;
    for (;;) {
        msg = (INT8U *)OSQPend(MsgQueue, OS_TICKS_PER_SEC / 4, &err);  /* Wait up to 250 mS for a msg  */
        if (err == OS_TIMEOUT) {
            PC_DispChar(70, 15, 'T',  DISP_FGND_YELLOW + DISP_BGND_RED);
        } else {
            PC_DispChar(70, 15, *msg, DISP_FGND_YELLOW + DISP_BGND_BLUE);
        }
    }
}

/*
*********************************************************************************************************
*                                               TASK #4
*
* Description: This task posts a message to a mailbox and then immediately reads the message.
*********************************************************************************************************
*/

void  Task4 (void *data)
{
    OS_EVENT *mbox;
    INT8U     err;


    data = data;
    mbox = OSMboxCreate((void *)0);
    for (;;) {
        OSMboxPost(mbox, (void *)1);             /* Send message to mailbox                            */
        OSMboxPend(mbox, 0, &err);               /* Get message from mailbox                           */
        OSTimeDlyHMSM(0, 0, 0, 10);              /* Delay 10 mS                                        */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               TASK #5
*
* Description: This task simply delays itself.  We basically want to determine how long OSTimeDly() takes
*              to execute.
*********************************************************************************************************
*/

void  Task5 (void *data)
{
    data = data;
    for (;;) {
        OSTimeDly(1);
    }
}

/*
*********************************************************************************************************
*                                      DISPLAY TASK RELATED STATISTICS
*********************************************************************************************************
*/

void DispTaskStat (INT8U id)
{
    char s[80];


    sprintf(s, "%-18s %05u      %5u          %10ld",
            TaskUserData[id].TaskName,
            TaskUserData[id].TaskCtr,
            TaskUserData[id].TaskExecTime,
            TaskUserData[id].TaskTotExecTime);
    PC_DispStr(0, id + 11, s, DISP_FGND_LIGHT_GRAY);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                               CLOCK TASK
*********************************************************************************************************
*/

void  TaskClk (void *data)
{
    char  s[40];


    data = data;
    for (;;) {
        PC_GetDateTime(s);
        PC_DispStr(0, 24, s, DISP_FGND_BLUE + DISP_BGND_CYAN);
        OSTimeDlyHMSM(0, 0, 0, 100);             /* Execute every 100 mS                               */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                            (BEGINNING)
*
* Description: This function is called by OSInit() at the beginning of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSInitHookBegin (void)
{
}
#endif


/*
*********************************************************************************************************
*                                       OS INITIALIZATION HOOK
*                                               (END)
*
* Description: This function is called by OSInit() at the end of OSInit().
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts should be disabled during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSInitHookEnd (void)
{
}
#endif

/*
*********************************************************************************************************
*                                           TASK CREATION HOOK
*
* Description: This function is called when a task is created.
*
* Arguments  : ptcb    is a pointer to the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskCreateHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}


/*
*********************************************************************************************************
*                                           TASK DELETION HOOK
*
* Description: This function is called when a task is deleted.
*
* Arguments  : ptcb    is a pointer to the task being created.
*
* Note(s)    : 1) Interrupts are disabled during this call.
*********************************************************************************************************
*/
void OSTaskDelHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                       /* Prevent compiler warning                                     */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                           TASK SWITCH HOOK
*
* Description: This function is called when a task switch is performed.  This allows you to perform other
*              operations during a context switch.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts are disabled during this call.
*              2) It is assumed that the global pointer 'OSTCBHighRdy' points to the TCB of the task that
*                 will be 'switched in' (i.e. the highest priority task) and, 'OSTCBCur' points to the task
*                 being switched out (i.e. the preempted task).
*********************************************************************************************************
*/
void OSTaskSwHook (void)
{
    INT16U          time;
    TASK_USER_DATA *puser;


    time  = PC_ElapsedStop();                    /* This task is done                                  */
    PC_ElapsedStart();                           /* Start for next task                                */
    puser = OSTCBCur->OSTCBExtPtr;               /* Point to used data                                 */
    if (puser != (void *)0) {
        puser->TaskCtr++;                        /* Increment task counter                             */
        puser->TaskExecTime     = time;          /* Update the task's execution time                   */
        puser->TaskTotExecTime += time;          /* Update the task's total execution time             */
    }
}

/*
*********************************************************************************************************
*                                          STATISTIC TASK HOOK
*
* Description: This function is called every second by uC/OS-II's statistics task.  This allows your
*              application to add functionality to the statistics task.
*
* Arguments  : none
*********************************************************************************************************
*/
void OSTaskStatHook (void)
{
    char   s[80];
    INT8U  i;
    INT32U total;
    INT8U  pct;


    total = 0L;                                          /* Totalize TOT. EXEC. TIME for each task */
    for (i = 0; i < 7; i++) {
        total += TaskUserData[i].TaskTotExecTime;
        DispTaskStat(i);                                 /* Display task data                      */
    }
    if (total > 0) {
        for (i = 0; i < 7; i++) {                        /* Derive percentage of each task         */
            pct = 100 * TaskUserData[i].TaskTotExecTime / total;
            sprintf(s, "%3d %%", pct);
            PC_DispStr(62, i + 11, s, DISP_FGND_YELLOW);
        }
    }
    if (total > 1000000000L) {                           /* Reset total time counters at 1 billion */
        for (i = 0; i < 7; i++) {
            TaskUserData[i].TaskTotExecTime = 0L;
        }
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                TICK HOOK
*
* Description: This function is called every tick.
*
* Arguments  : none
*
* Note(s)    : 1) Interrupts may or may not be are ENABLED during this call.
*********************************************************************************************************
*/
void OSTimeTickHook (void)
{
}


/*
*********************************************************************************************************
*                                           OSTCBInit() HOOK
*
* Description: This function is called by OSTCBInit() after setting up most of the TCB.
*
* Arguments  : ptcb    is a pointer to the TCB of the task being created.
*
* Note(s)    : 1) Interrupts may or may not be ENABLED during this call.
*********************************************************************************************************
*/
#if OS_VERSION > 203
void OSTCBInitHook (OS_TCB *ptcb)
{
    ptcb = ptcb;                                           /* Prevent Compiler warning                 */
}
#endif
