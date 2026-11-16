/**
 * @file bsp_led_drive.h
 * @par dependencies
 * stdint.h
 *
 * @brief 
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

#define OS_SUPPROT                                         /* TODO:need docs*/
#define DEBUG                                              /* TODO:need docs*/


#define INITIALIZATION_YES 1
#define INITIALIZATION_NO  0

#define DEBUG_OUT(X) SEGGER_RTT_printf(0,"%s\r\n",X)

typedef struct bsp_led_drive bsp_led_drive_t;
typedef enum
{
   PROPORTON_1_3 = 0,                                      /* TODO:need docs*/
   PROPORTON_1_2 = 0,                                      /* TODO:need docs*/
   PROPORTON_1_1 = 0,                                      /* TODO:need docs*/
   PROPORTON_x_x = 0xFF,                                   /* TODO:need docs*/
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
    LED_status_t (*pf_led_on)                  (void);/* TODO:need docs*/
    LED_status_t (*pf_led_off)                 (void);/* TODO:need docs*/
}led_operation_t;


typedef struct {
    LED_status_t (*pf_get_time_base)(uint32_t* const);/* TODO:need docs*/
}get_time_base_t;


#ifdef OS_SUPPROT
typedef struct {
    LED_status_t (*pf_os_delay_ms)   (const uint32_t);/* TODO:need docs*/
}os_api_t;
#endif

typedef struct bsp_led_drive {
    uint8_t                         initialization;
    /* led特性*/
    uint32_t                        blink_cycle_ms;     /* TODO:need docs*/
    uint32_t                           blink_times;     /* TODO:need docs*/
    time_proporton_on_off_t led_proporton_on_off_t;     /* TODO:need docs*/


    /*内部实现提供的接口*/
    /*MCU Core */
    led_operation_t                *p_led_ops_inst;     /* TODO:need docs*/
    get_time_base_t              *pf_get_time_base;     /* TODO:need docs*/

#ifdef OS_SUPPROT
    /*OS Interface*/
    os_api_t                         *p_os_ops_api;     /* TODO:need docs*/
#endif

    /*提供给外部的接口*/
    pf_led_control_t              pf_led_controler;     /* TODO:need docs*/


}bsp_led_drive_t;


/**
 * @brief 
 *
 * @param self
 * @param led_ops
 * @param os_ops_api
 * @param get_time_base
 *
 * @return 
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

