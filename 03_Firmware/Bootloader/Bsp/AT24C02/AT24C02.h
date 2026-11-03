/**
 * @file AT24C02.h
 * @brief 
 * @author Noser
 * @version 1
 * @date 2026-10-30
 */

#ifndef AT24C02_H_NLLUDFOQ
#define AT24C02_H_NLLUDFOQ

#include "i2c.h"


typedef struct {
    uint8_t device_address;
    uint8_t* write_address;
    uint8_t* read_address;
}AT24C02_Parameter;


void AT24C02_Init(void);


void AT24C02_Write_Byte(uint8_t address,uint8_t data);
void AT24C02_Read_Byte(uint8_t address,uint8_t *data);



#endif /* end of include guard: AT24C02_H_NLLUDFOQ */
