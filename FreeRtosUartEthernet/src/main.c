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
#include "platform_config.h"

#define START_BYTE 0x55
#define STOP_BYTE 0xAA

static TaskHandle_t xemacInputHandle;
static TaskHandle_t uartTxHandle;
static TaskHandle_t uartProcessRxDataHandle;
static TaskHandle_t startTaskHandle;
static TaskHandle_t ethernetRxHandler;

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
static void processUartRxTask(void *pvParameters);
static void initTask(void *pvParameters);
static void ethernetRxTask(void *pvParameters);

int main()
{
    xTaskCreate(xemacInputTask, (const char *)"xemac", 500,
                NULL, 1, &xemacInputHandle);
    xTaskCreate(ethernetRxTask, (const char *)"ethernetRx", 500,
                NULL, 1, &ethernetRxHandler);

    xTaskCreate(uartTxTask, (const char *)"uart tx", 500,
                NULL, 2, &uartTxHandle);

    xTaskCreate(processUartRxTask, (const char *)"uart process", 500,
                NULL, 3, &uartProcessRxDataHandle);

    xTaskCreate(initTask, (const char *)"init", 500,
                NULL, 4, &startTaskHandle);

    ethernetRxQueue = xQueueCreate(1, sizeof(applicationData)); /* Each space in the queue is large enough to hold a uint32_t. */
    uartRxQueue = xQueueCreate(1, sizeof(applicationData));     /* Each space in the queue is large enough to hold a uint32_t. */

    vTaskStartScheduler();
    while (1)
        ;
    return 0;
}
static void ethernetRxTask(void *pvParameters);
{
    applicationData tempData;
    static int state = 0;
    switch (state)
    {
    case 0:
        if (p->tot_len != 1)
        {
            tcp_write(tpcb, "data invalid, must be in the range of 0-255d, please send again\n", 65, TCP_WRITE_FLAG_COPY);
            break;
        }
        tempData.data0 = *((u8 *)p->payload);
        tcp_write(tpcb, "please send data 1:\n", 21, TCP_WRITE_FLAG_COPY);
        state = 1;
        break;
    case 1:
        if (p->tot_len != 1)
        {
            tcp_write(tpcb, "data invalid, must be in the range of 0-255d, please send again\n", 65, TCP_WRITE_FLAG_COPY);
            break;
        }
        tempData.data0 = *((u8 *)p->payload);
        state = 3;
        break;
    default:
        tcp_write(tpcb, "please send data 0:\n", 21, TCP_WRITE_FLAG_COPY);
        state = 0;
        break;
    }
}
// function for task
static void initTask(void *pvParameters)
{
    platform_setup_interrupts();
    if (!uartControllerConfig())
    {
        print("Uart configuration error");
        return;
    }
    platform_enable_interrupts();
    if (!tcpIpControllerConfig())
    {
        print("TCP/IP configuration error");
        return 0;
    }
    vTaskDelete(startTaskHandle);
}
static void xemacInputTask(void *pvParameters)
{
    for (;;)
    {
        // xemacif_input(&server_netif);
        XUartLite_Send(&uart1, "dcmm", 4);
    }
}
static void uartTxTask(void *pvParameters)
{
    applicationData tempData;
    for (;;)
    {
        xQueueReceive(ethernetRxQueue, &tempData, portMAX_DELAY);
        tempData.startByte = 0x55;
        tempData.stopByte = 0xAA;
        XUartLite_Send(&uart1, &tempData, 4);
    }
}
static void processUartRxTask(void *pvParameters)
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
// Function to config uart and tcp ip
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
    return 1;
}
// callback function

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
    XUartLite_ResetFifos(&uart2);
    //    xQueueSendToFrontFromISR(uartRxQueue, &data, xHigherPriorityTaskWoken);
    //    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

err_t tcpRxCallback(void *arg, struct tcp_pcb *tpcb, struct pbuf *p, err_t err)
{
    BaseType_t xHigherPriorityTaskWoken;
    applicationData tempData;
    static int state = 0;
    switch (state)
    {
    case 0:
        if (p->tot_len != 1)
        {
            tcp_write(tpcb, "data invalid, must be in the range of 0-255d, please send again\n", 65, TCP_WRITE_FLAG_COPY);
            break;
        }
        tempData.data0 = *((u8 *)p->payload);
        tcp_write(tpcb, "please send data 1:\n", 21, TCP_WRITE_FLAG_COPY);
        state = 1;
        break;
    case 1:
        if (p->tot_len != 1)
        {
            tcp_write(tpcb, "data invalid, must be in the range of 0-255d, please send again\n", 65, TCP_WRITE_FLAG_COPY);
            break;
        }
        tempData.data0 = *((u8 *)p->payload);
        tcp_write(tpcb, "done\n", 21, TCP_WRITE_FLAG_COPY);
        state = 3;
        break;
    default:
        tcp_write(tpcb, "please send data 0:\n", 21, TCP_WRITE_FLAG_COPY);
        xQueueSendToFrontFromISR(ethernetRxQueue, &tempData, xHigherPriorityTaskWoken);
        state = 0;
        break;
    }
    portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    return 0;
}
