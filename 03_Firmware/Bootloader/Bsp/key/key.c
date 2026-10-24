#include "key.h" 




#define KEY_INFO  self->need_arg->key_info
#define KEY_OPS   self->need_arg->key_ops



key_handler_status_t key_scan(struct key_handler *self)
{

    if(NULL == self)
    {
        return KEY_HANDLER_ERROR;
    }

   if(KEY_LOW == KEY_OPS->key_read_bit(KEY_INFO->GPIOx,KEY_INFO->GPIO_Pin))     
   {
        KEY_OPS->key_delay(30);        
        if(KEY_LOW == KEY_OPS->key_read_bit(KEY_INFO->GPIOx,KEY_INFO->GPIO_Pin))     
        {
            return KEY_HANDLER_OK;
        }
   }
   return KEY_HANDLER_ERROR;
}
    




key_handler_status_t key_handler_init(key_handler_t* self,key_need_arg_t* external_arg)
{
    if(NULL == self || NULL == external_arg)
    {
        return KEY_HANDLER_ERROR;
    }

    self->need_arg = external_arg;

    self->key_scan = key_scan;

    self->need_arg->key_ops->key_init(KEY_INFO->GPIOx,KEY_INFO->GPIO_Pin);
    return KEY_HANDLER_OK;
}

