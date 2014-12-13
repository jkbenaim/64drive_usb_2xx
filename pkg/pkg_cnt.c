
#include "stdio.h"
#include "string.h"
#include "stdlib.h"

#include "common.h"
#include "../helper.h"
#include "package.h"
#include "pkg_cnt.h"
#include "../lzf/lzf.h"
#include "../lzf/crc32.h"


int pkg_cnt_unwrap(void *in_ptr, u8 max_blobs, u32* bytes_read, u8* *out_ptr, u32* *out_size, u8* *magic, u32 *count)
{
	pkg_container	*c = in_ptr;
	void			*in_start;
	void			*data;
	int				data_size;
	u8				*inc_ptr;
	u32				inc_read;

	if(in_ptr == NULL){
		_printf("%s: in_ptr is NULL\n", __FUNCTION__);
		return RET_FAIL;
	}
	if(out_size == NULL){
		_printf("%s: out_size is NULL\n", __FUNCTION__);
		return RET_FAIL;
	}

	in_start = (u8*)in_ptr + (u8)sizeof(*c);

	if(c->magic[2] != '0'){
		_printf("%s: unknown block version\n", __FUNCTION__);
		return RET_FAIL;
	}
	data_size = c->length;
	data = malloc(c->length_unpack);
	if(data == NULL){
		_printf("%s: malloc'd data is NULL\n", __FUNCTION__);
		return RET_FAIL;
	}
	if(c->crc != crc32(0, in_start, c->length)){
		_printf("%s: CRC32 failed\n", __FUNCTION__);
		return RET_FAIL;
	}
	if(c->magic[3] == 'C'){
		// lzf compressed
		lzf_decompress(in_start, c->length, data, c->length_unpack);
		data_size = c->length_unpack;
	} else if(c->magic[3] == 'U'){
		// uncompressed
		memcpy(data, in_start, c->length);
		data_size = c->length;
	} else {
		_printf("Unknown blob compression");
		return RET_FAIL;
	}

	if(c->count){
		// contains child blobs
		int b;
		inc_ptr = data;
		for(b = 0; b < (c->count < max_blobs ? c->count : max_blobs); b++){
			pkg_cnt_unwrap(inc_ptr, max_blobs, &inc_read, out_ptr, out_size, magic, count);
			// increment pointer past that block, accounting for wrapping overhead + EOB marker
			inc_ptr += inc_read + sizeof(*c) + 4;
		}
		*bytes_read = c->length;
	} else {
		// actual data, copy out
		*bytes_read = c->length;

		out_ptr[*count] = malloc(data_size);
		magic[*count] = malloc(4);
		out_size[*count] = data_size;
		memcpy(out_ptr[*count], data, data_size);
		memcpy(magic[*count], c->magic, 4);
		
		if(strncmp(magic[*count], "PM0", 3) == 0){
			if(data_size != sizeof(pkg_metadata)){
				_printf("Reported metadata blob size doesn't match internal struct");
				return RET_FAIL;
			}
		}
		(*count)++;
	}
	free(data);
	return RET_OK;
}