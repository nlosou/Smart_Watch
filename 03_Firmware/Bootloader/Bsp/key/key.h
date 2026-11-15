#ifndef KEY_H_0XLACLSO
#define KEY_H_0XLACLSO



#include <stdint.h>
#include <stddef.h>





typedef enum {
    KEY_LOW  = 0,
    KEY_HIGH = 1,
}key_level;



/**
 * @brief 函数返回状态
 */
typedef enum {
    KEY_HANDLER_ERROR,
    KEY_HANDLER_OK,
}key_handler_status_t;


typedef struct {
    void (*key_init)(void* GPIOx,uint16_t GPIO_Pin);
    uint8_t (*key_read_bit)(void *GPIOx,uint16_t GPIOx_Pin);
    void  (*key_delay)(uint32_t time);
}key_ops_t;


typedef struct {
   void* GPIOx; 
   uint16_t GPIO_Pin;
}key_info_t;


typedef struct {
    key_info_t* key_info;
    key_ops_t* key_ops;
}key_need_arg_t;



typedef struct key_handler {
    key_need_arg_t* need_arg;


    key_handler_status_t (*key_scan)(struct key_handler *self);
}key_handler_t;



/**
 * @brief 
 * 初始化key模块
 * @param self 待初始化的句柄
 * @param external_arg 外部依赖
 *
 * @return  KEY_HANDLER_OK 成功
 *          KEY_HANDLER_ERROR 失败
 */
key_handler_status_t key_handler_init(key_handler_t* self,key_need_arg_t* external_arg);






#endif /* end of include guard: KEY_H_0XLACLSO */
