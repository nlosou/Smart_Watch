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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "elog.h"
#include "queue.h"
#include "mid_ring_buffer.h"

/* USER CODE END Includes */
#include "usart.h"

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define FRAME_HEAD_FLAG          (0xFE)
#define FRAME_TAIL_FLAG          (0xFF)
#define NOTIFY_UART_PUBSUB_TASK  (0xCC)


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

TaskHandle_t Task_A;
TaskHandle_t Bsp_uart_driver;

/*emun Variables*/
typedef enum {
    FRAME_HEAD_NOT_DECTED,
    FRAME_LEN,
    FRAME_DATAs,
    FRAME_CRC,
    FRAME_END,
}uart_protocol_frame_status_t;

/*emun Variables*/


/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */


void FreeRTOS_create_task(void (*task_code)(void*),
                           char *const task_name,
                           const uint32_t StackDepth,
                           void *const argument,
                           uint32_t task_priority,
                           void **const task_handler);

void FreeRTOS_queue_create(uint32_t queue_length,
                           uint32_t queue_item_size,
                           void **const queue_handler);
    
void FreeRTOS_queue_receive(void* queue_hendler,
                            uint8_t* pvBuffer,
                            uint32_t WaitTime);

void FreeRTOS_queue_send(void* queue_hendler,
                         uint8_t* pvBuffer,
                         uint32_t WaitTime);
    
void FreeRTOS_queue_send_isr(void* queue_hendler,
                            uint8_t* pvBuffer);

void* FreeRTOS_malloc(uint32_t size);

void frame_parse_alog(ring_buffer_t*,
                      uint8_t* payload_buf,
                      uint8_t* payload_cnt);

void function_cb1(const uint8_t *payload,uint16_t len,void *user_ctx);

void function_cb2(const uint8_t *payload,uint16_t len,void *user_ctx);

void function_cb3(const uint8_t *payload,uint16_t len,void *user_ctx);

void function_cb4(const uint8_t *payload,uint16_t len,void *user_ctx);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/* USER CODE BEGIN Variables */
os_ops_t g_os_ops = {
    .os_task_create    = FreeRTOS_create_task,
    .os_queue_create   = FreeRTOS_queue_create,
    .os_queue_receive  = FreeRTOS_queue_receive,
    .os_queue_send     = FreeRTOS_queue_send,
    .os_queue_send_isr = FreeRTOS_queue_send_isr,
    .os_malloc         = FreeRTOS_malloc,
};


frame_parse_t g_frame_parse = {
    .ring_buffer = &g_ring_buffer,
    .frame_parse_alog = frame_parse_alog
};

uart_need_arg_t g_uart_need_arg = {
    .os_ops = &g_os_ops,
    .frame_parse = &g_frame_parse,
    .uart_ops   = &g_uart_ops
};

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

  for(;;)
  {
     vTaskDelay(0xFFFFFFFFUL);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void FreeRTOS_create_task(void (*task_code)(void*),
                           char *const task_name,
                           const uint32_t StackDepth,
                           void *const argument,
                           uint32_t task_priority,
                           void **const task_handler)
{
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

void FreeRTOS_queue_create(uint32_t queue_length,uint32_t queue_item_size,void **const queue_handler)
{
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
    
void FreeRTOS_queue_receive(void* queue_handler,uint8_t* pvBuffer,uint32_t WaitTime)
{
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

void FreeRTOS_queue_send(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime)
{
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
    
void FreeRTOS_queue_send_isr(void* queue_hendler,uint8_t* pvBuffer)
{
    
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


void* FreeRTOS_malloc(uint32_t size)
{
   return  pvPortMalloc(size);
}

static uint8_t crc8(const uint8_t *data, uint16_t len)
{
    uint8_t crc = 0x00;   // 初始值

    for (uint16_t i = 0; i < len; i++) {
        crc ^= data[i];

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
 * @brief 
 *  帧格式:
 *  0xFE|LEN|CMD|DATA0|DATA1|~~|DATA(LEN - 1)|CRC|0xFF
 *  其中crc根据LEN CMD 和 DATA0到~DATA(LEN - 1)并通过crc8计算得到
 *
 * @param ring_buffer
 * @param payload_buf
 * @param payload_cnt
 */
void frame_parse_alog(ring_buffer_t*ring_buffer,uint8_t* payload_buf,uint8_t* payload_cnt)
{
    //进入状态机
    static uart_protocol_frame_status_t frame_status = FRAME_HEAD_NOT_DECTED;
    static uint8_t temp_frame_data[RING_BUFFER_SIZE*2] = {0};
    static uint8_t             temp_data = 0;
    static uint8_t             frame_len = 0;
    static uint8_t             frame_crc= 0;
    static uint8_t             data_idx = 0 ;
    uint8_t calculate_crc ;
    uint8_t notify_uart_pubsub_patern = NOTIFY_UART_PUBSUB_TASK;
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
        vTaskDelay(2);
        switch(frame_status)
        {
            //1.如果检测到某一字节数据为0xFE,就说明是帧头
            case FRAME_HEAD_NOT_DECTED:
                if(temp_data == FRAME_HEAD_FLAG)
                {
                    frame_status = FRAME_LEN;
                    elog_i("backend","frame start");
                }
                break;
            case FRAME_LEN:
                frame_len = temp_data; 
                temp_frame_data[data_idx++] = temp_data; 
                frame_status =FRAME_DATAs;
                break;
            case FRAME_DATAs:
                temp_frame_data[data_idx++] = temp_data; 
                if(data_idx == frame_len + 2)
                {
                    frame_status =FRAME_CRC;
                }
                break;
            //5,获取完数据后,获取CRC
            case FRAME_CRC:
                frame_crc = temp_data;
                temp_frame_data[data_idx++] = temp_data; 
                frame_status = FRAME_END;
                break;
            //6,获取帧尾巴
            case FRAME_END:
                if(temp_data!=FRAME_TAIL_FLAG)
                {
                //6.1不是帧尾巴就丢弃数据并之前获得的数据清理掉
                    elog_e("DATA PARSE","data is not complete");
                    frame_status = FRAME_HEAD_NOT_DECTED;
                }
                else
                {
                    elog_d("DATA PARSE","data iscomplete");
                    calculate_crc = crc8(temp_frame_data,data_idx - 1);   
                    elog_i("INFO","%x %x",calculate_crc,frame_crc);
                    if(calculate_crc == frame_crc)
                    {
                            
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
                            g_uart_handler.need_arg->os_ops->os_queue_send(
                                    get_queue_address(&g_uart_handler),
                                    &notify_uart_pubsub_patern,
                                    0);
                    }
                    else
                    {
                            elog_e("INFO","Data verification falsed");
                            data_idx = 0;
                            frame_status = FRAME_HEAD_NOT_DECTED;
                    }                
                }
                break;
         }
    }    
}
void function_cb1(const uint8_t *payload,uint16_t len,void *user_ctx)
{
    //elog_e("payload","function_cb1 start");
    for(uint8_t i = 0 ; i < len ; i++)
    {
        elog_e("payload","payload[%d] is [%x]",i,payload[i]);
    }
}

void function_cb2(const uint8_t *payload,uint16_t len,void *user_ctx)
{
    //elog_e("payload","function_cb2 start");
 for(uint8_t i = 0 ; i < len ; i++)
    {
        elog_e("payload","payload[%d] is [%x]",i,payload[i]);
    }
}

void function_cb3(const uint8_t *payload,uint16_t len,void *user_ctx)
{
    //elog_e("payload","function_cb3 start");
 for(uint8_t i = 0 ; i < len ; i++)
    {
        elog_e("payload","payload[%d] is [%x]",i,payload[i]);
    }
}

void function_cb4(const uint8_t *payload,uint16_t len,void *user_ctx)
{
    //elog_e("payload","function_cb3 start");
 for(uint8_t i = 0 ; i < len ; i++)
    {
        elog_e("payload","payload[%d] is [%x]",i,payload[i]);
    }
}

/* USER CODE END Application */

