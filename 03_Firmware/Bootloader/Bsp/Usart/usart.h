#ifndef USART_H_SVGLECWT
#define USART_H_SVGLECWT

#include <stdint.h>
#include <stddef.h>
#include "stm32f4xx_usart.h"


/*
typedef enum {
    USART_OK,
    USART_ERROR,
}usart_status_t;



typedef struct {
    void (*usart_init)(void* USARTx,void* USARTx_Init);
    void (*usart_senddata)(void* USARTx,uint16_t data);
    uint16_t (*usart_receivedata)(void* USARTx);
}usart_ops_t;


typedef struct {
    uint32_t usart_baudrate;
    uint16_t usart_wordlength;
    uint16_t usart_stopbits;
    uint16_t usart_use_rx_pin;
    uint16_t usart_use_tx_pin;
}usart_info_t;


typedef struct {
    usart_ops_t* usart_ops;
    usart_info_t* usart_info;
}usart_need_arg_t;

typedef struct {
    usart_need_arg_t *need_arg;

}usart_handler_t;



usart_status_t usart_handler_init(usart_handler_t *self,usart_need_arg_t* external_arg);
*/


extern void USART1_Init(void);
extern void USART_SendChar(USART_TypeDef* USART,uint8_t data);
extern uint8_t USART_ReceiveChar(USART_TypeDef *USART);


#endif /* end of include guard: USART_H_SVGLECWT */                                                  
