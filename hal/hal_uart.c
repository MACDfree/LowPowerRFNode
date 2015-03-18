#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_uart.h"

uint8 rxbuf[10];
uint8 index=0;

void halUartInit(void)
{
  UCTL1 |= SWRST;
  P3SEL |= (BIT6 + BIT7);
  ME2 |= UTXE1 + URXE1;
  UCTL1 |= CHAR;
  UTCTL1 |= SSEL0;
  UBR01 = 0x03;
  UBR11 = 0x00;
  UMCTL1 = 0x4a;
  UCTL1 &= ~SWRST;
  IE2 |= URXIE1;
}

void halUartWrite(const uint8* buf)
{
  uint8 i=0;
  while(buf[i]!='\0')
  {
    while(!(IFG2 & UTXIFG1));
    TXBUF1 = buf[i++];
  }
}

void halUartRead(uint8* buf)
{}

#pragma vector=UART1RX_VECTOR
__interrupt void usart1_rx(void)
{
  index = index % 10;
  rxbuf[index] = RXBUF1;
}