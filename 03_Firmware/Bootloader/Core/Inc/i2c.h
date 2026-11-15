/**
 * @file i2c.h
 * @brief 
 * @author Noser
 * @version 1
 * @date 2026-10-30
 */

#ifndef I2C_H_X1DC80BN
#define I2C_H_X1DC80BN

#include "stm32f4xx.h"



typedef struct {
    GPIO_TypeDef* GPIOx;    
    I2C_TypeDef* I2Cx;    
    uint32_t I2C_APBx_Clock;
    uint32_t I2C_GPIOx_Clock;
    uint32_t I2C_SDA_PIN;
    uint32_t I2C_SCL_PIN;
    uint32_t I2C_SMBA_PIN;
}I2C_Parametere_t;

extern I2C_Parametere_t i2c_para;
void i2c_init(void);


void i2c_senddata(uint8_t data);

void i2c_sendDeviceAddress(uint8_t address,uint8_t Direction);

void i2c_receivedata(uint8_t *data);
#endif /* end of include guard: I2C_H_X1DC80BN */

