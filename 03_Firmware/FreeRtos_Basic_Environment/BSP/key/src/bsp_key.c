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
 * @version V1.1 2026-09-07
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

//********************************Includes***********************************//

#include "bsp_key.h"
//********************************Includes***********************************//


//********************************Defines***********************************//

QueueHandle_t                   key_queue = 0;

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
key_status_t key_init(key_info_t *key,GPIO_TypeDef* GPIOx,uint16_t pin)
{
    if(NULL == key || NULL == GPIOx)    
    {
        return KEY_ERROGETINFO;
    }
    else
    {
        key->KEY_USE_GPIOx = GPIOx;
        key->KEY_USE_PIN = pin;
        key->g_key_state = NOT_INSPECTING;

        return KEY_OK;
    }
}

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
key_status_t key_scan(
              key_info_t *key,
              void (*function_call_back)(void*),
              void*argument
             )
{
    key_status_t key_ret = KEY_ERRORTIMEOUT;
    if(NULL == key)
    {
        return KEY_ERRORRESOURCE; 
    }
    else
    {
        switch(key->g_key_state)
        {  
            case NOT_INSPECTING:
                if(!HAL_GPIO_ReadPin(key->KEY_USE_GPIOx,key->KEY_USE_PIN))
                {
                    key->g_key_state = INSPECTING;
                    key->KEY_TICK_START = xTaskGetTickCount();
                }
                break;
            case INSPECTING:
                if(!HAL_GPIO_ReadPin(key->KEY_USE_GPIOx,key->KEY_USE_PIN))
                {
                    key->g_key_state = INSPECTING;
                }
                else
                {
                    key->g_key_state = INSPECTING_COMPLETE;
                    key->KEY_TICK_START = xTaskGetTickCount();
                }
                break;
            case INSPECTING_COMPLETE:
                if(SHORT_LONG_KEY > (key->KEY_TICK_END - key->KEY_TICK_START))
                {
                    key_ret = KEY_SHORT;
                }
                else
                {
                    key_ret = KEY_LONG;
                }
                key->g_key_state = NOT_INSPECTING;
                break;

        }
    }
    return key_ret;
}

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

void key_function_callback(void*argument)
{
    
    uint32_t* temp = (uint32_t*)argument;
    if(pdPASS == xQueueSendToBack(key_queue,temp,0))
    {
        (*temp)^=(*temp);
    }
    else
    {
        printf("queue full\r\n");
    }
}

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

void Key_task(void*argument)
{
    uint32_t key_count = 0;
    key_status_t key_scan_ret = KEY_OK;
    for(;;)
    {
        osDelay(20);
        key_scan_ret = key_scan(
                &g_key1,
                key_function_callback,
                &key_count);
        if(pdPASS == xQueueSendToBack(key_queue,&key_scan_ret,0))
        {
            printf("Sent to key queue success\r\n");
        }
        else
        {
            printf("queue full\r\n");
        }
            
        }
}

