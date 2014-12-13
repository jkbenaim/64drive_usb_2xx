//
// upgrade.c
//

#include "usb.h"
#include "upgrade.h"
#include "device.h"
#include "helper.h"
#include "pkg/package.h"
#include "pkg/pkg_cnt.h"
#include <io.h>

u8		buffer[CHUNK_SIZE];

char *upgrade_status[] = {	
	"UPG_STATUS_RESET",
	"UPG_STATUS_READY",
	"UPG_STATUS_CHECK",
	"UPG_STATUS_ER_0",
	"UPG_STATUS_ER_25",
	"UPG_STATUS_ER_50",
	"UPG_STATUS_ER_75",
	"UPG_STATUS_WR_0",
	"UPG_STATUS_WR_25",
	"UPG_STATUS_WR_50",
	"UPG_STATUS_WR_75",
	"UPG_STATUS_INVALID",
	"UPG_STATUS_SUCCESS",
	"UPG_STATUS_BADGEN",
	"UPG_STATUS_BADVARIANT",
	"UPG_STATUS_BADVERIFY"
};

void upgrade_load(upgrade_files_t *u, ftdi_context_t *c)
{
	int				i;
	int				j;
	FILE			*fp;
	u8				*rpk;
	pkg_metadata	*metadata;

	u32				blob_count = 0;
	u32				blob_size[2];
	u8*				blobs[2];
	u8*				blob_magic[2];

	u8				*asset_read;
	u32				new_size;
	u32				temp;

	char			key;
	
	

	for(i = 0; i < u->num_files; i++) {
		// load file
		fp = fopen(u->files[i], "rb");
		if(fp == NULL) die(err[DEV_ERR_NULL_FILE], __FUNCTION__);
		_printf("Loading %s for %s upgrade", u->files[i], u->firm[i] ? "firmware" : u->bootld[i] ? "bootloader" : "unknown");

		// get file size
		fseek(fp, 0L, SEEK_END);
		u->sizes[i] = ftell(fp);
		fseek(fp, 0L, SEEK_SET);
		rpk = malloc(u->sizes[i]);
		if(rpk == NULL) die(err[DEV_ERR_CANT_MALLOC], __FUNCTION__);
		fread(rpk, u->sizes[i], 1, fp);
		fclose(fp);

		// unwrap (recursively unpack) the package
		if(pkg_cnt_unwrap(rpk, 2, &temp, blobs, &blob_size, blob_magic, &blob_count) != 0){
			die("Problem unwrapping the RPK", __FUNCTION__);
		}
		if(blob_count != 2){
			die("Unexpected number of blobs in RPK", __FUNCTION__);
		}
		// grab the metadata
		if(memcmp(blob_magic[0], "PM", 2) != 0){
			die("Metadata expected but not found", __FUNCTION__);
		}
		metadata = blobs[0];
		if(u->firm[i] && strcmp(metadata->package_type_text, "Firmware") != 0){
			die("Package doesn't contain any firmware!", __FUNCTION__);
		}
		if(u->bootld[i] && strcmp(metadata->package_type_text, "Bootloader") != 0){
			die("Package doesn't contain any bootloader!", __FUNCTION__);
		}
		if(strncmp(metadata->target_magic, c->magic, 4) != 0){
			die("Package isn't meant for this device series", __FUNCTION__);
		}
		if(strncmp(metadata->target_variant, c->variant, 2) != 0){
			die("Package isn't meant for this device variant", __FUNCTION__);
		}
		_printf("%s %s, use -v switch to view more info", metadata->package_type_text, metadata->content_version_text);
		if(c->verbose){
			printf("\n[Target]\nDesigned for %s %s variant %s\n", metadata->target_product_text, metadata->target_magic, metadata->target_variant);
			printf("\n[PackageDate]\n%s\n", metadata->package_date);
			if(metadata->prerequisites_text[0]) printf("\n[PrerequisitesText]\n%s\n", metadata->prerequisites_text);
			if(metadata->content_note[0]) printf("\n[ContentNote]\n%s\n", metadata->content_note);
			if(metadata->content_changes[0]) printf("\n[ContentChanges]\n%s\n", metadata->content_changes);
			if(metadata->content_errata[0]) printf("\n[ContentErrata]\n%s\n", metadata->content_errata);
			if(metadata->content_extra[0]) printf("\n[ContentExtra]\n%s\n", metadata->content_extra);
			printf("\n");
		}
		
		// grab the asset
		if(memcmp(blob_magic[1], "PA", 2) != 0){
			die("Asset blob expected but not found", __FUNCTION__);
		}

		// round up to 512bytes
		new_size = (blob_size[1] / 512 + 1) * 512;
		asset_read = malloc(new_size);
		if(asset_read == NULL) die(err[DEV_ERR_CANT_MALLOC], __FUNCTION__);
		_printf("Checking memory integrity :");
		for(j = 0; j < 8; j++){
			upgrade_transfer(blobs[1],		c, 0, 0x0, new_size);
			upgrade_transfer(asset_read,	c, 1, 0x0, new_size);
			if(memcmp(asset_read, blobs[1], new_size)) {
				printf(" failed in iteration %d\n", j);
				die("Memory integrity check failed! SDRAM malfunction", __FUNCTION__);
			}
		}	
		printf(" passed\n");

		_printf("Do you want to continue with the upgrade (Y/n):");
		key = getchar();
		if(!(key == '\n' || key == 'Y' || key == 'y')) {
			_printf("Cancelled upgrade");
			return;
		}
		if(u->firm[i]){
			upgrade_firm(c);
		}
		if(u->bootld[i]){
			upgrade_bootld(c, blobs[1], blob_size[1]);
		}

		_printf("Please unplug your 64drive to allow the update to take effect");
		// clean up
		for(j = 0; j < blob_count; j++){
			free(blobs[j]);
			free(blob_magic[j]);
		}
		if(rpk != NULL)	free(rpk); else die("Can't free null ptr", __FUNCTION__);
	}
}

u32 upgrade_get_report(ftdi_context_t *c)
{
	dev_cmd_resp_t	r;
	device_sendcmd(c, &r, DEV_CMD_UPGREPORT, 0, 1, 0, 0, 0);
	return swap_endian(r.resp_imm);
}

void upgrade_firm(ftdi_context_t *c)
{
	dev_cmd_resp_t	r;
	int				j = 0;
	u32				resp;

	// behold ye brickproof upgrader
	if((upgrade_get_report(c) & 0xF) != UPG_STATUS_READY){
		die("Upgrade module isn't ready, power cycle?", __FUNCTION__);
	}
	if(c->verbose) _printf("Starting firmware upgrade process...");
	device_sendcmd(c, &r, DEV_CMD_UPGRADE, 0, 0, 0, 0, 0);
	while(1){
		Sleep(100);
		resp = upgrade_get_report(c) & 0xf;

		printf("\r");
		switch(resp){
		case UPG_STATUS_RESET:
			die("Upgrade module is stuck in reset", __FUNCTION__); break;
		case UPG_STATUS_READY:
			die("Upgrade module never started", __FUNCTION__); break;
		case UPG_STATUS_INVALID:
			die("Unknown status!", __FUNCTION__); break;
		case UPG_STATUS_BADGEN:
			die("Upgrade module encountered a general error", __FUNCTION__); break;
		case UPG_STATUS_BADVARIANT:
			die("Device didn't accept the mismatched variant", __FUNCTION__); break;
		case UPG_STATUS_BADVERIFY:
			die("Upgrade module failed while verifying the package", __FUNCTION__); break;
		case UPG_STATUS_ER_0:
		case UPG_STATUS_ER_25:
		case UPG_STATUS_ER_50:
		case UPG_STATUS_ER_75:
			_printf("Erasing firmware :"); break;
		case UPG_STATUS_WR_0:
			_printf("Writing firmware [0%%]:"); break;
		case UPG_STATUS_WR_25:
			_printf("Writing firmware [25%%]:"); break;
		case UPG_STATUS_WR_50:
			_printf("Writing firmware [50%%]:"); break;
		case UPG_STATUS_WR_75:
			_printf("Writing firmware [75%%]:"); break;
		case UPG_STATUS_SUCCESS:
			_printf("Written successfully! "); return;
		}
		if(j++ == 5000) die("Took too long, giving up upgrade", __FUNCTION__);
	}
}

void upgrade_bootld(ftdi_context_t *c, u8 *data, u32 size)
{
	dev_cmd_resp_t	r;
	u32				i = 0;
	u32				j = 0;
	u32				resp;

	if(c->verbose) _printf("Starting bootloader upgrade process...");
	_printf("Please ensure your 64drive is NOT plugged into any N64!");
	_printf("Press Enter to continue :");
	getchar();

	// chip erase
	printf("\r");
	_printf("Performing chip erase   :");
	// switch CPLD to whole-chip access
	device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x17000000, 0x0000);
	device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x5555*2, 0xAA00 << 8);
	device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x2AAA*2, 0x5500 << 8);
	device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x5555*2, 0x8000 << 8);
	device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x5555*2, 0xAA00 << 8);
	device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x2AAA*2, 0x5500 << 8);
	device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x5555*2, 0x1000 << 8);
	Sleep(2000);
	printf("\r");
	_printf("Writing data...          ");

	while(i < size){
		u16 word;
		u8 *tx_ptr = buffer;
		if((i % 16384) == 0) prog_draw(i, size);
		// batch up to 16 words to write in a single usb transaction.
		// this is a special case, these device commands are the only ones not to transmit
		// a success token, since waiting for it makes the process take forever.
		for(j = 0; j < 16; j++){
			word = data[i] << 8 | data[i+1];
			device_sendcmd_batch(c, DEV_CMD_PI_WR_BL, 0x5555*2, 0xAA00 << 8, tx_ptr); tx_ptr += 12;
			device_sendcmd_batch(c, DEV_CMD_PI_WR_BL, 0x2AAA*2, 0x5500 << 8, tx_ptr); tx_ptr += 12;
			device_sendcmd_batch(c, DEV_CMD_PI_WR_BL, 0x5555*2, 0xA000 << 8, tx_ptr); tx_ptr += 12;
			device_sendcmd_batch(c, DEV_CMD_PI_WR_BL_LONG, i, word, tx_ptr); tx_ptr += 12;			
			i += 2;
		}
		device_sendcmd_commit(c, buffer, 4*16);		
	}
	/*
		device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x5555*2, 0xAA00 << 8);
		device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x2AAA*2, 0x5500 << 8);
		device_sendcmd(c, &r, DEV_CMD_PI_WR_BL, 2, 0, 0, 0x5555*2, 0xA000 << 8);
		device_sendcmd(c, &r, DEV_CMD_PI_WR_BL_LONG, 2, 0, 0, i, word);
	*/
	prog_erase();
}

void upgrade_transfer(u8 *ptr, ftdi_context_t *c, u8 dump, u32 addr, u32 size)
{	
	dev_cmd_resp_t	r;

	// make sure handle is valid
	if(!c->handle) die(err[DEV_ERR_NULL_HANDLE], __FUNCTION__);

	//_printf("Starting transfer op at address 0x%x, size %x", addr, new_size);	
	device_sendcmd(c, &r, dump ? DEV_CMD_DUMPRAM : DEV_CMD_LOADRAM, 2, 0, 1,
		addr, (size & 0xffffff) | 1 << 24);

	if(dump){
		c->status = FT_Read(c->handle, ptr, size, &c->bytes_written);
	}else{
		c->status = FT_Write(c->handle, ptr, size, &c->bytes_written);
	}

	// check for a timeout
	if(c->bytes_written == 0) die(err[DEV_ERR_TIMED_OUT], __FUNCTION__);
	if(c->bytes_written != size) _printf("Mismatch in bytes written: was %d, should be %d", c->bytes_written, size);
	// dump response
	c->status = FT_Read(c->handle, buffer, 4, &c->bytes_read);
	c->status = FT_GetStatus(c->handle, &c->bytes_read, &c->bytes_written, &c->event_status);
}
