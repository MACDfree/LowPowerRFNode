#include <msp430x14x.h>
#include "hal_types.h"

void _delayus(uint16 n)
{
  CCR0 = n;
  TACTL |= MC_1;
  while(!(TACTL & BIT0));
  TACTL &= ~MC_1;
  TACTL &= ~BIT0;
}
