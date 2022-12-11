# Cirque Plot Sketch

## Intro

cirque.plot.ino uses the Arduino IDE's Serial Plotter (Ctrl-Shift-L) to display Absolute or Relative trackpad data.

The setup and execution of the plotting sketch is nearly identical to cirque_demo.ino except that is makes use of my pPlot library for plotting.

## Plotting Code

Here each value is given a string label and added to the plot string. "Button" taps are displayed as a spike to a level designated for that button.

```c++
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
```

The button flags are masked off and if set the button's plotting value is returned, else 0.

```c++
// mask out a button flag and return plot value
int button_value(trackpad_data_t& trackpadData, uint8_t button, int value) {
  return (trackpadData.rel_data.buttons & button) ? value : 0;
}
```

## Plot Samples

### Absolute Data

<img src="assets\abs_plot.png" alt="abs plot" style="zoom:75%;" />

### Relative Data

The spokes at the end are me doing Primary and Secondary button taps.

<img src="assets\rel_plot.png" alt="rel plot" style="zoom:75%;" />