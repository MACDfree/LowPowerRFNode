#ifndef hal_oled_h
#define hal_oled_h

#include "hal_types.h"

void halOledInit(void); // OLED����ʼ��
void halOledClear(void); // ��������
void halOledDisplayOn(void); // ������ʾ
void halOledDisplayOff(void); // �ر���ʾ
void halOledShowStr6x8(uint8 *chr); // ��ʾ6x8���ص��ַ���
void halOledShowStr6x8Ex(uint8 x, uint8 y, uint8 *chr); // ��ָ��λ�ÿ�ʼ��ʾ6x8���ص��ַ���

#endif