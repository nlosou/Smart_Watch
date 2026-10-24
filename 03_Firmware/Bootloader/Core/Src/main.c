/**
  ******************************************************************************
	WeAct 微行创新 
	>> 标准库实例例程
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "SEGGER_RTT.h"
#include "elog.h"
#include "boot_manager.h"
#include "key.h"
#include "led.h"
// 全局定义 STM32F411xE 或者 STM32F401xx
// 当前定义 STM32F411xE

// STM32F411 外部晶振25Mhz，考虑到USB使用，内部频率设置为96Mhz
// 需要100mhz,自行修改system_stm32f4xx.c

/** @addtogroup Template_Project
  * @{
  */ 

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/


//App addresss
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/



static __IO uint32_t uwTimingDelay;
RCC_ClocksTypeDef RCC_Clocks;

extern key_handler_t  key_handler;
extern key_need_arg_t key_need_arg;

extern led_handler_t  led_handler;
extern led_need_arg_t led_need_arg;

/* Private function prototypes -----------------------------------------------*/

void app_elog_init(void);
void jump_to_app(void);

/* Private functions ---------------------------------------------------------*/

 
 /*
  *power by WeAct Studio
  *The board with `WeAct` Logo && `version number` is our board, quality guarantee. 
  *For more information please visit: https://github.com/WeActTC/MiniF4-STM32F4x1
  *更多信息请访问：https://gitee.com/WeActTC/MiniF4-STM32F4x1
  */
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
	/* Enable Clock Security System(CSS): this will generate an NMI exception
     when HSE clock fails *****************************************************/
  RCC_ClockSecuritySystemCmd(ENABLE);
	SEGGER_RTT_Init();
    app_elog_init();
 /*!< At this stage the microcontroller clock setting is already configured, 
       this is done through SystemInit() function which is called from startup
       files before to branch to application main.
       To reconfigure the default setting of SystemInit() function, 
       refer to system_stm32f4xx.c file */

  /* SysTick end of count event each 1ms */
  SystemCoreClockUpdate();
  RCC_GetClocksFreq(&RCC_Clocks);
  SysTick_Config(RCC_Clocks.HCLK_Frequency / 1000);
  

  /* Add your application code here */
  /* Insert 50 ms delay */
  elog_i("INFO","Bootloader running.......");

  key_handler_init(&key_handler,&key_need_arg);
  led_handler_init(&led_handler,&led_need_arg);
  TIM_Config();   
  //jump_to_app();
  /* Infinite loop */
  uint32_t flag = 0;
  while (1)
  {
      if(KEY_HANDLER_OK == key_handler.key_scan(&key_handler))
      {
            elog_i("INFO", "key press");            
            flag = 0;
      }
      if(flag == 1)
      {
          led_handler.led_breathring(&led_handler);
         //led_handler.led_need_arg->led_ops->led_toggle(led_handler.led_need_arg->led_info->GPOIx,
          //       led_handler.led_need_arg->led_info->GPIO_Pin
           //      );
      }
	}
}

/**
  * @brief  Inserts a delay time.
  * @param  nTime: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay(__IO uint32_t nTime)
{ 
  uwTimingDelay = nTime;

  while(uwTimingDelay != 0);
}

/**
  * @brief  Decrements the TimingDelay variable.
  * @param  None
  * @retval None
  */
void TimingDelay_Decrement(void)
{
  if (uwTimingDelay != 0x00)
  { 
    uwTimingDelay--;
  }
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{ 
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

void app_elog_init(void)
{
    elog_init(); 
    elog_set_text_color_enabled(true);
    elog_set_fmt(ELOG_LVL_ASSERT,ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR,ELOG_FMT_LVL |ELOG_FMT_TAG );
    elog_set_fmt(ELOG_LVL_WARN,ELOG_FMT_LVL |ELOG_FMT_TAG );
    elog_set_fmt(ELOG_LVL_INFO,ELOG_FMT_LVL |ELOG_FMT_TAG );
    elog_set_fmt(ELOG_LVL_DEBUG,ELOG_FMT_ALL & ~(ELOG_FMT_TIME | ELOG_FMT_P_INFO));
    elog_set_fmt(ELOG_LVL_VERBOSE,ELOG_FMT_ALL);
    elog_start();
}

/**
  * @}
  */


/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
