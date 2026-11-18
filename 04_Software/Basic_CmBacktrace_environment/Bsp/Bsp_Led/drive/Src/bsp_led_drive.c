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
 * @file bsp_led_drive.c
 *
 * @par dependencies
 * bsp_led_drive.h
 *
 * @brief 
 * TODO:need ai write docs
 *
 * @author Noser
 * @version 1
 * @date 2026-11-16
 */


//********************************Includes***********************************//
#include "bsp_led_drive.h"
//********************************Includes***********************************//



/**
 * @brief 
 *
 * TODO:need ai write docs
 * @param self
 *
 * TODO:need ai write docs
 * @return 
 * TODO:need ai write docs
 */
static LED_status_t led_driver_init(bsp_led_drive_t *const self)
{
    LED_status_t ret = LED_OK;
    if(NULL == self)
    {

#ifdef DEBUG
        DEBUG_OUT("led driver inst");
#endif
        ret = LED_ERRORPARAMETER;
        return ret;
    }
    self->p_led_ops_inst->pf_led_off();
    self->p_os_ops_api->pf_os_delay_ms(500);

    uint32_t test_time = 0;
    self->pf_get_time_base->pf_get_time_base(&test_time);

    return ret;
}




/**
 * @brief 
 * TODO:need ai write docs
 *
 * @param self
 * TODO:need ai write docs
 * @param blink_cycle_ms
 * TODO:need ai write docs
 * @param blink_times
 * TODO:need ai write docs
 * @param led_proporton_on_off_t
 * TODO:need ai write docs
 *
 * @return 
 * TODO:need ai write docs
 */
static LED_status_t led_control(bsp_led_drive_t* self,
                               uint32_t blink_cycle_ms,
                               uint32_t blink_times,
                               time_proporton_on_off_t led_proporton_on_off_t)
{
    LED_status_t ret = LED_OK;
    /*1.检查self是否为NULL,对象是否已经初始化完毕*/

    if(NULL == self||INITIALIZATION_NO == self->initialization)
    {
#ifdef DEBUG
        DEBUG_OUT("led is null or led is not init");
#endif
        ret = LED_ERRORPARAMETER;
        return ret; 
    }

    /*2.编写led控制逻辑*/
    /*获取led亮多久*/
    uint32_t CMD_GET_SWITCH_TIME = 0;
    if(PROPORTON_1_3 == led_proporton_on_off_t)
    {
        CMD_GET_SWITCH_TIME = blink_cycle_ms * (1/(1+3));
    }
    else if(PROPORTON_1_2 == led_proporton_on_off_t)
    {
        CMD_GET_SWITCH_TIME = blink_cycle_ms * (1/(1+2));
    }
    else if(PROPORTON_1_1 == led_proporton_on_off_t)
    {

        CMD_GET_SWITCH_TIME = blink_cycle_ms * (1/(1+1));
    }
    else
    {

#ifdef DEBUG
        DEBUG_OUT("led is null or led is not init");
#endif
        ret = LED_ERRORPARAMETER;
        return ret; 
     
    }

    for(uint32_t i = 0 ; i < blink_times; i++)
    {
#ifdef DEBUG
        DEBUG_OUT("led start blink");
#endif
 
        self->p_led_ops_inst->pf_led_on();  
        self->p_os_ops_api->pf_os_delay_ms(CMD_GET_SWITCH_TIME);
        self->p_led_ops_inst->pf_led_off();  
        self->p_os_ops_api->pf_os_delay_ms(blink_cycle_ms - CMD_GET_SWITCH_TIME);
    }
    return ret;
}



/**
 * @brief 
 * TODO:need ai write docs
 *
 * @param self
 * TODO:need ai write docs
 * @param led_ops
 * TODO:need ai write docs
 * @param os_ops_api
 * TODO:need ai write docs
 * @param get_time_base
 * TODO:need ai write docs
 *
 * @return 
 * TODO:need ai write docs
 */
LED_status_t led_driver_inst(bsp_led_drive_t *const         self,
                             led_operation_t *const      led_ops,
#ifdef OS_SUPPROT
                             os_api_t        *const   os_ops_api,
#endif
                             get_time_base_t *const get_time_base
)
{
    LED_status_t ret = LED_OK;

#ifdef DEBUG
        DEBUG_OUT("led driver inst start");
#endif

    /*判断空*/
    if(NULL == self      ||
       NULL == led_ops   ||
#ifdef OS_SUPPROT
       NULL == os_ops_api||
#endif
       NULL == get_time_base)
    {

#ifdef DEBUG
        DEBUG_OUT("arg is null");
#endif
       ret = LED_ERRORPARAMETER; 
       return ret;
    }

    /*判断是否已经初始化过了*/
    if(INITIALIZATION_YES == self->initialization)
    {

#ifdef DEBUG
        DEBUG_OUT("Initialization has been completed");
#endif
       ret = LED_ERRORRESOURCE; 
       return ret;
    }

    /*绑定外部提供的接口*/
    self->p_led_ops_inst   = led_ops;
    self->pf_get_time_base = get_time_base;
#ifdef OS_SUPPROT
    self->p_os_ops_api     = os_ops_api;
#endif

    /*绑定提供给外部使用的api*/
    self->pf_led_controler = led_control;

    /*初始化目标*/
    self->blink_times            = 0;
    self->blink_cycle_ms         = 0;
    self->led_proporton_on_off_t = PROPORTON_x_x;

    ret = led_driver_init(self);
    if(LED_OK!=ret )
    {
#ifdef DEBUG
        DEBUG_OUT("Initialization is false");
#endif
 
        self->p_led_ops_inst   = NULL;
        self->pf_get_time_base = NULL;
#ifdef OS_SUPPROT
        self->p_os_ops_api     = NULL;
#endif

        return ret;
    }
#ifdef DEBUG
        DEBUG_OUT("Initialization is successfully");
#endif
 
   self->initialization = INITIALIZATION_YES;
    return ret;
}


