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
 *
 * @file Bsp_led_handler.c
 * @par dependencies
 * stdint.h
 * stddef.h
 * bsp_led_handler.h
 *
 * @brief 
 * TODO:need ai write docs
 * @author Noser
 * @version 1.1
 * @date 2026-11-17
 */

//********************************Includes***********************************//
#include "Bsp_led_handler.h"
//********************************Includes***********************************//


//*********************************Defines***********************************//
#define INIT_PATERN 0xA5A5A5A5
//*********************************Defines***********************************//


/**
 * @brief 
 *
 * TODO:need ai write docs
 * @param led_handler
 *
 * TODO:need ai write docs
 * @return 
 * TODO:need ai write docs
 */
static HANDLER_LED_status_t led_handler_init_array(
    bsp_led_handler_t* led_handler)
{
    if (NULL == led_handler)
    {
        return HANDLER_LED_ERRORPARAMETER;
    }

    led_handler->led_drive_mount.led_index = 0;

    for (uint32_t i = 0; i < MAX_LED_X; i++)
    {
        led_handler->led_drive_mount.led_drive_array[i] = NULL;
    }

    return HANDLER_LED_OK;
}



/**
 * @brief 
 * TODO:need ai write docs
 *
 * @param led_driver
 * TODO:need ai write docs
 * @param led_handler
 * TODO:need ai write docs
 * @param led_idx
 * TODO:need ai write docs
 *
 * @return 
 * TODO:need ai write docs
 */
static HANDLER_LED_status_t led_handler_mount(bsp_led_drive_t*  led_driver,
                                    bsp_led_handler_t * led_handler,
                                              uint32_t*     led_idx)

{

    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    /*1.判断指针是否为空以及是否成功初始化*/

    if(NULL == led_driver||
       NULL == led_handler||
       INIT_HANDLER_LED_NO == led_handler->is_handler_led_init||
       INITIALIZATION_NO == led_driver->initialization
       )
    {

#ifdef DEBUG
        DEBUG_OUT("led mount arg is null");
#endif
        ret = HANDLER_LED_ERRORPARAMETER;
        return ret;
    }

    if(led_handler->led_drive_mount.led_index >=MAX_LED_X)
    {

#ifdef DEBUG
        DEBUG_OUT("led array is full");
#endif
 
        ret = HANDLER_LED_ERRORRESOURCE;
        return ret;
    }

    /*进入临界区*/
    led_handler->p_os_critical->pf_os_critical_enter();

    *led_idx = led_handler->led_drive_mount.led_index;
    
    led_handler->led_drive_mount.led_drive_array[led_handler-> \
        led_drive_mount.led_index++] = led_driver;

    led_handler->p_os_critical->pf_os_critical_exti();
    /*退出临界区*/
#ifdef DEBUG
        DEBUG_OUT("led mount is successfully");
#endif

    return ret;
    
}

static HANDLER_LED_status_t led_handler_control(
                             bsp_led_drive_t* led_driver,
                             bsp_led_handler_t* led_handler,
                             uint32_t need_control_led_idx)
{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;

    //1,检测参数
    if(NULL == led_driver || NULL == led_handler)
    {

#ifdef DEBUG
    DEBUG_OUT("led handler control arg is null");
#endif
    ret = HANDLER_LED_ERRORPARAMETER; 
    return ret;
    }
    //2.组装msg

    //3.发送msg

    static uint32_t test_data = 0xA5A5A5A5;

    ret = led_handler->\
        p_os_queue->\
        pf_os_queue_sent(led_handler->queue_handler,&test_data,0);

    if(HANDLER_LED_OK!=ret)
    {

#ifdef DEBUG
    DEBUG_OUT("queue sent fasle");
#endif
 
        ret = HANDLER_LED_ERROR;
        return ret;
    }

#ifdef DEBUG
    DEBUG_OUT("queue sent successful");
#endif
 
    return ret;
}


static void handler_led_task_code(void *self) 
{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    bsp_led_handler_t* handler_self = (bsp_led_handler_t*)self;

    uint32_t test_data = 0;
    while(1)
    {
        
     ret =handler_self->\
        p_os_queue->\
        pf_os_queue_recevie(handler_self->queue_handler,&test_data,0xffffffffUL);
   
     SEGGER_RTT_printf(0,"%x",test_data);
        
    }
}

/**
 * @brief 
 * TODO:need ai write docs
 *
 * @param self
 * TODO:need ai write docs
 * @param os_delay_ms
 * TODO:need ai write docs
 * @param os_queue
 * TODO:need ai write docs
 * @param os_critical
 * TODO:need ai write docs
 * @param get_time_base
 * TODO:need ai write docs
 *
 * @return 
 * TODO:need ai write docs
 */
HANDLER_LED_status_t led_handler_inst(
        bsp_led_handler_t          *const            self,
#ifdef OS_SUPPROT
        handler_led_os_delay_t     *const     os_delay_ms,
        handler_led_os_queue_t     *const        os_queue,
        handler_led_os_critical_t  *const     os_critical,
        handler_led_os_task_t      *const         os_task,
#endif
        handler_led_get_time_base_t*const   get_time_base)

{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;

#ifdef DEBUG
    DEBUG_OUT("led handler inst start");
#endif


    /*1,判断空*/
    if(NULL == self       ||
#ifdef OS_SUPPROT
       NULL == os_delay_ms||
       NULL == os_queue   ||
       NULL == os_critical||
       NULL == os_task    ||
#endif
       NULL == get_time_base)
    {

#ifdef DEBUG
        DEBUG_OUT("arg is null");
#endif
       ret = HANDLER_LED_ERRORPARAMETER; 
       return ret;
    }

    /*2,判断是否已经初始化过了*/
    if(INIT_HANDLER_LED_YES == self->is_handler_led_init)
    {

#ifdef DEBUG
        DEBUG_OUT("led_handler Initialization has been completed");
#endif
       ret =HANDLER_LED_ERRORRESOURCE; 
       return ret;
    }

    /*3.绑定外部提供的接口*/
    self->pf_get_time_base = get_time_base;
#ifdef OS_SUPPROT
    self->p_os_delay       = os_delay_ms;
    self->p_os_queue       = os_queue;
    self->p_os_critical    = os_critical;
    self->p_os_task        = os_task;
#endif
    
    /*4.初始化内部资源*/
    led_handler_init_array(self);
    /*5.绑定提供给外部使用的api*/
    self->pf_led_mount   = led_handler_mount;
    self->pf_led_control = led_handler_control;

    /*6.创建队列与任务*/
    
    /* TODO:need ai write docs*/
    if(HANDLER_LED_OK !=self-> \
            p_os_queue->\
            pf_os_queue_create(10,sizeof(uint32_t),&self->queue_handler))
    {

#ifdef DEBUG
    DEBUG_OUT("queue create is false");
#endif

       ret =HANDLER_LED_ERRORRESOURCE; 
       return ret;
    }   

    /* TODO:need ai write docs*/

    if(HANDLER_LED_OK !=self-> \
            p_os_task->\
            pf_os_task_create(handler_led_task_code,
                            "handler led task code",
                                                521,
                                                self,
                                                  0,
                                &self->task_handler))

    {

#ifdef DEBUG
    DEBUG_OUT("task create is false");
#endif

       /*创建任务失败,之前创建的队列要一同删除*/
       self->p_os_queue->pf_os_queue_delete(self->queue_handler);
       ret =HANDLER_LED_ERRORRESOURCE; 
       return ret;
    }

    self->is_handler_led_init = INIT_HANDLER_LED_YES;
#ifdef DEBUG
    DEBUG_OUT("led handler inst successful");
#endif

    return ret;
}
