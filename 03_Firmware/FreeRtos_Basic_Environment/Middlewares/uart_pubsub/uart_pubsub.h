#ifndef UART_PUBSUB_H
#define UART_PUBSUB_H

#include <stdint.h>

#define UART_PUBSUB_MAX_SUBSCRIBERS    4U

/* 每收到一帧有效数据时，调用一次订阅者回调 */
typedef void (*uart_frame_callback_t)(
    const uint8_t *payload,
    uint16_t len,
    void *user_ctx
);

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

typedef enum {
    UART_PUBSUB_OK = 0,
    UART_PUBSUB_ERROR = -1
} uart_pubsub_status_t;


/**
 * @brief 
 *
 * @param pubsub
 */
void uart_pubsub_init(uart_pubsub_t *pubsub);

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
