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
    uint8_t                      *ring_buffer;
    uint32_t                             head;
    uint32_t                             tail;
}ring_buffer_t;


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
typedef uart_proto_fuc_ret_state (*uart_subcriber_f)(struct uart_proto_t* const self);

/* ---------- 中间件对象 ---------- */
typedef struct uart_proto_t {

    uart_ops_t*                  uart_ops;
    os_interface_t*          os_interface;
    frame_parse_att_t*    frame_parse_att;
    uart_os_queue_t          notify_queue;  

    /* 运行时状态 */
    uint8_t              is_inited;

    /* 订阅者回调——上层注册进来 */
    uart_subcriber_f     uart_subcriber;
} uart_proto_t;

/* ---------- API ---------- */
uart_proto_fuc_ret_state uart_proto_init(uart_proto_t *self,
                                         uart_proto_input_arg_t* input_arg);

uart_proto_fuc_ret_state uart_proto_register_subscriber(
                                         uart_proto_t *self,
                                         uart_subcriber_f cb);

#endif
