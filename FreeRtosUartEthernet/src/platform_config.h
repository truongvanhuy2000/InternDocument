#ifndef __PLATFORM_CONFIG_H_
#define __PLATFORM_CONFIG_H_
#include <stdio.h>
#include "xparameters.h"
#include "xil_printf.h"
#include "xscugic_hw.h"

#define INTC_DEVICE_ID XPAR_SCUGIC_SINGLE_DEVICE_ID

#define INTC_BASE_ADDR XPAR_SCUGIC_0_CPU_BASEADDR
#define INTC_DIST_ADDR XPAR_SCUGIC_0_DIST_BASEADDR
#define INTC_DIST_BASE_ADDR XPAR_SCUGIC_0_DIST_BASEADDR
#define TIMER_IRPT_INTR XPAR_SCUTIMER_INTR

void platform_setup_interrupts(void);
void platform_enable_interrupts();
void platform_register_interrupt(s32 InterruptID,
                                 Xil_InterruptHandler Handler, void *CallBackRef,
                                 u8 Priority, u8 Trigger);
#endif
