
/* USER CODE BEGIN Includes */

#include "bsp_uart_driver.h"
#include "usart.h"
#include "FreeRTOS.h"
#include "queue.h"
#include "elog.h"
#include "mid_ring_buffer.h"

/* USER CODE END Includes */


/*Define Macro*/
#define BUFFER_A                             (0)
#define BUFFER_B                             (1)
#define INTEERUPT_TO_FRONT_PARTERN      (0xAAAA)
#define FRONT_TO_BACKEND_PARTERN        (0xBBBB)



#define OPEN_SINGLE_BYTE_INTERRUPT           (0)
#define OPEN_UART_DMA_INTERRUPT              (1)

/*Define Macro*/


/* Globle Variables */


//AB-Buffer
#if 0
uint8_t flagSwapAB = BUFFER_A;
uint8_t g_buffer_A[1] = {0};
uint8_t g_buffer_B[1] = {0};
#endif

uint8_t                 g_buffer = 0;

QueueHandle_t queue_irq_front = NULL;
extern QueueHandle_t queue_front_to_backend;
/* Globle Variables */


/**
  * @brief  半满回调
  * @param  argument: Not used
  * @retval None
  */

void dma_half_callback(uint16_t size)
{
    /*
    elog_d("isr","dma half");
    //1,获取当前head
    uint32_t current_head;
    if(0x00 == get_current_head(&g_ring_buffer,&current_head))
    {
     //   elog_d("DMA_HALF","current head is [%d]",current_head);
    }
    //2,当前dma到达的位置(dma 会不断在 0~RING_BUFFER_SIZE 之间徘徊)
    uint32_t current_data_pos = RING_BUFFER_SIZE/ 2 ;
    //elog_d("DMA_HALF","current data pos is [%d]",current_data_pos);
    //3,计算需要让head移动多少
    uint32_t current_head_mod = current_head % (RING_BUFFER_SIZE);
    uint32_t need_move_len = 0 ;
    if(current_data_pos <current_head_mod)
    {
        need_move_len = current_data_pos  + RING_BUFFER_SIZE -current_head_mod ;
    }
    else
    {

        need_move_len = current_data_pos  - current_head_mod;
    
    }
    //elog_d("DMA_HALF","need_move_len is [%d]",need_move_len);
    //4,移动head
    if(0x00 == change_head(&g_ring_buffer,need_move_len))
    {
        
    }
    if(0x00 == get_current_head(&g_ring_buffer,&current_head))
    {
        //elog_d("DMA_HALF","After moving head is [%d]",current_head);
    }

    //通知前端函数,数据已经就绪
    uint32_t info_to_front = INTEERUPT_TO_FRONT_PARTERN;
    if(pdTRUE ==xQueueGenericSendFromISR(queue_irq_front,&info_to_front,NULL,queueOVERWRITE))
    {
        //elog_d("irq","irq to front function is successfully");
    }
    else
    {

        //elog_d("irq","irq to front function is falsed");
    }
    */

}


/**
  * @brief  全满回调
  * @param  argument: Not used
  * @retval None
  */

void dma_full_callback(uint16_t size)
{
    /*
    elog_d("isr","dma full");
    //1,获取当前head
    uint32_t current_head;
    if(0x00 == get_current_head(&g_ring_buffer,&current_head))
    {
      //  elog_d("DMA_FULL","current head is [%d]",current_head);
    }
    //2,当前dma到达的位置(dma 会不断在 0~RING_BUFFER_SIZE 之间徘徊)
    uint32_t current_data_pos = RING_BUFFER_SIZE ;
    //elog_d("DMA_FULL","current data pos is [%d]",current_data_pos);
    //3,计算需要让head移动多少
    uint32_t current_head_mod = current_head % (RING_BUFFER_SIZE);
    uint32_t need_move_len = 0 ;
    if(current_data_pos <current_head_mod)
    {
        need_move_len = current_data_pos  + RING_BUFFER_SIZE -current_head_mod ;
    }
    else
    {

        need_move_len = current_data_pos  - current_head_mod;
    
    }
    //elog_d("DMA_FULL","need_move_len is [%d]",need_move_len);
    //4,移动head
    if(0x00 == change_head(&g_ring_buffer,need_move_len))
    {
        
    }
    if(0x00 == get_current_head(&g_ring_buffer,&current_head))
    {
     //   elog_d("DMA_FULL","After moving head is [%d]",current_head);
    }

    //通知前端函数,数据已经就绪
    uint32_t info_to_front = INTEERUPT_TO_FRONT_PARTERN;
    if(pdTRUE ==xQueueGenericSendFromISR(queue_irq_front,&info_to_front,NULL,queueOVERWRITE))
    {
        //elog_d("irq","irq to front function is successfully");
    }
    else
    {

        //elog_d("irq","irq to front function is falsed");
    }
    */
}

/**
  * @brief  空闲回调
  * @param  argument: Not used
  * @retval None
  */
void uart_idle_callback(uint16_t size)
{
    /*
    elog_d("isr","uart idle");
    //1,获取当前head
    uint32_t current_head;
    if(0x00 == get_current_head(&g_ring_buffer,&current_head))
    {
        //elog_d("UART_IDLE","current head is [%d]",current_head);
    }
    //2,当前dma到达的位置(dma 会不断在 0~RING_BUFFER_SIZE 之间徘徊)
    uint32_t current_data_pos = size;
    //elog_d("UART_IDLE","current data pos is [%d]",current_data_pos);

    //3,计算需要让head移动多少
    uint32_t current_head_mod = current_head % (RING_BUFFER_SIZE);
    uint32_t need_move_len = 0 ;
    if(current_data_pos <current_head_mod)
    {
        need_move_len = current_data_pos  + RING_BUFFER_SIZE -current_head_mod ;
    }
    else
    {

        need_move_len = current_data_pos  - current_head_mod;
    
    }
    //elog_d("UART_IDLE","need_move_len is [%d]",need_move_len);

    //4,移动head
    if(0x00 == change_head(&g_ring_buffer,need_move_len))
    {
        
    }
    if(0x00 == get_current_head(&g_ring_buffer,&current_head))
    {
        //elog_d("UART_IDLE","After moving head is [%d]",current_head);
    }
    */

}
