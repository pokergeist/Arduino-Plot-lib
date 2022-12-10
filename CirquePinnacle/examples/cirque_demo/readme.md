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



