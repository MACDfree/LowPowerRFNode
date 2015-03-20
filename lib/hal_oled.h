#ifndef hal_oled_h
#define hal_oled_h

#include "hal_types.h"

void halOledInit(void);
void halOledClear(void);
void halOledDisplayOn(void);
void halOledDisplayOff(void);
void halOledShowStr6x8(uint8 *chr);
void halOledShowStr6x8Ex(uint8 x, uint8 y, uint8 *chr);

#endif