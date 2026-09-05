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
 * @brief 提供按键api
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

#ifndef KEY_H__
#define KEY_H__

#include "gpio.h"
#include "cmsis_os2.h"

//********************************Includes***********************************//


//********************************Defines***********************************//

/* 按键状态          */
typedef enum
{
    KEY_IDLE = 0,
    KEY_PRESSED,
    KEY_HELD,
    KEY_RELEASED

}key_state_t;

typedef struct{
    GPIO_TypeDef *KEY_USE_GPIOx;
    uint16_t KEY_USE_PIN;
    key_state_t g_key_state;
}key_t;


//********************************Defines***********************************//

void key_init(key_t *key,GPIO_TypeDef* GPIOx,uint16_t pin);

void key_scan(key_t *key,
              void (*function_call_back)(void*),
              void*argument
             );


extern key_t g_key1;

#endif





