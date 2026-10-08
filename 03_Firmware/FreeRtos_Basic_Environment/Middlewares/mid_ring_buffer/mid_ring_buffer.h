#ifndef MID_RING_BUFFER_H
#define MID_RING_BUFFER_H


/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
/* Includes ------------------------------------------------------------------*/

/* Define macro ------------------------------------------------------------*/

#define RING_BUFFER_SIZE 100
typedef uint8_t data_type_t;
/* Define macro ------------------------------------------------------------*/

typedef struct {
    data_type_t ring_buffer[RING_BUFFER_SIZE];
    uint32_t                             head;
    uint32_t                             tail;
}ring_buffer_t;


/**
  * @brief  创建一个100长度的环形缓冲区
  * @param  argument: void
  * @retval NULL:创建失败
  *
  */
ring_buffer_t*                             create_empty_rintg_buffer(void);

/**
  * @brief  判断一个环形缓冲区是否是空的
  * @param  argument: void
  * @retval 0x00:环形缓冲满了
  *         0x01:环形缓冲没有满
  *         0xFF:缓冲区不存在
  */
uint8_t                  ring_buffer_is_full(ring_buffer_t* p_ring_buffer);

/**
  * @brief  判断一个环形缓冲区是否是空的
  * @param  argument: void
  * @retval 0x00:环形缓冲空了
  *         0x01:环形缓冲不为空
  *         0xFF:缓冲区不存在
  */
uint8_t                 ring_buffer_is_empty(ring_buffer_t* p_ring_buffer);



/**
  * @brief  从缓存区里获取数据
  * @param  p_ring_buffer: ring_buffer_t*
  *         data:需要插入类型为data_type_t*类型的数据
  * @retval 0x00:获取成功
  *         0xFF:环形缓冲区为空
  *         0xFE:缓冲区不存在
  */
uint8_t            get_data(ring_buffer_t* p_ring_buffer,data_type_t* data);



/**
  * @brief  往环形缓冲区插入数据
  * @param  p_ring_buffer: ring_buffer_t*
  *         data:需要插入类型为data_type_t类型的数据
  * @retval 0x00:插入成功
  *         0xFF:环形缓冲区为满
  *         0xFE:缓冲区不存在
  */
uint8_t         insert_data(ring_buffer_t* p_ring_buffer,data_type_t data);


#endif



