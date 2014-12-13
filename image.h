//
// image.h
//

#pragma once

extern void image_load(game_files_t *g, ftdi_context_t *c);
extern void image_transfer(FILE *fp, ftdi_context_t *c, u8 dump, u8 type, u32 addr, u32 size);
extern void image_set_save(ftdi_context_t *c, u8 save_type);