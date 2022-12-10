/****************************************************************************
 * Arduino driver for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * Credit to Ryan Young's https://github.com/ryanayoung/projectSpigot project
 * and Cirque for example cade and inspiration.
 *
 * My driver is quite simple compared to Ryan's. If you want to see more
 * comprehensive utilization of the hardware, please visit his repo.
 *
 * 2022-12-10 - Added interrupt capability.
 * 2022-12-06 - Original.
 ****************************************************************************/

#include <CirquePinnacle.h>

/*
 * Statuc variables
 */

uint8_t    CirquePinnacle::isr_enumerator;    // one-up number to enumerate the instance read callback data
isr_data_t CirquePinnacle::isr_data[MAX_ISRS];

// 'tors
CirquePinnacle::CirquePinnacle(data_mode_t dataMode, uint8_t zIdleCount,  bool yInvert)
  : data_mode(dataMode), z_idle_count(zIdleCount), y_invert(yInvert) { }
CirquePinnacle::~CirquePinnacle() { }

void CirquePinnacle::Set_Config_Values(uint8_t cfgFeed1, uint8_t cfgFeed2) {
  cfg_feed1 = cfgFeed1;
  cfg_feed2 = cfgFeed2;
  use_cfg_values = true;
}

uint8_t CirquePinnacle::begin(int8_t dataReadyPin) {
  data_ready_pin = dataReadyPin;
  if (data_ready_pin >= 0) pinMode(data_ready_pin, INPUT);
  if (use_cfg_values) {
    Pinnacle_Init(false);
  } else {
    Pinnacle_Init(); // use pre-configured values with c'tor overrides
  }
  return 0;
}

// this init routine uses pre-configured values with c'tor overrides applied
void CirquePinnacle::Pinnacle_Init(void) {
  // clear SW_CC & SW_DR flags
  ClearFlags();
    // set feed cfg 1 register value
  uint8_t feed1 = (data_mode == DATA_MODE_ABS) ? PINNACLE_VAL_FEED1_CFG_ABS1 : PINNACLE_VAL_FEED1_CFG_REL;
  SetFlag(feed1, PINNACLE_FLG_FEED1_Y_DATA_INVERT, y_invert);
  SetFlag(feed1, PINNACLE_FLG_FEED1_DATA_MODE, data_mode);
    // set z-idle packet count & speed
  RAP_Write(PINNACLE_REG_Z_IDLE, z_idle_count);
  Set_Speed(cfg_speed);
    // set feed cfg 2 register value and apply
  uint8_t feed2 = (data_mode == DATA_MODE_ABS) ? PINNACLE_VAL_FEED2_CFG_ABS  : PINNACLE_VAL_FEED2_CFG_REL;
  RAP_Write(PINNACLE_REG_FEED_CONFIG_2, feed2);
    // apply feed cfg 1 register value
  RAP_Write(PINNACLE_REG_FEED_CONFIG_1, feed1); // set feed enable last
}

// this init routine uses values privided via SetFlag()
void CirquePinnacle::Pinnacle_Init(bool disableFeed) {
  if (disableFeed) EnableFeed(false);
  // set data mode from config data
  data_mode = (cfg_feed1 & PINNACLE_FLG_FEED1_DATA_MODE);
  // clear SW_CC & SW_DR flags
  ClearFlags();
  // set z-idle packet count & speed
  RAP_Write(PINNACLE_REG_Z_IDLE, z_idle_count);
  Set_Speed(cfg_speed);
  // feed cfg 2
  RAP_Write(PINNACLE_REG_FEED_CONFIG_2, cfg_feed2);
  // feed cfg 1
  RAP_Write(PINNACLE_REG_FEED_CONFIG_1, cfg_feed1); // set feed enable last
}

void CirquePinnacle::Get_Data(trackpad_data_t& trackpad_data) {
  uint8_t data_len = (data_mode == DATA_MODE_ABS)
                     ? PINNACLE_TRACKPAD_ABS_DATA_LEN : PINNACLE_TRACKPAD_REL_DATA_LEN;
  uint8_t raw_data[CP_RAW_DATA_LEN];
  RAP_ReadBytes(PINNACLE_REG_TRACKPAD_DATA, raw_data, data_len);
  ClearFlags();
  Decode_Data(raw_data, data_len, trackpad_data);
}

// Clears Status1 register flags (SW_CC and SW_DR)
void CirquePinnacle::ClearFlags() {
  RAP_Write(PINNACLE_REG_STATUS, PINNACLE_VAL_STATUS_CLEAR);
  delayMicroseconds(50);
}

// Enables/Disables the feed
void CirquePinnacle::EnableFeed(bool enableFeed) {
  uint8_t feed1;
  RAP_ReadBytes(PINNACLE_REG_FEED_CONFIG_1, &feed1, 1);  // Get contents of FeedConfig1 register
  SetFlag(feed1, PINNACLE_FLG_FEED1_FEED_ENABLE, enableFeed);
  RAP_Write(PINNACLE_REG_FEED_CONFIG_1, feed1);
}

/*  ERA (Extended Register Access) Functions  */
// Reads <count> bytes from an extended register at <address> (16-bit address),
// stores values in <*data>
void CirquePinnacle::ERA_ReadBytes(uint16_t address, uint8_t* data, uint16_t count) {
  uint8_t ERAControlValue = 0xFF;
  EnableFeed(false); // Disable feed
  RAP_Write(PINNACLE_REG_ERA_ADDR_HIGH, (uint8_t)(address >> 8));     // Send upper byte of ERA address
  RAP_Write(PINNACLE_REG_ERA_ADDR_LOW,  (uint8_t)(address & 0x00FF)); // Send lower byte of ERA address
  for(uint16_t i = 0; i < count; i++) {
    RAP_Write(PINNACLE_REG_ERA_CONTROL, PINNACLE_VAL_ERA_CONTROL_CFG_RD_RDAI);  // Signal ERA-read (auto-increment)
    // Wait for status register 0x1E to clear
    do {
      RAP_ReadBytes(PINNACLE_REG_ERA_CONTROL, &ERAControlValue, 1);
    } while(ERAControlValue != 0x00);
    RAP_ReadBytes(PINNACLE_REG_ERA_VALUE, data + i, 1);
    ClearFlags();
  }
}

// Writes a byte, <data>, to an extended register at <address> (16-bit address)
void CirquePinnacle::ERA_WriteByte(uint16_t address, uint8_t data) {
  uint8_t ERAControlValue = 0xFF;
  EnableFeed(false); // Disable feed
  RAP_Write(PINNACLE_REG_ERA_VALUE, data);      // Send data byte to be written

  RAP_Write(PINNACLE_REG_ERA_ADDR_HIGH, (uint8_t)(address >> 8));       // Upper byte of ERA address
  RAP_Write(PINNACLE_REG_ERA_ADDR_LOW,  (uint8_t)(address & 0x00FF));   // Lower byte of ERA address
  RAP_Write(PINNACLE_REG_ERA_CONTROL, PINNACLE_FLG_ERA_CONTROL_WRITE);  // Signal an ERA-write to Pinnacle
  // Wait for status register 0x1E to clear
  do {
    RAP_ReadBytes(PINNACLE_REG_ERA_CONTROL, &ERAControlValue, 1);
  } while(ERAControlValue != 0x00);
  ClearFlags();
}

/*  Logical Scaling Functions */
// Clips raw coordinates to "reachable" window of sensor
// NOTE: values outside this window can only appear as a result of noise
void CirquePinnacle::ClipCoordinates(absData_t& coordinates) {
  if(coordinates.xValue < PINNACLE_X_LOWER)   {
    coordinates.xValue = PINNACLE_X_LOWER;
  } else if(coordinates.xValue > PINNACLE_X_UPPER) {
    coordinates.xValue = PINNACLE_X_UPPER;
  }
  if(coordinates.yValue < PINNACLE_Y_LOWER) {
    coordinates.yValue = PINNACLE_Y_LOWER;
  } else if(coordinates.yValue > PINNACLE_Y_UPPER) {
    coordinates.yValue = PINNACLE_Y_UPPER;
  }
}

// Scales data to desired X & Y resolution
void CirquePinnacle::ScaleData(absData_t& coordinates, uint16_t xResolution, uint16_t yResolution) {
  uint32_t xTemp = 0;
  uint32_t yTemp = 0;

  ClipCoordinates(coordinates);

  xTemp = coordinates.xValue;
  yTemp = coordinates.yValue;

  // translate coordinates to (0, 0) reference by subtracting edge-offset
  xTemp -= PINNACLE_X_LOWER;
  yTemp -= PINNACLE_Y_LOWER;

  // scale coordinates to (xResolution, yResolution) range
  coordinates.xValue = (uint16_t)(xTemp * xResolution / PINNACLE_X_RANGE);
  coordinates.yValue = (uint16_t)(yTemp * yResolution / PINNACLE_Y_RANGE);
}

bool CirquePinnacle::Data_Ready() {
  if (isr_in_use) {
    // read ISR flag
    return isr_data[my_isr_num].data_ready;
  } else if (data_ready_pin >= 0) {
    // read DR gpio pin
    return digitalRead(data_ready_pin); // active high
  } else {
    // read Status register flag
    uint8_t status;
    RAP_ReadBytes(PINNACLE_REG_STATUS, &status, 1);
    return status & PINNACLE_FLG_STATUS_DATA_READY;
  }
}

void CirquePinnacle::Invert_Y(bool invert) {
  uint8_t feed1;
  RAP_ReadBytes(PINNACLE_REG_FEED_CONFIG_1, &feed1, 1);
  SetFlag(feed1, PINNACLE_FLG_FEED1_Y_DATA_INVERT, invert);
  RAP_Write(PINNACLE_REG_FEED_CONFIG_1, feed1);
}

void CirquePinnacle::Set_Speed(cp_speed_t speed) {
  cfg_speed = speed;
  RAP_Write(PINNACLE_REG_SAMPLE_RATE, cfg_speed);
}

void CirquePinnacle::Get_ID(uint8_t& chip_id, uint8_t& firmware_version, uint8_t& product_id) {
  RAP_ReadBytes(PINNACLE_REG_ASIC_ID, & chip_id, 1);
  RAP_ReadBytes(PINNACLE_REG_FIRMWARE_VERSION, & firmware_version, 1);
  RAP_ReadBytes(PINNACLE_REG_PRODUCT_ID, & product_id, 1);
}

cp_error_t CirquePinnacle::Start_ISR(uint8_t pin_addr, trackpad_data_t& trackpadData) {
  if (data_ready_pin < 0) return E_BAD_PIN; // no interrupt pin
  if (isr_enumerator > MAX_ISRS) return E_OUT_OF_ISRS;  // too many ISRs - add callbacks
  my_isr_num = isr_enumerator++;
  isr_data[my_isr_num].pin_addr = pin_addr;     // the SPI select pin or I2C address
  isr_data[my_isr_num].raw_data_len =
      (data_mode == DATA_MODE_ABS) ? PINNACLE_TRACKPAD_ABS_DATA_LEN : PINNACLE_TRACKPAD_REL_DATA_LEN;
  isr_data[my_isr_num].trackpad_data_p = &trackpadData; // pointer to caller's data store
  isr_data[my_isr_num].data_ready = false;      // set when the IRS has populated user data
  Set_RAP_Callbacks(my_isr_num);                // child object sets the RAP callbacks in the ISR data
  isr_in_use = true;                            // check this data_ready flag
  voidFuncPtr theISR;
  switch (my_isr_num) {
  case 0: theISR = Read_Data_ISR_0;
    break;
  case 1: theISR = Read_Data_ISR_1;
    break;
  case 2: theISR = Read_Data_ISR_2;
    break;
  case 3: theISR = Read_Data_ISR_3;
    break;
  default:
    return E_OUT_OF_ISRS;   // bad ISR number
  }
  EnableFeed(false);
  ClearFlags();
  if (digitalPinToInterrupt(data_ready_pin) == NOT_AN_INTERRUPT) return E_NOT_AN_INTERRUPT;
  attachInterrupt(digitalPinToInterrupt(data_ready_pin), theISR, RISING);
  EnableFeed(true);
  return E_OKAY;
}

bool CirquePinnacle::End_ISR(uint8_t isr_number) {
  isr_in_use = false;                            // don't check this data_ready flag
  if (data_ready_pin < 0) return true;
  detachInterrupt(digitalPinToInterrupt(data_ready_pin));
  return true;
}

void CirquePinnacle::Clear_DR(void) {
  isr_data[my_isr_num].data_ready = false;
}

/*
 * static methods
 */

void CirquePinnacle::Decode_Data(uint8_t* raw_data, uint8_t data_len, trackpad_data_t& trackpad_data) {
  if (data_len == PINNACLE_TRACKPAD_ABS_DATA_LEN) {
    // Absolute data
    absData_t& absData = trackpad_data.abs_data;
    absData.buttonFlags = raw_data[0] & 0x3F;
    absData.xValue = raw_data[2] | ((raw_data[4] & 0x0F) << 8);
    absData.yValue = raw_data[3] | ((raw_data[4] & 0xF0) << 4);
    absData.zValue = raw_data[5] & 0x3F;
    absData.touchDown = absData.xValue != 0;
  } else {
    // Relative data
    relData_t& relData = trackpad_data.rel_data;
    relData.buttons = raw_data[0] & 0x7;
    relData.x       = raw_data[1];
    relData.y       = (int8_t)raw_data[2];
    relData.scroll  = (int8_t)raw_data[3];
  }
}

void CirquePinnacle::Read_Data_ISR(uint8_t myISRnum) {
  uint8_t* raw_data = isr_data[myISRnum].raw_data;
  uint8_t data_len  = isr_data[myISRnum].raw_data_len;
  uint8_t pin_addr  = isr_data[myISRnum].pin_addr;
  // read the trackpad data
  isr_data[myISRnum].rap_read_cb(myISRnum, PINNACLE_REG_TRACKPAD_DATA, raw_data, data_len);
  isr_data[myISRnum].trackpad_data_p->timestamp_ms = millis();
  // clear the data ready flag (and HW_DR line)
  isr_data[myISRnum].rap_write_cb(myISRnum, PINNACLE_REG_STATUS, PINNACLE_VAL_STATUS_CLEAR);
  // populate the users data structure
  Decode_Data(raw_data, data_len, *isr_data[myISRnum].trackpad_data_p);
  isr_data[myISRnum].data_ready = true;
}

// ISR callbacks bound to an index (MAX_ISRS)
void CirquePinnacle::Read_Data_ISR_0(void) { Read_Data_ISR(0); }
void CirquePinnacle::Read_Data_ISR_1(void) { Read_Data_ISR(1); }
void CirquePinnacle::Read_Data_ISR_2(void) { Read_Data_ISR(2); }
void CirquePinnacle::Read_Data_ISR_3(void) { Read_Data_ISR(3); }

String CirquePinnacle::Decode_Buttons(uint8_t buttonData) {
  String s;
  if (data_mode == DATA_MODE_REL) {
    s  = ((buttonData & PINNACLE_FLG_REL_BUTTON_PRIMARY)   ?  "PRI" :  "-p-");
    s += ((buttonData & PINNACLE_FLG_REL_BUTTON_SECONDARY) ? " SEC" : " -s-");
    s += ((buttonData & PINNACLE_FLG_REL_BUTTON_AUXILLARY) ? " AUX" : " -a-"); // caller terminates line
  } else {
    s  = ((buttonData & PINNACLE_FLG_ABS_SWITCH_0) ?  "0" :  "-");
    s += ((buttonData & PINNACLE_FLG_ABS_SWITCH_1) ? " 1" : " -");
    s += ((buttonData & PINNACLE_FLG_ABS_SWITCH_2) ? " 2" : " -");
    s += ((buttonData & PINNACLE_FLG_ABS_SWITCH_2) ? " 3" : " -");
    s += ((buttonData & PINNACLE_FLG_ABS_SWITCH_2) ? " 4" : " -");
    s += ((buttonData & PINNACLE_FLG_ABS_SWITCH_2) ? " 5" : " -"); // caller terminates line
  }
  return s; // "return value optimization" - deleted in caller code when exiting scope
}

void CirquePinnacle::SetFlag(uint8_t& value, uint8_t flag, bool test) {
  if (test) value |= flag;
  else value &= ~flag;
}

// CirquePinnacle.cpp
