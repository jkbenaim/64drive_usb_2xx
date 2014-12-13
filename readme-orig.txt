
64drive USB loader
_____________________________________________________________________

(c) 2014 Retroactive
Code is BSD-licensed.


Usage
-----

   -l <file> [bank] [addr]        Load binary to bank
   -d <file> <bank> <addr> <len>  Dump binary from bank
               1 - Cartridge ROM              2 - SRAM 256kbit
               3 - SRAM 768kbit               4 - FlashRAM 1Mbit
               5 - FlashRAM 1Mbit (PokeStdm2) 6 - EEPROM 16kbit

   -s <int>                       Set save emulation type
               0 - None (default)       1 - EEPROM 4k
               2 - EEPROM 16k           3 - SRAM 256kbit
               4 - FlashRAM 1Mbit       5 - SRAM 768kbit (Dezaemon 3D)
               6 - FlashRAM 1Mbit (PokeStdm2)
   -b <file.rpk>                  Update bootloader
   -f <file.rpk>                  Update firmware
   -v                             Verbose output for debug

1. Arguments: <required>, [optional]
2. Address and size values are hexadecimal and byte-addressed.
3. Any bank can be loaded at any time. However, some banks overlap
   each other and so only one save bank should be used at once.
4. Save banks can be loaded regardless of what the 64drive has been
   instructed to emulate.
5. Refer to the 64drive Hardware Specification for more information.
6. Use the verbose flag when updating firmware or bootloader to see
   detailed revision information about that update.





Building
--------
I use Visual Studio 2008. You will need:
 - ftd2xx.h
 - ftd2xx.lib
Both are available within FTDI's D2xx driver package, which is a
self-extracting EXE. Open with 7zip or Winrar and pull these files.

If you just want to run the program, the EXE is in /Release/.






Release History
----------------------------------------------------------------------

Nov 25 2014
-----------
 - Initial release
 - Ground-up rewrite of the PC-side code
 - Supports device firmwares 2.xx only
 - Devices running the older 1.xx firmwares must use the older USB 
   loader to upgrade. This newer code can only perform upgrades 
   within the 2.xx series firmwares.
 - Support for chaining multiple operations on the command line. 

EOF