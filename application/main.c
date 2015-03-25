#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_spi.h"
#include "hal_rf.h"
#include "hal_cc1101.h"
#include "rf_setting.h"
#include "hal_ds18b20.h"

// 设置条件编译
//#define MODE_TEST // 测试模式
//#define MODE_HJ // 汇集节点模式
//#define MODE_CJ // 采集节点模式
//#define MODE_OLED // 使用OLED
//#define MODE_UART // 使用串口

#ifdef MODE_UART
#include "hal_uart.h"
#endif

#ifdef MODE_OLED
#include "hal_oled.h"
#endif


uint8 isSent = 0;
uint8 test[9];

#ifdef MODE_TEST
//2进制 out: 9
void itob(uint8 t, uint8 *out)
{
  uint8 i;
  for(i=0; i<8; i++)
  {
    if(t & 0x80)
    {
      out[i] = '1';
    }
    else
    {
      out[i] = '0';
    }
    t <<= 1;
  }
  out[8] = '\0';
}

//16进制 out: 5
void itoh(uint8 t, uint8 *out)
{
  uint8 temp;
  out[0] = '0';
  out[1] = 'x';
  temp = (t & 0xf0) >> 4;
  if(temp<10)
  {
    out[2] = temp + '0';
  }
  else
  {
    out[2] = (temp-10) + 'A';
  }
  temp = t & 0x0f;
  if(temp<10)
  {
    out[3] = temp + '0';
  }
  else
  {
    out[3] = (temp-10) + 'A';
  }
  out[4] = '\0';
}

//10进制 out: 4
void itoo(uint8 t, uint8 *out)
{
  out[0] = t / 100 + '0';
  t = t % 100;
  out[1] = t / 10 + '0';
  out[2] = t % 10 + '0';
  out[3] = '\0';
}
#endif

// 设置中断功能
void ioInit(void)
{
  P1DIR &= ~BIT5;
  P1SEL &= ~BIT5;
  P1IES |= BIT5;
  P1IE |= BIT5;
  P1IFG = 0;
}

// 中断处理函数
#pragma vector=PORT1_VECTOR
__interrupt void ei(void)
{
  if((P1IFG & BIT5) != 0)
  {
    isSent = 1;
    P1IFG &= ~BIT5;
  }

#ifdef MODE_CJ
  uint8 rc;
  if((P1IFG & BIT5) != 0)
  {
    _DINT();
    P1IFG &= ~BIT5;
    rc = halRfReadFifo(&len, 1);
    if((rc & CC1101_STATUS_STATE_BM) == CC1101_STATE_RX_OVERFLOW)
    {
      goto err;
    }
    else if(len == 0 || len > 8)
    {
      goto err;
    }
    else
    {
      halRfReadFifo(pak, len + 2);
      if((pak[len+1] & CC1101_LQI_CRC_OK_BM) != CC1101_LQI_CRC_OK_BM)
      {
        goto err;
      }
      else
      {
        isSent = 1;
      }
    }
  err:
    halRfStrobe(CC1101_SIDLE);
    halRfStrobe(CC1101_SFRX);
    enterWor();
    _EINT();
  }
#endif

}

// 发送数据包
void sendPacket(uint8 *data, uint8 length)
{
  isSent = 0;
  halRfWriteFifo(data, length);
  halRfStrobe(CC1101_STX);
  while(isSent==0);
  
  halRfStrobe(CC1101_SFTX);
}

// 进入WOR模式
void enterWor(void)
{
  halRfStrobe(CC1101_SIDLE);
  halRfWriteReg(CC1101_MCSM2, 0x04);// 占空比0.781%
  
  /* tevent0 1.125s
   * tevent0=750/(26*10^6)*event0*2^(5*WOR_RES)
   *             晶振频率
   * event0=39000(0x9858) */
  halRfWriteReg(CC1101_WOREVT1, 0x98);
  halRfWriteReg(CC1101_WOREVT0, 0x58);
  
  halRfWriteReg(CC1101_WORCTRL, 0x38); // EVENT1=3,RC_CAL=1,WOR_RES=0
  
  halRfStrobe(CC1101_SWORRST);
  halRfStrobe(CC1101_SWOR);
}

// 唤醒采集节点函数
void wakeUp(void)
{
  //唤醒包 [0]包长 [1]广播地址 [2-3]16位密钥
  uint8 pak[4] = {3, 0, 10, 11};
  //包总bit 4*8+16+4*8+16=96
  //设置传输速率为250kb/s 唤醒时间1.125s 需要发送次数250000*1.125/96=2930
  uint16 i;
  //实际测试次数2930/3=977
  for(i=0; i<977; i++)
  {
    sendPacket(pak, 4);
  }
}

#ifdef MODE_HJ
// 接收数据包
uint8 receivePacket(uint8 *data, uint8 *length)
{
  uint8 rc;
  isSent = 0;
  halRfStrobe(CC1101_SRX);
  while(isSent==0);
  rc = halRfGetRxStatus();//halRfReadReg(CC1101_RXBYTES);
  itob(rc, test);
  //halOledShowStr6x8Ex(0, 7, test);
  rc = halRfReadFifo(length, 1);
  
  if((rc & CC1101_STATUS_STATE_BM) == CC1101_STATE_RX_OVERFLOW)
  {
    halRfStrobe(CC1101_SIDLE);
    halRfStrobe(CC1101_SFRX);
    rc = 1;
  }
  else if(*length == 0 || *length > 8)
  {
    halRfStrobe(CC1101_SIDLE);
    halRfStrobe(CC1101_SFRX);
    rc = 2;
  }
  else
  {
    halRfReadFifo(data, *length + 2);
    if((data[*length+1] & CC1101_LQI_CRC_OK_BM) != CC1101_LQI_CRC_OK_BM)
    {
      rc = 3;
    }
    else
    {
      rc = 0;
    }
  }
  halRfStrobe(CC1101_SFRX);
  return rc;
}
#endif

void main(void)
{
  //[0]包长 [1]目的地址 [2]源地址 [3-4]16位密钥 [5]符号位 [6]整数 [7]小数
  uint8 temp[8] = {7, 1, 2, 10, 11, 0, 0, 0};
  uint16 i, count = 0;
  uint8 id, ver, rc, cfg;
  
  WDTCTL = WDTPW + WDTHOLD;
  
  BCSCTL1 &= ~XT2OFF; //打开XT2高频晶体振荡器
  do
  {
    IFG1 &= ~OFIFG; //清除晶振失败标志
    for(i = 0xFF; i > 0; i--); //等待8MHz晶体起振
  } while(IFG1 & OFIFG); //晶振失效标志仍然存在
  BCSCTL2 |= SELM_2 + SELS; //MCLK和SMCLK选择高频晶振
  
  TACTL |= TASSEL_2 + ID_3;
  
  
  ioInit();
  halOledInit();
  halSpiInit();
  halRfReset();
  halRfConfig(&rf_setting3, myPaTable2, 8);
  
  _EINT();
  
  wakeUp();
  halOledShowStr6x8Ex(0, 0, "#");
  while(1)
  {/*
    //halTemp(temp + 5);
    //sendPacket(temp, 8);
    wakeUp();
    if(count == 0)
    {
      halOledShowStr6x8Ex(0, 0, "#          ");
    }
    else
    {
      halOledShowStr6x8Ex(count, 0, "#");
    }
    count = (count+6) % 60;
    for(i=0; i<65535; i++);
    for(i=0; i<65535; i++);
    for(i=0; i<65535; i++);
    for(i=0; i<65535; i++);
    for(i=0; i<65535; i++);
    //halRfStrobe(CC1101_SIDLE);
    rc = halRfGetTxStatus();
    itob(rc, test);
    halOledShowStr6x8Ex(0, 4, test);*/
  }
}
