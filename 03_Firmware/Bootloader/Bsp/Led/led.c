#include "led.h"


#define LED_INFO  self->led_need_arg->led_info
#define LED_OPS   self->led_need_arg->led_ops


led_handler_status_t led_breathring(struct led_handler* self)
{
    if(self == NULL)
    {
        return LED_HANDLER_ERROR;
    }
    static uint8_t pwmset;
    static uint16_t time;
    static uint8_t timeflag;
    static uint8_t timecount;

     /* 呼吸灯 */
    if(timeflag == 0)
    {
        time ++;
        if(time >= 1600) timeflag = 1;
    }
    else
    {
        time --;
        if(time == 0) timeflag = 0;
    }

    /* 占空比设置 */
    pwmset = time/80;

    /* 20ms 脉宽 */
    if(timecount > 20) timecount = 0;
    else timecount ++;

    if(timecount >= pwmset ) LED_OPS->led_on(LED_INFO->GPOIx,LED_INFO->GPIO_Pin);
    else LED_OPS->led_off(LED_INFO->GPOIx,LED_INFO->GPIO_Pin);
    
    LED_OPS->led_delay(1);
    return LED_HANDLER_OK;
}

led_handler_status_t led_handler_init(led_handler_t* self,led_need_arg_t* external_arg)
{

    if(NULL == self || NULL == external_arg)
    {
        return LED_HANDLER_ERROR;
    }
    
    self->led_need_arg = external_arg;

    self->led_breathring = led_breathring;
    LED_OPS->led_init(LED_INFO->GPOIx,LED_INFO->GPIO_Pin);
    return LED_HANDLER_OK;
}

