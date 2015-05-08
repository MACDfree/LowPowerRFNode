#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_ds18b20.h"
#include "delay.h"

// ����DS18B20���������ź�������ü�����
#define DQ1 P6OUT |= BIT3 //����������1
#define DQ0 P6OUT &= ~BIT3 //������������
#define DQ_IN P6DIR &= ~BIT3 //������������Ϊ����
#define DQ_OUT P6DIR |= BIT3 //������������Ϊ���
#define DQ_VAL (P6IN & BIT3) //��ȡ�������ŵ�ֵ

//DS18B20��ʼ������
uint8 _init(void)
{
  uint8 errno;
  
  DQ_OUT;
  _DINT(); //�ر����ж�
  DQ0;
  _delayus(500);
  DQ1;
  _NOP();
  DQ_IN;
  _delayus(55);
  if(DQ_VAL) 
  {
    errno = 1; // ds18b20������
  }
  else
  {
    errno = 0; // ds18b20����
  }
  DQ_OUT;
  DQ1;
  _EINT(); //�������ж�
  
  _delayus(400);
  return errno;
}

// ��DS18B20дһ���ֽ�����
void _write(uint8 dat)
{
  uint8 i;
  _DINT(); //�ر����ж�
  
  // �������ݴӵ�λ����λ
  for(i=0; i<8; i++)
  {
    DQ0;
    _delayus(6);
    if(dat & 0x01) DQ1;
    else DQ0;
    dat >>= 1;
    _delayus(50);
    DQ1;
    _delayus(10);
  }
  _EINT(); //�������ж�
}

// ��DS18B20��ȡһ���ֽ�����
uint8 _read(void)
{
  uint8 i;
  uint8 temp;
  
  _DINT(); //�ر����ж�
  
  // �������ݴӵ�λ����λ
  for(i=0; i<8; i++)
  {
    temp >>= 1;
    DQ0;
    _delayus(6);
    DQ1;
    _delayus(8);
    DQ_IN;
    _NOP();
    if(DQ_VAL) temp |= 0x80;
    _delayus(45);
    DQ_OUT;
    DQ1;
    _delayus(10);
  }
  _EINT(); // �������ж�
  
  return temp;
}

#define _skip() _write(0xcc) // �궨���

// Skip ROM������ʹ�ú궨�壬���ٺ������� 2015-03-20 10:48��
//void _skip(void)
//{
//  _write(0xcc);
//}

#define _convert() _write(0x44) // �궨���

// �����¶�ת��ָ�ͬ�ϣ���ʹ�ú궨�� 2015-03-20 10:49��
//void _convert(void)
//{
//  _write(0x44);
//}

#define _sp() _write(0xbe) // �궨���

// ���Ͷ�ȡ�¶�ָ��
//void _sp(void)
//{
//  _write(0xbe);
//}

// tΪ���� Ԫ�ظ���Ϊ3 �ֱ��ʾ ���� �������� С������
uint8 halTemp(uint8 *t)
{
  uint8 i, j;
  uint16 temp;
  
  j = 0;
  do
  {
    i = _init();
    j++;
  }while((i==1)&&(j<=10)); // ���豸�����ڣ�i==1���������Դ���������10��ʱѭ��
  
  _skip();
  _convert(); // ��ת��һ�Σ��������85������
  
  for(i=0; i<20; i++) // ת�����800ms����
  {
    _delayus(60000);
  }
  
  j = 0;
  do
  {
    i = _init();
    j++;
  }while((i==1)&&(j<=10)); // ���豸�����ڣ�i==1���������Դ���������10��ʱѭ��
  
  _skip();
  _sp(); // ���Ͷ�ȡ�¶�ָ��
  
  // ��ȡ16λ�¶�����
  i = _read();
  temp = _read();
  temp = (temp<<8) | i;
  
  // �жϷ���λ��ʹ�ò��뷽ʽ��ʾ��
  if(temp>63488)
  {
    temp = 65536-temp;
    t[0] = 1;
  }
  t[1] = (temp>>4) & 0xff; // 8λ�¶���������
  t[2] = temp & 0x0f; // 4λ�¶�С������
  return 0;
}
