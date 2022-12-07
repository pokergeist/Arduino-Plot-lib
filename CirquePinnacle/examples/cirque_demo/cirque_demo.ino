/****************************************************************************
 * Arduino demo sketch for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * Both SPI and I2C interfaces are supported, as are Absolute and Relative
 * data reads.
 *
 * Credit to Ryan Young's https://github.com/ryanayoung/projectSpigot project
 * and Cirque for example cade and inspiration.
 *
 * 2022-12-06 - Original.
 ****************************************************************************/

/****************************************************************************
 * Note: I2C Warning :: no pull-ups.
 * I tested with a model TM035035-2024 GlidePoint. I removed R1 for I2C
 * but neither SCL nor SDA is pulled high to 3.3V. There are no external
 * provisions for adding pull-up resistors, nor are those lines connected
 * to a GPIO pin that could be pulled high. They use 2.2k resistors in a
 * reference schematic. I used 1k resistors without issue.
 ****************************************************************************/

// Select the interface being used
#define USING_SPI
// #define USING_I2C

// for my QT Cirque board with an Adafruit QT Py (#4600) MCU and an FFC-12 connector
#define SPI_SELECT_PIN           0
#define CIRQUE_DATA_READY_PIN    1    // or -1 if not wired
#define SPI_SPEED_MAX         10e6

#ifdef USING_SPI
  #include <CirquePinnacleSPI.h>
#else
  #include <CirquePinnacleI2C.h>
#endif

// Select the preferred data mode, 1 or true for Absolute, 0 or false for Relative
data_mode_t data_mode = (1) ? DATA_MODE_ABS : DATA_MODE_REL;

#include <Streaming.h>

absData_t trackpadAbsData;
relData_t trackpadRelData;

#define MY_Z_IDLE_COUNT  1

#ifdef USING_SPI
  CirquePinnacleSPI trackpad(MY_Z_IDLE_COUNT, data_mode, true);   // overrides for Z idle count and invert Y data
#else
  CirquePinnacleI2C trackpad(MY_Z_IDLE_COUNT, data_mode, true);   // overrides for Z idle count and invert Y data
#endif

void setup(void) {
  Serial.begin(9600);
  while (not Serial and millis() < 10e3); // wait up to 10secs for an open Console
  setMyConfigVars();  // set my configuration parameters before begin() is called
#ifdef USING_SPI
  trackpad.begin(CIRQUE_DATA_READY_PIN, SPI_SELECT_PIN, SPI_SPEED_MAX);
#else
  uint8_t status = 7;
  while (status) {
    uint8_t status = trackpad.begin(CIRQUE_DATA_READY_PIN); // ,addr=default I2C address)
    if (status == 2) {
      Serial << "I2C address ACK error - trackpad not answering." << endl;
    } else {
      Serial << "Unknown error " << status << "returned by I2C trackpad." << endl;
    }
    delay(5e3);
  }
#endif
  print_ID();
}

void loop(void) {
  if(trackpad.Data_Ready()) {
    if (data_mode == DATA_MODE_ABS) {
      trackpad.GetAbsoluteData(trackpadAbsData);
      print_trackpad_abs_data();
    } else {
      trackpad.GetRelativeData(trackpadRelData);
      print_trackpad_rel_data();
    }
    trackpad.ScaleData(trackpadAbsData, 1024, 1024);  // Scale coordinates to arbitrary X, Y resolution
  }
  // delay(5);
}

// print an Absolute data read
//  buttons are printed with leading b1 for column formatting purposes
void print_trackpad_abs_data(void) {
  Serial    << trackpadAbsData.xValue
    << "\t" << trackpadAbsData.yValue
    << "\t" << trackpadAbsData.zValue
    << "\t" << trackpadAbsData.touchDown
    << "\t" << trackpad.Decode_Buttons(trackpadAbsData.buttonFlags) << endl;
}

// print a Relative data read
//  buttons are printed with leading b1 for column formatting purposes
void print_trackpad_rel_data(void) {
  Serial    << trackpadRelData.x
    << "\t" << trackpadRelData.y
    << "\t" << trackpadRelData.scroll
    << "\t" << trackpad.Decode_Buttons(trackpadRelData.buttons) << endl;
}

// just read and print chip and firmware info
void print_ID() {
  uint8_t chip_id, fw_ver, product_id;
  trackpad.Get_ID(chip_id, fw_ver, product_id);
  Serial << "Chip ID:0x" << _HEX(chip_id)
         << " Firmware Version:0x" << _HEX(fw_ver)
         << " Product ID:0x" << _HEX(product_id) << endl;
}

// user set their own config vars
// Z idle cont set from c'tor
void setMyConfigVars(void) {
  uint8_t cfg_feed1, cfg_feed2;
  cfg_feed1 =   PINNACLE_FLG_FEED1_Y_DATA_INVERT
              | ((data_mode == DATA_MODE_ABS) ? PINNACLE_FLG_FEED1_DATA_MODE : 0)
              | PINNACLE_FLG_FEED1_FEED_ENABLE;
  cfg_feed2 = PINNACLE_FLG_FEED1_FEED_ENABLE; // all taps enabled
  trackpad.Set_Config_Values(cfg_feed1, cfg_feed2);  // sets values and flag for Init call
}

// cirque_demo.ino
