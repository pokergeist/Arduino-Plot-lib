/****************************************************************************
 * Arduino SPI driver for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * 2022-12-06 - Original.
 ****************************************************************************/

#include <CirquePinnacleSPI.h>

CirquePinnacleSPI::CirquePinnacleSPI(data_mode_t dataMode, uint8_t zIdleCount,  bool yInvert)
      : CirquePinnacle(dataMode, zIdleCount, yInvert) { }

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
void CirquePinnacleSPI::RAP_ReadBytes(pinnacle_register_t register_addr, uint8_t *data, uint8_t count) {
  SPI.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE1));
  digitalWrite(select_pin, LOW);
  SPI.transfer(0xA0 | register_addr);
  SPI.transfer(0xFC);
  SPI.transfer(0xFC);
  for (byte i=0 ; i<count ; i++) {
    data[i] = SPI.transfer(0xFC);
  }
  digitalWrite(select_pin, HIGH);
  SPI.endTransaction();
}

// Writes single-byte <data> to <address>
void CirquePinnacleSPI::RAP_Write(pinnacle_register_t register_addr, uint8_t data) {
  SPI.beginTransaction(SPISettings(spi_speed, MSBFIRST, SPI_MODE1));
  digitalWrite(select_pin, LOW);
  SPI.transfer((uint8_t)(0x80 | register_addr));
  SPI.transfer(data);
  digitalWrite(select_pin, HIGH);
  SPI.endTransaction();
}

void CirquePinnacleSPI::Set_RAP_Callbacks(uint8_t isr_number) {
  isr_data[isr_number].spi_speed    = spi_speed;
  isr_data[isr_number].rap_read_cb  = RAP_ReadBytes_INT;
  isr_data[isr_number].rap_write_cb = RAP_Write_INT;
}

/*
 * Static methods
 */

void CirquePinnacleSPI::RAP_ReadBytes_INT(uint8_t isr_number, pinnacle_register_t register_addr,
                                          uint8_t* data, uint8_t count) {
  uint8_t pin_addr = isr_data[isr_number].pin_addr;
  SPI.beginTransaction(SPISettings(isr_data[isr_number].spi_speed, MSBFIRST, SPI_MODE1));
  digitalWrite(pin_addr, LOW);
  SPI.transfer(0xA0 | register_addr);
  SPI.transfer(0xFC);
  SPI.transfer(0xFC);
  for (byte i=0 ; i<count ; i++) {
    data[i] = SPI.transfer(0xFC);
  }
  digitalWrite(pin_addr, HIGH);
  SPI.endTransaction();
}

void CirquePinnacleSPI::RAP_Write_INT(uint8_t isr_number, pinnacle_register_t register_addr, uint8_t data) {
  uint8_t pin_addr = isr_data[isr_number].pin_addr;
  SPI.beginTransaction(SPISettings(isr_data[isr_number].spi_speed, MSBFIRST, SPI_MODE1));
  digitalWrite(pin_addr, LOW);
  SPI.transfer((uint8_t)(0x80 | register_addr));
  SPI.transfer(data);
  digitalWrite(pin_addr, HIGH);
  SPI.endTransaction();
}

// CirquePinnacleSPI.cpp
