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
#include "semphr.h"
//********************************Includes***********************************//


//********************************Defines***********************************//


//********************************Defines***********************************//
QueueHandle_t                        key_queue = 0;
QueueHandle_t                  key_interrupt_queue = NULL;
key_interrupt_data_t            key_interrupt_data = {0,0};



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
key_result_t key_init(key_info_t *key,GPIO_TypeDef* GPIOx,uint16_t pin)
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
key_result_t key_scan(key_info_t               *key,
                      TickType_t short_pressed_time,
                      key_event_t         *key_event)

{
    key_result_t key_ret = KEY_ERROR;
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
                    //1.当持续时间大于阈值时,应立刻结束状态机
                    if(short_pressed_time < (xTaskGetTickCount() - key->KEY_TICK_START))
                    {
                        *key_event = KEY_LONG_PRESSED;
                         key_ret = KEY_OK;
                         key->g_key_state = WAIT_RELEASE;
                    }
                }
                else 
                {
                    key->KEY_TICK_END = xTaskGetTickCount();
                    key->g_key_state = INSPECTING_COMPLETE;
                }
                break;
            case WAIT_RELEASE:
                if(HAL_GPIO_ReadPin(key->KEY_USE_GPIOx,key->KEY_USE_PIN))
                {
                        key->g_key_state = NOT_INSPECTING;
                         *key_event =KEY_NOT_PRESSED;
                        key_ret = KEY_ERROR;
                }
                break;
            case INSPECTING_COMPLETE:
                if( short_pressed_time > (key->KEY_TICK_END - key->KEY_TICK_START))
                {
                        *key_event =KEY_SHORT_PRESSED;
                }
                key->g_key_state = NOT_INSPECTING;
                key_ret = KEY_OK;
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

key_event_t key_check_pressedType(key_interrupt_data_t key_interrupt_data)
{
     static uint32_t                  tick_start = 0; 
     static uint32_t                  tick_end   = 0; 
     key_event_t           key_ret = KEY_NOT_PRESSED;
     key_edge_state_t      key_edge_state  = key_interrupt_data.KEY_EDGE_STATE;

     switch(key_edge_state)
     {
         case FALLING_EDGE:
             tick_start = key_interrupt_data.SYS_CURRENT_TICK;
             break;
         case RISEING_EDGE:
             tick_end = key_interrupt_data.SYS_CURRENT_TICK;
             if(tick_end  - tick_start < 100)
             {
                    key_ret = KEY_NOT_PRESSED;
             }
             else if(tick_end  - tick_start > 100 && tick_end - tick_start <1000)
             {
                    key_ret = KEY_SHORT_PRESSED;
             }
             else
             {
                    key_ret = KEY_LONG_PRESSED;
             }
             break;
         default:
             break;
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

void Key_task(void*argument)
{
    /**     Variables (in task stack)             **/
    uint32_t                           key_count = 0;
    uint8_t                       start_scan_key = 0;
    key_event_t          key_event = KEY_NOT_PRESSED;
    /**     Variables (in task stack)             **/


    /**     Variables (in os heap)             **/

   key_interrupt_queue = xQueueCreate(1,sizeof(key_interrupt_data_t));
   key_queue = xQueueCreate(1,sizeof(key_event_t));

    /**     Variables (in os heap)             **/

   if(NULL ==key_interrupt_queue)
   {
     printf("key_interrupt_queue is failed\r\n");
   }
   else
   {
     printf("key_interrupt_queue is successfully\r\n");
   }

  if(NULL == key_queue)
  {
    printf("key_queue created failed \r\n");
  }
  else
  {

    printf("key_queue created successfully \r\n");
  }

    for(;;)
    {
        if(pdTRUE == xQueueReceive(key_interrupt_queue,
                    &key_interrupt_data,
                    portMAX_DELAY))
        {

            if(key_interrupt_data.KEY_EDGE_STATE == FALLING_EDGE)
            {
                printf("FALLING come at [%d] tick\r\n",HAL_GetTick());
            }
            else
            {
                printf("RISING come at [%d] tick\r\n",HAL_GetTick());
            }

            key_event = key_check_pressedType(key_interrupt_data);


            if(key_event == KEY_SHORT_PRESSED)
            {

                printf("short pressed come at [%d] tick\r\n",HAL_GetTick());
            }
            else if(key_event == KEY_LONG_PRESSED)
            {
                printf("long pressed come at [%d] tick\r\n",HAL_GetTick());
            }

            if(key_event!=KEY_NOT_PRESSED)
            {
                if(pdPASS == xQueueSendToBack(key_queue,&key_event,0))
                {
                    printf("Sent key_event to key_queue at [%d] tick\r\n",
                            HAL_GetTick());
                }
                else
                {
                    printf("key_queue is full at [%d] tick\r\n",HAL_GetTick());
                }
            }
        }
        else
        {
            printf("key_interrupt_queue is empty at [%d] tick\r\n",HAL_GetTick());
        }
     }
}


/**
  * @brief 
  * @retval None
  */
void Key_Interrupt_Handler(void)
{
    BaseType_t xHigherPriorityTaskWoken;

    xHigherPriorityTaskWoken              =       pdFALSE;
    g_key_interrupt_data.SYS_CURRENT_TICK = HAL_GetTick();  
    
    if(key_exti_config.Trigger == EXTI_TRIGGER_FALLING )
    {
        g_key_interrupt_data.KEY_EDGE_STATE = FALLING_EDGE;
        key_exti_config.Trigger = EXTI_TRIGGER_RISING;
    }
    else if(key_exti_config.Trigger == EXTI_TRIGGER_RISING)
    {
        g_key_interrupt_data.KEY_EDGE_STATE = RISEING_EDGE;
        key_exti_config.Trigger = EXTI_TRIGGER_FALLING;
    }    
    HAL_EXTI_SetConfigLine(&key_exti_handle,&key_exti_config);
    xQueueSendToBackFromISR(key_interrupt_queue,&g_key_interrupt_data,&xHigherPriorityTaskWoken);
    portYIELD_FROM_ISR(&xHigherPriorityTaskWoken);
}
