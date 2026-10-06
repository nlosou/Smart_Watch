/* USER CODE BEGIN Includes */
#include "uart_parse_task.h"
#include "elog.h"
/* USER CODE END Includes */


/* Globle Variables */
QueueHandle_t queue_irq_rec_A = NULL;
uint8_t  bufferA[1] = {0};
uint8_t  bufferB[1] = {0};

/* Globle Variables */


/**
  * @brief  Function implementing the Task_A thread.
  * @param  argument: Not used
  * @retval None
  */
void Uart_rec_A_task(void* argument)
{
    queue_irq_rec_A = xQueueCreate(1,4); 
    if(NULL!=queue_irq_rec_A)
    {

        elog_i("Task_A","queue_irq_rec_A is created successfully");
    }
    else
    {

        elog_e("Task_A","queue_irq_rec_A is created false");
    }
    elog_i("Task_A","hello task a");
    uint32_t receive_data = 0;
    for(;;)
    {
        xQueueReceive(queue_irq_rec_A,&receive_data,portMAX_DELAY);

        elog_i("Task_A","receive_data is %x",receive_data);

    }
}
