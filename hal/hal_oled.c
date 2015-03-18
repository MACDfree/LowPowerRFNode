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
#define SCLK_INIT P2DIR|=BIT0
#define SCLK_SET  P2OUT|=BIT0
#define SCLK_CLR  P2OUT&=(~BIT0)

// D1(dat)
#define SDIN_INIT P2DIR|=BIT1
#define SDIN_SET  P2OUT|=BIT1
#define SDIN_CLR  P2OUT&=(~BIT1)

// DC(数据/命令)
#define DC_INIT P2DIR|=BIT3
#define DC_SET  P2OUT|=BIT3
#define DC_CLR  P2OUT&=(~BIT3)

// CS
#define CS_INIT P2DIR|=BIT4
#define CS_SET  P2OUT|=BIT4
#define CS_CLR  P2OUT&=(~BIT4)

// RES
#define RES_INIT P2DIR|=BIT2
#define RES_SET  P2OUT|=BIT2
#define RES_CLR  P2OUT&=(~BIT2)

#define CMD 0
#define DAT 1

uint8 gx;//光标横坐标,0~127
uint8 gy;//光标纵坐标,0~7

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

void _writeByte(uint8 dat, uint8 cmd)
{
  uint8 i;
  if(cmd)
    DC_SET;
  else
    DC_CLR;
  CS_CLR;
  for(i=0; i<8; i++)
  {
    SCLK_CLR;
    if(dat&0x80)
      SDIN_SET;
    else
      SDIN_CLR;
    SCLK_SET;
    dat<<=1;
  }
  DC_SET;
  CS_SET;
}

void _setPos(uint8 x, uint8 y)
{
  x = x % 128;
  y = y % 8;
  gx = x;
  gy = y;
  _writeByte(0xb0+y, CMD);
  _writeByte(((x&0xf0)>>4)|0x10, CMD);
  _writeByte((x&0x0f), CMD);
}

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

void halOledDisplayOn(void)
{
  _writeByte(0x8d, CMD);
  _writeByte(0x14, CMD);
  _writeByte(0xaf, CMD);
}

void halOledDisplayOff(void)
{
  _writeByte(0x8d, CMD);
  _writeByte(0x10, CMD);
  _writeByte(0xae, CMD);
}

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

void halOledShowStr6x8Ex(uint8 x, uint8 y, uint8 *chr)
{
  _setPos(x, y);
  while(*chr!='\0')
  {
    _showChar6x8(gx, gy, *chr++);
  }
}
