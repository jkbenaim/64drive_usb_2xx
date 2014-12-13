//
// device.c
//

#include "usb.h"
#include "device.h"
#include "helper.h"

void device_find(ftdi_context_t *c)
{	
	u32 i;
	u32 found = -1;

	if(c->verbose) _printf(info[INFO_VERBOSE]);

	c->status = FT_CreateDeviceInfoList(&c->devices);
	if(c->status != FT_OK) fail(c->status);
	if(c->devices == 0){
		_printf(err[DEV_ERR_NO_DEVICES]);
		exit(-1);
	}else{
		// allocate storage
		c->dev_info = 
			(FT_DEVICE_LIST_INFO_NODE*) malloc( sizeof(FT_DEVICE_LIST_INFO_NODE) *c->devices);
		// get device info list
		c->status = FT_GetDeviceInfoList(c->dev_info, &c->devices);
		if(c->status == FT_OK && c->verbose ){
			_printf(info[INFO_ENUMERATING]);
			for(i = 0; i < c->devices; i++){
				printf("\t[Device %d]\n", i);
				printf("\t-- Flags\t0x%x\n",	c->dev_info[i].Flags); 
				printf("\t-- Type\t\t0x%x\n",	c->dev_info[i].Type); 
				printf("\t-- ID\t\t0x%x\n",		c->dev_info[i].ID); 
				printf("\t-- LocId\t0x%x\n",	c->dev_info[i].LocId); 
				printf("\t-- SerialNumber\t%s\n", c->dev_info[i].SerialNumber); 
				printf("\t-- Name\t\t%s\n",		c->dev_info[i].Description); 
			}
		}
	}

	for(i = 0; i < c->devices; i++){
		if(strcmp(c->dev_info[i].Description, "64drive USB device A") == 0){
			if(c->verbose) {
				_printf(info[INFO_FOUNDAT], c->dev_info[i].LocId);
			}
			found = FT_OK;
			break;
		}
	}
	free(c->dev_info);
	if(found != FT_OK){
		_printf(err[DEV_ERR_CANT_FIND]);
		exit(-1);
	}
}

void device_open(ftdi_context_t *c)
{
	u32 magic = 0;
	u32 ver = 0;
	c->status = FT_Open(0, &c->handle);
	if(c->status != FT_OK || !c->handle){
		die(err[DEV_ERR_CANT_OPEN], __FUNCTION__);
	}
	// set read/write timeouts
	fail(FT_ResetDevice(c->handle));
	fail(FT_SetTimeouts(c->handle, 5000, 5000));
	// set synchronous FIFO mode
	//fail(FT_SetBitMode(c->handle, 0xff, 0x40));
	fail(FT_Purge(c->handle, FT_PURGE_RX | FT_PURGE_TX));

	ver = device_getver(c, &magic) ;
	if(c->verbose) {
		_printf(info[INFO_QUERYFW]);
		printf(" %08x\n", ver);
		_printf(info[INFO_DEVMAGIC], c->magic);
	} else {
		_printf(info[INFO_OPENED],	((ver&0xffff)) / 100.0f, c->variant);
	}
}

void device_close(ftdi_context_t *c)
{
	c->status = FT_Close(c->handle);
	//_printf(info[INFO_DONE]);
}

u32 device_getver(ftdi_context_t *c, u32 *magic)
{
	dev_cmd_resp_t r;
	u32 val;
	u32 dummy[32] = {0, };

	if(!c->handle) die(err[DEV_ERR_NULL_HANDLE], __FUNCTION__);

	c->status = FT_Write(c->handle, dummy, 4, &c->bytes_written) ;

	device_sendcmd(c, &r, DEV_CMD_GETVER, 0, 1, 0, 0, 0);
	val = swap_endian(r.resp_imm);
	*magic = swap_endian(r.resp_immb);
	if(val == 0){
		// some configs
		fail(FT_Purge(c->handle, FT_PURGE_RX | FT_PURGE_TX));
		device_sendcmd(c, &r, DEV_CMD_GETVER, 0, 1, 0, 0, 0);
		val = swap_endian(r.resp_imm);
		*magic = swap_endian(r.resp_immb);
	}
	if(*magic != DEV_MAGIC) 
		die(err[DEV_ERR_BADMAGIC], __FUNCTION__);
	if(val == 0) 
		die(err[DEV_ERR_UNKNOWN], __FUNCTION__);

	sprintf(c->magic, "%c%c%c%c", (*magic >> 24)&0xff, (*magic >> 16)&0xff, (*magic >> 8)&0xff, (*magic)&0xff);
	sprintf(c->variant, "%c%c", (val >> 24)&0xff, (val >> 16)&0xff);

	return val;
}

void device_sendcmd(ftdi_context_t *c, dev_cmd_resp_t *resp, 
					u8 dev_cmd, u8 params, u8 has_resp, u8 has_dma, u32 param1, u32 param2)
{
	u8	tx_buf[32];
	u32	rx_buf[8];

	if(!c->handle) die(err[DEV_ERR_NULL_HANDLE], __FUNCTION__);

	memset(tx_buf, 0, 32);
	memset(rx_buf, 0, 32);
	tx_buf[0] = dev_cmd; 
	tx_buf[1] = 0x43;	// CMD
	tx_buf[2] = 0x4D;
	tx_buf[3] = 0x44;

	if(params > 0) *(u32 *)&tx_buf[4] = swap_endian(param1);
	if(params > 1) *(u32 *)&tx_buf[8] = swap_endian(param2);

	c->status = FT_Write(c->handle, tx_buf, 4+(params*4), &c->bytes_written) ; 
	device_checkwrite(c, __FUNCTION__);
	if(has_resp && !has_dma) {
		if(resp == NULL) die(err[DEV_ERR_NULL_PTR], __FUNCTION__);
		c->status = FT_Read(c->handle, rx_buf, dev_cmd == DEV_CMD_GETVER ? 8 : 4, &c->bytes_read) ;
		if(c->status != FT_OK || c->bytes_read == 0) {
			// no response to get version, probably older 1.xx firmware
			if(dev_cmd == DEV_CMD_GETVER) _printf(err[DEV_ERR_NOGETVER]);
		}
		device_checkread(c, __FUNCTION__); 

		resp->cmd = dev_cmd;
		resp->resp_len = 4;
		resp->resp_ptr = 0;
		resp->resp_imm = rx_buf[0];
		resp->resp_immb = rx_buf[1];
		//_printf("Grabbed %d bytes in response to cmd %x", c->bytes_read, dev_cmd);
	} 
	if(!has_dma){
		// these two instructions do not return a success
		if(dev_cmd == DEV_CMD_PI_WR_BL || dev_cmd == DEV_CMD_PI_WR_BL_LONG) return;
		c->status = FT_Read(c->handle, rx_buf, 4, &c->bytes_read) ;
		rx_buf[1] = dev_cmd << 24 | 0x504D43;
		if(memcmp(rx_buf, &rx_buf[1], 4) != 0){
			die("Received wrong CMPlete signal", __FUNCTION__);
		}
		//_printf("Grabbed %d bytes in copmlete to cmd %x", c->bytes_read, dev_cmd);
	}
}

void device_sendcmd_batch(ftdi_context_t *c, u8 dev_cmd, u32 param1, u32 param2, u8 *tx_buf)
{
	// these two functions (_batch, _commit) are for emitting the bootloader writes, 
	// which are a special case. they don't return a status like all the other 
	// commands, since that would kill usb latency
	if(!c->handle) die(err[DEV_ERR_NULL_HANDLE], __FUNCTION__);
	memset(tx_buf, 0, 12);
	tx_buf[0] = dev_cmd; 
	tx_buf[1] = 0x43;	// CMD
	tx_buf[2] = 0x4D;
	tx_buf[3] = 0x44;
	*(u32 *)&tx_buf[4] = swap_endian(param1);
	*(u32 *)&tx_buf[8] = swap_endian(param2);
}

void device_sendcmd_commit(ftdi_context_t *c, u8 *tx_buf, u8 num)
{
	if(!c->handle) die(err[DEV_ERR_NULL_HANDLE], __FUNCTION__);
	c->status = FT_Write(c->handle, tx_buf, 12*num, &c->bytes_written) ; 
	device_checkwrite(c, __FUNCTION__);
}

void device_checkread(ftdi_context_t *c, char *msg)
{
	if(c->status != FT_OK || c->bytes_read == 0) {
		die(err[DEV_ERR_TIMEOUT_READ], msg);
	}
}

void device_checkwrite(ftdi_context_t *c, char *msg)
{
	if(c->status != FT_OK || c->bytes_written == 0) {
		die(err[DEV_ERR_TIMEOUT_WRITE], msg);
	}
}