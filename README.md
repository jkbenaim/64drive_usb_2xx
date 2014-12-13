64drive USB loader for Linux
============================
(c) 2014 Retroactive
ported to Linux by jkbenaim

Update firmware and transfer data to/from your 64drive. Requires [FTDI's D2XX driver package](http://www.ftdichip.com/Drivers/D2XX.htm).

## Usage
```console
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
```

See the original readme in readme-orig.txt.

## Building

Similarly to the Windows version, you will need some files from
[FTDI's D2XX driver package](http://www.ftdichip.com/Drivers/D2XX.htm):
  - ftd2xx.h
  - libftd2xx.a
  - WinTypes.h
Copy these files into the project, then run ```console make ```.

## Missing features

This port is missing two features from the original Windows version:

  1. The original Windows version has code for a filepicker. I don't
     think it was actually used. I commented it out.
     
  2. The original Windows version has a sweet text-mode progress bar.
     I broke it.
     
     
## License

Retroactive says it is "BSD-licensed."
