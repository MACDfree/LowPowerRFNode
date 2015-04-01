#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_clock.h"

// ��ʼ��ʱ��Դ
void initClock(void)
{
  uint16 i;
  BCSCTL1 &= ~XT2OFF; //��XT2��Ƶ��������
  do
  {
    IFG1 &= ~OFIFG; //�������ʧ�ܱ�־
    for(i = 0xFF; i > 0; i--); //�ȴ�8MHz��������
  } while(IFG1 & OFIFG); //����ʧЧ��־��Ȼ����
  BCSCTL2 |= SELM_2 + SELS; //MCLK��SMCLKѡ���Ƶ����
}