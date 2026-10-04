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

    elog_i("INFO","The semaphore was created successfully.");
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
  }

  g_Handle_data_mailbox = xQueueCreate(1,4);
  if(g_Handle_data_mailbox)
  {
        
    elog_i("INFO","handle data mailbox is create Successfully");
  }
  else
  {

    elog_e("INFO","handle data mailbox is create error");
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
   if(pdPASS == xTaskCreate(Task_A,"Task_A",100,NULL,2,NULL))
   {
        elog_i("INFO","Task_A is created successfully");
   }        
   else
   {

        elog_e("ERROR","Task_A is created error");
   }

   if(pdPASS == xTaskCreate(Task_B,"Task_B",100,NULL,3,NULL))
   {
        elog_i("INFO","Task_B is created successfully");
   }        
   else
   {
        elog_e("ERROR","Task_B is created error");
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
  HAL_TIM_Base_Start(&htim2);  
  DMA_POINT = 0;
  if(HAL_OK == HAL_ADC_Start_DMA(&hadc1,buffer_1,10))
  {
    elog_i("INFO","HAL_ADC_Start is Success");
  }
  else
  {
    elog_e("error","HAL_ADC_Start is serror");

  }
#if 0
    if(pdPASS == xQueueOverwrite(g_adc_dma_conv_complete_mailbox,&temp))
    {
        elog_i("INFO",
                "sent info to g_adc_dma_conv_complete_mailbox is successfully");
    }
    else
    {

        elog_e("INFO","sent to g_adc_dma_conv_complete_mailbox is error");
    }
#endif
    for(;;)
    {
        //任务A的主要任务就是
        //1.获取DMA完成中断发过来的邮箱
        //1.1阻塞等待
        if(pdPASS == xQueueReceive(g_adc_dma_conv_complete_mailbox,&temp,portMAX_DELAY))
        {
            elog_i("INFO","adc_dma_mail come");
               if(pdTRUE == xSemaphoreTake(xSeamphore,portMAX_DELAY)) 
               {
                    //2 重新配置DMA存储位置
                    if(0 == DMA_POINT)
                    {
                        DMA_POINT = 1;
                        HAL_ADC_Start_DMA(&hadc1,buffer_2,10);
                        need_buffer = BUFFER1_NEED_PROCESS;
                    }
                    else
                    {
                        DMA_POINT = 0;
                        HAL_ADC_Start_DMA(&hadc1,buffer_1,10);
                        need_buffer = BUFFER2_NEED_PROCESS;
                    }
                    //3.发送邮箱给任务B
                    if(pdPASS == xQueueSendToBack(g_Handle_data_mailbox,&need_buffer,0))
                    {
                        elog_i("INFO",
                                "sent info to g_Handle_data__mailbox is successfully");
                    }
                    else
                    {

                        elog_e("INFO","sent to g_Handle_data__mailbox is error");
                    }
                    xSemaphoreGive(xSeamphore);
               }
        }
    }
}



uint32_t ADC_DATA_Handle(uint32_t *adc_data,uint32_t len)
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

