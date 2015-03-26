#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_spi.h"
#include "hal_rf.h"
#include "hal_cc1101.h"
#include "rf_setting.h"
#include "hal_ds18b20.h"
#include "delay.h"

// 定义条件编译标识
//#define MODE_TEST // 测试模式
//#define MODE_HJ // 汇集节点模式
//#define MODE_CJ // 采集节点模式
//#define MODE_OLED // 使用OLED
//#define MODE_UART // 使用串口

// 定义节点基本信息
#define ADDR_HJ 10 // 汇集节点地址为10
#define ADDR_CJ1 11 // 采集节点1地址
#define ADDR_CJ2 12 // 采集节点2地址
#define ADDR_CJ3 13 // 采集节点3地址
#define ADDR_CJ4 14 // 采集节点4地址
#define ADDR_B 0 // 广播地址为0
#define KEY_H 111 // 密钥高8位
#define KEY_L 111 // 密钥低8位


#ifdef MODE_UART
#include "hal_uart.h"
#endif

#ifdef MODE_OLED
#include "hal_oled.h"
#endif

uint8 status = 0; // 记录节点工作状态
uint8 isSent = 0;
uint8 isReceived = 0;


#ifdef MODE_TEST

uint8 test[9];
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

// 设置IO引脚中断功能
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
    _DINT();
    P1IFG &= ~BIT5;
#ifdef MODE_HJ
    if(status==1 || 
       status==2 ||
       status==4 ||
       status==6 ||
       status==8)
    {
      isSent = 1;
    }
    else if(status==3 ||
            status==5 ||
            status==7 ||
            status==9)
    {
      isReceived = 1;
    }
#endif
    _EINT();
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
  uint8 pak[4] = {3, ADDR_B, KEY_L, KEY_H};
  //包总bit 4*8+16+4*8+16=96
  //设置传输速率为250kb/s 唤醒时间1.125s 需要发送次数250000*1.125/96=2930
  uint16 i;
  //实际测试次数2930/3=977
  for(i=0; i<977; i++)
  {
    sendPacket(pak, 4);
  }
}

// 接收数据包
uint8 receivePacket(uint8 *data, uint8 *length)
{
  uint8 rc;
  isReceived = 0;
  halRfStrobe(CC1101_SRX);
  while(isReceived==0);
  rc = halRfReadFifo(length, 1);
  
#ifdef MODE_HJ
  if((rc & CC1101_STATUS_STATE_BM) == CC1101_STATE_RX_OVERFLOW)
  {
    halRfStrobe(CC1101_SIDLE);
    halRfStrobe(CC1101_SFRX);
    rc = 1;
  }
  else if(*length!=7)
  {
    halRfStrobe(CC1101_SIDLE);
    halRfStrobe(CC1101_SFRX);
    rc = 2
  }
  else
  {
    halRfReadFifo(data, *length + 2);
    if((data[*length+1] & CC1101_LQI_CRC_OK_BM) != 
       CC1101_LQI_CRC_OK_BM)
    {
      rc = 3;
    }
    else if(data[2]!=KEY_L || data[3]!=KEY_H)
    {
      rc = 4;
    }
    else
    {
      rc = 0;
    }
  }
  halRfStrobe(CC1101_SFRX);
  return rc;
#endif
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

void main(void)
{
  
#ifdef MODE_HJ
  //[0]包长 [1]目的地址 [2]源地址 [3-4]16位密钥 [5]符号位 [6]整数 [7]小数
  uint8 pakTemp[9] = {0};
  //[0]包长:4；[1]目的地址:11-14；[2]源地址:10；[3-4]16位密钥
  uint8 pakAsk[5] = {4, 0, ADDR_HJ, KEY_L, KEY_H};
  uint8 i;
#endif
  
  uint16 i, count = 0;
  uint8 id, ver, rc, cfg;
  
  WDTCTL = WDTPW + WDTHOLD;
  
  initClock();

  ioInit();
  
#ifdef MODE_OLED
  halOledInit();
#endif
  
#ifdef MODE_UART
  halUartInit();
#endif
  
  halSpiInit();
  halRfReset();
  halRfConfig(&rf_setting3, myPaTable2, 8);
  
  // 初始化完成，进入状态1
  status = 1;
  _EINT();
  
#ifdef MODE_HJ
  INIT_TIMER_A(1000);
  wakeUp();
  // 发送唤醒数据包，进入状态2
  status = 2;
  for(i=0; i<4; i++)
  {
    pakAsk[1] = ADDR_CJ1 + i;
    sendPacket(pakAsk, 5);
    // 发送节点i+1定向询问数据包，进入状态3
    status++;
    nms = 0;
    START_TIMER_A;
    while((receivePacket(pakTemp, 9)!=0) ||
          (nms>100));
    STOP_TIMER_A;
    status++;
  }
#endif
  
#ifdef MODE_CJ
#endif
  
  
  
  //halOledShowStr6x8Ex(0, 0, "#");
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
