/**
 * @file uart_pubsub.c
 * @brief UART 发布-订阅中间件实现。
 *
 * @author Noser
 * 负责管理 UART/DMA 环形缓冲区、解析任务和发布任务，
 * 并将接收到的完整帧按命令字分发到已注册订阅者。
 * @date 2026-10-18
 */

#include "uart_pubsub.h"



/** ISR 通知解析任务时写入队列的固定触发字节 */
#define INFO_TO_BACKEND 0xAA


/**
 * @brief UART 协议私有数据。
 *
 * 集中保存模块运行期间的队列句柄、解析缓冲区和各类计数状态。
 */
typedef struct uart_proto_priv_data
{
    bool                if_inited;         /* Initialization flag (true = initialized) */
    volatile uint16_t   parse_fail_cnt;
    void                *queue_handle;     /* RTOS queue handle for parsed frame info */
    void                *queue_handle2;     /* RTOS queue handle for parsed frame info */
    volatile uint16_t   data_counter;      /* DMA receive counter cache (remaining bytes) */
    volatile uint32_t   header;            /* Ring buffer read pointer (total bytes read) */
    volatile uint32_t   tail;              /* Ring buffer write pointer (total bytes parsed) */
    uint8_t             *parse_buf;        /* Temporary buffer for frame parsing (linearized ring data) */
    uint8_t    payload_cnt;       /* Counter for pending frames in queue */
    /** 解析任务输出的帧负载缓存区 */
    uint8_t    payload_buf[RING_BUFFER_SIZE];       /* Counter for pending frames in queue */
} uart_proto_priv_data_t;



/* 预留的订阅表初始化函数。
 * 当前订阅表在 uart_handler_init 中分配；若后续需要复位订阅表，
 * 可参考此实现补充调用。 */
/*
void  uart_pubsub_init(struct uart_handler* self)
{
    if (NULL == pubsub) {
        return;
    }

    memset(pubsub, 0, sizeof(*pubsub));
}
*/

/**
 * @brief 订阅指定命令字的 UART 帧。
 *
 * @param self     uart_handler 句柄
 * @param cmd      需要匹配的帧命令字
 * @param callback 匹配成功后调用的回调
 * @param user_ctx 回调时回传的用户上下文
 *
 * @return UART_PUBSUB_OK 成功；UART_PUBSUB_ERROR 参数非法或订阅表已满
 */
static uart_pubsub_status_t uart_pubsub_subscribe(
    uart_handler_t* self,
    uint8_t cmd,
    uart_frame_callback_t callback,
    void *user_ctx)
{
    if ((NULL == self) || (NULL == callback)) 
    {
        return UART_PUBSUB_ERROR;
    }
    uart_pubsub_t *pubsub = self->subscriber;
    

    /* 避免重复注册相同的回调和上下文 */
    for (uint8_t i = 0; i < pubsub->count; i++) {
        if ((pubsub->subscribers[i].callback == callback) &&
            (pubsub->subscribers[i].user_ctx == user_ctx)) {
            return UART_PUBSUB_OK;
        }
    }

    /* 订阅表已满，拒绝继续注册 */
    if (pubsub->count >= UART_PUBSUB_MAX_SUBSCRIBERS) {
        return UART_PUBSUB_ERROR;
    }

    /* 追加新的订阅项 */
    pubsub->subscribers[pubsub->count].cmd = cmd;
    pubsub->subscribers[pubsub->count].callback = callback;
    pubsub->subscribers[pubsub->count].user_ctx = user_ctx;
    pubsub->count++;

    return UART_PUBSUB_OK;
}

/**
 * @brief 向订阅了指定命令字的订阅者发布 UART 帧。
 *
 * @param self uart_handler 句柄
 * @param cmd  帧命令字
 * @param data 帧负载数据指针
 * @param len  帧负载长度（字节）
 *
 * @return UART_PUBSUB_OK 发布完成；UART_PUBSUB_ERROR 参数非法
 */
static uart_pubsub_status_t uart_pubsub_publish(
    uart_handler_t* self,
    uint8_t cmd,
    const uint8_t *data,
    uint16_t len)
{
    if ((NULL == self)) 
    {
        return UART_PUBSUB_ERROR;
    }
    uart_pubsub_t *pubsub = self->subscriber;
    
    /* 遍历订阅表，向所有命令字匹配的订阅者分发数据 */
    for (uint8_t i = 0; i < pubsub->count; i++) {
        uart_subscriber_t *sub = &pubsub->subscribers[i];

        if ((sub->cmd == cmd) && (NULL != sub->callback)) {
            sub->callback(data, len, sub->user_ctx);
        }
    }

    return UART_PUBSUB_OK;
}

/**
 * @brief UART 帧解析任务入口。
 *
 * 该任务阻塞等待 ISR 发送的队列通知，随后驱动底层帧解析算法，
 * 从环形缓冲区中提取完整帧。
 *
 * @param argument 实际传入 uart_handler_t 指针
 */
static void uart_parse_task(void* argument)
{
    if(NULL == argument)
    {
        return;
    }
    uint8_t pvBuffer = 0x23;
    uint8_t get_Buffer;
    uart_handler_t *current_uart_handler = (uart_handler_t*)argument;
    for(;;)
    {
        /* 阻塞等待 UART ISR 通过队列发来的数据到达通知 */
        current_uart_handler->need_arg->os_ops->os_queue_receive(
                (current_uart_handler->uart_proto_priv_data->queue_handle),
                &get_Buffer,
                INDEFINITE_TIME
                ); 

        elog_d("info","info come from isr");
        /* 从环形缓冲区中解析完整帧 */
        //解析
        current_uart_handler->need_arg->frame_parse->frame_parse_alog(
                current_uart_handler->need_arg->frame_parse->ring_buffer,
                current_uart_handler->uart_proto_priv_data->payload_buf,
                &current_uart_handler->uart_proto_priv_data->payload_cnt
                );
    }
}


/**
 * @brief UART 帧发布任务入口。
 *
 * 该任务等待解析完成通知，并将解析得到的帧通过发布接口分发给订阅者。
 *
 * @param argument 实际传入 uart_handler_t 指针
 */
static void uart_publish(void*argument)
{
    if(NULL == argument)
    {
        return;
    }
    uint8_t get_Buffer;
    uart_handler_t *current_uart_handler = (uart_handler_t*)argument;
    for(;;)
    {
    /* 等待解析任务完成一帧解析后写入队列的通知 */
    current_uart_handler->need_arg->os_ops->os_queue_receive(
                (current_uart_handler->uart_proto_priv_data->queue_handle2),
                &get_Buffer,
                INDEFINITE_TIME
                ); 
        elog_d("info","start publish come from parsed algo");
        /* 将 payload_buf 中缓存的帧负载发布给订阅者 */
        //发布
        current_uart_handler->uart_pubsub_publish(
                current_uart_handler,
                current_uart_handler->uart_proto_priv_data->payload_buf[1],
                current_uart_handler->uart_proto_priv_data->payload_buf + 2,
                current_uart_handler->uart_proto_priv_data->payload_buf[0]
                );
    
    }
}

/**
 * @brief 初始化 UART 发布-订阅模块。
 *
 * 依次完成私有数据和订阅表内存分配、两个 RTOS 队列创建，
 * 以及解析任务和发布任务的创建。
 *
 * @param self    待初始化的 uart_handler 句柄
 * @param need_arg 模块依赖的 UART、OS 和帧解析接口
 *
 * @return UART_HANDLER_OK 成功；UART_HANDLER_ERROR 失败
 */
uart_handler_status_t uart_handler_init(uart_handler_t* self,uart_need_arg_t* need_arg)
{
    if(NULL==self || NULL == need_arg)
    {
       return UART_HANDLER_ERROR; 
    }

    self->need_arg = need_arg;
    

    /* 分配并清零 UART 协议私有数据 */
    self->uart_proto_priv_data = self->need_arg->os_ops->os_malloc(sizeof(uart_proto_priv_data_t))  ;
    if(NULL == self->uart_proto_priv_data)
    {
        goto err_cleanup;
    }
    memset(self->uart_proto_priv_data,0,sizeof(uart_proto_priv_data_t));


    /* 分配订阅表并绑定发布-订阅方法 */
    self->subscriber            = self->need_arg->os_ops->os_malloc(sizeof(uart_pubsub_t));
    if(NULL == self->subscriber)
    {
        goto err_cleanup;
    }
    memset(self->uart_proto_priv_data,0,sizeof(uart_pubsub_t));

    self->uart_pubsub_subscribe = uart_pubsub_subscribe;
    self->uart_pubsub_publish   = uart_pubsub_publish;

    /* 创建 ISR -> 解析任务、解析任务 -> 发布任务 两个通知队列 */
     //创建队列
     
    self->need_arg->os_ops->os_queue_create(1,4,&(self->uart_proto_priv_data->queue_handle));
    self->need_arg->os_ops->os_queue_create(1,4,&(self->uart_proto_priv_data->queue_handle2));


    /* 创建解析串口任务 */
    //创建解析串口任务
    if(UART_OS_OK != self->need_arg->os_ops->os_task_create(uart_parse_task,    
                                          "uart_parse_task",
                                           128,
                                           self,
                                           9,
                                           NULL))
    {
        goto err_cleanup;
    }
    /* 创建发布任务 */
    //创建发布线程
    if(UART_OS_OK!=self->need_arg->os_ops->os_task_create(uart_publish,    
                                          "uart_publish",
                                           128,
                                           self,
                                           8,
                                           NULL))
    {
        goto err_cleanup;
    }

    /* 启动底层 UART/DMA 接收 */
    //开启串口DMA
    self->need_arg->uart_ops->uart_start();     

    //uart_pubsub_init(self->subscriber);
    
    return UART_HANDLER_OK;
err_cleanup:
    if(self->uart_proto_priv_data)
    {

        self->need_arg->os_ops->os_free(self->uart_proto_priv_data);
        self->uart_proto_priv_data = NULL;
    }

    if(self->subscriber)
    {

        self->need_arg->os_ops->os_free(self->subscriber);
        self->subscriber = NULL;
    }
    return UART_HANDLER_ERROR;
}

/**
 * @brief UART 接收中断通知回调。
 *
 * 根据 DMA 剩余字节数计算本次新增接收字节数，推进环形缓冲区写指针，
 * 并通过队列通知解析任务处理数据。该函数仅允许在中断上下文中调用。
 *
 * @param self uart_handler 句柄
 * @return UART_HANDLER_OK 成功；UART_HANDLER_ERROR 参数非法
 */
uart_handler_status_t notify_isr_cb(uart_handler_t *const self)
{
    if(NULL == self) 
    {
        return UART_HANDLER_ERROR;
    }
    static uint32_t last_dma_pos = 0;
    static uint8_t INFO_TO_BACKEND_PATTERN = INFO_TO_BACKEND;
    /* 记录上一次 DMA 已消费位置，用于计算本次新增字节数 */
    //1,获取当前head
     
    uint32_t current_head =self->need_arg->frame_parse->ring_buffer->head;

    //elog_d("INFO","current_head is [%d]",current_head);
    //2,当前dma到达的位置(dma 会不断在 0~RING_BUFFER_SIZE 之间徘徊)

    uint32_t current_data_pos =RING_BUFFER_SIZE - self->need_arg->uart_ops->uart_get_counter();

    /* 处理 DMA 地址回绕，计算本次需要推进的字节数 */
    //3,计算需要让head移动多少
    uint32_t need_move_len = 0 ;
    if(current_data_pos >= last_dma_pos)
    {
        need_move_len = current_data_pos - last_dma_pos;
    }
    else
    {

        /* DMA 位置发生回绕，需要补齐环形缓冲区整圈长度 */
        need_move_len = RING_BUFFER_SIZE + current_data_pos - last_dma_pos;
    
    }
    //elog_d("INFO","need_move_len is [%d]",need_move_len);
    //2,当前dma到达的位置(dma 会不断在 0~RING_BUFFER_SIZE 之间徘徊)
    //4,移动head
    //self->uart_proto_priv_data->header+=need_move_len;
    self->need_arg->frame_parse->ring_buffer->head+=need_move_len;
    /* 缓存本次 DMA 位置，供下次中断增量计算使用 */
    last_dma_pos = current_data_pos;
    //elog_d("INFO","current_head is [%d]",self->need_arg->frame_parse->ring_buffer->head);

    /* 通过队列唤醒解析任务 */
    //5通知解析任务
    self->need_arg->os_ops->os_queue_send_isr(
            self->uart_proto_priv_data->queue_handle,
            &INFO_TO_BACKEND_PATTERN
            ); 
    return UART_HANDLER_OK;
}



/**
 * @brief 获取发布任务所使用的队列句柄地址。
 *
 * @param self uart_handler 句柄
 * @return 队列句柄地址
 */
void *get_queue_address(uart_handler_t *const self)
{
    return  self->uart_proto_priv_data->queue_handle2;
}
