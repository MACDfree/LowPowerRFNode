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

//DS18B20初始化函数
uint8 _init(void)
{
  uint8 errno;
  
  DQ_OUT;
  _DINT(); //关闭总中断
  DQ0;
  _delayus(500);
  DQ1;
  _NOP();
  DQ_IN;
  _delayus(55);
  if(DQ_VAL) 
  {
    errno = 1; // ds18b20不存在
  }
  else
  {
    errno = 0; // ds18b20存在
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

#define _skip() _write(0xcc) // 宏定义版

// Skip ROM（可以使用宏定义，减少函数调用 2015-03-20 10:48）
//void _skip(void)
//{
//  _write(0xcc);
//}

#define _convert() _write(0x44) // 宏定义版

// 发送温度转换指令（同上，可使用宏定义 2015-03-20 10:49）
//void _convert(void)
//{
//  _write(0x44);
//}

#define _sp() _write(0xbe) // 宏定义版

// 发送读取温度指令
//void _sp(void)
//{
//  _write(0xbe);
//}

// t为数组 元素个数为3 分别表示 符号 整数部分 小数部分
uint8 halTemp(uint8 *t)
{
  uint8 i, j;
  uint16 temp;
  
  j = 0;
  do
  {
    i = _init();
    j++;
  }while((i==1)&&(j<=10)); // 当设备不存在（i==1）而且重试次数不超过10次时循环
  
  _skip();
  _convert(); // 先转换一次，避免出现85的问题
  
  for(i=0; i<20; i++) // 转换间隔800ms以上
  {
    _delayus(60000);
  }
  
  j = 0;
  do
  {
    i = _init();
    j++;
  }while((i==1)&&(j<=10)); // 当设备不存在（i==1）而且重试次数不超过10次时循环
  
  _skip();
  _sp(); // 发送读取温度指令
  
  // 读取16位温度数据
  i = _read();
  temp = _read();
  temp = (temp<<8) | i;
  
  // 判断符合位，使用补码方式表示的
  if(temp>63488)
  {
    temp = 65536-temp;
    t[0] = 1;
  }
  t[1] = (temp>>4) & 0xff; // 8位温度整数部分
  t[2] = temp & 0x0f; // 4位温度小数部分
  return 0;
}
