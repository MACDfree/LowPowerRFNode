#ifndef hal_spi_h
#define hal_spi_h

#include "hal_types.h"

#define CSN0 (P3OUT &= ~BIT0)
#define CSN1 (P3OUT |= BIT0)
#define SOMI (P3IN & BIT2)

void halSpiInit(void);
uint8 halSpiWrite(uint8 addr, const uint8 *data, uint8 length);
uint8 halSpiRead(uint8 addr, uint8 *data, uint8 length);
uint8 halSpiStrobe(uint8 cmd);

#endif