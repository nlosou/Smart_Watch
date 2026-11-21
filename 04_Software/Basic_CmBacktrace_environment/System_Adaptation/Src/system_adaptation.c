

/* Copyright (C) 
 * 2026 - Noser
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 * ))
 * @file system_adaptation.c
 *
 * @brief 
 * @author Noser
 * @version 1.1
 * @date 2026-11-20
 */

//********************************Includes***********************************//

#include "system_adaptation.h"

//********************************Includes***********************************//


//*********************************Defines***********************************//

#define KEEP_SYMBOL __attribute__((used))

//********************************Variables**********************************//
__attribute__ ((used,section("bsp_target")))
static bsp_led_handler_t test_handler ;

__attribute__ ((used,section("bsp_target")))
static bsp_led_drive_t   led_driver1;
//********************************Variables**********************************//


//-------application code --------------------------------------------------//



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

LED_status_t FreeRTOS_delay_ms(const uint32_t time)
{
    vTaskDelay(time);
    return LED_OK;
}

LED_status_t get_time_test(uint32_t* const time)
{
    *time = HAL_GetTick();
    return LED_OK;
}

/* 创建 FreeRTOS 队列 */
HANDLER_LED_status_t (os_queue_create)(const uint32_t  queue_lenth,
                                          const uint32_t queue_item_size,
                                         void ** const     queue_handler)
{

    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    *queue_handler =  xQueueCreate(10,sizeof(uint32_t));

    if(NULL == *queue_handler)
    {

#ifdef DEBUG
        DEBUG_OUT("Queue create fasle");
#endif
        ret = HANDLER_LED_ERRORNOMEMORY;
        return ret;
    }

 #ifdef DEBUG
        DEBUG_OUT("Queue create successful");
#endif
    
    return ret;

}

HANDLER_LED_status_t os_delay_ms(const uint32_t time)
{
    vTaskDelay(time);
    return HANDLER_LED_OK;
}
/* 删除 FreeRTOS 队列 */
HANDLER_LED_status_t (os_queue_delete)(void * const queue_handler)
{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    if(NULL == queue_handler)
    {
#ifdef DEBUG
        DEBUG_OUT("queue_handler is null");
#endif
        ret = HANDLER_LED_ERRORRESOURCE;
        return ret;
    }

    vQueueDelete((QueueHandle_t)queue_handler);
    return ret;
}

/* 向 FreeRTOS 队列发送数据 */
HANDLER_LED_status_t (os_queue_sent)(void * const queue_handler,
                                     void * const         iteme,
                                     uint32_t          time_out)
{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    if(NULL == queue_handler||NULL == iteme)
    {
#ifdef DEBUG
        DEBUG_OUT("queue_handler is null or iteme is null");
#endif
        ret = HANDLER_LED_ERRORRESOURCE;
        return ret;
    }
    BaseType_t ret_queue = 0;
    ret_queue = xQueueSend((QueueHandle_t)queue_handler,iteme,time_out);
    if(pdTRUE != ret_queue)
    {

#ifdef DEBUG
        DEBUG_OUT("os queue sent is false");
#endif
        ret = HANDLER_LED_ERRORRESOURCE;
        return ret;
 
    }
    
#ifdef DEBUG
        DEBUG_OUT("os queue sent is successful");
#endif
 
    return ret;

}

/* 从 FreeRTOS 队列接收数据 */
HANDLER_LED_status_t (os_queue_recevie)(void * const queue_handler,
                                        void * const         iteme,
                                         uint32_t         time_out)
{

    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    if(NULL == queue_handler||NULL == iteme)
    {
#ifdef DEBUG
        DEBUG_OUT("queue_handler is null or iteme is null");
#endif
        ret = HANDLER_LED_ERRORRESOURCE;
        return ret;
    }

    BaseType_t queue_ret = xQueueReceive((QueueHandle_t)queue_handler,
                                                              iteme,
                                                      time_out);
    if (pdTRUE != queue_ret)
    {
        
#ifdef DEBUG
        DEBUG_OUT("queue recevie is false");
#endif
        ret = HANDLER_LED_ERRORRESOURCE;
        return ret;
    }
    
#ifdef DEBUG
        DEBUG_OUT("queue receive is successful");
#endif
 
    return ret;
    
}

/* 进入临界区 */
HANDLER_LED_status_t  (os_critical_enter)(void)
{
#ifdef DEBUG
        DEBUG_OUT("critical enter");
#endif
 
    HANDLER_LED_status_t ret = HANDLER_LED_OK;
	taskENTER_CRITICAL();
    return ret;
}

/* 退出临界区 */
HANDLER_LED_status_t  (os_critical_exti)(void)
{
#ifdef DEBUG
        DEBUG_OUT("critical exti");
#endif
 
    HANDLER_LED_status_t ret = HANDLER_LED_OK;
	taskEXIT_CRITICAL();
    return ret;
}


/* 获取系统时基 */
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

HANDLER_LED_status_t handler_led_os_task_create(
                                              void * const       task_code,
                                              const char * const task_name,
                                              uint32_t     task_stack_size,
                                          void     * const      parameters,
                                          uint32_t                priority,
                                          void    **const     task_handler)

{

    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    if(NULL == task_code||
       NULL == task_name||
       NULL == parameters
       )
    {

#ifdef DEBUG
        DEBUG_OUT("arg is null");
#endif
        ret = HANDLER_LED_ERRORPARAMETER;
        return ret;
    }

      if(pdPASS !=  xTaskCreate(task_code,
                                    task_name,
                        task_stack_size,
                             parameters,
                                 priority,
          (TaskHandle_t*)task_handler))
      {
#ifdef DEBUG
        DEBUG_OUT("task create fasle");
#endif
        ret = HANDLER_LED_ERRORNOMEMORY;
        return ret;
 
       }

#ifdef DEBUG
        DEBUG_OUT("task create successful");
#endif

        return ret;
}

HANDLER_LED_status_t (os_task_delete)(void * const task_handler)
{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    if(NULL == task_handler)
    {
#ifdef DEBUG
        DEBUG_OUT("task _handler is null");
#endif
        ret = HANDLER_LED_ERRORRESOURCE;
        return ret;
    }

    vQueueDelete((QueueHandle_t)task_handler);

#ifdef DEBUG
        DEBUG_OUT("task _handler is delete successfully");
#endif
 
    return ret;
}

/**
 * @brief LED 操作实例。
 */
 KEEP_SYMBOL led_operation_t led_test_operation = {
    .pf_led_on = led_on,
    .pf_led_off = led_off
};


/**
 * @brief OS 延时接口实例。
 */
 KEEP_SYMBOL os_api_t os_test = {
    .pf_os_delay_ms = FreeRTOS_delay_ms,
};


/**
 * @brief 时基获取接口实例。
 */
 KEEP_SYMBOL get_time_base_t get_time_base_test = {
    .pf_get_time_base = get_time_test
};


/**
 * @brief handler OS 延时接口实例。
 */
 KEEP_SYMBOL handler_led_os_delay_t      led_handler_os_delay      = {
    .pf_os_delay_ms   = os_delay_ms,
};

/**
 * @brief handler 时基获取接口实例。
 */
KEEP_SYMBOL handler_led_get_time_base_t led_handler_get_time_base = {
    .pf_get_time_base = handler_get_time_base
};

/**
 * @brief handler OS 队列接口实例。
 */
KEEP_SYMBOL handler_led_os_queue_t     led_handler_os_queue       = {
    .pf_os_queue_create  = os_queue_create,
    .pf_os_queue_delete  = os_queue_delete,
    .pf_os_queue_recevie = os_queue_recevie,
    .pf_os_queue_sent    = os_queue_sent
};

/**
 * @brief handler OS 临界区接口实例。
 */
 KEEP_SYMBOL handler_led_os_critical_t  led_handler_os_critical    = {
    .pf_os_critical_enter = os_critical_enter,
    .pf_os_critical_exti  = os_critical_exti
};

/**
 * @brief handler OS 任务接口实例。
 */
 KEEP_SYMBOL handler_led_os_task_t      led_handler_os_task        = {
    .pf_os_task_create = handler_led_os_task_create,
    .pf_os_task_delete = os_task_delete
};


/**
 * @brief 初始化系统适配层。
 *
 * 依次实例化 LED 驱动与 LED handler。
 * @return ADAPT_OK 成功；ADAPT_ERRORRESOURCE 初始化失败。
 */

 KEEP_SYMBOL system_adapt_status_t system_adapt_init(void)
{

    LED_status_t         ret1  = LED_OK;
    HANDLER_LED_status_t ret2  = HANDLER_LED_OK;
    system_adapt_status_t ret3 = ADAPT_OK;

    ret1 = led_driver_inst(
              &led_driver1,
              &led_test_operation,
              &os_test,
              &get_time_base_test);

    if(LED_OK!=ret1)
    {
        
#ifdef DEBUG
        DEBUG_OUT("led driver inst false");
#endif
        ret3 = ADAPT_ERRORRESOURCE;
        return ret3;
    }
    else
    {

#ifdef DEBUG
        DEBUG_OUT("led driver inst successful");
#endif

    }



    ret2 = led_handler_inst(&test_handler,
              &led_handler_os_delay,
              &led_handler_os_queue,
              &led_handler_os_critical,
              &led_handler_os_task,
              &led_handler_get_time_base
              );
    if(HANDLER_LED_OK!=ret2)
    {
        
#ifdef DEBUG
        DEBUG_OUT("led handler inst false");
#endif
        ret3 = ADAPT_ERRORRESOURCE;
        return ret3;
    }
    else
    {

#ifdef DEBUG
        DEBUG_OUT("led handler inst successful at system_adapt_init");
#endif

    }

    return ret3;
}

void Test1(void)
{

    uint32_t idx = 0;
    uint32_t idx2 = 0;
 
    bsp_led_handler_t *p_test_handler = (bsp_led_handler_t*)0x08060000 ;

    bsp_led_drive_t   *p_led_driver1  = (bsp_led_drive_t*)0x08060054;


    p_test_handler->pf_led_mount(p_led_driver1,
                              p_test_handler,
                                          &idx);


    p_test_handler->pf_led_control(p_test_handler,
                                    1000,
                                          10,
                   PROPORTON_1_1,
                               idx2);

    /*
    uint32_t idx = 0;
    uint32_t idx2 = 0;
      
    test_handler.pf_led_mount(&led_driver1,
                              &test_handler,
                                          &idx);


    test_handler.pf_led_control(&test_handler,
                                    1000,
                                          10,
                   PROPORTON_1_1,
                               idx2);
    */

}
//-------application code --------------------------------------------------//




