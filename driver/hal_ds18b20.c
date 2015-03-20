#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_ds18b20.h"
#include "delay.h"

#define DQ1 P6OUT |= BIT3
#define DQ0 P6OUT &= ~BIT3
#define DQ_IN P6DIR &= ~BIT3
#define DQ_OUT P6DIR |= BIT3
#define DQ_VAL (P6IN & BIT3)


uint8 sign, integer, decimal;

uint8 _init(void)
{
  uint8 errno;
  
  DQ_OUT;
  _DINT();//disable interrupt
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
  _EINT();
  
  _delayus(400);
  return errno;
}

void _write(uint8 dat)
{
  uint8 i;
  _DINT();
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
  _EINT();
}

uint8 _read(void)
{
  uint8 i;
  uint8 temp;
  
  _DINT();
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
  _EINT();
  
  return temp;
}

void _skip(void)
{
  _write(0xcc);
}

void _convert(void)
{
  _write(0x44);
}

void _sp(void)
{
  _write(0xbe);
}

uint16 _tempRead(void)
{
  uint8 temp_low;
  uint16 temp;
  
  temp_low = _read();
  temp = _read();
  temp = (temp<<8) | temp_low;
  
  return temp;
}

// t为数组 元素个数为3 分别表示 符号 整数部分 小数部分
void halTemp(uint8 *t)
{
  uint8 i;
  uint16 temp;
  do
  {
    i = _init();
  }while(i);
  _skip();
  _convert();
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
  temp = _tempRead();
  sign = 0;
  if(temp>63488)
  {
    temp = 65536-temp;
    t[0] = 1;
  }
  t[1] = (temp>>4) & 0xff;
  t[2] = temp & 0x0f;
}
