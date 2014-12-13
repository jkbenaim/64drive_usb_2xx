//
// helper.c
//

#include "usb.h"
#include "helper.h"
#include <stdarg.h>
#define MAX_PATH 260

char	*compile_date = __TIMESTAMP__;

u32 swap_endian(u32 val)
{
	return ((val<<24) ) | 
		   ((val<<8)  & 0x00ff0000) |
		   ((val>>8)  & 0x0000ff00) | 
		   ((val>>24) );
}

// void image_pick(char *filename)
// {
// 	static char szFilter[] = "Images (*.z64, *.v64, *.bin, *.rom,)\0*.z64;*.v64;*.bin;*.rom;\0All Files\0*.*;\0\0";
// 	static char szFileName [MAX_PATH], szTitleName [MAX_PATH];
// 	static OPENFILENAME ofn;
// 	memset(&ofn, 0, sizeof (OPENFILENAME));
// 
// 	ofn.lStructSize       = sizeof (OPENFILENAME) ;
// 	ofn.lpstrFilter       = szFilter ;
// 	ofn.lpstrFile         = szFileName ;
// 	ofn.nMaxFile          = MAX_PATH ;
// 	ofn.lpstrFileTitle    = szTitleName ;
// 	ofn.nMaxFileTitle     = MAX_PATH ;
// 	ofn.lpfnHook		  = NULL;
// 	ofn.Flags			  = OFN_ENABLEHOOK|OFN_EXPLORER|OFN_ENABLESIZING;
// 
// 	GetOpenFileName (&ofn);
// 	if(ofn.lpstrFile[0] != 0){
// 		if(filename != 0)
// 			strcpy(filename, ofn.lpstrFile);
// 	}
// }

void _printf(const char *format, ...)
{
	char buffer[256] = {' ', '-', ' '};
	va_list args;
	va_start(args, format);
	vsprintf(&buffer[3], format, args);
	printf(buffer);
	if(buffer[strlen(buffer)-1] != ':')
		printf("\n");
	va_end(args);
}

void prog_draw(u32 amount, u32 total)
{
	// progress bar
	int blocks_done = (int)(((float)amount / (float)total)*64.0f);
	int blocks_left = 64-blocks_done;
	int i;
	printf("\r   ");
	for(i = 0; i < blocks_done; i++) printf("\xdb");
	for(i = 0; i < blocks_left; i++) printf("\xb0");
}

void prog_erase()
{
	int i;
	// erase progress bar
	printf("\r");	
	for(i = 0; i < 79; i++) printf(" "); printf("\r");	
}

void fail(FT_STATUS st)
{ 
	if(st == FT_OK) return;
	printf("\n\n ***\n *** FAIL: %d\n ***\n", st);
	exit(-1);
}

void die(char *cause, char *msg)
{ 
	if(msg == 0 || cause == 0) fail(9001);
	printf("\n\n *\n * DIE: %s [in %s()]\n *\n", cause, msg);
	exit(-1);
}