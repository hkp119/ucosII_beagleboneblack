#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "ucos_ii.h"
#include "os_cpu.h"
#include "os_cfg.h"
                               
#include "interrupt.h"
#include "dmtimer.h"

#define TASK_SIZE 255
#define TASKS 3
#define TIMER_INITIAL_COUNT             (0xFF000000u)
#define TIMER_RLD_COUNT                 (0xFF000000u)
#define TIMER_FINAL_COUNT               (0x0FFFFu)

/*******************************************************************************
**                      INTERNAL VARIABLE DEFINITIONS
*******************************************************************************/
volatile unsigned int tmrFlag = FALSE;
unsigned int tmrClick = FALSE;
volatile  unsigned int tmr4Flag = FALSE;

unsigned int timerCount[10] = 
						{
							0xFFF00000u,
							0xFFE00000u,
							0xFFC00000u,
							0xFFA00000u,
							0xFF800000u,
							0xFF600000u,
							0xFF400000u,
							0xFF200000u,
							0xFF000000u,
							0xFD000000u,
						};

/*******************************************************************************
**                      CREATE TASK STK 
*******************************************************************************/

OS_STK StartStk[TASK_SIZE];  
OS_STK TaskStk[TASKS][TASK_SIZE];

char TaskData[TASKS];                   // For Master Task
void TaskStart(void *pdata);            // Master Task Start 
void Task(void *pdata);                 // Others Task Start 
char cnt = 0;                
char cnt2 = 0; 
/******************************************************************************
**              INTERNAL FUNCTION PROTOTYPES
******************************************************************************/
static void TimerSetupAndEnable(void);
static void DMTimerIsr(void);
void App_Configure();

/*
** Enable all the peripherals in use
*/
static void PeripheralsSetup(void)
{
//    enableModuleClock(CLK_UART0);
//    enableModuleClock(CLK_I2C0);
    /* Timer6 is used for Standby wakeup */
//    enableModuleClock(CLK_TIMER6);
//    GPIO0ModuleClkConfig();
    DMTimer2ModuleClkConfig();          /* Timer ? */
//    DMTimer3ModuleClkConfig();
//    DMTimer4ModuleClkConfig();
//    RTCModuleClkConfig();
//    CPSWPinMuxSetup();
//    CPSWClkEnable();
//    EDMAModuleClkConfig();
    GPIO1ModuleClkConfig();             /* gpio module clk config */
    GPIO1Pin23PinMuxSetup();
//    HSMMCSDPinMuxSetup();
//    HSMMCSDModuleClkConfig();
//    I2CPinMuxSetup(0);
}


static void dummyIsr(void)
{
    ;
}
/*
** Timer 2 Interrupt Service Routine
*/
static void Timer2Isr(void)
{
	static unsigned int index = 0;
	
    /* Clear the status of the interrupt flags */
    DMTimerIntStatusClear(SOC_DMTIMER_2_REGS, DMTIMER_INT_OVF_EN_FLAG);
 
    if(TRUE == tmrClick)
    {
        tmrFlag = TRUE;
    }
    
    else 
    {
        tmrFlag = FALSE;
    }
	
    DMTimerCounterSet(SOC_DMTIMER_2_REGS, timerCount[index++%10]);
	
	DMTimerEnable(SOC_DMTIMER_2_REGS);	
}	
/* Register timer2 interrupt */
void Timer2IntRegister(void)
{
    IntRegister(SYS_INT_TINT2, Timer2Isr);
}

int main(void)
{
    UARTPuts("Start ucos Task\n"); 

    PeripheralsSetup();     

    UARTPuts("Initialize Interrupt\n");
    IntAINTCInit();                     // NEED TO Configure before using interrupt

    UARTPuts("Register the ISRs\n"); 
    Timer2IntRegister();                                                                                                                                                     //    Timer4IntRegister();
//    EnetIntRegister();
//   RtcIntRegister();
//    CM3IntRegister();
//    HSMMCSDIntRegister();
    IntRegister(127, dummyIsr);

    UARTPuts("Activate Registered IRQ");
    IntMasterIRQEnable();               // activate registed IRQ

//    pageIndex = 0;                      // ??
//    prevAction = 0;                     // ??

    UARTPuts("Enable system interrupts");      
//    IntSystemEnable(SYS_INT_RTCINT);
//    IntPrioritySet(SYS_INT_RTCINT, 0, AINTC_HOSTINT_ROUTE_IRQ);
//    IntSystemEnable(SYS_INT_3PGSWTXINT0);
//    IntPrioritySet(SYS_INT_3PGSWTXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);
//    IntSystemEnable(SYS_INT_3PGSWRXINT0);
//    IntPrioritySet(SYS_INT_3PGSWRXINT0, 0, AINTC_HOSTINT_ROUTE_IRQ);
    IntSystemEnable(SYS_INT_TINT2);
    IntPrioritySet(SYS_INT_TINT2, 0, AINTC_HOSTINT_ROUTE_IRQ);
//    IntSystemEnable(SYS_INT_TINT4);
//    IntPrioritySet(SYS_INT_TINT4, 0, AINTC_HOSTINT_ROUTE_IRQ);
//    IntSystemEnable(SYS_INT_MMCSD0INT);
//    IntPrioritySet(SYS_INT_MMCSD0INT, 0, AINTC_HOSTINT_ROUTE_IRQ);
//    IntSystemEnable(SYS_INT_EDMACOMPINT);
//    IntPrioritySet(SYS_INT_EDMACOMPINT, 0, AINTC_HOSTINT_ROUTE_IRQ);
//    IntPrioritySet(SYS_INT_M3_TXEV, 0, AINTC_HOSTINT_ROUTE_IRQ );
//    IntSystemEnable(SYS_INT_M3_TXEV);
    IntSystemEnable(127);
    IntPrioritySet(127, 0, AINTC_HOSTINT_ROUTE_IRQ);
//
//    IntSystemEnable(SYS_INT_UART0INT);
//    IntPrioritySet(SYS_INT_UART0INT, 0, AINTC_HOSTINT_ROUTE_IRQ);
//    IntRegister(SYS_INT_UART0INT, uartIsr);
   


    DEBUG_LED(0x3FFFF);

    UARTPuts("OSInit\n", -1);
    OSInit();
    UARTPuts("Create Master stack", -1);
    OSTaskCreate(TaskStart, (void *)0, &StartStk[TASK_SIZE-1], 10);
    OSStart();               

    return 0;                
}

void App_Configure()
{

// Enable IRQ in CPSR 
    intMasterIRQEnable();

// Initialize ARM interrupt controller 
    IntAINTCInit();  

    // Register Interrupt occure

// Registers App_interrupt 
    IntRegister(SYS_INT_TINT1_1MS, OSTickISR);
    //IntPrioritySet(SYS_INT_TINT1_1MS, 0, AINTC_HOSTING_ROUTE_IRQ);

    IntSystemEnable(SYS_INT_TINT1_1MS);
// 
}

void TaskStart(void *pdata)  
{ 
    int i;

    //OSStatInit(); //statistic process task

    for (i = 0; i < TASKS; i++)
    {
        UARTPuts("Create Task" );
        UARTPuts(i);
        TaskData[i] = '0' + i;
        OSTaskCreate(Task, (void *)&TaskData[i], &TaskStk[i][TASK_SIZE-1], i + 5);
    }

    for(;;)
    {
        OSTaskCtr;          // display #tasks running       
        //OSCPUUsage;         // cpu usage %
        OSCtxSwCtr;         // context switch per second    
        OSCtxSwCtr = 0;

        OSTimeDly(1);       // 1txick delay
    }
}

void Task(void *data)
{
    cnt2++;
    for(;;)
    {   
        cnt++;
        UARTPuts("Active task\n");
        OSTimeDly(1);
    }
}
            

