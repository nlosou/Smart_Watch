/**
  ******************************************************************************
	WeAct 微行创新 
	>> 标准库 TIM
  ******************************************************************************
  */

#include "tim.h"
/**
  * @brief  Configures the TIM IRQ Handler.
  * @param  None
  * @retval None
  */
void TIM_Config(void)
{
  NVIC_InitTypeDef NVIC_InitStructure;
  TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
  uint16_t PrescalerValue = 0;
  /* TIM3 clock enable */
  RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

  /* Enable the TIM3 global Interrupt */
  NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
  NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
  NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
  NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
  NVIC_Init(&NVIC_InitStructure);

  /* Compute the prescaler value */
  PrescalerValue = (uint16_t) ((SystemCoreClock / 2) / 1000000) - 1; // 1Mhz

  /* Time base configuration */
  TIM_TimeBaseStructure.TIM_Period = 100;  // 0.1ms
  TIM_TimeBaseStructure.TIM_Prescaler = 0;
  TIM_TimeBaseStructure.TIM_ClockDivision = 0;
  TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
  
  TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);
  
  /* Prescaler configuration */
  TIM_PrescalerConfig(TIM3, PrescalerValue, TIM_PSCReloadMode_Immediate);
  
  /* TIM Interrupts enable */
  TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
  
  /* TIM3 enable counter */
  TIM_Cmd(TIM3, ENABLE); 
}


void TIM3_IT_Update_Callback()
{

}

