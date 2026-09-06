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

#ifndef LED_H__
#define LED_H__

#include "gpio.h"
#include "cmsis_os2.h"

//********************************Includes***********************************//


//********************************Defines***********************************//

typedef struct{
    GPIO_TypeDef *LED_USE_GPIOx;
    uint16_t LED_USE_PIN;
}led_info_t;


//********************************Defines***********************************//

void led_init(led_info_t *led,GPIO_TypeDef *GPIOx,uint16_t led_use_pin);

void led_toggle(led_info_t *led);

extern led_info_t g_led1;

#endif





