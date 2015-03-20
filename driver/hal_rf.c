#include <msp430x14x.h>
#include "hal_types.h"
#include "hal_spi.h"
#include "hal_rf.h"
#include "hal_cc1101.h"
#include "delay.h"

void halRfReset(void)
{
  CSN1;
  _delayus(30);
  CSN0;
  _delayus(30);
  CSN1;
  _delayus(45);
  
  CSN0;
  while(SOMI);
  U0TXBUF = CC1101_SRES;
  while(!(U0TCTL & TXEPT));//µÈ´ýTXBUF¿Õ
  while(SOMI);
  CSN1;
}

void halRfConfig(const hal_rf_config *rfConfig, const uint8 *rfPaTable, uint8 rfPaTableLen)
{
  halRfWriteReg(CC1101_FSCTRL1,  rfConfig->fsctrl1);    // Frequency synthesizer control.
  halRfWriteReg(CC1101_FSCTRL0,  rfConfig->fsctrl0);    // Frequency synthesizer control.
  halRfWriteReg(CC1101_FREQ2,    rfConfig->freq2);      // Frequency control word, high byte.
  halRfWriteReg(CC1101_FREQ1,    rfConfig->freq1);      // Frequency control word, middle byte.
  halRfWriteReg(CC1101_FREQ0,    rfConfig->freq0);      // Frequency control word, low byte.
  halRfWriteReg(CC1101_MDMCFG4,  rfConfig->mdmcfg4);    // Modem configuration.
  halRfWriteReg(CC1101_MDMCFG3,  rfConfig->mdmcfg3);    // Modem configuration.
  halRfWriteReg(CC1101_MDMCFG2,  rfConfig->mdmcfg2);    // Modem configuration.
  halRfWriteReg(CC1101_MDMCFG1,  rfConfig->mdmcfg1);    // Modem configuration.
  halRfWriteReg(CC1101_MDMCFG0,  rfConfig->mdmcfg0);    // Modem configuration.
  halRfWriteReg(CC1101_CHANNR,   rfConfig->channr);     // Channel number.
  halRfWriteReg(CC1101_DEVIATN,  rfConfig->deviatn);    // Modem deviation setting (when FSK modulation is enabled).
  halRfWriteReg(CC1101_FREND1,   rfConfig->frend1);     // Front end RX configuration.
  halRfWriteReg(CC1101_FREND0,   rfConfig->frend0);     // Front end RX configuration.
  halRfWriteReg(CC1101_MCSM0,    rfConfig->mcsm0);      // Main Radio Control State Machine configuration.
  halRfWriteReg(CC1101_FOCCFG,   rfConfig->foccfg);     // Frequency Offset Compensation Configuration.
  halRfWriteReg(CC1101_BSCFG,    rfConfig->bscfg);      // Bit synchronization Configuration.
  halRfWriteReg(CC1101_AGCCTRL2, rfConfig->agcctrl2);   // AGC control.
  halRfWriteReg(CC1101_AGCCTRL1, rfConfig->agcctrl1);   // AGC control.
  halRfWriteReg(CC1101_AGCCTRL0, rfConfig->agcctrl0);   // AGC control.
  halRfWriteReg(CC1101_FSCAL3,   rfConfig->fscal3);     // Frequency synthesizer calibration.
  halRfWriteReg(CC1101_FSCAL2,   rfConfig->fscal2);     // Frequency synthesizer calibration.
  halRfWriteReg(CC1101_FSCAL1,   rfConfig->fscal1);     // Frequency synthesizer calibration.
  halRfWriteReg(CC1101_FSCAL0,   rfConfig->fscal0);     // Frequency synthesizer calibration.
  halRfWriteReg(CC1101_FSTEST,   rfConfig->fstest);     // Frequency synthesizer calibration.
  halRfWriteReg(CC1101_TEST2,    rfConfig->test2);      // Various test settings.
  halRfWriteReg(CC1101_TEST1,    rfConfig->test1);      // Various test settings.
  halRfWriteReg(CC1101_TEST0,    rfConfig->test0);      // Various test settings.
  halRfWriteReg(CC1101_IOCFG2,   rfConfig->iocfg2);     // GDO2 output pin configuration.
  halRfWriteReg(CC1101_IOCFG0,   rfConfig->iocfg0);     // GDO0 output pin configuration.
  halRfWriteReg(CC1101_PKTCTRL1, rfConfig->pktctrl1);   // Packet automation control.
  halRfWriteReg(CC1101_PKTCTRL0, rfConfig->pktctrl0);   // Packet automation control.
  halRfWriteReg(CC1101_ADDR,     rfConfig->addr);       // Device address.
  halRfWriteReg(CC1101_PKTLEN,   rfConfig->pktlen);     // Packet length.

  halSpiWrite(CC1101_PATABLE | CC1101_WRITE_BURST, rfPaTable, rfPaTableLen);
}

void halRfBurstConfig(const uint8 *rfConfig, const uint8 *rfPaTable, uint8 rfPaTableLen)
{
  halSpiWrite(CC1101_IOCFG2  | CC1101_WRITE_BURST, rfConfig, 34);
  halSpiWrite(CC1101_PATABLE | CC1101_WRITE_BURST, rfPaTable, rfPaTableLen);
}

uint8 halRfGetId(void)
{
  return (halRfReadStatusReg(CC1101_PARTNUM));
}

uint8 halRfGetVer(void)
{
  return (halRfReadStatusReg(CC1101_VERSION));
}

uint8 halRfStrobe(uint8 cmd)
{
  return (halSpiStrobe(cmd));
}

uint8 halRfReadStatusReg(uint8 addr)
{
  uint8 reg;
  halSpiRead(addr | CC1101_READ_BURST, &reg, 1);
  return reg;
}

uint8 halRfReadReg(uint8 addr)
{
  uint8 reg;
  halSpiRead(addr | CC1101_READ_SINGLE, &reg, 1);
  return reg;
}

uint8 halRfWriteReg(uint8 addr, uint8 data)
{
  uint8 rc;
  rc = halSpiWrite(addr, &data, 1);
  return rc;
}

uint8 halRfWriteFifo(uint8 *data, uint8 length)
{
  return (halSpiWrite(CC1101_TXFIFO | CC1101_WRITE_BURST, data, length));
}

uint8 halRfReadFifo(uint8 *data, uint8 length)
{
  return (halSpiRead(CC1101_RXFIFO | CC1101_READ_BURST, data, length));
}

uint8 halRfGetTxStatus(void)
{
  return (halSpiStrobe(CC1101_SNOP));
}

uint8 halRfGetRxStatus(void)
{
  return (halSpiStrobe(CC1101_SNOP | CC1101_READ_SINGLE));
}
