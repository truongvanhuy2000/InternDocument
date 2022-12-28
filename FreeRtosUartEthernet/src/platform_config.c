#include "platform_config.h"
void platform_setup_interrupts(void)
{
    Xil_ExceptionInit();

    XScuGic_DeviceInitialize(INTC_DEVICE_ID);
    /*
     * Connect the interrupt controller interrupt handler to the hardware
     * interrupt handling logic in the processor.
     */
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_IRQ_INT,
                                 (Xil_ExceptionHandler)XScuGic_DeviceInterruptHandler,
                                 (void *)INTC_DEVICE_ID);
    return;
}
void platform_enable_interrupts()
{
    Xil_ExceptionEnableMask(XIL_EXCEPTION_IRQ);
    return;
}
void platform_register_interrupt(s32 InterruptID,
                                 Xil_InterruptHandler Handler, void *CallBackRef,
                                 u8 Priority, u8 Trigger)
{
    XScuGic_RegisterHandler(INTC_BASE_ADDR, InterruptID, Handler, CallBackRef);
    XScuGic_SetPriTrigTypeByDistAddr(INTC_DIST_ADDR, InterruptID, Priority, Trigger);
    XScuGic_EnableIntr(INTC_DIST_ADDR, InterruptID);
}
