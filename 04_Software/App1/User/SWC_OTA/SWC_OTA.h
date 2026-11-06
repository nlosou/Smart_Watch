/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file Ota_task.h
 * 
 * @par dependencies 
 * - Ota_task.h
 * 
 * @author Jack | R&D Dept. | EternalChip 立芯嵌入式
 * 
 * @brief Execute the Ota upgrade process
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __SWC_OTA_H
#define __SWC_OTA_H
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "Freertos.h"
#include "task.h"
#include "cmsis_os.h"
/* Exported types ------------------------------------------------------------*/

typedef enum {
    WaitReqDownload,
    OtaDownload,
    WaitReqUpdate,
    OtaEnd 
}ota_status_t;


/* Exported variable --------------------------------------------------------*/

/* Exported constants --------------------------------------------------------*/
/* Exported macro ------------------------------------------------------------*/
#define BUFFER_SIZE                     1030
/* Exported functions ------------------------------------------------------- */
extern void ota_task_runnable(void *argument);
#endif
