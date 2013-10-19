typedef enum IRQn
{
/******  Cortex-M3 Processor Exceptions Numbers ***************************************************/
  NonMaskableInt_IRQn         = -14,    /*!< 2 Non Maskable Interrupt                             */
  MemoryManagement_IRQn       = -12,    /*!< 4 Cortex-M3 Memory Management Interrupt              */
  BusFault_IRQn               = -11,    /*!< 5 Cortex-M3 Bus Fault Interrupt                      */
  UsageFault_IRQn             = -10,    /*!< 6 Cortex-M3 Usage Fault Interrupt                    */
  SVCall_IRQn                 = -5,     /*!< 11 Cortex-M3 SV Call Interrupt                       */
  DebugMonitor_IRQn           = -4,     /*!< 12 Cortex-M3 Debug Monitor Interrupt                 */
  PendSV_IRQn                 = -2,     /*!< 14 Cortex-M3 Pend SV Interrupt                       */
  SysTick_IRQn                = -1,     /*!< 15 Cortex-M3 System Tick Interrupt                   */
} IRQn_Type;
#define __NVIC_PRIO_BITS    3

#include "CMSIS/include/core_cm4.h"

#include <stdio.h>

void SystemInit(void) {
	SCB->SHCSR |= 0x00007000; // enable Usage Fault, Bus Fault, and MMU Fault
}

// Use the 'naked' attribute so that C stacking is not used.
__attribute__((naked))
void HardFault_Handler(void){
        /*
         * Get the appropriate stack pointer, depending on our mode,
         * and use it as the parameter to the C handler. This function
         * will never return
         */
        __asm__(
                        "MOVS   R0, #4  \n"
                        "MOV    R1, LR  \n"
                        "TST    R0, R1  \n"
                        "BEQ    _MSP    \n"
                        "MRS    R0, PSP \n"
                        "B      hard_fault_handler_c      \n"
                		"_MSP:  \n"
                        "MRS    R0, MSP \n"
                        "B      hard_fault_handler_c      \n") ;
}

// From Joseph Yiu, minor edits by FVH
// hard fault handler in C,
// with stack frame location as input parameter
// called from HardFault_Handler in file startup_*.S
void hard_fault_handler_c (unsigned int * hardfault_args)
{
  unsigned int stacked_r0;
  unsigned int stacked_r1;
  unsigned int stacked_r2;
  unsigned int stacked_r3;
  unsigned int stacked_r12;
  unsigned int stacked_lr;
  unsigned int stacked_pc;
  unsigned int stacked_psr;

  stacked_r0 = ((unsigned long) hardfault_args[0]);
  stacked_r1 = ((unsigned long) hardfault_args[1]);
  stacked_r2 = ((unsigned long) hardfault_args[2]);
  stacked_r3 = ((unsigned long) hardfault_args[3]);

  stacked_r12 = ((unsigned long) hardfault_args[4]);
  stacked_lr = ((unsigned long) hardfault_args[5]);
  stacked_pc = ((unsigned long) hardfault_args[6]);
  stacked_psr = ((unsigned long) hardfault_args[7]);

  printf ("\n\n[Hard fault handler - all numbers in hex]\n");
  printf ("R0       %08x\n", stacked_r0);
  printf ("R1       %08x\n", stacked_r1);
  printf ("R2       %08x\n", stacked_r2);
  printf ("R3       %08x\n", stacked_r3);
  printf ("R12      %08x\n", stacked_r12);
  printf ("LR [R14] %08x  subroutine call return address\n", stacked_lr);
  printf ("PC [R15] %08x  program counter\n", stacked_pc);
  printf ("PSR      %08x\n", stacked_psr);
  printf ("BFAR     %08x\n", (*((volatile unsigned long *)(0xE000ED38))));
  printf ("CFSR     %08x\n", (*((volatile unsigned long *)(0xE000ED28))));
  printf ("HFSR     %08x\n", (*((volatile unsigned long *)(0xE000ED2C))));
  printf ("DFSR     %08x\n", (*((volatile unsigned long *)(0xE000ED30))));
  printf ("AFSR     %08x\n", (*((volatile unsigned long *)(0xE000ED3C))));
  printf ("SCB_SHCSR = %x\n", SCB->SHCSR);

  __asm("BKPT #0\n") ; // Break into the debugger
}

