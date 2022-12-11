/****************************************************************************
 * Arduino plotting sketch for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * See the range of X Y Z or relative values and "button" taps. Use the
 * Arduino IDE's Serial Plotter (Ctrl-Shift-L) to see the plots. Buttons
 * in Relative mode are at 35, 30, & 25 (and color coded).
 *
 * Both SPI and I2C interfaces are supported, as are Absolute and Relative
 * data reads.
 *
 * Credit to Ryan Young's https://github.com/ryanayoung/projectSpigot project
 * and Cirque for example cade and inspiration.
 *
 * 2022-12-11 - Original.
 ****************************************************************************/

// Select the interface being used
#define USING_SPI
// #define USING_I2C

// for my QT Cirque board with an Adafruit QT Py (#4600) MCU and an FFC-12 connector
#define SPI_SPEED_MAX       10e6

// for Trackpad 1 (TP1_)
#define TP1_SPI_SELECT_PIN  0    // SPI chip/slave select pin
#define TP1_DATA_READY_PIN  1    // or -1 if not wired (no ISR, uses SW_DR in Status register)

#ifdef USING_SPI  
  #include <CirquePinnacleSPI.h>
  #define TP1_PIN_ADDR  TP1_SPI_SELECT_PIN
#else
  #include <CirquePinnacleI2C.h>
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

#include <pPlot.h>

Plot plot; // instantiate

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
  // print_ID();
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

    // this is probably for Absolute data - I haven't worked with it
    // trackpad1.ScaleData(trackpadAbsData, 1024, 1024);  // Scale coordinates to arbitrary X, Y resolution
    
    plot_trackpad_data(trackpadData1);
  }
} // loop()

// mask out a button flag and return plot value
int button_value(trackpad_data_t& trackpadData, uint8_t button, int value) {
  return (trackpadData.rel_data.buttons & button) ? value : 0;
}

// plot decoded data
//  "button" taps are displayed as spike to different levels for each button
void plot_trackpad_data(trackpad_data_t& trackpadData) {
  if (data_mode == DATA_MODE_ABS) {
    if (not trackpadData.abs_data.touchDown) return;
    plot.add("x", trackpadData.abs_data.xValue);
    plot.add("y", trackpadData.abs_data.yValue);
    plot.add("z", trackpadData.abs_data.zValue);
  } else {
    plot.add("rx", trackpadData.rel_data.x);
    plot.add("ry", trackpadData.rel_data.y);
    plot.add("scroll", trackpadData.rel_data.scroll);
    plot.add("Bp", button_value(trackpadData, PINNACLE_FLG_REL_BUTTON_PRIMARY,   35));
    plot.add("Bs", button_value(trackpadData, PINNACLE_FLG_REL_BUTTON_SECONDARY, 30));
    plot.add("Ba", button_value(trackpadData, PINNACLE_FLG_REL_BUTTON_AUXILLARY, 25));
  }
  plot.print(Serial);
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
