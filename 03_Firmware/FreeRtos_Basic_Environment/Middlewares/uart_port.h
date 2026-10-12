#ifndef UART_PORT_H
#define UART_PORT_H

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Includes ------------------------------------------------------------------*/

/* Define macro ------------------------------------------------------------*/

#define RING_BUFFER_SIZE 10
typedef uint8_t data_type_t;
typedef void (*TaskFunction_t)( void * );
typedef void*   uart_os_queue_t;
typedef uint32_t uart_os_tick_t;

#define UART_OS_WAIT_FOREVER   ((uart_os_tick_t)0xFFFFFFFFU)
#define UART_OS_NO_WAIT        ((uart_os_tick_t)0U)

#define UART_PROTO_MAX_SUBSCRIBERS  4
#define UART_PROTO_MAX_FRAME_LEN    32

/* Define macro ------------------------------------------------------------*/



/*return value-------------------------------------------------------------*/
typedef enum  {
    uart_fuc_ok ,
    uart_fuc_error,
    uart_fuc_busy,
    uart_fuc_null
}uart_proto_fuc_ret_state;

/*return value-------------------------------------------------------------*/

typedef struct {
    uint8_t ring_buffer[RING_BUFFER_SIZE];
    uint32_t head;
    uint32_t tail;
} ring_buffer_t;

typedef struct {
    void *ring_buffer; 
    uart_proto_fuc_ret_state (*pf_parsing_algo_fuc)(uint8_t byte, void* ctx);
}frame_parse_att_t;

/**
 * @brief uart 函数表
 */
typedef struct {
    /**
     * @brief 
     *   开启uart的DMA模式
     * @param 
     */
    void (*pf_uart_init)       (void);
    void (*pf_uart_deinit)     (void);
    void (*pf_uart_send)       (void);
    void (*pf_uart_receive)    (void);
    uint16_t (*pf_uart_get_counter)(void);
}uart_ops_t;


/**
 * @brief os函数
 */
typedef struct {
    /* 队列 */
    uart_os_queue_t (*pf_os_queue_create)(uint32_t len, uint32_t item_size);
    int             (*pf_os_queue_send)(uart_os_queue_t q,
                                        const void* item,
                                        uart_os_tick_t timeout);
    int             (*pf_os_queue_recv)(uart_os_queue_t q,
                                        void* item,
                                        uart_os_tick_t timeout);
    /* 中断版本（你的 uart_proto_task 里用不到，但 BSP 层要用） */
    int             (*pf_os_queue_send_from_isr)(uart_os_queue_t q,
                                                 const void* item);


}os_interface_t;

/* ---------- 初始化入参 ---------- */
typedef struct
{
    frame_parse_att_t   *frame_parse_att;  /* Pointer to frame parsing attributes */
    uart_ops_t          *uart_ops;         /* Pointer to UART hardware operation interface */
    os_interface_t      *os_interface;     /* Pointer to OS abstraction interface */
    uart_os_queue_t      notify_queue;  
} uart_proto_input_arg_t;
/* ---------- 初始化入参 ---------- */



/* ---------- 前向声明 ---------- */
struct uart_proto_t;

/* 订阅者回调类型：一帧就绪时，中间件回调上层 */

/* 一个订阅者的回调函数 */
typedef void (*uart_subscriber_cb_t)(
    const uint8_t *frame,
    uint16_t len,
    void *user_ctx
);

/* 订阅者信息 */
typedef struct {
    uart_subscriber_cb_t callback;//回调函数
    void *user_ctx;//用户上下文
} uart_subscriber_t;

/* ---------- 中间件对象 ---------- */
typedef struct uart_proto_t {
    uart_ops_t *uart_ops;
    os_interface_t *os_interface;
    frame_parse_att_t *frame_parse_att;
    uart_os_queue_t notify_queue;

    uint8_t is_inited;

    /* 多个订阅者 */
    uart_subscriber_t subscribers[UART_PROTO_MAX_SUBSCRIBERS];
    uint8_t subscriber_count;

} uart_proto_t;


/* ---------- API ---------- */


/**
 * @brief 
 *
 * @param self
 * @param input_arg
 *
 * @return 
 */
uart_proto_fuc_ret_state uart_proto_init(uart_proto_t *self,

                                         uart_proto_input_arg_t* input_arg);

/**
 * @brief 
 *用户注册订阅登记感兴趣的模块
 * @param self
 * @param callback
 * @param user_ctx
 *
 * @return 
 */
uart_proto_fuc_ret_state uart_proto_register_subscriber(
    uart_proto_t *self,
    uart_subscriber_cb_t callback,
    void *user_ctx
);

/**
 * @brief 
 *等待接收通知，解析 UART 数据
 * @param argument
 */
void uart_proto_task(void *argument);

/**
 * @brief 
 *通知解析任务有新数据可读
 * @param self
 *
 * @return 
 */
uart_proto_fuc_ret_state uart_proto_notify_rx_data(
    uart_proto_t *self
);
#endif
