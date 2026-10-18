/**
 * @file uart_pubsub.h
 * @brief UART 发布-订阅中间件对外接口定义。
 *
 * 该模块抽象底层 UART/DMA 收发、RTOS 任务和队列接口，
 * 向上层提供基于命令字的帧订阅与发布能力。
 */
#ifndef UART_PUBSUB_H
#define UART_PUBSUB_H



/* USER CODE BEGIN Includes */
/* 标准库头文件：提供固定宽度整数、size_t、bool 以及内存操作接口 */
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include "elog.h"


/* USER CODE BEGIN Includes */

/*Define Macro*/
/**
 * @brief 模块配置宏定义。
 *
 * 以下参数应根据实际 RTOS、UART/DMA 配置以及产品需求进行核对。
 */
#define UART_PUBSUB_MAX_SUBSCRIBERS    4U
/** 同一命令字最多允许的订阅者数量 */
#define INDEFINITE_TIME                (0xFFFFFFFFUL)
/** 队列/信号量接收时的无限等待时间，具体单位由底层 OS 接口定义 */
#define RING_BUFFER_SIZE 32
/** UART DMA 接收环形缓冲区长度，需与底层 DMA 缓冲配置保持一致 */
/*Define Macro*/

/*定义函数类型*/

/* 每收到一帧有效数据时，调用一次订阅者回调 */
/**
 * @brief 有效 UART 帧回调函数类型。
 *
 * @param payload 帧负载数据指针
 * @param len     帧负载长度（字节）
 * @param user_ctx 订阅时传入的用户上下文
 */
typedef void (*uart_frame_callback_t)(
    const uint8_t *payload,
    uint16_t len,
    void *user_ctx
);

/*定义函数类型*/

/**
 * @brief 发布-订阅接口通用返回状态。
 */
typedef enum {
    UART_PUBSUB_OK = 0,
    UART_PUBSUB_ERROR = -1
} uart_pubsub_status_t;

/**
 * @brief UART handler 接口通用返回状态。
 */
typedef enum {
    UART_HANDLER_OK = 0,
    UART_HANDLER_ERROR = -1
} uart_handler_status_t;




/**
 * @brief 底层 UART DMA 接收环形缓冲区描述。
 */
typedef struct {
    /** 环形缓冲区实际存储区域 */
    uint8_t     ring_buffer[RING_BUFFER_SIZE];
    /** 生产侧指针：由 ISR 依据 DMA 接收进度推进 */
    volatile uint32_t                             head;
    /** 消费侧指针：由帧解析逻辑推进 */
    volatile uint32_t                             tail;
}ring_buffer_t;


/**
 * @brief UART 硬件抽象操作集合。
 *
 * 由具体平台/驱动层注册，模块通过这些接口完成串口初始化和收发管理。
 */
typedef struct {
    /** UART 外设初始化 */
    void (*uart_init)(void);
    /** UART 外设去初始化 */
    void (*uart_deinit)(void);
    /** 启动 UART/DMA 接收 */
    void (*uart_start)(void);
    /** 获取 DMA 剩余字节计数 */
    uint32_t (*uart_get_counter)(void);
}uart_ops_t;


/**
 * @brief RTOS 抽象操作集合。
 *
 * 将任务、队列、内存等 OS 能力抽象为函数指针，便于在不同 RTOS 间移植。
 */
typedef struct {

    /** 创建任务 */
    void (*os_task_create)(void (*task_code)(void*),
                           char *const task_name,
                           const uint32_t StackDepth,
                           void *const argument,
                           uint32_t task_priority,
                           void **const task_handler);

    /** 创建队列 */
    void (*os_queue_create)(uint32_t queue_length,uint32_t queue_item_size,void **const queue_handler);
    
    /** 阻塞接收队列数据 */
    void (*os_queue_receive)(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime);

    /** 向队列发送数据 */
    void (*os_queue_send)(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime);
    
    /** 从中断上下文向队列发送数据 */
    void (*os_queue_send_isr)(void* queue_hendler,uint8_t* pvBuffer);
    /** 动态内存申请 */
    void* (*os_malloc)(uint32_t size);
    /** 动态内存释放 */
    void (*os_free)(void*pv);

}os_ops_t;

/**
 * @brief 帧解析器描述。
 *
 * ring_buffer 由 UART ISR 写入，解析任务通过 frame_parse_alog 提取完整帧。
 */
typedef struct {
    ring_buffer_t *ring_buffer;    
    /** 从环形缓冲区中解析完整帧并写入 payload_buf */
    void (*frame_parse_alog)(ring_buffer_t*,uint8_t* payload_buf,uint8_t* payload_cnt);
}frame_parse_t;


/* 单个订阅者 */
/**
 * @brief 单个 UART 帧订阅者描述。
 */
typedef struct {
    /** 需要匹配的帧命令字 */
    uint8_t cmd;
    /** 匹配成功后调用的回调 */
    uart_frame_callback_t callback;
    /** 用户自定义上下文，回调时原样传入 */
    void *user_ctx;
} uart_subscriber_t;

/* 发布-订阅对象 */
/**
 * @brief 发布-订阅订阅表。
 */
typedef struct {
    /** 订阅者数组 */
    uart_subscriber_t subscribers[UART_PUBSUB_MAX_SUBSCRIBERS];
    /** 当前已注册的订阅者数量 */
    uint8_t count;
} uart_pubsub_t;

/** 前向声明，具体定义位于 uart_pubsub.c 中 */
typedef struct uart_proto_priv_data uart_proto_priv_data_t;

/**
 * @brief uart_handler 初始化所需的依赖参数集合。
 */
typedef struct {
    uart_ops_t*          uart_ops;
    os_ops_t  *            os_ops;
    frame_parse_t*    frame_parse;
}uart_need_arg_t;


/**
 * @brief UART 发布-订阅核心句柄。
 *
 * 聚合硬件/OS 抽象接口、私有数据和订阅表，并对上层暴露订阅/发布方法。
 */
typedef struct uart_handler {
    /** 模块依赖的外部接口集合 */
    uart_need_arg_t*                     need_arg;
    /** UART 协议私有数据 */
    uart_proto_priv_data_t  *uart_proto_priv_data; 

    /** 订阅表 */
    uart_pubsub_t*     subscriber;

    /** 订阅接口 */
    uart_pubsub_status_t (*uart_pubsub_subscribe)(
        struct uart_handler* self,
        uint8_t cmd,
        uart_frame_callback_t callback,
        void *user_ctx
    );

    /** 发布接口 */
    uart_pubsub_status_t (*uart_pubsub_publish)(
        struct uart_handler* self,
        uint8_t cmd,
        const uint8_t *payload,
        uint16_t len
    );



}uart_handler_t;


/**
 * @brief 初始化 UART 发布-订阅模块。
 *
 * @param self    待初始化的 uart_handler 句柄
 * @param need_arg 模块依赖的 UART、OS 和帧解析接口
 *
 * @return UART_HANDLER_OK 成功；UART_HANDLER_ERROR 失败
 */
uart_handler_status_t uart_handler_init(uart_handler_t* self,uart_need_arg_t* need_arg);

/**
 * @brief UART 接收中断通知回调。
 *
 * 该函数应在 UART/DMA 接收中断上下文中调用，
 * 用于更新环形缓冲区写指针并唤醒解析任务。
 *
 * @param self uart_handler 句柄
 * @return UART_HANDLER_OK 成功；UART_HANDLER_ERROR 失败
 */
uart_handler_status_t notify_isr_cb(uart_handler_t *const self);


/**
 * @brief 获取发布任务所使用的队列句柄地址。
 *
 * @param self uart_handler 句柄
 * @return 队列句柄地址
 */
void *get_queue_address(uart_handler_t *const self);

#endif
