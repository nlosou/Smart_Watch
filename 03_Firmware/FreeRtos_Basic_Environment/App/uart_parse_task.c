/* USER CODE BEGIN Includes */
#include "uart_parse_task.h"
#include "elog.h"
#include "mid_ring_buffer.h"
#include "uart_pubsub.h"
#include "bsp_uart_driver.h"
/* USER CODE END Includes */




/*Define Macro*/

#define UART_FRAME_MAX_LEN  32U
#define FRAME_HEAD_FLAG (0xFE)
#define FRAME_TAIL_FLAG (0xFF)

/*Define Macro*/

/*emun Variables*/
typedef enum {
    FRAME_HEAD_NOT_DECTED,
    FRAME_HEAD,
}uart_protocol_frame_status_t;

/*emun Variables*/


/* Globle Variables */
QueueHandle_t queue_front_to_backend= NULL;
ring_buffer_t* ring_buffer;
static uart_pubsub_t g_uart_pubsub;
/* Globle Variables */

static void uart_log_subscriber(
    const uint8_t *payload,
    uint16_t len,
    void *user_ctx);

static void uart_command_subscriber(
    const uint8_t *payload,
    uint16_t len,
    void *user_ctx);

/**
  * @brief  Function implementing the Task_A thread.
  * @param  argument: Not used
  * @retval None
  */
void Uart_rec_A_task(void* argument)
{
    uint32_t front_to_backend_parten = 0;
    queue_front_to_backend = xQueueCreate(1,4); 

    uint8_t frame_buf[UART_FRAME_MAX_LEN];
    uint16_t frame_len = 0;
    uint8_t in_frame = 0;
    uint8_t temp_data = 0;
    if(NULL!=queue_front_to_backend)
    {

        elog_i("Task_A","front_to_backend is created successfully");
    }
    else
    {

        elog_e("Task_A","front_to_backend is created false");
        return;
    }
    
    ring_buffer = uart_driver_get_ring_buffer_address();
    elog_i("Task_A","Task_A is active");
    uart_pubsub_init(&g_uart_pubsub);

    if (UART_PUBSUB_OK != uart_pubsub_subscribe(
                               0x01,
                               &g_uart_pubsub,
                               uart_log_subscriber,
                               NULL)) {
        elog_e("UART", "Register log subscriber failed");
        return;
    }

    if (UART_PUBSUB_OK != uart_pubsub_subscribe(
                               0x02,
                               &g_uart_pubsub,
                               uart_command_subscriber,
                               NULL)) {
        elog_e("UART", "Register command subscriber failed");
        return;
    }

    for(;;)
    {
        //1.从队列里接收"开始处理数据"通知
        while (0x01 == ring_buffer_is_empty(ring_buffer)) 
        {
            if (0x00 != get_data(ring_buffer, &temp_data)) {
                break;
            }
         // 帧头 | CMD | DATA | CHECKSUM | 帧尾
            /* 尚未找到帧头 */
            if (0U == in_frame) {
                if (FRAME_HEAD_FLAG == temp_data) {
                    in_frame = 1U;
                    frame_len = 0U;
                }
                continue;
            }

            /* 收到帧尾：验证校验和，然后发布 */
            if (FRAME_TAIL_FLAG == temp_data) {
                if (frame_len >= 2U) {
                    uint8_t checksum = 0U;

                    /*
                     * frame_buf 最后一个字节是校验和，
                     * 前面的字节是载荷。
                     */
                    for (uint16_t i = 0U; i < frame_len - 1U; i++) {
                        checksum =
                            (uint8_t)(checksum + frame_buf[i]);
                    }

                    if (checksum == frame_buf[frame_len - 1U]) {
                        /* 关键：只发布校验成功的载荷 */
                        uart_pubsub_publish(
                            frame_buf[0],
                            &g_uart_pubsub,
                            frame_buf,
                            frame_len - 1U
                        );
                    } else {
                        elog_w("UART", "Frame checksum failed");
                    }
                }

                in_frame = 0U;
                frame_len = 0U;
                continue;
            }

            /* 暂存帧内容，并防止数组越界 */
            if (frame_len < sizeof(frame_buf)) {
                frame_buf[frame_len++] = temp_data;
            } else {
                /* 超长帧直接丢弃，重新寻找帧头 */
                in_frame = 0U;
                frame_len = 0U;
            }
        }   
    }
}


/* 订阅者一：记录有效帧 */
static void uart_log_subscriber(
    const uint8_t *payload,
    uint16_t len,
    void *user_ctx)
{
    (void)payload;
    (void)user_ctx;

    elog_i("UART", "subscriber Valid frame, payload len=%u",
           (unsigned int)len);
}

/* 订阅者二：处理应用层命令 */
static void uart_command_subscriber(
    const uint8_t *payload,
    uint16_t len,
    void *user_ctx)
{
    (void)user_ctx;

    if ((NULL == payload) || (0U == len)) {
        return;
    }

    /*
     * 可以在这里根据命令字分发对应的处理逻辑。
     */

    elog_i("UART","subscriber2 %x",payload[0]);
}
