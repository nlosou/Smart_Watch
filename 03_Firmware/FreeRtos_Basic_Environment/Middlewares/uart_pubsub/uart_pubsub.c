#include "uart_pubsub.h"



#define INFO_TO_BACKEND 0xAA


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
    uint8_t    payload_buf[RING_BUFFER_SIZE];       /* Counter for pending frames in queue */
} uart_proto_priv_data_t;



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
 * @brief 
 *
 * @param self
 * @param cmd
 * @param callback
 * @param user_ctx
 *
 * @return 
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

    if (pubsub->count >= UART_PUBSUB_MAX_SUBSCRIBERS) {
        return UART_PUBSUB_ERROR;
    }

    pubsub->subscribers[pubsub->count].cmd = cmd;
    pubsub->subscribers[pubsub->count].callback = callback;
    pubsub->subscribers[pubsub->count].user_ctx = user_ctx;
    pubsub->count++;

    return UART_PUBSUB_OK;
}

/**
 * @brief 
 *
 * @param self
 * @param cmd
 * @param data
 * @param len
 *
 * @return 
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
    
    for (uint8_t i = 0; i < pubsub->count; i++) {
        uart_subscriber_t *sub = &pubsub->subscribers[i];

        if ((sub->cmd == cmd) && (NULL != sub->callback)) {
            sub->callback(data, len, sub->user_ctx);
        }
    }

    return UART_PUBSUB_OK;
}

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
        current_uart_handler->need_arg->os_ops->os_queue_receive(
                (current_uart_handler->uart_proto_priv_data->queue_handle),
                &get_Buffer,
                INDEFINITE_TIME
                ); 

        elog_d("info","info come from isr");
        //解析
        current_uart_handler->need_arg->frame_parse->frame_parse_alog(
                current_uart_handler->need_arg->frame_parse->ring_buffer,
                current_uart_handler->uart_proto_priv_data->payload_buf,
                &current_uart_handler->uart_proto_priv_data->payload_cnt
                );
    }
}


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
    current_uart_handler->need_arg->os_ops->os_queue_receive(
                (current_uart_handler->uart_proto_priv_data->queue_handle2),
                &get_Buffer,
                INDEFINITE_TIME
                ); 
        elog_d("info","start publish come from parsed algo");
        //发布
        current_uart_handler->uart_pubsub_publish(
                current_uart_handler,
                current_uart_handler->uart_proto_priv_data->payload_buf[1],
                current_uart_handler->uart_proto_priv_data->payload_buf + 1,
                current_uart_handler->uart_proto_priv_data->payload_buf[0]
                );
    
    }
}

uart_handler_status_t uart_handler_init(uart_handler_t* self,uart_need_arg_t* need_arg)
{
    if(NULL==self || NULL == need_arg)
    {
       return UART_HANDLER_ERROR; 
    }

    self->need_arg = need_arg;
    

    self->uart_proto_priv_data = self->need_arg->os_ops->os_malloc(sizeof(uart_proto_priv_data_t))  ;
    if(NULL == self->uart_proto_priv_data)
    {
        return UART_HANDLER_ERROR;
    }
    memset(self->uart_proto_priv_data,0,sizeof(uart_proto_priv_data_t));


    self->subscriber            = self->need_arg->os_ops->os_malloc(sizeof(uart_pubsub_t));
    self->uart_pubsub_subscribe = uart_pubsub_subscribe;
    self->uart_pubsub_publish   = uart_pubsub_publish;

    //创建解析串口任务
    self->need_arg->os_ops->os_task_create(uart_parse_task,    
                                          "uart_parse_task",
                                           128,
                                           self,
                                           9,
                                           NULL);
    //创建发布线程
    self->need_arg->os_ops->os_task_create(uart_publish,    
                                          "uart_publish",
                                           128,
                                           self,
                                           8,
                                           NULL);

    //创建队列
    self->need_arg->os_ops->os_queue_create(1,4,&(self->uart_proto_priv_data->queue_handle));
    self->need_arg->os_ops->os_queue_create(1,4,&(self->uart_proto_priv_data->queue_handle2));

    
    //开启串口DMA
    self->need_arg->uart_ops->uart_start();     

    //uart_pubsub_init(self->subscriber);
    

    return UART_HANDLER_OK;
}

uart_handler_status_t notify_isr_cb(uart_handler_t *const self)
{
    if(NULL == self) 
    {
        return UART_HANDLER_ERROR;
    }
    static uint32_t last_dma_pos = 0;
    static uint8_t INFO_TO_BACKEND_PATTERN = INFO_TO_BACKEND;
    //1,获取当前head
     
    uint32_t current_head =self->need_arg->frame_parse->ring_buffer->head;

  //  elog_d("INFO","current_head is [%d]",current_head);
    //2,当前dma到达的位置(dma 会不断在 0~RING_BUFFER_SIZE 之间徘徊)

    uint32_t current_data_pos =RING_BUFFER_SIZE - self->need_arg->uart_ops->uart_get_counter();

    //3,计算需要让head移动多少
    uint32_t need_move_len = 0 ;
    if(current_data_pos >= last_dma_pos)
    {
        need_move_len = current_data_pos - last_dma_pos;
    }
    else
    {

        need_move_len = RING_BUFFER_SIZE + current_data_pos - last_dma_pos;
    
    }
 //   elog_d("INFO","need_move_len is [%d]",need_move_len);
    //2,当前dma到达的位置(dma 会不断在 0~RING_BUFFER_SIZE 之间徘徊)
    //4,移动head
    //self->uart_proto_priv_data->header+=need_move_len;
    self->need_arg->frame_parse->ring_buffer->head+=need_move_len;
    last_dma_pos = current_data_pos;
//    elog_d("INFO","current_head is [%d]",self->need_arg->frame_parse->ring_buffer->head);

    //5通知解析任务
    self->need_arg->os_ops->os_queue_send_isr(
            self->uart_proto_priv_data->queue_handle,
            &INFO_TO_BACKEND_PATTERN
            ); 
    return UART_HANDLER_OK;
}



void *get_queue_address(uart_handler_t *const self)
{
    return  self->uart_proto_priv_data->queue_handle2;
}
