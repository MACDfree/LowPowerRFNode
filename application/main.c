#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_spi.h"
#include "hal_rf.h"
#include "hal_cc1101.h"
#include "rf_setting.h"
#include "hal_ds18b20.h"
#include "delay.h"
#include "hal_clock.h"
#include "hal_led.h"
#include "settings.h"

#ifdef MODE_UART
#include "hal_uart.h"
#endif

#ifdef MODE_OLED
#include "hal_oled.h"
#endif

uint8 status = 0; // 记录节点工作状态
uint8 isSent = 0; // 是否发送完成标志
uint8 isReceived = 0; // 是否接收完成标志
uint16 nms = 0; // 定时器计数

#ifdef MODE_CJ
uint8 len = 0; // 中断中使用以记录数据包长度
uint8 stat = 0; // 中断中使用以记录数据状态
uint8 pak[5] = {0}; // 中断中使用以接收唤醒数据包
uint8 const MCSM2=0x07, WOREVT1=0x87, WOREVT0=0x6b, WORCTRL=0xf8; // 保存原始值，直接固定初始值
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
  //MCSM2 = halRfReadReg(CC1101_MCSM2);
  halRfWriteReg(CC1101_MCSM2, 0x04);// 占空比0.781%
  
  /* tevent0 1.125s
   * tevent0=750/(26*10^6)*event0*2^(5*WOR_RES)
   *             晶振频率
   * event0=39000(0x9858) */
  //WOREVT1 = halRfReadReg(CC1101_WOREVT1);
  //WOREVT0 = halRfReadReg(CC1101_WOREVT0);
  halRfWriteReg(CC1101_WOREVT1, 0x98);
  halRfWriteReg(CC1101_WOREVT0, 0x58);
  
  //WORCTRL = halRfReadReg(CC1101_WORCTRL);
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
          halRfWriteReg(CC1101_MCSM2, MCSM2);
          halRfWriteReg(CC1101_WOREVT1, WOREVT1);
          halRfWriteReg(CC1101_WOREVT0, WOREVT0);
          halRfWriteReg(CC1101_WORCTRL, WORCTRL);
          LPM3_EXIT; // msp430退出低功耗模式
        }
      }
    }
    else if(status==3)
    {
      isReceived = 1;
    }
    else if(status==4)
    {
      isSent = 1;
    }
#endif
    _EINT();
  }
}

// 发送数据包
void sendPacket(uint8 *data, uint8 length)
{
  isSent = 0;
  halRfWriteFifo(data, length);
  halRfStrobe(CC1101_STX);
  while(isSent==0);
  
  halRfStrobe(CC1101_SIDLE);
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
  //重新测试次数900 时间2015-05-28
  //920 时间2015-06-09
  for(i=0; i<920; i++)
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

  while(1)
  {
    if(isReceived==1)
    {
      break;
    }
    /*
    对于采集节点，当接收到唤醒包后，设置接收超时时间，超过时间后重新进入wor模式
    时间设置为4倍的汇集节点超时时间
    对于汇集节点，发送完定向询问包后，设置接收超时时间，超过时间后跳过该节点
    由于采集节点要进行温度转换，需要1.2s，所以超时时间设置应大于1.2s
    */
    if(nms>TIMEOUT) // 此处设置接收超时时间
    {
      STOP_TIMER_A;
      halRfStrobe(CC1101_SIDLE);
      halRfStrobe(CC1101_SFRX);
      return 5; // 接收超时
    }
  }
  
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
      halRfStrobe(CC1101_SIDLE);
      halRfStrobe(CC1101_SFRX);
      rc = 3; // error 3
    }
    else if(data[2]!=KEY_L || data[3]!=KEY_H)
    {
      halRfStrobe(CC1101_SIDLE);
      halRfStrobe(CC1101_SFRX);
      rc = 4; // error 4
    }
    else
    {
      halRfStrobe(CC1101_SIDLE);
      halRfStrobe(CC1101_SFRX);
      STOP_TIMER_A;
      rc = 0; // ok
    }
  }
  return rc;
}

// 定时器A中断处理函数
#pragma vector=TIMERA0_VECTOR
__interrupt void IntimerA(void)
{
  nms = (nms + 1) % 0xffff;
}

void main(void)
{
  uint8 length=0;
  uint8 err;
#ifdef MODE_HJ
  // 此为接收数据包
  // [0]目的地址 [1]源地址 [2-3]16位密钥 [4]符号位 [5]整数 [6]小数 [7]RSSI [8]LQI
  // 总共4个节点，所以使用4行的二维数组保存数据
  uint8 pakTemp[4][9] = {0};
  
  // 此为发送数据包
  // [0]包长:4；[1]目的地址:11-14；[2]源地址:10；[3-4]16位密钥
  uint8 pakAsk[5] = {4, 0, ADDR_HJ, KEY_L, KEY_H};
  uint8 i, j;
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
  INIT_IO;
  
  initClock();

  ioInit();
  
#ifdef MODE_OLED
  halOledInit();
#endif
  
#ifdef MODE_UART
  halUartInit();
#endif
  _DINT();
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
    // 清空缓冲区
    for(i=0; i<4; i++)
    {
      for(j=0; j<9; j++)
      {
        pakTemp[i][j] = 0;
      }
    }
    
    wakeUp();
    // 发送唤醒数据包，进入状态2
    status = 2;
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 1, "Send wakeup");
#endif
  
#ifdef MODE_UART
    halUartWrite("Send wakeup\n");
#endif
    
    // 增加延时
    for(i=0; i<24; i++) // 转换间隔800ms以上，
    {
      _delayus(50000);
    }
    
    for(i=0; i<4; i++)
    {
      pakAsk[1] = i + ADDR_CJ1;
      sendPacket(pakAsk, 5);
      // 发送节点i+1定向询问数据包，进入状态3
      status++;
      INIT_TIMER_A(TCOUNT); // 传入参数1000表示每1ms进一次中断
      START_TIMER_A;
      while(1)
      {
        err = receivePacket(pakTemp[i], &length);
        if(err==0||err==5)
        {
          break;
        }
      }
      halRfStrobe(CC1101_SIDLE);
      status++;
    }
    
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 2, "Received");
#endif
  
#ifdef MODE_UART
    for(i=0; i<4; i++)
    {
      halUartWrite("d: ");
      itoh(pakTemp[i][0], test);
      halUartWrite(test);
      halUartWrite("| s: ");
      itoh(pakTemp[i][1], test);
      halUartWrite(test);
      halUartWrite("| key: ");
      itoh(pakTemp[i][2], test);
      halUartWrite(test);
      halUartWrite(",");
      itoh(pakTemp[i][3], test);
      halUartWrite(test);
      halUartWrite("| temp: ");
      itoo(pakTemp[i][4], test);
      halUartWrite(test);
      halUartWrite(",");
      itoo(pakTemp[i][5], test);
      halUartWrite(test);
      halUartWrite(",");
      itoo(pakTemp[i][6], test);
      halUartWrite(test);
      halUartWrite("| rssi: ");
      itoo(pakTemp[i][7], test);
      halUartWrite(test);
      halUartWrite("| lqi: ");
      itoo(pakTemp[i][8] & CC1101_LQI_EST_BM, test);
      halUartWrite(test);
      halUartWrite("\n");
    }
#endif
    
    for(i=0; i<120; i++)
    {
      _delayus(60000);
    }
    status = 1;
  }
#endif

#ifdef MODE_CJ
  while(1)
  {
  loop:
    enterWor();
#ifdef MODE_OLED
    halOledClear();
    halOledShowStr6x8Ex(0, 1, "Enter wor");
#endif
  
#ifdef MODE_UART
    halUartWrite("Enter wor\n");
#endif
    
    status = 2;
    LPM3; // msp430进入低功耗模式，程序将在这里停止，等待唤醒数据包
    
    LED_ON(0); // 被唤醒
    halTemp(pakTemp+5); // 获取温度
    LED_ON(1); // 温度采集完成
    
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 2, "Wakeup");
    uint8 rc = halRfGetRxStatus();
    itob(rc, test);
    halOledShowStr6x8Ex(0, 3, test);
#endif
  
#ifdef MODE_UART
    halUartWrite("Wakeup\n");
    uint8 rc = halRfGetRxStatus();
    itob(rc, test);
    halUartWrite(test);
    halUartWrite("\n");
#endif
    
    status = 3;
    INIT_TIMER_A(TCOUNT); // 传入参数1000表示每1ms进一次中断
    START_TIMER_A;
    while(1)
    {
      err = receivePacket(pakAsk, &length);
      if(err==0)
      {
        break;
      }
      else if(err==5)
      {
        LED_OFF(0);
        LED_OFF(1);
        goto loop;
      }
    }
    LED_ON(2); // 收到定向询问包
    status = 4;
    //halTemp(pakTemp+5); // 获取温度
    //LED_ON(3);
    sendPacket(pakTemp, 8);
    LED_ON(3); // 温度包发送完成
    
#ifdef MODE_OLED
    halOledShowStr6x8Ex(0, 5, "send temp finished");
    halOledClear();
#endif
  
#ifdef MODE_UART
    halUartWrite("send temp finished\n");
#endif

    status = 5;
    LED_OFF(0);LED_OFF(1);LED_OFF(2);LED_OFF(3);
  }
#endif

}
