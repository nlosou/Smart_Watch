/******************************************************************************
 * Copyright (C) 2026.Noser
 * 
 * All Rights Reserved.
 * 
 * @file key.c
 * 
 * @par dependencies 
 * 
 * @author Noser
 * 
 * @brief 提供按键api
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.1 2026-09-07
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/

//********************************Includes***********************************//

#ifndef KEY_H__
#define KEY_H__

#include "gpio.h"
#include "cmsis_os2.h"
#include "queue.h"

//********************************Includes***********************************//


//********************************Defines***********************************//


/* 按键状态          */
typedef enum
{
    KEY_IDLE               = 0,
    KEY_PRESSED,
    KEY_HELD,
    KEY_RELEASED
}key_state_t;

typedef enum{
    KEY_OK                = 0,           /* Operation completed successfully.  */
    KEY_ERROR             = 1,           /* Run-time error without case matched*/
    KEY_ERRORTIMEOUT      = 2,           /* Operation failed with timeout      */
    KEY_ERRORRESOURCE     = 3,           /* Resource not available.            */
    KEY_ERRORPARAMETER    = 4,           /* Parameter error.                   */
    KEY_ERRORNOMEMORY     = 5,           /* Out of memory.                     */
    KEY_ERRORISR          = 6,           /* Not allowed in ISR context         */
    KEY_ERROGETINFO       = 7,
    KEY_RESERVED          = 8,  
}key_status_t;

typedef struct{
    GPIO_TypeDef               *KEY_USE_GPIOx;
    uint16_t                      KEY_USE_PIN;
    key_state_t                   g_key_state;
}key_info_t;


extern key_info_t                      g_key1;
extern QueueHandle_t                   key_queue;

//********************************Defines***********************************//

//********************************Declaring***********************************//



/**
 * @brief Initialize a key object with GPIO port and pin information.
 *
 * @details
 * This function stores the GPIO port and pin configuration into the key structure.
 *
 * @param key Pointer to the key structure.
 * @param GPIOx GPIO port used by the key.
 * @param pin GPIO pin number used by the key.
 *
 * @return Status of key initialization.
 */
key_status_t key_init(key_info_t *key,GPIO_TypeDef* GPIOx,uint16_t pin);


/**
 * @brief:
 * scan key is held or not held
 * 
 * @Steps:
 *  
 * @param:
 * 
 * @return key_handler_status_t : Status of the function.
 */ 
key_status_t key_scan(key_info_t *key,
              void (*function_call_back)(void*),
              void*argument
             );



/**
 * @brief:
 * scan key is held or not held
 * 
 * @Steps:
 *  
 * @param:
 * 
 * @return key_handler_status_t : Status of the function.
 */ 
void key_function_callback(void*argument);

/**
 * @brief:
 * scan key is held or not held
 * 
 * @Steps:
 *  
 * @param:
 * 
 * @return key_handler_status_t : Status of the function.
 */ 
void Key_task(void*argument);

//********************************Declaring***********************************//

#endif





