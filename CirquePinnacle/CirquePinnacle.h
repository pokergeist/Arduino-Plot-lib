/****************************************************************************
 * Arduino driver for Cirque GlidePoint (Pinnacle ASIC) touchpad.
 *
 * Credit to Ryan Young's https://github.com/ryanayoung/projectSpigot project
 * and Cirque for example cade and inspiration.
 *
 * My driver is quite simple compared to Ryan's. If you want to see more
 * comprehensive utilization of the hardware, please visit his repo.
 *
 * 2023-01-10 - Fixed isr_data.spi_speed size (!).
 * 2022-12-10 - Added interrupt capability.
 * 2022-12-06 - Original.
 ****************************************************************************/

#ifndef CIRQUE_PINNACLE_H
#define CIRQUE_PINNACLE_H

#include <Arduino.h>

#define CP_MAX_SPI_SPEED      13e6      // up to 13 MHz unless exiting Sleep and Shutdown modes
#define CP_DEFAULT_SPI_SPEED   8e6
#define CP_DEFAULT_I2C_SPEED  100e3

// Cirque's 7-bit I2C Slave Address
#define CP_DEFAULT_I2C_ADDRESS   0x2A
#define CP_ALTERNATE_I2C_ADDRESS 0x2C

#ifdef USING_SPI
  #define CP_DEFAULT_SPEED    CP_DEFAULT_SPI_SPEED
#else
  #define CP_DEFAULT_SPEED    CP_DEFAULT_I2C_SPEED
#endif


// register definitions w/ ReadOnly and default annotations
enum pinnacle_register_t {
	PINNACLE_REG_ASIC_ID          = 0x00,  // RO "Firmware ASIC ID" 0x07
	PINNACLE_REG_FIRMWARE_VERSION = 0x01,  // RO 0x3A
	PINNACLE_REG_STATUS           = 0x02,
	PINNACLE_REG_SYS_CONFIG       = 0x03,
	PINNACLE_REG_FEED_CONFIG_1    = 0x04,
	PINNACLE_REG_FEED_CONFIG_2    = 0x05,
	PINNACLE_REG_RESERVED_06      = 0x06,
	PINNACLE_REG_CAL_CONFIG_1     = 0x07,
	PINNACLE_REG_PS2_AUX_CNTL     = 0x08,
	PINNACLE_REG_SAMPLE_RATE      = 0x09,
	PINNACLE_REG_Z_IDLE           = 0x0A,
	PINNACLE_REG_Z_SCALER         = 0x0B,
	PINNACLE_REG_SLEEP_INTERVAL   = 0x0C,  // time of sleep until checking for finger
	PINNACLE_REG_SLEEP_TIMER      = 0x0D,  // time after idle mode until sleep starts
	PINNACLE_REG_DYN_EMI_ADJ_TH   = 0x0E,  // Dynamic EMI Adjust Threshold
	PINNACLE_REG_RESERVED_1       = 0x0F,  // RO
	PINNACLE_REG_RESERVED_2       = 0x10,  // RO
	PINNACLE_REG_RESERVED_3       = 0x11,  // RO
	PINNACLE_REG_TRACKPAD_DATA    = 0x12,  // RO x 6B
	PINNACLE_REG_PACKET_BYTE_0    = 0x12,  // RO
	PINNACLE_REG_PACKET_BYTE_1    = 0x13,  // RO
	PINNACLE_REG_PACKET_BYTE_2    = 0x14,  // RO
	PINNACLE_REG_PACKET_BYTE_3    = 0x15,  // RO
	PINNACLE_REG_PACKET_BYTE_4    = 0x16,  // RO
	PINNACLE_REG_PACKET_BYTE_5    = 0x17,  // RO
	PINNACLE_REG_PORT_A_GPIO_CNTL = 0x18,
	PINNACLE_REG_PORT_A_GPIO_DATA = 0x19,
	PINNACLE_REG_PORT_B_GPIO_CDTA = 0x1A,  // Port B Control & Data
	PINNACLE_REG_ERA_VALUE        = 0x1B,  // Extended Register Access Value
	PINNACLE_REG_ERA_ADDR         = 0x1C,  // Extended Register Access Address x 2B
	PINNACLE_REG_ERA_ADDR_HIGH    = 0x1C,  // Extended Register Access Address High
	PINNACLE_REG_ERA_ADDR_LOW     = 0x1D,  // Extended Register Access Address Low
	PINNACLE_REG_ERA_CONTROL      = 0x1E,  // Extended Register Access Control
	PINNACLE_REG_PRODUCT_ID       = 0x1F   // RO Product ID
};

// Masks for Cirque Register Access Protocol (RAP)
#define WRITE_MASK  0x80
#define READ_MASK   0xA0

// definitions for register flags and values for combinations of those flags
#define PINNACLE_FLG_STATUS_CMD_COMPLETE      0b00001000
#define PINNACLE_FLG_STATUS_DATA_READY        0b00000100
#define PINNACLE_VAL_STATUS_CLEAR             0x00

#define PINNACLE_FLG_FEED1_Y_DATA_INVERT      0b10000000
#define PINNACLE_FLG_FEED1_X_DATA_INVERT      0b01000000
#define PINNACLE_FLG_FEED1_Y_DISABLE          0b00010000
#define PINNACLE_FLG_FEED1_X_DISABLE          0b00001000
#define PINNACLE_FLG_FEED1_FILTER_DISABLE     0b00000100
#define PINNACLE_FLG_FEED1_DATA_MODE          0b00000010 // 1 for Absolute
#define PINNACLE_FLG_FEED1_FEED_ENABLE        0b00000001
#define PINNACLE_VAL_FEED1_CFG_ABS1 0x03  // Abs Data mode | Feed enabled
#define PINNACLE_VAL_FEED1_CFG_ABS2 0x83  // Abs Data mode | Feed enabled / Invert Y
#define PINNACLE_VAL_FEED1_CFG_REL  0x81  // Rel Data mode | Feed enabled / Invert Y

#define PINNACLE_FLG_FEED2_SWAP_XY                0b10000000
#define PINNACLE_FLG_FEED2_GLIDE_EXTEND_DISABLE   0b00010000
#define PINNACLE_FLG_FEED2_SCROLL_DISABLE         0b00001000
#define PINNACLE_FLG_FEED2_SECONDART_TAP_DISABLE  0b00000100
#define PINNACLE_FLG_FEED2_ALL_TAPS_DISABLE       0b00000010
#define PINNACLE_FLG_FEED2_INTELLIMOUSE_ENABLE    0b00000001
#define PINNACLE_VAL_FEED2_CFG_ABS  0x1F  // disable all, enable intellimouse
#define PINNACLE_VAL_FEED2_CFG_REL  0x01  // enable all, no swap

#define PINNACLE_FLG_ERA_CONTROL_WR_AUTO_INCR     0b00001000
#define PINNACLE_FLG_ERA_CONTROL_RD_AUTO_INCR     0b00000100
#define PINNACLE_FLG_ERA_CONTROL_WRITE            0b00000010
#define PINNACLE_FLG_ERA_CONTROL_READ             0b00000001
#define PINNACLE_VAL_ERA_CONTROL_CFG_RD_RDAI  0x05

#define PINNACLE_FLG_REL_BUTTON_AUXILLARY         0b00000100
#define PINNACLE_FLG_REL_BUTTON_SECONDARY         0b00000010
#define PINNACLE_FLG_REL_BUTTON_PRIMARY           0b00000001

#define PINNACLE_FLG_ABS_SWITCH_5                 0b00100000
#define PINNACLE_FLG_ABS_SWITCH_4                 0b00010000
#define PINNACLE_FLG_ABS_SWITCH_3                 0b00001000
#define PINNACLE_FLG_ABS_SWITCH_2                 0b00000100
#define PINNACLE_FLG_ABS_SWITCH_1                 0b00000010
#define PINNACLE_FLG_ABS_SWITCH_0                 0b00000001

#define PINNACLE_TRACKPAD_ABS_DATA_LEN  6
#define PINNACLE_TRACKPAD_REL_DATA_LEN  4

#define Z_IDLE_COUNT  5                 // default 30

// Coordinate scaling values - FIXME - REVIEW!
#define PINNACLE_XMAX     2047    // max value Pinnacle can report for X
#define PINNACLE_YMAX     1535    // max value Pinnacle can report for Y
#define PINNACLE_X_LOWER  127     // min "reachable" X value
#define PINNACLE_X_UPPER  1919    // max "reachable" X value
#define PINNACLE_Y_LOWER  63      // min "reachable" Y value
#define PINNACLE_Y_UPPER  1471    // max "reachable" Y value
#define PINNACLE_X_RANGE  (PINNACLE_X_UPPER-PINNACLE_X_LOWER)
#define PINNACLE_Y_RANGE  (PINNACLE_Y_UPPER-PINNACLE_Y_LOWER)

// decoded metrics
typedef struct _absData {
  uint16_t xValue;
  uint16_t yValue;
  uint8_t  zValue;
  uint8_t  buttonFlags;
  bool     touchDown;
} absData_t;

typedef struct _relData {
  uint8_t buttons;
  int8_t x;
  int8_t y;
  int8_t scroll;
} relData_t;

enum data_mode_t {
  DATA_MODE_REL,
  DATA_MODE_ABS
};

enum cp_error_t {
  E_OKAY,
  E_BAD_PIN,
  E_OUT_OF_ISRS,
  E_NOT_AN_INTERRUPT,
};

typedef struct {
  union {
    absData_t abs_data;
    relData_t rel_data;
  };
  uint32_t timestamp_ms;
} trackpad_data_t;

#define CP_RAW_DATA_LEN  PINNACLE_TRACKPAD_ABS_DATA_LEN   // max data size
#define MAX_ISRS 4  // set the number of ISRs to support

typedef void (*rap_read_f) (uint8_t isr_number, pinnacle_register_t register_addr, uint8_t* data, uint8_t count);
typedef void (*rap_write_f)(uint8_t isr_number, pinnacle_register_t register_addr, uint8_t data);

typedef struct _isr_data_t {
  uint8_t  pin_addr;        // the SPI select pin or I2C address of device to read
  uint32_t spi_speed;       // the SPI speed
  uint8_t  raw_data_len;    // the raw data length: ABS=6 or REL=4 - for decoder
  uint8_t  raw_data[CP_RAW_DATA_LEN]; // raw data for this ISR
  trackpad_data_t* trackpad_data_p; // make the caller's data structure available to the ISR
  bool     data_ready;      // signal fresh data
  rap_read_f  rap_read_cb;  // RAP read callback
  rap_write_f rap_write_cb; // rap_write callback
  bool     isr_in_use;      // this dataset is in use by ISR
} isr_data_t;

// sampling speed in samples/sec (and sample period)
enum cp_speed_t {
  SAMPLE_RATE_100 = 0x64,  //  10.0ms
  SAMPLE_RATE_80  = 0x50,  //  12.5ms
  SAMPLE_RATE_60  = 0x3C,  //  16.7ms
  SAMPLE_RATE_40  = 0x28,  //  25.0ms
  SAMPLE_RATE_20  = 0x14,  //  50.0ms
  SAMPLE_RATE_10  = 0x0A   // 100.0ms
};

class CirquePinnacle {
  // these vars hold c'tor overrides
  uint8_t z_idle_count;
  bool    y_invert;
  bool    data_mode;

  // configuration register values
  bool    use_cfg_values;
  uint8_t cfg_feed1;
  uint8_t cfg_feed2;
  cp_speed_t cfg_speed = SAMPLE_RATE_100;

  trackpad_data_t trackpad_data;  // holds the assembled abs or relData
  int8_t  data_ready_pin; // the gpio pin wired to the trackpad's HW_DR line
  bool    isr_in_use;     // ISR active, read its data ready flag
  uint8_t my_isr_num = -1; // index into the isr_data table for this instance

  virtual
  void Set_RAP_Callbacks(uint8_t isr_number) = 0; // child classes set their callback pointers

  // interface specific virtual methods
  virtual
  void RAP_ReadBytes(pinnacle_register_t register_addr, uint8_t* data, uint8_t count) = 0;
  virtual
  void RAP_Write(pinnacle_register_t register_addr, uint8_t data) = 0;

  void ERA_ReadBytes(uint16_t address, uint8_t * data, uint16_t count);
  void ERA_WriteByte(uint16_t address, uint8_t data);

protected:
  // sets attribues and calls Pinnacle_Init()
  uint8_t begin(int8_t data_ready_pin);

public:
   CirquePinnacle(data_mode_t data_mode, uint8_t z_idle_count=Z_IDLE_COUNT, bool y_invert=false);
  ~CirquePinnacle();
  virtual
  void end(void);                           // disable the ISR
  void Pinnacle_Init(void);                 // sets configuration registers
  void Pinnacle_Init(bool disableFeed);     // sets configuration registers using Set_Config_* data
  void Get_Data(trackpad_data_t& trackpad_data); // read and decode trackpad data
  void ClearFlags(void);                    // clears command_complete and data_ready flags
  void EnableFeed(bool feedEnable);         // enables the data feed (signalled by data_ready)
  void ClipCoordinates(absData_t& coordinates);
  void ScaleData(absData_t& coordinates, uint16_t xResolution, uint16_t yResolution);
  bool Data_Ready(void);                    // check the data_ready line or status flag
  void Invert_Y(bool invert);               // invert the Absolute Y values (or set in c'tor)
  void Get_ID(uint8_t& chip_id, uint8_t& firmware_version, uint8_t& product_id);
  String Decode_Buttons(uint8_t buttonData);   // list active buttons
  // Set Config Register Values first then call begin() or Pinnacle_Init(disableFeed)
  void Set_Config_Values(uint8_t cfg_feed1, uint8_t cfg_feed2);
  void Set_Speed(cp_speed_t speed=SAMPLE_RATE_100); // set the sampling speed
  // Interrupt Service Routines
  cp_error_t Start_ISR(uint8_t pin_address, trackpad_data_t& trackpadData);
  void End_ISR(void);
  void Clear_DR(void);

  /*
   * Static methods
   */
  static void Read_Data_ISR(uint8_t myISRnum);
  static void Read_Data_ISR_0(void);  // MAX_ISRS-many functions
  static void Read_Data_ISR_1(void);
  static void Read_Data_ISR_2(void);
  static void Read_Data_ISR_3(void);
  static void Decode_Data(uint8_t* raw_data, uint8_t data_len, trackpad_data_t& trackpad_data);

  static void SetFlag(uint8_t& value, uint8_t flag, bool test); // utility for setting and clearing register flags

  /*
   * Static variables;
   */

  static uint8_t isr_enumerator;        // one-up number to enumerate the callback functions's data
  static isr_data_t isr_data[MAX_ISRS];

}; // class CirquePinnacle

#endif // _H

// CirquePinnacle.h
