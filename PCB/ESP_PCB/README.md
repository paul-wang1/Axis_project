This file contains the KiCAD PCB files for the ESP32 to Daisy Board

The board features:
 - A 9V barrel jack for voltage input
 - A 9V to 5V buck converter and 5V to 3.3V LDO
 - An ESP32 C3 Wroom N2 board
 - A 6-pin header pin connector designed for ESP prog flashing (in UART)
 - A 4-pin header pin connector for I2C communication with the Daisy C

Progress:
 - Finished the schematic (ignored two errors on the ERC that could not be fixed; may be a source of concern)
 - Finished spacing each of the components
 - Started wiring the footprints

Key Constraints:
 - Only a singular top plane allowed, no ground plane due to the milling machine not being able to include vias
 - Relies on I2C communication to the Daisy C
 - Needs bluetooth to function properly
