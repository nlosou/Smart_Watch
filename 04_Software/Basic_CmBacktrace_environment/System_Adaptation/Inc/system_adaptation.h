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


//********************************Variables**********************************//





//********************************Prototypes**********************************//



//********************************Prototypes**********************************//


/**
 * @brief 初始化系统适配层。
 *
 * 依次实例化 LED 驱动与 LED handler。
 * @return ADAPT_OK 成功；ADAPT_ERRORRESOURCE 初始化失败。
 */
system_adapt_status_t system_adapt_init(void);

/**
 * @brief 
 *TODO:need ai to Annotation
 */
void System_Adaptation_runtime_init(void);

#if 1
/**
 * @brief 
 *TODO:need ai to Annotation
 */
void Test1(void);

#endif


#endif /* end of include guard:    __SYSTEM_ADAPTATION_H_ */
