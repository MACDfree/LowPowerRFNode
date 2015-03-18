#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_spi.h"

void halSpiInit(void)
{
  // 进入reset状态
  U0CTL = SWRST;

  // 8位数据 SPI主机 3线制, 使用SMCLK
  U0CTL  |= CHAR + SYNC + MM;
  U0TCTL |= CKPH + SSEL1 + SSEL0 + STC;

  // 时钟源2分频
  U0BR0  = 0x02;
  U0BR1  = 0x00;
  U0MCTL = 0x00;

  // 使能SPI
  ME1 |= USPIE0;

  P3DIR |= (BIT0 + BIT1 + BIT3); // P3.0 1 3 设为输出 CS SI SCLK
  P3DIR &= ~BIT2; // P3.2 设为输入 SO
  P3SEL |= BIT1 + BIT2 + BIT3; // 引脚特殊功能设置
  // 退出reset状态
  U0CTL &= ~SWRST;
}

uint8 halSpiWrite(uint8 addr, const uint8 *data, uint8 length)
{
  uint8 rc, i;
  CSN0;
  while(SOMI);
  U0TXBUF = addr;
  while(!(U0TCTL & TXEPT));//等待TXBUF空
  rc = U0RXBUF;
  for(i=0; i<length; i++)
  {
    U0TXBUF = data[i];
    while(!(U0TCTL & TXEPT));//等待TXBUF空
  }
  CSN1;
  return rc;
}

uint8 halSpiRead(uint8 addr, uint8 *data, uint8 length)
{
  uint8 rc, i;
  CSN0;
  while(SOMI);
  U0TXBUF = addr;
  while(!(U0TCTL & TXEPT));//等待TXBUF空
  rc = U0RXBUF;
  for(i=0; i<length; i++)
  {
      U0TXBUF = 0;
      while(!(U0TCTL & TXEPT));//等待TXBUF空
      data[i] = U0RXBUF;
  }
  CSN1;
  return rc;
}

uint8 halSpiStrobe(uint8 cmd)
{
    uint8 rc;
    CSN0;
    while(SOMI);
    U0TXBUF = cmd;
    while(!(U0TCTL & TXEPT));//等待TXBUF空
    rc = U0RXBUF;
    CSN1;
    return rc;
}