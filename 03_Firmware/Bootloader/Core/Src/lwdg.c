/**
 * @file lwdg.c
 * @brief 
 * @author Noser
 * @version 1
 * @date 2026-11-05
 */

/* Includes ------------------------------------------------------------------*/
#include "lwdg.h"
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* extern variables ----------------------------------------------------------*/

void IWDG_Init(uint8_t prer,uint16_t rlr)
{
    IWDG_WriteAccessCmd(IWDG_WriteAccess_Enable);
    IWDG_SetPrescaler(prer);
    IWDG_SetReload(rlr);
    IWDG_ReloadCounter();
    IWDG_Enable();
}
