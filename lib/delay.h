#ifndef delay_h
#define delay_h

#include "hal_types.h"

#define INIT_TIMER_A(n) CCR0=(n);TACTL|=TASSEL_2+ID_3;nms=0;CCTL0|=CCIE
#define START_TIMER_A TACTL |= MC_1
#define STOP_TIMER_A TACTL &= ~MC_1; TACTL &= ~TAIE; TACTL &= ~BIT0

extern uint16 nms;

void _delayus(uint16 n);//ÑÓÊ±º¯Êý£¬n us

#endif