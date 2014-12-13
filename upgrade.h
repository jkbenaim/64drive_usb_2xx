//
// upgrade.h
//

#pragma once


void upgrade_load(upgrade_files_t *u, ftdi_context_t *c);
void upgrade_firm(ftdi_context_t *c);
void upgrade_bootld(ftdi_context_t *c, u8 *data, u32 size);

u32  upgrade_get_report(ftdi_context_t *c);
void upgrade_transfer(u8 *ptr, ftdi_context_t *c, u8 dump, u32 addr, u32 size);

enum {
	UPG_STATUS_RESET,
	UPG_STATUS_READY,
	UPG_STATUS_CHECK,
	UPG_STATUS_ER_0,
	UPG_STATUS_ER_25,
	UPG_STATUS_ER_50,
	UPG_STATUS_ER_75,
	UPG_STATUS_WR_0,
	UPG_STATUS_WR_25,
	UPG_STATUS_WR_50,
	UPG_STATUS_WR_75,
	UPG_STATUS_INVALID,
	UPG_STATUS_SUCCESS,
	UPG_STATUS_BADGEN,
	UPG_STATUS_BADVARIANT,
	UPG_STATUS_BADVERIFY
};