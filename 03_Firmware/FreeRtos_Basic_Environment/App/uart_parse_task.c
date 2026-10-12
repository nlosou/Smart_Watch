/* USER CODE BEGIN Includes */
/* Includes ------------------------------------------------------------------*/
#include "uart_parse_task.h"
#include "elog.h"
#include "bsp_uart_driver.h"
#include "uart_port.h"

/* Includes ------------------------------------------------------------------*/


/* USER CODE END Includes */


/*emun Variables*/


/* Globle Variables */
QueueHandle_t queue_front_to_backend= NULL;
ring_buffer_t* ring_buffer;

extern uart_ops_t g_uart_ops;
/* Globle Variables */
static uart_proto_fuc_ret_state on_frame_ready(struct uart_proto_t* const self);
uart_proto_t g_uart_proto;

/**
  * @brief  Function implementing the Task_A thread.
  * @param  argument: Not used
  * @retval None
  */
void Uart_rec_A_task(void* argument)
{
    uint32_t front_to_backend_parten = 0;
    queue_front_to_backend = xQueueCreate(1,4); 
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


    /* 1. 依赖注入 */
    uart_proto_input_arg_t arg = {
        .uart_ops        = &g_uart_ops,
        .os_interface    = NULL,
        .frame_parse_att = {
        }
    };
    uart_proto_init(&g_uart_proto, &arg);

    /* 2. 注册订阅者：把"帧就绪时该干什么"登记进去 */
    uart_proto_register_subscriber(&g_uart_proto, on_frame_ready);
    for(;;)
    {
        vTaskDelay(portMAX_DELAY);
    }
}
