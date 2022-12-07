/****************************************************************************
 * Arduino driver for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * Credit to Ryan Young's https://github.com/ryanayoung/projectSpigot project
 * and Cirque for example cade and inspiration.
 *
 * My driver is quite simple compared to Ryan's. If you want to see more
 * comprehensive utilization of the hardware, please visit his repo.
 *
 * 2022-12-06 - Original.
 ****************************************************************************/

#include <CirquePinnacle.h>

CirquePinnacle::CirquePinnacle(uint8_t zIdleCount,  data_mode_t dataMode, bool yInvert)
  : z_idle_count(zIdleCount), data_mode(dataMode), y_invert(yInvert) { }

CirquePinnacle::~CirquePinnacle() { }


void CirquePinnacle::Set_Config_Values(uint8_t cfgFeed1, uint8_t cfgFeed2) {
  cfg_feed1 = cfgFeed1;
  cfg_feed2 = cfgFeed2;
  use_cfg_values = true;
}

void CirquePinnacle::begin(int8_t dataReadyPin) {
  data_ready_pin = dataReadyPin;
  if (data_ready_pin >= 0) pinMode(data_ready_pin, INPUT);
  if (use_cfg_values) {
    Pinnacle_Init(false);
  } else {
    Pinnacle_Init(); // use pre-configured values with c'tor overrides
  }
}

// this init routine uses pre-configured values with c'tor overrides applied
void CirquePinnacle::Pinnacle_Init(void) {
  // clear SW_CC & SW_DR flags
  ClearFlags();
    // set feed cfg 1 register value
  uint8_t feed1 = (data_mode == DATA_MODE_ABS) ? PINNACLE_VAL_FEED1_CFG_ABS1 : PINNACLE_VAL_FEED1_CFG_REL;
  SetFlag(feed1, PINNACLE_FLG_FEED1_Y_DATA_INVERT, y_invert);
  SetFlag(feed1, PINNACLE_FLG_FEED1_DATA_MODE, data_mode);
    // set z-idle packet count
  RAP_Write(PINNACLE_REG_Z_IDLE, z_idle_count);
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
  // set z-idle packet count
  RAP_Write(PINNACLE_REG_Z_IDLE, z_idle_count);
  // feed cfg 2
  RAP_Write(PINNACLE_REG_FEED_CONFIG_2, cfg_feed2);
  // feed cfg 1
  RAP_Write(PINNACLE_REG_FEED_CONFIG_1, cfg_feed1); // set feed enable last
}

// Reads XYZ data from Pinnacle registers 0x14 through 0x17
// Stores result in absData_t struct with xValue, yValue, and zValue members
void CirquePinnacle::GetAbsoluteData(absData_t& absData) {
  if (data_mode == DATA_MODE_ABS) {
    uint8_t data[PINNACLE_TRACKPAD_ABS_DATA_LEN];
    RAP_ReadBytes(PINNACLE_REG_TRACKPAD_DATA, data, PINNACLE_TRACKPAD_ABS_DATA_LEN);
    ClearFlags();
    absData.buttonFlags = data[0] & 0x3F;
    absData.xValue = data[2] | ((data[4] & 0x0F) << 8);
    absData.yValue = data[3] | ((data[4] & 0xF0) << 4);
    absData.zValue = data[5] & 0x3F;
    absData.touchDown = absData.xValue != 0;
  }
}

void CirquePinnacle::GetRelativeData(relData_t& relData) {
  if (data_mode == DATA_MODE_REL) {
    uint8_t data[PINNACLE_TRACKPAD_REL_DATA_LEN];
    RAP_ReadBytes(PINNACLE_REG_TRACKPAD_DATA, data, PINNACLE_TRACKPAD_REL_DATA_LEN);
    ClearFlags();
    relData.buttons = data[0] & 0x7;
    relData.x       = (int8_t)data[1];
    relData.y       = (int8_t)data[2];
    relData.scroll  = (int8_t)data[3];
  }
}

// Clears Status1 register flags (SW_CC and SW_DR)
void CirquePinnacle::ClearFlags() {
  RAP_Write(PINNACLE_REG_STATUS, PINNACLE_VAL_STATUS_CLEAR);
  delayMicroseconds(50);
}

// Enables/Disables the feed
void CirquePinnacle::EnableFeed(bool feedEnable) {
  uint8_t feed1;
  RAP_ReadBytes(PINNACLE_REG_FEED_CONFIG_1, &feed1, 1);  // Get contents of FeedConfig1 register
  SetFlag(feed1, PINNACLE_FLG_FEED1_FEED_ENABLE, feedEnable);
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
  if (data_ready_pin >= 0) return digitalRead(data_ready_pin); // active high
  // else check Status register
  uint8_t status;
  RAP_ReadBytes(PINNACLE_REG_STATUS, &status, 1);
  return status & PINNACLE_FLG_STATUS_DATA_READY;
}

void CirquePinnacle::Invert_Y(bool invert) {
  uint8_t feed1;
  RAP_ReadBytes(PINNACLE_REG_FEED_CONFIG_1, &feed1, 1);
  SetFlag(feed1, PINNACLE_FLG_FEED1_Y_DATA_INVERT, invert);
  RAP_Write(PINNACLE_REG_FEED_CONFIG_1, feed1);
}

void CirquePinnacle::Get_ID(uint8_t& chip_id, uint8_t& firmware_version, uint8_t& product_id) {
  RAP_ReadBytes(PINNACLE_REG_ASIC_ID, & chip_id, 1);
  RAP_ReadBytes(PINNACLE_REG_FIRMWARE_VERSION, & firmware_version, 1);
  RAP_ReadBytes(PINNACLE_REG_PRODUCT_ID, & product_id, 1);
}

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
