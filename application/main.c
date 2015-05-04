#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_spi.h"
#include "hal_rf.h"
#include "hal_cc1101.h"
#include "rf_setting.h"
#include "hal_ds18b20.h"
#include "delay.h"
#include "hal_clock.h"

// �������������ʶ
/*
�ڵ㹤��ģʽ��Ϊ�㼯�ڵ㣨MODE_HJ���Ͳɼ��ڵ㣨MODE_CJ��
MODE_TESTΪ���Ա�־
���������Ա�־����MODE_OLED��MODE_UARTҲ��Ҫ���壨���߿�ͬʱ���壩
*/
#define MODE_TEST // ����ģʽ
//#define MODE_HJ // �㼯�ڵ�ģʽ
#define MODE_CJ // �ɼ��ڵ�ģʽ
#define MODE_OLED // ʹ��OLED
//#define MODE_UART // ʹ�ô���

// ����ڵ������Ϣ
#define ADDR_HJ 10 // �㼯�ڵ��ַΪ10
#define ADDR_CJ1 11 // �ɼ��ڵ�1��ַ
#define ADDR_CJ2 12 // �ɼ��ڵ�2��ַ
#define ADDR_CJ3 13 // �ɼ��ڵ�3��ַ
#define ADDR_CJ4 14 // �ɼ��ڵ�4��ַ
#define ADDR_CJ 11 // �ɼ��ڵ��ַ
#define ADDR_B 0 // �㲥��ַΪ0
#define KEY_H 111 // ��Կ��8λ
#define KEY_L 111 // ��Կ��8λ

#ifdef MODE_HJ
#define LEN 7
#endif

#ifdef MODE_CJ
#define LEN 4
#endif


#ifdef MODE_UART
#include "hal_uart.h"
#endif

#ifdef MODE_OLED
#include "hal_oled.h"
#endif

uint8 status = 0; // ��¼�ڵ㹤��״̬
uint8 isSent = 0; // �Ƿ�����ɱ�־
uint8 isReceived = 0; // �Ƿ������ɱ�־

#ifdef MODE_CJ
uint8 len = 0; // �ж���ʹ���Լ�¼���ݰ�����
uint8 stat = 0; // �ж���ʹ���Լ�¼����״̬
uint8 pak[5] = {0}; // �ж���ʹ���Խ��ջ������ݰ�
#endif

#ifdef MODE_TEST

uint8 test[9]; // ��ŵ��������Ϣ
//2���� out: 9
void itob(uint8 t, uint8 *out)
{
  uint8 i;
  for(i=0; i<8; i++)
  {
    if(t & 0x80)
    {
      out[i] = '1';
    }
    else
    {
      out[i] = '0';
    }
    t <<= 1;
  }
  out[8] = '\0';
}

//16���� out: 5
void itoh(uint8 t, uint8 *out)
{
  uint8 temp;
  out[0] = '0';
  out[1] = 'x';
  temp = (t & 0xf0) >> 4;
  if(temp<10)
  {
    out[2] = temp + '0';
  }
  else
  {
    out[2] = (temp-10) + 'A';
  }
  temp = t & 0x0f;
  if(temp<10)
  {
    out[3] = temp + '0';
  }
  else
  {
    out[3] = (temp-10) + 'A';
  }
  out[4] = '\0';
}

//10���� out: 4
void itoo(uint8 t, uint8 *out)
{
  out[0] = t / 100 + '0';
  t = t % 100;
  out[1] = t / 10 + '0';
  out[2] = t % 10 + '0';
  out[3] = '\0';
}
#endif

// ����IO�����жϹ���
void ioInit(void)
{
  P1DIR &= ~BIT5;
  P1SEL &= ~BIT5;
  P1IES |= BIT5;
  P1IE |= BIT5;
  P1IFG = 0;
}

#ifdef MODE_CJ
// ����WORģʽ
void enterWor(void)
{
  halRfStrobe(CC1101_SIDLE);
  halRfWriteReg(CC1101_MCSM2, 0x04);// ռ�ձ�0.781%
  
  /* tevent0 1.125s
   * tevent0=750/(26*10^6)*event0*2^(5*WOR_RES)
   *             ����Ƶ��
   * event0=39000(0x9858) */
  halRfWriteReg(CC1101_WOREVT1, 0x98);
  halRfWriteReg(CC1101_WOREVT0, 0x58);
  
  halRfWriteReg(CC1101_WORCTRL, 0x38); // EVENT1=3,RC_CAL=1,WOR_RES=0
  
  halRfStrobe(CC1101_SWORRST);
  halRfStrobe(CC1101_SWOR);
}
#endif

// �ⲿ�жϴ�����
#pragma vector=PORT1_VECTOR
__interrupt void ei(void)
{
  if((P1IFG & BIT5) != 0) //�ж��Ƿ��Ƕ�������Ŵ����ж�
  {
    _DINT(); // �ر��ж���Ӧ
    P1IFG &= ~BIT5; // ����жϱ�־λ
#ifdef MODE_HJ
    if(status==1 || 
       status==2 ||
       status==4 ||
       status==6 ||
       status==8)
    {
      isSent = 1;
    }
    else if(status==3 ||
            status==5 ||
            status==7 ||
            status==9)
    {
      isReceived = 1;
    }
#endif

#ifdef MODE_CJ
    if(status==2) // ����״̬2�������յ����Ѱ������״̬3
    {
      stat = halRfReadFifo(&len, 1); // ��ȡ�������ݰ�����
      if((stat & CC1101_STATUS_STATE_BM) == CC1101_STATE_RX_OVERFLOW)
      {
        halRfStrobe(CC1101_SIDLE);
        halRfStrobe(CC1101_SFRX);
        enterWor();
      }
      else if(len!=3) // ���ݰ����ж��Ƿ��ǻ������ݰ�������Ϊ3
      {
        halRfStrobe(CC1101_SIDLE);
        halRfStrobe(CC1101_SFRX);
        enterWor();
      }
      else
      {
        halRfReadFifo(pak, len+2); // ��ȡ�������ݰ�
        if((pak[len+1] & CC1101_LQI_CRC_OK_BM) != CC1101_LQI_CRC_OK_BM) // CRCУ��
        {
          halRfStrobe(CC1101_SIDLE);
          halRfStrobe(CC1101_SFRX);
          enterWor();
        }
        else if(pak[1]!=KEY_L || pak[2]!=KEY_H) // ����16λ��Կ
        {
          halRfStrobe(CC1101_SIDLE);
          halRfStrobe(CC1101_SFRX);
          enterWor();
        }
        else // ���м���ͨ��
        {
          halRfStrobe(CC1101_SIDLE);
          halRfStrobe(CC1101_SFRX);
          LPM3_EXIT; // msp430�˳��͹���ģʽ
        }
      }
    }
    else if(status==3)
    {
      isReceived = 1;
    }
#endif
    _EINT();
  }

//#ifdef MODE_CJ
//  uint8 rc;
//  if((P1IFG & BIT5) != 0)
//  {
//    _DINT();
//    P1IFG &= ~BIT5;
//    rc = halRfReadFifo(&len, 1);
//    if((rc & CC1101_STATUS_STATE_BM) == CC1101_STATE_RX_OVERFLOW)
//    {
//      goto err;
//    }
//    else if(len == 0 || len > 8)
//    {
//      goto err;
//    }
//    else
//    {
//      halRfReadFifo(pak, len + 2);
//      if((pak[len+1] & CC1101_LQI_CRC_OK_BM) != CC1101_LQI_CRC_OK_BM)
//      {
//        goto err;
//      }
//      else
//      {
//        isSent = 1;
//      }
//    }
//  err:
//    halRfStrobe(CC1101_SIDLE);
//    halRfStrobe(CC1101_SFRX);
//    enterWor();
//    _EINT();
//  }
//#endif

}

// �������ݰ�
void sendPacket(uint8 *data, uint8 length)
{
  isSent = 0;
  halRfWriteFifo(data, length);
  halRfStrobe(CC1101_STX);
  while(isSent==0);
  
  halRfStrobe(CC1101_SFTX);
}

#ifdef MODE_HJ
// ���Ѳɼ��ڵ㺯��
void wakeUp(void)
{
  //���Ѱ� [0]���� [1]�㲥��ַ [2-3]16λ��Կ
  uint8 pak[4] = {3, ADDR_B, KEY_L, KEY_H};
  //����bit 4*8+16+4*8+16=96
  //���ô�������Ϊ250kb/s ����ʱ��1.125s ��Ҫ���ʹ���250000*1.125/96=2930
  uint16 i;
  //ʵ�ʲ��Դ���2930/3=977
  for(i=0; i<977; i++)
  {
    sendPacket(pak, 4);
  }
}
#endif

// �������ݰ�
uint8 receivePacket(uint8 *data, uint8 *length)
{
  uint8 rc;
  isReceived = 0;
  halRfStrobe(CC1101_SRX);
  while(isReceived==0);
  rc = halRfReadFifo(length, 1);

  if((rc & CC1101_STATUS_STATE_BM) == CC1101_STATE_RX_OVERFLOW)
  {
    halRfStrobe(CC1101_SIDLE);
    halRfStrobe(CC1101_SFRX);
    rc = 1; // error 1
  }
  else if(*length!=LEN)
  {
    halRfStrobe(CC1101_SIDLE);
    halRfStrobe(CC1101_SFRX);
    rc = 2; // error 2
  }
  else
  {
    halRfReadFifo(data, *length + 2);
    if((data[*length+1] & CC1101_LQI_CRC_OK_BM) != 
       CC1101_LQI_CRC_OK_BM)
    {
      rc = 3; // error 3
    }
    else if(data[2]!=KEY_L || data[3]!=KEY_H)
    {
      rc = 4; // error 4
    }
    else
    {
      rc = 0; // ok
    }
  }
  halRfStrobe(CC1101_SFRX);
  return rc;
}

void main(void)
{
  uint8 length;
#ifdef MODE_HJ
  // ��Ϊ�������ݰ�
  // [0]Ŀ�ĵ�ַ [1]Դ��ַ [2-3]16λ��Կ [4]����λ [5]���� [6]С�� [7]RSSI [8]LQI
  uint8 pakTemp[4][9] = {0};
  
  // ��Ϊ�������ݰ�
  // [0]����:4��[1]Ŀ�ĵ�ַ:11-14��[2]Դ��ַ:10��[3-4]16λ��Կ
  uint8 pakAsk[5] = {4, 0, ADDR_HJ, KEY_L, KEY_H};
  uint8 i;
#endif
  
#ifdef MODE_CJ
  // ��Ϊ�������ݰ�
  // [0]Ŀ�ĵ�ַ [1]Դ��ַ [2-3]16λ��Կ [4]RSSI [5]LQI
  uint8 pakAsk[6] = {0};
  
  // ��Ϊ�������ݰ�
  // [0]����:7��[1]Ŀ�ĵ�ַ:10��[2]Դ��ַ:11-14��[3-4]16λ��Կ��[5]����λ��[6]�������֣�[7]С������
  uint8 pakTemp[8] = {7, ADDR_HJ, ADDR_CJ, KEY_L, KEY_H, 0, 0, 0};
#endif
  
  WDTCTL = WDTPW + WDTHOLD;
  
  initClock();

  ioInit();
  
#ifdef MODE_OLED
  halOledInit();
#endif
  
#ifdef MODE_UART
  halUartInit();
#endif
  
  halSpiInit();
  halRfReset();
  halRfConfig(&rf_setting3, myPaTable2, 8);
  
  // ��ʼ����ɣ�����״̬1
  status = 1;
  _EINT();
  
#ifdef MODE_OLED
  halOledShowStr6x8Ex(0, 0, "Init finish");
#endif
  
#ifdef MODE_UART
  halUartWrite("Init finish\n");
#endif
  
#ifdef MODE_HJ
  while(1)
  {
    INIT_TIMER_A(1000);
    wakeUp();
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 1, "Send wakeup");
#endif
  
#ifdef MODE_UART
    halUartWrite("Send wakeup\n");
#endif
    // ���ͻ������ݰ�������״̬2
    status = 2;
    for(i=0; i<4; i++)
    {
      pakAsk[1] = ADDR_CJ1 + i;
      sendPacket(pakAsk, 5);
      // ���ͽڵ�i+1����ѯ�����ݰ�������״̬3
      status++;
      nms = 0;
      START_TIMER_A;
      while((receivePacket(pakTemp[i], &length)!=0) &&
            (nms<100));
      STOP_TIMER_A;
      status++;
    }
    // �ȴ�һ��ʱ��
    LPM3; // ����ʹ�ã�ִֻ��һ��ѭ�������Գɹ���ĳ��ӳٺ���
    status = 1;
  }
#endif

#ifdef MODE_CJ
  while(1)
  {
  loop:
    enterWor();
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 1, "Enter wor");
#endif
  
#ifdef MODE_UART
    halUartWrite("Enter wor\n");
#endif
    status = 2;
    LPM3; // msp430����͹���ģʽ������������ֹͣ���ȴ��������ݰ�
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 2, "Wakeup");
#endif
  
#ifdef MODE_UART
    halUartWrite("Wakeup\n");
#endif
    INIT_TIMER_A(1000);
    status = 3;
    nms = 0;
    START_TIMER_A;
    while(1)
    {
      if(receivePacket(pakAsk, &length)==0)
      {
        STOP_TIMER_A;
        break;
      }
      if(nms>500)
      {
        STOP_TIMER_A;
        goto loop;
      }
    }
    status = 4;
    halTemp(pakTemp+5);
    sendPacket(pakTemp, 8);
    status = 5;
  }
#endif

}
