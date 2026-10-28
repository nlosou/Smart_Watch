#ifndef AES_H_WSTZU85R
#define AES_H_WSTZU85R


/***********************include****************************/
#include <stddef.h>
#include <string.h>
/***********************include****************************/


/***********************enum****************************/
//定义AES密钥长度的枚举
typedef enum {
    AES_KEY_LENGTH_128 = 128,
    AES_KEY_LENGTH_192 = 192,
    AES_KEY_LENGTH_256 = 256,
}AES_Key_Length;


//定义加密模式
typedef enum {
    AES_MODE_ECB = 0,
    AES_MODE_CBC = 1,
}AES_Mode;

/***********************enum****************************/


/***********************define****************************/
#define AES_MODE          AES_MODE_CBC
#define AES_KEY_LENGTH    AES_KEY_LENGTH_128
/***********************define****************************/



/**
 * @brief 
 *
 * @param pkey
 */
void AES_Init(const void *pkey);


/**
 * @brief 
 *
 * @param pPlainText
 * @param pCipherText
 * @param nDataLen
 * @param 
 */
void AES_Encrypt(const unsigned char *pPlainText,
        unsigned char *pCipherText,
        unsigned int nDataLen,
        const unsigned char *pIV
        );

/**
 * @brief 
 *
 * @param pCipherText
 * @param pPlainText
 * @param nDataLen
 * @param 
 */
void AES_Decrypt(const unsigned char *pCipherText,
        unsigned char *pPlainText,
        unsigned int nDataLen,
        const unsigned char *pIV
        );


#endif /* end of include guard: AES_H_WSTZU85R */
