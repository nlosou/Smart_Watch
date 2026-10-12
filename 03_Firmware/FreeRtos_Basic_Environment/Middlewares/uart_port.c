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
uart_proto_fuc_ret_state uart_proto_init(
    uart_proto_t *self,
    uart_proto_input_arg_t *input_arg)
{
    if ((NULL == self) || (NULL == input_arg)) {
        return uart_fuc_null;
    }

    if ((NULL == input_arg->uart_ops) ||
        (NULL == input_arg->os_interface) ||
        (NULL == input_arg->frame_parse_att) ||
        (NULL == input_arg->frame_parse_att->ring_buffer)) {
        return uart_fuc_error;
    }

    self->uart_ops = input_arg->uart_ops;
    self->os_interface = input_arg->os_interface;
    self->frame_parse_att = input_arg->frame_parse_att;

    self->subscriber_count = 0;
    self->is_inited = 0;

    self->notify_queue =
        self->os_interface->pf_os_queue_create(1, sizeof(uint32_t));

    if (NULL == self->notify_queue) {
        return uart_fuc_error;
    }

    if (NULL != self->uart_ops->pf_uart_init) {
        self->uart_ops->pf_uart_init();
    }

    self->is_inited = 1;
    return uart_fuc_ok;
}


/**
 * @brief 
 *登记感兴趣的模块
 * @param self
 * @param callback
 * @param user_ctx
 *
 * @return 
 */
uart_proto_fuc_ret_state uart_proto_register_subscriber(
    uart_proto_t *self,
    uart_subscriber_cb_t callback,
    void *user_ctx)
{
    if ((NULL == self) || (NULL == callback)) {
        return uart_fuc_null;
    }

    if (self->subscriber_count >= UART_PROTO_MAX_SUBSCRIBERS) {
        return uart_fuc_error;
    }

    self->subscribers[self->subscriber_count].callback = callback;
    self->subscribers[self->subscriber_count].user_ctx = user_ctx;
    self->subscriber_count++;

    return uart_fuc_ok;
}
/**
 * @brief 
 *发布函数
 * @param self
 * @param frame
 * @param len
 */
static void uart_proto_publish_frame(
    uart_proto_t *self,
    const uint8_t *frame,
    uint16_t len)
{
    if ((NULL == self) || (NULL == frame)) {
        return;
    }

    for (uint8_t i = 0; i < self->subscriber_count; i++) {
        uart_subscriber_t *subscriber = &self->subscribers[i];

        if (NULL != subscriber->callback) {
            subscriber->callback(
                frame,
                len,
                subscriber->user_ctx
            );
        }
    }
}

/**
 * @brief 
 *
 *等待接收通知，解析 UART 数据
 * @param argument
 */
void uart_proto_task(void *argument)
{
    uart_proto_t *self = (uart_proto_t *)argument;

    if ((NULL == self) || (0 == self->is_inited)) {
        return;
    }

    ring_buffer_t *rb =
        (ring_buffer_t *)self->frame_parse_att->ring_buffer;

    uint8_t frame[UART_PROTO_MAX_FRAME_LEN];
    uint16_t frame_idx = 0;
    uint8_t in_frame = 0;
    uint32_t event = 0;

    for (;;) {
        /* 等待数据到达通知 */
        if (0 != self->os_interface->pf_os_queue_recv(
                     self->notify_queue,
                     &event,
                     UART_OS_WAIT_FOREVER)) {
            continue;
        }

        /* 一次通知后，把当前可读的字节全部取出 */
        while (ring_buffer_is_empty(rb) == 0x01) {
            uint8_t byte;

            if (0x00 != get_data(rb, &byte)) {
                break;
            }

            /* 等待帧头 */
            if (0 == in_frame) {
                if (0xFE == byte) {
                    in_frame = 1;
                    frame_idx = 0;
                }
                continue;
            }

            /* 遇到帧尾：校验并发布 */
            if (0xFF == byte) {
                if (frame_idx >= 1) {
                    uint8_t checksum = 0;

                    /* 最后一个字节是接收到的校验和 */
                    for (uint16_t i = 0; i < frame_idx - 1; i++) {
                        checksum =
                            (uint8_t)(checksum + frame[i]);
                    }

                    if (checksum == frame[frame_idx - 1]) {
                        /*
                         * 发布有效帧。
                         * 不把最后的校验和传给订阅者。
                         */
                        uart_proto_publish_frame(
                            self,
                            frame,
                            frame_idx - 1
                        );
                    }
                }

                in_frame = 0;
                frame_idx = 0;
                continue;
            }

            /* 防止超长帧写越界 */
            if (frame_idx < sizeof(frame)) {
                frame[frame_idx++] = byte;
            } else {
                /* 超长帧直接丢弃，重新寻找帧头 */
                in_frame = 0;
                frame_idx = 0;
            }
        }
    }
}

uart_proto_fuc_ret_state uart_proto_notify_rx_data(
    uart_proto_t *self)
{
    uint32_t event = 1;

    if ((NULL == self) || (0 == self->is_inited)) {
        return uart_fuc_error;
    }

    if (0 == self->os_interface->pf_os_queue_send(
                 self->notify_queue,
                 &event,
                 UART_OS_NO_WAIT)) {
        return uart_fuc_ok;
    }

    return uart_fuc_error;
}
