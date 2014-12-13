//
// usb.h
//

#pragma once

// #ifndef _WIN32_WINNT           
// #define _WIN32_WINNT 0x0500
// #endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
// #include <windows.h>

#ifndef DEBUG
#define FTD2XX_EXPORTS	// used for statically linking FTDI
#endif

#include "ftd2xx.h"

typedef unsigned char	u8;
typedef unsigned short	u16;
typedef unsigned int	u32;
typedef char			s8;
typedef short			s16;
typedef int				s32;

#define CHUNK_SIZE (4*1024*1024)

#define MODE_UNSPECIFIED		0
#define MODE_LOADIMAGE_CODE		1
#define MODE_LOADIMAGE_NAME		2
#define MODE_LOADIMAGE_BANK		4
#define MODE_DUMPIMAGE_BANK		8
#define MODE_SAVETYPE_BANK		16
#define MODE_UPGRADE_FIRM		64
#define MODE_UPGRADE_BOOTLD		128

enum {
	BANK_INVALID,
	BANK_CARTROM,
	BANK_SRAM256,
	BANK_SRAM768,
	BANK_FLASHRAM1M,
	BANK_FLASHPKM1M,
	BANK_EEPROM16,
	BANK_LAST
};

enum {
	SAVE_INVALID,
	SAVE_EEP4K,
	SAVE_EEP16K,
	SAVE_SRAM256K,
	SAVE_FLASHRAM1M,
	SAVE_SRAM768K,
	SAVE_FLASHPKM1M,
	SAVE_LAST
};

typedef struct {
	DWORD		rx_queue;
	DWORD		tx_queue;
	DWORD		event_status;
	DWORD		devices;
	FT_STATUS	status;
	FT_DEVICE_LIST_INFO_NODE *dev_info;
	FT_HANDLE	handle;
	DWORD		bytes_written;
	DWORD		bytes_read;
	DWORD		verbose;

	char		magic[5];
	char		variant[3];
} ftdi_context_t;


#define MAX_ROM_FILES 16
#define MAX_UPGRADE_FILES 16

typedef struct {
	u8			num_files;
	char		image_base[256];
	u8			dump;
	char		files[MAX_ROM_FILES][512];
	u32			sizes[MAX_ROM_FILES];
	u32			addrs[MAX_ROM_FILES];
	u8			types[MAX_ROM_FILES];
	u8			save_types[MAX_ROM_FILES];
} game_files_t;

typedef struct {
	u8			num_files;
	u8			firm[MAX_UPGRADE_FILES];
	u8			bootld[MAX_UPGRADE_FILES];
	char		files[MAX_UPGRADE_FILES][512];
	u32			sizes[MAX_UPGRADE_FILES];
} upgrade_files_t;


// prototypes
//
// extern void parse_args(game_files_t *g, int argc, char *argv[]);
extern void parse_args(game_files_t *g, upgrade_files_t *u, int argc, char *argv[]);
extern void invalid_args();