//
// image.c
//

#include "usb.h"
#include "image.h"
#include "device.h"
#include "helper.h"
#include "inttypes.h"
// #include <io.h>
#define LONGLONG int64_t
typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG  HighPart;
  };
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;

u8		buffer[CHUNK_SIZE];


void image_load(game_files_t *g, ftdi_context_t *c)
{
	int		i;
	int		cnt = 0;
	FILE	*fp;
	for(i = 0; i < g->num_files; i++) {
		if(g->files[i][0]) {
			// load file
			fp = fopen(g->files[i], g->dump ? "wb" : "rb");
			if(fp == NULL) die(err[DEV_ERR_NULL_FILE], __FUNCTION__);
			if(!g->dump){
				// get file size
				fseek(fp, 0L, SEEK_END);
				g->sizes[i] = ftell(fp);
				fseek(fp, 0L, SEEK_SET);
			}
			// print status
			_printf("%s %s %s %s at 0x%x (%d kb)", 
				g->dump ? "Dumping" : "Loading", g->files[i], g->dump ? "from" : "into", 
				bank_desc[g->types[i]], g->addrs[i], g->sizes[i] / 1024);
			image_transfer(fp, c, g->dump, g->types[i], g->addrs[i], g->sizes[i]);
			if(g->sizes[i] < 1052672 && g->types[i] == BANK_CARTROM && g->addrs[i] == 0 && !g->dump){
				// sanity check 
				_printf("Image is smaller than 1028Kbyte, will probably fail boot CRC.");
				_printf("Please pad the image out to 1028KB.");
			}
			fclose(fp);
			cnt++;
		}
		if(g->save_types[i] > 0) {
			// set save type
			_printf("Setting save type to %s", save_desc[g->save_types[i]-1]);
			image_set_save(c, g->save_types[i] - 1);
		}
	}
	if(c->verbose) _printf(info[INFO_TOTALDONE], cnt);
}

void image_transfer(FILE *fp, ftdi_context_t *c, u8 dump, u8 type, u32 addr, u32 size)
{	
	u32		ram_addr = addr;
	int		bytes_left = size;
	int		bytes_done = 0;
	int		bytes_do;
	int		trunc_flag = 0;
	int		i; 
	int		chunk = 0;
	LARGE_INTEGER	time_start;
	LARGE_INTEGER	time_stop;
	LARGE_INTEGER	time_freq;
	LONGLONG		time_diff;
	double			time_duration;
	dev_cmd_resp_t	r;
  
	// make sure handle is valid
	if(!c->handle) die(err[DEV_ERR_NULL_HANDLE], __FUNCTION__);

	// decide a better, more optimized chunk size
	if(size > 16 * 1024 * 1024) 
		chunk = 32;
	else if( size > 2 * 1024 * 1024)
		chunk = 16;
	else 
		chunk = 4;

	// convert to megabytes
	chunk *= 128 * 1024;
	if(c->verbose) _printf(info[INFO_CHUNK], CHUNK_SIZE);
	if(c->verbose) _printf(info[INFO_OPT_CHUNK], chunk);

	// get initial time count
        // TODO: port this to Linux
// 	QueryPerformanceFrequency(&time_freq);
// 	QueryPerformanceCounter(&time_start);

	while(1){
		if(bytes_left >= chunk) 
			bytes_do = chunk;
		else
			bytes_do = bytes_left;
		if(bytes_do % 512 != 0) {
			trunc_flag = 1;
			bytes_do -= (bytes_do % 512);
		}
		if(bytes_do <= 0) break;
		
		for(i = 0; i < 2; i++){
			if(i == 1) {
				printf("\n");
				_printf("Retrying\n");

				FT_ResetPort(c->handle);
				FT_ResetDevice(c->handle);
				_printf("Retrying FT_ResetDevice() success\n");
				// set synchronous FIFO mode
				//FT_SetBitMode(c->handle, 0xff, 0x40);
				_printf("Retrying FT_SetBitMode() success\n");
				FT_Purge(c->handle, FT_PURGE_RX | FT_PURGE_TX);
				_printf("Retrying FT_Purge() success\n");

			}
			device_sendcmd(c, &r, dump ? DEV_CMD_DUMPRAM : DEV_CMD_LOADRAM, 2, 0, 1, 
				ram_addr, (bytes_do & 0xffffff) | type << 24);

			if(dump){
				c->status = FT_Read(c->handle, buffer, bytes_do, &c->bytes_written);
				fwrite(buffer, bytes_do, 1, fp);
			}else{
				fread(buffer, bytes_do, 1, fp);
				c->status = FT_Write(c->handle, buffer, bytes_do, &c->bytes_written);
			}
			if(c->bytes_written) break;
		}
		// check for a timeout
		if(c->bytes_written == 0) die(err[DEV_ERR_TIMED_OUT], __FUNCTION__);
		// dump success response
		c->status = FT_Read(c->handle, buffer, 4, &c->bytes_read);

		bytes_left -= bytes_do;
		bytes_done += bytes_do;
		ram_addr += bytes_do;

		// progress bar
		prog_draw(bytes_done, size);

		c->status = FT_GetStatus(c->handle, &c->bytes_read, &c->bytes_written, &c->event_status);
	}
	// stop the timer
	// TODO: port this to Linux
// 	QueryPerformanceCounter(&time_stop);
	time_diff = time_stop.QuadPart - time_start.QuadPart;
	// get the difference of the timer
        // TODO: uncomment this for Linux
// 	time_duration = ((double) time_diff * 1000.0 / (double) time_freq.QuadPart) / 1000.0f;
	// erase progress bar
	prog_erase();
	if(c->verbose && trunc_flag) 
		_printf(info[INFO_TRUNCATED]);
	if(c->verbose) 
		_printf(info[INFO_COMPLETED_TIME],	time_duration, (float)size/1048576.0f/(float)time_duration);
}

void image_set_save(ftdi_context_t *c, u8 save_type)
{	
	dev_cmd_resp_t r;
	device_sendcmd(c, &r, DEV_CMD_SETSAVE, 1, 0, 0, save_type, 0);
}

