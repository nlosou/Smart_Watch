/**
 * @file uart_port.c
 * @brief uart中间件
 * @author Noser
 * @version 1.0
 * @date 2026-10-11
 */

/* Includes ------------------------------------------------------------------*/
#include "uart_port.h"
#include "elog.h"
#include "mid_ring_buffer.h"
/* Includes ------------------------------------------------------------------*/


/*emun Variables*/
typedef enum {
    FRAME_HEAD_NOT_DECTED,
    FRAME_HEAD,
}uart_protocol_frame_status_t;

/*emun Variables*/

/*Define Macro*/

#define FRAME_HEAD_FLAG (0xFE)
#define FRAME_TAIL_FLAG (0xFF)

/*Define Macro*/




/**
 * @brief  初始化 UART 中间件对象
 * @param  self       : 由上层静态/动态分配的 uart_proto_t 实例
 * @param  input_arg  : 依赖注入（ops / os / ring_buffer / 解析算法）
 * @retval uart_fuc_ok / uart_fuc_error / uart_fuc_null
 */
uart_proto_fuc_ret_state uart_proto_init(uart_proto_t* self,
                                         uart_proto_input_arg_t* input_arg)
{
    if (NULL == self || NULL == input_arg)       return uart_fuc_null;
    if (NULL == input_arg->uart_ops)             return uart_fuc_error;
    if (NULL == input_arg->os_interface)         return uart_fuc_error;  /* 需要 OS 抽象 */

    self->uart_ops        = input_arg->uart_ops;
    self->os_interface    = input_arg->os_interface;
    self->frame_parse_att = input_arg->frame_parse_att;
    self->uart_subcriber  = NULL;
    self->is_inited       = 0;

    /* 由中间件自己创建通知队列 —— 因为它是队列的消费者 */
    self->notify_queue = self->os_interface->pf_os_queue_create(1, sizeof(uint32_t));
    if (NULL == self->notify_queue) {
        return uart_fuc_error;
    }

    if (NULL != self->uart_ops->pf_uart_init) {
        self->uart_ops->pf_uart_init();
    }

    self->is_inited = 1;
    return uart_fuc_ok;
}


uart_proto_fuc_ret_state uart_proto_register_subscriber(uart_proto_t *self,
                                                       uart_subcriber_f cb)
{
    if (NULL == self || NULL == cb) return uart_fuc_null;
    self->uart_subcriber = cb;
    return uart_fuc_ok;
}

void uart_proto_task(void* argument)
{
    uart_proto_t* self = (uart_proto_t*)argument;

    uart_protocol_frame_status_t frame_status =FRAME_HEAD_NOT_DECTED;
    static uint8_t temp_frame_data[32] = {0};
    static uint8_t             frame_idx = 0;
    static uint32_t            frame_sum = 0;
    int32_t                  pattern = 0;
    data_type_t               temp_data;

    for (;;) {
        /* 1. 等"数据来了"的通知 —— 走抽象接口 */
        if (0 != self->os_interface->pf_os_queue_recv(
                     self->notify_queue,
                     &pattern,
                     UART_OS_WAIT_FOREVER)) {
            continue;
        }

        /* 2. 状态机主体完全不变 */
        while (ring_buffer_is_empty(self->frame_parse_att->ring_buffer) == 0x01) {
            if (0x00 != get_data(self->frame_parse_att->ring_buffer, &temp_data)) {
                break;
            }
            /* ... 和原来一模一样 ... */
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
                                elog_i("INFO",
                                        "Data verification successful");
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


