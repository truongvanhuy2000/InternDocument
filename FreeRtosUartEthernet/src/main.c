/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
/* Xilinx includes. */
#include "xil_printf.h"
#include "xparameters.h"
#include "uartController.h"
#include "tcpIpController.h"
// All application will share this data
#include "ApplicationData.h"

#define START_BYTE 0x55
#define STOP_BYTE 0xAA

static TaskHandle_t xemacInputHandle;
static TaskHandle_t uartTxHandle;
static TaskHandle_t ethernetProcessDataHandle;
static TaskHandle_t uartProcessRxDataHandle;

static QueueHandle_t ethernetRxQueue;
static QueueHandle_t uartRxQueue;

extern XUartLite uart1;
extern XUartLite uart2;
extern struct netif server_netif;

XStatus tcpIpControllerConfig();
XStatus uartControllerConfig();
static void uartRecvCallback(void *CallBackRef, unsigned int ByteCount);
static err_t tcpRxCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err);
static void xemacInputTask(void *pvParameters);
static void xemacInputTask(void *pvParameters);
static void uartTxTask(void *pvParameters);
static void processUartRxData(void *pvParameters);

int main()
{
    if (!uartControllerConfig())
    {
        print("Uart configuration error");
        return 0;
    }
    if (!tcpIpControllerConfig())
    {
        print("TCP/IP configuration error");
        return 0;
    }
    xTaskCreate(xemacInputTask, (const char *)"xemac", configMINIMAL_STACK_SIZE,
                NULL, 1, &xemacInputHandle);
    xTaskCreate(ethernetProcessDataTask, (const char *)"ethernet", configMINIMAL_STACK_SIZE,
                NULL, 1, &ethernetProcessDataHandle);
    xTaskCreate(uartTxTask, (const char *)"uart tx", configMINIMAL_STACK_SIZE,
                NULL, 2, &uartTxHandle);
    xTaskCreate(processUartRxData, (const char *)"uart process", configMINIMAL_STACK_SIZE,
                NULL, 3, &uartProcessRxDataHandle);

    ethernetRxQueue = xQueueCreate(1, sizeof(struct pbuf *)); /* Each space in the queue is large enough to hold a uint32_t. */
    configASSERT(ethernetRxQueue);
    uartRxQueue = xQueueCreate(1, sizeof(applicationData)); /* Each space in the queue is large enough to hold a uint32_t. */
    configASSERT(uartRxQueue);
    vTaskStartScheduler();
    return 0;
}
static void xemacInputTask(void *pvParameters)
{
    for (;;)
    {
        xemacif_input(&server_netif);
    }
}
static void ethernetProcessDataTask(void *pvParameters)
{
    for (;;)
    {
    }
}
static void uartTxTask(void *pvParameters)
{
    for (;;)
    {
        xQueueReceive(uartRxQueue, &tempData, portMAX_DELAY);
    }
}
static void processUartRxData(void *pvParameters)
{
    applicationData tempData;
    for (;;)
    {
        xQueueReceive(uartRxQueue, &tempData, portMAX_DELAY);
        if (tempData.startByte != START_BYTE || tempData.stopByte != STOP_BYTE)
        {
            xil_printf("wrong data packet");
            continue;
        }
        u8 *data = (u8 *)malloc(100 * sizeof(u8));
        if (data == NULL)
        {
            xil_printf("Memory not allocated.\n");
            continue;
        }
        sprintf(data, "\ndata0: %c, data1: %c", tempData.data0, tempData.data1);
        xil_printf(data);
        free(data);
    }
}
XStatus uartControllerConfig()
{
    if (uartConfig() != XST_SUCCESS)
    {
        return 0;
    }
    if (interruptConfig(uartRecvCallback) != XST_SUCCESS)
    {
        return 0;
    }
    return 1;
}
XStatus tcpIpControllerConfig()
{
    if (!tcpIpControllerInit())
    {
        return 0;
    }
    setCallbackFunction(tcp_recv);
    return 1;
}
static void uartRecvCallback(void *CallBackRef, unsigned int ByteCount)
{
    BaseType_t xHigherPriorityTaskWoken;
    applicationData data;
    u8 dataBuff[4];
    if (ByteCount != 4)
    {
        xil_printf("wrong data packet");
        XUartLite_ResetFifos(&uart1);
        XUartLite_ResetFifos(&uart2);
        return;
    }
    XUartLite_Recv(CallBackRef, dataBuff, 4);
    data = *((applicationData *)dataBuff);
    xQueueSendToFrontFromISR(uartRxQueue, &data, xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}
err_t tcpRxCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
}
