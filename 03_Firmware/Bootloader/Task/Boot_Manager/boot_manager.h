#ifndef BOOT_MANAGER_H_NOPD8GAE
#define BOOT_MANAGER_H_NOPD8GAE


#include "main.h"
#include <stdint.h>

#define APP_FLASH_ADDR     0x08008000
#define BACK_FLASH_ADDR    0x08044000

typedef void (*pFunction)(void);
/**
 * @brief 跳转到app
 */
void jump_to_app(void);


/**
 * @brief 备份区代码保存到运行区
 */
int8_t back_to_app(int32_t app_size);


#endif /* end of include guard: BOOT_MANAGER_H_NOPD8GAE */
