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

// Select the preferred data mode
data_mode_t data_mode = DATA_MODE_ABS;
// data_mode_t data_mode = DATA_MODE_REL;

#include <Streaming.h>

absData_t trackpadAbsData;
relData_t trackpadRelData;

#ifdef USING_SPI
  CirquePinnacleSPI trackpad(1, data_mode, true);   // overrides for Z idle count and invert Y data
#else
  CirquePinnacleI2C trackpad(1, data_mode, true);   // overrides for Z idle count and invert Y data
#endif

void setup(void) {
  Serial.begin(9600);
  while (not Serial and millis() < 10e3);
#ifdef USING_SPI
  trackpad.begin(CIRQUE_DATA_READY_PIN, SPI_SELECT_PIN, SPI_SPEED_MAX);
#else
  trackpad.begin(CIRQUE_DATA_READY_PIN); // ,addr=default I2C address)
#endif
  print_ID();
  delay(500);
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
  delay(10);
}

// print an Absolute data read
//  buttons are printed with leading b1 for column formatting purposes
void print_trackpad_abs_data(void) {
  Serial    << trackpadAbsData.xValue
    << "\t" << trackpadAbsData.yValue
    << "\t" << trackpadAbsData.zValue
    << "\t" << trackpadAbsData.touchDown
    << "\tb" << String((trackpadAbsData.buttonFlags+0x80) >> 3, BIN) << endl;
}

// print a Relative data read
//  buttons are printed with leading b1 for column formatting purposes
void print_trackpad_rel_data(void) {
  Serial    << trackpadRelData.x
    << "\t" << trackpadRelData.y
    << "\t" << trackpadRelData.scroll
    << "\tb" << String(trackpadRelData.buttons+0x08, BIN) << endl;
}

// just read and print chip and firmware info
void print_ID() {
  uint8_t chip_id, fw_ver, product_id;
  trackpad.Get_ID(chip_id, fw_ver, product_id);
  Serial << "Chip ID: 0x" << _HEX(chip_id)
         << " Firmware Version: 0x" << _HEX(fw_ver)
         << " Product ID: 0x" << _HEX(product_id) << endl;
}

// cirque_demo.ino
