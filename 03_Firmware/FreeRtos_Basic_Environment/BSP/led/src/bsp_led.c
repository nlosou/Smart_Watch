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
//********************************Includes***********************************//


//********************************Defines***********************************//

QueueHandle_t led_queue = 0;

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
        if(pdTRUE == xQueueReceive(led_queue,&temp,portMAX_DELAY))
        {
            vTaskSuspendAll();
            printf("Get led_queue at [%d] tick\r\n",HAL_GetTick());
            xTaskResumeAll();
            if(LED_TOGGLE == temp)
            {
                led_toggle(&g_led1);
                temp = LED_OFF;

                vTaskSuspendAll();
                printf("led toggle\r\n");
                xTaskResumeAll();
            }
            else if(LED_BLINK_3 == temp)
            {
                led_blink_3(&g_led1);
                temp = LED_OFF;
                vTaskSuspendAll();
                printf("Led blink\r\n");
                xTaskResumeAll();
            }
            else
            {
                led_off(&g_led1);
                temp = LED_OFF;
            }
        }
        else
        {
            printf("led_Queue is empty at [%d] tick\r\n",HAL_GetTick());
        }
    }
}

