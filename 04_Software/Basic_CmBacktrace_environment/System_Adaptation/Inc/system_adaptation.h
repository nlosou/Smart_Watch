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
 * @file system_adaptation.h
 *
 * @brief 
 * @author Noser
 * @version 1.1
 * @date 2026-11-20
 */

#ifndef   __SYSTEM_ADAPTATION_H_
#define   __SYSTEM_ADAPTATION_H_


//********************************Includes***********************************//
//
/* Standard C Library */
#include "main.h"

/* MCU / CMSIS Library */

/* RTOS Library */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* BSP Dependencies */
#include "bsp_led_drive.h"

/* Module Interface */
#include "Bsp_led_handler.h"

//********************************Includes***********************************//

//*********************************Defines***********************************//
typedef enum
{
    ADAPT_OK               = 0,        /* Operation completed successfully.  */
    ADAPT_ERROR            = 1,        /* Run-time error without case matched*/
    ADAPT_ERRORTIMEOUT     = 2,        /* Operation failed with timeout     */
    ADAPT_ERRORRESOURCE    = 3,        /* Resource not available.           */
    ADAPT_ERRORPARAMETER   = 4,        /* Parameter error.                  */
    ADAPT_ERRORNOMEMORY    = 5,        /* Out of memory.                    */
    ADAPT_ERRORISR         = 6,        /* Not allowed in ISR context        */
    ADAPT_RESERVED         = 0x7FFFFFFF /* Reserved                          */
} system_adapt_status_t;


//*********************************Defines***********************************//



//********************************Variables**********************************//

extern bsp_led_handler_t test_handler;
extern bsp_led_drive_t   led_driver1;

//********************************Variables**********************************//





//********************************Prototypes**********************************//

/* TODO:need ai write docs*/
LED_status_t led_on(void);

/* TODO:need ai write docs*/
LED_status_t led_off(void);


/* TODO:need ai write docs*/
LED_status_t FreeRTOS_delay_ms(const uint32_t time);

/* TODO:need ai write docs*/
LED_status_t get_time_test(uint32_t* const time);


/* TODO:need ai write docs*/
HANDLER_LED_status_t (os_queue_create)(const uint32_t  queue_lenth,
                                          const uint32_t queue_item_size,
                                         void ** const     queue_handler);

HANDLER_LED_status_t os_delay_ms(const uint32_t time);
/* TODO:need ai write docs*/
HANDLER_LED_status_t (os_queue_delete)(void * const queue_handler);

/* TODO:need ai write docs*/
HANDLER_LED_status_t (os_queue_sent)(void * const queue_handler,
                                     void * const         iteme,
                                     uint32_t          time_out);

/* TODO:need ai write docs*/
HANDLER_LED_status_t (os_queue_recevie)(void * const queue_handler,
                                        void * const         iteme,
                                         uint32_t         time_out);

/* TODO:need ai write docs*/
HANDLER_LED_status_t  (os_critical_enter)(void);

/* TODO:need ai write docs*/
HANDLER_LED_status_t  (os_critical_exti)(void);



/* TODO:need ai write docs*/
HANDLER_LED_status_t (handler_get_time_base)(uint32_t* const time);

/* TODO:need ai write docs*/
HANDLER_LED_status_t handler_led_os_task_create(
                                              void * const       task_code,
                                              const char * const task_name,
                                              uint32_t     task_stack_size,
                                          void     * const      parameters,
                                          uint32_t                priority,
                                          void    **const     task_handler);


/* TODO:need ai write docs*/
HANDLER_LED_status_t (os_task_delete)(void * const task_handler);




//********************************Prototypes**********************************//


/**
 * @brief 
 *TODO:need ai write docs*
 * @return 
 *TODO:need ai write docs*
 */
system_adapt_status_t system_adapt_init(void);


#if 1
/**
 * @brief 
 */
void Test1(void);

#endif


#endif /* end of include guard:    __SYSTEM_ADAPTATION_H_ */
