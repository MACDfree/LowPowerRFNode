#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_ds18b20.h"
#include "delay.h"

// 定义DS18B20的数据引脚和相关设置及操作
#define DQ1 P6OUT |= BIT3 //数据引脚置1
#define DQ0 P6OUT &= ~BIT3 //数据引脚清零
#define DQ_IN P6DIR &= ~BIT3 //数据引脚设置为输入
#define DQ_OUT P6DIR |= BIT3 //数据引脚设置为输出
#define DQ_VAL (P6IN & BIT3) //获取数据引脚的值

// 温度的符号位，整数部分，小数部分
//uint8 sign, integer, decimal;

//DS18B20初始化函数
uint8 _init(void)
{
  uint8 errno;
  
  DQ_OUT;
  _DINT(); //关闭总中断
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
  _EINT(); //开启总中断
  
  _delayus(400);
  return errno;
}

// 向DS18B20写一个字节数据
void _write(uint8 dat)
{
  uint8 i;
  _DINT(); //关闭总中断
  
  // 传输数据从低位到高位
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
  _EINT(); //开启总中断
}

// 从DS18B20读取一个字节数据
uint8 _read(void)
{
  uint8 i;
  uint8 temp;
  
  _DINT(); //关闭总中断
  
  // 传输数据从低位到高位
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
  _EINT(); // 开启总中断
  
  return temp;
}

// Skip ROM（可以使用宏定义，减少函数调用 2015-03-20 10:48）
void _skip(void)
{
  _write(0xcc);
}

// 发送温度转换指令（同上，可使用宏定义 2015-03-20 10:49）
void _convert(void)
{
  _write(0x44);
}

// 发送读取温度指令
void _sp(void)
{
  _write(0xbe);
}

// 读取温度，返回16位原始的温度数据
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
  }while(i); // 等待初始化完成（容易变成死循环，需要改成错误次数 2015-03-20 11:10）
  _skip();
  _convert(); // 先转换一次，避免出现85的问题
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
  temp = _tempRead(); // 读取16位温度数据
  sign = 0;
  
  // 判断符合位，使用补码方式表示的
  if(temp>63488)
  {
    temp = 65536-temp;
    t[0] = 1;
  }
  t[1] = (temp>>4) & 0xff; // 8位温度整数部分
  t[2] = temp & 0x0f; // 4位温度小数部分
}
