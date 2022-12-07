/****************************************************************************
 * Arduino I2C driver for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * 2022-12-06 - Original.
 ****************************************************************************/

#ifndef CIRQUE_PINNACLE_I2C_H
#define CIRQUE_PINNACLE_I2C_H

#include <Arduino.h>
#include <Wire.h>
#include <CirquePinnacle.h>

// Cirque's 7-bit I2C Slave Address
#define CIRQUE_PINNACLE_DEFAULT_I2C_ADDRESS   0x2A

class CirquePinnacleI2C : public virtual CirquePinnacle {
  uint8_t i2c_addr;

  void RAP_ReadBytes(pinnacle_register_t address, uint8_t* data, uint8_t count);
  void RAP_Write    (pinnacle_register_t address, uint8_t data);

public:
   CirquePinnacleI2C(uint8_t z_idle_count=Z_IDLE_COUNT, data_mode_t data_mode=DATA_MODE_ABS, bool y_invert=false);
  ~CirquePinnacleI2C();
  uint8_t begin(int8_t dataReadyPin, uint8_t address=CIRQUE_PINNACLE_DEFAULT_I2C_ADDRESS);
}; // class CirquePinnacleI2C

#endif // _H

// CirquePinnacleI2C.h
