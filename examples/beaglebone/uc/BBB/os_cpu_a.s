@
@********************************************************************************************************
@                                               uC/OS-II
@                                         The Real-Time Kernel
@
@
@                             (c) Copyright 1992-2007, Micrium, Weston, FL
@                                          All Rights Reserved
@
@                                           Generic ARM Port
@
@ File      : OS_CPU_A.ASM
@ Version   : V1.82
@ By        : Jean J. Labrosse
@             Jean-Denis Hatier
@
@ For       : ARM7 or ARM9
@ Mode      : ARM or Thumb
@ Toolchain : GNU GCC
@********************************************************************************************************
@

@********************************************************************************************************
@                                           PUBLIC FUNCTIONS
@********************************************************************************************************
                                                                @ External references.
    .extern  OSRunning
    .extern  OSPrioCur
    .extern  OSPrioHighRdy
    .extern  OSTCBCur
    .extern  OSTCBHighRdy
    .extern  OSIntNesting
    .extern  OSIntExit
    .extern  OSTaskSwHook

                                                                @ Functions declared in this file.
    .global  OS_CPU_SR_Save
    .global  OS_CPU_SR_Restore
    .global  OSStartHighRdy
    .global  OSCtxSw
    .global  OSIntCtxSw

/*
                                                                @ Functions related to exception handling.
    .global  OS_CPU_ARM_ExceptResetHndlr
    .global  OS_CPU_ARM_ExceptUndefInstrHndlr
    .global  OS_CPU_ARM_ExceptSwiHndlr
    .global  OS_CPU_ARM_ExceptPrefetchAbortHndlr
    .global  OS_CPU_ARM_ExceptDataAbortHndlr
    .global  OS_CPU_ARM_ExceptAddrAbortHndlr
    .global  OS_CPU_ARM_ExceptIrqHndlr
    .global  OS_CPU_ARM_ExceptFiqHndlr

    .extern  OS_CPU_ExceptHndlr

*/


@********************************************************************************************************
@                                                EQUATES
@********************************************************************************************************

    .equ     OS_CPU_ARM_CONTROL_INT_DIS,       0xC0             @ Disable both FIQ and IRQ.
    .equ     OS_CPU_ARM_CONTROL_FIQ_DIS,       0x40             @ Disable FIQ.
    .equ     OS_CPU_ARM_CONTROL_IRQ_DIS,       0x80             @ Disable IRQ.
    .equ     OS_CPU_ARM_CONTROL_THUMB,         0x20             @ Set THUMB mode.
    .equ     OS_CPU_ARM_CONTROL_ARM,           0x00             @ Set ARM mode.

    .equ     OS_CPU_ARM_MODE_MASK,             0x1F
    .equ     OS_CPU_ARM_MODE_USR,              0x10
    .equ     OS_CPU_ARM_MODE_FIQ,              0x11
    .equ     OS_CPU_ARM_MODE_IRQ,              0x12
    .equ     OS_CPU_ARM_MODE_SVC,              0x13
    .equ     OS_CPU_ARM_MODE_ABT,              0x17
    .equ     OS_CPU_ARM_MODE_UND,              0x1B
    .equ     OS_CPU_ARM_MODE_SYS,              0x1F

    .equ     OS_CPU_ARM_EXCEPT_RESET,          0x00
    .equ     OS_CPU_ARM_EXCEPT_UNDEF_INSTR,    0x01
    .equ     OS_CPU_ARM_EXCEPT_SWI,            0x02
    .equ     OS_CPU_ARM_EXCEPT_PREFETCH_ABORT, 0x03
    .equ     OS_CPU_ARM_EXCEPT_DATA_ABORT,     0x04
    .equ     OS_CPU_ARM_EXCEPT_ADDR_ABORT,     0x05
    .equ     OS_CPU_ARM_EXCEPT_IRQ,            0x06
    .equ     OS_CPU_ARM_EXCEPT_FIQ,            0x07


@********************************************************************************************************
@                                      CODE GENERATION DIRECTIVES
@********************************************************************************************************

    .code 32



@*********************************************************************************************************
@                                  CRITICAL SECTION METHOD 3 FUNCTIONS
@
@ Description: Disable/Enable interrupts by preserving the state of interrupts.  Generally speaking you
@              would store the state of the interrupt disable flag in the local variable 'cpu_sr' and then
@              disable interrupts.  'cpu_sr' is allocated in all of uC/OS-II's functions that need to
@              disable interrupts.  You would restore the interrupt disable state by copying back 'cpu_sr'
@              into the CPU's status register.
@
@ Prototypes : OS_CPU_SR  OS_CPU_SR_Save    (void);
@              void       OS_CPU_SR_Restore (OS_CPU_SR  os_cpu_sr);
@
@
@ Note(s)    : (1) These functions are used in general like this:
@
@                 void Task (void  *p_arg)
@                 {
@                                                               /* Allocate storage for CPU status register.            */
@                 #if (OS_CRITICAL_METHOD == 3)
@                      OS_CPU_SR  os_cpu_sr;
@                 #endif
@
@                          :
@                          :
@                      OS_ENTER_CRITICAL();                     /* os_cpu_sr = OS_CPU_SR_Save();                        */
@                          :
@                          :
@                      OS_EXIT_CRITICAL();                      /* OS_CPU_SR_Restore(cpu_sr);                           */
@                          :
@                          :
@                 }
@*********************************************************************************************************

OS_CPU_SR_Save:
    MRS     R0, CPSR
    ORR     R1, R0, #OS_CPU_ARM_CONTROL_INT_DIS                 @ Set IRQ and FIQ bits in CPSR to disable all interrupts.
    MSR     CPSR_c, R1
    BX      LR                                                  @ Disabled, return the original CPSR contents in R0.


OS_CPU_SR_Restore:
    MSR     CPSR_c, R0
    BX      LR


@*********************************************************************************************************
@                                           START MULTITASKING
@                                       void OSStartHighRdy(void)
@
@ Note(s) : 1) OSStartHighRdy() MUST:
@              a) Call OSTaskSwHook() then,
@              b) Set OSRunning to TRUE,
@              c) Switch to the highest priority task.
@*********************************************************************************************************

OSStartHighRdy:
                                                                @ Change to SVC mode.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)

    LDR     R0, =OSTaskSwHook                                   @ OSTaskSwHook();
    MOV     LR, PC
    BX      R0

    LDR     R0, =OSRunning                                      @ OSRunning = TRUE;
    MOV     R1, #1
    STRB    R1, [R0]

                                                                @ SWITCH TO HIGHEST PRIORITY TASK:
    LDR     R0, =OSTCBHighRdy                                   @    Get highest priority task TCB address,
    LDR     R0, [R0]                                            @    Get stack pointer,
    LDR     SP, [R0]                                            @    Switch to the new stack,

    LDR     R0, [SP], #4                                        @    Pop new task's CPSR,
    MSR     CPSR_cxsf, R0

    LDR     R0,  [SP], #4           
    LDR     R1,  [SP], #4
    LDR     R2,  [SP], #4
    @STR     R1,  [SP, #-4]!
    @STR     R0,  [SP, #-4]!
    LDR     R3,  [SP], #4
    LDR     R4,  [SP], #4
    LDR     R5,  [SP], #4
    LDR     R6,  [SP], #4
    LDR     R7,  [SP], #4
    LDR     R8,  [SP], #4
    LDR     R9,  [SP], #4
    LDR     R10, [SP], #4
    LDR     R11, [SP], #4
    LDR     R12, [SP], #4
    LDR     LR,  [SP], #4
    LDR     PC,  [SP], #4
    @LDMFD   SP!, {R0-R12, LR, PC}^                              @    Pop new task's context.
    

@*********************************************************************************************************
@                         PERFORM A CONTEXT SWITCH (From task level) - OSCtxSw()
@
@ Note(s) : 1) OSCtxSw() is called in SVC mode with BOTH FIQ and IRQ interrupts DISABLED.
@
@           2) The pseudo-code for OSCtxSw() is:
@              a) Save the current task's context onto the current task's stack,
@              b) OSTCBCur->OSTCBStkPtr = SP;
@              c) OSTaskSwHook();
@              d) OSPrioCur             = OSPrioHighRdy;
@              e) OSTCBCur              = OSTCBHighRdy;
@              f) SP                    = OSTCBHighRdy->OSTCBStkPtr;
@              g) Restore the new task's context from the new task's stack,
@              h) Return to new task's code.
@
@           3) Upon entry:
@              OSTCBCur      points to the OS_TCB of the task to suspend,
@              OSTCBHighRdy  points to the OS_TCB of the task to resume.
@*********************************************************************************************************

OSCtxSw:
                                                                @ SAVE CURRENT TASK'S CONTEXT:
    @STMFD   SP!, {LR}                                           @     Push return address,
    @STMFD   SP!, {LR}
    STR     LR,  [SP, #-4]!
    STR     LR,  [SP, #-4]!
    STR     R12, [SP, #-4]!
    STR     R11, [SP, #-4]!
    STR     R10, [SP, #-4]!
    STR     R9,  [SP, #-4]!
    STR     R8,  [SP, #-4]!
    STR     R7,  [SP, #-4]!
    STR     R6,  [SP, #-4]!
    STR     R5,  [SP, #-4]!
    STR     R4,  [SP, #-4]!
    STR     R3,  [SP, #-4]!
    STR     R2,  [SP, #-4]!
    STR     R1,  [SP, #-4]!
    STR     R0,  [SP, #-4]!
    @STMFD   SP!, {R0-R12}                                       @     Push registers,
    MRS     R0, CPSR                                             @     Push current CPSR,
    @TST     LR, #1                                              @     See if called from Thumb mode,
    @ORRNE   R0, R0, #OS_CPU_ARM_CONTROL_THUMB                   @     If yes, set the T-bit.
    STMFD   SP!, {R0}

    LDR     R0, =OSTCBCur                                       @ OSTCBCur->OSTCBStkPtr = SP;
    LDR     R1, [R0]
    STR     SP, [R1]

    LDR     R0, =OSTaskSwHook                                   @ OSTaskSwHook();
    MOV     LR, PC
    BX      R0

    LDR     R0, =OSPrioCur                                      @ OSPrioCur = OSPrioHighRdy;
    LDR     R1, =OSPrioHighRdy
    LDRB    R2, [R1]
    STRB    R2, [R0]

    LDR     R0, =OSTCBCur                                       @ OSTCBCur  = OSTCBHighRdy;
    LDR     R1, =OSTCBHighRdy
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     SP, [R2]                                            @ SP = OSTCBHighRdy->OSTCBStkPtr;

                                                                @ RESTORE NEW TASK'S CONTEXT:
    @LDMFD   SP!, {R0}                                          @    Pop new task's CPSR,
    LDR     R0,  [SP], #4                                       @    Pop new task's CPSR,
    MSR     CPSR_cxsf, R0

    LDR     R0,  [SP], #4           
    LDR     R1,  [SP], #4
    LDR     R2,  [SP], #4
    LDR     R3,  [SP], #4
    LDR     R4,  [SP], #4
    LDR     R5,  [SP], #4
    LDR     R6,  [SP], #4
    LDR     R7,  [SP], #4
    LDR     R8,  [SP], #4
    LDR     R9,  [SP], #4
    LDR     R10, [SP], #4
    LDR     R11, [SP], #4
    LDR     R12, [SP], #4
    LDR     LR,  [SP], #4
    LDR     PC,  [SP], #4
    @LDMFD   SP!, {R0-R12, LR, PC}^                              @    Pop new task's context.


@*********************************************************************************************************
@                     PERFORM A CONTEXT SWITCH (From interrupt level) - OSIntCtxSw()
@
@ Note(s) : 1) OSIntCtxSw() is called in SVC mode with BOTH FIQ and IRQ interrupts DISABLED.
@
@           2) The pseudo-code for OSCtxSw() is:
@              a) OSTaskSwHook();
@              b) OSPrioCur             = OSPrioHighRdy;
@              c) OSTCBCur              = OSTCBHighRdy;
@              d) SP                    = OSTCBHighRdy->OSTCBStkPtr;
@              e) Restore the new task's context from the new task's stack,
@              f) Return to new task's code.
@
@           3) Upon entry:
@              OSTCBCur      points to the OS_TCB of the task to suspend,
@              OSTCBHighRdy  points to the OS_TCB of the task to resume.
@*********************************************************************************************************

OSIntCtxSw:
    LDR     R0, =OSTaskSwHook                                   @ OSTaskSwHook();
    MOV     LR, PC
    BX      R0

    LDR     R0, =OSPrioCur                                      @ OSPrioCur = OSPrioHighRdy;
    LDR     R1, =OSPrioHighRdy
    LDRB    R2, [R1]
    STRB    R2, [R0]

    LDR     R0, =OSTCBCur                                       @ OSTCBCur  = OSTCBHighRdy;
    LDR     R1, =OSTCBHighRdy
    LDR     R2, [R1]
    STR     R2, [R0]

    LDR     SP, [R2]                                            @ SP = OSTCBHighRdy->OSTCBStkPtr;

                                                                @ RESTORE NEW TASK'S CONTEXT:
    LDMFD   SP!, {R0}                                           @    Pop new task's CPSR,
    MSR     CPSR_cxsf, R0

    @LDMFD   SP!, {R0-R12, LR, PC}^                              @    Pop new task's context.
    LDR     R0,  [SP], #4           
    LDR     R1,  [SP], #4
    LDR     R2,  [SP], #4
    LDR     R3,  [SP], #4
    LDR     R4,  [SP], #4
    LDR     R5,  [SP], #4
    LDR     R6,  [SP], #4
    LDR     R7,  [SP], #4
    LDR     R8,  [SP], #4
    LDR     R9,  [SP], #4
    LDR     R10, [SP], #4
    LDR     R11, [SP], #4
    LDR     R12, [SP], #4
    LDR     LR,  [SP], #4
    LDR     PC,  [SP], #4


@********************************************************************************************************
@                                        RESET EXCEPTION HANDLER
@
@ Register Usage:  R0     Exception Type
@                  R1
@                  R2
@                  R3     Return PC
@********************************************************************************************************
/*
OS_CPU_ARM_ExceptResetHndlr:
                                                                @ LR offset to return from this exception:  0.
                                                                @  (there is no way to return from a RESET exception).
    STMFD   SP!, {R0-R12, LR}                                   @ Push working registers.
    MOV     R3, LR                                              @ Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_RESET                        @ Set exception ID to OS_CPU_ARM_EXCEPT_RESET.
    B            OS_CPU_ARM_ExceptHndlr                         @ Branch to global exception handler.


@********************************************************************************************************
@                                UNDEFINED INSTRUCTION EXCEPTION HANDLER
@
@ Register Usage:  R0     Exception Type
@                  R1
@                  R2
@                  R3     Return PC
@********************************************************************************************************

OS_CPU_ARM_ExceptUndefInstrHndlr:
                                                                @ LR offset to return from this exception:  0.
    STMFD   SP!, {R0-R12, LR}                                   @ Push working registers.
    MOV     R3, LR                                              @ Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_UNDEF_INSTR                  @ Set exception ID to OS_CPU_ARM_EXCEPT_UNDEF_INSTR.
    B            OS_CPU_ARM_ExceptHndlr                         @ Branch to global exception handler.


@********************************************************************************************************
@                                 SOFTWARE INTERRUPT EXCEPTION HANDLER
@
@ Register Usage:  R0     Exception Type
@                  R1
@                  R2
@                  R3     Return PC
@********************************************************************************************************

OS_CPU_ARM_ExceptSwiHndlr:
                                                                @ LR offset to return from this exception:  0.
    STMFD   SP!, {R0-R12, LR}                                   @ Push working registers.
    MOV     R3, LR                                              @ Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_SWI                          @ Set exception ID to OS_CPU_ARM_EXCEPT_SWI.
    B            OS_CPU_ARM_ExceptHndlr                         @ Branch to global exception handler.


@********************************************************************************************************
@                                   PREFETCH ABORT EXCEPTION HANDLER
@
@ Register Usage:  R0     Exception Type
@                  R1
@                  R2
@                  R3     Return PC
@********************************************************************************************************

OS_CPU_ARM_ExceptPrefetchAbortHndlr:
    SUB     LR, LR, #4                                          @ LR offset to return from this exception: -4.
    STMFD   SP!, {R0-R12, LR}                                   @ Push working registers.
    MOV     R3, LR                                              @ Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_PREFETCH_ABORT               @ Set exception ID to OS_CPU_ARM_EXCEPT_PREFETCH_ABORT.
    B            OS_CPU_ARM_ExceptHndlr                         @ Branch to global exception handler.


@********************************************************************************************************
@                                     DATA ABORT EXCEPTION HANDLER
@
@ Register Usage:  R0     Exception Type
@                  R1
@                  R2
@                  R3     Return PC
@********************************************************************************************************

OS_CPU_ARM_ExceptDataAbortHndlr:
    SUB     LR, LR, #8                                          @ LR offset to return from this exception: -8.
    STMFD   SP!, {R0-R12, LR}                                   @ Push working registers.
    MOV     R3, LR                                              @ Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_DATA_ABORT                   @ Set exception ID to OS_CPU_ARM_EXCEPT_DATA_ABORT.
    B            OS_CPU_ARM_ExceptHndlr                         @ Branch to global exception handler.


@********************************************************************************************************
@                                    ADDRESS ABORT EXCEPTION HANDLER
@
@ Register Usage:  R0     Exception Type
@                  R1
@                  R2
@                  R3     Return PC
@********************************************************************************************************

OS_CPU_ARM_ExceptAddrAbortHndlr:
    SUB     LR, LR, #8                                          @ LR offset to return from this exception: -8.
    STMFD   SP!, {R0-R12, LR}                                   @ Push working registers.
    MOV     R3, LR                                              @ Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_ADDR_ABORT                   @ Set exception ID to OS_CPU_ARM_EXCEPT_ADDR_ABORT.
    B            OS_CPU_ARM_ExceptHndlr                         @ Branch to global exception handler.


@********************************************************************************************************
@                                  INTERRUPT REQUEST EXCEPTION HANDLER
@
@ Register Usage:  R0     Exception Type
@                  R1
@                  R2
@                  R3     Return PC
@********************************************************************************************************

OS_CPU_ARM_ExceptIrqHndlr:
    SUB     LR, LR, #4                                          @ LR offset to return from this exception: -4.
    STMFD   SP!, {R0-R12, LR}                                   @ Push working registers.
    MOV     R3, LR                                              @ Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_IRQ                          @ Set exception ID to OS_CPU_ARM_EXCEPT_IRQ.
    B            OS_CPU_ARM_ExceptHndlr                         @ Branch to global exception handler.


@********************************************************************************************************
@                               FAST INTERRUPT REQUEST EXCEPTION HANDLER
@
@ Register Usage:  R0     Exception Type
@                  R1
@                  R2
@                  R3     Return PC
@********************************************************************************************************

OS_CPU_ARM_ExceptFiqHndlr:
    SUB     LR, LR, #4                                          @ LR offset to return from this exception: -4.
    STMFD   SP!, {R0-R12, LR}                                   @ Push working registers.
    MOV     R3, LR                                              @ Save link register.
    MOV     R0, #OS_CPU_ARM_EXCEPT_FIQ                          @ Set exception ID to OS_CPU_ARM_EXCEPT_FIQ.
    B            OS_CPU_ARM_ExceptHndlr                         @ Branch to global exception handler.


@********************************************************************************************************
@                                       GLOBAL EXCEPTION HANDLER
@
@ Register Usage:  R0     Exception Type
@                  R1	  Exception's SPSR
@                  R2     Old CPU mode
@                  R3     Return PC
@********************************************************************************************************

OS_CPU_ARM_ExceptHndlr:
    MRS     R1, SPSR                                            @ Save CPSR (i.e. exception's SPSR).

                                                                @ DETERMINE IF WE INTERRUPTED A TASK OR ANOTHER LOWER PRIORITY EXCEPTION:
                                                                @   SPSR.Mode = SVC                :  task,
                                                                @   SPSR.Mode = FIQ, IRQ, ABT, UND :  other exceptions,
                                                                @   SPSR.Mode = USR                : *unsupported state*.
    AND     R2, R1, #OS_CPU_ARM_MODE_MASK
    CMP     R2,     #OS_CPU_ARM_MODE_SVC
    BNE     OS_CPU_ARM_ExceptHndlr_BreakExcept


@********************************************************************************************************
@                                  EXCEPTION HANDLER: TASK INTERRUPTED
@
@ Register Usage:  R0     Exception Type
@                  R1	  Exception's SPSR
@                  R2	  Exception's CPSR
@                  R3     Return PC
@                  R4     Exception's SP
@********************************************************************************************************

OS_CPU_ARM_ExceptHndlr_BreakTask:
    MRS     R2, CPSR                                            @ Save exception's CPSR.
    MOV     R4, SP                                              @ Save exception's stack pointer.

                                                                @ Change to SVC mode & disable interruptions.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)

                                                                @ SAVE TASK'S CONTEXT ONTO TASK'S STACK:
    STMFD   SP!, {R3}                                           @   Push task's PC,
    STMFD   SP!, {LR}                                           @   Push task's LR,
    STMFD   SP!, {R5-R12}                                       @   Push task's R12-R5,
    LDMFD   R4!, {R5-R9}                                        @   Move task's R4-R0 from exception stack to task's stack.
    STMFD   SP!, {R5-R9}
    STMFD   SP!, {R1}                                           @   Push task's CPSR (i.e. exception SPSR).

                                                                @ if (OSRunning == 1)
    LDR     R1, =OSRunning
    LDRB    R1, [R1]
    CMP     R1, #1
    BNE     OS_CPU_ARM_ExceptHndlr_BreakTask_1

                                                                @ HANDLE NESTING COUNTER:
    LDR     R3, =OSIntNesting                                   @   OSIntNesting++;
    LDRB    R4, [R3]
    ADD     R4, R4, #1
    STRB    R4, [R3]

    LDR     R3, =OSTCBCur                                       @   OSTCBCur->OSTCBStkPtr = SP;
    LDR     R4, [R3]
    STR     SP, [R4]

OS_CPU_ARM_ExceptHndlr_BreakTask_1:
    MSR     CPSR_cxsf, R2                                       @ RESTORE INTERRUPTED MODE.

                                                                @ EXECUTE EXCEPTION HANDLER:
    LDR     R1, =OS_CPU_ExceptHndlr                             @ OS_CPU_ExceptHndlr(except_type = R0);
    MOV     LR, PC
    BX      R1

                                                                @ Adjust exception stack pointer.  This is needed because
                                                                @ exception stack is not used when restoring task context.
    ADD     SP, SP, #(14 * 4)

                                                                @ Change to SVC mode & disable interruptions.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)

                                                                @ Call OSIntExit().  This call MAY never return if a ready
                                                                @  task with higher priority than the interrupted one is
                                                                @  found.
    LDR     R0, =OSIntExit
    MOV     LR, PC
    BX      R0

                                                                @ RESTORE NEW TASK'S CONTEXT:
    LDMFD   SP!, {R0}                                           @    Pop new task's CPSR,
    MSR     SPSR_cxsf, R0

    LDMFD   SP!, {R0-R12, LR, PC}^                              @    Pop new task's context.


@********************************************************************************************************
@                               EXCEPTION HANDLER: EXCEPTION INTERRUPTED
@
@ Register Usage:  R0     Exception Type
@                  R1
@                  R2
@                  R3
@********************************************************************************************************

OS_CPU_ARM_ExceptHndlr_BreakExcept:
    MRS     R2, CPSR                                            @ Save exception's CPSR.

                                                                @ Change to SVC mode & disable interruptions.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)

                                                                @ HANDLE NESTING COUNTER:
    LDR     R3, =OSIntNesting                                   @   OSIntNesting++;
    LDRB    R4, [R3]
    ADD     R4, R4, #1
    STRB    R4, [R3]

    MSR     CPSR_cxsf, R2                                       @ RESTORE INTERRUPTED MODE.

                                                                @ EXECUTE EXCEPTION HANDLER:
    LDR     R3, =OS_CPU_ExceptHndlr                             @ OS_CPU_ExceptHndlr(except_type = R0);
    MOV     LR, PC
    BX      R3

                                                                @ Change to SVC mode & disable interruptions.
    MSR     CPSR_c, #(OS_CPU_ARM_CONTROL_INT_DIS | OS_CPU_ARM_MODE_SVC)

                                                                @ HANDLE NESTING COUNTER:
    LDR     R3, =OSIntNesting                                   @   OSIntNesting--;
    LDRB    R4, [R3]
    SUB     R4, R4, #1
    STRB    R4, [R3]

    MSR     CPSR_cxsf, R2                                       @ RESTORE INTERRUPTED MODE.

                                                                @ RESTORE OLD CONTEXT:
    LDMFD   SP!, {R0-R12, PC}^                                  @ Pull working registers and return from exception.
*/

    .ltorg
