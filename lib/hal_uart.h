#ifndef hal_uart_h
#define hal_uart_h

#include "hal_types.h"

void halUartInit(void); // ����1��ʼ��������0�ʹ���1��
void halUartWrite(const uint8* buf); // ���ڷ���
void halUartRead(uint8* buf);

#endif