/******************************************************************************
 * Copyright (C) 2026.Noser
 * 
 * All Rights Reserved.
 * 
 * @file led.h
 * 
 * @par dependencies 
 * 
 * @author Noser
 * 
 * @brief 提供led的api
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

#ifndef LED_H__
#define LED_H__

#include "gpio.h"
#include "cmsis_os2.h"
#include "queue.h"

//********************************Includes***********************************//


//********************************Defines***********************************//

typedef struct{
    GPIO_TypeDef *LED_USE_GPIOx;
    uint16_t LED_USE_PIN;
}led_info_t;

//函数返回
typedef enum{
    LED_OK                = 0,           /* Operation completed successfully.  */
    LED_ERROR             = 1,           /* Run-time error without case matched*/
    LED_ERRORTIMEOUT      = 2,           /* Operation failed with timeout      */
    LED_ERRORRESOURCE     = 3,           /* Resource not available.            */
    LED_ERRORPARAMETER    = 4,           /* Parameter error.                   */
    LED_ERRORNOMEMORY     = 5,           /* Out of memory.                     */
    LED_ERRORISR          = 6,           /* Not allowed in ISR context         */
    LED_ERROGETINFO       = 7,
    LED_RESERVED          = 8,  
}led_status_t;


typedef enum{
    LED_ON,
    LED_OFF,
    LED_TOGGLE,
    LED_BLINK_3,
    LED_BLINK_10,
}led_function_t;


extern led_info_t                      g_led1;
extern QueueHandle_t                led_queue;
extern volatile uint8_t             pwm_times;
//********************************Defines***********************************//

//********************************Declaring***********************************//

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
led_status_t led_init(led_info_t *led,GPIO_TypeDef *GPIOx,uint16_t led_use_pin);


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
led_status_t led_toggle(led_info_t *led);



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
void led_toggle_task(void*argument);

/**
  * @brief  led to tim2 callback
  * @note   
  * @param  void
  * @retval None
  */
void led_tim_Callback(void);

//********************************Declaring***********************************//

#endif





