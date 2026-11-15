#ifndef LED_H_YLJYMEDX
#define LED_H_YLJYMEDX


#include <stdint.h>
#include <stddef.h>



typedef enum {
    LED_HANDLER_ERROR,
    LED_HANDLER_OK,
}led_handler_status_t;

typedef struct {
    void (*led_init)(void *GPIOx,uint16_t GPIO_PIN);
    void (*led_on)(void *GPIOx,uint16_t GPIO_PIN);
    void (*led_off)(void *GPIOx,uint16_t GPIO_PIN);
    void (*led_toggle)(void *GPIox,uint16_t);
    void (*led_delay)(uint32_t time);
}led_ops_t;

typedef struct {
   void* GPOIx;
   uint16_t GPIO_Pin;
}led_info_t;


typedef struct {
    led_info_t* led_info;
    led_ops_t*  led_ops;
}led_need_arg_t;


typedef struct led_handler {
    led_need_arg_t* led_need_arg;

   led_handler_status_t (*led_breathring)(struct led_handler* self);
}led_handler_t;


/**
 * @brief 
 * 初始化led模块
 * @param self 待初始化的句柄
 * @param external_arg 外部依赖
 *
 * @return   LED_HANDLER_ERROR
             LED_HANDLER_OK
 */
led_handler_status_t led_handler_init(led_handler_t* self,led_need_arg_t* external_arg);


#endif /* end of include guard: LED_H_YLJYMEDX */
