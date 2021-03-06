//
// local.h
//

enum {
	DEV_ERR_OK,
	DEV_ERR_TIMEOUT_READ,
	DEV_ERR_TIMEOUT_WRITE,
	DEV_ERR_NULL_HANDLE,
	DEV_ERR_NULL_PTR,
	DEV_ERR_NULL_FILE,
	DEV_ERR_NO_DEVICES,
	DEV_ERR_CANT_FIND,
	DEV_ERR_CANT_OPEN,
	DEV_ERR_CANT_MALLOC,
	DEV_ERR_BADMAGIC,
	DEV_ERR_TIMED_OUT,
	DEV_ERR_NOGETVER,
	DEV_ERR_UNKNOWN,
};

enum {
	INFO_VERBOSE,
	INFO_FOUNDAT,
	INFO_OPENED,
	INFO_DEVMAGIC,
	INFO_QUERYFW,
	INFO_COMPLETED_TIME,
	INFO_TRUNCATED,
	INFO_CHUNK,
	INFO_OPT_CHUNK,
	INFO_TOTALDONE,
	INFO_EXTRAFILE,
	INFO_ENUMERATING,
	INFO_NOMATCHES,
	INFO_DONE,
};

extern char *err[];
extern char *info[];
extern char	*bank_desc[];
extern char	*save_desc[];