/**
 * @file bsp_led_drive.c
 * @par dependencies
 * bsp_led_drive.h
 * @brief 
 * @author Noser
 * @version 1
 * @date 2026-11-16
 */


//********************************Includes***********************************//
#include "bsp_led_drive.h"
//********************************Includes***********************************//




LED_status_t led_driver_init(bsp_led_drive_t *const self)
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
       NULL == os_ops_api||
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


