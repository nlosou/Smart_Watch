/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file Ota_task.c
 * 
 * @par dependencies 
 * - Ota_task.h
 * 
 * @author Jack | R&D Dept. | EternalChip 立芯嵌入式
 * 
 * @brief Functions related to reading and writing in the chip's flash area.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2024-09-13
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/
/* Includes ------------------------------------------------------------------*/
#include "SWC_OTA.h"
#include "ymodem.h"
#include "semphr.h"
#include "W25Qxx_handler.h"
#include "AT24Cxx_Driver.h"
/* Private typedef -----------------------------------------------------------*/
osThreadId_t OTA_taskHandle;
const osThreadAttr_t OTA_task_attributes = {
  .name = "OTA_task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};

osThreadId_t download_taskHandle;
const osThreadAttr_t download_task_attributes = {
  .name = "download_app_task",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal1,
};

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern QueueHandle_t Q_YmodemReclength;
extern int32_t packet_length;
QueueHandle_t Q_App_address;
QueueHandle_t Semaphore_ExtFlashState;
uint8_t g_au8_YmodemRec_A[1030];
uint8_t g_au8_YmodemRec_B[1030];
static uint8_t s_ota_cmd[4];
static ota_status_t s_ota_current_status = WaitReqDownload;
/* Private function prototypes -----------------------------------------------*/

/**
 * @brief 
 *
 * @return 
 */
static int8_t key_scan(void);

/**
 * @brief 
 *
 * @param argument
 */
void download_app_task(void*argument);
/* Private functions ---------------------------------------------------------*/
/* extern variables ---------------------------------------------------------*/

void ota_task_runnable(void *argument)
{
  /* USER CODE BEGIN key_task_runnable */

  uint8_t tmep_data = 0;
  uint8_t ReqUpdate_Cmd[] = {0x44,0x55,0x66};
  /* Infinite loop */
  uint8_t at24_ota_status = 0x00;
  int32_t get_app_size = 0;
  ee_WriteBytes(&at24_ota_status,0x00,1);
  for(;;)
  {

      switch (s_ota_current_status)
      {
          case WaitReqDownload :
              //1,阻塞接收数据(接收指令)
              HAL_UARTEx_ReceiveToIdle_DMA(&huart1,s_ota_cmd,4);
              xQueueReceive(Q_YmodemReclength,&tmep_data,portMAX_DELAY);
              if(3 == tmep_data)
              {
                 //2.判断是否是相应命令
                  if(0x11 == s_ota_cmd[0] && 0x22 == s_ota_cmd[1] && 0x33 == s_ota_cmd[2]) 
                  {
                      s_ota_current_status = OtaDownload ;
                      //创建任务
                      download_taskHandle= osThreadNew(
                              download_app_task,
                              NULL,
                              &download_task_attributes);
                      //创建队列
                      Q_App_address= xQueueCreate(1,sizeof(uint16_t));
                      //创建二值信号量
                      Semaphore_ExtFlashState =xSemaphoreCreateMutex();
                      at24_ota_status = 0x11;
                      ee_WriteBytes(&at24_ota_status,0x00,1);
                  }
                  else
                  {
                        s_ota_cmd[0] = 0;
                        s_ota_cmd[1] = 0;
                        s_ota_cmd[2] = 0;
                        s_ota_cmd[3] = 0;
                  }
              }
              else
              {
                        s_ota_cmd[0] = 0;
                        s_ota_cmd[1] = 0;
                        s_ota_cmd[2] = 0;
                        s_ota_cmd[3] = 0;

              }
              
              break;
          case OtaDownload:
              //1,接受app数据
              get_app_size = Ymodem_Receive(g_au8_YmodemRec_A,g_au8_YmodemRec_B);
              if( 0 <get_app_size)
              {
                    s_ota_current_status = WaitReqUpdate;                  
                    HAL_UART_Transmit(&huart1,
                            ReqUpdate_Cmd,
                            sizeof(ReqUpdate_Cmd),
                            0);
                    //接收完数据释放资源
                
                    xSemaphoreTake(Semaphore_ExtFlashState,portMAX_DELAY);
                    xSemaphoreGive(Semaphore_ExtFlashState);

                    W25Q64_WriteData_End();

                    vTaskDelete(download_taskHandle);
                    vQueueDelete(Q_App_address);
                    vSemaphoreDelete(Semaphore_ExtFlashState);

                    at24_ota_status = 0x22;
                    ee_WriteBytes(&at24_ota_status,0x00,1);

										
                    ee_ReadBytes(&at24_ota_status,0x00,1);

                    ee_WriteBytes((uint8_t*)&get_app_size,0x01,4);
                    uint8_t temp_get_at24[4] = {0};
                    ee_ReadBytes(temp_get_at24,0x01,4);
                    HAL_UART_Transmit(&huart1,temp_get_at24,4,1000);
              }
              else
              {
                  s_ota_current_status = WaitReqDownload;
                    s_ota_cmd[0] = 0;
                    s_ota_cmd[1] = 0;
                    s_ota_cmd[2] = 0;
                    s_ota_cmd[3] = 0;
                    //接收完数据释放资源
                    vTaskDelete(download_taskHandle);
                    vQueueDelete(Q_App_address);
                    vSemaphoreDelete(Semaphore_ExtFlashState);
              }
              break;
          case WaitReqUpdate:
              //1,阻塞接收数据(接收指令)
              
              HAL_UARTEx_ReceiveToIdle_DMA(&huart1,s_ota_cmd,4);
              xQueueReceive(Q_YmodemReclength,&tmep_data,portMAX_DELAY);
              if(3 == tmep_data)
              {
                  //2.判断是否是相应命令
                  if(0x77 == s_ota_cmd[0] && 0x88 == s_ota_cmd[1] && 0x99 == s_ota_cmd[2]) 
                  {
                      s_ota_current_status = OtaEnd;
                  }
                  else
                  {
                        s_ota_cmd[0] = 0;
                        s_ota_cmd[1] = 0;
                        s_ota_cmd[2] = 0;
                        s_ota_cmd[3] = 0;
                  }
              }
              else
              {
                        s_ota_cmd[0] = 0;
                        s_ota_cmd[1] = 0;
                        s_ota_cmd[2] = 0;
                        s_ota_cmd[3] = 0;
              }
              
              break;
          case OtaEnd:
              if( 1 == key_scan())
              {
                  //进入软复位
                  HAL_NVIC_SystemReset();
              }
              else
              {
                  //阻塞
                  s_ota_current_status = WaitReqDownload;
              }
              break;
          default:
              break;
              
      }
  }
  /* USER CODE END key_task_runnable */
}

/**
 * @brief 检测key在20s内是否按下
 *
 * @return 
 * 1:返回1表示按下
 *-1:表示没有按下
 */
static int8_t key_scan(void)
{
    for(uint16_t i = 0 ; i < 400 ; i++)
    {
        if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(Key_GPIO_Port ,Key_Pin))
        {
            osDelay(20);
            if(GPIO_PIN_RESET == HAL_GPIO_ReadPin(Key_GPIO_Port ,Key_Pin))
            {
                return 1;
            }   
        }
        osDelay(50);

    }
    return -1;
}


void download_app_task(void*argument)
{
  uint8_t * pu8_data = NULL;
  int32_t * pu32_size = NULL;
  //第一帧数据一定是数据的长度
  xQueueReceive(Q_App_address,&pu32_size,portMAX_DELAY);
  //需要擦除掉flash size对应的所有扇区
  //TODO:
  xSemaphoreGive(Semaphore_ExtFlashState);
  while(1)
  {
    xQueueReceive(Q_App_address,&pu8_data,portMAX_DELAY);
    xSemaphoreTake(Semaphore_ExtFlashState,0);
    if(pu8_data == NULL)
    {
      continue;
    }
    //执行写入W25q的逻辑
    W25Q64_WriteData(pu8_data + 0x20000000,(uint32_t)packet_length);
    xSemaphoreGive(Semaphore_ExtFlashState);
  }
}
