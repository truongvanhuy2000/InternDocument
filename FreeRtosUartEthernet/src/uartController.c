#include "uartController.h"

int SetupInterruptSystem(INTC *IntcInstancePtr, XUartLite *UartLiteInstPtr, u16 UartLiteIntrId);

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
    if (status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }
    status = XUartLite_Initialize(&uart2, UART2_ID);
    if (status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    status = XUartLite_SelfTest(&uart1);
    if (status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }
    status = XUartLite_SelfTest(&uart2);
    if (status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }

    XUartLite_ResetFifos(&uart1);
    XUartLite_ResetFifos(&uart2);

    return XST_SUCCESS;
}

XStatus interruptConfig(XUartLite_Handler FuncPtr)
{
    XStatus Status;

    Status = SetupInterruptSystem(&interruptController, &uart2, UART_INTERRUPT_ID);
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }
    XUartLite_SetRecvHandler(&uart2, FuncPtr, &uart2);
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
    if (Status != XST_SUCCESS)
    {
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
    if (Status != XST_SUCCESS)
    {
        return XST_FAILURE;
    }
    /*
     * Start the interrupt controller such that interrupts are enabled for
     * all devices that cause interrupts, specific real mode so that
     * the UART can cause interrupts through the interrupt controller.
     */
    Status = XIntc_Start(IntcInstancePtr, XIN_REAL_MODE);
    if (Status != XST_SUCCESS)
    {
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
    if (NULL == IntcConfig)
    {
        return XST_FAILURE;
    }

    Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig,
                                   IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS)
    {
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
    if (Status != XST_SUCCESS)
    {
        return Status;
    }
    /*
     * Enable the interrupt for the  device.
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