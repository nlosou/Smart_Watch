#ifndef BSP_UART_DRIVER_H
#define BSP_UART_DRIVER_H

#include "mid_ring_buffer.h"

void uart_driver_fucn(void* argument);
ring_buffer_t* uart_driver_get_ring_buffer_address(void);

#endif
