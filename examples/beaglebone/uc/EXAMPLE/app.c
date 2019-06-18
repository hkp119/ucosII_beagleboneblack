/* Standard C Header Files */
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include <stdio.h>
/* uc-os ii Header Files */
#include"ucos_ii.h"
#include"os_cpu.h"
#include"os_cfg.h"
/* Starterware Header Files */
#include "consoleUtils.h"
#include "soc_AM335x.h"
#include "beaglebone.h"
#include "interrupt.h"
#include "dmtimer.h"
#include "error.h"
/******************************************************************************
**                      INTERNAL MACRO DEFINITIONS
*******************************************************************************/
#define TIMER_INITIAL_COUNT             (0xFF000000u)
#define TIMER_RLD_COUNT                 (0xFF000000u)
/******************************************************************************
**                      INTERNAL FUNCTION PROTOTYPES
*******************************************************************************/
static void DMTimerAintcConfigure(void);
static void DMTimerSetUp(void);
static void DMTimerIsr(void);

void MyTask(void *pdata);
void Task1(void *pdata);
void Task2(void *pdata);
void Task3(void *pdata);
/******************************************************************************
**                      INTERNAL VARIABLE DEFINITIONS
*******************************************************************************/
static volatile unsigned int cntValue = 10;
static volatile unsigned int flagIsr = 0;

OS_STK MyTaskStack[200];
OS_STK TaskStack[3][200];
int cnt;

int main(void)
{
    ConsoleUtilsPrintf("Start Main\n");
    cnt = 0;
    ConsoleUtilsPrintf("Start OSInit\n");
    OSInit();
    ConsoleUtilsPrintf("Start Master Task Create\n");
    OSTaskCreate(MyTask, (void *)0, &MyTaskStack[199], 1);
    ConsoleUtilsPrintf("Start MultiTasking\n");
    OSStart();
    return 0;
}

void MyTask(void *pdata)
{
    ConsoleUtilsPrintf("MyTask Start point\n");
    /********************* Timer and Interrupt initialization ***********************/
    /* This function will enable clocks for the DMTimer2 instance */
    ConsoleUtilsPrintf("......\n");
    DMTimer2ModuleClkConfig();
    /* Enable IRQ in CPSR */
    ConsoleUtilsPrintf("......\n");
    IntMasterIRQEnable();
    /* Register DMTimer2 interrupts on to AINTC */
    ConsoleUtilsPrintf("......\n");
    DMTimerAintcConfigure();
    /* Perform the necessary configurations for DMTimer */
    ConsoleUtilsPrintf("......\n");
    DMTimerSetUp();
    /* Enable the DMTimer interrupts */
    ConsoleUtilsPrintf("......\n");
    DMTimerIntEnable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);
    /* Start the DMTimer */
    ConsoleUtilsPrintf("......\n");
    DMTimerEnable(SOC_DMTIMER_2_REGS);  
    /**************************** Create Tasks ***********************************/
    ConsoleUtilsPrintf("crtsk1.\n");
    OSTaskCreate(Task1, (void *)0, &TaskStack[0][199], 2);
    ConsoleUtilsPrintf("crts2.\n");
    OSTaskCreate(Task2, (void *)0, &TaskStack[1][199], 3);
    ConsoleUtilsPrintf("crtsk3.\n");
    OSTaskCreate(Task3, (void *)0, &TaskStack[2][199], 4);

    for(;;) {
        ConsoleUtilsPrintf("into loop.\n");
        OSTimeDly(1000);
    }
}

void Task1(void *pdata)
{
    ConsoleUtilsPrintf("Task 1 Start point\n");
    for(;;) {
        ConsoleUtilsPrintf("Task 1 Loop Start\n");
        cnt++;
        OSTimeDly(3);
    }
}

void Task2(void *pdata)
{
    ConsoleUtilsPrintf("Task 2 Start point\n");
    for(;;) {
        ConsoleUtilsPrintf("Task 2 Loop Start\n");
        cnt++;
        OSTimeDly(3);
    }
}

void Task3(void *pdata)
{
    ConsoleUtilsPrintf("Task 3 Start point\n");
    for(;;) {
        ConsoleUtilsPrintf("Task 3 Loop Start\n");
        cnt++;
        OSTimeDly(3);
    }
}

static void DMTimerAintcConfigure(void)
{
    /* Initialize the ARM interrupt control */
    IntAINTCInit();
    /* Registering DMTimerIsr */
    IntRegister(SYS_INT_TINT2, DMTimerIsr);
    /* Set the priority */
    IntPrioritySet(SYS_INT_TINT2, 0, AINTC_HOSTINT_ROUTE_IRQ);
    /* Enable the system interrupt */
    IntSystemEnable(SYS_INT_TINT2);
}
/*
** Setup the timer for one-shot and compare mode.
*/
static void DMTimerSetUp(void)
{
    /* Load the counter with the initial count value */
    DMTimerCounterSet(SOC_DMTIMER_2_REGS, TIMER_INITIAL_COUNT);
    /* Load the load register with the reload count value */
    DMTimerReloadSet(SOC_DMTIMER_2_REGS, TIMER_RLD_COUNT);
    /* Configure the DMTimer for Auto-reload and compare mode */
    DMTimerModeConfigure(SOC_DMTIMER_2_REGS, DMTIMER_AUTORLD_NOCMP_ENABLE);
}
/*
** DMTimer interrupt service routine. This will send a character to serial 
** console.
*/    
static void DMTimerIsr(void)
{
    /* Disable the DMTimer interrupts */
    DMTimerIntDisable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);
    /* Clear the status of the interrupt flags */
    DMTimerIntStatusClear(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_IT_FLAG);

    flagIsr = 1;

    /* Enable the DMTimer interrupts */
    DMTimerIntEnable(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);
    ConsoleUtilsPrintf("!!!!\n");
}
