/****************************************************************************
 * Arduino I2C driver for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * 2022-12-06 - Original.
 ****************************************************************************/

#include <CirquePinnacleI2C.h>

CirquePinnacleI2C::CirquePinnacleI2C(uint8_t zIdleCount,  data_mode_t dataMode, bool yInvert)
  : CirquePinnacle(zIdleCount, dataMode, yInvert) { }

CirquePinnacleI2C::~CirquePinnacleI2C() { }

void CirquePinnacleI2C::begin(int8_t dataReadyPin, uint8_t address) {
  i2c_addr = address;
  Wire.begin();
  CirquePinnacle::begin(dataReadyPin);
}

/*  RAP Functions */

// Reads <count> Pinnacle registers starting at <address>
void CirquePinnacleI2C::RAP_ReadBytes(pinnacle_register_t address, uint8_t * data, uint8_t count) {
  uint8_t cmdByte = READ_MASK | address;   // Form the READ command byte
  uint8_t i = 0;
  Wire.beginTransmission(i2c_addr);   // Set up an I2C-write to the I2C slave (Pinnacle)
  Wire.write(cmdByte);                  // Signal a RAP-read operation starting at <address>
  Wire.endTransmission(true);           // I2C stop condition
  Wire.requestFrom((uint8_t)i2c_addr, count, (uint8_t)true);  // Read <count> bytes from I2C slave
  while(Wire.available()) {
    data[i++] = Wire.read();
  }
}

// Writes single-byte <data> to <address>
void CirquePinnacleI2C::RAP_Write(pinnacle_register_t address, uint8_t data) {
  uint8_t cmdByte = WRITE_MASK | address;  // Form the WRITE command byte
  Wire.beginTransmission(i2c_addr);   // Set up an I2C-write to the I2C slave (Pinnacle)
  Wire.write(cmdByte);                  // Signal a RAP-write operation at <address>
  Wire.write(data);                     // Write <data> to I2C slave
  Wire.endTransmission();               // I2C w/ stop condition
}

// CirquePinnacleI2C.cpp
