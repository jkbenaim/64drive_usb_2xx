
64drive USB loader for Linux
_____________________________________________________________________

original code (c) 2014 Retroactive
ported to Linux by jkbenaim

See the original readme in readme.txt.

Building
--------
Similarly to the Windows version, you will need some files from
FTDI's D2XX driver package:
  - ftd2xx.h
  - libftd2xx.a
  - WinTypes.h
Copy these files into the main directory here.


This port is missing two features from the original Windows version:

  1. The original Windows version has code for a filepicker. I don't
     think it was actually used. I commented it out.
     
  2. The original Windows version has a sweet text-mode progress bar.
     I broke it.
     
     
     