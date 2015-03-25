#include <msp430x14x.h>
#include "hal_oled.h"
#include "hal_types.h"
#include "hal_oled_font.h"

/*
 * D0 P20 (clk)
 * D1 P21 (dat)
 * RES P22
 * DC P23
 * CS P24
 */

// D0(clk)
#define SCLK_INIT P2DIR|=BIT0 // CLK设置成输出
#define SCLK_SET  P2OUT|=BIT0 // CLK置1
#define SCLK_CLR  P2OUT&=(~BIT0) // clk清零

// D1(dat)
#define SDIN_INIT P2DIR|=BIT1 // 数据引脚设置成输出
#define SDIN_SET  P2OUT|=BIT1 // 数据引脚置1
#define SDIN_CLR  P2OUT&=(~BIT1) //数据引脚清零

// DC(数据/命令)
#define DC_INIT P2DIR|=BIT3 // 数据/命令引脚设置成输出
#define DC_SET  P2OUT|=BIT3 // 数据/命令引脚置1
#define DC_CLR  P2OUT&=(~BIT3) // 数据/命令引脚清零

// CS
#define CS_INIT P2DIR|=BIT4 // CS设置成输出
#define CS_SET  P2OUT|=BIT4 // CS置1
#define CS_CLR  P2OUT&=(~BIT4) // CS清零

// RES
#define RES_INIT P2DIR|=BIT2 // RES设置成输出
#define RES_SET  P2OUT|=BIT2 // RES置1
#define RES_CLR  P2OUT&=(~BIT2) // RES清零

#define CMD 0 // 发送指令时DC引脚为0
#define DAT 1 // 发送数据时DC引脚为1

uint8 gx; // 光标横坐标,0~127
uint8 gy; // 光标纵坐标,0~7

// 延时函数
void _delayMs(uint16 ms)
{                         
  uint16 a;
  while(ms)
  {
    a=800;
    while(a--);
    ms--;
  }
}

// 写一个字节数据/指令，使用SPI总线协议
void _writeByte(uint8 dat, uint8 cmd)
{
  uint8 i;
  
  // 判断是否是指令
  if(cmd)
    DC_SET;
  else
    DC_CLR;
  
  CS_CLR; //CS清零，选中外设
  
  for(i=0; i<8; i++)
  {
    SCLK_CLR; // CLK为低，数据引脚电平可变化
    if(dat&0x80) //从高位到低位传输数据 
      SDIN_SET;
    else
      SDIN_CLR;
    SCLK_SET; // CLK为高，形成上升沿，移入一位
    dat<<=1;
  }
  
  //收尾工作
  DC_SET;
  CS_SET;
}

// 设置光标位置
void _setPos(uint8 x, uint8 y)
{
  x = x % 128;
  y = y % 8;
  
  // gx, gy为全局变量保持更新
  gx = x;
  gy = y;
  _writeByte(0xb0+y, CMD); // 设置行号 0xb0-0xb7
  _writeByte(((x&0xf0)>>4)|0x10, CMD); // 设置列号的高4位 0x10-0x1f
  _writeByte((x&0x0f), CMD); // 设置列号的低4位 0x00-0x0f
}

// OLED屏初始化
void halOledInit(void)
{
  SCLK_INIT;
  SDIN_INIT;
  DC_INIT;
  CS_INIT;
  RES_INIT;
  
  RES_SET;
  _delayMs(100);
  RES_CLR;
  _delayMs(100);
  RES_SET;
  _writeByte(0xAE, CMD);//--turn off oled panel
  _writeByte(0x02, CMD);//---set low column address
  _writeByte(0x10, CMD);//---set high column address
  _writeByte(0x40, CMD);//--set start line address  Set Mapping RAM Display Start Line (0x00~0x3F)
  _writeByte(0x81, CMD);//--set contrast control register
  _writeByte(0xCF, CMD); // Set SEG Output Current Brightness
  _writeByte(0xA1, CMD);//--Set SEG/Column Mapping
  _writeByte(0xC8, CMD);//Set COM/Row Scan Direction
  _writeByte(0xA6, CMD);//--set normal display
  _writeByte(0xA8, CMD);//--set multiplex ratio(1 to 64)
  _writeByte(0x3f, CMD);//--1/64 duty
  _writeByte(0xD3, CMD);//-set display offset	Shift Mapping RAM Counter (0x00~0x3F)
  _writeByte(0x00, CMD);//-not offset
  _writeByte(0xd5, CMD);//--set display clock divide ratio/oscillator frequency
  _writeByte(0x80, CMD);//--set divide ratio, Set Clock as 100 Frames/Sec
  _writeByte(0xD9, CMD);//--set pre-charge period
  _writeByte(0xF1, CMD);//Set Pre-Charge as 15 Clocks & Discharge as 1 Clock
  _writeByte(0xDA, CMD);//--set com pins hardware configuration
  _writeByte(0x12, CMD);
  _writeByte(0xDB, CMD);//--set vcomh
  _writeByte(0x40, CMD);//Set VCOM Deselect Level
  _writeByte(0x20, CMD);//-Set Page Addressing Mode (0x00/0x01/0x02)
  _writeByte(0x02, CMD);//
  _writeByte(0x8D, CMD);//--set Charge Pump enable/disable
  _writeByte(0x14, CMD);//--set(0x10) disable
  _writeByte(0xA4, CMD);// Disable Entire Display On (0xa4/0xa5)
  _writeByte(0xA6, CMD);// Disable Inverse Display On (0xa6/a7) 
  _writeByte(0xAF, CMD);//--turn on oled panel
 
  _writeByte(0xAF, CMD); /*display ON*/ 
  halOledClear();
  _setPos(0,0);
}

// 清屏函数
void halOledClear(void)
{
  uint8 i, n;
  for(i=0; i<8; i++)
  {
    _writeByte(0xb0+i, CMD);
    _writeByte(0x00, CMD);
    _writeByte(0x10, CMD);
    for(n=0; n<128; n++)
      _writeByte(0, DAT);
  }
  
  gx = 0;
  gy = 0;
}

// 在指定位置显示6×8像素的字符
void _showChar6x8(uint8 x, uint8 y, uint8 ch)
{
  uint8 i;
  uint8 t=ch-' ';
  _setPos(x, y);
  for(i=0; i<6; i++)
  {
    _writeByte(char6x8[t][i], DAT);
  }
  gx += 6;
}

// 开启显示
void halOledDisplayOn(void)
{
  _writeByte(0x8d, CMD);
  _writeByte(0x14, CMD);
  _writeByte(0xaf, CMD);
}

// 关闭显示
void halOledDisplayOff(void)
{
  _writeByte(0x8d, CMD);
  _writeByte(0x10, CMD);
  _writeByte(0xae, CMD);
}

// 显示6×8像素的字符串
void halOledShowStr6x8(uint8 *chr)
{
  while(*chr!='\0')
  {
    if(*chr=='\n')
    {
      _setPos(0, ++gy);
    }
    else
    {
      _showChar6x8(gx, gy, *chr);
    }
    chr++;
  }
}

// 从指定位置开始显示6x8像素的字符串
void halOledShowStr6x8Ex(uint8 x, uint8 y, uint8 *chr)
{
  _setPos(x, y);
  while(*chr!='\0')
  {
    _showChar6x8(gx, gy, *chr++);
  }
}
