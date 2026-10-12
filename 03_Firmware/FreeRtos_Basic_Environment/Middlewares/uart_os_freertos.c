#include "FreeRTOS.h"
#include "queue.h"
#include "uart_port.h"

static uart_os_queue_t freertos_queue_create(uint32_t len, uint32_t item_size)
{
    return (uart_os_queue_t)xQueueCreate(len, item_size);
}

static int freertos_queue_send(uart_os_queue_t q, const void* item, uart_os_tick_t timeout)
{
    TickType_t t = (timeout == UART_OS_WAIT_FOREVER) ? portMAX_DELAY : (TickType_t)timeout;
    return (pdTRUE == xQueueSend((QueueHandle_t)q, item, t)) ? 0 : -1;
}

static int freertos_queue_recv(uart_os_queue_t q, void* item, uart_os_tick_t timeout)
{
    TickType_t t = (timeout == UART_OS_WAIT_FOREVER) ? portMAX_DELAY : (TickType_t)timeout;
    return (pdTRUE == xQueueReceive((QueueHandle_t)q, item, t)) ? 0 : -1;
}

static int freertos_queue_send_from_isr(uart_os_queue_t q, const void* item)
{
    BaseType_t woken = pdFALSE;
    BaseType_t r = xQueueSendFromISR((QueueHandle_t)q, item, &woken);
    portYIELD_FROM_ISR(woken);
    return (pdTRUE == r) ? 0 : -1;
}

/* 全局唯一实例，供中间件注入 */
os_interface_t g_uart_os_freertos = {
    .pf_os_queue_create       = freertos_queue_create,
    .pf_os_queue_send         = freertos_queue_send,
    .pf_os_queue_recv         = freertos_queue_recv,
    .pf_os_queue_send_from_isr = freertos_queue_send_from_isr,
};
