#ifndef BOOT_MANAGER_H_NOPD8GAE
#define BOOT_MANAGER_H_NOPD8GAE


#include "main.h"
#include <stdint.h>

#define APP_FLASH_ADDR     0x08008000
#define BACK_FLASH_ADDR    0x08044000

#define    NO_APP_NEED_TO_UPDATE   0x00
#define    APP_DOWNLOADING         0x11
#define    APP_REQ_UPDATE          0x22
#define    DOWNLOAD                0xAA


typedef void (*pFunction)(void);
/**
 * @brief 跳转到app
 */
void jump_to_app(void);


/**
 * @brief 备份区代码保存到运行区
 */
int8_t back_to_app(int32_t app_size);


void Ota_SateManger(void);
#endif /* end of include guard: BOOT_MANAGER_H_NOPD8GAE */
