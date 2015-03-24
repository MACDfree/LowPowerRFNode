#ifndef hal_uart_h
#define hal_uart_h

#include "hal_types.h"

void halUartInit(void); // 串口1初始化（串口0和串口1）
void halUartWrite(const uint8* buf); // 串口发送
void halUartRead(uint8* buf);

#endif