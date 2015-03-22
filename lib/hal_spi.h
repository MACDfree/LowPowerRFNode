#ifndef hal_spi_h
#define hal_spi_h

#include "hal_types.h"

#define CSN0 (P3OUT &= ~BIT0) // CSN��������
#define CSN1 (P3OUT |= BIT0) // CSN������1
#define SOMI (P3IN & BIT2) // ��SOMI���ŵ�ֵ

void halSpiInit(void); // SPI��ʼ��
uint8 halSpiWrite(uint8 addr, const uint8 *data, uint8 length); // дһ�����ݵ����豸
uint8 halSpiRead(uint8 addr, uint8 *data, uint8 length); // ��һ�����ݵ����豸
uint8 halSpiStrobe(uint8 cmd); // ����һ���ֽڵ�ָ����豸

#endif