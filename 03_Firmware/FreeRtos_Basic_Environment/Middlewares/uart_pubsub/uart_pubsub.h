#ifndef UART_PUBSUB_H
#define UART_PUBSUB_H



/* USER CODE BEGIN Includes */
#include <stdint.h>
/* USER CODE BEGIN Includes */

/*Define Macro*/
#define UART_PUBSUB_MAX_SUBSCRIBERS    4U
#define INDEFINITE_TIME                (0xFFFFFFFFUL)
#define RING_BUFFER_SIZE 10
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
    uint32_t                             head;
    uint32_t                             tail;
}ring_buffer_t;


typedef struct {
    void (*uart_init)(void);
    void (*uart_deinit)(void);
    void (*uart_start)(void);
    void (*uart_get_counter)(void);
}uart_ops_t;


typedef struct {

    void (*os_task_create)(void (*task_code)(void),
                           char *const task_name,
                           const uint32_t StackDepth,
                           void *argument,
                           uint16_t task_priority,
                           void *task_handler);

    void (*os_queue_create)(uint32_t queue_length,uint32_t queue_item_size);
    
    void (*os_queue_receive)(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime);

    void (*os_queue_send)(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime);
    
    void (*os_queue_send_isr)(void* queue_hendler,uint8_t* pvBuffer);

}os_ops_t;

typedef struct {
    ring_buffer_t *ring_buffer;    
    void (*frame_parse_alog)(void);
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


typedef struct {
    uart_ops_t*          uart_ops;
    os_ops_t  *            os_ops;
    frame_parse_t*    frame_parse;
}uart_need_arg_t;


typedef struct uart_handler {
    uart_need_arg_t*     need_arg;
    uart_pubsub_t*     subscriber;

    void  (*uart_pubsub_init)(struct uart_handler* self);

}uart_handler_t;




/**
 * @brief 
 *
 * @param pubsub
 */
void uart_pubsub_init(uart_pubsub_t *pubsub);

uart_handler_status_t uart_handler_init(uart_handler_t* self,uart_need_arg_t* need_arg);

/**
 * @brief 
 *
 * @param pubsub
 * @param callback
 * @param user_ctx
 *
 * @return 
 */
uart_pubsub_status_t uart_pubsub_subscribe(
    uint8_t cmd,
    uart_pubsub_t *pubsub,
    uart_frame_callback_t callback,
    void *user_ctx
);

/**
 * @brief 
 *
 * @param pubsub
 * @param payload
 * @param len
 */
void uart_pubsub_publish(
    uint8_t cmd,
    uart_pubsub_t *pubsub,
    const uint8_t *payload,
    uint16_t len
);




#endif
