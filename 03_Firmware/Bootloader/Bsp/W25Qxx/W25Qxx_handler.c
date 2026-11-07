
/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file W25Q_Handler.c
 * 
 * @par dependencies 
 * - W25Q_Handler.h
 * 
 * @author Jack | R&D Dept. | EternalChip ��оǶ��ʽ
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
#include "w25qxx_Handler.h"
#include "sfud_adapter.h"  // 使用SFUD适配层
#include "sfud.h"  // 包含SFUD头文件
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
static st_W25Q_Handler s_ast_W25Q_Handler[2];
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/* extern variables ---------------------------------------------------------*/

void SetBlockParmeter(u8 block_index,uint32_t app_size)
{
    s_ast_W25Q_Handler[block_index].write_index = app_size;
    s_ast_W25Q_Handler[block_index].write_databuf_index = app_size % BLOCK_SIZE;
    s_ast_W25Q_Handler[block_index].write_sector_index = app_size / BLOCK_SIZE;
    
    // 同时设置SFUD适配层的块参数
    SFUD_SetBlockParmeter(block_index, app_size);
}

uint32_t Read_BlockSize(u8 block_index)
{
    // 从SFUD适配层读取块大小
    return SFUD_Read_BlockSize(block_index);
}

/**
 * @brief w25q��ʼ��
 * 
 * @param None
 * 
 * @return: None
 */
void W25Q64_Init(void)
{
    // 使用SFUD初始化替换原有的W25Qx_Init
    SFUD_W25Q64_Init();
    
    s_ast_W25Q_Handler[0].read_index = 0;
    s_ast_W25Q_Handler[0].read_sector_index = 0;
    s_ast_W25Q_Handler[0].write_databuf_index = 0;
    s_ast_W25Q_Handler[0].write_index = 0;
    s_ast_W25Q_Handler[0].write_sector_index = 0;

    s_ast_W25Q_Handler[1].read_index = 0;
    s_ast_W25Q_Handler[1].read_sector_index = 0;
    s_ast_W25Q_Handler[1].write_databuf_index = 0;
    s_ast_W25Q_Handler[1].write_index = 0;
    s_ast_W25Q_Handler[1].write_sector_index = 0;
}

void Erase_Flash_Block(u8 block_index)
{
	// 使用SFUD擦除Flash块
	SFUD_Erase_Flash_Block(block_index);
	
	s_ast_W25Q_Handler[block_index].read_index = 0;
	s_ast_W25Q_Handler[block_index].read_sector_index = 0;
	s_ast_W25Q_Handler[block_index].write_databuf_index = 0;
	s_ast_W25Q_Handler[block_index].write_index = 0;
	s_ast_W25Q_Handler[block_index].write_sector_index = 0;
}

/**
 * @brief w25qоƬ����
 * 
 * @param None
 * 
 * @return: None
 */
u8 W25Q64_EraseChip(void)
{
    // 使用SFUD擦除整个芯片
    if(1 == SFUD_W25Q64_EraseChip())
    {
        s_ast_W25Q_Handler[0].read_index = 0;
        s_ast_W25Q_Handler[0].read_sector_index = 0;
        s_ast_W25Q_Handler[0].write_databuf_index = 0;
        s_ast_W25Q_Handler[0].write_index = 0;
        s_ast_W25Q_Handler[0].write_sector_index = 0;
    
        s_ast_W25Q_Handler[1].read_index = 0;
        s_ast_W25Q_Handler[1].read_sector_index = 0;
        s_ast_W25Q_Handler[1].write_databuf_index = 0;
        s_ast_W25Q_Handler[1].write_index = 0;
        s_ast_W25Q_Handler[1].write_sector_index = 0;
        return 0;
    }
    return 1;
}

/**
 * @brief w25qоƬ����
 * 
 * @param None
 * 
 * @return: None
 */
u8 W25Q64_WriteData(u8 block_index, u8 *data, u32 length)
{
    u32 addr = 0;
    u16 index = 0;
    const u32 SUBSECTOR_SIZE = 4096;  // 4KB subsector size
    const u32 PAGE_SIZE = 256;        // 256 byte page size
    
    for(u16 i = 0; i < length; i++)
    {
        index = s_ast_W25Q_Handler[block_index].write_databuf_index;
        s_ast_W25Q_Handler[block_index].databuf[index] = *(data + i);
        s_ast_W25Q_Handler[block_index].write_databuf_index++;
        if(s_ast_W25Q_Handler[block_index].write_databuf_index == SUBSECTOR_SIZE)
        {
            s_ast_W25Q_Handler[block_index].write_databuf_index = 0;
            addr = SUBSECTOR_SIZE * s_ast_W25Q_Handler[block_index].write_sector_index;
            addr += block_index * BLOCK_SIZE;
            
            // 使用SFUD写入整个扇区数据
            uint32_t write_addr = (SUBSECTOR_SIZE * s_ast_W25Q_Handler[block_index].write_sector_index) + (block_index * BLOCK_SIZE);
            
            // 直接使用SFUD的擦除写入功能
            if(SFUD_SUCCESS == sfud_erase_write(sfud_get_device(SFUD_W25Q64_DEVICE_INDEX), write_addr, SUBSECTOR_SIZE, s_ast_W25Q_Handler[block_index].databuf))
            {
                s_ast_W25Q_Handler[block_index].write_sector_index++;
                s_ast_W25Q_Handler[block_index].write_index += SUBSECTOR_SIZE;
            }
        }
    }
    return 0;
}

u8 W25Q64_WriteData_End(u8 block_index)
{
    if(0 != s_ast_W25Q_Handler[block_index].write_databuf_index)
    {
        // 使用SFUD写入剩余的数据
        uint32_t write_addr = (4096 * s_ast_W25Q_Handler[block_index].write_sector_index) + (block_index * BLOCK_SIZE);
        
        if(SFUD_SUCCESS == sfud_erase_write(sfud_get_device(SFUD_W25Q64_DEVICE_INDEX), write_addr, 
                                           s_ast_W25Q_Handler[block_index].write_databuf_index, 
                                           s_ast_W25Q_Handler[block_index].databuf))
        {
            s_ast_W25Q_Handler[block_index].write_index += s_ast_W25Q_Handler[block_index].write_databuf_index;
            s_ast_W25Q_Handler[block_index].write_databuf_index = 0;
        }
    }
    return 0;
}

u8 W25Q64_ReadData(u8 block_index, u8 *data, u16 *length)
{
    u32 addr = 0;
    const u32 SUBSECTOR_SIZE = 4096;  // 4KB subsector size

    if(s_ast_W25Q_Handler[block_index].write_index > s_ast_W25Q_Handler[block_index].read_index)
    {
        if(s_ast_W25Q_Handler[block_index].write_sector_index > s_ast_W25Q_Handler[block_index].read_sector_index)
        {
            *length = SUBSECTOR_SIZE;
            addr = s_ast_W25Q_Handler[block_index].read_sector_index * SUBSECTOR_SIZE;
            addr += block_index * BLOCK_SIZE;
            // 使用SFUD读取数据
            uint32_t read_addr = s_ast_W25Q_Handler[block_index].read_sector_index * SUBSECTOR_SIZE + (block_index * BLOCK_SIZE);
            if(SFUD_SUCCESS != sfud_read(sfud_get_device(SFUD_W25Q64_DEVICE_INDEX), read_addr, *length, data))
                return 2;
            s_ast_W25Q_Handler[block_index].read_sector_index++;
        }
        else
        {
            *length = s_ast_W25Q_Handler[block_index].write_index - s_ast_W25Q_Handler[block_index].read_index;
            addr = s_ast_W25Q_Handler[block_index].read_sector_index * SUBSECTOR_SIZE;
            addr += block_index * BLOCK_SIZE;
            // 使用SFUD读取数据
            uint32_t read_addr2 = s_ast_W25Q_Handler[block_index].read_sector_index * SUBSECTOR_SIZE + (block_index * BLOCK_SIZE);
            if(SFUD_SUCCESS != sfud_read(sfud_get_device(SFUD_W25Q64_DEVICE_INDEX), read_addr2, *length, data))
                return 2;
        }
        s_ast_W25Q_Handler[block_index].read_index += *length;
        return 0;
    }
    else
    {
        return 1;
    }
}

