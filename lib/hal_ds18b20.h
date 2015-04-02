#ifndef hal_ds18b20_h
#define hal_ds18b20_h

// 获取温度值，t为输出参数，共3字节，分别表示[0]符号位 [1]整数部分 [2]小数部分
uint8 halTemp(uint8 *t);

#endif