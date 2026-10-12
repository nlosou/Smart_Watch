#include "uart_pubsub.h"

#include <stddef.h>
#include <string.h>

void uart_pubsub_init(uart_pubsub_t *pubsub)
{
    if (NULL == pubsub) {
        return;
    }

    memset(pubsub, 0, sizeof(*pubsub));
}

uart_pubsub_status_t uart_pubsub_subscribe(
    uint8_t cmd,
    uart_pubsub_t *pubsub,
    uart_frame_callback_t callback,
    void *user_ctx)
{
    if ((NULL == pubsub) || (NULL == callback)) {
        return UART_PUBSUB_ERROR;
    }

    /* 避免重复注册相同的回调和上下文 */
    for (uint8_t i = 0; i < pubsub->count; i++) {
        if ((pubsub->subscribers[i].callback == callback) &&
            (pubsub->subscribers[i].user_ctx == user_ctx)) {
            return UART_PUBSUB_OK;
        }
    }

    if (pubsub->count >= UART_PUBSUB_MAX_SUBSCRIBERS) {
        return UART_PUBSUB_ERROR;
    }

    pubsub->subscribers[pubsub->count].cmd = cmd;
    pubsub->subscribers[pubsub->count].callback = callback;
    pubsub->subscribers[pubsub->count].user_ctx = user_ctx;
    pubsub->count++;

    return UART_PUBSUB_OK;
}

void uart_pubsub_publish(
    uint8_t cmd,
    uart_pubsub_t *pubsub,
    const uint8_t *data,
    uint16_t len)
{
    if (NULL == pubsub) {
        return;
    }

    for (uint8_t i = 0; i < pubsub->count; i++) {
        uart_subscriber_t *sub = &pubsub->subscribers[i];

        if ((sub->cmd == cmd) && (NULL != sub->callback)) {
            sub->callback(data, len, sub->user_ctx);
        }
    }
}

uart_handler_status_t uart_handler_init(uart_handler_t* self,uart_need_arg_t* need_arg)
{
    if(NULL==self || NULL == need_arg)
    {
       return UART_HANDLER_ERROR; 
    }

    self->need_arg->uart_ops       = need_arg->uart_ops;
    self->need_arg->os_ops         = need_arg->os_ops;
    self->need_arg->frame_parse    = need_arg->frame_parse;
    

    return UART_HANDLER_OK;
}
