#include <msp430x14x.h>
#include "hal_types.h"

// ʹ��Timer_Aʵ����ʱ��n (us΢��)
void _delayus(uint16 n)
{
  CCR0 = n; //��Ҫ��ʱ��ʱ�� CCR0ΪTACCR0
  TACTL |= MC_1; //Timer_A��ʱģʽѡ��UP����ʱ����0������TACCR0��ֵ�󴥷��жϣ�Ȼ�����´�0����
  while(!(TACTL & BIT0)); //ʹ�ò�ѯ��ʽ���TACTL��0λ���жϱ�־λTAIFG
  TACTL &= ~MC_1; //����MCλ��ʹTimer_A����STOPģʽ
  TACTL &= ~BIT0; //����жϱ�־λ
}
