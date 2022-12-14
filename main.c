/******************************************************************************
 *
 * Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * Use of the Software is limited solely to applications:
 * (a) running on a Xilinx device, or
 * (b) that interact with a Xilinx device through a bus or interconnect.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of the Xilinx shall not be used
 * in advertising or otherwise to promote the sale, use or other dealings in
 * this Software without prior written authorization from Xilinx.
 *
 ******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include "xparameters.h"
#include "sleep.h"
#include "xuartlite.h"
#include "xil_exception.h"
#include "xuartlite_l.h"

#ifdef XPAR_INTC_0_DEVICE_ID
#include "xintc.h"
#include <stdio.h>
#else
#include "xscugic.h"
#include "xil_printf.h"
#endif

#define BYTE_SENT_COUNT 48
#define BYTE_RECV_COUNT 64

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC_DEVICE_ID		XPAR_INTC_0_DEVICE_ID
#else
#define INTC_DEVICE_ID		XPAR_SCUGIC_SINGLE_DEVICE_ID
#endif /* XPAR_INTC_0_DEVICE_ID */


#define UART1_ID XPAR_AXI_UARTLITE_0_DEVICE_ID
#define UART2_ID XPAR_AXI_UARTLITE_1_DEVICE_ID
#define UART_INTERRUPT_ID XPAR_FABRIC_AXI_UARTLITE_1_INTERRUPT_INTR

#ifdef XPAR_INTC_0_DEVICE_ID
#define INTC		XIntc
#define INTC_HANDLER	XIntc_InterruptHandler
#else
#define INTC		XScuGic
#define INTC_HANDLER	XScuGic_InterruptHandler
#endif /* XPAR_INTC_0_DEVICE_ID */

XUartLite uart1;
XUartLite uart2;

INTC interruptController;

u8 sendData[BYTE_SENT_COUNT];
u8 recvData[BYTE_RECV_COUNT];
volatile u32* data = (volatile u32*)(XPAR_UARTLITE_1_BASEADDR + 0x08);

int byteSent;
volatile int byteRecieve;

XStatus uartConfig();
XStatus interruptConfig();
static void recvCallback(void *CallBackRef, unsigned int ByteCount);
int SetupInterruptSystem(INTC *IntcInstancePtr, XUartLite *UartLiteInstPtr, u16 UartLiteIntrId);
void customPrint(u8 *ptr, int count);
void generateData(u8 *data, int count);


int main()
{
	print("hello world");
	if (uartConfig() != XST_SUCCESS)
	{
		print("configuration error");
		return 0;
	}

	if (interruptConfig() != XST_SUCCESS)
	{
		print("configuration error");
		return 0;
	}
	generateData(sendData, BYTE_SENT_COUNT);

	while (1)
	{
		while (byteSent != BYTE_SENT_COUNT)
		{
			byteSent += XUartLite_Send(&uart1, &sendData[byteSent], BYTE_SENT_COUNT - byteSent);
		}
		if (byteRecieve == BYTE_RECV_COUNT)
		{
			customPrint(recvData, BYTE_RECV_COUNT);
			byteRecieve = 0;
			memset((char*)byteRecieve, 0, BYTE_RECV_COUNT);
		}
		byteSent = 0;
		print("\n");
		usleep(400 * 1000);
	}
	return 0;
}
//-------------------------------CONFIG FUNCTION------------------------------------------------------------------//
XStatus uartConfig()
{
	XStatus status;

	XUartLite_Config *uart1Config = XUartLite_LookupConfig(UART1_ID);
	XUartLite_Config *uart2Config = XUartLite_LookupConfig(UART2_ID);

	status = XUartLite_CfgInitialize(&uart1, uart1Config, uart1Config->RegBaseAddr);
	if (status != XST_SUCCESS)
	{
		print("error");
		return XST_FAILURE;
	}
	status = XUartLite_CfgInitialize(&uart2, uart2Config, uart2Config->RegBaseAddr);
	if (status != XST_SUCCESS)
	{
		print("error");
		return XST_FAILURE;
	}

	status = XUartLite_Initialize(&uart1, UART1_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	status = XUartLite_Initialize(&uart2, UART2_ID);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	status = XUartLite_SelfTest(&uart1);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}
	status = XUartLite_SelfTest(&uart2);
	if (status != XST_SUCCESS) {
		return XST_FAILURE;
	}

	XUartLite_ResetFifos(&uart1);
	XUartLite_ResetFifos(&uart2);

	return XST_SUCCESS;
}

XStatus interruptConfig()
{
	XStatus Status;

	Status = SetupInterruptSystem(&interruptController, &uart2, UART_INTERRUPT_ID);
	if (Status != XST_SUCCESS)
	{
		return XST_FAILURE;
	}
	XUartLite_SetRecvHandler(&uart2, recvCallback, &uart2);
	XUartLite_EnableInterrupt(&uart2);
	return XST_SUCCESS;
}
int SetupInterruptSystem(INTC *IntcInstancePtr, XUartLite *UartLiteInstPtr, u16 UartLiteIntrId)
{
	int Status;

	#ifdef XPAR_INTC_0_DEVICE_ID

		/*
		 * Initialize the interrupt controller driver so that it is ready
		 * to use.
		 */

		Status = XIntc_Initialize(IntcInstancePtr, INTC_DEVICE_ID);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}

		/*
		 * Connect a device driver handler that will be called when an interrupt
		 * for the device occurs, the device driver handler performs the specific
		 * interrupt processing for the device.
		 */
		Status = XIntc_Connect(IntcInstancePtr, UartLiteIntrId,
				(XInterruptHandler)XUartLite_InterruptHandler,
				(void *)UartLiteInstPtr);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		/*
		 * Start the interrupt controller such that interrupts are enabled for
		 * all devices that cause interrupts, specific real mode so that
		 * the UART can cause interrupts through the interrupt controller.
		 */
		Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		/*
		 * Enable the interrupt for the UartLite.
		 */
		XIntc_Enable(IntcInstancePtr, UartLiteIntrId);
	#else
		XScuGic_Config *IntcConfig;

		/*
		 * Initialize the interrupt controller driver so that it is ready to
		 * use.
		 */
		IntcConfig = XScuGic_LookupConfig(INTC_DEVICE_ID);
		if (NULL == IntcConfig) {
			return XST_FAILURE;
		}

		Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
						IntcConfig->CpuBaseAddress);
		if (Status != XST_SUCCESS) {
			return XST_FAILURE;
		}
		XScuGic_SetPriorityTriggerType(IntcInstancePtr, UartLiteIntrId,
						0xA0, 0x3);
		/*
		 * Connect the interrupt handler that will be called when an
		 * interrupt occurs for the device.
		 */
		Status = XScuGic_Connect(IntcInstancePtr, UartLiteIntrId,
					 (Xil_ExceptionHandler)XUartLite_InterruptHandler,
					 UartLiteInstPtr);
		if (Status != XST_SUCCESS) {
			return Status;
		}
		/*
		 * Enable the interrupt for the Timer device.
		 */
		XScuGic_Enable(IntcInstancePtr, UartLiteIntrId);

	#endif /* XPAR_INTC_0_DEVICE_ID */

		Xil_ExceptionInit();

		/*
		 * Register the interrupt controller handler with the exception table.
		 */
		Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,
				(Xil_ExceptionHandler)INTC_HANDLER,
				IntcInstancePtr);

		/*
		 * Enable exceptions.
		 */
		Xil_ExceptionEnable();


		return XST_SUCCESS;
}

//----------------------------CALL BACK FUNCTION-------------------------------------------------------------------------------//

static void recvCallback(void *CallBackRef, unsigned int ByteCount)
{
	// byteSent = ByteCount;
	XUartLite_Recv(&uart2, &recvData[byteRecieve], BYTE_RECV_COUNT - ByteCount);
	byteRecieve += ByteCount;
	if(byteRecieve > BYTE_RECV_COUNT)
	{
		byteRecieve = BYTE_RECV_COUNT;
	}
	XUartLite_ResetFifos(&uart2);
}
//--------------------------------------------------------------------------//
void customPrint(u8 *ptr, int count)
{
	int i = 0;
	while (i < count)
	{
		outbyte(*ptr);
		ptr++;
		i++;
	}
}
void generateData(u8 *data, int count)
{
	int index;
	for (index = 0; index < count; index++)
	{
		data[index] = 'A' + index;
	}
}
