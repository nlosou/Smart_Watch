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
#include "uart_parse_task.h"
#include "bsp_uart_driver.h"
#include "queue.h"

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

#define FRAME_HEAD_FLAG (0xFE)
#define FRAME_TAIL_FLAG (0xFF)


/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

TaskHandle_t Task_A;
TaskHandle_t Bsp_uart_driver;

/*emun Variables*/
typedef enum {
    FRAME_HEAD_NOT_DECTED,
    FRAME_HEAD,
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

void FreeRTOS_queue_create(uint32_t queue_length,uint32_t queue_item_size,void **const queue_handler);
    
void FreeRTOS_queue_receive(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime);

void FreeRTOS_queue_send(void* queue_hendler,uint8_t* pvBuffer,uint32_t WaitTime);
    
void FreeRTOS_queue_send_isr(void* queue_hendler,uint8_t* pvBuffer);

void* FreeRTOS_malloc(uint32_t size);

void frame_parse_alog(ring_buffer_t*);

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
  //defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */

  uart_handler_init(&g_uart_handler,&g_uart_need_arg);
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
  for(;;)
  {
     //vTaskDelay(0xFFFFFFFFUL);
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
 *帧格式:
 *  0xFE|LEN|DATA0|DATA1|~~|DATA(LEN - 1)|CRC|0xFF
 * @param ring_buffer
 */
void frame_parse_alog(ring_buffer_t* ring_buffer)
{
    //进入状态机
    static uart_protocol_frame_status_t frame_status = FRAME_HEAD_NOT_DECTED;
    static uint8_t temp_frame_data[32] = {0};
    static uint8_t             frame_idx = 0;
    static uint32_t            frame_sum = 0;
    static uint8_t             temp_data = 0;
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
        vTaskDelay(5);
        switch(frame_status)
        {
            //1.如果检测到某一字节数据为0xFE,就说明是帧头
            case FRAME_HEAD_NOT_DECTED:
                if(temp_data == FRAME_HEAD_FLAG)
                {
                    frame_status = FRAME_HEAD;
                    elog_i("backend","frame start");
                }
                break;
            //2.开始连续打印数据,直到遇到0xFF帧尾巴,或者环形缓存区为空
            case FRAME_HEAD:
                //检测到末尾开始计算
                if(temp_data == FRAME_TAIL_FLAG)
                {
                    frame_status = FRAME_HEAD_NOT_DECTED;
                    elog_i("backend","frame end");
                    //计算校验和
                    for(uint8_t idx = 0 ; idx <( frame_idx - 1); idx++) 
                    {
                        frame_sum+=temp_frame_data[idx];
                    }
                    //验证成功才打印数据
                    if(frame_sum == temp_frame_data[frame_idx - 1])
                    {
                        elog_i("INFO","Data verification successful");
                        for(uint8_t idx = 0 ; idx <( frame_idx - 1); idx++) 
                        {

                            elog_i("backend",
                           "frame data is [%x]",
                                   temp_frame_data[idx] );
                        }
                          elog_i("backend",
                           "verification data is  [%x]",
                                   frame_sum);
                    }
                    //验证失败
                    else
                    {
                        elog_e("INFO","Data verification falsed");
                        elog_e("INFO",
                                "calculate sum is %x,get sum from frame is %x",
                                frame_sum,temp_frame_data[frame_idx - 1]);
                    }
                    for(uint8_t idx = 0 ; idx <( frame_idx); idx++) 
                    {
                        temp_frame_data[idx] = 0;
                    }
                    frame_idx = 0;
                    frame_sum = 0;
                }    
                else
                {

                    temp_frame_data[frame_idx++] = temp_data;
                    //elog_i("backend","frame data is [%x]",temp_data);
                }
                break;
        }
    }    
}
/* USER CODE END Application */

