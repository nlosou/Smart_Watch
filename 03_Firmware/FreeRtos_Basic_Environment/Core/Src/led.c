/******************************************************************************
 * Copyright (C) 2026.Noser
 * 
 * All Rights Reserved.
 * 
 * @file key.c
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
 * @version V1.0 2026-09-05
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

//********************************Includes***********************************//
#include "led.h"
//********************************Includes***********************************//


//********************************Defines***********************************//


//********************************Defines***********************************//

void led_init(led_info_t *led,GPIO_TypeDef *GPIOx,uint16_t led_use_pin)
{
    if(led == NULL)
    {
        return;;
    }
    else
    {
        led->LED_USE_GPIOx = GPIOx;
        led->LED_USE_PIN = led_use_pin;
    }
}

void led_toggle(led_info_t *led)
{
    if(led == NULL)
    {
        return;;
    }
    else
    {
        HAL_GPIO_TogglePin(led->LED_USE_GPIOx,led->LED_USE_PIN);
    }

}

