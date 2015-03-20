#ifndef hal_uart_h
#define hal_uart_h

#include "hal_types.h"

void halUartInit(void);
void halUartWrite(const uint8* buf);
void halUartRead(uint8* buf);

#endif