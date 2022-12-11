#ifndef PLOT_H
#define PLOT_H

#include <Streaming.h>
#include <String.h>

/*****************************************************************************
 * Plot - Format a plotter string for use with the Arduino IDE's
 *        Serial Plotter.
 *
 * Plotter string format: "label[=value]:value[,label[=value]:value...]\n
 *
 * The plot string is empty on initialization and after calling print().
 * Additional plot items are appended after a comma.
 * The plot string line is terminated with endl on print().
 *
 * label   - the label that appears in the header
 * float   - the numeric value that is used for graphing, and optionally
 *           appended to the label (e.g., "angle=3.14")
 * offset  - offsets the graph value so graphs can be staggered across the
 *           window. The actual value can be displayed with the label.
 * value_in_label - appends "=value" to the label in the header.
 * os      - the output stream to print to, typically Serial.
 * min,max - limit the vertical range of display graph values. Does not affect
 *           values displayed with the labels in the header.
 *
 * Usage:
 *  Plot plot;                  // declare an instance
 *  plot.add("fx", sin(x),  5); // plot fx around +5 with values in the header
 *  plot.add("fx", sin(y), -5); // plot fy around -5
 *  plot.print(Serial);         // Send the plot info to the Serial Plotter.
 *  constrain_on(min,max);      // turn graphing constrains to clip graph at
 *                              // min and max, including offsets.
 *  constrain_off();            // turn off graph constraints/
 *
 * Tips:
 *  Add a fixed value to maintain the autoranging boundaries,
 *  e.g., add("t",95,false); add("b",-95, false); to display horizontal lines
 *  near the edge of a graph ranging from 100 to -100.
 *
 *  You can change graph sets interactively - just plot the same number of
 *  values. e.g, Plot (x,y,z) sets for gyroscope, accelerometer,
 *  magnetometer, and (pitch,roll,yaw) for IMS sensor fusion.
 *****************************************************************************/

class Plot {
 public:
 
  void add(String label, float value, int offset, bool value_in_label=true) {
    add(label, value, value_in_label, float(offset));
  } // needed for integer offset

  void add(String label, float value, float offset, bool value_in_label=true) {
    add(label, value, value_in_label, offset);
  } // allows default value_in_label

  void add(String label, float value, bool value_in_label=true, float offset=0) {
    if (str.length()) str += String(",");
    str += label;
    if (value_in_label) str += (String("=") + value);
    str += (String(":") + _constrain(value + offset));
  } // allows default offset, then value_in_label

  void print (Stream& os) {
    os << str << endl;
    str = "";
  }

  void constrain_off (void) {
    constrained = false;
  }

  void constrain_on (float vmin, float vmax) {
     if (vmin > vmax) return;
    constraint_min = vmin;
    constraint_max = vmax;
    constrained = true;
  }

 private:

  float _constrain (float val) {
    if (not constrained) return val;
    return constrain(val, constraint_min, constraint_max);
  }

  String str;
  bool constrained;
  float constraint_max, constraint_min;
};

#endif /* _H */
