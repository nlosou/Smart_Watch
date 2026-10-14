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
