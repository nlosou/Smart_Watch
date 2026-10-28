/**
  ******************************************************************************
	WeAct ΢?д??? 
	>> ??׼?? GPIO
  ******************************************************************************
  */
  
  #include "gpio.h"

#include "key.h"
#include "led.h"



/* Private function prototypes -----------------------------------------------*/
void    stm32_key_init(void* GPIOx,uint16_t GPIO_Pin);
void    stm32_key_delay(uint32_t time);
uint8_t stm32_key_read_bit(void *GPIOx,uint16_t GPIOx_Pin);


void    stm32_led_init(void* GPIOx,uint16_t GPIO_Pin);
void    stm32_led_on(void *GPIOx,uint16_t GPIO_PIN);
void    stm32_led_off(void *GPIOx,uint16_t GPIO_PIN);
void    stm32_led_delay(uint32_t time);
void    stm32_led_toggle(void *GPIox,uint16_t);


/* Private function prototypes -----------------------------------------------*/

/**
 * @brief key??,??key??,????????
 */
key_info_t key_info         = {
    .GPIOx                  = (void*)GPIOA,
    .GPIO_Pin               = GPIO_Pin_0
};

led_info_t led_info = {
    .GPOIx    = (void*)GPIOC,
    .GPIO_Pin = GPIO_Pin_13
};

/**
 * @brief ???key???
 */
key_ops_t key_need_ops      = {
    .key_init               = stm32_key_init,
    .key_read_bit           = stm32_key_read_bit,
    .key_delay              = stm32_key_delay
};
key_need_arg_t key_need_arg = {
    .key_info               = &key_info,
    .key_ops                = &key_need_ops
};

led_ops_t led_need_ops = {
    .led_init   = stm32_led_init,
    .led_on     = stm32_led_on,
    .led_off    = stm32_led_off,
    .led_toggle = stm32_led_toggle,
    .led_delay  = stm32_led_delay
};


led_need_arg_t led_need_arg = {
    .led_ops  = &led_need_ops,
    .led_info = &led_info
};


key_handler_t key_handler;
led_handler_t led_handler;

  
void GPIO_Config(void)
{
  GPIO_InitTypeDef GPIO_InitStructure;
  
   /* Output HSE clock on MCO1 pin(PA8) ****************************************/ 
  /* Enable the GPIOA peripheral */ 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
  
  /* Configure MCO1 pin(PA8) in alternate function */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_8;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
    
  /* HSE clock selected to output on MCO1 pin(PA8)*/
  RCC_MCO1Config(RCC_MCO1Source_HSE, RCC_MCO1Div_1);
  
  /* LED on C13 pin(PC13) ***********************************/ 
  /* Enable the GPIOCperipheral */ 
  RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
  
  /* Configure C13 pin(PC13) in output function */
  GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
  GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
  GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_NOPULL;
  GPIO_Init(GPIOC, &GPIO_InitStructure);
   
}

void stm32_key_init(void* GPIOx,uint16_t GPIO_Pin)
{
    if(NULL == GPIOx)
    {
        return;
    }
    GPIO_InitTypeDef GPIO_INIT;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);

    GPIO_INIT.GPIO_Pin   = GPIO_Pin;
    GPIO_INIT.GPIO_Mode  = GPIO_Mode_IN;
    GPIO_INIT.GPIO_OType = GPIO_OType_PP;
    GPIO_INIT.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_INIT.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_Init((GPIO_TypeDef*)GPIOx,&GPIO_INIT);

}


uint8_t stm32_key_read_bit(void *GPIOx,uint16_t GPIOx_Pin)
{
   if(NULL == GPIOx) 
   {
       return NULL;
   }

   return GPIO_ReadInputDataBit((GPIO_TypeDef*)GPIOx,GPIOx_Pin);
}

void  stm32_key_delay(uint32_t time)
{
    Delay(time);
}

void  stm32_led_init(void* GPIOx,uint16_t GPIO_Pin)
{
    if(NULL == GPIOx)
    {
        return;
    }
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
    GPIO_InitTypeDef GPIO_InitStructure;

    GPIO_InitStructure.GPIO_Pin   = GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;

    GPIO_Init((GPIO_TypeDef*)GPIOx,&GPIO_InitStructure);
}

void    stm32_led_on(void *GPIOx,uint16_t GPIO_PIN)
{
    if(NULL == GPIOx)
    {
        return;
    }
    GPIO_ResetBits((GPIO_TypeDef*)GPIOx,GPIO_PIN);

}

void    stm32_led_off(void *GPIOx,uint16_t GPIO_PIN)
{
    if(NULL == GPIOx)
    {
        return;
    }
    GPIO_SetBits((GPIO_TypeDef*)GPIOx,GPIO_PIN);
}

void    stm32_led_delay(uint32_t time)
{

        Delay(time);
}

void    stm32_led_toggle(void *GPIOx,uint16_t GPIO_PIN)
{
    if(NULL == GPIOx)
    {
        return;
    }
    GPIO_ToggleBits((GPIO_TypeDef*)GPIOx,GPIO_PIN);
}



