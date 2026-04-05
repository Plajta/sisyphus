# Sisyphus
This repository includes the hardware, firmware, chassis and documentation for our soundboard helping those who need it (or don't and want a thing that makes sounds).

Code for the firmware's web configurator (Lithos) can be found [here](https://github.com/Plajta/lithos).

## Key features
Built using open source software on widely documented hardware.

With RP2040 as it's beating heart and a nice crisp I2S DAC, it can (but probably won't be used to) do CD quality audio.

Built into a strudy, but slim and relatively lite 3D printed construction, held together by friction and screws for ease of repair and construction. (construction manual coming soon™)

## Directory structure
- [3D](3D/) - FreeCAD project, the [STLs](3D/STLs) subdirectory includes the exported files ready for 3D printing
- [PCBs](PCBs/) - Two KiCAD projects, one for Sisyphus's [mainboard](PCBs/sisyphus) and one for our own slim [RGB sensor module](PCBs/rgb_sensor)
- [src](src/) - The board's firmware, has it's own README, also usable on a Raspberry Pi Pico with a bit of configuration

## Licensing
All main directories have their own LICENSE files.

`3D` and `PCBs` are under CC-BY-SA. `src` is licensed under GPL3.
That means you can use the whole thing, or just a part of it in your commercial projects but must publically share your potential changes under the same license.
