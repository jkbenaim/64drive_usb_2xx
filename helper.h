//
// helper.h
//

#pragma once

#include "local.h"

// prototypes
//
extern u32 swap_endian(u32 val);
extern void image_pick(char *filename);
extern void _printf(const char *format, ...);
extern void prog_draw(u32 amount, u32 total);
extern void prog_erase();

extern void fail(unsigned long st);
extern void die(char *cause, char *msg);

// globals
//
extern char	*compile_date;
