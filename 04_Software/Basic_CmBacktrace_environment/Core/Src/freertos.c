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
#include "SEGGER_RTT.h"
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
void fault_test(void);
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
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
__attribute__ ((used, section ("myflash"))) 
void flash_function_pre()
{
	int i = 0;
	i = i / 3;
	return;
}

void flash_function()
{
	int i = 0;
	i = i / 3;
	return;
}

__attribute__ ((used, section ("myram"))) 
void sram_function(void)
{
	int i = 0;
	i = i / 3;
	return;
}

__attribute__ ((used, section ("myram"))) 
void sram_function_pre(void)
{
	int i = 0;
	i = i / 3;
	return;
}

uint32_t call_flash_func_time_record(void)
{
	uint32_t start_tick = HAL_GetTick();
	uint32_t end_tick = 0;
	uint32_t i_counter = 10000000;
	void (* volatile fpointer_1)(void) = flash_function;
	void (* volatile fpointer_2)(void) = flash_function_pre;
	
	while( i_counter > 0 )
	{
		fpointer_1();
		fpointer_2();
		i_counter --;
	}
	
	end_tick = HAL_GetTick();
	
	return (end_tick - start_tick);
}

__attribute__ ((used, section ("myram"))) 
uint32_t call_sram_func_time_record(void)
{
	uint32_t start_tick = HAL_GetTick();
	uint32_t end_tick = 0;
	uint32_t i_counter = 10000000;
	void (* volatile fpointer_1)(void) = sram_function;
	void (* volatile fpointer_2)(void) = sram_function_pre;
	
	while( i_counter > 0 )
	{
		fpointer_1();
		fpointer_2();
		i_counter --;
	}
	
	end_tick = HAL_GetTick();
	
	return (end_tick - start_tick);
}
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
	fault_test();
  for(;;)
  {
		
    osDelay(1000);
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void fault_test(void) {

    volatile int *SCB_CCR=(volatile int *)0xE000ED14;

    int x, y, z;

    *SCB_CCR |= (1<<4);

    x=10;
    y=0;
    z=x/y;
		SEGGER_RTT_printf(0,"%d",z);
}
/* USER CODE END Application */

