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
 * @brief 
 * TODO:need ai write docs
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

#define OS_SUPPROT                                         /* TODO:need ai write docs*/
#define DEBUG                                              /* TODO:need ai write docs*/


#define INITIALIZATION_YES 1                               /* TODO:need ai write docs*/

#define INITIALIZATION_NO  0                               /* TODO:need ai write docs*/

#define DEBUG_OUT(X) SEGGER_RTT_printf(0,"%s\r\n",X)       /* TODO:need ai write docs*/


typedef struct bsp_led_drive bsp_led_drive_t;
typedef enum
{
   PROPORTON_1_3 = 0,                                      /* TODO:need ai write docs*/
   PROPORTON_1_2 = 1,                                      /* TODO:need ai write docs*/
   PROPORTON_1_1 = 2,                                      /* TODO:need ai write docs*/
   PROPORTON_x_x = 0xFF,                                   /* TODO:need ai write docs*/
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
    LED_status_t (*pf_led_on)                  (void);/* TODO:need ai write docs*/
    LED_status_t (*pf_led_off)                 (void);/* TODO:need ai write docs*/
}led_operation_t;


typedef struct {
    LED_status_t (*pf_get_time_base)(uint32_t* const);/* TODO:need ai write docs*/
}get_time_base_t;


#ifdef OS_SUPPROT
typedef struct {
    LED_status_t (*pf_os_delay_ms)   (const uint32_t);/* TODO:need ai write docs*/
}os_api_t;
#endif

typedef struct bsp_led_drive {
    uint8_t                         initialization;
    /* led特性*/
    uint32_t                        blink_cycle_ms;     /* TODO:need ai write docs*/
    uint32_t                           blink_times;     /* TODO:need ai write docs*/
    time_proporton_on_off_t led_proporton_on_off_t;     /* TODO:need ai write docs*/


    /*内部实现提供的接口*/
    /*MCU Core */
    led_operation_t                *p_led_ops_inst;     /* TODO:need ai write docs*/
    get_time_base_t              *pf_get_time_base;     /* TODO:need ai write docs*/

#ifdef OS_SUPPROT
    /*OS Interface*/
    os_api_t                         *p_os_ops_api;     /* TODO:need ai write docs*/
#endif



}bsp_led_drive_t;


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
);

//*********************************Declaring***********************************//




#endif

