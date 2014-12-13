
#ifndef _PKG_CNT_H
#define _PKG_CNT_H



int pkg_cnt_wrap(void *in_ptr, int in_size, void **out_ptr, int *out_size, unsigned char *magic);
int pkg_cnt_unwrap(void *in_ptr, u8 max_blobs, u32* bytes_read, u8* *out_ptr, u32* *out_size, u8* *magic, u32 *count);

#endif