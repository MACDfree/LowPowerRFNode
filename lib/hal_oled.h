#ifndef hal_oled_h
#define hal_oled_h

#include "hal_types.h"

void halOledInit(void); // OLED屏初始化
void halOledClear(void); // 清屏函数
void halOledDisplayOn(void); // 开启显示
void halOledDisplayOff(void); // 关闭显示
void halOledShowStr6x8(uint8 *chr); // 显示6x8像素的字符串
void halOledShowStr6x8Ex(uint8 x, uint8 y, uint8 *chr); // 从指定位置开始显示6x8像素的字符串

#endif