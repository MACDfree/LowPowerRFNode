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

// �¶ȵķ���λ���������֣�С������
//uint8 sign, integer, decimal;

//DS18B20��ʼ������
uint8 _init(void)
{
  uint8 errno;
  
  DQ_OUT;
  _DINT(); //�ر����ж�
  DQ0;
  _delayus(500);
  DQ1;
  _delayus(55);
  DQ_IN;
  _NOP();
  if(DQ_VAL)
  {
    errno = 1;
  }
  else
  {
    errno = 0;
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

// Skip ROM������ʹ�ú궨�壬���ٺ������� 2015-03-20 10:48��
void _skip(void)
{
  _write(0xcc);
}

// �����¶�ת��ָ�ͬ�ϣ���ʹ�ú궨�� 2015-03-20 10:49��
void _convert(void)
{
  _write(0x44);
}

// ���Ͷ�ȡ�¶�ָ��
void _sp(void)
{
  _write(0xbe);
}

// ��ȡ�¶ȣ�����16λԭʼ���¶�����
uint16 _tempRead(void)
{
  uint8 temp_low;
  uint16 temp;
  
  temp_low = _read();
  temp = _read();
  temp = (temp<<8) | temp_low;
  
  return temp;
}

// tΪ���� Ԫ�ظ���Ϊ3 �ֱ��ʾ ���� �������� С������
void halTemp(uint8 *t)
{
  uint8 i;
  uint16 temp;
  
  do
  {
    i = _init();
  }while(i); // �ȴ���ʼ����ɣ����ױ����ѭ������Ҫ�ĳɴ������ 2015-03-20 11:10��
  _skip();
  _convert(); // ��ת��һ�Σ��������85������
  for(i=0; i<20; i++)
  {
    _delayus(60000);
  }
  
  do
  {
    i = _init();
  }while(i);
  _skip();
  _sp();
  temp = _tempRead(); // ��ȡ16λ�¶�����
  sign = 0;
  
  // �жϷ���λ��ʹ�ò��뷽ʽ��ʾ��
  if(temp>63488)
  {
    temp = 65536-temp;
    t[0] = 1;
  }
  t[1] = (temp>>4) & 0xff; // 8λ�¶���������
  t[2] = temp & 0x0f; // 4λ�¶�С������
}
