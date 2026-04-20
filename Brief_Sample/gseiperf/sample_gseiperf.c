/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdlib.h>
#include <pthread.h>
#include <linux/fs.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#include "mt_type.h"
#include "mt_unf_common.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_unf_disp.h"
#include "mt_unf_vo.h"
#include "mt_unf_demux.h"
#include "mt_unf_frontend.h"
#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"
#include "mt_adp_search.h"
#include "mt_adp_demux.h"
#include "mt_adp_frontend.h"
#include <sys/time.h>

/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_GSE_DEBUG

#define MT_GSE_PRINT   printf
#else

#define MT_GSE_PRINT

#endif

#define SAMPLE_GSE_FUNCTION_ENTER() MT_GSE_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_GSE_FUNCTION_EXIT()      MT_GSE_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_GSE_FATAL_PRINT(fmt...)      MT_GSE_PRINT(" [FATAL] " fmt)
#define SAMPLE_GSE_ERR_PRINT(fmt...)            MT_GSE_PRINT(" [ERROR] " fmt)
#define SAMPLE_GSE_WARN_PRINT(fmt...)           MT_GSE_PRINT(" [WARN] "  fmt)
#define SAMPLE_GSE_INFO_PRINT(fmt...)           MT_GSE_PRINT(" [INFO] "  fmt)
#define SAMPLE_GSE_DBG_PRINT(fmt...)            MT_GSE_PRINT(" [DEBUG] " fmt)


#define SAMPLE_GSE_PRINT  printf


#define DMX_ID_0            0
#define TUNER_ID_0          0
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

#define RECORD_BUFF_SIZE 4*1024*1024
#define MAX_GSE_PACKET_ID 0xC74D


/*************************** Structure Definition ****************************/

typedef struct
{
    mt_s32 tuner_id;
    mt_u32 freq; /**<Frequency, in kHz*/
    mt_u32 sym_rate; /**<Symbol rate, in bit/s*/
    mt_u32 onoff_22k; /**<22k*/
    mt_u32 polar; /**<Polarization mode>*/
    mt_u32 dvbs_type; /**<dvbs type>*/
	mt_u8 package_mode;/**<0:GSE MODE 1:BBFRAME MODE*/
	mt_u32 source_id;/**<pcap file source id;0:iperf_udp_test.pcap 1:iperf_udp_45Mbps.pcap*/
} mt_input_Dvbs_para_t;


typedef enum GSE_PACKET_TYPE{
    PADDING_TYPE = 0,
    INTERMEDIATE_TYPE = 1,
    START_TYPE = 2,
    END_TYPE = 3,
    COMPLETE_TYPE = 4
}GSE_PACKET_TYPE;



typedef enum TS_GS_PACKET_TYPE{
	ERR_PACKET = 0,
	START_TS_PACKET = 1,
	MID_TS_PACKET = 2,
	END_TS_PACKET = 3,
	START_END_TS_PACKET = 4,
}TS_GS_PACKET_TYPE;



struct bbheader
{
    mt_u8  ts_gs_mode;//0:GSE packet //1:ges continuous //2:GSE-HEM  //3:TS
    mt_u8  is_sis;//1//1: single 0:multi
    mt_u8  is_ccm;//1//1:CCM 0:ACM
    mt_u8  is_issy;//1//1:active 0:no active
    mt_u8  is_gse_lite;//1//1:active 0:no active
    mt_u16 dfl_len;//in byte
    mt_u16 upl_len;//in byte
    mt_u8 sync;
    mt_u16 syncd_size;
    mt_u8 crc;

};
struct gseheader
{
    mt_u8 start_bit;
    mt_u8 end_bit;
    mt_u8 label_type;
    mt_u8 label_size;//0: no label 3:label 3byte  6: label 6byte
    mt_u16 gse_len;
    mt_u8 frag_id;
    mt_u16 total_len;
    mt_u16 total_data_len;
    mt_u16 data_len;
    mt_u16 protocol_type;
    GSE_PACKET_TYPE packet_type;
    mt_u8 start_copy_id;
};
typedef struct {
    struct bbheader bbh;
    struct gseheader gseh;
    mt_u8 pdu_data[65535+4096];
    mt_u8 remain_gse_data[4096+4096];
    mt_u8 pdu_data_frag_id;
    mt_u32 pdu_total_size;
    mt_u32 remain_pdu_data;
    mt_u32 remain_data_gse;
    mt_u32 remain_copy_data_gse;

}data_decoder;

typedef struct {
	mt_u8 start_parse_flag;
    TS_GS_PACKET_TYPE packet_type;
    struct bbheader bbheader_data;
    struct gseheader gseh;
    mt_u16 adaption_len;
    mt_u8 dfl_data[58112];
	mt_u8 gs_data[4096];
    mt_u8 bbh[10];
    mt_u16 remaining_gs_data_count;
    mt_u16 remaining_dfl_data_count;
    mt_u8 is_null_bbf_flag;/*1:is null bbf flag*/
    mt_u8 is_null_gse_flag;/*1:is null gse flag*/
    mt_u16 padding_length;
}data_decoder_ts;





typedef struct
{
    pthread_t ges_thread;
    mt_u32 last_gse_packet_id;
    mt_u32 error_happen_flag;
    mt_u32 gse_packet_total;
    mt_u32 gse_packet_err_total;
    MT_HANDLE gse_handle;
    data_decoder gse_decoder;
    data_decoder_ts ts_gse_decoder;
} MT_GSE_RUN_INFO;



/********************** Global Variable declaration **************************/

static MT_BOOL    g_bTaskQuit = MT_TRUE;

static MT_GSE_RUN_INFO    g_stGseRunInfo;
static mt_input_Dvbs_para_t    g_sInputParam = {0};

#ifdef MT_SAMPLE_APP
MT_S32 MT_GseiperfMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_s32 MT_GseCheckParam(mt_input_Dvbs_para_t *p_dvbs_in)
{
    if((p_dvbs_in->freq) > 4200 || (p_dvbs_in->freq) < 3000)
    {
        SAMPLE_GSE_ERR_PRINT("freq error. freq = %d \n", p_dvbs_in->freq);
        SAMPLE_GSE_ERR_PRINT("freq must be more than 3,000 and less than 4,200.\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}



/*!
@brief Demux initializes and retrieves the PMT and PAT tables in TS.
@return::MT_SUCCESS             Success.
@return::ret                    The return value of the error.
@*/
static MT_S32 MT_GseDmxInit(mt_u32 tuner_id)
{
    MT_S32                 ret = MT_FAILURE;
    mt_sys_version_s       stSysChipInfo = { 0 };

    /** Obtain the chip model */
    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    ret = mt_sys_get_version(&stSysChipInfo);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSE_ERR_PRINT("mt_sys_get_version failed-------<%s> line: %d\n", __FUNCTION__, __LINE__);

        return ret;
    }

    /** Initializes the demux module */
    ret = MT_UNF_DMX_Init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSE_ERR_PRINT("call MT_UNF_DMX_Init failed.\n");
        return ret;
    }

    if(MT_CHIP_SYMPHONY2_A0  <= stSysChipInfo.enChipVersion)
    {
        if(tuner_id == 0)
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_0);

            SAMPLE_GSE_INFO_PRINT("Connect port 0!\n");
        }
        else if(tuner_id == 1)
        {
#ifdef CONFIG_MT_CHIP_SYMPHONY6
            /** Bind Demux to tuner port 3 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_3);
            SAMPLE_GSE_INFO_PRINT("Connect port 3!\n");
#else
            /** Bind Demux to tuner port 1 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
            SAMPLE_GSE_INFO_PRINT("Connect port 1!\n");
#endif
        }
        else
        {
            /** Bind Demux to tuner port 0 */
            ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, DMX_DVB_TSI_IN_PORT);

            SAMPLE_GSE_INFO_PRINT("Connect port 0!\n");
        }
    }
    else
    {
        /** Bind Demux to tuner port 1 */
        ret = MT_UNF_DMX_AttachTSPort(DMX_ID_0, MT_UNF_DMX_PORT_TSI_1);
        SAMPLE_GSE_INFO_PRINT("Connect port 1!\n");
    }

    if(MT_SUCCESS != ret)
    {
        SAMPLE_GSE_ERR_PRINT("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        return ret;
    }

    return MT_SUCCESS;
}

/*
 @brief DmxDeinit
 @return void
*/
static void MT_GseDmxDeInit(MT_VOID)
{

    (MT_VOID)MT_UNF_DMX_DetachTSPort(DMX_ID_0);

    (MT_VOID)MT_UNF_DMX_DeInit();

}

static MT_U32 MT_Gseget_systime(void)
{
    struct  timeval  start;
    MT_U64 systime;
    gettimeofday(&start,NULL);
    systime = 1000*start.tv_sec+ start.tv_usec/1000;
    return (MT_U32)systime;
}
static void MT_Gse_decoder_info( data_decoder_ts *p_decoder,mt_s32 line)
{
    SAMPLE_GSE_INFO_PRINT("%d start_parse_dfl_flag:%d \n",line,p_decoder->start_parse_flag);
    SAMPLE_GSE_INFO_PRINT("%d packet_type:%d(1:start,2:mid,3:end) \n",line,p_decoder->packet_type);
    SAMPLE_GSE_INFO_PRINT("%d remaining_gs_data_count:%d\n",line,p_decoder->remaining_gs_data_count);
    SAMPLE_GSE_INFO_PRINT("%d remaining_dfl_data_count:%d\n",line,p_decoder->remaining_dfl_data_count);

}
static void MT_Gsedump_data(char *p_name, mt_u8 *p_data, mt_s32 length)
{
    mt_u32 i;
    printf("\n");
    printf("begin dump [%s] data, length = [%d]: \n", p_name, length);
    for (i = 0; i < length; i++)
    {
        printf("%02x ", p_data[i]);
        if((i+1)%16 == 0)
            printf("\n");
    }
    printf("\n\n");
}
static void MT_Gsebbheader_init(const mt_u8 *p_in,struct bbheader *p_bbheader)
{
    if(p_in[0] == 0xb4)
    {
        p_bbheader->ts_gs_mode = 2;
        p_bbheader->is_sis = 1;
        p_bbheader->is_ccm = 1;
        p_bbheader->is_issy = 0;
        p_bbheader->is_gse_lite = 1;
    }
    else
    {
        printf("[Info]matype1 != 0xb4,is 0x%x \n",p_in[0]);
        p_bbheader->ts_gs_mode = (p_in[0]&0xc0)>>6;
        p_bbheader->is_sis = (p_in[0]&0x20)>>5;
        p_bbheader->is_ccm = (p_in[0]&0x10)>>4;
        p_bbheader->is_issy = (p_in[0]&0x08)>>3;
        p_bbheader->is_gse_lite = (p_in[0]&0x04)>>2;
    }
    /*BBHeader format:MATYPE (2 bytes),UPL (2 bytes),DFL (2 bytes),SYNC (1 byte),SYNCD (2 bytes)*/
    p_bbheader->sync = p_in[6];
    p_bbheader->syncd_size = ((p_in[7]<<8)|p_in[8])/8;
    p_bbheader->upl_len = ((p_in[2]<<8)|p_in[3])/8;
    p_bbheader->dfl_len = ((p_in[4]<<8)|p_in[5])/8;
    p_bbheader->crc = p_in[9];

}
static void MT_Gseheader_init(const mt_u8 *p_in,struct gseheader *p_gseheader)
{
    memset(p_gseheader, 0, sizeof(struct gseheader));
    if((p_in[0]&0x80)== 0x80)
    {
        p_gseheader->start_bit = 1;
    }
    else
    {
        p_gseheader->start_bit = 0;
    }


    if((p_in[0]&0x40)== 0x40)
    {
       p_gseheader->end_bit = 1;
    }
    else
    {
        p_gseheader->end_bit = 0;
    }


    if((p_in[0]&0x30)== 0x30)
    {
        p_gseheader->label_type = 3;
        p_gseheader->label_size = 0;
    }
    else if((p_in[0]&0x30)== 0x20)
    {
        p_gseheader->label_type = 2;
        p_gseheader->label_size = 0;
    }
    else if((p_in[0]&0x30)== 0x10)
    {
        p_gseheader->label_type = 1;
        p_gseheader->label_size = 3;
    }
    else
    {
        p_gseheader->label_type = 0;
        p_gseheader->label_size = 6;
    }

    if((p_gseheader->start_bit == 0)&&(p_gseheader->end_bit == 0) && (p_gseheader->label_type == 0))
    {
        p_gseheader->packet_type = PADDING_TYPE;
    }
    else
    {
        p_gseheader->gse_len = ((p_in[0]&0x0F)<<8)|p_in[1];
        if((p_gseheader->start_bit == 0) || (p_gseheader->end_bit == 0))
        {
            p_gseheader->frag_id = p_in[2];
            p_gseheader->packet_type = INTERMEDIATE_TYPE;
        }

        if((p_gseheader->start_bit == 1) && (p_gseheader->end_bit == 0))
        {
            p_gseheader->total_len = (p_in[3]<<8)|p_in[4];
            p_gseheader->packet_type = START_TYPE;
            p_gseheader->protocol_type = (p_in[5]<<8)|p_in[6];
        }
        if(p_gseheader->start_bit == 1)
        {
            if(p_gseheader->end_bit == 1)
            {
                p_gseheader->packet_type = COMPLETE_TYPE;
                p_gseheader->protocol_type = (p_in[2]<<8)|p_in[3];
            }
            if(p_gseheader->label_type == 0)
            {
                p_gseheader->label_size = 6;
            }
            else if (p_gseheader->label_type == 1)
            {
                p_gseheader->label_size = 3;
            }

        }
        if((p_gseheader->start_bit == 0) && (p_gseheader->end_bit == 1))
        {
            p_gseheader->packet_type = END_TYPE;
        }

    }

    if(p_gseheader->packet_type == COMPLETE_TYPE)
    {
        p_gseheader->start_copy_id =2 + 2 + p_gseheader->label_size;
        p_gseheader->data_len = p_gseheader->gse_len - 2 - p_gseheader->label_size;
        p_gseheader->total_data_len = p_gseheader->data_len;
    }
    else if(p_gseheader->packet_type == START_TYPE)
    {
        p_gseheader->start_copy_id = 2+5+p_gseheader->label_size;
        p_gseheader->data_len = p_gseheader->gse_len-5-p_gseheader->label_size;
        p_gseheader->total_data_len = p_gseheader->total_len-2-p_gseheader->label_size;

    }
    else if((p_gseheader->packet_type == INTERMEDIATE_TYPE)||(p_gseheader->packet_type == END_TYPE))
    {
        p_gseheader->start_copy_id = 2 + 1 + p_gseheader->label_size;
        p_gseheader->data_len = p_gseheader->gse_len - 1 - p_gseheader->label_size;
    }
    else if(p_gseheader->packet_type == PADDING_TYPE)
    {
        SAMPLE_GSE_INFO_PRINT("PADDING_TYPE packtype is %d \n",p_gseheader->packet_type);
        p_gseheader->data_len = 0;
    }
    else
    {
        SAMPLE_GSE_ERR_PRINT("Unsupport packtype is %d \n",p_gseheader->packet_type);
        p_gseheader->data_len = 0;
    }


}

static mt_s32 MT_Gseparse_payload(const mt_u8 *p_gse_data,mt_s32 in_length)
{
    mt_u32 gse_packet_id = 0;
    mt_u32 i,j;
    mt_u8 value = 0;

    g_stGseRunInfo.gse_packet_total++;

    gse_packet_id = p_gse_data[0x1E]<<8|p_gse_data[0x1F];

    if(g_stGseRunInfo.last_gse_packet_id == 0xFFFF)
    {
       g_stGseRunInfo.last_gse_packet_id = gse_packet_id;
    }
    else
    {
        if((gse_packet_id != (g_stGseRunInfo.last_gse_packet_id+1))&&(g_stGseRunInfo.last_gse_packet_id != MAX_GSE_PACKET_ID))
        {
            g_stGseRunInfo.gse_packet_err_total++;
            g_stGseRunInfo.error_happen_flag = 1;
            SAMPLE_GSE_ERR_PRINT("[ERR]No match g_stGseRunInfo.last_gse_packet_id:%d,gse_packet_id:%d \n",g_stGseRunInfo.last_gse_packet_id,gse_packet_id);
            goto ERR;
        }
        else
        {
            i = 0x44;
            while(i<(in_length))
            {
                for(j =0;j<10;j++)
                {
                    value = p_gse_data[i+j];
                    if(value != (0x30+j))
                    {
                        SAMPLE_GSE_ERR_PRINT("[ERR]No match i:%d j:%d value:0x%x != 0x%x \n",i,j,value,(0x30+j));
                        SAMPLE_GSE_ERR_PRINT("in_length:%d gse_packet_id:%d \n",in_length,gse_packet_id);
                        g_stGseRunInfo.gse_packet_err_total++;
                        g_stGseRunInfo.error_happen_flag = 1;
                        goto ERR;
                    }
                }
                i +=10;
            }
        }
        g_stGseRunInfo.last_gse_packet_id = gse_packet_id;
    }
    return 0;
ERR:
    g_stGseRunInfo.last_gse_packet_id = gse_packet_id;
    return -1;
}
/*prase the gse payload for iperf_udp_45Mbps.pcap */
static mt_s32 MT_Gseparse_payload_for_45M_ipref(const mt_u8 *p_gse_data,mt_s32 in_length)
{
    mt_u32 gse_packet_id = 0;
    mt_u32 i,j;
    mt_u8 value = 0;
    
    g_stGseRunInfo.gse_packet_total++;

    gse_packet_id = p_gse_data[0x1E]<<8|p_gse_data[0x1F];

	/*check gse packet id*/
	if((g_stGseRunInfo.last_gse_packet_id == 0xFFFF) && (gse_packet_id != 0x0000))
	{
		g_stGseRunInfo.last_gse_packet_id = gse_packet_id; /*Start play*/ 
	}
	else if((g_stGseRunInfo.last_gse_packet_id == 0x80D7) && (gse_packet_id == 0x7F2D))
	{
		g_stGseRunInfo.last_gse_packet_id = 0xFFFF;/*The end of iperf_udp_45Mbps.pcap*/
	}
	else if((g_stGseRunInfo.last_gse_packet_id == 0xFFFF) && (gse_packet_id == 0x0000))
	{
		g_stGseRunInfo.last_gse_packet_id = 0x0000;/*id rollback*/
	}
    else if (gse_packet_id != (g_stGseRunInfo.last_gse_packet_id+1))
    {
        g_stGseRunInfo.gse_packet_err_total++;
        g_stGseRunInfo.error_happen_flag = 1;
        SAMPLE_GSE_ERR_PRINT("[45M ERR]No match last_gse_packet_id:0x%x,gse_packet_id:0x%x \n",g_stGseRunInfo.last_gse_packet_id,gse_packet_id);   
        goto ERR;		
	}
	else if(gse_packet_id == (g_stGseRunInfo.last_gse_packet_id+1))
	{
		g_stGseRunInfo.last_gse_packet_id = gse_packet_id;
	}
	else
	{
        g_stGseRunInfo.gse_packet_err_total++;
        g_stGseRunInfo.error_happen_flag = 1;
        SAMPLE_GSE_ERR_PRINT("[45M ERR]last_gse_packet_id:0x%x,gse_packet_id:0x%x \n",g_stGseRunInfo.last_gse_packet_id,gse_packet_id);   
        goto ERR;
	}

	/*check data*/
	i = 0x44;
    while(i<(in_length))
    {
        for(j =0;j<10;j++)
        {
            value = p_gse_data[i+j];
            //SAMPLE_GSE_ERR_PRINT("i:%d,j:%d,value:0x%x \n",i,j,value);
            if(value != (0x30+j))
            {
                SAMPLE_GSE_ERR_PRINT("[45M ERR]No match i:%d j:%d value:0x%x != 0x%x \n",i,j,value,(0x30+j));
                SAMPLE_GSE_ERR_PRINT("in_length:%d gse_packet_id:%d \n",in_length,gse_packet_id);
                g_stGseRunInfo.gse_packet_err_total++;
                g_stGseRunInfo.error_happen_flag = 1;
                goto ERR;
            }
        }
        i +=10;
    } 
    //SAMPLE_GSE_ERR_PRINT("gse_packet_total:%d,gse_packet_id:%d \n",g_stGseRunInfo.gse_packet_total,gse_packet_id);
    return 0;
ERR:
    g_stGseRunInfo.last_gse_packet_id = gse_packet_id;
    return -1;
}
static void MT_Gseget_ts_packet_info_in_gse_mode(const mt_u8 *p_ts_packet,data_decoder_ts *p_decoder)
{
	//dump_data("p_ts_packet",p_ts_packet,5);
	if((p_ts_packet[0] == 0x47) &&(p_ts_packet[2] == 0xe0))
	{
        if(p_ts_packet[1] == 0x40)//GSE start packet
        {
			if(p_decoder->remaining_gs_data_count !=0)
			{
				SAMPLE_GSE_ERR_PRINT("[ERR]remaining dfl data should be 0 when bbframe start,now is %d,reset decoder\n",p_decoder->remaining_gs_data_count);
				MT_Gse_decoder_info(&g_stGseRunInfo.ts_gse_decoder,__LINE__);
			    memset(&g_stGseRunInfo.ts_gse_decoder, 0, sizeof(data_decoder_ts));
			}

			p_decoder->packet_type = START_TS_PACKET;
   			memset(p_decoder->gs_data, 0, 4096*sizeof(p_decoder->gs_data[0]));
            MT_Gseheader_init(&p_ts_packet[13], &(p_decoder->gseh));
            if(p_decoder->gseh.gse_len == 0x00)
            {
                SAMPLE_GSE_ERR_PRINT("[ERR] It is NULL GSE\n");
                g_stGseRunInfo.ts_gse_decoder.is_null_gse_flag = 1;
            }
            else
            {
                g_stGseRunInfo.ts_gse_decoder.is_null_gse_flag = 0;
            }
            p_decoder->remaining_gs_data_count = p_decoder->gseh.gse_len+2;
            p_decoder->start_parse_flag = 0;
            if(p_decoder->remaining_gs_data_count < 188 - 13)
            {
                SAMPLE_GSE_ERR_PRINT("[%s %d]START_TS_PACKET  remaining_gs_data_count :%d \n ",__FUNCTION__,__LINE__,p_decoder->remaining_gs_data_count);
                p_decoder->start_parse_flag = 1;
            }

        }
        else if((p_ts_packet[3]&0xF0) == 0x30)//GSE end packet 
        {
            p_decoder->packet_type = END_TS_PACKET;
			p_decoder->adaption_len = p_ts_packet[4];
			p_decoder->start_parse_flag = 1;
        }
        else
        {
            p_decoder->packet_type = MID_TS_PACKET;
            if(p_decoder->remaining_gs_data_count == 184)
            {
                SAMPLE_GSE_ERR_PRINT("[%s %d]BBFRAME_MID_TS_PACKET  remaining_dfl_data_count :%d \n ",__FUNCTION__,__LINE__,p_decoder->remaining_gs_data_count);
            }

        }
    }
    else
	{
		p_decoder->packet_type = ERR_PACKET;
		SAMPLE_GSE_ERR_PRINT("[ERR %s %d]This is no a TS packet,TS[0] = 0x%x TS[2] = 0x%x\n ",__FUNCTION__,__LINE__,p_ts_packet[0],p_ts_packet[2]);
	}
}
static void MT_Gseget_ts_packet_info(const mt_u8 *p_ts_packet, data_decoder_ts *p_decoder)
{
    mt_u32 bbframe_header_start_index = 0;

    if((p_ts_packet[0] == 0x47) &&(p_ts_packet[2] == 0xe0))
    {
        if(p_ts_packet[1] == 0x40)//BB Frame start packet
        {
            if(p_decoder->remaining_dfl_data_count !=0)
            {
                SAMPLE_GSE_ERR_PRINT("[ERR]remaining dfl data should be 0 when bbframe start,now is %d,reset decoder\n",p_decoder->remaining_dfl_data_count);
                MT_Gse_decoder_info(&g_stGseRunInfo.ts_gse_decoder,__LINE__);
                memset(&g_stGseRunInfo.ts_gse_decoder, 0, sizeof( data_decoder_ts));
            }

            if((p_ts_packet[3]&0xF0) == 0x30)
            {
                p_decoder->packet_type = START_END_TS_PACKET;
                p_decoder->adaption_len = p_ts_packet[4];
                bbframe_header_start_index  = 4+1+p_decoder->adaption_len+9;
                p_decoder->start_parse_flag = 1;
            }
            else
            {
                p_decoder->packet_type = START_TS_PACKET;
                bbframe_header_start_index = 13;
                p_decoder->start_parse_flag = 0;
            }

            //p_decoder->packet_type = BBFRAME_START_TS_PACKET;
            memset(p_decoder->dfl_data, 0, 58112*sizeof(p_decoder->dfl_data[0]));
            MT_Gsebbheader_init(&p_ts_packet[bbframe_header_start_index],&(p_decoder->bbheader_data));
            if((p_decoder->bbheader_data.dfl_len == 0)&&(p_decoder->bbheader_data.syncd_size == 0x1FFF))
            {
                g_stGseRunInfo.ts_gse_decoder.is_null_bbf_flag = 1;
            }
            else
            {
                g_stGseRunInfo.ts_gse_decoder.is_null_bbf_flag = 0;
                memcpy(&(p_decoder->bbh[0]), &(p_ts_packet[bbframe_header_start_index]), 10);
            }

            p_decoder->remaining_dfl_data_count = p_decoder->bbheader_data.dfl_len;
            if(p_decoder->remaining_dfl_data_count < 188 - 23)
            {
                p_decoder->start_parse_flag = 1;
            }
        }
        else if((p_ts_packet[3]&0xF0) == 0x30)//BB Frame end packet
        {
            p_decoder->packet_type = END_TS_PACKET;
            p_decoder->adaption_len = p_ts_packet[4];
            p_decoder->start_parse_flag = 1;
        }
        else //BB Frame packet
        {
            p_decoder->packet_type = MID_TS_PACKET;
            if(p_decoder->remaining_dfl_data_count == 184)
            {
                SAMPLE_GSE_ERR_PRINT("[%s %d]BBFRAME_MID_TS_PACKET  remaining_dfl_data_count :%d \n ",__FUNCTION__,__LINE__,p_decoder->remaining_dfl_data_count);
            }
		}
	}
    else
    {
        p_decoder->packet_type = ERR_PACKET;
        SAMPLE_GSE_ERR_PRINT("[ERR]This is no a TS packet,TS[0] = 0x%x \n ",p_ts_packet[0]);
    }
}
static int MT_Gseparse_ts_packet_to_dfl_in_gse_mode(const mt_u8 *p_ts_packet, data_decoder_ts *p_decoder)
{
	mt_u16 gse_start_byte = 0;
	mt_u8 ts_start_byte = 0;
	mt_u8 copy_data_len = 0;


	gse_start_byte = (p_decoder->gseh.gse_len + 2)- p_decoder->remaining_gs_data_count;
	switch(p_decoder->packet_type)
	{
		case START_TS_PACKET:
			ts_start_byte = 13;//insert 13byte TS header 		
			break;
		case MID_TS_PACKET:
			ts_start_byte = 4;//insert 4byte TS header 47 00 pid 00			
			break;
		case END_TS_PACKET:
			ts_start_byte = 5 + p_decoder->adaption_len; //insert 5byte TS header 47 00 pid 30 adaption_len + adaption_len 00
			
			break;
		default:
			break;
	}
	copy_data_len = 188 - ts_start_byte;
	if(copy_data_len > p_decoder->remaining_gs_data_count)
	{
        copy_data_len = p_decoder->remaining_gs_data_count;

	}
	memcpy(&(p_decoder->gs_data[gse_start_byte]), &(p_ts_packet[ts_start_byte]), copy_data_len);
	p_decoder->remaining_gs_data_count -= copy_data_len;
	if((p_decoder->packet_type == END_TS_PACKET)&&(p_decoder->remaining_gs_data_count))
	{
		SAMPLE_GSE_ERR_PRINT("[ERR]This is end TS packer,but still %d byte need to read !!\n",p_decoder->remaining_gs_data_count);
        MT_Gsedump_data("gseheader", &(p_decoder->gs_data[0]), 10);
        MT_Gsedump_data("TSpacket",&(p_ts_packet[ts_start_byte]),188);
		return MT_FAILURE;
	}
	return MT_SUCCESS;
}
static mt_s32 MT_Gseparse_ts_packet_to_dfl(const mt_u8 *p_ts_packet, data_decoder_ts *p_decoder)
{
    mt_u16 dfl_start_byte = 0;
    mt_u8 ts_start_byte = 0;
    mt_u8 copy_data_len = 0;


    dfl_start_byte = p_decoder->bbheader_data.dfl_len - p_decoder->remaining_dfl_data_count;
    switch(p_decoder->packet_type)
    {
        case START_TS_PACKET:
            ts_start_byte = 23;//insert 13byte TS header and 10byte bbheader
            break;
        case MID_TS_PACKET:
            ts_start_byte = 4;//insert 4byte TS header 47 00 pid 00
            break;
        case END_TS_PACKET:
            ts_start_byte = 5 + p_decoder->adaption_len; //insert 5byte TS header 47 00 pid 30 adaption_len + adaption_len 00
            break;
        case START_END_TS_PACKET:
            ts_start_byte = 5 + p_decoder->adaption_len + 9 + 10;//insert 5byte TS header 47 40 pid 30 adaption_len + adaption_len 00 + 9 byte + 10byte bbheader
            break;
        default:
            break;
    }
    copy_data_len = 188 - ts_start_byte;
    if(copy_data_len > p_decoder->remaining_dfl_data_count)
    {
        copy_data_len = p_decoder->remaining_dfl_data_count;

    }
    memcpy(&(p_decoder->dfl_data[dfl_start_byte]), &(p_ts_packet[ts_start_byte]), copy_data_len);
    p_decoder->remaining_dfl_data_count -= copy_data_len;
    if((p_decoder->packet_type == END_TS_PACKET)&&(p_decoder->remaining_dfl_data_count))
    {
        SAMPLE_GSE_ERR_PRINT("[ERR]This is end TS packer,but still %d byte need to read !!\n",p_decoder->remaining_dfl_data_count);
        MT_Gsedump_data("BBHeader",&(p_decoder->bbh[0]),10);
        MT_Gsedump_data("TSpacket",&(p_ts_packet[ts_start_byte]),188);
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}
static mt_s32 MT_Gseget_gse_payload(const mt_u8 *p_in,mt_u8 *p_out, mt_s32 in_length)
{
    mt_s32 need_parse_data = 0;
    mt_u32 parsed_data = 0;
    mt_u32 coped_len = 0;
    mt_u8 buf[4096] = {0};
    mt_s32 ret = 0;

    need_parse_data = in_length;
    do{
		if((need_parse_data < 2)&&(parsed_data != 0))
		{/*When analyzing GSE data in a loop, if the data volume is less than 2, it should be carried over to the next round for analysis*/
			memset(g_stGseRunInfo.gse_decoder.remain_gse_data, 0, 4096*sizeof(g_stGseRunInfo.gse_decoder.remain_gse_data[0]));
			g_stGseRunInfo.gse_decoder.remain_data_gse = need_parse_data;
			g_stGseRunInfo.gse_decoder.remain_copy_data_gse = 0; /*Since the length of the gse date cannot be parsed in this case, this variable is set to 0*/
			memcpy(g_stGseRunInfo.gse_decoder.remain_gse_data, &p_in[parsed_data], g_stGseRunInfo.gse_decoder.remain_data_gse);
			break;
		}
        MT_Gseheader_init(&p_in[parsed_data], &g_stGseRunInfo.gse_decoder.gseh);


        if(need_parse_data < (g_stGseRunInfo.gse_decoder.gseh.data_len + g_stGseRunInfo.gse_decoder.gseh.start_copy_id) )  
        {
            memset(g_stGseRunInfo.gse_decoder.remain_gse_data, 0, 4096*sizeof(g_stGseRunInfo.gse_decoder.remain_gse_data[0]));
            g_stGseRunInfo.gse_decoder.remain_data_gse = need_parse_data;
            g_stGseRunInfo.gse_decoder.remain_copy_data_gse = (g_stGseRunInfo.gse_decoder.gseh.data_len + g_stGseRunInfo.gse_decoder.gseh.start_copy_id) - need_parse_data;
            memcpy(g_stGseRunInfo.gse_decoder.remain_gse_data, &p_in[parsed_data], g_stGseRunInfo.gse_decoder.remain_data_gse);
            need_parse_data = 0;
        }
        else
        {
            if(g_stGseRunInfo.gse_decoder.gseh.packet_type == PADDING_TYPE)
            {
                goto END;
            }
            else if(g_stGseRunInfo.gse_decoder.gseh.packet_type == COMPLETE_TYPE)
            {
                memcpy(buf,&p_in[parsed_data+g_stGseRunInfo.gse_decoder.gseh.start_copy_id],g_stGseRunInfo.gse_decoder.gseh.data_len);
                if(g_sInputParam.source_id == 1)
					ret = MT_Gseparse_payload_for_45M_ipref(buf,g_stGseRunInfo.gse_decoder.gseh.data_len);
				else
                	ret = MT_Gseparse_payload(buf,g_stGseRunInfo.gse_decoder.gseh.data_len);
                if(ret != 0)
                {
                    SAMPLE_GSE_ERR_PRINT("[ERR %s %d]parse_gse_err\n",__FUNCTION__,__LINE__);
                    MT_Gsedump_data("GSE_packet",&p_in[parsed_data],(g_stGseRunInfo.gse_decoder.gseh.data_len + g_stGseRunInfo.gse_decoder.gseh.start_copy_id));
                    
                }
                memcpy(&p_out[coped_len],&p_in[parsed_data+g_stGseRunInfo.gse_decoder.gseh.start_copy_id],g_stGseRunInfo.gse_decoder.gseh.data_len);
                coped_len += g_stGseRunInfo.gse_decoder.gseh.data_len;
            }
            else if(g_stGseRunInfo.gse_decoder.gseh.packet_type == START_TYPE)
            {
                g_stGseRunInfo.gse_decoder.pdu_data_frag_id = g_stGseRunInfo.gse_decoder.gseh.frag_id;
                g_stGseRunInfo.gse_decoder.pdu_total_size = g_stGseRunInfo.gse_decoder.gseh.total_data_len;
                memcpy(&p_out[coped_len],&p_in[parsed_data+g_stGseRunInfo.gse_decoder.gseh.start_copy_id],g_stGseRunInfo.gse_decoder.gseh.data_len);
                g_stGseRunInfo.gse_decoder.remain_pdu_data = g_stGseRunInfo.gse_decoder.pdu_total_size - g_stGseRunInfo.gse_decoder.gseh.data_len;
            }
            else if((g_stGseRunInfo.gse_decoder.gseh.packet_type == INTERMEDIATE_TYPE)&&(g_stGseRunInfo.gse_decoder.pdu_total_size > g_stGseRunInfo.gse_decoder.remain_pdu_data))
            {
                if(g_stGseRunInfo.gse_decoder.pdu_data_frag_id == g_stGseRunInfo.gse_decoder.gseh.frag_id)
                {
                    memcpy(&p_out[coped_len+(g_stGseRunInfo.gse_decoder.pdu_total_size - g_stGseRunInfo.gse_decoder.remain_pdu_data)],&p_in[parsed_data+g_stGseRunInfo.gse_decoder.gseh.start_copy_id],g_stGseRunInfo.gse_decoder.gseh.data_len);
                    g_stGseRunInfo.gse_decoder.remain_pdu_data -= g_stGseRunInfo.gse_decoder.gseh.data_len;
                }
                else
                {
                    SAMPLE_GSE_ERR_PRINT("Frag Id unmatch %d %d!!\n",g_stGseRunInfo.gse_decoder.pdu_data_frag_id,g_stGseRunInfo.gse_decoder.gseh.frag_id);
                }
            }
            else if(g_stGseRunInfo.gse_decoder.gseh.packet_type == END_TYPE)
            {
                if((g_stGseRunInfo.gse_decoder.pdu_data_frag_id == g_stGseRunInfo.gse_decoder.gseh.frag_id)&&(g_stGseRunInfo.gse_decoder.pdu_total_size > g_stGseRunInfo.gse_decoder.remain_pdu_data))
                {
                    memcpy(&p_out[coped_len+(g_stGseRunInfo.gse_decoder.pdu_total_size - g_stGseRunInfo.gse_decoder.remain_pdu_data)],&p_in[parsed_data+g_stGseRunInfo.gse_decoder.gseh.start_copy_id],g_stGseRunInfo.gse_decoder.gseh.data_len);
                    g_stGseRunInfo.gse_decoder.remain_pdu_data -= g_stGseRunInfo.gse_decoder.gseh.data_len;
                    coped_len += g_stGseRunInfo.gse_decoder.pdu_total_size;
                }
                else
                {
                    SAMPLE_GSE_ERR_PRINT("Frag Id unmatch %d %d!!\n",g_stGseRunInfo.gse_decoder.pdu_data_frag_id,g_stGseRunInfo.gse_decoder.gseh.frag_id);
                }
            }
            else
            {
                SAMPLE_GSE_ERR_PRINT("[ERR][%s %d]unsupported packet_type %d parsed_data %d \n",__FUNCTION__,__LINE__,g_stGseRunInfo.gse_decoder.gseh.packet_type,parsed_data);

            }
            need_parse_data -=(g_stGseRunInfo.gse_decoder.gseh.gse_len+2);
            parsed_data += (g_stGseRunInfo.gse_decoder.gseh.gse_len+2);
        }

    }while(need_parse_data > 0);
END:
    return coped_len;
}

static mt_s32 MT_Gseparse_bbframe(const mt_u8 *p_in, mt_u8 *p_out, mt_s32 in_length)
{
    mt_u32 need_parse_data = 0;
    mt_u32 parsed_data = 0;

    mt_u32 parsed_len = 0;
    mt_s32 len = 0;

    need_parse_data = in_length;

    do{
        MT_Gsebbheader_init(&p_in[parsed_data],&g_stGseRunInfo.gse_decoder.bbh);
        if((g_stGseRunInfo.gse_decoder.bbh.syncd_size == 0x1FFF || (g_stGseRunInfo.gse_decoder.bbh.syncd_size == 0xFFFF))&&(g_stGseRunInfo.gse_decoder.bbh.dfl_len != 0)&&(g_stGseRunInfo.gse_decoder.remain_copy_data_gse != 0))
        {
            g_stGseRunInfo.gse_decoder.bbh.syncd_size = g_stGseRunInfo.gse_decoder.remain_copy_data_gse;
        }
        if((g_stGseRunInfo.gse_decoder.remain_copy_data_gse == g_stGseRunInfo.gse_decoder.bbh.syncd_size)&&(g_stGseRunInfo.gse_decoder.bbh.syncd_size != 0))//32bit CRC
        {
            memcpy(&g_stGseRunInfo.gse_decoder.remain_gse_data[g_stGseRunInfo.gse_decoder.remain_data_gse], &p_in[10], g_stGseRunInfo.gse_decoder.remain_copy_data_gse);
            len = MT_Gseget_gse_payload(&g_stGseRunInfo.gse_decoder.remain_gse_data[0],g_stGseRunInfo.gse_decoder.pdu_data,(g_stGseRunInfo.gse_decoder.remain_data_gse+g_stGseRunInfo.gse_decoder.remain_copy_data_gse));
            memcpy(&p_out[parsed_len],g_stGseRunInfo.gse_decoder.pdu_data,len);
            parsed_len += len;
        }
		else if((g_stGseRunInfo.gse_decoder.remain_data_gse == 1) &&(g_stGseRunInfo.gse_decoder.remain_copy_data_gse == 0) &&(g_stGseRunInfo.gse_decoder.bbh.syncd_size != 0))
		{
			//MT_TEST_PRINTF("[%s %d]prase remain gse date in ges header not completion\n",__FUNCTION__,__LINE__);
			//memcpy(&gse_decoder.remain_gse_data[gse_decoder.remain_data_gse], &p_in[0], gse_decoder.remain_copy_data_gse);
            memcpy(&g_stGseRunInfo.gse_decoder.remain_gse_data[g_stGseRunInfo.gse_decoder.remain_data_gse], &p_in[10], g_stGseRunInfo.gse_decoder.bbh.syncd_size);
			len = MT_Gseget_gse_payload(&g_stGseRunInfo.gse_decoder.remain_gse_data[0],g_stGseRunInfo.gse_decoder.pdu_data,(g_stGseRunInfo.gse_decoder.remain_data_gse+g_stGseRunInfo.gse_decoder.bbh.syncd_size));
			memcpy(&p_out[parsed_len],g_stGseRunInfo.gse_decoder.pdu_data,len);
			parsed_len += len;
		}
        g_stGseRunInfo.gse_decoder.remain_copy_data_gse = 0;
        g_stGseRunInfo.gse_decoder.remain_data_gse = 0;

        if(g_stGseRunInfo.gse_decoder.bbh.dfl_len == 0)
        {
            SAMPLE_GSE_ERR_PRINT("[ERR] gse_decoder.bbh.dfl_len is 0\n");
            break;
        }
        else if(g_stGseRunInfo.gse_decoder.bbh.syncd_size > g_stGseRunInfo.gse_decoder.bbh.dfl_len)
        {
            SAMPLE_GSE_ERR_PRINT("[ERR] gse_decoder.bbh.syncd_size %d is lager than gse_decoder.bbh.dfl_len %d\n",g_stGseRunInfo.gse_decoder.bbh.syncd_size,g_stGseRunInfo.gse_decoder.bbh.dfl_len);
        }
		else if(g_stGseRunInfo.gse_decoder.bbh.dfl_len > g_stGseRunInfo.gse_decoder.bbh.syncd_size)
        {
            len = MT_Gseget_gse_payload(&p_in[parsed_data+10+g_stGseRunInfo.gse_decoder.bbh.syncd_size], g_stGseRunInfo.gse_decoder.pdu_data, (g_stGseRunInfo.gse_decoder.bbh.dfl_len - g_stGseRunInfo.gse_decoder.bbh.syncd_size));

            if(len > 0)
            {
                memcpy(&p_out[parsed_len],g_stGseRunInfo.gse_decoder.pdu_data,len);
                parsed_len += len;
            }
        }

        if(need_parse_data < (g_stGseRunInfo.gse_decoder.bbh.dfl_len+10))
        {
            need_parse_data = 0;
        }
        else
        {
            need_parse_data -= (g_stGseRunInfo.gse_decoder.bbh.dfl_len+10);
            parsed_data += (g_stGseRunInfo.gse_decoder.bbh.dfl_len+10);
        }
    }while(need_parse_data != 0);
    return parsed_len;
}
static mt_s32 MT_Gseparse_ts_packet_in_gse_mode(const mt_u8 *p_in, mt_u8 *p_out, mt_s32 in_length)
{
	mt_s32 out_length = -1;
	mt_s32 i = 0;
	mt_u32 packets_num;
	mt_u8 *p_ts_packet = NULL;
	mt_u8 *p_ts_stream = NULL;
	mt_u32 parse_len = 0;


	packets_num = in_length/188;
	p_ts_stream = p_out;
	out_length = 0;
    for (i = 0; i < packets_num; i++)
    {
        p_ts_packet = &p_in[i*188];
		if(p_ts_packet == NULL)
		{
			SAMPLE_GSE_ERR_PRINT("[ERR]packet %d is Null \n ",i);
			continue;
		}
        if(g_stGseRunInfo.error_happen_flag == 1)
        {
            SAMPLE_GSE_ERR_PRINT("[%s %d] i:%d \n",__FUNCTION__,__LINE__,i);
            MT_Gsedump_data("ts_packet3", &p_ts_packet[0], 188);
            g_stGseRunInfo.error_happen_flag = 0;

        }
        MT_Gseget_ts_packet_info_in_gse_mode(p_ts_packet, &g_stGseRunInfo.ts_gse_decoder);
        if(g_stGseRunInfo.ts_gse_decoder.is_null_gse_flag == 1)
		{
			SAMPLE_GSE_ERR_PRINT("[%s %d]is_null_gse_flag is 0,null packet %d \n",__FUNCTION__,__LINE__,i);
			continue;
		}
		if(g_stGseRunInfo.ts_gse_decoder.gseh.gse_len== 0)
		{
			SAMPLE_GSE_ERR_PRINT("[%s %d]gse_len is 0,waive packet %d \n",__FUNCTION__,__LINE__,i);
			continue;
		}
        if(g_stGseRunInfo.ts_gse_decoder.remaining_gs_data_count != 0)
		{
          if(MT_Gseparse_ts_packet_to_dfl_in_gse_mode(p_ts_packet, &g_stGseRunInfo.ts_gse_decoder)!= MT_SUCCESS)
		    {
			    /*Reset gse decoder*/
			    SAMPLE_GSE_ERR_PRINT("[ERR]In GSE mode parse_ts_packet_to_dfl in TS %d packet failed!!Reset Gse decoder \n ",i);
			    MT_Gse_decoder_info(&g_stGseRunInfo.ts_gse_decoder,__LINE__);
			    memset(&g_stGseRunInfo.ts_gse_decoder, 0, sizeof(data_decoder_ts));
			    continue;
		    }
        }
        if(g_stGseRunInfo.ts_gse_decoder.start_parse_flag == 1)
		{
            parse_len = MT_Gseget_gse_payload(&g_stGseRunInfo.ts_gse_decoder.gs_data[0], p_ts_stream, (g_stGseRunInfo.ts_gse_decoder.gseh.gse_len+2));
            if(g_stGseRunInfo.error_happen_flag == 1)
            {
                SAMPLE_GSE_ERR_PRINT("[%s %d] i:%d \n",__FUNCTION__,__LINE__,i);
                MT_Gsedump_data("ts_packet2", &p_ts_packet[0], 188);
            }
			out_length += parse_len;
		}
    }

	return out_length;

}
static mt_s32 MT_Gseparse_ts_packet(const mt_u8 *p_in, mt_u8 *p_out, mt_s32 in_length)
{

    mt_s32 out_length = -1;
    mt_s32 i = 0;
    mt_u32 packets_num;
    mt_u8 *p_ts_packet = NULL;
    mt_u8 *p_ts_stream = NULL;
    mt_u32 parse_len = 0;
    MT_U8 *bbframe = MT_NULL_PTR;

    bbframe = (MT_U8 *)malloc(sizeof(mt_u8)*65535);
    if (MT_NULL_PTR == bbframe)
    {
        SAMPLE_GSE_ERR_PRINT("[%s %d]kzalloc bbframe error\n", __FUNCTION__, __LINE__);
        return MT_FAILURE;
    }
    packets_num = in_length/188;
    p_ts_stream = p_out;
    out_length = 0;
    for (i = 0; i < packets_num; i++)
    {
        p_ts_packet = &p_in[i*188];
        if(p_ts_packet == NULL)
        {
            SAMPLE_GSE_ERR_PRINT("[ERR]packet %d is Null \n ",i);
            continue;
        }
        if(g_stGseRunInfo.error_happen_flag == 1)
        {
            SAMPLE_GSE_INFO_PRINT("[%s %d] i:%d \n",__FUNCTION__,__LINE__,i);
            MT_Gsedump_data("ts_packet3", &p_ts_packet[0], 32);
            g_stGseRunInfo.error_happen_flag = 0;
        }
        MT_Gseget_ts_packet_info(p_ts_packet, &g_stGseRunInfo.ts_gse_decoder);
        if(g_stGseRunInfo.ts_gse_decoder.is_null_bbf_flag == 1)
        {
            continue;
        }
        if(g_stGseRunInfo.ts_gse_decoder.bbheader_data.dfl_len == 0)
        {
            continue;
        }
        if(g_stGseRunInfo.ts_gse_decoder.remaining_dfl_data_count != 0)
        {
          if(MT_Gseparse_ts_packet_to_dfl(p_ts_packet, &g_stGseRunInfo.ts_gse_decoder)!= MT_SUCCESS)
            {
                /*Reset gse decoder*/
                SAMPLE_GSE_ERR_PRINT("[ERR]MT_Gseparse_ts_packet_to_dfl in TS %d packet failed!!Reset Gse decoder \n ",i);
                MT_Gse_decoder_info(&g_stGseRunInfo.ts_gse_decoder,__LINE__);
                memset(&g_stGseRunInfo.ts_gse_decoder, 0, sizeof( data_decoder));
                continue;
            }
        }

        if(g_stGseRunInfo.ts_gse_decoder.start_parse_flag == 1)
        {
            memcpy(bbframe, &(g_stGseRunInfo.ts_gse_decoder.bbh[0]), 10);
            memcpy(bbframe+10, &(g_stGseRunInfo.ts_gse_decoder.dfl_data[0]), g_stGseRunInfo.ts_gse_decoder.bbheader_data.dfl_len);
            parse_len = MT_Gseparse_bbframe(bbframe, p_ts_stream, (g_stGseRunInfo.ts_gse_decoder.bbheader_data.dfl_len+10));
            if(g_stGseRunInfo.error_happen_flag == 1)
            {
                SAMPLE_GSE_INFO_PRINT("[%s %d] i:%d \n",__FUNCTION__,__LINE__,i);
                MT_Gsedump_data("ts_packet2", &p_ts_packet[0], 32);
            }
            out_length += parse_len;
        }

    }
    if (bbframe != MT_NULL_PTR)
    {
        free(bbframe);
    }
    return out_length;
}


static mt_void *MT_Gsegse_iperf_thread(mt_void *args)
{
    mt_s32 ret = 0;
    mt_u32 count = 0;
    mt_u32 parsed_len = 0;
    MT_UNF_DMX_REC_DATA_S RecData;

    MT_U8 *data_out = MT_NULL_PTR;
    MT_U8 *remain_data_buf = MT_NULL_PTR;

    mt_u32 start_time_ms,cost_time_ms,cost_time = 0;
    mt_u32 data_size = 0;
    mt_u32 data_size_kb = 0;
    mt_u32 output_data_size = 0;
    mt_u32 output_data_size_kb = 0;
    mt_u32 valid_data_size = 0;
    mt_u32 valid_data_size_kb = 0;


    data_out = (MT_U8 *)malloc(sizeof(mt_u8)*(RECORD_BUFF_SIZE+4096));
    if (MT_NULL_PTR == data_out)
    {
        SAMPLE_GSE_ERR_PRINT("[%s %d]kzalloc priv error\n", __FUNCTION__, __LINE__);
        return NULL;
    }
    remain_data_buf = (MT_U8 *)malloc(sizeof(mt_u8)*(RECORD_BUFF_SIZE+4096));
    if (MT_NULL_PTR == remain_data_buf)
    {
        SAMPLE_GSE_ERR_PRINT("[%s %d]kzalloc priv error\n", __FUNCTION__, __LINE__);
        free(data_out);
        return NULL;
    }

    start_time_ms = MT_Gseget_systime();
    while (!g_bTaskQuit)
    {
        cost_time_ms = MT_Gseget_systime()- start_time_ms;
        if(cost_time_ms > 10000) //1//10s
        {
            data_size_kb = data_size/1000;
            valid_data_size_kb = valid_data_size/1000;
            output_data_size_kb = output_data_size/1000;
            cost_time = cost_time_ms/1000;
            SAMPLE_GSE_PRINT("\nInput data:%d byte| input valid data:%d byte| output data:%d byte in %d ms\n",data_size,valid_data_size,output_data_size,cost_time_ms);
            SAMPLE_GSE_PRINT("Input rate :%d Kbyte/s,%d Kbps\n",data_size_kb/cost_time,((data_size_kb*8)/cost_time));
            SAMPLE_GSE_PRINT("Input valid rate :%d Kbyte/s,%d Kbps\n",valid_data_size_kb/cost_time,((valid_data_size_kb*8)/cost_time));
            SAMPLE_GSE_PRINT("Output rate :%d Kbyte/s,%d Kbps\n",output_data_size_kb/cost_time,((output_data_size_kb*8)/cost_time));
            SAMPLE_GSE_PRINT("g_stGseRunInfo.gse_packet_total:%d,g_stGseRunInfo.gse_packet_err_total:%d \n",g_stGseRunInfo.gse_packet_total,g_stGseRunInfo.gse_packet_err_total);
            data_size = 0;
            output_data_size = 0;
            valid_data_size = 0;
            start_time_ms = MT_Gseget_systime();
        }

        ret = MT_UNF_DMX_AcquireRecData(g_stGseRunInfo.gse_handle, &RecData, 2000);
        if (MT_SUCCESS != ret)
        {
            count++;
            if (MT_ERR_DMX_TIMEOUT == ret)
            {
                if(count == 100)
                {
                    SAMPLE_GSE_ERR_PRINT("MT_UNF_DMX_AcquireRecData  MT_ERR_DMX_TIMEOUT %d times!!\n",count);
                    count = 0;
                }
                continue;
            }
            if (MT_ERR_DMX_NOAVAILABLE_DATA == ret)
            {
                if(count == 100)
                {
                    SAMPLE_GSE_ERR_PRINT("MT_UNF_DMX_AcquireRecData  MT_ERR_DMX_NOAVAILABLE_DATA %d times!!\n",count);
                    count = 0;
                }
                continue;
            }
            SAMPLE_GSE_ERR_PRINT("[%s] MT_UNF_DMX_AcquireRecData failed 0x%x\n", __FUNCTION__, ret);
            break;
        }


        memcpy(remain_data_buf,RecData.pDataAddr,RecData.u32Len);
        valid_data_size += RecData.u32Len;
        data_size = valid_data_size;
		if(g_sInputParam.package_mode==PROCESS_IN_BBFRAME_MODE)
        	parsed_len =  MT_Gseparse_ts_packet(remain_data_buf, data_out, RecData.u32Len);
		else if (g_sInputParam.package_mode==PROCESS_IN_GSE_MODE)
			parsed_len =  MT_Gseparse_ts_packet_in_gse_mode(remain_data_buf, data_out, RecData.u32Len);
        output_data_size += parsed_len;

        ret = MT_UNF_DMX_ReleaseRecData(g_stGseRunInfo.gse_handle, &RecData);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("[%s] MT_UNF_DMX_ReleaseRecData failed 0x%x\n", __FUNCTION__, ret);
            break;
        }
        MT_USLEEP(5000);

    }
    if(data_out != MT_NULL_PTR)
    {
        free(data_out);
    }

    if(remain_data_buf != MT_NULL_PTR)
    {
        free(remain_data_buf);
    }


	return NULL;

}


static MT_VOID MT_GsePrint_Menu(void)
{

#ifdef MT_SAMPLE_APP
    SAMPLE_GSE_PRINT("     b : background run \n");
#endif
    SAMPLE_GSE_PRINT("     h : help \n");
    SAMPLE_GSE_PRINT("     q : quit \n");
    SAMPLE_GSE_PRINT("gseiperf>> ");

}

static MT_VOID MT_GseExit(void)
{
    SAMPLE_GSE_FUNCTION_ENTER();

    (MT_VOID)pthread_join(g_stGseRunInfo.ges_thread, NULL);

    (MT_VOID)MT_GseDmxDeInit();


    (MT_VOID)MTADP_Fe_DeInit(TUNER_ID_0);

    g_bTaskQuit = MT_TRUE;

    SAMPLE_GSE_FUNCTION_EXIT();
}


/*!
@brief Help information.
@param[in]  name            Enter the value
@return::void
@*/
static void MT_GsePrint_help(char *name)
{
    SAMPLE_GSE_PRINT(" [ options ]...\n"
       "\n"
       "Options:\n"
       " ?/-h/-H        prmt_s32 this help\n"
       " -t <tuner_id>  set tuner id 0/1: 0 is in tuner, 1 is out tuner \n"
       " -f <freq.M>    set freq (3000~4200) \n"
       " -s <srate.K>   set srate default:27500\n"
       " -k <22k>       set 22k on/off:0 is off, 1 is on\n"
       " -p <polar>     0/1:0 is the horizontal polarization,1 is vertically polarized\n"
       " -m <sig_type>  0/1/2:0 is dvbs, 1 is dvbs2, 2 is dvbs_auto\n"
       " -a <package_mode> 0/1:  0 is GSE MODE 1 is BBFRAME MODE\n"
       " -o <pcap file source>  0/1:0 is iperf_udp_test.pcap, 1 is iperf_udp_45Mbps.pcap\n");
    SAMPLE_GSE_PRINT("example: %s -t 0 -f 4000 -s 35000 -k 0 -p 0 -m 2 -a 0 -o 0 \n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_GSE_PRINT("         %s -q  <exit> \n", name);
#endif
}

static void MT_GseCmdTask(void)
{
    MT_CHAR                 inputCmd[32] = { 0 };


    while(1)
    {
        (MT_VOID)MT_GsePrint_Menu();
        fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);

        if('q' == inputCmd[0])
        {
            SAMPLE_GSE_INFO_PRINT("prepare to exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }

#ifdef MT_SAMPLE_APP
        else if('b' == inputCmd[0])
        {
            SAMPLE_GSE_INFO_PRINT("Dvbs play in back!\n");
            break;
        }
#endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_GSE_INFO_PRINT("Prmt_s32 help info \n");
            continue;
        }
    }
}

/*!
@brief gets the external input parameters.
@param[in]  argc            The number of external input parameters
@param[in]  argv            External input parameter values
@return::void
@*/
static mt_s32 MT_GseParase_args(mt_s32 argc, char *argv[], mt_input_Dvbs_para_t *pInutParam)
{
    mt_s32 opt = 0;

    SAMPLE_GSE_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, "?hHf:s:k:t:p:m:a:o:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (void)MT_GsePrint_help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_GseExit();
                }
                return MT_TASK_EXIT;

            case 'f':
                pInutParam->freq = strtol(mt_optarg, 0, 0);
                break;

            case 's':
                pInutParam->sym_rate = strtol(mt_optarg, 0, 0);
                break;

            case 'k':
                pInutParam->onoff_22k = strtol(mt_optarg, 0, 0);
                break;

            case 'm':
                pInutParam->dvbs_type = strtol(mt_optarg, 0, 0);
                break;

            case 'p':
                pInutParam->polar = strtol(mt_optarg, 0, 0);
                break;

            case 't':
                pInutParam->tuner_id = strtol(mt_optarg, 0, 0);
                break;
            case 'a':
                pInutParam->package_mode = strtol(mt_optarg, 0, 0);
                break;
			case 'o':
                pInutParam->source_id = strtol(mt_optarg, 0, 0);
                break;
            default:
                (void)MT_GsePrint_help(argv[0]);
                return MT_FAILURE;
            break;
        }
    }
    SAMPLE_GSE_FUNCTION_EXIT();
    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_GseiperfMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    MT_S32                  ret = 0;
    
    MT_UNF_DMX_REC_ATTR_S   RecAttr = {0};
    MT_U8 ts_gs_mode = 0x02;

	memset(&g_stGseRunInfo, 0, sizeof(MT_GSE_RUN_INFO));
	memset(&g_sInputParam, 0, sizeof(mt_input_Dvbs_para_t));

    if(argc < 13 && g_bTaskQuit == MT_TRUE)
    {
        (MT_VOID)MT_GsePrint_help(argv[0]);
        return MT_SUCCESS;
    }
    ret = MT_GseParase_args(argc, argv, &g_sInputParam);
    if (MT_FAILURE == ret)
    {
        SAMPLE_GSE_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_GSE_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
        ret = MT_GseCheckParam(&g_sInputParam);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("MT_GseCheckParam failed.\n");
            return MT_FAILURE;
        }

#ifndef MT_SAMPLE_APP

        ret = mt_sys_init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("mt_sys_init error. ret=0x%x \n", ret);
            return MT_FAILURE;
        }

#endif

        /** Tuner initialization, Set the default parameters for tuner */
        ret = MTADP_Fe_Init(g_sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("MTADP_Fe_Init failed, ret = %x\n", ret);
            goto ERR1;
        }

        ret = MTADP_Fe_Connect_Dvbs(g_sInputParam.tuner_id, g_sInputParam.freq, g_sInputParam.sym_rate, g_sInputParam.onoff_22k, g_sInputParam.polar, g_sInputParam.dvbs_type);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("MTADP_Fe_Connect_Dvbs error\n");
            goto ERR2;
        }

        g_stGseRunInfo.last_gse_packet_id = 0xFFFF;

        ret = mt_unf_fe_set_gs_package_mode(g_sInputParam.tuner_id, &g_sInputParam.package_mode);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("mt_unf_fe_set_gs_package_mode error\n");
            goto ERR2;
        }
	   printf("TS Data packet in %s \n",(g_sInputParam.package_mode == PROCESS_IN_GSE_MODE)? "GSE Mode":"BBFrame Mode");
       printf("source_file:%s \n",(g_sInputParam.source_id == 1)? "iperf_udp_45Mbps.pcap":"iperf_udp_test.pcap");
#ifdef CONFIG_MT_CHIP_SYMPHONY6
        mt_unf_fe_get_s2_bbheader_ts_gse_mode(g_sInputParam.tuner_id,&ts_gs_mode);
        switch(ts_gs_mode){
            case BBHEADER_TRANSPORT_MODE:
                printf("[%s] BBHEADER_TRANSPORT_MODE\n", __FUNCTION__);
                break;
            case BBHEADER_GENERIC_PACKETIZED_MODE:
                printf("[%s] BBHEADER_GENERIC_PACKETIZED_MODE\n", __FUNCTION__);
                break;
            case BBHEADER_GSE_HEM_MODE:
                printf("[%s] BBHEADER_GSE_HEM_MODE \n", __FUNCTION__);
                break;
            case BBHEADER_GENERIC_CONTINUOUS_MODE:
                printf("[%s] BBHEADER_GENERIC_CONTINUOUS_MODE \n", __FUNCTION__);
                break;
            default:
                printf("[%s]ERR GET ts gs failed!!!\n", __FUNCTION__);
                break;

        }
#endif

        ret = mt_unf_fe_set_s2_set_gse_attr(g_sInputParam.tuner_id, ts_gs_mode);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("mt_unf_fe_set_s2_set_gse_attr error\n");
            goto ERR2;
        }

        ret = MT_GseDmxInit(g_sInputParam.tuner_id);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("MT_GseDmxInit failed, ret = %x\n", ret);
            goto ERR2;
        }

        memset(&RecAttr, 0 , sizeof(MT_UNF_DMX_REC_ATTR_S));
        RecAttr.u32DmxId = DMX_ID_0;
        RecAttr.u32RecBufSize = RECORD_BUFF_SIZE;

        RecAttr.enRecType = MT_UNF_DMX_REC_TYPE_ALL_PID;
        RecAttr.bDescramed = MT_FALSE;
        RecAttr.enIndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
        RecAttr.type_mode = DMX_FULL_TS_WITHOUT_NULL_PACKET;

        ret = MT_UNF_DMX_CreateRecChn(&RecAttr, &g_stGseRunInfo.gse_handle);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("MT_UNF_DMX_CreateRecChn failed, ret = %x\n", ret);
            goto ERR3;
        }
        MT_USLEEP(1000);

        ret = MT_UNF_DMX_StartRecChn(g_stGseRunInfo.gse_handle);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("MT_UNF_DMX_StartRecChn failed, ret = %x\n", ret);
            goto ERR3;
        }

        ret = pthread_create(&g_stGseRunInfo.ges_thread, MT_NULL, MT_Gsegse_iperf_thread, MT_NULL);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_GSE_ERR_PRINT("pthread_create failed, ret = %x\n", ret);
            goto ERR3;
        }
        g_bTaskQuit = MT_FALSE;
    }


    (void)MT_GseCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    printf("During the test,total revice %d gse packet,%d packet is error\n",g_stGseRunInfo.gse_packet_total, g_stGseRunInfo.gse_packet_err_total);
    if(g_stGseRunInfo.gse_packet_err_total == 0)
    {
        SAMPLE_GSE_INFO_PRINT("Test pass!!!!\n");
    }
    else
    {
        SAMPLE_GSE_ERR_PRINT("Test Failed!!!!\n");
    }

    (MT_VOID)pthread_join(g_stGseRunInfo.ges_thread, NULL);


ERR3:
    /** Demux module deinitialization */
    (MT_VOID)MT_GseDmxDeInit();

ERR2:
    /** Disconnect the tuner lock */
    (MT_VOID)MTADP_Fe_DeInit(g_sInputParam.tuner_id);

ERR1:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    return ret;


}

