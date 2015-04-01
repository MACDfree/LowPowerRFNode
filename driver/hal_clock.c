#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_clock.h"

// 初始化时钟源
void initClock(void)
{
  uint16 i;
  BCSCTL1 &= ~XT2OFF; //打开XT2高频晶体振荡器
  do
  {
    IFG1 &= ~OFIFG; //清除晶振失败标志
    for(i = 0xFF; i > 0; i--); //等待8MHz晶体起振
  } while(IFG1 & OFIFG); //晶振失效标志仍然存在
  BCSCTL2 |= SELM_2 + SELS; //MCLK和SMCLK选择高频晶振
}