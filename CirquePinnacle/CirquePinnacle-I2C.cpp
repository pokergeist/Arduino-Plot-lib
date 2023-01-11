/****************************************************************************
 * Arduino I2C driver for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * 2022-12-06 - Original.
 ****************************************************************************/

#include <CirquePinnacle-I2C.h>

CirquePinnacleI2C::CirquePinnacleI2C(data_mode_t dataMode, uint8_t zIdleCount,  bool yInvert)
      : CirquePinnacle(dataMode, zIdleCount, yInvert) { }

CirquePinnacleI2C::~CirquePinnacleI2C() { }

uint8_t CirquePinnacleI2C::begin(int8_t dataReadyPin, int8_t i2c_address, uint32_t clockSpeed) {
  CirquePinnacle::begin(dataReadyPin); // mod if other than 0 is ever returned
  i2c_addr = i2c_address;              // otherwise, a fault implies I2C ping failure
  Wire.begin(i2c_addr);
  Wire.setClock(clockSpeed);
  // i2c ping
  Wire.beginTransmission(i2c_addr);
  int status = Wire.endTransmission();
  return status;
}

/*  RAP Functions */

// Reads <count> Pinnacle registers starting at <register_addr>
void CirquePinnacleI2C::RAP_ReadBytes(pinnacle_register_t register_addr, uint8_t* data, uint8_t count) {
  uint8_t cmdByte = READ_MASK | register_addr;   // Form the READ command byte
  uint8_t i = 0;
  Wire.beginTransmission(i2c_addr);   // Set up an I2C-write to the I2C slave (Pinnacle)
  Wire.write(cmdByte);                // Signal a RAP-read operation starting at <register_addr>
  Wire.endTransmission(true);         // I2C stop condition
  Wire.requestFrom((uint8_t)i2c_addr, count, (uint8_t)true);  // Read <count> bytes from I2C slave
  while(Wire.available()) {
    data[i++] = Wire.read();
  }
}

// Writes single-byte <data> to <register_addr>
void CirquePinnacleI2C::RAP_Write(pinnacle_register_t register_addr, uint8_t data) {
  uint8_t cmdByte = WRITE_MASK | register_addr;  // Form the WRITE command byte
  Wire.beginTransmission(i2c_addr); // Set up an I2C-write to the I2C slave (Pinnacle)
  Wire.write(cmdByte);              // Signal a RAP-write operation at <register_addr>
  Wire.write(data);                 // Write <data> to I2C slave
  Wire.endTransmission();           // I2C w/ stop condition
}

void CirquePinnacleI2C::Set_RAP_Callbacks(uint8_t isr_number) {
  isr_data[isr_number].rap_read_cb  = RAP_ReadBytes_INT;
  isr_data[isr_number].rap_write_cb = RAP_Write_INT;
}

/*
 * Static methods
 */

// Reads <count> Pinnacle registers starting at <register_addr>
void CirquePinnacleI2C::RAP_ReadBytes_INT(uint8_t isr_number, pinnacle_register_t register_addr,
                                          uint8_t* data, uint8_t count) {
  uint8_t pin_addr = isr_data[isr_number].pin_addr;
  uint8_t cmdByte = READ_MASK | register_addr;   // Form the READ command byte
  uint8_t i = 0;
  Wire.beginTransmission(pin_addr);   // Set up an I2C-write to the I2C slave (Pinnacle)
  Wire.write(cmdByte);                // Signal a RAP-read operation starting at <register_addr>
  Wire.endTransmission(true);         // I2C stop condition
  Wire.requestFrom((uint8_t)pin_addr, count, (uint8_t)true);  // Read <count> bytes from I2C slave
  while(Wire.available()) {
    data[i++] = Wire.read();
  }
}

// Writes single-byte <data> to <register_addr>
void CirquePinnacleI2C::RAP_Write_INT(uint8_t isr_number, pinnacle_register_t register_addr, uint8_t data) {
  uint8_t pin_addr = isr_data[isr_number].pin_addr;
  uint8_t cmdByte = WRITE_MASK | register_addr;  // Form the WRITE command byte
  Wire.beginTransmission(pin_addr); // Set up an I2C-write to the I2C slave (Pinnacle)
  Wire.write(cmdByte);              // Signal a RAP-write operation at <register_addr>
  Wire.write(data);                 // Write <data> to I2C slave
  Wire.endTransmission();           // I2C w/ stop condition
}

// CirquePinnacleI2C.cpp
