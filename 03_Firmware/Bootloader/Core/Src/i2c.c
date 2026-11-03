/**
 * @file i2c.c
 * @brief 
 * @author Noser
 * @version 1
 * @date 2026-10-30
 */


#include "i2c.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_i2c.h"





void i2c_init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    I2C_InitTypeDef  I2C_InitStructure;

    // 1. 使能时钟
    RCC_APB1PeriphClockCmd(i2c_para.I2C_APBx_Clock, ENABLE);
    RCC_AHB1PeriphClockCmd(i2c_para.I2C_GPIOx_Clock, ENABLE);

    // 2. 配置 GPIO 复用功能
    GPIO_PinAFConfig(
        i2c_para.GPIOx,
        GPIO_PinSource6,
        GPIO_AF_I2C1
    );

    GPIO_PinAFConfig(
        i2c_para.GPIOx,
        GPIO_PinSource7,
        GPIO_AF_I2C1
    );    
    // SMBA引脚如果不用，可以不配置AF，或保持默认

    // 3. 配置GPIO为复用开漏模式，并使能内部上拉
    GPIO_InitStructure.GPIO_Pin   = i2c_para.I2C_SCL_PIN | i2c_para.I2C_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF;       // 复用模式
    GPIO_InitStructure.GPIO_OType = GPIO_OType_OD;      // 开漏输出（关键！）
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_PuPd  = GPIO_PuPd_UP;       // 内部上拉（建议）
    GPIO_Init(i2c_para.GPIOx, &GPIO_InitStructure);

    // 4. （可选）软件复位I2C外设，确保处于初始状态
    I2C_DeInit(i2c_para.I2Cx);

    // 5. 配置I2C参数
    I2C_InitStructure.I2C_Mode                = I2C_Mode_I2C;
    I2C_InitStructure.I2C_ClockSpeed          = 100000;  // 建议略低于400kHz，更稳妥
    I2C_InitStructure.I2C_DutyCycle           = I2C_DutyCycle_2;
    I2C_InitStructure.I2C_OwnAddress1         = 0x00;    // 主机地址，可保持0x00或改为0x30等
    I2C_InitStructure.I2C_Ack                 = I2C_Ack_Enable;
    I2C_InitStructure.I2C_AcknowledgedAddress = I2C_AcknowledgedAddress_7bit;

    I2C_Init(i2c_para.I2Cx, &I2C_InitStructure);

    // 6. 使能I2C外设
    I2C_Cmd(i2c_para.I2Cx, ENABLE);
}

void i2c_sendDeviceAddress(uint8_t address,uint8_t Direction)
{
    I2C_Send7bitAddress(
            i2c_para.I2Cx,
            address,
            Direction);
}

void i2c_senddata(uint8_t data)
{
    I2C_SendData(i2c_para.I2Cx,data);
}

void i2c_receivedata(uint8_t *data)
{
    *data = I2C_ReceiveData(i2c_para.I2Cx);
}






