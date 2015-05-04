#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_spi.h"
#include "hal_rf.h"
#include "hal_cc1101.h"
#include "rf_setting.h"
#include "hal_ds18b20.h"
#include "delay.h"
#include "hal_clock.h"

// 定义条件编译标识
/*
节点工作模式分为汇集节点（MODE_HJ）和采集节点（MODE_CJ）
MODE_TEST为测试标志
如果定义测试标志，则MODE_OLED或MODE_UART也需要定义（两者可同时定义）
*/
#define MODE_TEST // 测试模式
//#define MODE_HJ // 汇集节点模式
#define MODE_CJ // 采集节点模式
#define MODE_OLED // 使用OLED
//#define MODE_UART // 使用串口

// 定义节点基本信息
#define ADDR_HJ 10 // 汇集节点地址为10
#define ADDR_CJ1 11 // 采集节点1地址
#define ADDR_CJ2 12 // 采集节点2地址
#define ADDR_CJ3 13 // 采集节点3地址
#define ADDR_CJ4 14 // 采集节点4地址
#define ADDR_CJ 11 // 采集节点地址
#define ADDR_B 0 // 广播地址为0
#define KEY_H 111 // 密钥高8位
#define KEY_L 111 // 密钥低8位

#ifdef MODE_HJ
#define LEN 7
#endif

#ifdef MODE_CJ
#define LEN 4
#endif


#ifdef MODE_UART
#include "hal_uart.h"
#endif

#ifdef MODE_OLED
#include "hal_oled.h"
#endif

uint8 status = 0; // 记录节点工作状态
uint8 isSent = 0; // 是否发送完成标志
uint8 isReceived = 0; // 是否接收完成标志

#ifdef MODE_CJ
uint8 len = 0; // 中断中使用以记录数据包长度
uint8 stat = 0; // 中断中使用以记录数据状态
uint8 pak[5] = {0}; // 中断中使用以接收唤醒数据包
#endif

#ifdef MODE_TEST

uint8 test[9]; // 存放调试输出信息
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

#ifdef MODE_CJ
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
#endif

// 外部中断处理函数
#pragma vector=PORT1_VECTOR
__interrupt void ei(void)
{
  if((P1IFG & BIT5) != 0) //判断是否是定义的引脚触发中断
  {
    _DINT(); // 关闭中断响应
    P1IFG &= ~BIT5; // 清除中断标志位
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

#ifdef MODE_CJ
    if(status==2) // 处于状态2，当接收到唤醒包后进入状态3
    {
      stat = halRfReadFifo(&len, 1); // 读取接收数据包长度
      if((stat & CC1101_STATUS_STATE_BM) == CC1101_STATE_RX_OVERFLOW)
      {
        halRfStrobe(CC1101_SIDLE);
        halRfStrobe(CC1101_SFRX);
        enterWor();
      }
      else if(len!=3) // 根据包长判断是否是唤醒数据包，包长为3
      {
        halRfStrobe(CC1101_SIDLE);
        halRfStrobe(CC1101_SFRX);
        enterWor();
      }
      else
      {
        halRfReadFifo(pak, len+2); // 读取唤醒数据包
        if((pak[len+1] & CC1101_LQI_CRC_OK_BM) != CC1101_LQI_CRC_OK_BM) // CRC校验
        {
          halRfStrobe(CC1101_SIDLE);
          halRfStrobe(CC1101_SFRX);
          enterWor();
        }
        else if(pak[1]!=KEY_L || pak[2]!=KEY_H) // 检验16位密钥
        {
          halRfStrobe(CC1101_SIDLE);
          halRfStrobe(CC1101_SFRX);
          enterWor();
        }
        else // 所有检验通过
        {
          halRfStrobe(CC1101_SIDLE);
          halRfStrobe(CC1101_SFRX);
          LPM3_EXIT; // msp430退出低功耗模式
        }
      }
    }
    else if(status==3)
    {
      isReceived = 1;
    }
#endif
    _EINT();
  }

//#ifdef MODE_CJ
//  uint8 rc;
//  if((P1IFG & BIT5) != 0)
//  {
//    _DINT();
//    P1IFG &= ~BIT5;
//    rc = halRfReadFifo(&len, 1);
//    if((rc & CC1101_STATUS_STATE_BM) == CC1101_STATE_RX_OVERFLOW)
//    {
//      goto err;
//    }
//    else if(len == 0 || len > 8)
//    {
//      goto err;
//    }
//    else
//    {
//      halRfReadFifo(pak, len + 2);
//      if((pak[len+1] & CC1101_LQI_CRC_OK_BM) != CC1101_LQI_CRC_OK_BM)
//      {
//        goto err;
//      }
//      else
//      {
//        isSent = 1;
//      }
//    }
//  err:
//    halRfStrobe(CC1101_SIDLE);
//    halRfStrobe(CC1101_SFRX);
//    enterWor();
//    _EINT();
//  }
//#endif

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

#ifdef MODE_HJ
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
#endif

// 接收数据包
uint8 receivePacket(uint8 *data, uint8 *length)
{
  uint8 rc;
  isReceived = 0;
  halRfStrobe(CC1101_SRX);
  while(isReceived==0);
  rc = halRfReadFifo(length, 1);

  if((rc & CC1101_STATUS_STATE_BM) == CC1101_STATE_RX_OVERFLOW)
  {
    halRfStrobe(CC1101_SIDLE);
    halRfStrobe(CC1101_SFRX);
    rc = 1; // error 1
  }
  else if(*length!=LEN)
  {
    halRfStrobe(CC1101_SIDLE);
    halRfStrobe(CC1101_SFRX);
    rc = 2; // error 2
  }
  else
  {
    halRfReadFifo(data, *length + 2);
    if((data[*length+1] & CC1101_LQI_CRC_OK_BM) != 
       CC1101_LQI_CRC_OK_BM)
    {
      rc = 3; // error 3
    }
    else if(data[2]!=KEY_L || data[3]!=KEY_H)
    {
      rc = 4; // error 4
    }
    else
    {
      rc = 0; // ok
    }
  }
  halRfStrobe(CC1101_SFRX);
  return rc;
}

void main(void)
{
  uint8 length;
#ifdef MODE_HJ
  // 此为接收数据包
  // [0]目的地址 [1]源地址 [2-3]16位密钥 [4]符号位 [5]整数 [6]小数 [7]RSSI [8]LQI
  uint8 pakTemp[4][9] = {0};
  
  // 此为发送数据包
  // [0]包长:4；[1]目的地址:11-14；[2]源地址:10；[3-4]16位密钥
  uint8 pakAsk[5] = {4, 0, ADDR_HJ, KEY_L, KEY_H};
  uint8 i;
#endif
  
#ifdef MODE_CJ
  // 此为接收数据包
  // [0]目的地址 [1]源地址 [2-3]16位密钥 [4]RSSI [5]LQI
  uint8 pakAsk[6] = {0};
  
  // 此为发送数据包
  // [0]包长:7；[1]目的地址:10；[2]源地址:11-14；[3-4]16位密钥；[5]符号位；[6]整数部分；[7]小数部分
  uint8 pakTemp[8] = {7, ADDR_HJ, ADDR_CJ, KEY_L, KEY_H, 0, 0, 0};
#endif
  
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
  
#ifdef MODE_OLED
  halOledShowStr6x8Ex(0, 0, "Init finish");
#endif
  
#ifdef MODE_UART
  halUartWrite("Init finish\n");
#endif
  
#ifdef MODE_HJ
  while(1)
  {
    INIT_TIMER_A(1000);
    wakeUp();
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 1, "Send wakeup");
#endif
  
#ifdef MODE_UART
    halUartWrite("Send wakeup\n");
#endif
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
      while((receivePacket(pakTemp[i], &length)!=0) &&
            (nms<100));
      STOP_TIMER_A;
      status++;
    }
    // 等待一段时间
    LPM3; // 测试使用，只执行一次循环，测试成功后改成延迟函数
    status = 1;
  }
#endif

#ifdef MODE_CJ
  while(1)
  {
  loop:
    enterWor();
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 1, "Enter wor");
#endif
  
#ifdef MODE_UART
    halUartWrite("Enter wor\n");
#endif
    status = 2;
    LPM3; // msp430进入低功耗模式，程序将在这里停止，等待唤醒数据包
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 2, "Wakeup");
#endif
  
#ifdef MODE_UART
    halUartWrite("Wakeup\n");
#endif
    INIT_TIMER_A(1000);
    status = 3;
    nms = 0;
    START_TIMER_A;
    while(1)
    {
      if(receivePacket(pakAsk, &length)==0)
      {
        STOP_TIMER_A;
        break;
      }
      if(nms>500)
      {
        STOP_TIMER_A;
        goto loop;
      }
    }
    status = 4;
    halTemp(pakTemp+5);
    sendPacket(pakTemp, 8);
    status = 5;
  }
#endif

}
