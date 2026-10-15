#ifndef UART_PUBSUB_H
#define UART_PUBSUB_H



/* USER CODE BEGIN Includes */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "elog.h"


/* USER CODE BEGIN Includes */

/*Define Macro*/
#define UART_PUBSUB_MAX_SUBSCRIBERS    4U
#define INDEFINITE_TIME                (0xFFFFFFFFUL)
#define RING_BUFFER_SIZE 32
/*Define Macro*/

/*定义函数类型*/

/* 每收到一帧有效数据时，调用一次订阅者回调 */
typedef void (*uart_frame_callback_t)(
    const uint8_t *payload,
    uint16_t len,
    void *user_ctx
);

/*定义函数类型*/

typedef enum {
    UART_PUBSUB_OK = 0,
    UART_PUBSUB_ERROR = -1
} uart_pubsub_status_t;

typedef enum {
    UART_HANDLER_OK = 0,
    UART_HANDLER_ERROR = -1
} uart_handler_status_t;




typedef struct {
    uint8_t     ring_buffer[RING_BUFFER_SIZE];
    volatile uint32_t                             head;
    volatile uint32_t                             tail;
}ring_buffer_t;


typedef struct {
    void (*uart_init)(void);
    void (*uart_deinit)(void);
    void (*uart_start)(void);
    uint32_t (*uart_get_counter)(void);
}uart_ops_t;


typedef struct {

    void (*os_task_create)(void (*task_code)(void*),
                           char *const task_name,
                           const uint32_t StackDepth,
                           void *const argument,
                           uint32_t task_priority,
                           void **const task_handler);

    void (*os_queue_create)(uint32_t queue_length,uint32_t queue_item_size,void **const queue_handler);
    
    void (*os_queue_receive)(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime);

    void (*os_queue_send)(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime);
    
    void (*os_queue_send_isr)(void* queue_hendler,uint8_t* pvBuffer);
    void* (*os_malloc)(uint32_t size);

}os_ops_t;

typedef struct {
    ring_buffer_t *ring_buffer;    
    void (*frame_parse_alog)(ring_buffer_t*,uint8_t* payload_buf,uint8_t* payload_cnt);
}frame_parse_t;


/* 单个订阅者 */
typedef struct {
    uint8_t cmd;
    uart_frame_callback_t callback;
    void *user_ctx;
} uart_subscriber_t;

/* 发布-订阅对象 */
typedef struct {
    uart_subscriber_t subscribers[UART_PUBSUB_MAX_SUBSCRIBERS];
    uint8_t count;
} uart_pubsub_t;

typedef struct uart_proto_priv_data uart_proto_priv_data_t;

typedef struct {
    uart_ops_t*          uart_ops;
    os_ops_t  *            os_ops;
    frame_parse_t*    frame_parse;
}uart_need_arg_t;


typedef struct uart_handler {
    uart_need_arg_t*                     need_arg;
    uart_proto_priv_data_t  *uart_proto_priv_data; 

    uart_pubsub_t*     subscriber;

    uart_pubsub_status_t (*uart_pubsub_subscribe)(
        struct uart_handler* self,
        uint8_t cmd,
        uart_frame_callback_t callback,
        void *user_ctx
    );

    uart_pubsub_status_t (*uart_pubsub_publish)(
        struct uart_handler* self,
        uint8_t cmd,
        const uint8_t *payload,
        uint16_t len
    );



}uart_handler_t;


/**
 * @brief 
 *
 * @param self
 * @param need_arg
 *
 * @return 
 */
uart_handler_status_t uart_handler_init(uart_handler_t* self,uart_need_arg_t* need_arg);

/**
 * @brief 中断
 *
 * @param self
 */
uart_handler_status_t notify_isr_cb(uart_handler_t *const self);

#endif
