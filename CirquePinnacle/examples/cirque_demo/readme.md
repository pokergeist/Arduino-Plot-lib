# Cirque Demo Sketch

## Intro

This sketch demonstrates how to use the CirquePinnacle library.

## Operating Options

### Communications Mode

```c++
// Select the interface being used
#define USING_SPI
// #define USING_I2C
...
// for Trackpad 1 (TP1_)
#define TP1_SPI_SELECT_PIN  0    // SPI chip/slave select pin
```

For SPI communications each device requires a **Select Pin** - a GPIO Output pin on the microcontroller.

For I2C communications each device has a different address. The Circular GlidePoint trackpads have two address available:

```c++
// Cirque's 7-bit I2C Slave Address
#define CIRQUE_PINNACLE_DEFAULT_I2C_ADDRESS   0x2A
#define CIRQUE_PINNACLE_ALTERNATE_I2C_ADDRESS 0x2C
```

More specific information is avail below or in the sketch file.

## Data Ready

A Data Ready signal indicates that the trackpad has data available for the host processor. This indicator has three forms:

* The trackpad sets the SW_DR flag in the Status Register. This flag controls hardware DR (HW_DR) line. The host processor will clear this bit after reading the data which will also drop the hardware DR line.
* The hardware DR (HW_DR) line is typically routed to processor's input GPIO pin. If connected this pin can be read via digitalRead() to initiate a data read.
* The third signal is set by the Interrupt Service Routing (ISR) after reading and decoding the raw trackpad data. The ISR clears the SW_DR and HW_DR  indicators. The sketch clears the isr_data data_ready flag.

The Data_Ready() method check for a Data Ready indication in this order:

* isr_data.data_ready is checked if an ISR is in use.
* if the DATA_READY_PIN >= 0 the pin status is read and returned.
* Otherwise, the SW_DR value is read from the Status register and returned.

## Interrupt Service Routines

Each instance of CirquePinnacle can have an ISR that reads and decodes the trackpad data. The DATA_READY pin must be >= 0 and be interrupt capable. The ISR takes priority and launches when the Data Ready line rises. It uses data cached by Start_ISR() to read, decode, and store the trackpad data in a structure designated by the sketch. The it sets a Data Ready indicator the the sketch polls.

```c++
// for Trackpad 1 (TP1_)
...
#define TP1_DATA_READY_PIN  1    // or -1 if not wired (no ISR, uses SW_DR in Status register)
...
// True if using an Interrupt Service Routine (provided by CirquePinnacle)
bool using_isr = true;
```

## Data Mode

Select the appropriate data mode:

* Absolute - report X Y Z coordinates of the trackpad contact
* Relative - reports the delta- X Y Z values since the last data acquisition.

```c++
// Select the preferred data mode, 1 or true for Absolute, 0 or false for Relative
data_mode_t data_mode = (1) ? DATA_MODE_ABS : DATA_MODE_REL;
```

## Create an Instance

You will need to create a child class instance using either SPI or I2C communication.

```c++
#ifdef USING_SPI
  CirquePinnacleSPI trackpad1(data_mode, MY_Z_IDLE_COUNT, true);   // overrides for Z idle count and invert Y data
#else
  CirquePinnacleI2C trackpad1(data_mode, MY_Z_IDLE_COUNT, true);   // overrides for Z idle count and invert Y data
#endif
```

You will need to specify the data mode (DATA_MODE_REL or DATA_MODE_ABS), the idle count (the number all "0" data records), and whether you want the Y-axis values inverted to ascending from top to bottom.

## setup()

The setup routine first waits up to 5 seconds for the Serial Console to be launched.

Then it calls the local setMyConfig to stage configuration values for the Feed1 and Feed2 registers. This is an optional process that more configuration control than the constructor does.

trackpad.begin() is called to initialize the trackpad object with the data ready pin and for SPI, the select pin and speed. The I2C class has diagnostics that will return an error if the I2C device address can't be pinged.

Next the hardware parameters are displayed - ASIC and firmware versions, and the product ID.

Finally, if using ISR, the ISR data is populated and the interrupt enabled.

## loop()

The trackpad's data ready parameters are checked.

Data Ready is checked:

* If there is data,
  * if using ISR, the Data Ready indicator set by the ISR is cleared. The ISR has already read and decoded the trackpad data.
  * If not using ISR, Get_Data is called to read and decode the trackpad data.
    * The trackpad is printed via local print_trackpad_data().
* If there is no data, the program simply loops.

## Trackpad Data

Trackpad data is composed of an absData_t and a relData_t structure (union'ed) and a millisecond timestamp.

```c++
// decoded metrics
typedef struct _absData {
  uint16_t xValue;
  uint16_t yValue;
  uint16_t zValue;
  uint8_t  buttonFlags;
  bool     touchDown;
} absData_t;

typedef struct _relData {
  uint8_t buttons;
  int8_t x;
  int8_t y;
  int8_t scroll;
} relData_t;

typedef struct {
  union {
    absData_t abs_data;
    relData_t rel_data;
  };
  uint32_t timestamp_ms;
} trackpad_data_t;
```

Note that you can only access either Absolute or Relative data since they overlay each other. The one that matches your data_mode will be valid. The interpretation of the data in the other structure will not be valid.

## Start_ISR()

Start_ISR() populates ISR data and callback pointers and returns an enumerated cp_error_t value.

Parameters are:

* pin_address - either the SPI select **pin** or the I2C **address**. You can use:
  * **(TPx_) PIN_ADDR** since it is set according to the interface selected, or
  * **TP1_SPI_SELECT_PIN** or **CIRQUE_PINNACLE\_(DEFAULT)\_ADDR** as appropriate.
* trackpadData - this is the trackpad_data_t structure for this trackpad, passed by reference.

## setMyConfigVars()

CirqiePinnacle.h contains:

* register addresses: PINNACLE_REG_... where byte values are written to and read from.
* register flags:  PINNACLE_FLG_ that can or OR'ed and masked with a register value as needed.
* register values: PINNACLE_VAL_ that are pre-configured register values that may or may not suit your purposes.

```c++
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
```

## Example Output

Absolute data. No switches.

```
1004  1140  28  1  - - - - - -
1004  1140  29  1  - - - - - -
1004  1140  31  1  - - - - - -
1004  1140  32  1  - - - - - -
1003  1140  32  1  - - - - - -
1002  1140  32  1  - - - - - -
1001  1139  31  1  - - - - - -
1001  1139  30  1  - - - - - -
1001  1139  29  1  - - - - - -
1000  1139  28  1  - - - - - -
1000  1139  28  1  - - - - - -
1001  1139  27  1  - - - - - -
1004  1139  27  1  - - - - - -
1008  1141  30  1  - - - - - -
1014  1143  30  1  - - - - - -
1020  1145  29  1  - - - - - -
1027  1144  30  1  - - - - - -
1035  1141  31  1  - - - - - -
1044  1138  31  1  - - - - - -
1053  1135  25  1  - - - - - -
1063  1131  10  1  - - - - - -
1072  1129  5   1  - - - - - -
0     0     0   0  - - - - - -
939   1133  27  1  - - - - - -
939   1133  34  1  - - - - - -
939   1133  35  1  - - - - - -
939   1133  33  1  - - - - - -
940   1132  32  1  - - - - - -
942   1128  33  1  - - - - - -
945   1122  29  1  - - - - - -
950   1115  24  1  - - - - - -
958   1107  15  1  - - - - - -
966   1100  9   1  - - - - - -
973   1096  6   1  - - - - - -
979   1093  5   1  - - - - - -
0     0     0   0  - - - - - -
```

## Notes From cirque_demo.ino

### Interrupts

```c++
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
```

### SPI vs. I2C

```c++
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
```



