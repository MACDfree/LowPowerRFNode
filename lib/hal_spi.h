#ifndef hal_spi_h
#define hal_spi_h

#include "hal_types.h"

#define CSN0 (P3OUT &= ~BIT0) // CSN引脚清零
#define CSN1 (P3OUT |= BIT0) // CSN引脚置1
#define SOMI (P3IN & BIT2) // 读SOMI引脚的值

void halSpiInit(void); // SPI初始化
uint8 halSpiWrite(uint8 addr, const uint8 *data, uint8 length); // 写一串数据到从设备
uint8 halSpiRead(uint8 addr, uint8 *data, uint8 length); // 读一串数据到主设备
uint8 halSpiStrobe(uint8 cmd); // 发送一个字节的指令到从设备

#endif