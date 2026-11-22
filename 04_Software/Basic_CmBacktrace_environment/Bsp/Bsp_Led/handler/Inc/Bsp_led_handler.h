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
 * @file Bsp_led_handler.h
 *
 *  @par dependencies
 * stdint.h
 * stddef.h
 * bsp_led_drive.h
 *
 * @brief LED handler 模块接口。
 * 定义 LED handler 对象、接口类型与初始化接口。
 * @author Noser
 * @version 1.1
 * @date 2026-11-17
 */

#ifndef __BSP_LED_HANDLER_H__
#define __BSP_LED_HANDLER_H__


//********************************Includes***********************************//
#include <stdint.h>
#include <stddef.h>
#include "bsp_led_drive.h"
//********************************Includes***********************************//

//*********************************Defines***********************************//

#define OS_SUPPROT                                 /* 启用 OS 适配接口 */
#define DEBUG                                      /* 启用调试日志 */

/* 通过 SEGGER RTT 输出调试字符串 */
#define DEBUG_OUT(X) SEGGER_RTT_printf(0,"%s\r\n",X)

#define INIT_HANDLER_LED_YES 1                     /* 已初始化 */
#define INIT_HANDLER_LED_NO  0                     /* 未初始化 */
#define INIT_PATERN 0xA5A5A5A5                     /* 初始化模式标记 */

typedef enum
{
    HANDLER_LED_OK               = 0,        /* Operation completed successfully.  */
    HANDLER_LED_ERROR            = 1,        /* Run-time error without case matched*/
    HANDLER_LED_ERRORTIMEOUT     = 2,        /* Operation failed with timeout     */
    HANDLER_LED_ERRORRESOURCE    = 3,        /* Resource not available.           */
    HANDLER_LED_ERRORPARAMETER   = 4,        /* Parameter error.                  */
    HANDLER_LED_ERRORNOMEMORY    = 5,        /* Out of memory.                    */
    HANDLER_LED_ERRORISR         = 6,        /* Not allowed in ISR context        */
    HANDLER_LED_RESERVED         = 0x7FFFFFFF /* Reserved                          */
} HANDLER_LED_status_t;


typedef enum
{
    LED_1 = 0,
    LED_2,
    LED_3,
    LED_4,
    LED_5,
    LED_6,
    LED_7,
    LED_8,
    LED_9,
    LED_10,
    MAX_LED_X,
    LED_INIT_IDX  = 0xFF
}led_x;


//*********************************Defines***********************************//

//*********************************Declaring*********************************//
typedef struct bsp_led_handler bsp_led_handler_t;

typedef HANDLER_LED_status_t (*pf_handler_led_control_t)(
                          bsp_led_handler_t* led_handler,
                                 uint32_t blink_cycle_ms,
                                    uint32_t blink_times,
          time_proporton_on_off_t led_proporton_on_off_t,
                          uint32_t need_control_led_idx);

typedef HANDLER_LED_status_t (*pf_handler_led_mount_t)(
                              bsp_led_drive_t  *  led_driver,
                              bsp_led_handler_t* led_handler,
                                     uint32_t*      led_idx);



typedef struct {
    /* 获取时基的回调 */
    HANDLER_LED_status_t (*pf_get_time_base)(uint32_t* const);
}handler_led_get_time_base_t;


#ifdef OS_SUPPROT

/**
 * @brief os接口,需要绑定对应os api
 */
typedef struct {

    /* OS 延时接口 */
    HANDLER_LED_status_t (*pf_os_delay_ms)   (const uint32_t);

}handler_led_os_delay_t;


typedef struct {

    /**
     * @brief 创建消息队列。
     * @param queue_lenth 队列长度。
     * @param queue_item_size 单个队列元素大小。
     * @param queue_handler 输出队列句柄。
     */
    HANDLER_LED_status_t (*pf_os_queue_create)(const uint32_t  queue_lenth,
                                            const uint32_t queue_item_size,
                                          void ** const     queue_handler);

    /**
     * @brief 删除消息队列。
     * @param queue_handler 待删除的队列句柄。
     */
    HANDLER_LED_status_t (*pf_os_queue_delete)(void * const queue_handler);

    /**
     * @brief 向队列发送数据。
     * @param queue_handler 队列句柄。
     * @param iteme 待发送数据指针。
     * @param time_out 等待超时时间。
     */
    HANDLER_LED_status_t (*pf_os_queue_sent)(void *    const queue_handler,
                                             void *    const         iteme,
                                             uint32_t            time_out);

    /**
     * @brief 从队列接收数据。
     * @param queue_handler 队列句柄。
     * @param iteme 接收数据指针。
     * @param time_out 等待超时时间。
     */
    HANDLER_LED_status_t (*pf_os_queue_recevie)(void * const queue_handler,
                                                void * const         iteme,
                                                uint32_t         time_out);

}handler_led_os_queue_t;

typedef struct {

    /* 进入临界区 */
    HANDLER_LED_status_t  (*pf_os_critical_enter)(void);

    /* 退出临界区 */
    HANDLER_LED_status_t  (*pf_os_critical_exti)(void);

}handler_led_os_critical_t;

typedef struct {

    HANDLER_LED_status_t (*pf_os_task_create)(void * const       task_code,
                                              const char * const task_name,
                                              uint32_t     task_stack_size,
                                          void     * const      parameters,
                                          uint32_t                priority,
                                          void    ** const   task_handler);

    HANDLER_LED_status_t (*pf_os_task_delete)(void * const task_handler);


}handler_led_os_task_t;


#endif

typedef struct {

    /* 当前已挂载 LED 驱动数量 */
    uint32_t                            led_index;

    bsp_led_drive_t    *led_drive_array[MAX_LED_X];     /* LED 驱动对象数组 */

}led_driver_array_t;

typedef struct bsp_led_handler {
    uint8_t                   is_handler_led_init;           /* 标记 handler 是
                                                                  否已初始化 */

		
    led_driver_array_t            led_drive_mount;       /* LED 驱动对象数组 */
    void*                           queue_handler;
    void*                           task_handler;

    /* 内部实现提供的接口 */
    /* MCU Core */
    handler_led_get_time_base_t *pf_get_time_base;         /* 获取时基的回调 */

#ifdef OS_SUPPROT
    /* OS Interface */
    handler_led_os_delay_t           *p_os_delay;             /* OS 相关 API */
    handler_led_os_queue_t           *p_os_queue;
    handler_led_os_critical_t     *p_os_critical;
    handler_led_os_task_t             *p_os_task;
#endif

    /* 提供给外部的接口 
     * App层通过bsp_led_handler_t类型的变量调用
     * */
    /* 提供给 App 层，操作 LED 的接口 */
    pf_handler_led_control_t     pf_led_control;           /* LED 控制接口 */

    /* 提供给 App 层，挂载 LED 的接口 */
    pf_handler_led_mount_t         pf_led_mount;           /* LED 挂载接口 */

} bsp_led_handler_t;


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
HANDLER_LED_status_t led_handler_inst(
        bsp_led_handler_t          *const            self,
#ifdef OS_SUPPROT
        handler_led_os_delay_t     *const     os_delay_ms,
        handler_led_os_queue_t     *const        os_queue,
        handler_led_os_critical_t  *const     os_critical,
        handler_led_os_task_t      *const         os_task,
#endif
        handler_led_get_time_base_t*const  get_time_base);


				


//*********************************Declaring***********************************//


#endif /* end of include guard: __BSP_LED_HANDLER_H__ */
