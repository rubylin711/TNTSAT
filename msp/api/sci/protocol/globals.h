/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef GLOBALS_H_
#define GLOBALS_H_
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include "mt_type.h"

#define DEBUG 1

#define CTA_RES_LEN 512

#if DEBUG
#define cs_debug_mask(l,  ...) \
    printf( __VA_ARGS__)

#define rdr_debug_mask(l, ll,  ...) \
    printf( __VA_ARGS__)

#define rdr_log(l,  ...) \
    printf(__VA_ARGS__)

#define rdr_ddump_mask(l, ll, d, s, t) \
{\
    int tmpi = 0;\
    printf("%s\n", (t));\
    for(; tmpi < (int)(s); tmpi ++)\
    {\
        if(tmpi != 0 && (tmpi % 20) == 0)\
        {\
            printf("\n");\
        }\
        printf("0x%02x ", (d)[tmpi]);\
    }\
    printf("\n");\
}
#else
#define cs_debug_mask(l,  ...)  do{ }while(0)
#define rdr_debug_mask(l, ll,  ...)  do{ }while(0)
#define rdr_log(l,  ...)  do{ }while(0)
#define rdr_ddump_mask(l, ll, d, s, t)  do{ }while(0)
#endif




#define call(arg) \
    if (arg) { \
        cs_debug_mask(D_TRACE, "ERROR, function call %s returns error.\n",#arg); \
        return ERROR; \
    }

#ifndef MAX
#define MAX(a,b) ((a) > (b) ? (a) : (b))
#endif

#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif

#define cs_sleepms(x) \
    MT_USLEEP((x) * 1000)

struct s_reader;
struct s_ATR ;
struct s_cardreader
{
//	char    *desc;
	int32_t (*reader_init)(struct s_reader *);
	int32_t (*get_status)(struct s_reader *, int *);
	int32_t (*activate)(struct s_reader *, struct s_ATR *);
	int32_t (*transmit)(struct s_reader *, unsigned char *sent, uint32_t size, uint32_t expectedlen, uint32_t delay, uint32_t timeout);
	int32_t (*receive)(struct s_reader *, unsigned char *data, uint32_t size, uint32_t delay, uint32_t timeout);
//	int32_t (*lock_init)(struct s_reader *);
//	void (*lock)(struct s_reader *);
//	void (*unlock)(struct s_reader *);
	int32_t (*close)(struct s_reader *);
//	int32_t (*set_parity)(struct s_reader *, uchar parity);
	// FIXME: All parameters passed to write_settingsX should be put in a struct
	//int32_t (*write_settings)(struct s_reader *,
	//						  uint32_t ETU,
	//						  uint32_t EGT,
	//						  unsigned char P,
	//						  unsigned char I,
	//						  uint16_t Fi,
	//						  unsigned char Di,
	//						  unsigned char Ni);
	// FIXME: write_settings2 is used by coolstream reader
	//int32_t (*write_settings2)(struct s_reader *, uint16_t F, uint8_t D, uint32_t WWT, uint32_t EGT, uint32_t BGT);
	// FIXME: write_settings3 is used by sci reader
	//int32_t (*write_settings3)(struct s_reader *, uint32_t ETU, uint32_t F, uint32_t WWT, uint32_t CWT, uint32_t BWT, uint32_t EGT, uint32_t I);
         int32_t (*write_settings)(struct s_reader *, uint32_t ETU, unsigned char N);
	//int32_t (*set_protocol)(struct s_reader *,
	//						unsigned char *params,
	//						uint32_t *length,
	//						uint32_t len_request);
//	int32_t (*set_baudrate)(struct s_reader *,
//							uint32_t baud); //set only for readers which need baudrate setting and timings need to be guarded by OSCam
	//int32_t (*card_write)(struct s_reader *pcsc_reader,
	//					  const uchar *buf,
	//					  unsigned char *cta_res,
	//					  uint16_t *cta_lr,
	//					  int32_t l);
//	void (*display_msg)(struct s_reader *, char *msg);

//	int32_t (*do_reset)(struct s_reader *, struct s_ATR *,
//						int32_t (*rdr_activate_card)(struct s_reader *, struct s_ATR *, uint16_t deprecated),
//						int32_t (*rdr_get_cardsystem)(struct s_reader *, struct s_ATR *));

	//bool (*set_DTS_RTS)(struct s_reader *, int32_t *dtr, int32_t *rts);

//	int32_t         typ;                // fixme: workaround, remove when all old code is converted

//	int8_t          max_clock_speed;    // 1 for reader->typ > R_MOUSE
//	int8_t          need_inverse;       // 0 = reader does inversing; 1 = inversing done by oscam
	//io_serial config
//	int8_t          flush;
//	int8_t          read_written;       // 1 = written bytes has to read from device
	//bool            skip_extra_atr_parsing;
	//bool            skip_t1_command_retries;
	//bool            skip_setting_ifsc;
};

struct s_reader                                     //contains device info, reader info and card info
{
         uchar          dev_id;
        	char            device[128];
 	int32_t         handle;                         // device handle
        	uchar           atr[64];
	uchar           card_atr[64];                   // ATR readed from card
         int8_t          card_atr_length;                // length of ATR
         uint32_t         mhz;                            // actual clock rate of reader in 10khz steps
	int32_t         convention;                     // Convention of this ICC
	unsigned char   protocol_type;                  // Type of protocol
//	uint32_t        current_baudrate;               // (for overclocking uncorrected) baudrate to prevent unnecessary conversions from/to termios structure
	double      worketu;            // in us for internal and external readers calculated (1/D)*(F/cardclock)*1000000
	uint32_t        read_timeout;                   // Max timeout (ETU) to receive characters
	uint32_t        char_delay;                     // Delay (ETU) after transmiting each successive char
	uint32_t    block_delay;          // Delay (ms) after starting to transmit
	uint32_t    BWT, CWT;           // (for overclocking uncorrected) block waiting time, character waiting time, in ETU
	////variables from io_serial.h
//	int32_t         written;                        // keep score of how much bytes are written to serial port, since they are echoed back they have to be read
	////variables from protocol_t1.h
	uint16_t    ifsc;             // Information field size for the ICC
	unsigned char ns;               // Send sequence number
	unsigned char use_default_etu;
//	int16_t             smartdev_found;
//	int16_t				smart_type;
//	uint16_t   statuscnt;
//	uint16_t   modemstat;
    	struct s_cardreader crdr;
};

#include "icc_async.h"
#include "cardreaders.h"
#include "atr.h"
char *cs_hexdump(int32_t m, const uchar *buf, int32_t n, char *target, int32_t len);

#endif
