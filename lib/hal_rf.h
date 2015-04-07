#ifndef hal_rf_h
#define hal_rf_h

#include "hal_types.h"
#include "hal_cc1101.h"

// ������ʾCC1101�Ĵ����Ľṹ��
typedef struct {
  uint8 fsctrl1;    // Frequency synthesizer control.
  uint8 fsctrl0;    // Frequency synthesizer control.
  uint8 freq2;      // Frequency control word, high byte.
  uint8 freq1;      // Frequency control word, middle byte.
  uint8 freq0;      // Frequency control word, low byte.
  uint8 mdmcfg4;    // Modem configuration.
  uint8 mdmcfg3;    // Modem configuration.
  uint8 mdmcfg2;    // Modem configuration.
  uint8 mdmcfg1;    // Modem configuration.
  uint8 mdmcfg0;    // Modem configuration.
  uint8 channr;     // Channel number.
  uint8 deviatn;    // Modem deviation setting (when FSK modulation is enabled).
  uint8 frend1;     // Front end RX configuration.
  uint8 frend0;     // Front end RX configuration.
  uint8 mcsm0;      // Main Radio Control State Machine configuration.
  uint8 foccfg;     // Frequency Offset Compensation Configuration.
  uint8 bscfg;      // Bit synchronization Configuration.
  uint8 agcctrl2;   // AGC control.
  uint8 agcctrl1;   // AGC control.
  uint8 agcctrl0;   // AGC control.
  uint8 fscal3;     // Frequency synthesizer calibration.
  uint8 fscal2;     // Frequency synthesizer calibration.
  uint8 fscal1;     // Frequency synthesizer calibration.
  uint8 fscal0;     // Frequency synthesizer calibration.
  uint8 fstest;     // Frequency synthesizer calibration.
  uint8 test2;      // Various test settings.
  uint8 test1;      // Various test settings.
  uint8 test0;      // Various test settings.
  uint8 iocfg2;     // GDO2 output pin configuration.
  uint8 iocfg0;     // GDO0 output pin configuration.
  uint8 pktctrl1;   // Packet automation control.
  uint8 pktctrl0;   // Packet automation control.
  uint8 addr;       // Device address.
  uint8 pktlen;     // Packet length.
} hal_rf_config;

// ���ݸ��������ýṹ��͹��ʱ�������Ƶģ��
void  halRfConfig(const hal_rf_config *rfConfig, const uint8 *rfPaTable, uint8 rfPaTableLen);
// ʹ����������͹��ʱ�������Ƶģ�飨���չ̶���˳��
void  halRfBurstConfig(const uint8 *rfConfig, const uint8 *rfPaTable, uint8 rfPaTableLen);
void  halRfReset(void); // ��Ƶģ���ʼ��

//uint8 halRfGetId(void); // ��ȡ��Ƶģ��ID
#define halRfGetId() halRfReadStatusReg(CC1101_PARTNUM)

//uint8 halRfGetVer(void); // ��ȡ��Ƶģ��汾��
#define halRfGetVer() halRfReadStatusReg(CC1101_VERSION)

uint8 halRfReadStatusReg(uint8 addr); // ����Ƶģ��״̬�Ĵ�����ͻ������λΪ1��
uint8 halRfReadReg(uint8 addr); // ����Ƶģ��Ĵ�����ͻ������λΪ0��

uint8 halRfWriteReg(uint8 addr, uint8 data); // ����Ƶģ��д�Ĵ���

uint8 halRfWriteFifo(uint8 *data, uint8 length); // дFIFO�Ĵ���

uint8 halRfReadFifo(uint8 *data, uint8 length); // ��FIFO�Ĵ���

//uint8 halRfStrobe(uint8 cmd); // ����ָ��
#define halRfStrobe(cmd) halSpiStrobe((cmd))

//uint8 halRfGetTxStatus(void); // ��ȡ����ģʽ��״̬
#define halRfGetTxStatus() halSpiStrobe(CC1101_SNOP)

//uint8 halRfGetRxStatus(void); // ��ȡ����ģʽ��״̬
#define halRfGetRxStatus() halSpiStrobe(CC1101_SNOP | CC1101_READ_SINGLE)

#endif
