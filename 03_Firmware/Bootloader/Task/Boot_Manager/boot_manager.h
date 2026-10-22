#ifndef BOOT_MANAGER_H_NOPD8GAE
#define BOOT_MANAGER_H_NOPD8GAE


#include "main.h"
#include <stdint.h>

#define APP_FLASH_ADDR 0x08008000

typedef void (*pFunction)(void);
/**
 * @brief 跳转到app
 */
void jump_to_app(void);


#endif /* end of include guard: BOOT_MANAGER_H_NOPD8GAE */
