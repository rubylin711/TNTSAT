#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/time.h>
#include <pthread.h>
#include <semaphore.h>
#include "mt_unf_sci.h"
#include "ca_icc.h"



#define PCB_ICC_ALWAYS_OPEN		0
#define PCB_ICC_ALWAYS_CLOSE	1
#define PCB_ICC_TYPE 			PCB_ICC_ALWAYS_OPEN

#define ICC_T1_EXCHANGE_DEBUG	0
#define ICC_DEBUG				0
#if ICC_DEBUG
#define ICC_ERROR_PRINT			printf
#define ICC_DEBUG_PRINT			printf
#define ENTER_FUNCTION()		do{struct timeval tv; gettimeofday(&tv, NULL);printf("[%ld.%06ld]Enter %s\n",tv.tv_sec,tv.tv_usec,__FUNCTION__);}while(0)
#define LEAVE_FUNCTION()		do{struct timeval tv; gettimeofday(&tv, NULL);printf("[%ld.%06ld]Leave %s@%d\n",tv.tv_sec,tv.tv_usec,__FUNCTION__,__LINE__);}while(0)
#define DMSG(fmt, ...)  printf("[DBG]: %s:%d " fmt "\n", __FUNCTION__, __LINE__, ##__VA_ARGS__)
#else
#define ICC_ERROR_PRINT(fmt, ...)			do {} while(0);
#define ICC_DEBUG_PRINT(fmt, ...)			do {} while(0);
#define ENTER_FUNCTION(fmt, ...)			do {} while(0);
#define LEAVE_FUNCTION(fmt, ...)
#define DMSG(fmt, ...)  do {} while(0);

#endif

#define T0_CMD_LENGTH 			5
#define T0_CLA_OFFSET 			0
#define T0_INS_OFFSET 			1
#define T0_P1_OFFSET 			2
#define T0_P2_OFFSET 			3
#define T0_P3_OFFSET 			4

#define REGISTRATION_ID			0x123456
#define SESSION_ID				0x654321
#define CLOCK_FREQ_KHZ			4000
#define ICC_PORT				MT_UNF_SCI_PORT0

struct icc_info_struct
{
	int card_status;
	int protocol;
	int active;
	TIccAccessMode access_mode;
	TIccClockFrequency	 clk_freq;
	mt_u32 etu; //us
	mt_u32 WT;  //us
	mt_u32 BWT; //us
	mt_u32 CWT; //us
	mt_u32 BGT; //us
	mt_u32 BWT_etu;
	mt_u32 CWT_etu;
	mt_u32 error_value;
	TUnsignedInt8 atr[ICC_ATR_MAX_LEN];
};

typedef enum
{
	ATR_GROUP_TA,
	ATR_GROUP_TB,
	ATR_GROUP_TC,
	ATR_GROUP_TD,
	ATR_GROUP_NUM,
}ATR_GROUP_E;

typedef struct
{
	mt_u8 value;
	MT_BOOL	 exist;
}ATR_GrouInfo_S;

typedef struct
{
	ATR_GrouInfo_S groupInfo[7][ATR_GROUP_NUM];
	mt_u32 TA1;
	mt_s32 Fi;
	mt_s32 Di;
	mt_u32 FI; //add by zwu 20180803
	mt_u32 DI; //add by zwu 20180803
	mt_u32 etu; //us
	mt_u32 GT; //work ETU
	mt_u32 WT; //us
	mt_u32 PPSTiemout; //us
	mt_u8 IFSC ;
	mt_u32 BWT; //us
	mt_u32 CWT; //us
	MT_BOOL historyExist;
	mt_u8 historyByte[15];
	MT_BOOL	 TCKExist;
	mt_u32 TCK;
	mt_u32 protocolCnt; // 指定的T个数
	mt_u8 protocol[15];
	mt_u8 protocol_type;
	MT_BOOL	 T1Exist;
	MT_BOOL	 T15Exist;
	MT_BOOL is_specific_mode;
}ATRInfo_S;

static mt_s32 g_FiMap[] =
{
	372,	372,	558,	744,	1116,	1488,	1860, 	-1,
	-1,	512,	768,	1024,	1536,	2048,	-1,	-1,
};

static mt_s32 g_DiMap[] =
{
	-1,	1,	2,	4,	8,	16,	32,	64,
	12,	20,	-1,	-1,	-1,	-1,	-1,	-1,
};


TIccEventNotification	icc_event_notification_callback = NULL;
static struct icc_info_struct	icc_info;
static pthread_mutex_t	icc_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_t		icc_monitor_thread;
static int			is_icc_driver_inited = 0;

static TIccStatus smartcard_reset(TBoolean xColdReset);
mt_void* card_status_monitor(mt_void* args);


static mt_s32 parse_atr(mt_u8 *ATR, mt_u32 Len, ATRInfo_S *ATRInfo)
{
	mt_u8	*atr = ATR;
	mt_u8	AtrMask = 0x0;
	mt_u8	AtrCheck = 0,Checklen = 0,i = 0,size = 0;;
	mt_u8	pos = 1;
	double clock_mhz = (double)CLOCK_FREQ_KHZ / 1000;

	if (atr[0] != 0x3b && atr[0] != 0x3f)
	{
		ICC_ERROR_PRINT("[ATR] First byte: 0x%02X.\n", atr[0]);
		return -1;
	}

	if((Len<2) || (Len > ICC_ATR_MAX_LEN))
	{
		ICC_ERROR_PRINT("[ATR] Len = %d is error!\n", Len);
		return -1;
	}

	ICC_DEBUG_PRINT("[ATR] TS:0x%x\n", atr[0]);
	ICC_DEBUG_PRINT("[ATR] T0:0x%x\n", atr[1]);
	size = 2;
	if((atr[pos] & 0x0f) > 0)
	{
		ATRInfo->historyExist = 1;
	}

	/*interface byte*/
	AtrMask = (atr[pos] & 0xf0) >> 3;
	if ((AtrMask >> 1) & 0x01)
	{
		ATRInfo->groupInfo[0][ATR_GROUP_TA].exist = 1;
		ATRInfo->groupInfo[0][ATR_GROUP_TA].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TA1: 0x%x\n", ATRInfo->groupInfo[0][ATR_GROUP_TA].value);
		ATRInfo->TA1 = ATRInfo->groupInfo[0][ATR_GROUP_TA].value;
		ATRInfo->FI = (ATRInfo->groupInfo[ATR_GROUP_TA][0].value & 0xf0) >> 4;
		ATRInfo->DI = ATRInfo->groupInfo[ATR_GROUP_TA][0].value & 0x0f;
		ATRInfo->Fi = g_FiMap[ATRInfo->FI];
		ATRInfo->Di = g_DiMap[ATRInfo->DI];
		size++;
	}
	else
	{
		ATRInfo->FI = 1;
		ATRInfo->DI = 1;
		ATRInfo->Fi = 372;
		ATRInfo->Di = 1;
	}

	if ((AtrMask >> 2) & 0x01)
	{
		ATRInfo->groupInfo[0][ATR_GROUP_TB].exist = 1;
		ATRInfo->groupInfo[0][ATR_GROUP_TB].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TB1: 0x%x\n", ATRInfo->groupInfo[0][ATR_GROUP_TB].value);
		size++;
	}

	if ((AtrMask >> 3) & 0x01)
	{
		ATRInfo->groupInfo[0][ATR_GROUP_TC].exist = 1;
		ATRInfo->groupInfo[0][ATR_GROUP_TC].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TC1: 0x%x\n", ATRInfo->groupInfo[0][ATR_GROUP_TC].value);
		size++;
	}

	if ((AtrMask >> 4) & 0x01)
	{
		ATRInfo->groupInfo[0][ATR_GROUP_TD].exist = 1;
		ATRInfo->groupInfo[0][ATR_GROUP_TD].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TD1: 0x%x   ", ATRInfo->groupInfo[0][ATR_GROUP_TD].value);
		size++;
		if((ATRInfo->groupInfo[0][ATR_GROUP_TD].value & 0x0f) == 1)
		{
			ATRInfo->T1Exist = 1;
			ICC_DEBUG_PRINT("[ATR] This is T1 protocol !!!\n");
		}
		else if((ATRInfo->groupInfo[0][ATR_GROUP_TD].value& 0x0f)  == 0)
		{
			ICC_DEBUG_PRINT("[ATR] This is T0 protocol !!!\n");
		}

		ATRInfo->protocol_type =
		  (ATRInfo->groupInfo[0][ATR_GROUP_TD].value & 0x0f);
		AtrMask = (ATRInfo->groupInfo[0][ATR_GROUP_TD].value & 0xf0) >> 3;
	}
	else
	{
		ATRInfo->protocol_type = 0; // T0 protocol
		AtrMask = 0x0;
	}

	if ((AtrMask >> 1) & 0x01)
	{
		mt_u8 TA2;

		ATRInfo->groupInfo[1][ATR_GROUP_TA].exist = 1;
		ATRInfo->groupInfo[1][ATR_GROUP_TA].value = atr[++pos];
		ATRInfo->is_specific_mode = MT_TRUE;

		size++;
		TA2 = ATRInfo->groupInfo[1][ATR_GROUP_TA].value;
		ATRInfo->protocol_type = TA2 & 0x0F;

		/*
		 * If bit 5 is set to 0, then the integers Fi and Di
		 * defined above by TA1 shall apply
		 */
		if (!(TA2 & 0x10)) {
			if (ATRInfo->groupInfo[0][ATR_GROUP_TA].exist) {
				ATRInfo->FI = ATRInfo->TA1 >> 4;
				ATRInfo->DI = ATRInfo->TA1 & 0x0f;
				ATRInfo->Fi = g_FiMap[ATRInfo->FI];
				ATRInfo->Di = g_DiMap[ATRInfo->DI];
			} else {
				ATRInfo->FI = 1;
				ATRInfo->DI = 1;
				ATRInfo->Fi = 372;
				ATRInfo->Di = 1;
			}
		} else {
			ICC_DEBUG_PRINT("[ATR] Specific mode: speed 'implicitly defined', "
			                "not sure how to proceed, assuming default values\n");
			ATRInfo->FI = 1;
			ATRInfo->DI = 1;
			ATRInfo->Fi = 372;
			ATRInfo->Di = 1;
		}

		ICC_DEBUG_PRINT("[ATR] TA2: 0x%x   "
		                "(Specific mode: T%i, F=%d, D=%d)\n",
		                TA2, ATRInfo->protocol_type, ATRInfo->Fi, ATRInfo->Di);
	}

	/* Calculate the ETU in us */
	double etu;
	etu = ATRInfo->Fi / ATRInfo->Di / clock_mhz;
	ATRInfo->etu = (mt_u32)(etu);
	ICC_DEBUG_PRINT("[ATR] Clock = %fMHz, Fi = %d, Di = %d, ETU = %dus\n",
	                clock_mhz, ATRInfo->Fi, ATRInfo->Di, ATRInfo->etu);

	if ((AtrMask >> 2) & 0x01)
	{
		ATRInfo->groupInfo[1][ATR_GROUP_TB].exist = 1;
		ATRInfo->groupInfo[1][ATR_GROUP_TB].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TB2: 0x%x\n", ATRInfo->groupInfo[1][ATR_GROUP_TB].value);
		size++;
	}

	if ((AtrMask >> 3) & 0x01)
	{
		ATRInfo->groupInfo[1][ATR_GROUP_TC].exist = 1;
		ATRInfo->groupInfo[1][ATR_GROUP_TC].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TC2:0x%x (noly for T0 show IC card max char timeout)\n",
		                ATRInfo->groupInfo[1][ATR_GROUP_TC].value);
		size++;
	}

	if ((AtrMask >> 4) & 0x01)
	{
		ATRInfo->groupInfo[1][ATR_GROUP_TD].exist = 1;
		ATRInfo->groupInfo[1][ATR_GROUP_TD].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TD2: 0x%x\n", ATRInfo->groupInfo[1][ATR_GROUP_TD].value);
		AtrMask = (ATRInfo->groupInfo[1][ATR_GROUP_TD].value & 0xf0) >> 3;
		size++;
	}
	else
	{
		AtrMask = 0x0;
	}

	if ((AtrMask >> 1) & 0x01)
	{
		ATRInfo->groupInfo[2][ATR_GROUP_TA].exist = 1;
		ATRInfo->groupInfo[2][ATR_GROUP_TA].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TA3: 0x%x(INF length)\n", ATRInfo->groupInfo[2][ATR_GROUP_TA].value);
		size++;
	}

	if ((AtrMask >> 2) & 0x01)
	{
		ATRInfo->groupInfo[2][ATR_GROUP_TB].exist = 1;
		ATRInfo->groupInfo[2][ATR_GROUP_TB].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TB3: 0x%x(b5~b8:BWI  b1~b4:CWI)\n", ATRInfo->groupInfo[2][ATR_GROUP_TB].value);
		size++;
	}

	if ((AtrMask >> 3) & 0x01)
	{
		ATRInfo->groupInfo[2][ATR_GROUP_TC].exist = 1;
		ATRInfo->groupInfo[2][ATR_GROUP_TC].value = atr[++pos];
		ICC_DEBUG_PRINT("[ATR] TC3: 0x%x\n", ATRInfo->groupInfo[2][ATR_GROUP_TC].value);
		size++;
	}

	Checklen = (mt_u8)(pos + (atr[1] & 0x0f) + 2);
	size =(mt_u8)((atr[1] & 0x0f)+size);
#if 1
	/*TCK check*/
	if(ATRInfo->T1Exist == 1)
	{
		AtrCheck = 0;
		for(i = 1;i <Checklen;i++)
		{
			AtrCheck ^= atr[i];
		}
		if(0 != AtrCheck)
		{
			ICC_ERROR_PRINT("[ATR] Check Tck is error!\n");
			return -1;
		}
	}
#endif
	AtrCheck = atr[1] & 0x0f;	//HistoryByte len
	Checklen = 0;
	/*HistoryByte*/
	if(ATRInfo->historyExist)
	{
		if(ATRInfo->T1Exist == 1)
		{
			++pos;
			ICC_DEBUG_PRINT("[ATR] HistoryByte:");
			while(pos < size)
			{
				ICC_DEBUG_PRINT("%#x ",atr[pos]);
				pos++;
				Checklen++;
			}
			ICC_DEBUG_PRINT("\nTCK:%#x\n",atr[pos]);
		}
		else
		{
			++pos;
			ICC_DEBUG_PRINT("[ATR] HistoryByte:");
			while(pos < size+1)
			{
				ICC_DEBUG_PRINT("%#x ",atr[pos]);
				pos++;
				Checklen++;
			}
			ICC_DEBUG_PRINT("\n");
		}
	}
	if(AtrCheck != Checklen)
	{
		ICC_ERROR_PRINT("[ATR] Check Parameter is error!\n");
		return -1;
	}

	/*TC1*/
	if(ATRInfo->T1Exist == 1)
	{
		ATRInfo->GT = 11;//add by zwu 20180803
	}
	else
	{
		ATRInfo->GT = 12; //add by zwu 20180803
	}

	if(ATRInfo->groupInfo[0][ATR_GROUP_TC].exist )
	{
		if (ATRInfo->groupInfo[0][ATR_GROUP_TC].value < 0xff)
		{
			ATRInfo->GT =( ATRInfo->GT+ATRInfo->groupInfo[0][ATR_GROUP_TC].value);//add by zwu 20180803
		}
	}
	ICC_DEBUG_PRINT("[ATR] GT = %d\n",ATRInfo->GT);

	/* Calculate the WT in us */
	mt_u8 wi = 10;
	double WT = 0;

	if (ATRInfo->groupInfo[1][ATR_GROUP_TC].exist) // TC2
		wi = ATRInfo->groupInfo[1][ATR_GROUP_TC].value & 0xff;
	else
		wi = 10;

	WT = wi * 960 * ATRInfo->Fi / clock_mhz;
	ATRInfo->WT = (mt_u32)(WT);
	ICC_DEBUG_PRINT("[ATR] WI = %d, WT = %dus\n", wi, ATRInfo->WT);

	/* Calculate the PPS in us */
	double pps_timeout;

	pps_timeout = 9600 * 372 / clock_mhz;
	ATRInfo->PPSTiemout = (mt_u32)(pps_timeout);
	ATRInfo->PPSTiemout += 50000; //timeout offset
	ICC_DEBUG_PRINT("[ATR] PPS timeout = %dus\n", ATRInfo->PPSTiemout);

	/*TA3     IFSC*/
	if(ATRInfo->groupInfo[2][ATR_GROUP_TA].exist )
	{
		if(((ATRInfo->groupInfo[2][ATR_GROUP_TA].value & 0xff) > 0x0f)&&
			((ATRInfo->groupInfo[2][ATR_GROUP_TA].value & 0xff) < 0xff))
			ATRInfo->IFSC = ATRInfo->groupInfo[2][ATR_GROUP_TA].value & 0xff;
	}

	/* Calculate the CWT and the BWT in us */
	mt_u8 cwi = 0, bwi = 0;
	double bwt = 0;

	if (ATRInfo->groupInfo[2][ATR_GROUP_TB].exist) // TB3
	{
		mt_u8 TB3 = ATRInfo->groupInfo[2][ATR_GROUP_TB].value;
		cwi = TB3 & 0x0F;
		bwi = TB3 >> 4;
	}
	else
	{
		cwi = 13;
		bwi = 4;
	}

	// Set CWT = (11+(2^CWI)) * work etu
	ATRInfo->CWT = (mt_u32)((mt_u32)(12 + (1 << cwi)) * ATRInfo->etu);

	// Set BWT = (2^BWI * 960 * 372 / clockspeed in mhz) us + 11 * work etu
	bwt = 11 * ATRInfo->etu + (1 << bwi) * 960 * 372 / clock_mhz;
	ATRInfo->BWT = (mt_u32)(bwt);

	ICC_DEBUG_PRINT("[ATR] CWI = %d, BWI = %d, CWT = %dus, BWT = %dus\n",
					cwi, bwi, ATRInfo->CWT, ATRInfo->BWT);

	return 0;
}

static mt_s32 SC_T0_ProcessProcedureBytes(mt_u8	card_index,
						mt_u8	ins,
						mt_u32	write_size,
						mt_u32	read_size,
						mt_u8	*buf_p,
						mt_u8	*size_p,
						mt_u32	*next_write_size_p,
						mt_u32	*next_read_size_p,
						mt_u8	sw1sw2[2])
{
    mt_s32	error = 0;
    mt_s32	ret = 0;
    mt_s32	more_procedure_bytes = 0;
    mt_u8		p = 0, i = 0;
    mt_u32	sz = 0, next_write_size = 0;
    /* Assume no more data to be sent by IFD */
    next_write_size = 0;

    *next_read_size_p = read_size;

    i = 0;                              /* Procedure byte count */
    do                                  /* Process each procedure byte */
    {
        /* Assume no more procedure bytes to come */
        more_procedure_bytes = FALSE;

        /* Try to read an available procedure byte */
        ret = mt_unf_sci_receive(card_index, &p, 1, &sz, icc_info.WT);

        /* Check procedure byte */
        if ((0 == ret) && (1 == sz))   /* Do we have a procedure byte? */
        {
            /* Set the procedure byte in the status structure */
            buf_p[i] = p;

            if (0 == i)                 /* First procedure byte? */
            {
                /* Check for ACK byte */
                if (0x00 == (ins ^ p)) /* ACK == ins */
                {
                    if (write_size > 0)
                    {
                        /* Send all remaining bytes */
                        next_write_size = write_size;
                    }
                    else if (0 == read_size)
                    {
                        /* Await further procedure bytes */
                        more_procedure_bytes = TRUE;
                        i = 0;
                        continue;
                    }
                }
                else if (0xFF == (ins ^ p)) /* ACK == ~ins */
                {
                    if (write_size > 0)
                    {
                        /* Send the next byte only */
                        next_write_size = 1;
                    }
                    else if (0 == read_size)
                    {
                        /* No more bytes available - await further bytes */
                        more_procedure_bytes = TRUE;
                        i = 0;
                        continue;
                    }
                    else
                    {
                        *next_read_size_p = 1;
                    }
                }
                else if (0x01 == (ins ^ p)) /* ACK == ins+1 */
                {
                    if (write_size > 0)
                    {
                        /* Send all remaining bytes */
                        next_write_size = write_size;
                    }
                    else if (0 == read_size)
                    {
                        /* No more bytes available - await further bytes */
                        more_procedure_bytes = TRUE;
                        i = 0;
                        continue;
                    }
                }
                else if (0xFE == (ins ^ p)) /* ACK == ~ins+1 */
                {
                    if (write_size > 0)
                    {
                        /* Send the next byte only */
                        next_write_size = 1;
                    }
                    else if (0 == read_size)
                    {
                        /* No more bytes available - await further bytes */
                        more_procedure_bytes = TRUE;
                        i = 0;
                        continue;
                    }
                    else
                    {
                        *next_read_size_p = 1;
                    }
                }
                else if (0x60 == (p & 0xF0)) /* SW1 or NULL */
                {
                    more_procedure_bytes = TRUE;/* nned not handle it*/
                    sw1sw2[0]=p;
                    if (p != 0x60)
                    {
                        *next_read_size_p = 0;
                    }

                    switch (p)
                    {
                        case 0x60:      /* NULL - reset work waiting time */
                            i = 0;      /* New sequence of bytes to come */
                            continue;
                        case 0x61://add by yangzc, 20051028
                            break;
                        case 0x6E:      /* SW1 byte */
                            error = 0x10;
                            break;
                        case 0x6D:      /* SW1 byte */
                            error = 0x11;
                            break;
                        case 0x68:      /* SW1 byte */
                            error = 0x12;
                            break;
                        case 0x67:      /* SW1 byte */
                            error = 0x13;
                            break;

                        case 0x6A://-错误的参数P1～P2, 在SW2中进一步的限定
                            break;
                        case 0x6B://-错误的参数P1～P2
                            break;

                        default:
                            case 0x6F:      /* SW1 byte */
                                error = 0x14;
                                break;
                    }
                }
                else if (0x90 == (p & 0xF0)) /* SW1 byte */
                {
                    /* Next procedure byte is the final one */
                    more_procedure_bytes = TRUE;
                    sw1sw2[0]=p;
                    *next_read_size_p = 0;

                    /* 0x90 00 is okay, anything else we assume is an error
                    */
                }
                else
                {
                    /* Unrecognised status byte */
                    ICC_ERROR_PRINT("[SC] Invalid ACK, ACK=0x%x , ins=0x%x! \n ", p, ins);
                    error = 0x15;
                }
            }
            else
            {
                sw1sw2[1]=p;
                /* This was SW2, so we should set the error status now */
            }
        }
        else
        {
	/* No answer from the card */
	ICC_ERROR_PRINT("[SC] recieve procedure byte failed, ret=0x%x! len = %d.\n ", ret, sz);
	ret = 0x16;
	if (more_procedure_bytes == FALSE)
		ret = 0x17;
            break;
        }
        i++;                        /* Next procedure byte */
    }while (more_procedure_bytes);

    /* Set number of procedure bytes received */
    *size_p = i;

    /* Set number of bytes to write in next IO transfer */
    *next_write_size_p = next_write_size;

    /* Make sure that any errors from any functions we call
    * get passed back up.
    */
   // if (0 == error) guoyb disable for bug 31228返回值为0， 交给ca库对sw1sw2做处理
    {
        error = ret;
    }

    /* Return error code */
    return error;
} /* CSSMCL_T0_ProcessProcedureBytes() */

static mt_s32 SC_T0_Receive (mt_u8 	u8SCIPortNo,
				mt_u8	*pSCIReceiveBuf,
				mt_u32	u32ReceiveLength,
				mt_u32	*pu32ActualLength)
{
    mt_s32	s32Return = -1;
    mt_u8		*pu8Buf;
    mt_u32	u32Actual = 0, u32BufLength = 0;
    mt_u32	index = 0;

    pu8Buf = pSCIReceiveBuf;
    u32BufLength = u32ReceiveLength;

    memset(pSCIReceiveBuf, 0, u32ReceiveLength);
    *pu32ActualLength = 0;

    for(index = 0; index < u32BufLength; index++)
    {
	s32Return = mt_unf_sci_receive(u8SCIPortNo, (mt_u8*)(pu8Buf+index), 1, &u32Actual, icc_info.WT);
    	if((0 != s32Return) || u32Actual != 1)
    	{
    	   	ICC_ERROR_PRINT("[SC] Receive WT timeout\n");
    	   	/*四达需要根据这个标识进行冷复位*/
    	   	 //s32_ret = HI_UNF_SCI_ResetCard(g_u8_sci_port, HI_FALSE);
        		//g_stResetInfo.bisNeedColdResetCard = 1;
           	 	return -1;
    	}
	//if((index % 10) == 9)
	//{
	   // usleep(100);
	//}
    }

    *pu32ActualLength = index;

    //spi_smartcard_perror( "sci receive error, returned:0x%x!\n", s32Return);
    if (*pu32ActualLength > 256)
    {
        ICC_ERROR_PRINT( "[SC] sci receive over flow!\n");
        return -1;
    }

    return 0;
}

static mt_s32 SC_T0_TransferData(mt_u8	card_index,
				    mt_u8		*write_buffer,
				    mt_u32	number_to_write,
				    mt_u8		*read_buffer,
				    mt_u32	*actual_read_length,
				    mt_u8		sw1sw2[2])
{
    mt_u32	number_written = 0, next_write_size = 0, next_read_size = 0 ;
    mt_u8		ins,pb[2],pb_size;
    mt_u32	number_to_read = 0;
    MT_BOOL	no_exchange=FALSE;
    mt_s32	ret = 0;

    if(number_to_write < T0_CMD_LENGTH)
    {
        number_to_write = 5;
        no_exchange = TRUE;
    }
    else
    {
        no_exchange = FALSE;
    }

    /* Calculate number of bytes expected to be received */
    if (T0_CMD_LENGTH == number_to_write)
    {
        if(no_exchange)
        {
            number_to_read = 0;
        }
        else
        {
            /* Extract number to read from P3 */
            //number_to_read = (write_buffer[T0_P3_OFFSET] == 0)? 256 :write_buffer[T0_P3_OFFSET];//WriteBuffer_p[T0_P3_OFFSET];
            number_to_read = write_buffer[T0_P3_OFFSET];
        }
    }
    else
    {
        /* Data is being sent, so no data response will be permitted */
        number_to_read = 0;
    }


    /* Flush FIFOs first */
    ins = write_buffer[T0_INS_OFFSET];

    /* **************************************
    *Initially we want to write the whole command; this then gets set by
    * the procedure bytes after the first write.
    *****************************************/
    next_write_size = T0_CMD_LENGTH;
    //	spi_smartcard_pinfo("1111111 hebh test write_buffer[%02x %02x %02x %02x %02x].\n", write_buffer[0],write_buffer[1],write_buffer[2],write_buffer[3],write_buffer[4],write_buffer[5]);

    /* Transmit data under the control of procedure bytes */
    while (number_to_write > 0 && 0 == ret && next_write_size > 0)//guoyb add next_write_size > 0 for bug 31228 当next_write_size=0时不发送消息
    {
        /* Transmit command */
        //		spi_smartcard_pinfo("222222222 hebh test error next_write_size =0x%x, number_to_write=0x%x! \n ", next_write_size, number_to_write);
		//SC_DumpData(write_buffer, next_write_size);

        ret = mt_unf_sci_send(card_index, (mt_u8*)(write_buffer), next_write_size, &number_written, icc_info.WT);
        if (0 != ret )
        {
            ICC_ERROR_PRINT("[SC] Send failed, ret = %d, next_write_size=%d! \n ", ret, next_write_size);
            return ret;
        }

        /* Update pointers */
        write_buffer += number_written;
        number_to_write -= number_written;

        if(0 == ret)
        {
            ret = SC_T0_ProcessProcedureBytes(card_index,
                                                ins,
                                                (mt_u32)number_to_write,
                                                number_to_read,
                                                (mt_u8 *)pb,
                                                (mt_u8 *)&pb_size,
                                                (mt_u32 *)&next_write_size,
                                                (mt_u32 *)&next_read_size,
                                                sw1sw2);
            if (0 != ret )
            {
                ICC_ERROR_PRINT("[SC] Process procedure bytes failed, ret %d!\n ", ret);
                return ret;
            }
        }
    }

    if (0 == ret && number_to_read > 0 && next_read_size > 0)
    {
        /* Read data response */
        *actual_read_length = 0;
        ret = SC_T0_Receive(card_index,read_buffer, number_to_read, actual_read_length);
        if(0 == ret)
        {
            ret = SC_T0_ProcessProcedureBytes(card_index,
                                                        ins,
                                                        0,
                                                        0,
                                                        (mt_u8 *)pb,
                                                        (mt_u8 *)&pb_size,
                                                        (mt_u32 *)&next_write_size,
                                                        (mt_u32 *)&next_read_size,
                                                        sw1sw2);
            if (0 != ret )
            {
                ICC_ERROR_PRINT("[SC] Process procedure bytes failed, ret %d!\n", ret);
                return ret;
            }
        }
    }
    else   /*hebh add here for number_to_read=0 */
    {
        *actual_read_length = 0;    /*Read data response */
    }

    return ret;
}
/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPE                             */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    This function allows the CA to register a notification callback function
 *    in order to be informed that a smartcard is inserted to or removed
 *    from the reader.
 *
 *    This notification must be called as soon as the state of the smartcard
 *    changes. After the registration the smartcard driver must asynchronously
 *    call the notification function in order to inform the CA of the initial
 *    state of the card.
 *    If no smartcard is available at registration time, the application is
 *    notified as soon as it is registered with a dedicated notification.
 *    The registration is performed at the start of the CA. As the CA is able
 *    to start and terminate at will, the registration call may happen at
 *    any time.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param    xIccEventNotification
 *             Notification callback function.
 *  @param    pxRegistrationId
 *             Identifier for this specific registration.
 *             It is required to cancel the registration.
 *
 *  @retval   ICC_NO_ERROR
 *             Notification callback has been successfully registred.
 *  @retval   ICC_ERROR
 *             A general error occured. The notification callback could not
 *             be registred.
 *
 *  @remarks
 *    -# This function shall perform an activation of the electrical circuits
 *       according to ISO/IEC 7816-3.
*/
static int start_card_status_monitor = 0;
mt_void* card_status_monitor(mt_void* args)
{
	mt_s32			ret = 0;
	MT_UNF_SCI_STATUS_E	status  = MT_UNF_SCI_STATUS_NOCARD;
	TIccStatus		st = ICC_NO_ERROR;

	ICC_DEBUG_PRINT("card_status_monitor runs.\n");
	while (1)
	{
		pthread_mutex_lock(&icc_mutex);

		if (start_card_status_monitor == 0) {
			pthread_mutex_unlock(&icc_mutex);
			usleep(500 * 1000);
			continue;
		}
		ret = mt_unf_sci_getcardstatus(ICC_PORT, &status);
		if (MT_SUCCESS != ret) {
			ICC_ERROR_PRINT("Error: card_status_monitor ===> mt_unf_sci_getcardstatus\n");
		}
		if(status != (mt_u32)(icc_info.card_status))
		{
			icc_info.card_status = status;
			//printf("card_status_monitor   ------------------------ status = %d  \n", icc_info.card_status);
			if (MT_UNF_SCI_STATUS_READY == status)
			{
				ICC_DEBUG_PRINT("SmartCard Inserted\n");
				st = smartcard_reset(TRUE);
				if (ICC_ERROR_CARD_MUTE == st) {
					icc_info.active = 0;
					memset(icc_info.atr, 0, sizeof(TUnsignedInt8) * ICC_ATR_MAX_LEN);
					 (*icc_event_notification_callback)(
								ICC_EVENT_CARD_MUTE,
								SESSION_ID,
								icc_info.atr,
								icc_info.clk_freq
								);
				} else {
					icc_info.active = 1;
					 (*icc_event_notification_callback)(
								ICC_EVENT_CARD_INSERTED_SINGLE_CLIENT_SUPPORT,
								SESSION_ID,
								icc_info.atr,
								icc_info.clk_freq
								);
				}

 			} else {
				ICC_DEBUG_PRINT("SmartCard Removed\n");
				if (icc_event_notification_callback != NULL) {
					icc_info.active = 0;
					memset(icc_info.atr, 0, sizeof(TUnsignedInt8) * ICC_ATR_MAX_LEN);
					mt_unf_sci_deactivecard(ICC_PORT);
					(*icc_event_notification_callback)(
								ICC_EVENT_CARD_REMOVED,
								SESSION_ID,
								icc_info.atr,
								icc_info.clk_freq
								);
				}
			}
		}

		pthread_mutex_unlock(&icc_mutex);
		usleep(500 * 1000);
	}
	return (mt_void*)NULL;
}

int naIccCardInserted( void )
{
	if( icc_info.card_status == MT_UNF_SCI_STATUS_READY )
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

TIccStatus iccRegister
(
  TIccEventNotification   xIccEventNotification,
  TIccRegistrationId*     pxRegistrationId
)
{
	mt_s32 			ret = 0;
	MT_UNF_SCI_STATUS_E	card_status = MT_UNF_SCI_STATUS_NOCARD;

	printf("iccRegister ------------------------\n");

	if (!xIccEventNotification) {
		ICC_ERROR_PRINT("Error: iccRegister ===> "
		  "Register the ICC callback with a null Event Notification Pointer.\n");
		return ICC_ERROR;
	}

	if (!pxRegistrationId) {
		ICC_ERROR_PRINT("Error: iccRegister ===> "
		  "Register the ICC callback with a null Registration ID Pointer.\n");
		return ICC_ERROR;
	}
	pthread_mutex_lock(&icc_mutex);

	if (is_icc_driver_inited == 0) {
		icc_info.active = 0;
		icc_info.card_status = MT_UNF_SCI_STATUS_NOCARD;
		icc_info.clk_freq =CLOCK_FREQ_KHZ * 1000;
		icc_info.protocol = 0;
		icc_info.access_mode = ICC_ACCESS_EXCLUSIVE;
		memset(icc_info.atr, 0, sizeof(TUnsignedInt8) * ICC_ATR_MAX_LEN);
		ret = mt_unf_sci_init();
		if (MT_SUCCESS != ret) {
			pthread_mutex_unlock(&icc_mutex);

			ICC_ERROR_PRINT("Error: iccRegister ===> mt_unf_sci_init\n");
			return ICC_ERROR;
		}
		ret = mt_unf_sci_open(ICC_PORT, MT_UNF_SCI_PROTOCOL_T1, CLOCK_FREQ_KHZ);
		if (MT_SUCCESS != ret) {
			pthread_mutex_unlock(&icc_mutex);

			ICC_ERROR_PRINT("Error: iccRegister ===> mt_unf_sci_open\n");
			return ICC_ERROR;
		}
		//ret = mt_unf_sci_configclkmode(ICC_PORT, MT_UNF_SCI_MODE_OD);
		//if (MT_SUCCESS != ret) {
		//	pthread_mutex_unlock(&icc_mutex);
		//	ICC_ERROR_PRINT("Error: iccRegister ===> mt_unf_sci_configvccen\n");
		//	return ICC_ERROR;
		//}
		ret = mt_unf_sci_configvccen(ICC_PORT, MT_UNF_SCI_LEVEL_HIGH);
		if (MT_SUCCESS != ret) {
			pthread_mutex_unlock(&icc_mutex);

			ICC_ERROR_PRINT("Error: iccRegister ===> mt_unf_sci_configvccen\n");
			return ICC_ERROR;
		}
		#if (PCB_ICC_TYPE == PCB_ICC_ALWAYS_CLOSE)
		mt_unf_sci_configdetect(ICC_PORT, MT_UNF_SCI_LEVEL_HIGH);
		#else
		mt_unf_sci_configdetect(ICC_PORT, MT_UNF_SCI_LEVEL_LOW);
		#endif
		if (MT_SUCCESS != ret) {
			pthread_mutex_unlock(&icc_mutex);

			ICC_ERROR_PRINT("Error: iccRegister ===> mt_unf_sci_configdetect\n");
			return ICC_ERROR;
		}
		ret = mt_unf_sci_configtype(ICC_PORT,MT_UNF_SCI_INVERSE_CARD);
		if (MT_SUCCESS != ret) {
			pthread_mutex_unlock(&icc_mutex);
			ICC_ERROR_PRINT("Error: iccRegister ===> mt_unf_sci_configtype\n");
			return ICC_ERROR;
		}
		printf("card_status_monitor init ------------------------------------------------------------\n");
		ret = pthread_create(&icc_monitor_thread, NULL, card_status_monitor, NULL);
		if(0 != ret) {
			pthread_mutex_unlock(&icc_mutex);

			ICC_ERROR_PRINT("Error: iccRegister ===> pthread_create\n");
			return ICC_ERROR;

		}
		is_icc_driver_inited = 1;
	}
	icc_event_notification_callback = xIccEventNotification;
	*pxRegistrationId = REGISTRATION_ID;

	ret = mt_unf_sci_getcardstatus(ICC_PORT, &card_status);
	if (MT_SUCCESS != ret) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccRegister ===> mt_unf_sci_getcardstatus\n");
		return ICC_ERROR;
	}

	if (MT_UNF_SCI_STATUS_NOCARD == card_status) {
		icc_info.card_status = MT_UNF_SCI_STATUS_NOCARD;
		icc_info.active = 0;
		 (*icc_event_notification_callback)(
					ICC_EVENT_NO_AVAILABLE_CARD,
					ICC_SESSION_ID_NONE,
					icc_info.atr,
					icc_info.clk_freq
					);

	}
	start_card_status_monitor = 1;

	pthread_mutex_unlock(&icc_mutex);
	return ICC_NO_ERROR;
}


/**
 *  @brief
 *    This function allows the CA to cancel its registration in order to inform
 *    the driver that it will not use the smartcard driver any more.
 *
 *
 *    The registration cancellation is performed just before the CA
 *    termination.  As the CA is able to start and terminate at will, this
 *    function call may happen at any time.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xRegistrationId
 *              Identifier of the registration to cancel.
 *              It is provided by the registration function.
 *
 *  @retval  ICC_NO_ERROR
 *              Registration has been successfully canceled.
 *  @retval  ICC_ERROR
 *              A general error occured.
 *
 *  @remarks
 *    -# This function shall perform a deactivation of the electrical circuits
 *       according to ISO/IEC 7816-3.
*/
TIccStatus iccCancelRegistration
(
  TIccRegistrationId    xRegistrationId
)
{
	mt_s32	ret = 0;

	if (xRegistrationId != REGISTRATION_ID) {
		ICC_ERROR_PRINT("Error: iccCancelRegistration ===> Unregistered ID\n");
		return ICC_ERROR;
	}

	DMSG("\n");

	pthread_mutex_lock(&icc_mutex);

	start_card_status_monitor = 0;
	icc_event_notification_callback = NULL;
	ICC_DEBUG_PRINT("[iccCancelRegistration] deactivecard !\n");
	ret = mt_unf_sci_deactivecard(ICC_PORT);
	if (MT_SUCCESS != ret) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error:iccCancelRegistration ===> mt_unf_sci_deactivecard\n");
		return ICC_ERROR;
	}
	icc_info.active = 0;
	icc_info.card_status = MT_UNF_SCI_STATUS_NOCARD;
	icc_info.clk_freq =CLOCK_FREQ_KHZ * 1000;
	icc_info.protocol = 0;
	icc_info.access_mode = ICC_ACCESS_EXCLUSIVE;
	memset(icc_info.atr, 0, sizeof(TUnsignedInt8) * ICC_ATR_MAX_LEN);

	pthread_mutex_unlock(&icc_mutex);
	return ICC_NO_ERROR;
}

#if ICC_T1_EXCHANGE_DEBUG
static void data_debug_dump(const unsigned char *data, unsigned int len)
{
	unsigned int i = 0, j = 0;

	for (i=0; i<len; i+=j) {
		for (j=0; ((j<16) && (i+j<len)); j++) {
			printf("0x%02x ", data[i + j]);
		}

		printf("\n");
	}
}
#endif

/**
 *  @brief
 *    This function is responsible of sending a T=1 data block to the smartcard
 *    and retrieving the related reply data block from the smartcard.
 *
 *    The function stops receiving bytes when the xReplyMaxLen value is reached
 *    or when no character has been received for more than CharacterWaitingTime.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface session, as given in the
 *             event notification callback.
 *  @param   xSendLen
 *             Length of send sequence.
 *  @param   pxSendBlock
 *             Pointer to the buffer containing the send sequence to be sent
 *             to the smartcard or NULL if only receive transfer.
 *  @param   xReplyMaxLen
 *             Maximum length of the reply sequence or 0 if only send
 *             transfer.
 *  @param   pxReplyLen
 *             Length of the reply block.
 *  @param   pxReplyBlock
 *             Pointer to the buffer where the smartcard reply block will
 *             be stored.
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session ID doesn't exist.
 *  @retval   ICC_ERROR_CARD_REMOVED
 *              The smartcard in not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *  @retval  ICC_ERROR_TIMEOUT
 *             The data exchange stopped due to excessive time between
 *             successive characters.
 *  @retval  ICC_ERROR
 *             The data exchange has failed due to communication errors.
 *
 *  @remarks
 *    -# This function is synchronous. Data pointed to by pxReplyBlock and
 *       pxReplyLen must be valid as soon as the CA returns from this function.
 *    -# This function shall return the error status ICC_ERROR_CARD_MUTE if no
 *       character is returned by the smart card within the block waiting time
 *       after sending the last byte of the command to the smart card.
 *    -# This function shall stop receiving bytes from the smart card when the
 *       xReplyMaxLen value is reached or if a byte is not received within the
 *       character waiting time. In this latter case, the function shall return
 *       the error status ICC_ERROR_TIMEOUT.
*/
TIccStatus iccT1RawExchange
(
        TIccSessionId     xSessionId,
        TSize             xSendLen,
  const TUnsignedInt8*    pxSendBlock,
        TSize             xReplyMaxLen,
        TSize*            pxReplyLen,
        TUnsignedInt8*    pxReplyBlock
)
{
	TIccStatus status = ICC_NO_ERROR;
	mt_s32 ret = 0;
	mt_u32 waiting_time = 0;
	mt_u32 transfer_data = 0;
	mt_u32 i = 0;
	mt_u32 waittimes = 0;

	DMSG("\n");


	if (xSessionId != SESSION_ID) {
		ICC_ERROR_PRINT("Error: iccT1RawExchange ===> xSessionId != SESSION_ID\n");
		return ICC_ERROR_SESSION_ID;
	}

	if (!xSendLen) {
		ICC_ERROR_PRINT("Error: iccT1RawExchange ===> "
		  "The sending data length is zero.\n");
		return ICC_ERROR;
	}

	if (xReplyMaxLen && (!pxReplyBlock || !pxReplyLen)) {
		ICC_ERROR_PRINT("Error: iccT1RawExchange ===> "
		  "This transfer expert to get %d bytes replying data. "
		  "The receiving buffer = %p and the repling length pointer = %p.\n",
		  xReplyMaxLen, pxReplyBlock, pxReplyLen);
		return ICC_ERROR;
	}

	pthread_mutex_lock(&icc_mutex);

	//printf(" card_status = %d \n", icc_info.card_status);
	if (icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT1RawExchange ===> "
		  "icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD\n");
		return ICC_ERROR_CARD_REMOVED;
	}

	if (icc_info.active == 0) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT1RawExchange ===> icc_info.active == 0\n");
		return ICC_ERROR_CARD_MUTE;
	}

	if (!pxSendBlock)
		goto receive;
#if ICC_T1_EXCHANGE_DEBUG
	ICC_DEBUG_PRINT("Debug: CWT = %d BWT = %d iccT1RawExchange ===> Send %d bytes to smartcard:\n",
	                icc_info.CWT,icc_info.BWT,xSendLen);
	data_debug_dump((unsigned char *)pxSendBlock, xSendLen);
#endif

	waittimes = (icc_info.BWT+(xSendLen * icc_info.CWT));


	ret = mt_unf_sci_send(ICC_PORT, (mt_u8 *)pxSendBlock, xSendLen,
	                      &transfer_data, icc_info.CWT * xSendLen);
	if (ret || (transfer_data != xSendLen)) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT1RawExchange ===> mt_unf_sci_send failed. "
		  "Try to send %d bytes, send successfully %d bytes. "
		  "Return value = %d.\n", xSendLen, transfer_data, ret);
		return ICC_ERROR;
	}

receive:
	if(pxSendBlock ==NULL){
	 	ICC_ERROR_PRINT("pxSendBlock ==NULL\n");
		pthread_mutex_unlock(&icc_mutex);
		return ICC_ERROR;
	}
	if (!xReplyMaxLen) {
		pthread_mutex_unlock(&icc_mutex);
		return ICC_NO_ERROR;
	}
	if(icc_info.error_value == 1)
	{
		mt_unf_sci_setblktimeout(ICC_PORT,icc_info.BWT_etu+(xSendLen+23));
	}
	else
	{
		mt_unf_sci_setblktimeout(ICC_PORT,icc_info.BWT_etu);
	}

	for (i=0, waiting_time=waittimes;
	     i<xReplyMaxLen;
		 i++, waiting_time=icc_info.CWT+(3*icc_info.etu)) {
		ret = mt_unf_sci_receive(ICC_PORT, (mt_u8 *)pxReplyBlock + i, 1,
								 &transfer_data, waiting_time);
		if (ret == MT_FAILURE) {
			/* Receive timeout */
			break;
		}

		if (ret) {
			pthread_mutex_unlock(&icc_mutex);

			ICC_ERROR_PRINT("Error: iccT1RawExchange ===> "
			  "mt_unf_sci_receive failed, return value = %d.\n", ret);
			return ICC_ERROR;
		}
	}
	mt_unf_sci_setblktimeout(ICC_PORT,0);
	*pxReplyLen = i;

	if (!*pxReplyLen) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT1RawExchange ===> "
		  "mt_unf_sci_receive can't receive any data.\n");
		if(!pxSendBlock)
		{
			return ICC_ERROR;
		}
		else
		{
			return ICC_ERROR_CARD_MUTE;
		}
	}

	if (*pxReplyLen != xReplyMaxLen) {
		ICC_ERROR_PRINT("Error: iccT1RawExchange ===> "
		  "mt_unf_sci_receive timeout, expect to receive %d bytes "
		  "but actually receive %d bytes.\n", xReplyMaxLen, *pxReplyLen);
		status = ICC_ERROR_TIMEOUT;
	}
	usleep(icc_info.BGT);
#if ICC_T1_EXCHANGE_DEBUG
	ICC_DEBUG_PRINT("Debug: iccT1RawExchange ===> "
	  "Receive %d bytes data from smartcard:\n", *pxReplyLen);
	data_debug_dump((unsigned char *)pxReplyBlock, *pxReplyLen);
#endif

	pthread_mutex_unlock(&icc_mutex);

	//printf("[BobbyLinDBG][%s:%d]status=%x\n", __FUNCTION__, __LINE__, status);
	return status;
}


/**
 *  @brief
 *    This function is responsible of handling a T=0 smart card incoming
 *    command. When processing such a command, the ICC driver is in charge
 *    of handling T=0 procedure bytes.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface session, as given in
 *             the event notification callback.
 *  @param   pxHeader
 *             5-byte T=0 command header (CLA, INS, P1, P2, P3) to be sent to
 *             the smart card.
 *  @param   xDataLen
 *             Length in bytes of the data to be sent to the smart card.
 *  @param   pxData
 *             Buffer containg the data to be sent to the smart card.
 *  @param   pxStatusWords
 *             2-byte array containing the two statuses SW1 and SW2 returned
 *             by the smart card upon completion of a T=0 command. SW1
 *             corresponds to *pxStatusWords[0] and SW2 to *pxStatusWords[1].
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session ID doesn't exist.
 *  @retval   ICC_ERROR_CARD_REMOVED
 *              The smartcard in not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *  @retval  ICC_ERROR_TIMEOUT
 *             The data exchange stopped due to excessive time between
 *             successive characters.
 *  @retval  ICC_ERROR
 *             Any other error.
 *
 *  @remarks
 *    -# This function is synchronous. If the data exchange is successful,
 *       data pointed to by pxStatusWords must be valid as soon as the
 *       function returns to the caller.
 *    -# This function shall return the status ICC_ERROR_CARD_MUTE if no
 *       character is received from the smart card within the work waiting
 *       time after sending the last byte of the header.
 *    -# The function shall stop receiving bytes from the smart card as soon
 *       as it has received the two statuses SW1 and SW2 or if a byte is not
 *       received within the work waiting time. In this latter case, the
 *       function shall return the error status ICC_ERROR_TIMEOUT.
 *    -# After receiving the command header, the smart card may directly return
 *       SW1 and SW2 to report an error (SW1 is considered as a procedure byte
 *       and replaces the ACK byte).
*/
static mt_u8 t0_send_buffer[300];

TIccStatus iccT0Send
(
        TIccSessionId      xSessionId,
  const TIccT0Header      pxHeader,
        TSize              xDataLen,
  const TUnsignedInt8*    pxData,
        TIccT0StatusWords pxStatusWords
)
{
	mt_s32 ret = 0;
	mt_u32 rl = 0;
	DMSG("\n");

	if (xSessionId != SESSION_ID) {
		ICC_ERROR_PRINT("Error: iccT0Send ===> xSessionId != SESSION_ID\n");
		return ICC_ERROR_SESSION_ID;
	}

	pthread_mutex_lock(&icc_mutex);

	if (icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT0Send ===> icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD\n");
		return ICC_ERROR_CARD_REMOVED;
	}

	if (icc_info.active == 0) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT0Receive ===> icc_info.active == 0\n");
		return ICC_ERROR;
	}

	memcpy(t0_send_buffer, pxHeader, T0_CMD_LENGTH);
	memcpy(&t0_send_buffer[T0_CMD_LENGTH], pxData, xDataLen);
	ret = SC_T0_TransferData(ICC_PORT, t0_send_buffer, T0_CMD_LENGTH + xDataLen,
				NULL, &rl, pxStatusWords);
	if (ret == 0x17) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT0Send ===> ICC_ERROR_CARD_MUTE\n");
		return ICC_ERROR_CARD_MUTE;
	}

	if (ret != 0) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT0Send ===> ICC_ERROR_TIMEOUT\n");
		return ICC_ERROR_TIMEOUT;
	}

	pthread_mutex_unlock(&icc_mutex);
	return ICC_NO_ERROR;

}



/**
 *  @brief
 *    This function is responsible of handling a T=0 smart card outgoing
 *    command. When processing such a command, the ICC driver is in charge
 *    of handling T=0 procedure bytes.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface to use.
 *  @param   pxHeader
 *             5-byte T=0 command header (CLA, INS, P1, P2, P3) to be sent to
 *             the smart card.
 *  @param   xDataExpectedLen
 *             Expected number of data bytes that should be returned by the
 *             smart card (SW1 and SW2 not included).
 *  @param   pxDataLen
 *             Number of data bytes actually returned by the smart card.
 *  @param   pxData
 *             Buffer containing the data bytes returned by the smart card.
 *  @param   pxStatusWords
 *             2-byte array containing the two statuses SW1 and SW2 returned
 *             by the smart card upon completion of a T=0 command. SW1
 *             corresponds to *pxStatusWords[0] and SW2 to *pxStatusWords[1].
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_CARD_REMOVED
 *              The smartcard in not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *  @retval  ICC_ERROR_TIMEOUT
 *             The data exchange stopped due to excessive time between
 *             successive characters.
 *  @retval  ICC_ERROR
 *             Any other error.
 *
 *  @remarks
 *    -# This function is synchronous. If the data exchange is successful,
 *       data pointed to by pxDataLen, pxData and pxStatusWords must be valid
 *       as soon as the function returns to the caller.
 *    -# This function shall return the status ICC_ERROR_CARD_MUTE if no
 *       character is received from the smart card for more than the work
 *       waiting time after sending the last byte of the header.
 *    -# The function shall stop receiving bytes from the smart card as soon as
 *       it has received xDataExpectedLen+2 bytes (procedure byte not included)
 *       or if a byte is not received within the work waiting time. In this
 *       latter case, the function shall return the error status
 *       ICC_ERROR_TIMEOUT and data pointed to by pxDataLen and pxData shall be
 *       valid and correct.
 *    -# After receiving the command header, the smart card may directly return
 *       SW1 and SW2 to report an error (SW1 is considered as a procedure byte
 *       and replaces the ACK byte) without sending back any data bytes.
 *       Although in that case the ICC driver does not receive the number of
 *       expected data bytes, it shall not return the error status
 *       ICC_ERROR_TIMEOUT and *pxDataLen shall be equal to 0.
*/
TIccStatus iccT0Receive
(
        TIccSessionId      xSessionId,
  const TIccT0Header      pxHeader,
        TSize              xDataExpectedLen,
        TSize*            pxDataLen,
        TUnsignedInt8*    pxData,
        TIccT0StatusWords pxStatusWords
)
{
	mt_s32 ret = 0;
	DMSG("\n");

	if (xSessionId != SESSION_ID) {
		ICC_ERROR_PRINT("Error: iccT0Receive ===> xSessionId != SESSION_ID\n");
		return ICC_ERROR_SESSION_ID;
	}

	pthread_mutex_lock(&icc_mutex);

	if (icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT0Receive ===> icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD\n");
		return ICC_ERROR_CARD_REMOVED;
	}

	if (icc_info.active == 0) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT0Receive ===> icc_info.active == 0\n");
		return ICC_ERROR;
	}

	if (xDataExpectedLen != pxHeader[T0_P3_OFFSET]  || pxHeader[T0_P3_OFFSET] == 0) {
		ICC_DEBUG_PRINT("Warning: xDataExpectedLen=%d, T0_P3_OFFSET=%d",
					xDataExpectedLen, pxHeader[T0_P3_OFFSET]);
	}

	ret = SC_T0_TransferData(ICC_PORT, (mt_u8 *)pxHeader, T0_CMD_LENGTH,
				pxData, (mt_u32	*)pxDataLen, pxStatusWords);
	if (ret == 0x17) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT0Receive ===> ICC_ERROR_CARD_MUTE\n");
		return ICC_ERROR_CARD_MUTE;
	}

	if (ret != 0) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT0Receive ===> ICC_ERROR_TIMEOUT\n");
		return ICC_ERROR_TIMEOUT;
	}

	pthread_mutex_unlock(&icc_mutex);
	return ICC_NO_ERROR;
}


/**
 *  @brief
 *    This function is responsible of sending a T=0 data block to the smartcard
 *    and retrieving the related reply data block from the smartcard.
 *
 *    The function stops receiving bytes when the xReplyMaxLen value is reached
 *    or when no character has been received for more than CharacterWaitingTime.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface session, as given in the
 *             event notification callback.
 *  @param   xSendLen
 *             Length of send sequence.
 *  @param   pxSendBytes
 *             Pointer to the buffer containing the send sequence to be sent
 *             to the smartcard or NULL if only receive transfer.
 *  @param   xReceiveMaxLen
 *             Maximum length of the reply sequence or 0 if only send
 *             transfer.
 *  @param   pxReceiveLen
 *             Length of the reply block.
 *  @param   pxReceiveBytes
 *             Pointer to the buffer where the smartcard reply block will
 *             be stored.
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_CARD_REMOVED
 *              The smartcard in not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *  @retval  ICC_ERROR_TIMEOUT
 *             The data exchange stopped due to excessive time between
 *             successive characters.
 *  @retval  ICC_ERROR
 *             The data exchange has failed due to communication errors.
 *
 *  @remarks
 *    -# This function is synchronous. Data pointed to by pxReplyBlock must be
 *       valid as soon as the CA returns from this function.
*/
TIccStatus iccT0Exchange
(
        TIccSessionId     xSessionId,
        TSize             xSendLen,
  const TUnsignedInt8*   pxSendBytes,
        TSize             xReceiveMaxLen,
        TSize*           pxReceiveLen,
        TUnsignedInt8*   pxReceiveBytes
)
{
	if (xSessionId != SESSION_ID) {
		ICC_ERROR_PRINT("Error: iccT0Exchange ===> xSessionId != SESSION_ID\n");
		return ICC_ERROR_SESSION_ID;
	}
	DMSG("\n");

	pthread_mutex_lock(&icc_mutex);

	if (icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD) {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccT0Exchange ===> icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD\n");
		return ICC_ERROR_CARD_REMOVED;
	}

	pthread_mutex_unlock(&icc_mutex);
	return ICC_ERROR;
}


/**
 *  @brief
 *    This function allows the CA to change the smartcard access mode. It may
 *    be called at any time between insertion and extraction notifications.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard session to modify, as given in the
 *             event notification callback.
 *  @param   xMode
 *             New smartcard access mode. ICC_ACCESS_NONE is used to release
 *             the smartcard.
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_MODE
 *              The requested access mode is not supported.
 *  @retval   ICC_ERROR_CONFLICT
 *              The requested access mode is in conflict with another
 *              application.
*/
TIccStatus iccModeChange
(
  TIccSessionId       xSessionId,
  TIccAccessMode      xMode
)
{
	DMSG("\n");

	if (xSessionId != SESSION_ID) {
		ICC_ERROR_PRINT("Error: iccModeChange ===> xSessionId != SESSION_ID\n");
		return ICC_ERROR_SESSION_ID;
	}

	pthread_mutex_lock(&icc_mutex);

	if (xMode == ICC_ACCESS_NONE) {
		icc_info.active = 0;
		icc_info.access_mode = ICC_ACCESS_NONE;
	} else if (xMode == ICC_ACCESS_EXCLUSIVE) {
		icc_info.active = 1;
		icc_info.access_mode = ICC_ACCESS_EXCLUSIVE;
	} else {
		pthread_mutex_unlock(&icc_mutex);

		ICC_ERROR_PRINT("Error: iccModeChange ===> Unsupport mode %d.\n",
		  xMode);
		return ICC_ERROR_MODE;
	}

	pthread_mutex_unlock(&icc_mutex);
	return ICC_NO_ERROR;
}


/*
 * This function is a internal implementation of iccSmartcardReset().
 * It must be protected by the mutex of icc_mutex.
 */


static TIccStatus smartcard_reset(TBoolean xColdReset)
{
	mt_s32	ret = 0;
	mt_u8	atr[ICC_ATR_MAX_LEN]={0};
	mt_u8 	atr_rl = 0;
	ATRInfo_S atr_info;

	printf("smartcard_reset-----------------------------------------\n");

	if (icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD) {
		ICC_ERROR_PRINT("Error: iccSmartcardReset ===> icc_info.card_status == MT_UNF_SCI_STATUS_NOCARD\n");
		return ICC_ERROR_CARD_REMOVED;
	}

	if (icc_info.access_mode != ICC_ACCESS_EXCLUSIVE) {
		ICC_ERROR_PRINT("Error: iccSmartcardReset ===> Current access mode is "
		  "%d. Only ICC_ACCESS_EXCLUSIVE mode can be reset.\n",
		  icc_info.access_mode);
		return ICC_ERROR_MODE;
	}

	if (xColdReset == TRUE) {
		ret = mt_unf_sci_resetcard(ICC_PORT, MT_FALSE);
		if (MT_SUCCESS != ret) {
			ICC_ERROR_PRINT("Error: iccSmartcardReset ===> mt_unf_sci_resetcard %d ret = %d\n", __LINE__, ret);
			goto CARD_MUTE;
		}
	} else {
		ret = mt_unf_sci_resetcard(ICC_PORT, MT_TRUE);
		if (MT_SUCCESS != ret) {
			ICC_ERROR_PRINT("Error: iccSmartcardReset ===> mt_unf_sci_resetcard %d\n", __LINE__);
			goto CARD_MUTE;
		}
	}
	ret = mt_unf_sci_getatr(ICC_PORT, atr, ICC_ATR_MAX_LEN, &atr_rl);
	if (MT_SUCCESS != ret) {
		ICC_ERROR_PRINT("Error: iccSmartcardReset ===> mt_unf_sci_getatr %d\n", __LINE__);
		goto CARD_MUTE;
	}

	if(0 == atr_rl)
	{
		ICC_ERROR_PRINT("Error: iccSmartcardReset ===> mt_unf_sci_getatr %d\n", __LINE__);
		goto CARD_MUTE;
	}

	memset(&atr_info, 0, sizeof(ATRInfo_S));
	if (parse_atr(atr, atr_rl, &atr_info) != 0) {
		ICC_ERROR_PRINT("Error: iccSmartcardReset ===> parse_atr %d\n", __LINE__);
		goto CARD_MUTE;
	}

	icc_info.etu = atr_info.etu;
	icc_info.WT = atr_info.WT;
	icc_info.CWT = atr_info.CWT;
	icc_info.BWT = atr_info.BWT;
	icc_info.BGT = 22 * atr_info.etu;
	if(atr_info.etu)
	{
		icc_info.BWT_etu = (atr_info.BWT/atr_info.etu)+15;
		icc_info.CWT_etu = (atr_info.CWT/atr_info.etu)+15;
	}
	else
	{
		icc_info.BWT_etu = 0;
		icc_info.CWT_etu = 0;
	}
	icc_info.error_value = 0;
    if(atr_info.GT < 14)
    {
    	atr_info.GT+=1;
		icc_info.error_value = 1;
    }

	ret = mt_unf_sci_setguardtime(ICC_PORT, atr_info.GT);
	if (MT_SUCCESS != ret) {
		ICC_ERROR_PRINT("Error: iccSmartcardReset ===> mt_unf_sci_setguardtime %d\n", __LINE__);
		goto CARD_MUTE;
	}

	if((atr[1] & 0x10) == 0) //if TA1 is not occur in the ATR.
	{
		 //No TA1, Do not need to negotiate PPS!
		 ICC_DEBUG_PRINT("[SC] No TA1, Do not need to negotiate PPS!\n");
		 goto NOT_NEED_PPS;
	}

	/*
	 * For the Nagra specific requirement(RQ_M-DAL-ICC-CN-0010)
	 * Nagravision smart cards do not support the negotiable mode.
	 * They work in specific mode. A given smart card supports one
	 * single protocol only (T=0 or T=1)
	 * So we must not do PPS negotiable.
	 */
	ret = mt_unf_sci_setetufactor(ICC_PORT, (mt_u32)atr_info.Fi, (mt_u32)atr_info.Di);
	if (MT_SUCCESS != ret) {
		ICC_ERROR_PRINT("Error: iccSmartcardReset ===> mt_unf_sci_setetufactor %d\n", __LINE__);
		goto CARD_MUTE;
	}
NOT_NEED_PPS:
	memcpy(icc_info.atr, atr, ICC_ATR_MAX_LEN);
	icc_info.active = 1;
	return ICC_NO_ERROR;
CARD_MUTE:
	icc_info.active = 0;
	return ICC_ERROR_CARD_MUTE;
}

/**
 *  @brief
 *    This function allows the CA to reset the smartcard.
 *
 *    It may be called at any time between insertion and extraction
 *    notifications. This call is only allowed if the application
 *    communicates with the smartcard in exclusive mode.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface session to reset,
 *             as given in the event notification callback.
 *  @param   xColdReset
 *             TRUE if the driver has to initiate a cold reset.
 *             FALSE if the driver has to initiate a warm reset.
 *
 *  @retval   ICC_NO_ERROR
 *              The smartcard has been successfully reset.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_MODE
 *              The current session is as shared session and thus the
 *              smartcard cannot be reset.
 *  @retval   ICC_ERROR_REMOVED
 *              The smart card is not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *
 *  @remarks
 *    -# This function is synchronous. The ATR record must have been updated
 *       when this function returns.
*/
TIccStatus iccSmartcardReset
(
  TIccSessionId       xSessionId,
  TBoolean            xColdReset
)
{
	TIccStatus	ret = ICC_NO_ERROR;
	DMSG("\n");

	if (xSessionId != SESSION_ID) {
		ICC_ERROR_PRINT("Error: iccSmartcardReset ===> xSessionId != SESSION_ID\n");
		return ICC_ERROR_SESSION_ID;
	}

	pthread_mutex_lock(&icc_mutex);
	ret = smartcard_reset(xColdReset);
	pthread_mutex_unlock(&icc_mutex);

	return ret;
}

