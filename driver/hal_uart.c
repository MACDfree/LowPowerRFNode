#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_uart.h"

uint8 rxbuf[10]; // ���ڽ��ջ�����
uint8 index=0; // ������ָ��

// ����1��ʼ��������0�ʹ���1��
void halUartInit(void)
{
  UCTL1 |= SWRST; // SWRST��1����������ģʽ
  P3SEL |= (BIT6 + BIT7); // �����������⹦��
  ME2 |= UTXE1 + URXE1; // ʹ�ܴ���1���պͷ���
  UCTL1 |= CHAR; // �����ַ�����Ϊ8λ
  UTCTL1 |= SSEL0; // ʱ��Դѡ����ʱ��
  // ����������9200
  UBR01 = 0x03;
  UBR11 = 0x00;
  UMCTL1 = 0x4a;
  UCTL1 &= ~SWRST; // ��������
  IE2 |= URXIE1; // �򿪴��ڽ����ж�
}

// ���ڷ���
void halUartWrite(const uint8* buf)
{
  uint8 i=0;
  while(buf[i]!='\0')
  {
    while(!(IFG2 & UTXIFG1)); // ��ѯ����������
    TXBUF1 = buf[i++];
  }
}

// ���ڽ��գ�û�õ���
void halUartRead(uint8* buf)
{}

#pragma vector=UART1RX_VECTOR
__interrupt void usart1_rx(void)
{
  index = index % 10;
  rxbuf[index] = RXBUF1;
}