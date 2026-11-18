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
#include "bsp_led_drive.h"
#include "Bsp_led_handler.h"
#include "queue.h"
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

/*TEST UNIT1*/
/*绑定具体实现*/

#if 1

bsp_led_drive_t test_led;

LED_status_t led_on(void)
{
#ifdef DEBUG
        DEBUG_OUT("led on");
#endif

    return LED_OK;
}
LED_status_t led_off(void)
{
#ifdef DEBUG
        DEBUG_OUT("led off");
#endif

    return LED_OK;
}

led_operation_t led_test_operation = {
    .pf_led_on = led_on,
    .pf_led_off = led_off
};

LED_status_t FreeRTOS_delay_ms(const uint32_t time)
{
    vTaskDelay(time);
    return LED_OK;
}
os_api_t os_test = {
    .pf_os_delay_ms = FreeRTOS_delay_ms,
};

LED_status_t get_time_test(uint32_t* const time)
{
    *time = HAL_GetTick();
    return LED_OK;
}

get_time_base_t get_time_base_test = {
    .pf_get_time_base = get_time_test
};

#endif

#if 1
bsp_led_handler_t test_handler;

/* TODO:need ai write docs*/
HANDLER_LED_status_t (freertos_queue_create)(const uint32_t  queue_lenth,
                                          const uint32_t queue_item_size,
                                         void ** const     queue_handler)
{

    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    if(NULL == queue_handler || NULL == *queue_handler)
    {

#ifdef DEBUG
        DEBUG_OUT("arg is null");
#endif
        ret = HANDLER_LED_ERRORPARAMETER;
        return ret;
    }
    *queue_handler =  xQueueCreate(10,sizeof(uint32_t));

    if(NULL == *queue_handler)
    {

#ifdef DEBUG
        DEBUG_OUT("Queue create fasle");
#endif
        ret = HANDLER_LED_ERRORNOMEMORY;
        return ret;
    }
    
    return ret;

}

HANDLER_LED_status_t os_delay_ms(const uint32_t time)
{
    vTaskDelay(time);
    return HANDLER_LED_OK;
}
/* TODO:need ai write docs*/
HANDLER_LED_status_t (os_queue_delete)(void * const queue_handler)
{

}

/* TODO:need ai write docs*/
HANDLER_LED_status_t (os_queue_sent)(void * const queue_handler,
                                     void * const         iteme,
                                     uint32_t          time_out)
{

}

/* TODO:need ai write docs*/
HANDLER_LED_status_t (os_queue_recevie)(void * const queue_handler,
                                        void * const         iteme,
                                         uint32_t         time_out)
{

}

/* TODO:need ai write docs*/
HANDLER_LED_status_t  (os_critical_enter)(void)
{

}

/* TODO:need ai write docs*/
HANDLER_LED_status_t  (os_critical_exti)(void)
{

}

handler_led_os_api_t led_handler_api = {
    .pf_os_delay_ms = os_delay_ms,
    .pf_os_critical_enter = os_critical_enter,
    .pf_os_critical_exti = os_critical_exti,
    .pf_os_queue_create = freertos_queue_create,
    .pf_os_queue_delete = os_queue_delete,
    .pf_os_queue_sent = os_queue_sent,
    .pf_os_queue_recevie = os_queue_recevie
};


/* TODO:need ai write docs*/

HANDLER_LED_status_t (handler_get_time_base)(uint32_t* const time)
{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    if(NULL == time)
    {
#ifdef DEBUG
    DEBUG_OUT("pointer is null");
#endif
    ret = HANDLER_LED_ERRORRESOURCE;
    return ret;
    }

     *time = xTaskGetTickCount();
     return ret;
}

handler_led_get_time_base_t led_handler_get_time_base = {
    .pf_get_time_base = handler_get_time_base
};

#endif


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
  bsp_led_drive_t  test_drive1;
  bsp_led_drive_t  test_drive2;
  led_driver_inst(
          &test_drive1,
          &led_test_operation,
          &os_test,
          &get_time_base_test);

led_driver_inst(
          &test_drive2,
          &led_test_operation,
          &os_test,
          &get_time_base_test);


  led_handler_inst(&test_handler,
          &led_handler_api,
          &led_handler_get_time_base);

  uint32_t idx =0;
  uint32_t idx2 =0;
  test_handler.pf_led_mount(&test_drive1,&test_handler,&idx);
  test_handler.pf_led_mount(&test_drive2,&test_handler,&idx2);
  for(;;)
  {
		
    osDelay(1);
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

