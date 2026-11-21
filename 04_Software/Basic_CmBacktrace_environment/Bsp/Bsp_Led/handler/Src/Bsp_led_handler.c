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
 * @brief LED handler 模块实现。
 * 提供 LED 驱动对象挂载、控制和任务事件处理。
 * @author Noser
 * @version 1.1
 * @date 2026-11-17
 */

//********************************Includes***********************************//
#include "Bsp_led_handler.h"
//********************************Includes***********************************//


//*********************************Defines***********************************//
//*********************************Defines***********************************//


/**
 * @brief 初始化 LED 驱动挂载数组。
 *
 * 将挂载计数清零，并把驱动指针数组中的每个元素置为空。
 * @param led_handler LED handler 对象。
 *
 * @return HANDLER_LED_OK 成功；HANDLER_LED_ERRORPARAMETER 参数为空。
 */
 __attribute__((used)) static HANDLER_LED_status_t led_handler_init_array(
    bsp_led_handler_t* led_handler)
{
    if (NULL == led_handler)
    {
        return HANDLER_LED_ERRORPARAMETER;
    }

    led_handler->led_drive_mount.led_index = 0;

    for (uint32_t i = 0; i < MAX_LED_X; i++)
    {
        led_handler->led_drive_mount.led_drive_array[i] =(bsp_led_drive_t*) INIT_PATERN;
    }

    return HANDLER_LED_OK;
}



/**
 * @brief 将 LED 驱动挂载到 handler 的驱动数组。
 *
 * 校验参数与初始化状态，在临界区中保存驱动指针并返回分配下标。
 * @param led_driver 待挂载的 LED 驱动对象。
 * @param led_handler LED handler 对象。
 * @param led_idx 输出分配到的挂载下标。
 *
 * @return HANDLER_LED_OK 成功；HANDLER_LED_ERRORPARAMETER 参数错误；
 *         HANDLER_LED_ERRORRESOURCE 驱动数组已满。
 */
__attribute__((used))static HANDLER_LED_status_t led_handler_mount(bsp_led_drive_t*  led_driver,
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
        led_drive_mount.led_index] = led_driver;
		
		if(led_handler->led_drive_mount.led_drive_array[led_handler-> \
        led_drive_mount.led_index] != led_driver)
		{
			   led_handler->p_os_critical->pf_os_critical_exti();
    /*退出临界区*/
#ifdef DEBUG
        DEBUG_OUT("led mount is write data false");
#endif
			  ret = HANDLER_LED_ERRORRESOURCE;
        return ret;
		}
		
		led_handler->led_drive_mount.led_index++;
    led_handler->p_os_critical->pf_os_critical_exti();
    /*退出临界区*/
#ifdef DEBUG
        DEBUG_OUT("led mount is successfully");
#endif

    return ret;
    
}

/**
 * @brief 处理一次 LED 闪烁事件。
 *
 * 根据占空比计算点亮时间，并循环执行 LED 亮灭与延时。
 * @param self LED 驱动对象。
 *
 * @return HANDLER_LED_OK 成功；HANDLER_LED_ERRORPARAMETER 参数错误。
 */
  __attribute__((used)) static HANDLER_LED_status_t __event_process(bsp_led_drive_t* self)
{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    /*1.检查self是否为NULL,对象是否已经初始化完毕*/

    if(NULL == self||INITIALIZATION_NO == self->initialization)
    {
#ifdef DEBUG
        DEBUG_OUT("led is null or led is not init");
#endif
        ret = HANDLER_LED_ERRORPARAMETER;
        return ret; 
    }

    /*2.编写led控制逻辑*/
    /*获取led亮多久*/
    uint32_t CMD_GET_SWITCH_TIME = 0;
    if(PROPORTON_1_3 ==self->led_proporton_on_off_t)
    {
        CMD_GET_SWITCH_TIME = self->blink_cycle_ms * (1/(1+3));
    }
    else if(PROPORTON_1_2 == self->led_proporton_on_off_t)
    {
        CMD_GET_SWITCH_TIME = self->blink_cycle_ms * (1/(1+2));
    }
    else if(PROPORTON_1_1 == self->led_proporton_on_off_t)
    {

        CMD_GET_SWITCH_TIME = self->blink_cycle_ms * (1/(1+1));
    }
    else
    {

#ifdef DEBUG
        DEBUG_OUT("led is null or led is not init");
#endif
        ret = HANDLER_LED_ERRORPARAMETER;
        return ret; 
     
    }

    for(uint32_t i = 0 ; i < self->blink_times; i++)
    {
#ifdef DEBUG
        DEBUG_OUT("led blink start");
#endif
 
        self->p_led_ops_inst->pf_led_on();  
        self->p_os_ops_api->pf_os_delay_ms(CMD_GET_SWITCH_TIME);
        self->p_led_ops_inst->pf_led_off();  
        self->p_os_ops_api->pf_os_delay_ms(self->blink_cycle_ms - CMD_GET_SWITCH_TIME);
    }

#ifdef DEBUG
        DEBUG_OUT("led blink end");
#endif
 
    return ret;
}

/**
 * @brief 
 *
 * @param 
 * @param blink_cycle_ms
 * @param blink_times
 * @param led_proporton_on_off_t
 * @param need_control_led_idx
 */
  __attribute__((used)) static HANDLER_LED_status_t led_handler_control(bsp_led_handler_t* led_handler,
                                                       uint32_t blink_cycle_ms,
                                                          uint32_t blink_times,
                                time_proporton_on_off_t led_proporton_on_off_t,
                                                 uint32_t need_control_led_idx)
{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;

    //1,检测参数
    if(NULL == led_handler ||
      INIT_HANDLER_LED_NO == led_handler->is_handler_led_init)
    {

#ifdef DEBUG
    DEBUG_OUT("led handler control arg is null");
#endif
    ret = HANDLER_LED_ERRORPARAMETER; 
    return ret;
    }
    //2.配置相应led drive特性
    //2.1 TODO: 检测输入的特性参数是否正确

    //2.2填入数据
    led_handler->led_drive_mount.led_drive_array[need_control_led_idx]\
                                     ->blink_cycle_ms = blink_cycle_ms;

    led_handler->led_drive_mount.led_drive_array[need_control_led_idx]\
                                         ->blink_times = blink_times;

    led_handler->led_drive_mount.led_drive_array[need_control_led_idx]\
                                     ->led_proporton_on_off_t= led_proporton_on_off_t;



    //3.通知任务开始运行,通知具体是哪个led需要控制

    uint32_t test_data = need_control_led_idx;

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


/**
 * @brief 
 *
 * @param 
 */
  __attribute__((used)) static void handler_led_task_code(void *self) 
{
    HANDLER_LED_status_t ret = HANDLER_LED_OK;
    bsp_led_handler_t* handler_self = (bsp_led_handler_t*)self;

    uint32_t test_data = 0;
    while(1)
    {
        
     ret =handler_self->\
        p_os_queue->\
        pf_os_queue_recevie(handler_self->queue_handler,&test_data,0xffffffffUL);
   
     __event_process(handler_self->led_drive_mount.led_drive_array[test_data]);
        
    }
}

/**
 * @brief 初始化 LED handler 并绑定外部接口。
 *
 * 创建消息队列与处理任务，注册挂载和控制接口。
 * @param self LED handler 对象。
 * @param os_delay_ms OS 延时接口。
 * @param os_queue OS 队列接口。
 * @param os_critical OS 临界区接口。
 * @param os_task OS 任务接口。
 * @param get_time_base 时基获取接口。
 *
 * @return HANDLER_LED_OK 成功；HANDLER_LED_ERRORPARAMETER 参数错误；
 *         HANDLER_LED_ERRORRESOURCE 资源创建失败。
 */
__attribute__((used)) HANDLER_LED_status_t led_handler_inst(
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
    
    /* 创建 LED 处理队列 */
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

    /* 创建 LED 处理任务 */

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
