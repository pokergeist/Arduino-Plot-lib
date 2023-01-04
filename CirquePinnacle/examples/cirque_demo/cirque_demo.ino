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
#define SPI_SPEED_MAX       10e6

// for Trackpad 1 (TP1_)
#define TP1_SPI_SELECT_PIN  0    // SPI chip/slave select pin
#define TP1_DATA_READY_PIN  1    // or -1 if not wired (no ISR, uses SW_DR in Status register)

/****************************************************************************
 * On Using Interrupts
 *
 * When new data is ready the hardware Data Ready (HW_DR) line will rise and
 * trigger the Interrupt Service Routine (ISR) to fetch and decode the data
 * from the trackpad device. It sets the trackpad_data.data_ready flag in the
 * sketch's data. The sketch polls this flag for new data. The ISR clears
 * the SW_DR flag in the Status register which drops the HW_DR line and
 * enables the trackpad to do the next data acquisition based on its Speed
 * setting.
 *
 * For busy applications using Absolute data this ensures that you always
 * have the latest data, not stale data from just after your last data read.
 *
 * ISR mode may not be suitable for Relative data users because ISR-read
 * but unused (by sketch) Relative data will be overwritten. It is assumed by
 * me that less frequent data reads will result in larger delta values that
 * yield better tracking data. Use Set_Speed() to better synchronize data
 * acquisition with your application.
 *
 * ISR mode needs the physical DATA_READY_PIN to trigger the interrupt. There
 * is no provision for timer based interrupts at this time.
 ****************************************************************************/

/****************************************************************************
 * On Using SPI or I2C
 *
 * An SPI buss has (3) lines: two uni-directional data lines and a clock line,
 * plus a select line for each device. An I2C buss has (2) lines: data and
 * clock. Each device has a unique address. SPI is faster but requires more
 * GPIO pins. I2C conserves your pins but requires identical devices to have
 * an address selection mechanism.
 *
 * The Cirque Pinnacle is not very accommodating in allowing multiple I2C
 * devices on the same buss. First, miniature resistor R1 had to be
 * desoldered to even allow I2C operation. To use the alternate I2C address,
 * a resistor needs to be installed across the ADR pads. Otherwise you can
 * use an I2C multiplexor which allows you to select 1 of n I2C busses to
 * talk on at a time. Then you have to add 3.3V pull-up resisors on both
 * I2C lines.
 *
 * SPI supports multiple devices much easier. You are limited by by GPIO
 * select lines unless you add a 2^n demultiplexer chip.
 ****************************************************************************/

#ifdef USING_SPI
  #include <CirquePinnacle-SPI.h>
  #define TP1_PIN_ADDR  TP1_SPI_SELECT_PIN
#else
  #include <CirquePinnacle-I2C.h>
  #define TP1_PIN_ADDR  CIRQUE_PINNACLE_DEFAULT_ADDR
#endif
#include <Streaming.h>

// True if using an Interrupt Service Routine (provided by CirquePinnacle)
bool using_isr = true;

// Select the preferred data mode, 1 or true for Absolute, 0 or false for Relative
data_mode_t data_mode = (1) ? DATA_MODE_ABS : DATA_MODE_REL;

trackpad_data_t trackpadData1;

#define MY_Z_IDLE_COUNT  1

#ifdef USING_SPI
  CirquePinnacleSPI trackpad1(data_mode, MY_Z_IDLE_COUNT, true);   // overrides for Z idle count and invert Y data
#else
  CirquePinnacleI2C trackpad1(data_mode, MY_Z_IDLE_COUNT, true);   // overrides for Z idle count and invert Y data
#endif

void setup(void) {
  Serial.begin(9600);
  while (not Serial and millis() < 10e3); // wait up to 10secs for an open Console
  setMyConfigVars();  // set my configuration parameters before begin() is called
#ifdef USING_SPI
  trackpad1.begin(TP1_DATA_READY_PIN, TP1_SPI_SELECT_PIN, SPI_SPEED_MAX);
#else
  uint8_t status = 7;
  while (status) {
    status = trackpad1.begin(TP1_DATA_READY_PIN); // ,addr=default I2C address)
    if (status == 2) {
      Serial << "I2C address ACK error - trackpad1 not answering." << endl;
    } else if (status) {
      Serial << "Unknown error " << status << " returned by I2C trackpad1." << endl;
    }
    if (status) delay(5e3);
  }
#endif
  print_ID();
  if (using_isr) {
    cp_error_t e_status = trackpad1.Start_ISR(TP1_PIN_ADDR, trackpadData1);
    if (e_status != E_OKAY) {
      Serial << "ERROR: Start_ISR() error number " << e_status << endl;
      // add LED indicator?
      while (1) delay(5e3);
    }
  }
} // setup()

void loop(void) {
  if(trackpad1.Data_Ready()) {
    if (using_isr) {
      // ISR already read and decoded our trackpad data
      // clear our data ready flag set by the ISR
      trackpad1.Clear_DR();
    } else {
      // read and decode trackpad data
      trackpad1.Get_Data(trackpadData1);
    }
    print_trackpad_data(trackpadData1);

    // this is probably for Absolute data - I haven't worked with it
    // trackpad1.ScaleData(trackpadAbsData, 1024, 1024);  // Scale coordinates to arbitrary X, Y resolution
  }
} // loop()

// print decoded data
//  buttons are printed with leading b1 for column formatting purposes
void print_trackpad_data(trackpad_data_t& trackpadData) {
  if (data_mode == DATA_MODE_ABS) {
    Serial    << trackpadData.abs_data.xValue
      << "\t" << trackpadData.abs_data.yValue
      << "\t" << trackpadData.abs_data.zValue
      << "\t" << trackpadData.abs_data.touchDown
      << "\t" << trackpad1.Decode_Buttons(trackpadData.abs_data.buttonFlags) << endl;
  } else {
    Serial    << trackpadData.rel_data.x
      << "\t" << trackpadData.rel_data.y
      << "\t" << trackpadData.rel_data.scroll
      << "\t" << trackpad1.Decode_Buttons(trackpadData.rel_data.buttons) << endl;
  }
}

// just read and print chip and firmware info
void print_ID() {
  uint8_t chip_id, fw_ver, product_id;
  trackpad1.Get_ID(chip_id, fw_ver, product_id);
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
  trackpad1.Set_Config_Values(cfg_feed1, cfg_feed2);  // sets values and flag for Init call
}

// cirque_demo.ino
