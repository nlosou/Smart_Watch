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
 * @brief 轮询检测按键是否按下
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

#include "key.h"

//********************************Includes***********************************//


//********************************Defines***********************************//


//********************************Defines***********************************//


void key_init(key_t *key,GPIO_TypeDef* GPIOx,uint16_t pin)
{
    if(key == NULL || GPIOx ==NULL)    
    {
        return;
    }
    else
    {
        key->KEY_USE_GPIOx = GPIOx;
        key->KEY_USE_PIN = pin;
        key->g_key_state = KEY_IDLE;
    }
}

void key_scan(
              key_t *key,
              void (*function_call_back)(void*),
              void*argument
             )
{
    if(key == NULL)
    {
        return; 
    }
    else
    {
        switch(key->g_key_state)
        {  
            case KEY_IDLE:
                if(!HAL_GPIO_ReadPin(key->KEY_USE_GPIOx,key->KEY_USE_PIN))
                {
                    key->g_key_state = KEY_PRESSED;
                    osDelay(20);
                }
                break;
            case KEY_PRESSED:
                if(!HAL_GPIO_ReadPin(key->KEY_USE_GPIOx,key->KEY_USE_PIN))
                {
                    key->g_key_state = KEY_HELD;
                }
                else
                {
                    key->g_key_state = KEY_IDLE;
                }
                break;
            case KEY_HELD:
                if(HAL_GPIO_ReadPin(key->KEY_USE_GPIOx,key->KEY_USE_PIN))
                {
                    
                    key->g_key_state = KEY_RELEASED;
                    osDelay(20);
                }
                else
                {
                }
                break;;
            case KEY_RELEASED:
                if(HAL_GPIO_ReadPin(Key_GPIO_Port,Key_Pin))
                {
                    key->g_key_state = KEY_IDLE;
                    function_call_back(argument);
                }
                else
                {
                    key->g_key_state = KEY_HELD;
                }
                break;
        }
    }
}
