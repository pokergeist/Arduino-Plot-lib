# Arduino Libraries

Here are a few of my Arduino libraries that someone may find useful.

## EMA (Exponential Moving Average)

This library calculates exponential moving averages (EMA) where the value is biased toward the most recent sample. This is in contrast to a simple moving average (SMA).

## Plot_pg

The Plot class helps you generate and accumulate variable strings to be sent to the Arduino IDE's Serial Plotter.

## AFS_MPU9250

This developing library supports the MPU-9250 9-DOF Motion Processing Unit/IMU, with the components derived from the Adafruit_Sensor type. Additional features are:

* access to the AK8963 Magnetometer sensor configuration and data registers
* ability to configure the MPU-9250's data ready interrupt signal which can be polled or associate with an interrupt handler
* ability to configure the MPU-9250's slave device configuration register for populating its External Sensor Data registers
* access to the individual Adafruit Sensor objects.

## AsyncTimer2

This is a clone of Luis Llamas' [AsyncTimerLib](https://github.com/luisllamasbinaburo/Arduino-AsyncTimer).

This library lets you configure timers and callbacks with an optional parameter. Each time through loop() check your timers. There are a number of good features and options. AsyncTimerLib may be documented better. I use it for concurrent I2C and Serial message processing, blinking LEDs, mode switch timing, and sensor monitoring in one project alone. Handy stuff.

I tinkered with the constructors and default parameters to suit me, added an option to return an int parameter to a callback, and probably renamed a few things.

## MemTest

This gives you a simple way to monitor free memory usage. It sends a notice to the Console anytime there's a reduction in Free Memory.
