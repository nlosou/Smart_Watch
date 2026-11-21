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
 * @file bsp_led_drive.h
 *
 * @par dependencies
 * stdint.h
 *
 * @brief LED 驱动模块接口。
 * 定义 LED 驱动对象、操作接口、占空比枚举与实例化接口。
 *
 * @author Noser
 * @version 1
 * @date 2026-11-16
 */

#ifndef __BSP_LED_DRIVE_H__
#define __BSP_LED_DRIVE_H__

//********************************Includes***********************************//
#include <stdint.h>
#include <stddef.h>
#include "SEGGER_RTT.h"
//********************************Includes***********************************//


//*********************************Defines***********************************//

#define OS_SUPPROT                                         /* 启用 OS 接口 */
#define DEBUG                                              /* 启用调试日志 */


#define INITIALIZATION_YES 1                               /* 已初始化 */
#define INITIALIZATION_NO  0                               /* 未初始化 */

/* 通过 SEGGER RTT 输出调试字符串 */
#define DEBUG_OUT(X) SEGGER_RTT_printf(0,"%s\r\n",X)


typedef struct bsp_led_drive bsp_led_drive_t;
typedef enum
{
   /* 亮灭时间比 1:3 */
   PROPORTON_1_3 = 0,
   /* 亮灭时间比 1:2 */
   PROPORTON_1_2 = 1,
   /* 亮灭时间比 1:1 */
   PROPORTON_1_1 = 2,
   /* 未配置占空比 */
   PROPORTON_x_x = 0xFF,
}time_proporton_on_off_t;

typedef enum
{
    LED_OK               = 0,        /* Operation completed successfully.  */
    LED_ERROR            = 1,        /* Run-time error without case matched*/
    LED_ERRORTIMEOUT     = 2,        /* Operation failed with timeout     */
    LED_ERRORRESOURCE    = 3,        /* Resource not available.           */
    LED_ERRORPARAMETER   = 4,        /* Parameter error.                  */
    LED_ERRORNOMEMORY    = 5,        /* Out of memory.                    */
    LED_ERRORISR         = 6,        /* Not allowed in ISR context        */
    LED_RESERVED         = 0x7FFFFFFF /* Reserved                          */
} LED_status_t;
//*********************************Defines***********************************//




//*********************************Declaring***********************************//

typedef LED_status_t (*pf_led_control_t)(bsp_led_drive_t* self,
                                         uint32_t,
                                         uint32_t,
                                         time_proporton_on_off_t);

/*led自身特性*/
typedef struct {
    /* 点亮 LED 的回调 */
    LED_status_t (*pf_led_on)                  (void);
    /* 熄灭 LED 的回调 */
    LED_status_t (*pf_led_off)                 (void);
}led_operation_t;


typedef struct {
    /* 获取时基的回调 */
    LED_status_t (*pf_get_time_base)(uint32_t* const);
}get_time_base_t;


#ifdef OS_SUPPROT
typedef struct {
    /* OS 延时接口 */
    LED_status_t (*pf_os_delay_ms)   (const uint32_t);
}os_api_t;
#endif

typedef struct bsp_led_drive {
    uint8_t                         initialization;
    /* led特性*/
    /* 闪烁周期，单位 ms */
    uint32_t                        blink_cycle_ms;
    /* 闪烁次数 */
    uint32_t                           blink_times;
    /* 亮灭时间占空比 */
    time_proporton_on_off_t led_proporton_on_off_t;


    /*内部实现提供的接口*/
    /*MCU Core */
    /* LED 操作接口 */
    led_operation_t                *p_led_ops_inst;
    /* 时基获取接口 */
    get_time_base_t              *pf_get_time_base;

#ifdef OS_SUPPROT
    /*OS Interface*/
    /* OS 延时接口 */
    os_api_t                         *p_os_ops_api;
#endif



}bsp_led_drive_t;


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
);

//*********************************Declaring***********************************//




#endif
