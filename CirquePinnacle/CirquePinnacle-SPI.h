/****************************************************************************
 * Arduino SPI driver for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * 2022-12-06 - Original.
 ****************************************************************************/

#ifndef CIRQUE_PINNACLE_SPI_H
#define CIRQUE_PINNACLE_SPI_H

#include <Arduino.h>
#include <SPI.h>
#include <CirquePinnacle.h>

// Cirque's 7-bit I2C Slave Address
#define CIRQUE_PINNACLE_DEFAULT_ADDR    0x2A

class CirquePinnacleSPI : public virtual CirquePinnacle {
  uint8_t  select_pin;
  uint32_t spi_speed;

  void RAP_ReadBytes(pinnacle_register_t address, uint8_t* data, uint8_t count);
  void RAP_Write    (pinnacle_register_t address, uint8_t  data);
  void Set_RAP_Callbacks(uint8_t isr_number);

public:
   CirquePinnacleSPI(data_mode_t data_mode, uint8_t z_idle_count=Z_IDLE_COUNT, bool y_invert=false);
  ~CirquePinnacleSPI();
  uint8_t begin(int8_t data_ready_pin, uint8_t select_pin, uint32_t spi_speed);

  // static methods
  static void RAP_ReadBytes_INT(uint8_t isr_number, pinnacle_register_t address, uint8_t* data, uint8_t count);
  static void RAP_Write_INT(uint8_t isr_number, pinnacle_register_t address, uint8_t data);

}; // class CirquePinnacleSPI


#endif // _H

// CirquePinnacleSPI.h
