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
#include "adc.h"
#include "tim.h"
#include "semphr.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define BUFFER1_NEED_PROCESS   0xA1
#define BUFFER2_NEED_PROCESS   0xA2

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */



QueueHandle_t           g_Handle_data_mailbox;
SemaphoreHandle_t                  xSeamphore;
uint32_t                        DMA_POINT = 0;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityHigh,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

void Task_A(void *argument);
void Task_B(void *argument);

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

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
  xSeamphore = xSemaphoreCreateMutex();
  if(NULL != xSeamphore )
  {
    elog_i("INFO","The semaphore was created successfully.");
  }
  else
  {

    elog_i("INFO","The semaphore was created error");
    return;
  }
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  g_adc_dma_conv_complete_mailbox= xQueueCreate(1,4);
  if(g_adc_dma_conv_complete_mailbox)
  {
    elog_i("INFO","adc1_dma mailbox is create Successfully");
  }
  else
  {
    elog_e("ERROR","adc1_dam mailbox is create ERROR");
    return;
  }

  g_Handle_data_mailbox = xQueueCreate(1,4);
  if(g_Handle_data_mailbox)
  {
        
    elog_i("INFO","handle data mailbox is create Successfully");
  }
  else
  {

    elog_e("INFO","handle data mailbox is create error");
    return;
  }
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  /*
  defaultTaskHandle = osThreadNew(
          StartDefaultTask,
          NULL, 
          &defaultTask_attributes);
          */
   if(pdPASS == xTaskCreate(Task_A,"Task_A",100,NULL,3,NULL))
   {
        elog_i("INFO","Task_A is created successfully");
   }        
   else
   {

        elog_e("ERROR","Task_A is created error");
        return;
   }

   if(pdPASS == xTaskCreate(Task_B,"Task_B",100,NULL,2,NULL))
   {
        elog_i("INFO","Task_B is created successfully");
   }        
   else
   {
        elog_e("ERROR","Task_B is created error");
       return; 
   }
  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
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

  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

void Task_A(void *argument)
{
    uint32_t temp;
    uint32_t need_buffer = 0;
    HAL_StatusTypeDef dma_status;

    HAL_TIM_Base_Start(&htim2);  
    DMA_POINT = 0;

    // 首次启动 DMA，也需要检查返回值
    if(HAL_OK == HAL_ADC_Start_DMA(&hadc1, buffer_1, 10))
    {
        elog_i("INFO","HAL_ADC_Start is Success");
    }
    else
    {
        elog_e("error","HAL_ADC_Start is error");
        // 首次启动失败，任务不能继续等待，应进入错误处理
        vTaskDelete(NULL); // 或者挂起任务，等待系统复位
        return;
    }

    for(;;)
    {
        // 1. 阻塞等待 DMA 完成中断发过来的邮箱
        if(pdPASS == xQueueReceive(g_adc_dma_conv_complete_mailbox, &temp, portMAX_DELAY))
        {
            elog_i("INFO","adc_dma_mail come");
            
            // 2. 判断当前已完成的是哪个 buffer
            if(0 == DMA_POINT)
            {
                need_buffer = BUFFER1_NEED_PROCESS;
            }
            else
            {
                need_buffer = BUFFER2_NEED_PROCESS;
            }

            // 3. 发送邮箱给任务B (根据反馈，如果消费者处理不过来，这里应当降速或停止)
            // 【修改点1】：发送失败时保留数据等待或丢帧停止采集。这里采用阻塞发送防止覆盖。
            // 如果希望非阻塞丢帧，改为使用带超时的 xQueueSendToBack(..., pdMS_TO_TICKS(10))
            // 并在失败时记录丢帧，停止DMA：HAL_ADC_Stop_DMA(&hadc1); continue;
            if(pdPASS == xQueueSendToBack(g_Handle_data_mailbox, &need_buffer, portMAX_DELAY))
            {
                elog_i("INFO", "sent info to g_Handle_data_mailbox is successfully");
            }
            else
            {
                // 理论上 portMAX_DELAY 不会失败，除非队列被删除。
                elog_e("ERROR", "sent to g_Handle_data_mailbox is error, drop frame and stop ADC.");
                HAL_ADC_Stop_DMA(&hadc1);
                continue; // 停止采集，不重启DMA
            }

            // 【修改点2】：移除错误的 xQueuePeek 逻辑。
            // 替代方案：只有消费者归还信号量（处理完数据），才能继续复用该 Buffer
            if(pdTRUE == xSemaphoreTake(xSeamphore, portMAX_DELAY)) 
            {
                // 2. 重新配置 DMA 存储位置 (乒乓切换)
                if(0 == DMA_POINT)
                {
                    DMA_POINT = 1;
                    elog_i("START", "adc start scan to buffer_2");
                    // 【修改点3】：检查 DMA 重启函数的返回值
                    dma_status = HAL_ADC_Start_DMA(&hadc1, buffer_2, 10);
                }
                else
                {
                    DMA_POINT = 0;
                    elog_i("START", "adc start scan to buffer_1");
                    // 【修改点3】：检查 DMA 重启函数的返回值
                    dma_status = HAL_ADC_Start_DMA(&hadc1, buffer_1, 10);
                }

                // 【修改点4】：失败时不能继续等待一个不会到来的完成通知
                if (dma_status != HAL_OK) 
                {
                    elog_e("ERROR", "HAL_ADC_Start_DMA failed! Stop waiting for completion.");
                    // 发生严重硬件错误，停止后续采集等待，避免死等
                    // 可以在这里加入系统级错误处理，或死循环报警
                    HAL_ADC_Stop_DMA(&hadc1);
                    vTaskDelay(pdMS_TO_TICKS(1000)); // 降速/延迟重试
                    // 注意：此处需要根据业务需求决定如何恢复，简单处理为重置状态并重启初始 buffer
                    DMA_POINT = 0;
                    HAL_ADC_Start_DMA(&hadc1, buffer_1, 10);
                }
            }
            else
            {
                // 如果信号量获取超时（如果设置了超时），说明消费者异常
                elog_e("ERROR", "data handle is not received (Semaphore timeout)");
                // 必须停止 DMA，否则会覆盖未处理的数据
                HAL_ADC_Stop_DMA(&hadc1);
            }
        }
    }
}uint32_t ADC_DATA_Handle(uint32_t *adc_data,uint32_t len)
{
    uint32_t temp_swap = 0;
    uint32_t final_adc = 0;
    uint32_t final_sum = 0;

    for(uint8_t i = 0 ; i < len - 1; i++) 
    {
        for(uint8_t j = 0 ; j < len - 1- i ;j++)
        {
            if(adc_data[j] >adc_data[j + 1])
            {
                temp_swap =adc_data[j];
                adc_data[j] =adc_data[j + 1];
                adc_data[j+1] = temp_swap;
            }
        }
        
    }
    final_sum = 0;
    for(uint8_t i = 1 ; i < 9 ; i++) 
    {
        final_sum+=adc_data[i];
    }
    final_adc = final_sum>>3;
    return final_adc;
}

void Task_B(void* argument)
{
       uint32_t need_buffer = 0;
//TEST UNIT
#if 0
    p_adc1_data[0] = 3423;
    p_adc1_data[1] = 2423;
    p_adc1_data[2] = 1423;
    p_adc1_data[3] = 7423;
    p_adc1_data[4] = 8423;
    p_adc1_data[5] = 423;
    p_adc1_data[6] = 323;
    p_adc1_data[7] = 4423;
    p_adc1_data[8] = 5423;
    p_adc1_data[9] = 6423;
    if(pdPASS == xQueueOverwrite(g_Handle_data_mailbox,&temp))
    {
        elog_i("INFO",
                "sent info to g_Handle_data__mailbox is successfully");
    }
    else
    {

        elog_e("INFO","sent to g_Handle_data__mailbox is error");
    }
#endif
    //任务B
    for(;;)
    {
        //1.任务从g_Handle_data_mailbox获取邮箱
        if(pdTRUE == xQueuePeek(g_Handle_data_mailbox,&need_buffer,portMAX_DELAY))
        {

            if(pdTRUE == xSemaphoreTake(xSeamphore,portMAX_DELAY)) 
            {
                if(pdPASS ==xQueueReceive(g_Handle_data_mailbox,&need_buffer,portMAX_DELAY))
                {
                    
                    elog_i("INFO","data handle _mail come");
                    //2.处理数据将获得的数据根据去极值平均值整理
                    //3.通过RTT打印到Jlink的终端上
                    if(BUFFER1_NEED_PROCESS == need_buffer)
                    {
                        elog_i("buffer_1","%d",ADC_DATA_Handle(buffer_1,10));
                    }
                    else
                    {

                        elog_i("buffer_2","%d",ADC_DATA_Handle(buffer_2,10));
                    }
                }
                xSemaphoreGive(xSeamphore);
            }
        }
    }
}
/* USER CODE END Application */

