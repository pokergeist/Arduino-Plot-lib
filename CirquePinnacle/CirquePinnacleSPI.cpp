/****************************************************************************
 * Arduino SPI driver for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * 2022-12-06 - Original.
 ****************************************************************************/

#include <CirquePinnacleSPI.h>

CirquePinnacleSPI::CirquePinnacleSPI(uint8_t zIdleCount,  data_mode_t dataMode, bool yInvert)
  : CirquePinnacle(zIdleCount, dataMode, yInvert) { }

CirquePinnacleSPI::~CirquePinnacleSPI() { }

uint8_t CirquePinnacleSPI::begin(int8_t dataReadyPin, uint8_t selectPin, uint32_t spiSpeed) {
  select_pin = selectPin;
  spi_speed  = spiSpeed;
  pinMode(select_pin, OUTPUT);
  SPI.begin();
  return CirquePinnacle::begin(dataReadyPin);
}

/*  RAP Functions */

// Reads <count> Pinnacle registers starting at <address>
void CirquePinnacleSPI::RAP_ReadBytes(pinnacle_register_t registerAddress, uint8_t *data, uint8_t count) {
  SPI.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE1));
  digitalWrite(select_pin, LOW);
  SPI.transfer(0xA0 | registerAddress);
  SPI.transfer(0xFC);
  SPI.transfer(0xFC);
  for (byte i=0 ; i<count ; i++) {
    data[i] = SPI.transfer(0xFC);
  }
  digitalWrite(select_pin, HIGH);
  SPI.endTransaction();
}

// Writes single-byte <data> to <address>
void CirquePinnacleSPI::RAP_Write(pinnacle_register_t registerAddress, uint8_t data) {
  SPI.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE1));
  digitalWrite(select_pin, LOW);
  SPI.transfer((uint8_t)(0x80 | registerAddress));
  SPI.transfer(data);
  digitalWrite(select_pin, HIGH);
  SPI.endTransaction();
}

// CirquePinnacleSPI.cpp
