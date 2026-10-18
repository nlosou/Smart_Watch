/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
/* FreeRTOS 内核接口 */
#include "FreeRTOS.h"
/* FreeRTOS 任务管理接口 */
#include "task.h"
/* STM32Cube 主模块接口 */
#include "main.h"
/* CMSIS-RTOS 封装接口 */
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* 日志组件 */
#include "elog.h"
/* FreeRTOS 队列接口 */
#include "queue.h"
/* 环形缓冲区中间件接口 */
#include "mid_ring_buffer.h"

/* USER CODE END Includes */
/* UART 驱动接口 */
#include "usart.h"

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* UART 私有协议配置宏 */
/** 帧头同步字节 */
#define FRAME_HEAD_FLAG          (0xFE)
/** 帧尾结束字节 */
#define FRAME_TAIL_FLAG          (0xFF)
/** 通知 UART 发布任务继续处理的队列标记 */
#define NOTIFY_UART_PUBSUB_TASK  (0xCC)


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/** 保留任务句柄：用于扩展 UART 接收任务 */
TaskHandle_t Task_A;
/** 保留任务句柄：用于扩展底层 UART 驱动任务 */
TaskHandle_t Bsp_uart_driver;

/*emun Variables*/
/**
 * @brief UART 协议帧解析状态机状态定义。
 */
typedef enum {
    /** 等待帧头同步字节 */
    FRAME_HEAD_NOT_DECTED,
    /** 接收帧长度字段 */
    FRAME_LEN,
    /** 接收帧命令字和负载数据 */
    FRAME_DATAs,
    /** 接收 CRC 校验字节 */
    FRAME_CRC,
    /** 接收帧尾结束字节 */
    FRAME_END,
}uart_protocol_frame_status_t;

/*emun Variables*/


/* USER CODE END Variables */
/* Definitions for defaultTask */
/** defaultTask 线程句柄 */
osThreadId_t defaultTaskHandle;
/** defaultTask 线程属性 */
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* 创建 FreeRTOS 任务，供 UART 发布-订阅模块通过函数指针调用 */
void FreeRTOS_create_task(void (*task_code)(void*),
                           char *const task_name,
                           const uint32_t StackDepth,
                           void *const argument,
                           uint32_t task_priority,
                           void **const task_handler);

/* 创建 FreeRTOS 队列 */
void FreeRTOS_queue_create(uint32_t queue_length,
                           uint32_t queue_item_size,
                           void **const queue_handler);
    
/* 从 FreeRTOS 队列阻塞接收一个字节 */
void FreeRTOS_queue_receive(void* queue_hendler,
                            uint8_t* pvBuffer,
                            uint32_t WaitTime);

/* 向 FreeRTOS 队列发送一个字节，队列满时覆盖旧数据 */
void FreeRTOS_queue_send(void* queue_hendler,
                         uint8_t* pvBuffer,
                         uint32_t WaitTime);
    
/* 从中断上下文向 FreeRTOS 队列发送一个字节 */
void FreeRTOS_queue_send_isr(void* queue_hendler,
                            uint8_t* pvBuffer);

/* 封装 FreeRTOS 动态内存申请接口 */
void* FreeRTOS_malloc(uint32_t size);

/* 封装 FreeRTOS 动态内存释放接口 */
void FreeRTOS_free(void *pv);

/* UART 环形缓冲区帧解析函数 */
void frame_parse_alog(ring_buffer_t*,
                      uint8_t* payload_buf,
                      uint8_t* payload_cnt);

/* 命令字 0x01 对应的 UART 帧回调 */
void function_cb1(const uint8_t *payload,uint16_t len,void *user_ctx);

/* 命令字 0x02 对应的 UART 帧回调 */
void function_cb2(const uint8_t *payload,uint16_t len,void *user_ctx);

/* 命令字 0x03 对应的 UART 帧回调 */
void function_cb3(const uint8_t *payload,uint16_t len,void *user_ctx);

/* 命令字 0x04 对应的 UART 帧回调 */
void function_cb4(const uint8_t *payload,uint16_t len,void *user_ctx);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN Variables */



/**
 * @brief FreeRTOS 抽象接口实例，供 UART 发布-订阅模块调用。
 */
os_ops_t g_os_ops = {
    .os_task_create    = FreeRTOS_create_task,
    .os_queue_create   = FreeRTOS_queue_create,
    .os_queue_receive  = FreeRTOS_queue_receive,
    .os_queue_send     = FreeRTOS_queue_send,
    .os_queue_send_isr = FreeRTOS_queue_send_isr,
    .os_malloc         = FreeRTOS_malloc,
    .os_free           = FreeRTOS_free
};


/**
 * @brief 帧解析器实例，绑定环形缓冲区与解析算法。
 */
frame_parse_t g_frame_parse = {
    .ring_buffer = &g_ring_buffer,
    .frame_parse_alog = frame_parse_alog
};

/**
 * @brief UART 发布-订阅模块所需依赖参数集合。
 */
uart_need_arg_t g_uart_need_arg = {
    .os_ops = &g_os_ops,
    .frame_parse = &g_frame_parse,
    .uart_ops   = &g_uart_ops
};

/** UART 发布-订阅核心句柄 */
uart_handler_t g_uart_handler;
/* USER CODE BEGIN Variables */
/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */
  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  /* 创建默认任务：任务内部完成 UART 发布-订阅模块初始化和订阅注册 */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  /*
   *
  if(pdPASS == xTaskCreate(
          Uart_rec_A_task,
          "Uart_rec_A_task",
          128*4,
          NULL,
          2,
          &Task_A))
  {
    elog_i("Task_A","Task_A was successfully created.");
  }
  else
  {
    elog_e("Task_A","Task_A creat failed."); 
    return;
  }

  if(pdPASS == xTaskCreate(
          uart_driver_fucn,
          "uart_driver",
          128*4,
          NULL,
          4,
          &Bsp_uart_driver))
  {
    elog_i("Uart_diver","Uart_diver was successfully created.");
  }
  else
  {
    elog_e("Uart_diver","Uart_diver creat failed."); 
    return;
  }
  */
  /* USER CODE END RTOS_THREADS */
  

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  /* Infinite loop */
  /* 初始化 UART 发布-订阅模块，成功后注册各命令字对应的回调 */
  if(UART_HANDLER_OK == uart_handler_init(&g_uart_handler,&g_uart_need_arg))
  {   
      //订阅
       g_uart_handler.uart_pubsub_subscribe(&g_uart_handler,0x01,function_cb1,NULL); 
       g_uart_handler.uart_pubsub_subscribe(&g_uart_handler,0x02,function_cb2,NULL); 
       g_uart_handler.uart_pubsub_subscribe(&g_uart_handler,0x03,function_cb3,NULL); 
       g_uart_handler.uart_pubsub_subscribe(&g_uart_handler,0x04,function_cb4,NULL); 

  }
  else
  {
        elog_e("error","uart handler init error");
         }

  /* 默认任务仅完成初始化，不执行周期性业务，进入低优先级等待 */
  for(;;)
  {
     vTaskDelay(0xFFFFFFFFUL);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
/**
 * @brief 封装 xTaskCreate，创建 FreeRTOS 任务。
 *
 * @param task_code    任务入口函数
 * @param task_name    任务名称
 * @param StackDepth   任务栈深度（字节）
 * @param argument     传入任务入口函数的参数
 * @param task_priority 任务优先级
 * @param task_handler 任务句柄输出指针
 */
void FreeRTOS_create_task(void (*task_code)(void*),
                           char *const task_name,
                           const uint32_t StackDepth,
                           void *const argument,
                           uint32_t task_priority,
                           void **const task_handler)
{
    /* 调用 FreeRTOS 创建任务，并将任务句柄通过参数返回 */
    if(pdPASS == xTaskCreate(task_code,
                             task_name,
                             StackDepth,
                             argument,
                             task_priority,
                             (TaskHandle_t*)task_handler))
    {
        elog_d("TASK","%s task create successfully!",task_name);
    }
    else
    {

        elog_e("TASK","%s task create failed",task_name);
        return;
    }
}

/**
 * @brief 封装 xQueueCreate，创建 FreeRTOS 队列。
 *
 * @param queue_length   队列长度
 * @param queue_item_size 单个队列项大小（字节）
 * @param queue_handler  队列句柄输出指针
 */
void FreeRTOS_queue_create(uint32_t queue_length,uint32_t queue_item_size,void **const queue_handler)
{
   /* 创建队列，成功后保存句柄 */
   QueueHandle_t   temp_queue_handler =  xQueueCreate(queue_length,queue_item_size);
   if(NULL != temp_queue_handler)
   {
        *queue_handler = temp_queue_handler;       
        elog_d("queue","queue create successfully");
   }
   else
   {

        elog_e("queue","queue create failed");
        return;
   }

}
    
/**
 * @brief 封装 xQueueReceive，从队列阻塞接收一个字节。
 *
 * @param queue_handler 队列句柄
 * @param pvBuffer      接收数据缓冲区
 * @param WaitTime      最大等待时间，由 FreeRTOS 时间单位定义
 */
void FreeRTOS_queue_receive(void* queue_handler,uint8_t* pvBuffer,uint32_t WaitTime)
{
    /* 成功接收后打印数据，失败则直接返回 */
    if(pdPASS == xQueueReceive((QueueHandle_t)queue_handler,
                               pvBuffer,
                              WaitTime))
    {

        elog_d("queue","get pvBuffer [%x] from queue",*pvBuffer);
    }
    else
    {
        return;
    }
}

/**
 * @brief 封装 xQueueGenericSend，向队列发送一个字节。
 *
 * 使用 queueOVERWRITE 方式，当队列满时覆盖已有数据。
 *
 * @param queue_hendler 队列句柄
 * @param pvBuffer      待发送数据缓冲区
 * @param WaitTime      最大等待时间
 */
void FreeRTOS_queue_send(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime)
{
    /* 发送数据并记录结果；发送失败时返回 */
    if(pdPASS == xQueueGenericSend((QueueHandle_t)queue_hendler,
                                   pvBuffer,
                                   WaitTime,
                                   queueOVERWRITE))
    {
            
        elog_d("queue","sent pvBuffer [%x] to queue",*pvBuffer);
    }
    else
    {
            
        elog_e("queue","queue sent error");
        return;
    }
}
    
/**
 * @brief 从中断上下文向队列发送一个字节。
 *
 * 仅允许在中断服务程序或中断回调中调用。
 *
 * @param queue_hendler 队列句柄
 * @param pvBuffer      待发送数据缓冲区
 */
void FreeRTOS_queue_send_isr(void* queue_hendler,uint8_t* pvBuffer)
{
    
    /* 中断上下文中仅发送数据，不进行日志输出或阻塞等待 */
    if(pdPASS == xQueueGenericSendFromISR((QueueHandle_t)queue_hendler,
                                   pvBuffer,
                                   NULL,
                                   queueOVERWRITE))
    {
            
        //elog_d("queue","sent pvBuffer [%x] to queue",*pvBuffer);
    }
    else
    {
        return;
    }
}

/**
 * @brief 封装 pvPortMalloc，申请动态内存。
 *
 * @param size 申请内存大小（字节）
 * @return 申请到的内存地址；失败时返回 NULL
 */
void* FreeRTOS_malloc(uint32_t size)
{
   return  pvPortMalloc(size);
}
/**
 * @brief 封装 vPortFree，释放动态内存。
 *
 * @param pv 待释放内存地址
 */
void FreeRTOS_free(void *pv)
{
   vPortFree(pv);
}


/**
 * @brief 计算 CRC-8 校验值。
 *
 * 多项式为 0x07，初始值为 0x00。
 *
 * @param data 待校验数据指针
 * @param len  待校验数据长度（字节）
 * @return 计算得到的 8 位 CRC
 */
static uint8_t crc8(const uint8_t *data, uint16_t len)
{
    /* CRC 初始值 */
    uint8_t crc = 0x00;   // 初始值

    for (uint16_t i = 0; i < len; i++) {
        /* 将当前数据字节并入 CRC */
        crc ^= data[i];

        /* 按位进行 CRC-8 运算，最高位为 1 时异或多项式 */
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x80) {
                crc = (crc << 1) ^ 0x07;   // 多项式 0x07
            } else {
                crc <<= 1;
            }
        }
    }

    return crc;
}



/**
 * @brief 从 UART 环形缓冲区解析完整帧。
 *
 *  帧格式:
 *  0xFE|LEN|CMD|DATA0|DATA1|~~|DATA(LEN - 1)|CRC|0xFF
 *  其中crc根据LEN CMD 和 DATA0到~DATA(LEN - 1)并通过crc8计算得到
 *
 * @param ring_buffer 输入环形缓冲区
 * @param payload_buf 输出完整帧缓存
 * @param payload_cnt 输出完整帧数据长度
 */
void frame_parse_alog(ring_buffer_t*ring_buffer,uint8_t* payload_buf,uint8_t* payload_cnt)
{
    //TODO:需要增加partern逻辑
    //进入状态机
    /* 帧解析状态机及临时缓存均为静态变量，跨多次调用保持状态 */
    static uart_protocol_frame_status_t frame_status = FRAME_HEAD_NOT_DECTED;
    /** 暂存帧长度、命令字、数据和 CRC 的临时缓冲 */
    static uint8_t temp_frame_data[RING_BUFFER_SIZE*2] = {0};
    /** 当前从环形缓冲区取出的一个字节 */
    static uint8_t             temp_data = 0;
    /** 帧头之后接收到的帧长度字段 */
    static uint8_t             frame_len = 0;
    /** 接收到的 CRC 校验字节 */
    static uint8_t             frame_crc= 0;
    /** 已存入临时缓冲的数据索引 */
    static uint8_t             data_idx = 0 ;
    /** 本地计算得到的 CRC */
    uint8_t calculate_crc ;
    /** 通知发布任务继续处理的队列标记 */
    uint8_t notify_uart_pubsub_patern = NOTIFY_UART_PUBSUB_TASK;
    /* 环形缓冲区仍有数据时，逐字节送入状态机处理 */
    while(0x00!=ring_buffer_is_empty(ring_buffer))
    {           
//        elog_i("backend","current tail is %d",ring_buffer->tail);
        if(0x00 == get_data(ring_buffer,&temp_data))
        {
 //           elog_i("backend","get_data is [%x]",temp_data);
        }
        else
        {
            elog_e("backend","get_data is falsed");
        }
        /* 解析流程为状态机驱动，为避免独占 CPU，此处短暂让出执行权 */
        vTaskDelay(2);
        switch(frame_status)
        {
            //1.如果检测到某一字节数据为0xFE,就说明是帧头
            case FRAME_HEAD_NOT_DECTED:
                /* 检测到帧头后进入长度字段接收状态 */
                if(temp_data == FRAME_HEAD_FLAG)
                {
                    frame_status = FRAME_LEN;
                    elog_i("backend","frame start");
                }
                break;
            /* 接收并保存帧长度字段 */
            case FRAME_LEN:
                frame_len = temp_data; 
                temp_frame_data[data_idx++] = temp_data; 
                frame_status =FRAME_DATAs;
                break;
            /* 接收命令字和负载数据，长度满足后进入 CRC 接收状态 */
            case FRAME_DATAs:
                temp_frame_data[data_idx++] = temp_data; 
                if(data_idx == frame_len + 2)
                {
                    frame_status =FRAME_CRC;
                }
                break;
            //5,获取完数据后,获取CRC
            case FRAME_CRC:
                /* 保存 CRC 字段并进入帧尾判断状态 */
                frame_crc = temp_data;
                temp_frame_data[data_idx++] = temp_data; 
                frame_status = FRAME_END;
                break;
            //6,获取帧尾巴
            case FRAME_END:
                /* 帧尾不匹配则丢弃当前帧，回到帧头搜索状态 */
                if(temp_data!=FRAME_TAIL_FLAG)
                {
                //6.1不是帧尾巴就丢弃数据并之前获得的数据清理掉
                    elog_e("DATA PARSE","data is not complete");
                    frame_status = FRAME_HEAD_NOT_DECTED;
                }
                else
                {
                    elog_d("DATA PARSE","data iscomplete");
                    /* 对 LEN、CMD 和 DATA 区域计算 CRC，并与接收值比较 */
                    calculate_crc = crc8(temp_frame_data,data_idx - 1);   
                    elog_i("INFO","%x %x",calculate_crc,frame_crc);
                    if(calculate_crc == frame_crc)
                    {
                            
                            /* CRC 校验通过，将完整帧复制到输出缓冲 */
                            for(uint8_t i = 0 ; i < data_idx ; i++)
                            {
                                payload_buf[i] = temp_frame_data[i];
                            }
                            *payload_cnt = data_idx;
                            //elog_d("Debug","%d",data_idx);
                            elog_i("INFO","Data verification successful");
                            data_idx = 0;
                            frame_status = FRAME_HEAD_NOT_DECTED;
                            //每完成一帧就通知发布线程
                            /* 每解析成功一帧，就通知发布任务进行分发 */
                            g_uart_handler.need_arg->os_ops->os_queue_send(
                                    get_queue_address(&g_uart_handler),
                                    &notify_uart_pubsub_patern,
                                    0);
                    }
                    else
                    {
                            /* CRC 校验失败，丢弃数据并复位状态机 */
                            elog_e("INFO","Data verification falsed");
                            data_idx = 0;
                            frame_status = FRAME_HEAD_NOT_DECTED;
                    }                
                }
                break;
         }
    }    
}
/**
 * @brief 命令字 0x01 对应的 UART 帧回调。
 *
 * @param payload 帧负载数据指针
 * @param len     帧负载长度（字节）
 * @param user_ctx 用户上下文，当前未使用
 */
void function_cb1(const uint8_t *payload,uint16_t len,void *user_ctx)
{
    //elog_e("payload","function_cb1 start");
    /* 逐个打印负载字节，便于调试观察 */
    for(uint8_t i = 0 ; i < len ; i++)
    {
        elog_e("payload","payload[%d] is [%x]",i,payload[i]);
    }
}

/**
 * @brief 命令字 0x02 对应的 UART 帧回调。
 *
 * @param payload 帧负载数据指针
 * @param len     帧负载长度（字节）
 * @param user_ctx 用户上下文，当前未使用
 */
void function_cb2(const uint8_t *payload,uint16_t len,void *user_ctx)
{
    //elog_e("payload","function_cb2 start");
    /* 逐个打印负载字节，便于调试观察 */
 for(uint8_t i = 0 ; i < len ; i++)
    {
        elog_e("payload","payload[%d] is [%x]",i,payload[i]);
    }
}

/**
 * @brief 命令字 0x03 对应的 UART 帧回调。
 *
 * @param payload 帧负载数据指针
 * @param len     帧负载长度（字节）
 * @param user_ctx 用户上下文，当前未使用
 */
void function_cb3(const uint8_t *payload,uint16_t len,void *user_ctx)
{
    //elog_e("payload","function_cb3 start");
    /* 逐个打印负载字节，便于调试观察 */
 for(uint8_t i = 0 ; i < len ; i++)
    {
        elog_e("payload","payload[%d] is [%x]",i,payload[i]);
    }
}

/**
 * @brief 命令字 0x04 对应的 UART 帧回调。
 *
 * @param payload 帧负载数据指针
 * @param len     帧负载长度（字节）
 * @param user_ctx 用户上下文，当前未使用
 */
void function_cb4(const uint8_t *payload,uint16_t len,void *user_ctx)
{
    //elog_e("payload","function_cb3 start");
    /* 逐个打印负载字节，便于调试观察 */
 for(uint8_t i = 0 ; i < len ; i++)
    {
        elog_e("payload","payload[%d] is [%x]",i,payload[i]);
    }
}

/* USER CODE END Application */

