# Cirque Pinnacle Arduino Lib

## Intro

This library is meant to facilitate the use of a Cirque GlidePoint trackpad. In this case, specifically ones using the Gen3 (or Gen2?) Pinnacle ASIC.

## Data Notification

The Pinnacle provide a Data Ready signal either as an active low data line, or as a Status register flag. You can either poll the I/O pin or the Data Ready flag in the Status register. An interrupt service routine was considered by not deemed necessary for now.

## Testing Hardware

I'm using a Cirque GlidePoint TM035035-2024 Pinnacle based circular trackpad to test with. My microcontroller is an Adafruit QT Py SAMD21 (product #[4600](https://www.adafruit.com/product/4600)) on a custom board with a FFC-12 connector for connecting to the trackpad. The QT Py accesses all 12 of the FFC lines.

## Interface Notes

### I2C Notes

The default I2C address is 0x2A once resistor R1 is removed from the TM035035-2024's board to enable I2C operation. Doing so disables SPI operation. **Pull-up resistors are required for this trackpad!** - See my note in cirque_demo.ino. Other models may differ - one reference schematic does show 2.2kΩ pull-up resistors installed.

### SPI Notes

Unfortunately the latest code has not been tested with SPI because I converted my only trackpad to I2C (because  I'm so I2C-centric) not thinking that I'm the only person that would bother. The fact that there are no I2C pull-ups just adds to the futility of it all. But at least it still reads ... 0xFF - for everything - all the time. But I'm 98.3% sure it will work. I'll enlist the help of a tester that will hopefully give me the thumbs up, then I'll post an update.

## example/../cirque_demo.ino

This example sketch will allow you to use either the SPI or I2C interface by changing a #define.

```c++
// Select the interface being used
#define USING_SPI
// #define USING_I2C
```

Next, define which pins are being used (if any).

```c++
// for my QT Cirque board with an Adafruit QT Py (#4600) MCU and an FFC-12 connector
#define SPI_SELECT_PIN           0
#define CIRQUE_DATA_READY_PIN    1    // or -1 if not wired
#define SPI_SPEED_MAX         10e6
```

|         Macro         | Description                                                  |
| :-------------------: | ------------------------------------------------------------ |
|    SPI_SELECT_PIN     | This is the SPI Slave/Chip Select output line that connects to the Pinnacle's SS line which enables the Pinnacle to communicate on the SPI buss. |
| CIRQUE_DATA_READY_PIN | This is the pin connected to the Pinnacle's DR (/Data Ready) active low output that signals when new feed data is ready. If this pin is not wired set the value to -1 (<0) to check the Status register DR flag instead. |
|     SPI_SPEED_MAX     | This is the SPI clock speed that will be used for SPI communications. 10Mbps seems to be a good speed for current microcontroller. |

Both Absolute and Relative data modes are supported (but not concurrently at this time).

```c++
// Select the preferred data mode
data_mode_t data_mode = DATA_MODE_ABS;
// data_mode_t data_mode = DATA_MODE_REL;
```

## Files

| File                        | Description                                                  |
| --------------------------- | ------------------------------------------------------------ |
| CirquePinnacle.h            | The header file for the CirquePinnacle base class. It contains the Register and Flag definitions, and a few common register settings. |
| CirquePinnacle.cpp          | The implementation for everything except the interface code. |
| CirquePinnacleSPI.h         | Creates the child class CirquePinnacleSPI that will be the one instantiated most likely. |
| CirquePinnacleSPI.cpp       | Contains the Register Access Protocol (RAP) methods using SPI. |
| CirquePinnacleI2C.h         | Creates the child class CirquePinnacleI2C as an alternative to using SPI. |
| CirquePinnacleI2C.cpp       | Contains the Register Access Protocol (RAP) methods using I2C. |
| examples/../cirque_demo.ino | This is the example sketch that creates a CirquePinnacle child class the polls the DR line, reads and prints the data. (*) |

\* The button data for both Absolute and Relative readings is suspiciously static with no activations at the moment.

## Library/Driver Code

### Constructors

The constructors allow you to override default parameters that will be applied when begin() is called. These c'tors could be overloaded and the parameters altered or reorganized as needed in the future.

### Methods etc.

| Method            | Description                                                  |
| ----------------- | ------------------------------------------------------------ |
| begin()           | Because it's an Arduino thing.                               |
| Pinnacle_Init()   | Called by begin() to set the configuration registers.        |
| GetAbsoluteData() | Pass your structure by reference to get the latest Absolute dataset. |
| GetRelativeData() | Pass your structure by reference to get the latest Relative dataset. |
| ClearFlags()      | Called frequently to clear the CC and DR flags in the Status Register. |
| EnableFeed()      | Used to disable then re-enable the feed for certain operations. |
| ERA_ReadBytes()   | Read bytes from an Extended Register Address.                |
| ERA_WriteByte()   | Write to an Extended Register Address.                       |
| ClipCoordinates() | Clips raw coordinates to "reachable" window of sensor.       |
| ScaleData()       | Scales data to desired X & Y resolution.                     |
| Data_Ready()      | Check the Data Ready line for new data. xxxxxxxxxxxxxxxx     |
| Invert_Y()        | Inverts the Y-Axis. Now better implemented with the c'tor override. |
| Get_ID()          | Retrieves the chip and firmware version, and the product ID. |
| SetFlag()         | A utility routine for setting and clearing flags in a register word. |

## Credit Where Credit is Due

I have to recognize the contributions of Cirque and to a greater extent Ryan Young for his comprehensive Cirque Pinnacle [code](https://github.com/ryanayoung/projectSpigot) on GitHub. I've only scratched the surface with my implementation - but that's probably good enough for now.

## ToDo

* Resolve the button data issue. The data was very dynamic and seemingly useless before. I either fixed or broke something.

## Notes on Style ...

### Streaming

If your Arduino Library Manager stops finding the Steaming lib or you want to see the documentation, get it [here](https://github.com/janelia-arduino/Streaming). If you cringe because you don't like iostream or round planets feel free to replace the prints with 17.3k Serial.print() calls.

### Engineering Notation

For timing and clock parameters I tend to use exponent notation, e.g., delay(5e3) for a 5 sec delay.

### Coding Style

Yes, I like K&R style, aligned columns, mixed Camel-lower-whatever ...  I'm not going to run it through a code-uglifier just to make the robots happy. I have to look at this stuff.
