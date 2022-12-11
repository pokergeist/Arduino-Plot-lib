# Arduino pPlot lib

The Plot class helps you generate and accumulate variable strings to be sent to the Arduino IDE's Serial Plotter.

### Overview

After instantiation incrementally add() a label, value and an optional offset. The value is appended to the label by default. The value will be plotted at value + offset. You can specify min and max graph constraints with constrain_on(min, max) and disable them with constrain_off().

The class will append subsequently added value strings with a comma, and terminate the string with "\n" when print(Serial) is called.

The directory and .h files are named with the _pg suffix to hopefully avoid a name collision in the future. pg is just shorthand for my GitHub moniker.

#### Usage:

``` c++
#include <pPlot.h>

Plot plot; // instantiate

void setup (void) {
  Serial.begin(9600);
  delay(100);
}

void loop (void) {
  .... manipulate x ...
  plot.add("sin_x", sin(x),  2);   // plot values offset at +/-2
  plot.add("cos_x", cos(x), -2);
  plot.print(Serial); // see ~ "sin_x=0.50:2.50,cos_x=0.87:-1.13\n"
  ....
}
```
If you use pointers to character strings and floats, and dereference them in the add call you can dynamically create some interesting graph sets. e.g.,
``` c++
// your normal variables, appropriately
// scoped for dereferencing
float velocity, acceleration;

// use simple variables or an array of values or structs
// to hold plot information
char *label1, *label2;
float *value1_p, *value2_p;
float offset1, offset2;

/* set the graph values whenever
 * you want to change what is plotted
 * e.g., for different graph sets */
void cfg_plot_set_1 (void) {
    label1 = "vel";
    value1_p = &velocity;
    offset1 = 0;

    label2 = "accel";
    value2_p = &acceleration;
    offset = -50;
}

void loop (void) {
    ...
    velocity = ... // calculate new values as normal
    acceleration = ...
    ...
    // plot will display new plot points using the updated values
    plot.add(label1, *value1_p, offset1);
    plot.add(label2, *value2_p, offset2);
    plot.print(Serial);
    ...
}
```
Enjoy.
