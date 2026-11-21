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
 * @brief LED 驱动模块实现。
 * 提供 LED 驱动对象初始化与外部接口绑定。
 *
 * @author Noser
 * @version 1
 * @date 2026-11-16
 */


//********************************Includes***********************************//
#include "bsp_led_drive.h"
//********************************Includes***********************************//



/**
 * @brief 初始化 LED 驱动对象。
 *
 * 关闭 LED 并做一次短暂延时，同时读取时基作为初始化自检。
 * @param self LED 驱动对象指针。
 *
 * @return LED_OK 成功；LED_ERRORPARAMETER 参数错误。
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
 * @brief 实例化 LED 驱动。
 *
 * 校验参数，绑定 LED 操作、OS 和时基接口，并完成内部状态初始化。
 * @param self LED 驱动对象。
 * @param led_ops LED 亮灭操作接口。
 * @param os_ops_api OS 延时接口。
 * @param get_time_base 时基获取接口。
 *
 * @return LED_OK 成功；LED_ERRORPARAMETER 参数错误；
 *         LED_ERRORRESOURCE 重复初始化。
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


