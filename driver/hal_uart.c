#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_uart.h"

uint8 rxbuf[10]; // 串口接收缓冲区
uint8 index=0; // 缓冲区指针

// 串口1初始化（串口0和串口1）
void halUartInit(void)
{
  UCTL1 |= SWRST; // SWRST置1，进入设置模式
  P3SEL |= (BIT6 + BIT7); // 设置引脚特殊功能
  ME2 |= UTXE1 + URXE1; // 使能串口1接收和发送
  UCTL1 |= CHAR; // 设置字符长度为8位
  UTCTL1 |= SSEL0; // 时钟源选择辅助时钟
  // 波特率设置9200
  UBR01 = 0x03;
  UBR11 = 0x00;
  UMCTL1 = 0x4a;
  UCTL1 &= ~SWRST; // 启动串口
  IE2 |= URXIE1; // 打开串口接收中断
}

// 串口发送
void halUartWrite(const uint8* buf)
{
  uint8 i=0;
  while(buf[i]!='\0')
  {
    while(!(IFG2 & UTXIFG1)); // 查询法发送数据
    TXBUF1 = buf[i++];
  }
}

// 串口接收（没用到）
void halUartRead(uint8* buf)
{}

#pragma vector=UART1RX_VECTOR
__interrupt void usart1_rx(void)
{
  index = index % 10;
  rxbuf[index] = RXBUF1;
}