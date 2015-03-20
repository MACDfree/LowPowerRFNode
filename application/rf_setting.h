#ifndef hal_rf_setting_h
#define hal_rf_setting_h

#include "hal_types.h"
#include "hal_rf.h"

/*
const hal_rf_config rf_setting1 = 
{
  0x08,   // FSCTRL1   Frequency synthesizer control.
  0x00,   // FSCTRL0   Frequency synthesizer control.
  0x10,   // FREQ2     Frequency control word, high byte.
  0xA7,   // FREQ1     Frequency control word, middle byte.
  0x62,   // FREQ0     Frequency control word, low byte.
  0x5B,   // MDMCFG4   Modem configuration.
  0xF8,   // MDMCFG3   Modem configuration.
  0x03,   // MDMCFG2   Modem configuration.
  0x22,   // MDMCFG1   Modem configuration.
  0xF8,   // MDMCFG0   Modem configuration.

  0x00,   // CHANNR    Channel number.
  0x47,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
  0xB6,   // FREND1    Front end RX configuration.
  0x10,   // FREND0    Front end RX configuration.
  0x18,   // MCSM0     Main Radio Control State Machine configuration.
  0x1D,   // FOCCFG    Frequency Offset Compensation Configuration.
  0x1C,   // BSCFG     Bit synchronization Configuration.
  0xC7,   // AGCCTRL2  AGC control.
  0x00,   // AGCCTRL1  AGC control.
  0xB2,   // AGCCTRL0  AGC control.

  0xEA,   // FSCAL3    Frequency synthesizer calibration.
  0x2A,   // FSCAL2    Frequency synthesizer calibration.
  0x00,   // FSCAL1    Frequency synthesizer calibration.
  0x11,   // FSCAL0    Frequency synthesizer calibration.
  0x59,   // FSTEST    Frequency synthesizer calibration.
  0x81,   // TEST2     Various test settings.
  0x35,   // TEST1     Various test settings.
  0x09,   // TEST0     Various test settings.
  0x0B,   // IOCFG2    GDO2 output pin configuration.
  0x06,   // IOCFG0D   GDO0 output pin configuration. Refer to SmartRF?Studio User Manual for detailed pseudo register explanation.

  0x06,   // PKTCTRL1  Packet automation control.
  0x05,   // PKTCTRL0  Packet automation control.
  0x02,   // ADDR      Device address.
  0x0f    // PKTLEN    Packet length.
};*/

// 434MHz, 2.4kb/s
const hal_rf_config rf_setting2 = 
{
  0x06,   // FSCTRL1   Frequency synthesizer control.
  0x00,   // FSCTRL0   Frequency synthesizer control.
  0x10,   // FREQ2     Frequency control word, high byte.
  0xb1,   // FREQ1     Frequency control word, middle byte.
  0x3b,   // FREQ0     Frequency control word, low byte.
  0xf6,   // MDMCFG4   Modem configuration.
  0x83,   // MDMCFG3   Modem configuration.
  0x13,   // MDMCFG2   Modem configuration.
  0x22,   // MDMCFG1   Modem configuration.
  0xf8,   // MDMCFG0   Modem configuration.

  0x00,   // CHANNR    Channel number.
  0x15,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
  0x56,   // FREND1    Front end RX configuration.
  0x10,   // FREND0    Front end RX configuration.
  0x18,   // MCSM0     Main Radio Control State Machine configuration.
  0x16,   // FOCCFG    Frequency Offset Compensation Configuration.
  0x6C,   // BSCFG     Bit synchronization Configuration.
  0x03,   // AGCCTRL2  AGC control.
  0x40,   // AGCCTRL1  AGC control.
  0x91,   // AGCCTRL0  AGC control.

  0xE9,   // FSCAL3    Frequency synthesizer calibration.
  0x2A,   // FSCAL2    Frequency synthesizer calibration.
  0x00,   // FSCAL1    Frequency synthesizer calibration.
  0x1f,   // FSCAL0    Frequency synthesizer calibration.
  0x59,   // FSTEST    Frequency synthesizer calibration.
  0x81,   // TEST2     Various test settings.
  0x35,   // TEST1     Various test settings.
  0x09,   // TEST0     Various test settings.
  0x0B,   // IOCFG2    GDO2 output pin configuration.
  0x06,   // IOCFG0    GDO0 output pin configuration. Refer to SmartRF?Studio User Manual for detailed pseudo register explanation.

  0x06,   // PKTCTRL1  Packet automation control.
  0x05,   // PKTCTRL0  Packet automation control.
  0x02,   // ADDR      Device address.
  0x0f    // PKTLEN    Packet length.
};

// 434MHz 250kb/s
const hal_rf_config rf_setting3 = 
{
  0x0c,   // FSCTRL1   Frequency synthesizer control.
  0x00,   // FSCTRL0   Frequency synthesizer control.
  0x10,   // FREQ2     Frequency control word, high byte.
  0xb1,   // FREQ1     Frequency control word, middle byte.
  0x3b,   // FREQ0     Frequency control word, low byte.
  0x2d,   // MDMCFG4   Modem configuration.
  0x3b,   // MDMCFG3   Modem configuration.
  0x13,   // MDMCFG2   Modem configuration.
  0x22,   // MDMCFG1   Modem configuration.
  0xf8,   // MDMCFG0   Modem configuration.

  0x00,   // CHANNR    Channel number.
  0x62,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
  0xb6,   // FREND1    Front end RX configuration.
  0x10,   // FREND0    Front end RX configuration.
  0x18,   // MCSM0     Main Radio Control State Machine configuration.
  0x1d,   // FOCCFG    Frequency Offset Compensation Configuration.
  0x1C,   // BSCFG     Bit synchronization Configuration.
  0x07,   // AGCCTRL2  AGC control.
  0x00,   // AGCCTRL1  AGC control.
  0xb0,   // AGCCTRL0  AGC control.

  0xEa,   // FSCAL3    Frequency synthesizer calibration.
  0x2A,   // FSCAL2    Frequency synthesizer calibration.
  0x00,   // FSCAL1    Frequency synthesizer calibration.
  0x1f,   // FSCAL0    Frequency synthesizer calibration.
  0x59,   // FSTEST    Frequency synthesizer calibration.
  0x88,   // TEST2     Various test settings.
  0x31,   // TEST1     Various test settings.
  0x09,   // TEST0     Various test settings.
  0x0B,   // IOCFG2    GDO2 output pin configuration.
  0x06,   // IOCFG0    GDO0 output pin configuration. Refer to SmartRF?Studio User Manual for detailed pseudo register explanation.

  0x06,   // PKTCTRL1  Packet automation control.
  0x05,   // PKTCTRL0  Packet automation control.
  0x02,   // ADDR      Device address.
  0x0f    // PKTLEN    Packet length.
};

const uint8 myPaTable1[8] = {0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04};
const uint8 myPaTable2[8] = {0x60,0x60,0x60,0x60,0x60,0x60,0x60,0x60};
const uint8 myPaTable3[8] = {0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0,0xc0};

#endif