CC = gcc
CFLAGS = -g -Wall -ldl -lpthread

SRC = 	device.c \
	helper.c \
	image.c \
	local.c \
	upgrade.c \
	usb.c \
	pkg/pkg_cnt.c \
	lzf/lzf_c.c \
	lzf/lzf_d.c \
	lzf/crc32.c
	
OBJ := $(patsubst %.c, %.o, $(filter %.c,$(SRC)))

.PHONY:all
all: 64drive_usb

	
.PHONY:clean
clean:
	rm -fv *.o lzf/*.o pkg/*.o 64drive_usb

64drive_usb: $(OBJ) Makefile libftd2xx.a
	$(CC) $(OBJ) $(CFLAGS) libftd2xx.a -o 64drive_usb
	
%.o:%.c Makefile
	$(CC) -c $< -o $@
	