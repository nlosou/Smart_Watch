
/******************************************************************************
 * Copyright (C) 2024 EternalChip, Inc.(Gmbh) or its affiliates.
 * 
 * All Rights Reserved.
 * 
 * @file aec.c
 * 
 * @par dependencies 
 * 
 * 
 * @author 
 * 
 * @brief Provide the HAL APIs of Key and corresponding opetions.
 * 
 * Processing flow:
 * 
 * call directly.
 * 
 * @version V1.0 2024-12-27
 *
 * @note 1 tab == 4 spaces!
 * 
 *****************************************************************************/


#include "AES.h"

//******************************** Defines **********************************//


// 计算在 GF(2^8) 中的乘法
unsigned char gmul(unsigned char a, unsigned char b);

// 使用暴力搜索找到 GF(2^8) 中的逆元
unsigned char ginv(unsigned char x);

// 仿射变换函数
unsigned char affineTransform(unsigned char x);

/*
 * #ifndef __C51__: 
 * 检查是否定义了 __C51__，用于识别是否在C51编译环境下。C51环境下有一些特定关键字（如 code、data、idata、xdata、pdata），用于不同的存储区。
 *如果不是C51环境（即没有定义 __C51__），就将这些关键字定义为空，以确保在其他环境中不会出错。
 * 同时，在非C51环境中，将 BOOL 定义为 unsigned char 类型，而在C51环境中定义为 bit 类型（8051系列芯片有一个特定的 bit 类型，可以单独占用1位内存）。    
 */
#ifndef __C51__
    #define code
    #define data
    #define idata
    #define xdata
    #define pdata
    typedef unsigned char BOOL;
#else
    typedef bit BOOL;
#endif

/*
 * Nk 表示AES密钥的长度（单位是字，4字节/字）。
 * 假设 AES_KEY_LENGTH 为128、192或256，则 Nk 分别为4、6、8。
 */
#define Nk  (AES_KEY_LENGTH / 32)       

/*
 * Nb 表示数据块大小（单位是字），固定为4。这是AES算法的固定标准，AES处理的是4x4的字节块。
 */
#define Nb  4         

// Nr 表示AES的轮数，取决于密钥长度。通过 #if 语句确定 AES_KEY_LENGTH 值为128、192或256时的轮数。如果值不在这三者范围，将会抛出编译错误。
#if   AES_KEY_LENGTH == AES_KEY_LENGTH_128
    #define Nr  10
#elif AES_KEY_LENGTH == AES_KEY_LENGTH_192
    #define Nr  12
#elif AES_KEY_LENGTH == AES_KEY_LENGTH_256
    #define Nr  14
#else
    #error AES_KEY_LENGTH must be 128, 192 or 256 BOOLs!
#endif      

/*
 * BPOLY 用于AES中有限域（GF(2^8)）运算。其值 0x1B 表示GF(2^8)中的不可约多项式 
 * (x^8 + x^4 + x^3 + x + 1) 的低8位。
 * 在AES的 MixColumns 操作中，有限域多项式 BPOLY 被用作乘法的模，确保运算在8位内循环。
*/
#define BPOLY 0x1B

// AES子密钥表，当密钥长度为128位时，占用176字节空间
static xdata unsigned char g_roundKeyTable[4*Nb*(Nr+1)];

static const unsigned char sBox[256] = {
    0x63,0x7c,0x77,0x7b,0xf2,0x6b,0x6f,0xc5,0x30,0x01,0x67,0x2b,0xfe,0xd7,0xab,0x76,
    0xca,0x82,0xc9,0x7d,0xfa,0x59,0x47,0xf0,0xad,0xd4,0xa2,0xaf,0x9c,0xa4,0x72,0xc0,
    0xb7,0xfd,0x93,0x26,0x36,0x3f,0xf7,0xcc,0x34,0xa5,0xe5,0xf1,0x71,0xd8,0x31,0x15,
    0x04,0xc7,0x23,0xc3,0x18,0x96,0x05,0x9a,0x07,0x12,0x80,0xe2,0xeb,0x27,0xb2,0x75,
    0x09,0x83,0x2c,0x1a,0x1b,0x6e,0x5a,0xa0,0x52,0x3b,0xd6,0xb3,0x29,0xe3,0x2f,0x84,
    0x53,0xd1,0x00,0xed,0x20,0xfc,0xb1,0x5b,0x6a,0xcb,0xbe,0x39,0x4a,0x4c,0x58,0xcf,
    0xd0,0xef,0xaa,0xfb,0x43,0x4d,0x33,0x85,0x45,0xf9,0x02,0x7f,0x50,0x3c,0x9f,0xa8,
    0x51,0xa3,0x40,0x8f,0x92,0x9d,0x38,0xf5,0xbc,0xb6,0xda,0x21,0x10,0xff,0xf3,0xd2,
    0xcd,0x0c,0x13,0xec,0x5f,0x97,0x44,0x17,0xc4,0xa7,0x7e,0x3d,0x64,0x5d,0x19,0x73,
    0x60,0x81,0x4f,0xdc,0x22,0x2a,0x90,0x88,0x46,0xee,0xb8,0x14,0xde,0x5e,0x0b,0xdb,
    0xe0,0x32,0x3a,0x0a,0x49,0x06,0x24,0x5c,0xc2,0xd3,0xac,0x62,0x91,0x95,0xe4,0x79,
    0xe7,0xc8,0x37,0x6d,0x8d,0xd5,0x4e,0xa9,0x6c,0x56,0xf4,0xea,0x65,0x7a,0xae,0x08,
    0xba,0x78,0x25,0x2e,0x1c,0xa6,0xb4,0xc6,0xe8,0xdd,0x74,0x1f,0x4b,0xbd,0x8b,0x8a,
    0x70,0x3e,0xb5,0x66,0x48,0x03,0xf6,0x0e,0x61,0x35,0x57,0xb9,0x86,0xc1,0x1d,0x9e,
    0xe1,0xf8,0x98,0x11,0x69,0xd9,0x8e,0x94,0x9b,0x1e,0x87,0xe9,0xce,0x55,0x28,0xdf,
    0x8c,0xa1,0x89,0x0d,0xbf,0xe6,0x42,0x68,0x41,0x99,0x2d,0x0f,0xb0,0x54,0xbb,0x16
};

static const unsigned char invSBox[256]= {
	0x52,0x09,0x6a,0xd5,0x30,0x36,0xa5,0x38,0xbf,0x40,0xa3,0x9e,0x81,0xf3,0xd7,0xfb,
	0x7c,0xe3,0x39,0x82,0x9b,0x2f,0xff,0x87,0x34,0x8e,0x43,0x44,0xc4,0xde,0xe9,0xcb,
	0x54,0x7b,0x94,0x32,0xa6,0xc2,0x23,0x3d,0xee,0x4c,0x95,0x0b,0x42,0xfa,0xc3,0x4e,
	0x08,0x2e,0xa1,0x66,0x28,0xd9,0x24,0xb2,0x76,0x5b,0xa2,0x49,0x6d,0x8b,0xd1,0x25,
	0x72,0xf8,0xf6,0x64,0x86,0x68,0x98,0x16,0xd4,0xa4,0x5c,0xcc,0x5d,0x65,0xb6,0x92,
	0x6c,0x70,0x48,0x50,0xfd,0xed,0xb9,0xda,0x5e,0x15,0x46,0x57,0xa7,0x8d,0x9d,0x84,
	0x90,0xd8,0xab,0x00,0x8c,0xbc,0xd3,0x0a,0xf7,0xe4,0x58,0x05,0xb8,0xb3,0x45,0x06,
	0xd0,0x2c,0x1e,0x8f,0xca,0x3f,0x0f,0x02,0xc1,0xaf,0xbd,0x03,0x01,0x13,0x8a,0x6b,
	0x3a,0x91,0x11,0x41,0x4f,0x67,0xdc,0xea,0x97,0xf2,0xcf,0xce,0xf0,0xb4,0xe6,0x73,
	0x96,0xac,0x74,0x22,0xe7,0xad,0x35,0x85,0xe2,0xf9,0x37,0xe8,0x1c,0x75,0xdf,0x6e,
	0x47,0xf1,0x1a,0x71,0x1d,0x29,0xc5,0x89,0x6f,0xb7,0x62,0x0e,0xaa,0x18,0xbe,0x1b,
	0xfc,0x56,0x3e,0x4b,0xc6,0xd2,0x79,0x20,0x9a,0xdb,0xc0,0xfe,0x78,0xcd,0x5a,0xf4,
	0x1f,0xdd,0xa8,0x33,0x88,0x07,0xc7,0x31,0xb1,0x12,0x10,0x59,0x27,0x80,0xec,0x5f,
	0x60,0x51,0x7f,0xa9,0x19,0xb5,0x4a,0x0d,0x2d,0xe5,0x7a,0x9f,0x93,0xc9,0x9c,0xef,
	0xa0,0xe0,0x3b,0x4d,0xae,0x2a,0xf5,0xb0,0xc8,0xeb,0xbb,0x3c,0x83,0x53,0x99,0x61,
	0x17,0x2b,0x04,0x7e,0xba,0x77,0xd6,0x26,0xe1,0x69,0x14,0x63,0x55,0x21,0x0c,0x7d
	 
};

#define AES_MOD 0x11B  // AES 的不可约多项式 x^8 + x^4 + x^3 + x + 1

// AddRoundKey的宏形式，比函数形式可以节省4字节的data数据
#define AddRoundKey(pState, pRoundKey) \
    xor_bytes((pState), (pRoundKey), 4*Nb)

// 计算在 GF(2^8) 中的乘法
unsigned char gmul(unsigned char a, unsigned char b) {
    unsigned char p = 0;  // 初始化乘积为 0
    while (b) {           // 当 b 不为 0 时循环
        if (b & 1) p ^= a; // 如果 b 的最低位为 1，则将 a 加入到 p 中（在 GF(2^8) 中是异或）
        a = (a << 1) ^ ((a & 0x80) ? AES_MOD : 0); // 将 a 左移一位，并根据最高位决定是否与 AES_MOD 异或
        b >>= 1;         // 将 b 右移一位
    }
    return p;            // 返回计算的结果
}


// 使用暴力搜索找到 GF(2^8) 中的逆元
unsigned char ginv(unsigned char x) {
    if (x == 0) return 0;  // 0 没有逆元
    for (unsigned char i = 1; i < 256; i++) {
        if (gmul(x, i) == 1) {
            return i;
        }
    }
    return 0;  // 不应该到达此行
}

// 仿射变换函数
unsigned char affineTransform(unsigned char x) {
    unsigned char result = x ^ ((x << 1) | (x >> 7)) ^ ((x << 2) | (x >> 6)) ^
        ((x << 3) | (x >> 5)) ^ ((x << 4) | (x >> 4)) ^ 0x63;
    return result & 0xFF;
}

// 生成 AES S-Box
void generate_aes_sbox(unsigned char sbox[256]) {
    for (int i = 0; i < 256; i++) {
        unsigned char x = ginv(i);  // 计算逆元素
        sbox[i] = affineTransform(x);  // 应用仿射变换
    }
    sbox[0] = 0x63;  // S-Box[0] 的特例处理
}

// 生成 AES 逆 S-Box
void generate_aes_inverse_sbox(const unsigned char sbox[256], unsigned char inv_sbox[256]) {
    for (int i = 0; i < 256; i++) {
        inv_sbox[sbox[i]] = i;
    }
}

/*
 * 函数名：     rotation_word
 * 描述：      定义一个静态函数，旋转输入的字（4个字节）
 * 输入参数：    pWord -- 要右移的4字节数据。
 * 输出参数：    pWord -- 右移后的4字节数据。
 * 返回值： 无。
 */
static void rotation_word(unsigned char *pWord) {
    // 检查输入指针是否为空，避免潜在的空指针解引用错误
    if (pWord == NULL) return; // 参数检查，若为空则返回

    // 保存第一个字节到临时变量
    unsigned char temp = pWord[0];

    // 将字的每个字节向前移动一个位置
    pWord[0]  = pWord[1]; // 第一个字节变为第二个字节
    pWord[1]  = pWord[2]; // 第二个字节变为第三个字节
    pWord[2]  = pWord[3]; // 第三个字节变为第四个字节

    // 将原本的第一个字节放到最后一个位置
    pWord[3]  = temp;     // 第四个字节变为原来的第一个字节
}

/**
 * @brief 批量异或两组数据。
 *
 * 此函数将 pData2 中的每个字节与 pData1 中的对应字节进行异或操作，结果直接存储在 pData1 中。
 * 适用于需要逐字节混淆或合并数据的应用场景，如加密算法中的混淆步骤。
 *
 * @param[out] pData1 要异或的第一组数据，同时用于存储异或结果。
 * @param[in]  pData2 要异或的第二组数据。
 * @param[in]  nCount 要异或的数据字节长度。
 *
 * @note 调用者需确保 pData1 和 pData2 均至少有 nCount 个字节，否则可能导致访问越界。
 *       本函数对指针为空不进行检查。
 */
static void xor_bytes(unsigned char *pData1, const unsigned char *pData2, unsigned char nCount)
{
    // 定义一个循环计数变量 i
    unsigned char i;
    
    // 循环遍历每个字节，进行逐位异或
    for (i = 0; i < nCount; i++)
    {
        // 将 pData1 的每个字节与 pData2 的相应字节异或
        // 异或结果直接存储在 pData1 中
        pData1[i] ^= pData2[i];
    }
}

///**
// * @brief 将中间状态数据与子密钥进行异或操作。
// *
// * 此函数将状态数据 pState 与子密钥 pRoundKey 进行异或运算，结果存储在 pState 中。
// * 数据长度固定为 16 字节（4 个字，每个字 4 字节），通常用于 AES 加密算法的轮密钥加操作。
// *
// * @param[in,out] pState    状态数据，经过异或后将更新为新的状态数据。
// * @param[in]     pRoundKey 子密钥数据，用于与状态数据进行异或。
// *
// * @note 此函数对输入指针不进行空值检查，调用者需确保 pState 和 pRoundKey 不为空并具有有效长度。
// */
//static void AddRoundKey(unsigned char *pState, const unsigned char *pRoundKey)
//{
//    xor_bytes(pState, pRoundKey, 4 * Nb);
//}

/**
 * @brief 使用 S-盒对状态数据进行字节替换。
 *
 * 此函数将状态数据中的每个字节用 S-盒进行替换。
 * 可选择使用反向 S-盒（解密时使用）。
 *
 * @param[in,out] pState 状态数据，经过替换后将更新为新的状态数据。
 * @param[in]     nCount 状态数据的长度。
 * @param[in]     bInvert 指示是否使用反向 S-盒。
 *
 * @note 此函数对输入指针不进行空值检查，调用者需确保 pState 不为空且 nCount 有效。
 */
static void sub_bytes(unsigned char *pState, unsigned char nCount, BOOL bInvert)
{
    unsigned char i;  // 循环索引
    const unsigned char code *pSBox = bInvert ? invSBox : sBox; // 根据是否反向选择 S-盒

    for (i = 0; i < nCount; i++)  // 遍历状态数据的每个字节
    {
        pState[i] = pSBox[pState[i]]; // 用 S-盒替换状态数据中的字节
    }
}


/**
 * @brief 对状态数据进行行移位操作。
 *
 * 此函数根据 AES 算法的规定，将状态数据中的每一行移位。
 * 可选择反向移位（解密时使用）。
 *
 * @param[in,out] pState 状态数据，经过移位后将更新为新的状态数据。
 * @param[in]     bInvert 指示是否进行反向移位。
 *
 * @note 状态数据以列为主进行存储，调用者需确保 pState 不为空。
 */
static void shift_rows(unsigned char *pState, BOOL bInvert)
{
    // 注意：状态数据以列形式存放！

    unsigned char r;       // 行索引
    unsigned char c;       // 列索引
    unsigned char temp;    // 用于存储移位值
    unsigned char rowData[4]; // 用于备份当前行数据

    for (r = 1; r < 4; r++) // 从第二行开始，循环到第四行
    {
        // 备份当前行的数据
        for (c = 0; c < 4; c++)
        {
            rowData[c] = pState[r + 4 * c]; // 将当前行的字节存入 rowData 中
        }

        // 根据是否反向选择移位的步数
        temp = bInvert ? (4 - r) : r; // 计算移位步数
        for (c = 0; c < 4; c++)
        {
            pState[r + 4 * c] = rowData[(c + temp) % 4]; // 更新状态数据，执行移位
        }
    }
}

/**
 * @brief 在 GF(2^8) 域内进行乘以 2 的运算。
 *
 * 该函数实现 GF(2^8) 的乘2运算。根据有限域的定义，
 * 当乘数的最高位为0时，直接左移一位；当最高位为1时，
 * 左移后需要与多项式 BPOLY 进行异或运算。
 *
 * @param[in] num 需要乘以 2 的数（8 位无符号整数）。
 *
 * @return num 乘以 2 的结果（8 位无符号整数）。
 */
static unsigned char gf_mult_by02(unsigned char num)
{
    // 检查 num 的最高位（第8位）是否为 0
    if ((num & 0x80) == 0)  // 通过位与操作检查最高位
    {
        num = num << 1;  // 如果最高位为 0，左移一位
    }
    else
    {
        // 如果最高位为 1，左移一位并与 GF(2^8) 的模多项式进行异或
        num = (num << 1) ^ BPOLY;  // 左移后与 BPOLY 异或
    }
    
    return num;  // 返回乘以 2 的结果
}

/**
 * @brief 混合状态的各列数据。
 *
 * 该函数实现 AES 加密算法中的列混合操作。对于每一列的状态数据，
 * 进行一定的线性变换以增加数据的扩散性。在解密时，该过程需要反向进行。
 *
 * @param[in,out] pState  状态数据（每列包含 4 个字节）。
 * @param[in] bInvert     是否执行反向混合（用于解密）。
 *
 * @return 无返回值。
 */
static void mix_columns(unsigned char *pState, BOOL bInvert)
{
    unsigned char i;  // 循环索引
    unsigned char temp;  // 中间存储变量
    unsigned char a0Pa2_M4;  // 4(a0 + a2)
    unsigned char a1Pa3_M4;  // 4(a1 + a3)
    unsigned char result[4];  // 存储混合后的结果

    // 对每一列进行混合操作
    for (i = 0; i < 4; i++, pState += 4)
    {
        // 计算 b0 = 2a0 + 3a1 + a2 + a3 
        // temp 是所有四个字节的异或结果
        temp = pState[0] ^ pState[1] ^ pState[2] ^ pState[3];

        // 计算混合后的每个字节
        result[0] = temp ^ pState[0] ^ gf_mult_by02((unsigned char)(pState[0] ^ pState[1]));
        result[1] = temp ^ pState[1] ^ gf_mult_by02((unsigned char)(pState[1] ^ pState[2]));
        result[2] = temp ^ pState[2] ^ gf_mult_by02((unsigned char)(pState[2] ^ pState[3]));
        result[3] = temp ^ pState[3] ^ gf_mult_by02((unsigned char)(pState[3] ^ pState[0]));

        // 如果是解密，进行反向混合操作
        if (bInvert)
        {
            // 反向混合：b0' = 14a0 + 11a1 + 13a2 + 9a3 
            a0Pa2_M4 = gf_mult_by02(gf_mult_by02((unsigned char)(pState[0] ^ pState[2])));  // 计算 4(a0 + a2)
            a1Pa3_M4 = gf_mult_by02(gf_mult_by02((unsigned char)(pState[1] ^ pState[3])));  // 计算 4(a1 + a3)
            temp = gf_mult_by02((unsigned char)(a0Pa2_M4 ^ a1Pa3_M4));  // 计算 2(a0 + a2 + a1 + a3)

            // 更新结果
            result[0] ^= temp ^ a0Pa2_M4;
            result[1] ^= temp ^ a1Pa3_M4;
            result[2] ^= temp ^ a0Pa2_M4;
            result[3] ^= temp ^ a1Pa3_M4;
        }

        // 将混合后的结果写回状态数据
        memcpy(pState, result, 4);
    }
}


/**
 * @brief 对单块数据进行加密。
 *
 * 此函数实现 AES 加密算法的基本结构，对给定的状态数据进行加密处理。
 * 加密过程中包含多个步骤，包括轮密钥添加、字节替换、行移位和列混合。
 * 最后，返回加密后的状态数据。
 *
 * @param[in,out] pState  状态数据（每个字节表示一个状态值）。
 *
 * @return 无返回值。
 */
static void block_encrypt(unsigned char *pState)
{
    unsigned char i;  // 循环索引

    // 添加初始轮密钥
    AddRoundKey(pState, g_roundKeyTable);
    
    // 进行 Nr 轮加密操作，i 从 1 到 Nr
    for (i = 1; i <= Nr; i++)  // i = [1, Nr]
    {
        // 替换字节
        sub_bytes(pState, 4*Nb, 0);
        
        // 行移位
        shift_rows(pState, 0);

        // 只有在最后一轮前，才进行列混合
        if (i != Nr)
        {
            mix_columns(pState, 0);
        }

        // 添加当前轮的轮密钥
        AddRoundKey(pState, &g_roundKeyTable[4*Nb*i]);
    }
    
    // 为了节省代码，合并到循环执行
    // sub_bytes(pState, 4*Nb);
    // shift_rows(pState, 0);
    // AddRoundKey(pState, &g_roundKeyTable[4*Nb*Nr]);
}


/**
 * @brief 对单块数据进行解密。
 *
 * 此函数实现 AES 解密算法的基本结构，对给定的状态数据进行解密处理。
 * 解密过程中包含多个步骤，包括轮密钥添加、行移位、字节替换和列混合。
 * 最后，返回解密后的状态数据。
 *
 * @param[in,out] pState  状态数据（每个字节表示一个状态值）。
 *
 * @return 无返回值。
 */
static void block_decrypt(unsigned char *pState)
{
    unsigned char i;  // 循环索引

    // 添加最后一轮的轮密钥
    AddRoundKey(pState, &g_roundKeyTable[4*Nb*Nr]);
    
    // 进行 Nr 轮解密操作，i 从 Nr 到 1
    for (i = Nr; i > 0; i--)  // i = [Nr, 1]
    {
        // 反向行移位
        shift_rows(pState, 1);
        
        // 反向替换字节
        sub_bytes(pState, 4*Nb, 1);
        
        // 添加当前轮的轮密钥
        AddRoundKey(pState, &g_roundKeyTable[4*Nb*(i-1)]);

        // 只有在最后一轮前，才进行列反混合
        if (i != 1)
        {
            mix_columns(pState, 1);
        }
    }
    
    // 为了节省代码，合并到循环执行
    // shift_rows(pState, 1);
    // sub_bytes(pState, 4*Nb, 1);
    // AddRoundKey(pState, g_roundKeyTable);
}


/**
 * @brief 初始化 AES 算法，执行扩展密钥操作。
 *
 * 此函数根据提供的原始密钥生成轮密钥表，轮密钥将在后续的加密和解密过程中使用。
 * 输入的原始密钥长度必须为 AES_KEY_LENGTH/8 字节，函数将根据 AES 标准扩展密钥。
 *
 * @param[in] pKey  指向原始密钥的指针，长度必须为 16、24 或 32 字节。
 *
 * @return 无返回值。
 */
void AES_Init(const void *pKey)
{
    // 扩展密钥
    unsigned char i;                        // 循环索引
    unsigned char *pRoundKey;               // 指向当前轮密钥的指针
    unsigned char Rcon[4] = {0x01, 0x00, 0x00, 0x00};  // 轮常量初始值

    // 将原始密钥复制到轮密钥表的开始部分
    memcpy(g_roundKeyTable, pKey, 4 * Nk);

    // 设置指向当前轮密钥的指针
    pRoundKey = &g_roundKeyTable[4 * Nk];

    // 生成从第 Nk 到 Nb*(Nr+1) 的轮密钥
    for (i = Nk; i < Nb * (Nr + 1); pRoundKey += 4, i++)
    {
        // 将前一个轮密钥复制到当前轮密钥
        memcpy(pRoundKey, pRoundKey - 4, 4);

        // 每 Nk 轮时进行特殊处理
        if (i % Nk == 0)
        {
            // 旋转当前轮密钥
            rotation_word(pRoundKey);
            // 对轮密钥进行字节替换
            sub_bytes(pRoundKey, 4, 0);
            // 将轮常量与当前轮密钥异或
            xor_bytes(pRoundKey, Rcon, 4);

            // 更新轮常量
            Rcon[0] = gf_mult_by02(Rcon[0]);
        }
        // 针对 Nk 大于 6 的情况进行字节替换
        else if (Nk > 6 && i % Nk == Nb)
        {
            sub_bytes(pRoundKey, 4, 0);
        }

        // 与 Nk 前的轮密钥异或，生成新的轮密钥
        xor_bytes(pRoundKey, pRoundKey - 4 * Nk, 4);
    }
}

/**
 * @brief AES加密函数，对输入的明文数据进行加密。
 *
 * 该函数可以在 ECB 或 CBC 模式下执行 AES 加密。对于 CBC 模式，需要提供初始化向量（IV），
 * 而 ECB 模式不需要 IV。在加密过程中，数据块会逐一加密，输出密文块。
 *
 * @param[in]  pPlainText   明文数据，需加密的数据，长度为 nDataLen 字节。
 * @param[out] pCipherText  密文数据，加密后的数据存储地址，允许与 pPlainText 相同。
 * @param[in]  nDataLen     数据长度，单位为字节，必须为 AES_KEY_LENGTH/8 的整数倍。
 * @param[in]  pIV          初始化向量（IV），如果使用 ECB 模式可以为 NULL。
 *
 * @return 无返回值。
 */
void AES_Encrypt(const unsigned char *pPlainText, unsigned char *pCipherText, 
                 unsigned int nDataLen, const unsigned char *pIV)
{
    unsigned int i;

    // 若密文和明文指针不同，将明文复制到密文存储区
    if (pPlainText != pCipherText)
    {
        memcpy(pCipherText, pPlainText, nDataLen);
    }

    // 分块加密，每个块大小为 4*Nb 字节
    for (i = nDataLen / (4 * Nb); i > 0; i--, pCipherText += 4 * Nb)
    {
        // 若使用 CBC 模式，进行块 XOR 操作
        #if AES_MODE == AES_MODE_CBC
            xor_bytes(pCipherText, pIV, 4 * Nb);
        #endif

        // 对当前块进行加密
        block_encrypt(pCipherText);

        // 更新 IV 为当前块密文，用于下一块的加密
        pIV = pCipherText;
    }
}


/**
 * @brief AES解密函数，对输入的密文数据进行解密。
 *
 * 该函数支持 ECB 和 CBC 模式解密。对于 CBC 模式，需提供初始化向量（IV）。
 * 在解密过程中，数据块会逆序逐一解密并恢复到原始明文数据。
 *
 * @param[in]  pCipherText  密文数据，需解密的数据，长度为 nDataLen 字节。
 * @param[out] pPlainText   明文数据，解密后的数据存储地址，可以与 pCipherText 相同。
 * @param[in]  nDataLen     数据长度，单位为字节，必须为 AES_KEY_LENGTH/8 的整数倍。
 * @param[in]  pIV          初始化向量（IV），如果使用 ECB 模式可以为 NULL。
 *
 * @return 无返回值。
 */
void AES_Decrypt(const unsigned char *pCipherText, unsigned char *pPlainText, 
                 unsigned int nDataLen, const unsigned char *pIV)
{
    unsigned int i;

    // 若密文和明文指针不同，将密文复制到明文存储区
    if (pPlainText != pCipherText)
    {
        memcpy(pPlainText, pCipherText, nDataLen);
    }

    // 设置指针从最后一个数据块开始解密，避免开辟额外空间保存 IV
    pPlainText += nDataLen - 4 * Nb;
    for (i = nDataLen / (4 * Nb); i > 0; i--, pPlainText -= 4 * Nb)
    {
        // 对当前数据块进行解密
        block_decrypt(pPlainText);

        // 若为 CBC 模式，对解密后的数据块进行 XOR 操作
        #if AES_MODE == AES_MODE_CBC
            if (i == 1)
            {
                // 第一块（初始块），与 IV 异或
                xor_bytes(pPlainText, pIV, 4 * Nb);
            }
            else
            {
                // 其余块，与前一个密文块异或
                xor_bytes(pPlainText, pPlainText - 4 * Nb, 4 * Nb);
            }
        #endif
    }
}





//******************************** Defines **********************************//






