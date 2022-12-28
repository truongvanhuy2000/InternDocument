#ifndef UART_CONTROLLER
#define UART_CONTROLLER
#include "xparameters.h"
#include "sleep.h"
#include "xuartlite.h"
#include "xil_exception.h"
#include "xuartlite_l.h"

#include "xscugic.h"
#include "xil_printf.h"
#include "platform_config.h"

#define BYTE_SENT_COUNT 48
#define BYTE_RECV_COUNT 64

#define INTC_DEVICE_ID XPAR_SCUGIC_SINGLE_DEVICE_ID

#define UART1_ID XPAR_AXI_UARTLITE_0_DEVICE_ID
#define UART2_ID XPAR_AXI_UARTLITE_1_DEVICE_ID
#define UART_INTERRUPT_ID XPAR_FABRIC_AXI_UARTLITE_1_INTERRUPT_INTR

#define INTC XScuGic
#define INTC_HANDLER XScuGic_InterruptHandler

INTC interruptController;
XUartLite uart1;
XUartLite uart2;

XStatus uartConfig();
XStatus interruptConfig(XUartLite_Handler FuncPtr);

#endif
