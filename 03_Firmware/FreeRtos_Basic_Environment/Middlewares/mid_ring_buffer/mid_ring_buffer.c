/* Includes ------------------------------------------------------------------*/
#include "mid_ring_buffer.h"
#include "FreeRTOS.h"
#include "elog.h"
#include <string.h>
/* Includes ------------------------------------------------------------------*/



/**
  * @brief  创建一个100长度的环形缓冲区
  * @param  argument: void
  * @retval NULL:创建失败
  *
  */
ring_buffer_t* create_empty_rintg_buffer(void)
{

    //1.创建指针并申请内存
    ring_buffer_t* p_ring_buffer = (ring_buffer_t*)pvPortMalloc(
                        sizeof(ring_buffer_t)*RING_BUFFER_SIZE);

    //2.判断申请内存是否成功
    if(NULL == p_ring_buffer)
    {
        return NULL;
    }

    //3.初始化数据
    memset(p_ring_buffer,0,sizeof(ring_buffer_t));

    //4.f返回成功创建的p_ring_buffer
    return p_ring_buffer;
}


/**
  * @brief  判断一个环形缓冲区是否是满的
  * @param  p_ring_buffer: ring_buffer_t*
  * @retval 0x00:环形缓冲满了
  *         0x01:环形缓冲没有满
  *         0xFF:缓冲区不存在
  */
uint8_t ring_buffer_is_full(ring_buffer_t* p_ring_buffer)
{
    //1.拿到一个指针需要进行判空
    if(NULL == p_ring_buffer)
    {
        return 0xFF;
    }
    
    //2.如果环形缓冲区的head+ 1 等于tail,就说明环形缓存区就满了
    if(((p_ring_buffer->head + 1) % RING_BUFFER_SIZE) == (p_ring_buffer->tail % RING_BUFFER_SIZE))
    {
        return 0x00;
    }
    else
    {
        return 0x01;
    }
}

/**
  * @brief  判断一个环形缓冲区是否是空的
  * @param  p_ring_buffer: ring_buffer_t*
  * @retval 0x00:环形缓冲空了
  *         0x01:环形缓冲不为空
  *         0xFF:缓冲区不存在
  */
uint8_t ring_buffer_is_empty(ring_buffer_t* p_ring_buffer)
{
    //1.拿到一个指针需要进行判空
    if(NULL == p_ring_buffer)
    {
        return 0xFF;
    }
    
    //2.如果环形缓冲区的head 等于tail,就说明环形缓存区就空了
    if((p_ring_buffer->head) == (p_ring_buffer->tail))
    {
        return 0x00;
    }
    else
    {
        return 0x01;
    }
}

/**
  * @brief  往环形缓冲区插入数据
  * @param  p_ring_buffer: ring_buffer_t*
  *         data:需要插入类型为data_type_t类型的数据
  * @retval 0x00:插入成功
  *         0xFF:环形缓冲区为满
  *         0xFE:缓冲区不存在
  */
uint8_t         insert_data(ring_buffer_t* p_ring_buffer,data_type_t data)
{
    //1.拿到一个指针需要进行判空
    if(NULL == p_ring_buffer)
    {
        return 0xFE;
    }

    //2.判断当前的环形缓冲区是否为满
    if(0x00 == ring_buffer_is_full(p_ring_buffer))
    {
        return 0xFF;
    }
    //3.插入数据
    p_ring_buffer->ring_buffer[p_ring_buffer->head % RING_BUFFER_SIZE] = data;
    p_ring_buffer->head++;
    return 0x00;
}

/**
  * @brief  从缓存区里获取数据
  * @param  p_ring_buffer: ring_buffer_t*
  *         data:需要插入类型为data_type_t*类型的数据
  * @retval 0x00:获取成功
  *         0xFF:环形缓冲区为空
  *         0xFE:缓冲区不存在
  */
uint8_t            get_data(ring_buffer_t* p_ring_buffer,data_type_t* data)
{
    //1.拿到一个指针需要进行判空
    if(NULL == p_ring_buffer)
    {
        return 0xFE;
    }

    //2.判断当前的环形缓冲区是否为满
    if(0x00 == ring_buffer_is_empty(p_ring_buffer))
    {
        return 0xFF;
    }
    //3.获取数据
    *data = p_ring_buffer->ring_buffer[p_ring_buffer->tail % RING_BUFFER_SIZE];
    p_ring_buffer->tail++;
    return 0x00;
}

/**
  * @brief  获取当前head
  * @param  p_ring_buffer: ring_buffer_t*
  * @param  *head:获取当前head
  * @retval 0x00:获取成功
  *         0xFF:环形缓冲区为空
  *         0xFE:缓冲区不存在
  */
uint8_t            get_current_head(ring_buffer_t* p_ring_buffer,uint32_t* head)
{
    //1.拿到一个指针需要进行判空
    if(NULL == p_ring_buffer)
    {
        return 0xFE;
    }
    //3.获取数据
    *head= p_ring_buffer->head;
    return 0x00;
}

/**
  * @brief  改变当前head
  * @param  p_ring_buffer: ring_buffer_t*
  * @retval 0x00:获取成功
  *         0xFF:环形缓冲区为空
  *         0xFE:缓冲区不存在
  */
uint8_t            change_head(ring_buffer_t* p_ring_buffer,uint32_t len)
{
    //1.拿到一个指针需要进行判空
    if(NULL == p_ring_buffer)
    {
        return 0xFE;
    }
    //3.获取数据
    p_ring_buffer->head+=len;
    return 0x00;
}



