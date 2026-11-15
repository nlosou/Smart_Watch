/**
 * @file AT24C02.c
 * @brief 
 * @author Noser
 * @version 1
 * @date 2026-10-30
 */

#include "i2c.h"
#include "elog.h"

#define  I2C_Direction_Transmitter      ((uint8_t)0x00)
#define  I2C_Direction_Receiver         ((uint8_t)0x01)

// AT24C02 的7位设备地址（A0=A1=A2=0）
#define AT24C02_ADDR       0x50

// 8位写地址
#define AT24C02_ADDR_W     (AT24C02_ADDR << 1)      // 0xA0
// 8位读地址
#define AT24C02_ADDR_R     ((AT24C02_ADDR << 1) | 1) // 0xA1

void AT24C02_Read_Byte(uint8_t word_address, uint8_t *data)
{
    // 1. 发送 START 信号
    I2C_GenerateSTART(i2c_para.I2Cx, ENABLE);
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    // 2. 发送设备地址 (写)，告诉 EEPROM 我要操作它
    I2C_Send7bitAddress(i2c_para.I2Cx, AT24C02_ADDR_W, I2C_Direction_Transmitter);
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED));

    // 3. 发送要读取的 EEPROM 内部地址
    I2C_SendData(i2c_para.I2Cx, word_address);
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    // 4. 重新发送 START 信号 (Restart)
    I2C_GenerateSTART(i2c_para.I2Cx, ENABLE);
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_MODE_SELECT));

    // 5. 发送设备地址 (读)
    I2C_Send7bitAddress(i2c_para.I2Cx, AT24C02_ADDR_R, I2C_Direction_Receiver);
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED));

    // 6. 准备接收数据：在读取最后一个字节前，必须关闭 ACK (发送 NACK)
    I2C_AcknowledgeConfig(i2c_para.I2Cx, DISABLE);
    
    // 7. 产生 STOP 信号 (在读取完数据前提前产生，让硬件知道这是最后一个字节)
    I2C_GenerateSTOP(i2c_para.I2Cx, ENABLE);

    // 8. 等待 EV7：接收数据寄存器非空 (RxNE)
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_BYTE_RECEIVED));

    // 9. 读取数据
    *data = I2C_ReceiveData(i2c_para.I2Cx);

    // 10. 重新打开 ACK，为下一次通信做准备
    I2C_AcknowledgeConfig(i2c_para.I2Cx, ENABLE);
}


void AT24C02_Write_Byte(uint8_t word_address, uint8_t data)
{
    // 1. 发送 START 信号
    I2C_GenerateSTART(i2c_para.I2Cx, ENABLE);
    // 等待 EV5：起始信号已发送，主机模式已进入
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_MODE_SELECT))
    {
        if(I2C_GetFlagStatus(
            i2c_para.I2Cx,
            I2C_FLAG_AF
        ) == SET)
        {
            elog_e("I2C", "AT24C02 address NACK!");

            I2C_GenerateSTOP(i2c_para.I2Cx, ENABLE);
            I2C_ClearFlag(i2c_para.I2Cx, I2C_FLAG_AF);

            return;
        }
    }

    // 2. 发送设备地址 (写)
    I2C_Send7bitAddress(i2c_para.I2Cx, AT24C02_ADDR_W, I2C_Direction_Transmitter);
    // 等待 EV6：地址发送完成，且收到了从机的 ACK 应答
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED))
{
        if(I2C_GetFlagStatus(
            i2c_para.I2Cx,
            I2C_FLAG_AF
        ) == SET)
        {
            elog_e("I2C", "AT24C02 address NACK!");

            I2C_GenerateSTOP(i2c_para.I2Cx, ENABLE);
            I2C_ClearFlag(i2c_para.I2Cx, I2C_FLAG_AF);

            return;
        }
    }

    // 3. 发送 EEPROM 内部字地址 (Word Address)
    I2C_SendData(i2c_para.I2Cx, word_address);
    // 等待 EV8：数据发送完成，且收到了 ACK
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTING));

    // 4. 发送要写入的数据
    I2C_SendData(i2c_para.I2Cx, data);
    // 等待 EV8_2：数据发送完成，且收到了 ACK
    while(!I2C_CheckEvent(i2c_para.I2Cx, I2C_EVENT_MASTER_BYTE_TRANSMITTED));

    // 5. 发送 STOP 信号
    I2C_GenerateSTOP(i2c_para.I2Cx, ENABLE);
    
    // 6. 极其重要：等待 EEPROM 内部写周期 (5ms)
    for(uint32_t i=0; i<50000; i++); // 粗略延时，具体根据主频调整
}
