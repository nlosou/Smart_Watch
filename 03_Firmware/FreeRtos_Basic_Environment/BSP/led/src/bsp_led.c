/******************************************************************************
 * Copyright (C) 2026.Noser
 * 
 * All Rights Reserved.
 * 
 * @file led.c
 * 
 * @par dependencies 
 * 
 * @author Noser
 * 
 * @brief 提供led的:wapi
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.1 2026-09-07
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

//********************************Includes***********************************//
#include "bsp_led.h"
#include "tim.h"
//********************************Includes***********************************//


//********************************Defines***********************************//

QueueHandle_t led_queue = 0;
static uint32_t g_blink_times = 0; // 1: blink 1, 5： blink 5
static uint32_t g_blink_order = 0;
volatile uint8_t pwm_times = 0;
//********************************Defines***********************************//

/**
 * @brief 
 * 
 * Steps:
 *  
 * @param[in] void 
 * 
 * @return 
 * 
 * */
led_status_t led_init(led_info_t *led,GPIO_TypeDef *GPIOx,uint16_t led_use_pin)
{
    if(NULL == led || NULL == GPIOx)
    {
        return LED_ERRORRESOURCE;
    }
    else
    {
        led->LED_USE_GPIOx = GPIOx;
        led->LED_USE_PIN = led_use_pin;
        return LED_OK;
    }
}

/**
 * @brief 
 * Flip the LED task
 * Steps:
 *  
 * @param[in] void 
 * 
 * @return 
 * 
 * */
led_status_t led_toggle(led_info_t *led)
{
    if(NULL == led)
    {
        return LED_ERRORRESOURCE;
    }
    else
    {
        HAL_GPIO_TogglePin(led->LED_USE_GPIOx,led->LED_USE_PIN);
        return LED_OK;
    }

}



/**
 * @brief 
 * Flip the LED task
 * Steps:
 *  
 * @param[in] void 
 * 
 * @return 
 * 
 * */
led_status_t led_off(led_info_t *led)
{
    if(NULL == led)
    {
        return LED_ERRORRESOURCE;
    }
    else
    {
        HAL_GPIO_WritePin(led->LED_USE_GPIOx,led->LED_USE_PIN,0);
        return LED_OK;
    }

}


/**
 * @brief 
 * Flip the LED task
 * Steps:
 *  
 * @param[in] void 
 * 
 * @return 
 * 
 * */
led_status_t led_on(led_info_t *led)
{
    if(NULL == led)
    {
        return LED_ERRORRESOURCE;
    }
    else
    {
        HAL_GPIO_WritePin(led->LED_USE_GPIOx,led->LED_USE_PIN,1);
        return LED_OK;
    }

}

/**
 * @brief 
 * Flip the LED task
 * Steps:
 *  
 * @param[in] void 
 * 
 * @return 
 * 
 * */

led_status_t led_blink_3(led_info_t *led)
{
    if(NULL == led)
    {
        return LED_ERRORRESOURCE;
    }
    else
    {
        for(uint8_t i = 0 ; i < 6 ;i++)
        {
            HAL_GPIO_TogglePin(led->LED_USE_GPIOx,led->LED_USE_PIN);
            osDelay(300);

        }
        return LED_OK;
    }

}

/**
 * @brief 
 * Flip the LED task
 * Steps:
 *  
 * @param[in] void 
 * 
 * @return 
 * 
 * */
led_status_t led_select_function(led_info_t *led,led_function_t led_function_state)
{
    if(NULL == led)
    {
        return LED_ERRORRESOURCE;
    }
    else
    {
        switch(led_function_state)
        {
            case LED_TOGGLE:
                //1.当从led_queue获得的是LED_TOGGLE时
                //led_toggle(&g_led1);

                vTaskSuspendAll();
                printf("led toggle\r\n");
                xTaskResumeAll();

               break;
            case LED_BLINK_1:
                vTaskSuspendAll();
                printf("Led blink 1\r\n");
                xTaskResumeAll();

                g_blink_times = 1;     
                g_blink_order = 0;
                break;
            case LED_BLINK_3:
                //led_blink_3(&g_led1);
                vTaskSuspendAll();
                printf("Led blink 3\r\n");
                xTaskResumeAll();

                g_blink_times = 3;     
                g_blink_order = 0;
                break;
            case LED_BLINK_10:
                //2.当从led_queue获得的是LED_BLINK时
                vTaskSuspendAll();
                printf("Led blink 3\r\n");
                xTaskResumeAll();

                g_blink_times = 10;     
                g_blink_order = 0;
               break;
            default:
                led_off(&g_led1);
                break;
        }
        return LED_OK;
    }
}


/**
 * @brief 
 * Flip the LED task
 * Steps:
 *  
 * @param[in] void 
 * 
 * @return 
 * 
 * */
void led_toggle_task(void*argument)
{
    /**     Variables (in task stack)             **/

    led_function_t temp = LED_OFF;

    /**     Variables (in task stack)             **/

    /**     Variables (in os heap)             **/
    
    led_queue = xQueueCreate(1,sizeof(led_function_t));

    /**     Variables (in os heap)             **/

   if(NULL == led_queue)
   {
     printf("led_queue created failed \r\n");
   }
   else
   {
     printf("led_queue created successfully \r\n");
   }

    for(;;)
    {

        printf("led task is active\r\n");
        if(pdTRUE == xQueueReceive(led_queue,&temp,portMAX_DELAY))
        {
            vTaskSuspendAll();
            printf("Get led_queue at [%d] tick\r\n",HAL_GetTick());
            xTaskResumeAll();
            led_select_function(&g_led1,temp);
            
        }
        else
        {
            printf("led_Queue is empty at [%d] tick\r\n",HAL_GetTick());
        }
        
    }
}

/**
  * @brief  led to tim2 callback
  * @note   200ms触发一次中断
  * @param  void
  * @retval None
  */
void led_tim_Callback(void)
{   
    if (g_blink_times > 0)
    {
        if (g_blink_order % 2 == 0)
        {
            led_on(&g_led1);
        }
        else
        {
            led_off(&g_led1);
            g_blink_times--;
        }
        g_blink_order++;
    }
    else
    {
        g_blink_order = 0;
    }
}
