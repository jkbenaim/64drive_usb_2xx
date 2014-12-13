//
// local.c
//

#include "local.h"

char *err[] = {	
	"Success",
	"Device read timeout/error",
	"Device read timeout/error",
	"Called with null FTDI handle",
	"Called with null pointer argument",
	"Null file handle",
	"No devices found",
	"Couldn't find device",
	"Can't open device",
	"malloc() failed",
	"Bad device magic",
	"Timed out during transfer. Cycle power and retry",
	"Make sure your 64drive has firmware 2.00 or later",
	"Unknown error",
};

char *info[] = {
	"Verbose output on",
	"Found device at 0x%x",
	"Opened device, firmware %.2f, variant %s",
	"Device magic: %s",
	"Querying device firmware version:",
	"Completed in %.2f seconds (%.2f MB/sec)",
	"Truncated operation to nearest 512bytes",
	"Max chunk size: %d",
	"Optimized chunk size: %d",
	"Total %d files transferred",
	"Couldn't decide what bank %s is, ignoring",
	"Enumerating devices recognized by FTDI driver",
	"Can't find images with given criteria",
	"Done",
};

char *bank_desc[] =	{
	"INVALID",
	"Cartridge ROM",
	"SRAM 256kbit",
	"SRAM 768kbit",
	"FlashRAM 1Mbit",
	"FlashRAM 1Mbit (PokeStdm2)",
	"EEPROM"
};

char *save_desc[] =	{
	"None (default)",
	"EEPROM 4k",
	"EEPROM 16k",
	"SRAM 256kbit",
	"FlashRAM 1Mbit",
	"SRAM 768kbit (Dezaemon 3D)",
	"FlashRAM 1Mbit (PokeStdm2)"
};
