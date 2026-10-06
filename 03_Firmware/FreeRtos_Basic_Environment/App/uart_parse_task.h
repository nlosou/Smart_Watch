#ifndef UART_PARSE_TASK_H_
#define UART_PARSE_TASK_H_





/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "queue.h"

/* Includes ------------------------------------------------------------------*/


extern QueueHandle_t queue_irq_rec_A;
extern TaskHandle_t Task_A;

void Uart_rec_A_task(void* argument);



#endif
