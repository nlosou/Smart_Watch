/**
 * @file boot_manager.c
 * @brief 
 * @author Noser
 * @version 1
 * @date 2026-11-06
 */




/* Includes ------------------------------------------------------------------*/
#include "boot_manager.h"
#include "Flash.h"
#include "elog.h"
#include "key.h"
#include "AES.h"
#include "AT24Cxx_Driver.h"
#include "ymodem.h"
#include "lwdg.h"
#include "w25qxx_Handler.h"
#include "sfud_adapter.h"  
/* Includes ------------------------------------------------------------------*/

extern key_handler_t  key_handler;
extern key_need_arg_t key_need_arg;


/* Private function prototypes -----------------------------------------------*/

int8_t OTA_BackupCurrentApp(void);
int8_t OTA_InstallApp(int32_t payload_app_size);
extern uint32_t  g_JumpInit;
/* Private functions ---------------------------------------------------------*/
/*! \brief 跳转
 *
 *  Detailed description of the function
 *
 * \param Parameter Parameter description
 * \return Return parameter description
 */
void jump_to_app(void)
{
    uint32_t JumpAddress;
    pFunction Jump_To_Application;

/* 检查栈顶地址是否合法 */
    if(((*(__IO uint32_t *)APP_FLASH_ADDR) & 0x2FFE0000) == 0x20000000)
    {
         //屏蔽所有中断，防止在跳转过程中，中断干扰出现异常 */
        __disable_irq();
        NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0x8000);
        RCC_DeInit();

        /* 用户代码区第二个 字 为程序开始地址(复位地址) */
        JumpAddress = *(__IO uint32_t *) (APP_FLASH_ADDR+ 4);

        /* Initialize user application's Stack Pointer */
        /* 初始化APP堆栈指针(用户代码区的第一个字用于存放栈顶地址) */
        __set_MSP(*(__IO uint32_t *)APP_FLASH_ADDR);

        /* 类型转换 */
        Jump_To_Application = (pFunction) JumpAddress;

        /* 跳转到 APP */
        Jump_To_Application();
    }
}



unsigned char AES128key[16] = "123456789abcdefa";
unsigned char AES_IV[16]    = "0102030405123456";
uint16_t w25qxx_len =0;
uint8_t w25q64xx_data[4096] = {0};

/**
 * @brief 软件复位单片机
 * 
 * @param None
 * 
 * @return: None
 */
void System_SoftwareReset(void)
{
  __NVIC_SystemReset();
}
/**
 * @brief 将解密后的BLOCK_B数据写入片上Flash运行区
 *
 * @param app_size
 *
 * @return 
 */
int8_t back_to_app(int32_t app_size)
{
    if(app_size <= 0) return -1;
    if(app_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) return -1;

    uint32_t app_address = APP_FLASH_ADDR;
    uint32_t remaining = (uint32_t)app_size;

    Flash_erase(app_address, app_size);

    uint32_t read_cnt = 0;
    while(remaining > 0)
    {
        // 读取 B 区数据
        W25Q64_ReadData(BLOCK_B,w25q64xx_data, &w25qxx_len);
        read_cnt++;

        // 检查读取长度
        if(w25qxx_len == 0 ||
           w25qxx_len > sizeof(w25q64xx_data))
        {
            return -1;
        }

        // 本次实际需要写入的字节数
        uint32_t write_len =
            (w25qxx_len > remaining) ? remaining : w25qxx_len;

        // 非最后一个数据块必须满足 4 字节对齐
        if(write_len < remaining && (write_len % 4 != 0))
        {
            return -1;
        }

        for(uint32_t j = 0; j < write_len; j += 4)
        {
            uint32_t word = 0xFFFFFFFF;

            uint32_t copy_len =
                (write_len - j >= 4) ? 4 : (write_len - j);

            memcpy(&word, w25q64xx_data + j, copy_len);

            // 写入内部 Flash
            Flash_Write(app_address, word);

            // 校验
            uint32_t verify_word = 0;
            memcpy(&verify_word, (void *)app_address, 4);

            if(verify_word != word)
            {
                return -1;
            }

            app_address += 4;
        }

        // 更新剩余数据量
        remaining -= write_len;
    }

    return 0;
}

/**
 * @brief 回滚代码
 *
 * @param app_size
 *
 * @return 
 */
int8_t blockA_to_app(int32_t app_size)
{
    if(app_size <= 0) return -1;
    if(app_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) return -1;

    uint32_t app_address = APP_FLASH_ADDR;
    uint32_t remaining = (uint32_t)app_size;

    Flash_erase(app_address, app_size);

    uint32_t read_cnt = 0;
    while(remaining > 0)
    {
        // 读取 A 区数据
        W25Q64_ReadData(BLOCK_A,w25q64xx_data, &w25qxx_len);
        read_cnt++;

        // 检查读取长度
        if(w25qxx_len == 0 ||
           w25qxx_len > sizeof(w25q64xx_data))
        {
            return -1;
        }

        // 本次实际需要写入的字节数
        uint32_t write_len =
            (w25qxx_len > remaining) ? remaining : w25qxx_len;

        // 非最后一个数据块必须满足 4 字节对齐
        if(write_len < remaining && (write_len % 4 != 0))
        {
            return -1;
        }

        for(uint32_t j = 0; j < write_len; j += 4)
        {
            uint32_t word = 0xFFFFFFFF;

            uint32_t copy_len =
                (write_len - j >= 4) ? 4 : (write_len - j);

            memcpy(&word, w25q64xx_data + j, copy_len);

            // 写入内部 Flash
            Flash_Write(app_address, word);

            // 校验
            uint32_t verify_word = 0;
            memcpy(&verify_word, (void *)app_address, 4);

            if(verify_word != word)
            {
                return -1;
            }

            app_address += 4;
        }

        // 更新剩余数据量
        remaining -= write_len;
    }

    return 0;
}




/**
 * @brief 解析A区的代码到B区间
 *
 * @param ciphertext_size 密文大小
 * @param payload_app_size 返回明文app大小
 *
 * @return 
 */
int8_t back_to_w25Qxx_back(int32_t ciphertext_size,int32_t* payload_app_size)
{ 
    if(ciphertext_size<= 0) return -1;
    if(ciphertext_size> (BACK_FLASH_ADDR - APP_FLASH_ADDR)) return -1;

    
    uint8_t descryptbuf[16];
    uint8_t current_iv[16]; // 用于存放动态更新的 IV
    uint32_t App_size;
    uint8_t *p_data =w25q64xx_data;

    int32_t current_4k_buf_remaining;
    AES_Init(AES128key);
    memcpy(current_iv, AES_IV, 16); // 初始化 IV
    
    // ---------------- 1. 解密头部 ----------------
   // memcpy(p_data, (uint8_t*)back_address, 16);

    W25Q64_ReadData(BLOCK_A,p_data, &w25qxx_len);
    current_4k_buf_remaining =w25qxx_len;
    AES_Decrypt(p_data, descryptbuf, 16, current_iv);
    memcpy(current_iv, p_data, 16); // 更新 IV 为第一个密文块
    p_data+=16;

    current_4k_buf_remaining-=16;
    // 解析 App_size (假设小端序)
    App_size = (descryptbuf[15]<< 24) | (descryptbuf[14]<< 16) | (descryptbuf[13]<< 8) | descryptbuf[12];

    // 校验合法性防止跑飞
    if(App_size == 0 || App_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) {
        return -2; // 长度不合法，可能解密错误或头部格式不对
    }

    *payload_app_size = App_size;
    // ---------------- 3. 循环解密并写入 ----------------
    uint32_t total_blocks = (App_size + 15) / 16; // 向上取整计算块数
    uint32_t remain_size = App_size;
    
    //每次写入16字节
    for(uint32_t i = 0; i < total_blocks; i++)
    {
        //当前缓冲区数据如果用完了就再次读取
        if(current_4k_buf_remaining <= 0)
        {

            p_data =w25q64xx_data;
            W25Q64_ReadData(BLOCK_A,p_data, &w25qxx_len);
            current_4k_buf_remaining = w25qxx_len;
        }
        
        // 使用当前的 IV 解密
        AES_Decrypt(p_data, descryptbuf, 16, current_iv);
        // 更新 IV 为当前密文块，供下一次解密使用
        memcpy(current_iv,p_data, 16);
        p_data+=16; 
        // 计算当前块实际要写入的字节数（处理最后一块不是16的倍数的情况）
        uint32_t write_len = (current_4k_buf_remaining>= 16) ? 16 :current_4k_buf_remaining;

        W25Q64_WriteData(BLOCK_B,descryptbuf,write_len);

        current_4k_buf_remaining-= write_len;         
    }
    
    W25Q64_WriteData_End(BLOCK_B);
    return 0;
}



/**
 * @brief 将B区域的数据复制到A区域中
 *
 * @param app_size
 *
 * @return 
 */
int8_t BLOCKB_to_BLOCKA(int32_t app_size)
{ 
    if(app_size <= 0) return -1;
    if(app_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) return -1;

    uint32_t app_address = APP_FLASH_ADDR;
    uint32_t remaining = (uint32_t)app_size;

    Flash_erase(app_address, app_size);

    uint32_t read_cnt = 0;
    while(remaining > 0)
    {
        // 读取 B 区数据
        W25Q64_ReadData(BLOCK_B,w25q64xx_data, &w25qxx_len);
        read_cnt++;

        // 检查读取长度
        if(w25qxx_len == 0 ||
           w25qxx_len > sizeof(w25q64xx_data))
        {
            return -1;
        }

        // 本次实际需要写入的字节数
        uint32_t write_len =
            (w25qxx_len > remaining) ? remaining : w25qxx_len;

        // 非最后一个数据块必须满足 4 字节对齐
        
        W25Q64_WriteData(BLOCK_A,w25q64xx_data,w25qxx_len);

        // 更新剩余数据量
        remaining -= write_len;
    }
    W25Q64_WriteData_End(BLOCK_A);
}

/**
 * @brief 备份片上Flash运行区App的代码到BLOCK_A区中
 *
 * @param app_size
 *
 * @return 
 */
int8_t App_to_w25Qxx_back(int32_t app_size)
{ 
    if(app_size <= 0) return -1;
    if(app_size > (BACK_FLASH_ADDR - APP_FLASH_ADDR)) return -1;
     
    uint32_t app_address  = APP_FLASH_ADDR;
    //uint32_t back_address = BACK_FLASH_ADDR;
    
    uint8_t descryptbuf[16];
    uint8_t current_iv[16]; // 用于存放动态更新的 IV
    uint32_t App_size;
    uint8_t *p_data =w25q64xx_data;

    uint32_t* read_addr =(uint32_t*) APP_FLASH_ADDR;
    
    for(uint32_t i = 0 ; i < app_size ; i+=4)
    {
        W25Q64_WriteData(BLOCK_A,(uint8_t*)read_addr,4);
        read_addr++;
    }
    W25Q64_WriteData_End(BLOCK_A);
    return 0;
}

/**
 * @brief ota状态机处理 
 */

uint8_t temp_app_data[1024] = {0};
uint8_t ota_status             = NO_APP_NEED_TO_UPDATE;
void Ota_SateManger(void)
{

    uint8_t temp_data[4]             = {0};
    int32_t app_size                 = 0;
    int32_t temp_app_size            = 0;
    uint8_t need_upate_app_size[4]   = {0};
    uint8_t back_app_size[4] = {0};
    int32_t back_app_size_temp;
    int32_t payload_app_size;
#if 0
    ota_status = DOWNLOAD;
#endif

#if 0
    ota_status =APP_REQ_UPDATE;
#endif

 #if 0
    ota_status = CODE_ROLLBACK;
#endif
 
    switch(ota_status)
    {
      case NO_APP_NEED_TO_UPDATE:
          //检测按键是否按下
          if(KEY_HANDLER_OK == key_handler.key_scan(&key_handler))
          {
                ota_status = DOWNLOAD;
          }
          else
          {
                //没有按下就跳转,跳转失败就进入下载模式
                //jump_to_app();
                g_JumpInit = 0x55AA55AA;
                System_SoftwareReset();
                //ota_status = DOWNLOAD;
          }
          break;
      case APP_DOWNLOADING:
         elog_e("ERROR","Download false!!!");
         ota_status = NO_APP_NEED_TO_UPDATE;
         ee_WriteBytes(&ota_status,0x00,1);
         //合法就跳转
         jump_to_app();
         elog_e("ERROR","App data broken");
         //不合法就跳转到下载状态
         ota_status = DOWNLOAD;
          break;
      case APP_REQ_UPDATE:
            //这里获取的大小是,密文的大小
            ee_ReadBytes(need_upate_app_size,0x01,4);
            
            //解密并写入B区间
            //配置W25Qxx从A区读取,解密并写入B区间
            // 将 4 个字节整合成 32 位无符号整型
           temp_app_size= (int32_t)need_upate_app_size[0] | 
                    ((int32_t)need_upate_app_size[1] << 8) | 
                    ((int32_t)need_upate_app_size[2] << 16) | 
                    ((int32_t)need_upate_app_size[3] << 24);

            SetBlockParmeter(BLOCK_A,temp_app_size);
            if( 0!=back_to_w25Qxx_back(temp_app_size,&payload_app_size))
            {
                elog_e("INFO", "Parse False!!");            
                
                ota_status = NO_APP_NEED_TO_UPDATE;
                ee_WriteBytes(&ota_status,0x00,1);
                return;

            }
            
            //把B区代码安装在片上Flash中
            if(0!=OTA_InstallApp(payload_app_size))    
            {
                ota_status = CODE_ROLLBACK;
                return;
            }
             
            jump_to_app();

            //跳转失败回滚代码
            ota_status = CODE_ROLLBACK;
          break;
      case DOWNLOAD:
        //进入下载模式 
        elog_i("INFO", "Start Download to back flash.......");            
        //接收数据并写入A区间
        temp_app_size = Ymodem_Receive(temp_app_data);

        //解密并写入B区间
        //配置W25Qxx从A区读取,解密并写入B区间
        if( 0!=back_to_w25Qxx_back(temp_app_size,&payload_app_size))
        {
            elog_e("INFO", "Parse False!!");            
            jump_to_app();
            ota_status = NO_APP_NEED_TO_UPDATE;
            return;

        }
        //把B区代码安装在片上Flash中
        if(0!=OTA_InstallApp(payload_app_size))    
        {
            ota_status = CODE_ROLLBACK;
            return;
            
        }
        ota_status = APP_FIRST_CHECK_START;
        ee_WriteBytes(&ota_status,0x00,1);
        g_JumpInit = 0x55AA55AA;
        System_SoftwareReset();
          break;
      case CODE_ROLLBACK:
            //读取当前app的size
        ee_ReadBytes(back_app_size,0x09,4);
            // 将 4 个字节整合成 32 位无符号整型
          back_app_size_temp= (int32_t)back_app_size[0] | 
                ((int32_t)back_app_size[1] << 8) | 
                ((int32_t)back_app_size[2] << 16) | 
                ((int32_t)back_app_size[3] << 24);

          blockA_to_app(back_app_size_temp);

          jump_to_app();
          //不合法就跳转到下载状态
          ota_status = DOWNLOAD;
          break;
        case APP_FIRST_CHECK_START:
          ota_status = APP_FIRST_CHECK; 
          ee_WriteBytes(&ota_status,0x00,1);  
          IWDG_Init(IWDG_Prescaler_64,3000);

          jump_to_app();
          break;
        case APP_FIRST_CHECK:
          ota_status = NO_APP_NEED_TO_UPDATE; 
          ee_WriteBytes(&ota_status,0x00,1);  
          //软件复位
          System_SoftwareReset();
          break;
      default:
         ota_status = DOWNLOAD;
          break;

    }
}
int8_t OTA_BackupCurrentApp(void)
{
    uint8_t back_app_size[4] = {0};
    uint32_t old_app_size = 0;


    // 判断当前是否存在有效 App
    if (((*(__IO uint32_t *)APP_FLASH_ADDR) & 0x2FFE0000) != 0x20000000)
    {
         // 第一次安装，不需要备份
        return 0;

    }

    // 读取当前 App 的大小
    ee_ReadBytes(back_app_size, 0x05, 4);

    old_app_size = (uint32_t)back_app_size[0] |
                   ((uint32_t)back_app_size[1] << 8) |
                   ((uint32_t)back_app_size[2] << 16) |
                   ((uint32_t)back_app_size[3] << 24);

    // 检查 App 大小是否合法
    if (old_app_size == 0 ||
        old_app_size == 0xFFFFFFFF ||
        old_app_size >0x80000)
    {
        return -1;
    }

		W25Q64_ResetBlockState(BLOCK_A);
    // 备份当前 App 到 W25Qxx
    if (App_to_w25Qxx_back(old_app_size) != 0)
    {
        elog_e("OTA", "Backup App Failed!");
        return -1;
    }
		 ee_WriteBytes((uint8_t*)&old_app_size,0x09,4);
    // TODO: 校验备份数据，例如比较 CRC32
    // if (W25Qxx_BackupVerify(old_app_size) != 0)
    // {
    //     return -1;
    // }

    return 0;
}

int8_t OTA_InstallApp(int32_t payload_app_size)
{
    // 1. 校验 W25Qxx B 区的新固件
    // 2. 备份当前 App

    if( 0!= OTA_BackupCurrentApp())
    {
        elog_i("OTA","OTA Backup False");
        return -1;
    }
    // 3. 持久化升级进行中的状态
    // 4. 将 B 区固件写入片上 Flash
//		SetBlockParmeter(BLOCK_B,payload_app_size);
    if( 0 != back_to_app(payload_app_size))
    {
        elog_i("INFO", "B Are to App");            
        return -1;
    }
    // 5. 校验片上 Flash 中的新固件

    // 6. 更新 EEPROM 中的 App 大小
    ee_WriteBytes((uint8_t*)&payload_app_size,0x05,4);
    // 7. 标记升级成功
     //将B区间代码写入片上Flash中
        
    return 0;
}
