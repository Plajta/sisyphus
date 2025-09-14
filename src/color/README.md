# Color subsystem
This directory contains the color sensor initialization and color processing code.

The code in [color.c](./color.c) is basically just a split-off part of [main.c](../main.c) so it's more readable.

Sisyphus uses a [color LUT](#color-lut) for choosing which sound it should play. Each color in that LUT has its assigned letter, which is used to determine a part of the played file's name.
# Adding a new sensor
When adding a new sensor, copy over an already existing one (eg. VEML3328) and edit over that.
Don't forget to edit the sensor's `CMakeLists.txt` and add it to [sensors/CMakeLists.txt](sensors/CMakeLists.txt).

# Color LUT
Colors and their name codes are described here in this format:
```
<H> <S> <V> <color_code>\n
```
The color is defined in a HSV format because it is better suited for color matching during detection.
H, S and V are in a DECIMAL format, and are space separated. Older beta implementations of this LUT used a HTML-like RGB hexadecimal notation.

Color code can be any letter of the alphabet, be sure to save this file in ASCII so you don't get any unicode characters in there and accidentally break it.

The line MUST be terminated by a newline, it wouldn't break the program but it WILL skip the line.

The values you define here are compared against the measured color by calculating the difference (`measured_value - defined_value`) for each channel.
The entry that has the smallest overall difference is selected as the match.
In the future we may choose to ignore some channels, such as S or V, if they prove unnecessary for detection.
