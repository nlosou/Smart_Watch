
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
ring_buffer_t*  g_ring_buffer = NULL;

QueueHandle_t queue_irq_front = NULL;
extern QueueHandle_t queue_front_to_backend;
/* Globle Variables */

void uart_driver_fucn(void* argument)
{   
    uint32_t receive_parten = 0;
    uint32_t front_to_backend_parten = FRONT_TO_BACKEND_PARTERN;

    ring_buffer_t*  ring_buffer = NULL;
    ring_buffer = create_empty_rintg_buffer();
    g_ring_buffer = ring_buffer;

    queue_irq_front = xQueueCreate(1,4); 
    if(NULL!=queue_irq_front )
    {

        elog_i("Front","queue_irq_front is created successfully");
    }
    else
    {

        elog_e("Front","queue_irq_front is created false");
        return;
    }

    if(NULL == ring_buffer)
    {
        
        elog_e("ERROR","ring buffer is created falsed");
    }
    else
    {

        elog_i("INFO","ring buffer is created successfully");
    }

#if OPEN_SINGLE_BYTE_INTERRUPT
    if(HAL_OK == HAL_UART_Receive_IT(&huart1,&g_buffer,1))
    {
        elog_i("INFO","Uart  rx interrupt is successfully");
    }
    else
    {

        elog_e("INFO","Uart rx interrupt is false");
        return;
    }
#endif

#if OPEN_UART_DMA_INTERRUPT
    if(HAL_OK ==HAL_UARTEx_ReceiveToIdle_DMA(&huart1,g_ring_buffer->ring_buffer,RING_BUFFER_SIZE))
    {
        elog_i("INFO","Uart DMA  rx interrupt is successfully");
    }
    else
    {

        elog_e("INFO","Uart DMA rx interrupt is false");
        return;
    }
#endif

//test ring_buffer Function
#if 0
    ring_buffer_t*  ring_buffer = NULL;
    ring_buffer = create_empty_rintg_buffer();
    if(NULL == ring_buffer)
    {
        
        elog_e("ERROR","ring buffer is created falsed");
    }
    else
    {

        elog_i("INFO","ring buffer is created successfully");
    }
    if(0x00 == ring_buffer_is_empty(ring_buffer))
    {

        elog_i("INFO","ring buffer is empty");
    }
    else
    {

        elog_i("INFO","ring buffer is not  empty");
    }
    if(0x00 == insert_data(ring_buffer,0x23))
    {
        
        elog_i("INFO","insert data is successfully");
    }
    else
    {

        elog_e("ERROR","insert data is falsed");
    }
    data_type_t temp_data ;
    uint8_t ret;
    ret = get_data(ring_buffer,&temp_data);
    if(0x00 == ret)
    {
        
        elog_i("INFO","get_data is %x",temp_data);
    }
    else
    {

        elog_e("ERROR","get_data  is falsed: ERROR CODE is [%x]",ret);
    }
    if(0x00 == ring_buffer_is_empty(ring_buffer))
    {

        elog_i("INFO","ring buffer is empty");
    }
    else
    {

        elog_i("INFO","ring buffer is not  empty");
    }
#endif
        for(;;)
    {
                
        //1.接受中断发过来的队列
        if(pdTRUE == xQueueReceive(
                    queue_irq_front,
                    &receive_parten,
                    portMAX_DELAY))
        {
            elog_d("front","queue_irq_front is comming [%x]",receive_parten);
            //2.通知后端程序开始处理数据
            if(pdTRUE ==xQueueGenericSend(queue_front_to_backend,&front_to_backend_parten,0,queueOVERWRITE))
            {
                elog_d("front","front to backend function is successfully");
            }
            else
            {

                elog_d("front","front to backend function is falsed");
            }
        }

    }
}


/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
 //   elog_i("Inteerupt","uart interrupt is come");
//AB-Buffer
#if 0
    if(BUFFER_A == flagSwapAB)
    {
        flagSwapAB = BUFFER_B;
        elog_i("Buffer_A","Buffer_A is %x",g_buffer_A[0]);
        if(HAL_OK == HAL_UART_Receive_IT(&huart1,g_buffer_B,1))
        {
            elog_i("INFO","Uart rx interrupt is successfully");
        }
        else
        {

            elog_e("INFO","Uart rx interrupt is false");
            return;
        }           
    }
    else
    {
        flagSwapAB = BUFFER_A;
        elog_i("Buffer_B","Buffer_B is %x",g_buffer_B[0]);
        if(HAL_OK == HAL_UART_Receive_IT(&huart1,g_buffer_A,1))
        {
            elog_i("INFO","Uart rx interrupt is successfully");
        }
        else
        {
            elog_e("INFO","Uart rx interrupt is false");
            return;
        }
    }
#endif

//无log的ring buffer
#if 1
    //1.将获得的数据存入环形缓冲区
    if(0x00 == insert_data(g_ring_buffer,g_buffer))
    {
        
        //elog_i("INFO","insert data is successfully");
    }
    else
    {

       // elog_e("ERROR","insert data is falsed");
    }   
    //2,通知前端函数,数据已经就绪
    
    uint32_t info_to_front = INTEERUPT_TO_FRONT_PARTERN;
    if(pdTRUE ==xQueueGenericSendFromISR(queue_irq_front,&info_to_front,NULL,queueOVERWRITE))
    {
        //elog_d("irq","irq to front function is successfully");
    }
    else
    {

        //elog_d("irq","irq to front function is falsed");
    }

    //3,开启下一次中断接受
    if(HAL_OK == HAL_UART_Receive_IT(&huart1,&g_buffer,1))
    {
        //elog_i("INFO","Start new data receive");
    }
    else
    {

        //elog_e("ERROR","new data receive is falsed");
        return;
    }
#endif


//有log的ring buffer
#if 0
    //1.将获得的数据存入环形缓冲区
    if(0x00 == insert_data(g_ring_buffer,g_buffer))
    {
        
        elog_i("INFO","insert data is successfully");
    }
    else
    {

        elog_e("ERROR","insert data is falsed");
    }   
    //2,通知前端函数,数据已经就绪
    
    uint32_t info_to_front = INTEERUPT_TO_FRONT_PARTERN;
    if(pdTRUE ==xQueueGenericSendFromISR(queue_irq_front,&info_to_front,NULL,queueOVERWRITE))
    {
        elog_d("irq","irq to front function is successfully");
    }
    else
    {

        elog_d("irq","irq to front function is falsed");
    }

    //3,开启下一次中断接受
    if(HAL_OK == HAL_UART_Receive_IT(&huart1,&g_buffer,1))
    {
        elog_i("INFO","Start new data receive");
    }
    else
    {

        elog_e("ERROR","new data receive is falsed");
        return;
    }
#endif

}

/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef* huart1,uint16_t size)
{

    elog_d("irq","dma half-full full come");
}



ring_buffer_t* uart_driver_get_ring_buffer_address(void)
{
    if(NULL== g_ring_buffer)
    {
        return NULL;
    }
    return g_ring_buffer;
    
}


/**
  * @brief  半满回调
  * @param  argument: Not used
  * @retval None
  */

void dma_half_callback(uint16_t size)
{
    //1,获取当前head
    uint32_t current_head;
    if(0x00 == get_current_head(g_ring_buffer,&current_head))
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
    if(0x00 == change_head(g_ring_buffer,need_move_len))
    {
        
    }
    if(0x00 == get_current_head(g_ring_buffer,&current_head))
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
}


/**
  * @brief  全满回调
  * @param  argument: Not used
  * @retval None
  */

void dma_full_callback(uint16_t size)
{
    //1,获取当前head
    uint32_t current_head;
    if(0x00 == get_current_head(g_ring_buffer,&current_head))
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
    if(0x00 == change_head(g_ring_buffer,need_move_len))
    {
        
    }
    if(0x00 == get_current_head(g_ring_buffer,&current_head))
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
}

/**
  * @brief  空闲回调
  * @param  argument: Not used
  * @retval None
  */
void uart_idle_callback(uint16_t size)
{
    //1,获取当前head
    uint32_t current_head;
    if(0x00 == get_current_head(g_ring_buffer,&current_head))
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
    if(0x00 == change_head(g_ring_buffer,need_move_len))
    {
        
    }
    if(0x00 == get_current_head(g_ring_buffer,&current_head))
    {
        //elog_d("UART_IDLE","After moving head is [%d]",current_head);
    }
}

