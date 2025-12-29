# Color subsystem
This directory contains the color sensor initialization and color processing code.

The code in [color.c](./color.c) is basically just a split-off part of [main.c](../main.c) so it's more readable.

Sisyphus uses a [color LUT](#color-lut) for choosing which sound it should play. Each color in that LUT has its assigned letter, which is used to determine a part of the played file's name.
Each color also optionally can have a representation to be shown using the ARGB LED indicator. This is useful for user feedback.

# Adding a new sensor
When adding a new sensor, copy over an already existing one (eg. VEML3328) and edit over that.
Don't forget to edit the sensor's `CMakeLists.txt` and add it to [sensors/CMakeLists.txt](sensors/CMakeLists.txt).

# Color LUT
Colors and their name codes are described here in this format:
```
<H> <S> <V> <color_code> <color_representation>\n
```
The color is defined in a HSV format because it is better suited for color matching during detection.
H, S and V are in a DECIMAL format, and are space separated.

Color code can be any letter of the alphabet, be sure to save this file in ASCII so you don't get any unicode characters in there and accidentally break it.

The color representation is optional but highly recommended for enhanced user visual feedback. It is written as a standard RGB hexadecimal value.
If no color representation is defined, a muted white will be used.

The line MUST be terminated by a newline, it wouldn't break the program but it WILL skip the line.
Lines longer than 21 characters will get ignored because they cannot be valid.

An example of a valid LUT can look like this:
```
22 44 55 r AA0000
86 22 47 g
281 6 41 b 0000AA
220 32 73 c 00AAAA
335 18 51 m AA00AA
65 42 74 y AAAA00
```
Notice `g` is missing it's color representation, that is still valid but discouraged.

The values you define here are compared against the measured color by calculating the difference (`measured_value - defined_value`) for each channel.
The entry that has the smallest overall difference is selected as the match.
In the future we may choose to ignore some channels, such as S or V, if they prove unnecessary for detection.
