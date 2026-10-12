#include "uart_parse_task.h"
#include "bsp_uart_driver.h"
#include "uart_port.h"
#include "elog.h"

QueueHandle_t queue_front_to_backend = NULL;

static ring_buffer_t *ring_buffer = NULL;
static frame_parse_att_t g_frame_parse_att;

uart_proto_t g_uart_proto;

extern uart_ops_t g_uart_ops;
extern os_interface_t g_uart_os_freertos;

static void on_frame_for_log(
    const uint8_t *frame,
    uint16_t len,
    void *user_ctx)
{
    (void)frame;
    (void)user_ctx;

    elog_i("UART", "Valid frame, payload length=%u",
           (unsigned int)len);
}

static void on_frame_for_command(
    const uint8_t *frame,
    uint16_t len,
    void *user_ctx)
{
    (void)user_ctx;

    if ((NULL == frame) || (0 == len)) {
        return;
    }

    /*
     * 在这里解释应用层命令。
     * 例如，根据 frame[0] 判断命令类型，
     * 再将命令发送给相应的应用任务。
     */
}

void Uart_rec_A_task(void *argument)
{
    (void)argument;

    uint32_t event;

    /* 1. 创建原有的 BSP -> 后端通知队列 */
    queue_front_to_backend =
        xQueueCreate(1, sizeof(uint32_t));

    if (NULL == queue_front_to_backend) {
        elog_e("UART", "Create backend queue failed");
        return;
    }

    /*
     * 2. 等待 BSP 创建环形缓冲区。
     * 更好的最终设计是让 BSP 在启动 DMA 前就完成初始化。
     */
    while (NULL == (ring_buffer =
                        uart_driver_get_ring_buffer_address())) {
        vTaskDelay(1);
    }

    /* 3. 准备协议层依赖 */
    g_frame_parse_att.ring_buffer = ring_buffer;
    g_frame_parse_att.pf_parsing_algo_fuc = NULL;

    uart_proto_input_arg_t arg = {
        .uart_ops = &g_uart_ops,
        .os_interface = &g_uart_os_freertos,
        .frame_parse_att = &g_frame_parse_att
    };

    if (uart_fuc_ok != uart_proto_init(&g_uart_proto, &arg)) {
        elog_e("UART", "Protocol init failed");
        return;
    }

    /* 4. 注册订阅者 */
    if (uart_fuc_ok != uart_proto_register_subscriber(
                           &g_uart_proto,
                           on_frame_for_log,
                           NULL)) {
        elog_e("UART", "Register log subscriber failed");
        return;
    }

    if (uart_fuc_ok != uart_proto_register_subscriber(
                           &g_uart_proto,
                           on_frame_for_command,
                           NULL)) {
        elog_e("UART", "Register command subscriber failed");
        return;
    }

    /* 5. 创建独立的协议解析任务 */
    if (pdPASS != xTaskCreate(
                      uart_proto_task,
                      "UartProto",
                      512,
                      &g_uart_proto,
                      tskIDLE_PRIORITY + 2,
                      NULL)) {
        elog_e("UART", "Create protocol task failed");
        return;
    }

    /* 6. 把原有的数据到达事件转发给协议任务 */
    for (;;) {
        if (pdTRUE == xQueueReceive(
                          queue_front_to_backend,
                          &event,
                          portMAX_DELAY)) {
            (void)uart_proto_notify_rx_data(&g_uart_proto);
        }
    }
}
