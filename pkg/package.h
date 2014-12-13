
#ifndef _PACKAGE_H
#define _PACKAGE_H

typedef struct {
	u32		package_format;
	u8		package_copyright[64];
	u8		package_date[32];
	u8		package_file[64];
	u32		package_type;
	u8		package_type_text[32];
	u8		target_product[16];
	u8		target_product_text[64];
	u8		target_device[32];
	u8		target_magic[16];
	u8		target_variant[8];
	u16		content_version;
	u8		content_version_special;
	u8		content_version_text[16];
	u32		prerequisites;
	u8		prerequisites_text[128];
	u8		content_note[128];
	u8		content_changes[1024];
	u8		content_errata[128];
	u8		content_extra[128];

} pkg_metadata;

typedef struct {
	u8		magic[4];
	u32		count;
	u32		length;
	u32		length_unpack;
	u32		crc;
} pkg_container;


#endif
