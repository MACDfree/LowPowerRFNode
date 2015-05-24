#include <msp430x14x.h>
#include "hal_types.h"
#include "delay.h"

// ʹ��Timer_Aʵ����ʱ��n (us΢��)
void _delayus(uint16 n)
{
  CCR0 = n; // ��Ҫ��ʱ��ʱ�� CCR0ΪTACCR0
  TACTL |= TASSEL_2 + ID_3; // Timer_A��ʱ��ԴΪSMCLK��8��Ƶ�����ڶ�ʱ�������û��д����� 2015-03-22 21:44��
  TACTL |= MC_1; // Timer_A��ʱģʽѡ��UP����ʱ����0������TACCR0��ֵ�󴥷��жϣ�Ȼ�����´�0����
  while(!(TACTL & BIT0)); //ʹ�ò�ѯ��ʽ���TACTL��0λ���жϱ�־λTAIFG
  TACTL &= ~MC_1; // ����MCλ��ʹTimer_A����STOPģʽ
  TACTL &= ~BIT0; // ����жϱ�־λ
}

//void delayusStart(uint16 n)
//{
//  CCR0 = n; // ��Ҫ��ʱ��ʱ�� CCR0ΪTACCR0
//  TACTL |= TASSEL_2 + ID_3; // Timer_A��ʱ��ԴΪSMCLK��8��Ƶ�����ڶ�ʱ�������û��д����� 2015-03-22 21:44��
//  TACTL |= MC_1; // Timer_A��ʱģʽѡ��UP����ʱ����0������TACCR0��ֵ�󴥷��жϣ�Ȼ�����´�0����
//  while(!(TACTL & BIT0)); //ʹ�ò�ѯ��ʽ���TACTL��0λ���жϱ�־λTAIFG
//  TACTL &= ~MC_1; // ����MCλ��ʹTimer_A����STOPģʽ
//  TACTL &= ~BIT0; // ����жϱ�־λ
//}
