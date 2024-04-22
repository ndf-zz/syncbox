# syncbox

MIDI/USB to DIN sync converter

![Prototype](prototype.jpg "Prototype")

[in progress]

## Contents

   - pcb: Kicad schematic and circuit board
   - enclosure: FreeCAD enclosure
   - firmware: STM32 firmware

## Firmware installation

From the firmware folder, install development environment
(gcc, binutils, openocd & gdb) using make:

	$ make requires

Build the combined firmware and fash loader, then upload
to attached target via stlink/SWD:

	$ make upload

