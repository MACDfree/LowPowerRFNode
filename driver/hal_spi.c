#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_spi.h"

void halSpiInit(void)
{
  // ����reset״̬
  U0CTL = SWRST;

  // 8λ���� SPI���� 3����, ʹ��SMCLK
  U0CTL  |= CHAR + SYNC + MM;
  U0TCTL |= CKPH + SSEL1 + SSEL0 + STC;

  // ʱ��Դ2��Ƶ
  U0BR0  = 0x02;
  U0BR1  = 0x00;
  U0MCTL = 0x00;

  // ʹ��SPI
  ME1 |= USPIE0;

  P3DIR |= (BIT0 + BIT1 + BIT3); // P3.0 1 3 ��Ϊ��� CS SI SCLK
  P3DIR &= ~BIT2; // P3.2 ��Ϊ���� SO
  P3SEL |= BIT1 + BIT2 + BIT3; // �������⹦������
  // �˳�reset״̬
  U0CTL &= ~SWRST;
}

uint8 halSpiWrite(uint8 addr, const uint8 *data, uint8 length)
{
  uint8 rc, i;
  CSN0;
  while(SOMI);
  U0TXBUF = addr;
  while(!(U0TCTL & TXEPT));//�ȴ�TXBUF��
  rc = U0RXBUF;
  for(i=0; i<length; i++)
  {
    U0TXBUF = data[i];
    while(!(U0TCTL & TXEPT));//�ȴ�TXBUF��
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
  while(!(U0TCTL & TXEPT));//�ȴ�TXBUF��
  rc = U0RXBUF;
  for(i=0; i<length; i++)
  {
      U0TXBUF = 0;
      while(!(U0TCTL & TXEPT));//�ȴ�TXBUF��
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
    while(!(U0TCTL & TXEPT));//�ȴ�TXBUF��
    rc = U0RXBUF;
    CSN1;
    return rc;
}