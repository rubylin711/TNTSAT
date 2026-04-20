#ifndef __CONIO_H__
#define __CONIO_H__
#include "mt_hdmi20_cfg.h"

#if (__HDMI_OS_LINUX__ || __HDMI_UBOOT__)
	#include <stdio.h>
#endif

#if defined(CONFIG_MT_CHIP_ETUDE2)
	#define MT_PATH_PREFIX "/tmp/hdmi20_cfg/"        //hdmi20_cfg should be in the same folder with hdmi20 app bin
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)
	#define MT_PATH_PREFIX "/media/casetest/hdmi20_cfg/"
#else
	#define MT_PATH_PREFIX "/media/casetest/hdmi20_cfg/"
#endif

#define fopen_s( pfile, filename, mode ) {\
		char filename_tmp[100];\
		memset(filename_tmp, 0, sizeof(filename_tmp)); \
		sprintf(filename_tmp, "%s%s", MT_PATH_PREFIX, filename);\
		printf("\nmacro open file::::%s\n",filename_tmp);\
		*(pfile) = fopen(filename_tmp, mode);\
	}
#define MT_PATH_AUD_PREFIX "/media/casetest/e2audio/"        //hdmi20_cfg should be in the same folder with hdmi20 app bin
#define fopen_aud( pfile, filename, mode ) {\
		char filename_tmp[100];\
		memset(filename_tmp, 0, sizeof(filename_tmp)); \
		sprintf(filename_tmp, "%s%s", MT_PATH_AUD_PREFIX, filename);\
		printf("\nmacro open file::::%s\n",filename_tmp);\
		*(pfile) = fopen(filename_tmp, mode);\
	}
#define fprintf_s fprintf
extern char * gets( char * buffer);
#define sscanf_s(str,fmt,args...) sscanf(str,fmt,##args)
#define gets_s(a,b) {\
		fgets(a,b,stdin);\
		if (b) \
			check_input_newline2delete();\
	}

/* ++++ MTLZ Interface ++++*/
int _getch(void);
int _kbhit(void);
void debug_key_wait(char *file, char *func, int line);
void check_input_newline2delete(void);

/* ------ MTLZ Interface ------*/
#endif /* __BLACK_BOX_H__ */
