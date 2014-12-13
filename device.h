//
// device.h
//

#pragma once

#define	DEV_CMD_LOADRAM			0x20
#define	DEV_CMD_DUMPRAM			0x30
#define	DEV_CMD_SETSAVE			0x70
#define	DEV_CMD_GETVER			0x80
#define	DEV_CMD_UPGRADE			0x84
#define	DEV_CMD_UPGREPORT		0x85
#define	DEV_CMD_PI_RD_32		0x90
#define	DEV_CMD_PI_WR_32		0x91
#define	DEV_CMD_PI_WR_BL		0x94
#define	DEV_CMD_PI_WR_BL_LONG	0x95

#define DEV_MAGIC		0x55444556	// UDEV

typedef struct {
	u8		cmd;			// command byte
	u32		resp_len;		// length in bytes of response
	u32		resp_imm;		// for immediates, actual response (version, etc)
	u32		resp_immb;
	u8		*resp_ptr;		// ptr to long response which must be freed
} dev_cmd_resp_t;

extern void device_find(ftdi_context_t *c);
extern void device_open(ftdi_context_t *c);
extern void device_close(ftdi_context_t *c);
extern u32  device_getver(ftdi_context_t *c, u32 *m);
extern void device_sendcmd(ftdi_context_t *c, dev_cmd_resp_t *resp, 
						   u8 dev_cmd, u8 params, u8 has_resp, u8 has_dma, u32 param1, u32 param2);

extern void device_sendcmd_batch(ftdi_context_t *c, u8 dev_cmd, u32 param1, u32 param2, u8 *tx_buf);
extern void device_sendcmd_commit(ftdi_context_t *c, u8 *tx_buf, u8 num);
extern void device_checkread(ftdi_context_t *c, char *msg);
extern void device_checkwrite(ftdi_context_t *c, char *msg);