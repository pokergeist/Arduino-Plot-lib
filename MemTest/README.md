# MemTest Arduino Library

## Intro

This library provides a simple way to monitor free memory. There are externals for ARM processors. This probably won't work for other processor families. It's been tested on an Adafruit QT Py SAMD21 (product #4600).

It emits a notification whenever free memory ebbs to a new low. The following is the baseline print. For new lows the hit counter will increment and the free memory will report a lower free memory value.

```
Free memory:26619 hit:0 ***********************************
```

## Basic Application

```c++
#include <MemTest.h>

MemTest mem_test;

void setup(void) {
    // initialize stuff
    Serial.begin(9600);
    mem_test.begin(Serial, 256, 250); // check every 256 check calls,
                                      // delay 250ms on a hit (for viewing)
    // initialize more stuff
}

void loop(void) {
    // ... do stuff ...
    mem_test.check();	// prints and returns true if free memory has gone down
}
```

## Methods

Here are the available methods:

*   void begin() - initialize
 *   bool check() - check for a reduction in free memory
 *   int hits() - see how many memory hits have occurred
 *   int freeMemoryMin() - get the free memory low point
 *   int freeMemory() (static) - see the available free memory

## Notes

* The first check() call will report the baseline value "hit:0" which should be after most objects are constructed and initialized.
* If your application creates and deletes/frees objects in bursts, you may want to call check() when things settle down. Leaks will still be detected but you won't see notices every time there's an increase in activity and object use.
* begin() can set a modulus to reduce the number of freeMemory() calls and potential notices and delays. The default is a modulus of 1 to check every time. You may want to increase this value reduce the performance hit.

## Examples

None yet.