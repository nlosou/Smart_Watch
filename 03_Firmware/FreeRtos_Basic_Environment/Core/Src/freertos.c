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
#include "queue.h"
#include "semphr.h"
#include "bsp_led.h"
#include "bsp_key.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */

/* Definitions for defaultTask */
/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

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
  if(pdPASS == xTaskCreate(Key_task,"Key_task",100,NULL,2,NULL))
  {
    printf("key_task was successfully create\r\n");
  }
  else
  {
    printf("key_task was not successfully create\r\n");
  }
  //xTaskCreate(printf_task,"prinf_task",100,NULL,1,NULL); 
  if(pdPASS == xTaskCreate(led_toggle_task,"led_task",100,NULL,1,NULL))
  {
    printf("led_toggle_task was successfully create\r\n");
  }
  else
  {
    printf("led_toggle_task was not successfully create\r\n");
  }
  if(pdPASS == xTaskCreate(StartDefaultTask,"default_task",100,NULL,1,NULL))
  {
    printf("StartDefaultTask was successfully create\r\n");
  }
  else
  {
    printf("StartDefaultTask was not successfully create\r\n");
  }
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
  key_event_t key_event = KEY_NOT_PRESSED;
  led_function_t led_function_sate = LED_OFF;
  for(;;)
  {     

        if(pdTRUE == xQueueReceive(key_queue,&key_event,portMAX_DELAY))
        {
            printf("Get key_queue at [%d] tick\r\n",HAL_GetTick());
            if(KEY_NOT_PRESSED != key_event)
            {
                if(key_event == KEY_SHORT_PRESSED)
                {
                    led_function_sate = LED_TOGGLE;    
                    printf("key_short\r\n");

                }
                if(key_event == KEY_LONG_PRESSED)
                {
                    led_function_sate = LED_BLINK_3;
                    printf("key_long\r\n");

                }
                if((xQueueSendToBack(led_queue,&led_function_sate,0)))
                {
                    printf("Sent led_function_t to led_queue at [%d] tick\r\n",HAL_GetTick());
                }
                else
                {
                    printf("led_queue is full at [%d] tick\r\n",HAL_GetTick());
                }
                key_event = KEY_NOT_PRESSED;
            }
        }
        else
        {
            printf("key_queue is empty at [%d] tick\r\n",HAL_GetTick());
        }
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

