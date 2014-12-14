//
// 64drive USB loader
// (c) 2014 Retroactive
//
// This code is open-source. Feel free to improve it but know that if you screw up
// the firmware update you'll need a JTAG cable to re-configure it.
//
// You're welcome to port to other platforms. Send me a link or zip file.
// Uses FTDI D2XX drivers. http://www.ftdichip.com/Drivers/D2XX.htm
//
// http://64drive.retroactive.be
//

#include "usb.h"
#include "helper.h"
#include "device.h"
#include "image.h"
#include "upgrade.h"
#pragma comment(lib, "FTD2XX.lib")

int				mode = MODE_UNSPECIFIED;
ftdi_context_t	usb = {0, };
game_files_t	game = {0, };
upgrade_files_t	upgrade = {0, };

int main(int argc, char* argv[])
{
	parse_args(&game, &upgrade, argc, argv);
	device_find(&usb);
	device_open(&usb);
	image_load(&game, &usb);
	upgrade_load(&upgrade, &usb);
	device_close(&usb);
}

void parse_args(game_files_t *g, upgrade_files_t *u, int argc, char *argv[])
{
	int		i;
	int		t = 0;					// total number of arguments
	char	f[16];					// arg character
	static char	a[16][16][256];		// parameters for each arg
	int		n[16] = {0, };			// number of parameters for each arg

	printf("\n");
	printf(" 64drive USB Loader\n--------------------------------------------\n");
	printf(" (c) 2014 Retroactive\n");
	printf(" Compiled %s\n\n", compile_date);

	for(i = 1; i < argc; i++){
		if(argv[i][0] == '-'){
			// new argument
			f[t] = argv[i][1];
			while(1){
				if(i == argc-1) break;
				i++;
				if(argv[i][0] == '-'){
					// new argument, back out and try again
					i--;
					break;
				} else {
					// append arg value to chain
					strcpy(a[t][n[t]], argv[i]);
					n[t]++;
					if(n[t] == 16) die("Too many values for argument", __FUNCTION__);
				}
			}
			t++;
			if(t == 16) die("Too many arguments", __FUNCTION__);
		}		
	}

	for (i = 0; i < t; i++)  {
		switch(f[i]) {
		case 'l':
			if(n[i] < 1) die("Not enough arguments for parameter 'l'", __FUNCTION__);
			strcpy(g->files[g->num_files], a[i][0]);
			// assume defaults
			g->types[g->num_files] = 1;
			g->addrs[g->num_files] = 0;
			// explicit arguments (optional)
			if(n[i] > 1) sscanf(a[i][1], "%d", &g->types[g->num_files]);
			if(n[i] > 2) sscanf(a[i][2], "%x", &g->addrs[g->num_files]);
			if( g->types[g->num_files] >= BANK_LAST || 
				g->types[g->num_files] <= 0) 
				die("Specified bank not in valid range", __FUNCTION__);
			g->num_files ++;
			mode |= MODE_LOADIMAGE_BANK;
			break;

		case 'd':
			if(n[i] < 4) die("Not enough arguments for parameter 'd'", __FUNCTION__);
			strcpy(g->files[g->num_files], a[i][0]);
			sscanf(a[i][1], "%d", &g->types[g->num_files]);
			sscanf(a[i][2], "%x", &g->addrs[g->num_files]);
			sscanf(a[i][3], "%x", &g->sizes[g->num_files]);
			// check range
			if( g->types[g->num_files] >= BANK_LAST || 
				g->types[g->num_files] <= 0) 
				die("Specified bank not in valid range", __FUNCTION__);	
			
			if(g->sizes[g->num_files] < 512) die("Size too small", __FUNCTION__);
			g->num_files ++;
			g->dump = 1;
			mode |= MODE_DUMPIMAGE_BANK;
			break;
		case 's':
			if(n[i] < 1) die("Not enough arguments for parameter 's'", __FUNCTION__);
			sscanf(a[i][0], "%d", &g->save_types[g->num_files]);
			// check range
			if( g->save_types[g->num_files] >= SAVE_LAST || 
				g->save_types[g->num_files] < 0) 
				die("Specified savetype not in valid range", __FUNCTION__);	
			// offset save type index so that 0 = null, 1 = no save, etc
			g->save_types[g->num_files] ++;
			g->num_files ++;
			mode |= MODE_SAVETYPE_BANK;
			break;
// 		case 'f':
// 			if(n[i] < 1) die("Not enough arguments for parameter 'f'", __FUNCTION__);
// 			mode |= MODE_UPGRADE_FIRM;
// 			strcpy(u->files[u->num_files], a[i][0]);
// 			u->firm[u->num_files] = 1;
// 			u->num_files++;
// 			break;
// 		case 'b':
// 			if(n[i] < 1) die("Not enough arguments for parameter 'b'", __FUNCTION__);
// 			mode |= MODE_UPGRADE_BOOTLD;
// 			strcpy(u->files[u->num_files], a[i][0]);
// 			u->bootld[u->num_files] = 1;
// 			u->num_files++;
// 			break;
		case 'v':
			usb.verbose = 1;
			break;
		default:
			die("Unhandled or unknown argument", __FUNCTION__);
			break;
		}
		
	}
	// parameter was not given, complain
	if(mode == MODE_UNSPECIFIED) invalid_args();
}

void invalid_args()
{
	int i;
	_printf("Invalid parameter(s)");
	_printf("Parameters:\n");
	printf("    -l <file> [bank] [addr]\t   Load binary to bank\n");
	printf("    -d <file> <bank> <addr> <len>  Dump binary from bank\n");
	for(i = 1; i < BANK_LAST; i++){
		if((i-1) & 1) 
			printf("  \t %d - %s\n", i, bank_desc[i]);
		else 
			printf("\t\t%d - %s", i, bank_desc[i]);
	}
	printf("\n");
	printf("    -s <int>\t\t\t   Set save emulation type\n");
	for(i = 0; i < SAVE_LAST; i++){
		if(i & 1) 
			printf("  \t %d - %s\n", i, save_desc[i]);
		else 
			printf("\t\t%d - %s", i, save_desc[i]);
	}
	printf("\n");
// 	printf("    -b <file.rpk>\t\t   Update bootloader\n");
// 	printf("    -f <file.rpk>\t\t   Update firmware\n");
	printf("    -v\t\t\t\t   Verbose output for debug\n");
	exit(1);
}
