#include <msp430x14x.h>
#include "hal_types.h"

// 使用Timer_A实现延时，n (us微秒)
void _delayus(uint16 n)
{
  CCR0 = n; //需要延时的时间 CCR0为TACCR0
  TACTL |= MC_1; //Timer_A定时模式选择UP，定时器从0递增到TACCR0的值后触发中断，然后重新从0递增
  while(!(TACTL & BIT0)); //使用查询方式检测TACTL的0位即中断标志位TAIFG
  TACTL &= ~MC_1; //清零MC位，使Timer_A进入STOP模式
  TACTL &= ~BIT0; //清除中断标志位
}
