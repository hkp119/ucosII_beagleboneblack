/*
*********************************************************************************************************
*                                                uC/OS-II
*                                          The Real-Time Kernel
*
*                           (c) Copyright 1992-1999, Jean J. Labrosse, Weston, FL
*                                           All Rights Reserved
*
*                                                 V2.00
*
*                                               EXAMPLE #1
*********************************************************************************************************
*/

#include "includes.h"

/*
*********************************************************************************************************
*                                               CONSTANTS
*********************************************************************************************************
*/

#define  TASK_STK_SIZE                 512       /* Size of each task's stacks (# of WORDs)            */
#define  N_TASKS                        10       /* Number of identical tasks                          */

/*
*********************************************************************************************************
*                                               VARIABLES
*********************************************************************************************************
*/

OS_STK           TaskStk[N_TASKS][TASK_STK_SIZE];     /* Tasks stacks                                  */
OS_STK           TaskStartStk[TASK_STK_SIZE];
char             TaskData[N_TASKS];                   /* Parameters to pass to each task               */
OS_EVENT        *RandomSem;

INT16U           TaskFPUFlag;

/*
*********************************************************************************************************
*                                           FUNCTION PROTOTYPES
*********************************************************************************************************
*/

void   Task(void *data);                              /* Function prototypes of tasks                  */
void   TaskStart(void *data);                         /* Function prototypes of Startup task           */

/*$PAGE*/
/*
*********************************************************************************************************
*                                                MAIN
*********************************************************************************************************
*/

void main (void)
{
    PC_DispClrScr(DISP_FGND_WHITE + DISP_BGND_BLACK);      /* Clear the screen                         */
    OSInit();                                              /* Initialize uC/OS-II                      */
    PC_DOSSaveReturn();                                    /* Save environment to return to DOS        */
    PC_VectSet(uCOS, OSCtxSw);                             /* Install uC/OS-II's context switch vector */
    RandomSem   = OSSemCreate(1);                          /* Random number semaphore                  */
    TaskFPUFlag = _8087;
    OSTaskCreate(TaskStart, (void *)0, &TaskStartStk[TASK_STK_SIZE - 1], 0);
    OSStart();                                             /* Start multitasking                       */
}

/*$PAGE*/
/*
*********************************************************************************************************
*                                              STARTUP TASK
*********************************************************************************************************
*/
void TaskStart (void *data)
{
#if OS_CRITICAL_METHOD == 3                      /* Allocate storage for CPU status register           */
    OS_CPU_SR  cpu_sr;
#endif
    UBYTE      i;
    char       s[100];
    WORD       key;


    data = data;                                           /* Prevent compiler warning                 */

    PC_DispStr(26,  0, "uC/OS-II, The Real-Time Kernel", DISP_FGND_WHITE + DISP_BGND_RED + DISP_BLINK);
    PC_DispStr(33,  1, "Jean J. Labrosse", DISP_FGND_WHITE);
    PC_DispStr(36,  3, "EXAMPLE #1", DISP_FGND_WHITE);

    OS_ENTER_CRITICAL();
    PC_VectSet(0x08, OSTickISR);                           /* Install uC/OS-II's clock tick ISR        */
    PC_SetTickRate(OS_TICKS_PER_SEC);                      /* Reprogram tick rate                      */
    OS_EXIT_CRITICAL();

    PC_DispStr(0, 22, "Determining  CPU's capacity ...", DISP_FGND_WHITE);
    OSStatInit();                                          /* Initialize uC/OS-II's statistics         */
    PC_DispClrRow(22, DISP_FGND_WHITE + DISP_BGND_BLACK);

    for (i = 0; i < N_TASKS; i++) {                        /* Create N_TASKS identical tasks           */
        TaskData[i] = '0' + i;                             /* Each task will display its own letter    */
        OSTaskCreate(Task, (void *)&TaskData[i], &TaskStk[i][TASK_STK_SIZE - 1], i + 1);
    }
    PC_DispStr( 0, 22, "#Tasks          : xxxxx  CPU Usage: xxx %", DISP_FGND_WHITE);
    PC_DispStr( 0, 23, "#Task switch/sec: xxxxx", DISP_FGND_WHITE);
    PC_DispStr(28, 24, "<-PRESS 'ESC' TO QUIT->", DISP_FGND_WHITE + DISP_BLINK);
    for (;;) {
        sprintf(s, "%5d", OSTaskCtr);                     /* Display #tasks running                    */
        PC_DispStr(18, 22, s, DISP_FGND_BLUE + DISP_BGND_CYAN);
#if OS_TASK_STAT_EN
        sprintf(s, "%3d", OSCPUUsage);                    /* Display CPU usage in %                    */
        PC_DispStr(36, 22, s, DISP_FGND_BLUE + DISP_BGND_CYAN);
#endif
        sprintf(s, "%5d", OSCtxSwCtr);                    /* Display #context switches per second      */
        PC_DispStr(18, 23, s, DISP_FGND_BLUE + DISP_BGND_CYAN);
        OSCtxSwCtr = 0;

        sprintf(s, "V%1d.%02d", OSVersion() / 100, OSVersion() % 100);
        PC_DispStr(75, 24, s, DISP_FGND_YELLOW + DISP_BGND_BLUE);
        PC_GetDateTime(s);                                 /* Get and display date and time            */
        PC_DispStr(0, 24, s, DISP_FGND_BLUE + DISP_BGND_CYAN);

        switch (TaskFPUFlag) {
            case 0:
                 PC_DispStr(71, 22, " NO  FPU ", DISP_FGND_BLUE + DISP_BGND_CYAN);
                 break;

            case 1:
                 PC_DispStr(71, 22, " 8087 FPU", DISP_FGND_BLUE + DISP_BGND_CYAN);
                 break;

            case 2:
                 PC_DispStr(71, 22, "80287 FPU", DISP_FGND_BLUE + DISP_BGND_CYAN);
                 break;

            case 3:
                 PC_DispStr(71, 22, "80387 FPU", DISP_FGND_BLUE + DISP_BGND_CYAN);
                 break;
        }

        if (PC_GetKey(&key) == TRUE) {                     /* See if key has been pressed              */
            if (key == 0x1B) {                             /* Yes, see if it's the ESCAPE key          */
                PC_DOSReturn();                            /* Return to DOS                            */
            }
        }

        OSTimeDlyHMSM(0, 0, 1, 0);                         /* Wait one second                          */
    }
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                                  TASKS
*********************************************************************************************************
*/

void Task (void *data)
{
    UBYTE x;
    UBYTE y;
    UBYTE err;


    for (;;) {
        OSSemPend(RandomSem, 0, &err);           /* Acquire semaphore to perform random numbers        */
        x = random(80);                          /* Find X position where task number will appear      */
        y = random(16);                          /* Find Y position where task number will appear      */
        OSSemPost(RandomSem);                    /* Release semaphore                                  */
                                                 /* Display the task number on the screen              */
        PC_DispChar(x, y + 5, *(char *)data, DISP_FGND_LIGHT_GRAY);
        OSTimeDly(1);                            /* Delay 1 clock tick                                 */
    }
}
