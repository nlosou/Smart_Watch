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
uint8_t blink_times = 0;
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

        printf("led task is active\r\n");
        if(pdTRUE == xQueueReceive(led_queue,&temp,portMAX_DELAY))
        {
            vTaskSuspendAll();
            printf("Get led_queue at [%d] tick\r\n",HAL_GetTick());
            xTaskResumeAll();
            if(LED_TOGGLE == temp)
            {
                //1.当从led_queue获得的是LED_TOGGLE时
                //led_toggle(&g_led1);
                temp = LED_OFF;

                vTaskSuspendAll();
                printf("led toggle\r\n");
                xTaskResumeAll();

                blink_times = 0;     
                //1.1 开启TIM2中断,开启TIM2 CH4的PWM输出
                HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
                __HAL_TIM_ENABLE_IT(&htim2,TIM_IT_UPDATE);

            }
            else if(LED_BLINK_3 == temp)
            {
                //2.当从led_queue获得的是LED_BLINK时

                //led_blink_3(&g_led1);
                temp = LED_OFF;
                vTaskSuspendAll();
                printf("Led blink\r\n");
                xTaskResumeAll();

                blink_times = 10;     
                //2.1 开启TIM2中断,开启TIM2 CH4的PWM输出
                HAL_TIM_PWM_Start(&htim2,TIM_CHANNEL_4);
                HAL_TIM_Base_Start_IT(&htim2);
            }
            else
            {
                //led_off(&g_led1);
                temp = LED_OFF;
            }
        }
        else
        {
            printf("led_Queue is empty at [%d] tick\r\n",HAL_GetTick());
        }

        
        }
}


/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    //1.2 并在中断里维护一个flag 
    //1.3 当flag == 1时,关闭TIM2中断,TIM_CH4的PWM输出
    //2.2 并在中断里维护一个flag 
    //2.3 当flag == 10时,关闭TIM2中断,TIM_CH4的PWM输出

     if (htim->Instance == TIM2) {

        pwm_times++;
        if(pwm_times > blink_times)
        {
            pwm_times = 0;     
            HAL_TIM_Base_Stop_IT(&htim2);
            HAL_TIM_PWM_Stop(&htim2,TIM_CHANNEL_4);
        }


    }   
    
    
    if (htim->Instance == TIM1) {
        HAL_IncTick();
    }
}



