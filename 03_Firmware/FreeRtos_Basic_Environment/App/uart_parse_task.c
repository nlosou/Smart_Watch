/* USER CODE BEGIN Includes */
#include "uart_parse_task.h"
#include "elog.h"
#include "mid_ring_buffer.h"

#include "bsp_uart_driver.h"
/* USER CODE END Includes */



/*Define Macro*/

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
/* Globle Variables */


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
    for(;;)
    {
        //1.从队列里接收"开始处理数据"通知
        if(pdTRUE == xQueueReceive(
                    queue_front_to_backend,
                    &front_to_backend_parten,
                    portMAX_DELAY))
        {
            elog_d("backend","queue_front to backend is comming [%x]",front_to_backend_parten);
            //2.开始处理数据(暂时用打印环形缓冲区间数据代替)
            
            //进入状态机
            static uart_protocol_frame_status_t frame_status = FRAME_HEAD_NOT_DECTED;
            static uint8_t temp_frame_data[32] = {0};
            static uint8_t             frame_idx = 0;
            static uint32_t            frame_sum = 0;
            while(0x00!=ring_buffer_is_empty(ring_buffer))
            {           
                if(0x00 == get_data(ring_buffer,&temp_data))
                {
                    //elog_i("backend","get_data is [%x]",temp_data);
                    
                }
                else
                {
                    elog_e("backend","get_data is falsed");
                }
                vTaskDelay(5);
                switch(frame_status)
                {
                    //1.如果检测到某一字节数据为0xFE,就说明是帧头
                    case FRAME_HEAD_NOT_DECTED:
                        if(temp_data == FRAME_HEAD_FLAG)
                        {
                            frame_status = FRAME_HEAD;
                            elog_i("backend","frame start");
                        }
                        break;
                    //2.开始连续打印数据,直到遇到0xFF帧尾巴,或者环形缓存区为空
                    case FRAME_HEAD:
                        if(temp_data == FRAME_TAIL_FLAG)
                        {
                            frame_status = FRAME_HEAD_NOT_DECTED;
                            elog_i("backend","frame end");
                            //计算校验和
                            for(uint8_t idx = 0 ; idx <( frame_idx - 1); idx++) 
                            {
                                frame_sum+=temp_frame_data[idx];
                            }
                            //验证成功才打印数据
                            if(frame_sum == temp_frame_data[frame_idx - 1])
                            {
                                elog_i("INFO","Data verification successful");
                                for(uint8_t idx = 0 ; idx <( frame_idx - 1); idx++) 
                                {

                                    elog_i("backend",
                                   "frame data is [%x]",
                                           temp_frame_data[idx] );
                                }
                                  elog_i("backend",
                                   "verification data is  [%x]",
                                           frame_sum);
                            }
                            //验证失败
                            else
                            {
                                elog_e("INFO","Data verification falsed");
                                elog_e("INFO",
                                        "calculate sum is %x,get sum from frame is %x",
                                        frame_sum,temp_frame_data[frame_idx - 1]);
                            }
                            for(uint8_t idx = 0 ; idx <( frame_idx); idx++) 
                            {
                                temp_frame_data[idx] = 0;
                            }
                            frame_idx = 0;
                            frame_sum = 0;
                        }    
                        else
                        {
                            temp_frame_data[frame_idx++] = temp_data;
                            //elog_i("backend","frame data is [%x]",temp_data);
                        }
                        break;
                }
            }
            //elog_e("backend","ring buffer is empty");
        }
    }
}
