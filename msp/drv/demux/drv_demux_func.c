/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/jiffies.h>

#include "mt_mach/irq.h"
#include <linux/mmzone.h>

#include "mt_type.h"
#include "mt_module.h"
#include "mt_drv_mmz.h"
#include "mt_drv_mem.h"
#include "mt_drv_demux.h"

//#include "mt_drv_sys.h"
#include "mt_kernel_adapt.h"

#include "mt_module_debug.h"
#include "hal_demux.h"
#include "drv_demux_reg.h"
#include "drv_demux_func.h"
#include "drv_demux_scd.h"
#include "drv_demux_index.h"
#include "drv_demux_sw.h"
#include "drv_demux_osal.h"
#include "drv_kt_if.h"
#include <asm/cacheflush.h>
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "mt_mach/chipinfo.h"
#include "asm/cacheflush.h"

#ifdef DMX_DESCRAMBLER_SUPPORT
#include "mt_drv_descrambler.h"
#include "drv_descrambler_func.h"
#endif
#include "hal_demux_regs.h"
#include <asm/div64.h>
#include "vfmw.h"
#include "drv_demux.h"

#define DMX_SECTION_SOFT_MASK_LEN   8
#define DMX_GET_REGION_ID(DmxId) ((0 == (DmxId)) ? 0 : ((4 == (DmxId)) ? 2 : 1))
#define DMX_PES_CHANNEL_MIN_SIZE 0x10000
#define DMX_PES_TSPACKET_MIN_SIZE (512*188)
#define DMX_RAM_PORT_AUTO_REGION 60
#define DMX_RAM_PORT_AUTO_STEP_1 0
#define DMX_RAM_PORT_AUTO_STEP_2 1
#define DMX_RAM_PORT_AUTO_STEP_4 2
#define DMX_RAM_PORT_AUTO_STEP_8 3

#define DMX_FQ_VID_BLOCK_SIZE 8192
#define DMX_FQ_AUD_BLOCK_SIZE 1280

#define DMX_REC_MIN_BUF_SIZE (256 * 1024)

#define DMX_REC_VID_SCD_BUF_SIZE (56 * 1024)
#define DMX_REC_AUD_SCD_BUF_SIZE (28 * 1024)

#define DMX_REC_TS_PACKETS_UNIT (4 * 188)

#define SEC_SECTION_DECS_DATA_LEN_PER_ITEM  (8)
#define SEC_SECTION_TS_DATA_LEN_PER_ITEM    (188)
#define SEC_ES_DECS_DATA_LEN_PER_ITEM       (16)

#define DMX_REC_VID_TS_PACKETS_PER_BLOCK (64 * DMX_REC_TS_PACKETS_UNIT)
#define DMX_REC_AUD_TS_PACKETS_PER_BLOCK (32 * DMX_REC_TS_PACKETS_UNIT)

#define DMX_REC_TS_WITH_TIMESTAMP_UNIT (4 * 192)
#define DMX_REC_VID_TS_WITH_TIMESTAMP_BLOCK_SIZE (64 * DMX_REC_TS_WITH_TIMESTAMP_UNIT)
#define DMX_REC_AUD_TS_WITH_TIMESTAMP_BLOCK_SIZE (32 * DMX_REC_TS_WITH_TIMESTAMP_UNIT)

#define DMX_REC_SOFT_INDEX_TIME_GAP (100) /*for scrambled stream ,we generate soft index, the min_time gap is 500ms*/
#define DMX_REC_VID_SCD_NUM_PER_BLOCK (8 * 28)
/*
fqdepth == DMX_REC_AUD_SCD_BUF_SIZE/DMX_REC_AUD_SCD_PER_BLOCK  >= 1023 , so ,the min DMX_REC_AUD_SCD_PER_BLOCK should be 28
*/
#define DMX_REC_AUD_SCD_NUM_PER_BLOCK (4 * 28)
#define DMX_REC_SCD_INVALID_CHANNEL 0x7F

#define DMX_MAX_SCR_MS (47721858) // scd wrap around value in ms: 0x100000000 / 90

#define DMX_FQ_DESC_SIZE 0x8
#define DMX_OQ_DESC_SIZE 0x10

#define MIN_MMZ_BUF_SIZE 4096

#define DMX_TS_BUFFER_EMPTY 2048
#define FQ_BP_TSO_PERSENT 20
#define DMX_SOFTPES_TSPACKLEN 188
#define DMX_INVALID_POS 0xffffffff
#define DMX_SOFTPES_TIMEOUT 500 //500ms
#define ENBALE_FILTER 1

#define TS_SAMPLE_STA_TS_LOCK      (1 << 31)
#define TS_SAMPLE_STA_SYNC_LOCK    (1 << 30)
#define TS_SAMPLE_STA_IC_ERROR     (1 << 24)
#define DMX_HW_RESET_DELAYED_TIME    (3)  //unit: second

#define ADAPTATION_AND_PAYLOAD_TS          (0x30)
#define NO_ADAPTATION_AND_PAYLOAD_TS       (0x10)
#define ADAPTATION_AND_NO_PAYLOAD_TS       (0x20)
#define NO_ADAPTATION_AND_NO_PAYLOAD_TS    (0x00)

#define DMX_GGLB_INT_TRPP5             (1 << 8)
#define DMX_GGLB_INT_TRPP10            (1 << 28)
#define DMX_GGLB_INT_PVR               (1 << 13)

#define DMX_ROUNDUP(x, y)   ((((x) + (y) - 1) / (y)) * (y))


mmz_buffer_s g_sDmxDevBuf;
DMX_DEV_OSI_S *g_pDmxDevOsi = MT_NULL;
DMX_Sub_DevInfo_S	g_strSubDevInfo[DMX_CNT]= { 0 };
mt_u32 s_u32DMXDropCnt[DMX_CHANNEL_CNT] = { 0 };
mt_u8 g_audioEsBufCleared = 0;

static mt_u32 s_u32DMXBufEop[DMX_OQ_CNT] = { 0 };
static u32 DMX_REC_CIRCLE_RANGE_UNIT = 188 * 1024;
mt_u32 g_dmx_loglevel = 0;
#ifndef CONFIG_MT_CHIP_SYMPHONY4
struct delayed_work  dmx_hw_reset_delayed_work;
#endif

//Because some resource are released in DMX_OsiDeInit, 
//but the DMX_AV_Proc_Thread and DMX_PES_Proc_Thread are still running.
//To avoid accessing the released resource, we must use semaphore.
//otherwise the kernel will panic.
struct semaphore  g_lock_av_sema;
struct semaphore  g_lock_pes_sema;
extern DMX_DEV_OSR_S g_stDmxOsr;

struct mutex g_demux_ch_mutex;

/***************************extern function *************************/
extern ulong mt_get_tsi_base(void);
extern ulong mt_get_vdec_base(void);
/***************************extern function *************************/

ulong reg_dmx_regs_base_vit = 0;
dmx_all_reg_s *g_dmx_str_reg = NULL;
void CheckVdecHang(unsigned long data);
extern u32 vdec_get_ves_rd(int ChanID);
//#ifdef CONFIG_MT_PVR
#if 0//need realize in pvr, so comment it temp		
mt_u32 PVR_GetRecNum(mt_u8 dmx_id);			//define in drv_prv_intf.c
#endif

#ifdef MT_DEMUX_PROC_SUPPORT
DMX_Proc_Global_Info_S GlobalProcInfo;
#endif

extern int sys_otp_read(u32 bit_addr, u8 len, u32 *p_result);

static MT_BOOL DMX_GET_CHIP_FUSED_FTA(void)
{
    mt_u32 val = 0;
    int ret = 0;
    ret = sys_otp_read(0x3AB0 * 8, 32, &val);
    if(ret != MT_SUCCESS){
        MT_FATAL_DEMUX("demux read otp failed!\n");
        return MT_TRUE;
    }
    MT_INFO_DEMUX(" ---read otp val=0x%x\n",val);
    return ((val & 0xFF) == 0xFA);
}

static __inline mt_u32 dmx_inl(ulong port)
{
    return *((volatile mt_u32 *)(port));
}
static __inline mt_void dmx_outl(ulong port, mt_u32 val)
{
    *((volatile mt_u32 *)(port)) = val;
}

mt_s32 MT_DRV_SYS_GetTimeStampMs(mt_u32 *pu32TimeMs)
{
    return 0;
}

#define IsDmxDevInit()                                \
    do {                                              \
    	if (!g_pDmxDevOsi) {                          \
    	    MT_FATAL_DEMUX("global is not exist!\n"); \
    	    return MT_ERR_DMX_NOT_INIT;               \
    	}                                             \
    } while (0)


extern mt_void DMX_OsrSaveEs(mt_u32 type, mt_u8 *buf, mt_u32 len, mt_u32 chnid);
static mt_void DMXOsiOQGetReadWrite(mt_u32 OQId, mt_u32 *BlockWrite, mt_u32 *BlockRead);
mt_void DMXOsiChnEsDescDataGetReadWrite(mt_u32 Id, mt_u32 *BlockWrite, mt_u32 *BlockRead);
mt_void DMXOsiChnEsDataGetReadWrite(mt_u32 Id, mt_u32 *BlockWrite, mt_u32 *BlockRead);
static mt_void DMXOsiChnSectionDataGetReadWrite(mt_u32 BufnID, mt_u32 *BlockWrite, mt_u32 *BlockRead);
static MT_BOOL DMX_OsiTsBufferPut_DmaStateEnhance(dmx_dma_config_t *p_dma_config);
static mt_s32 DMXOsiChannelWaitTimeOutForSec(DMX_ChanInfo_S *ChanInfo, mt_u32 u32TimeOutMs);
static mt_s32 DMXOsiChannelWaitTimeOutForPes(DMX_ChanInfo_S *ChanInfo, mt_u32 u32TimeOutMs);
static mt_s32 DMXOsiChannelWaitTimeOutForHWPes(DMX_ChanInfo_S *ChanInfo, mt_u32 u32TimeOutMs);
static mt_s32 DMXOsiChannelWaitTimeOutForTS(DMX_ChanInfo_S *ChanInfo, mt_u32 u32TimeOutMs);

#if !defined(CONFIG_MT_CHIP_SYMPHONY4) && !defined(CONFIG_MT_CHIP_SYMPHONY6)
static void DMX_OsiRestDmxForWeakSignal(struct work_struct *pdmx_reset_work);
#endif
static mt_u32 m_Table[256] =
{
  0x00000000, 0x04C11DB7, 0x9823B6E, 0xD4326D9, 0x130476DC, 0x17C56B6B, 0x1A864DB2,
  0x1E475005, 0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, 0x350C9B64, 0x31CD86D3,
  0x3C8EA00A, 0x384FBDBD, 0x4C11DB70, 0x48D0C6C7, 0x4593E01E, 0x4152FDA9, 0x5F15ADAC,
  0x5BD4B01B, 0x569796C2, 0x52568B75,  0x6A1936C8, 0x6ED82B7F, 0x639B0DA6, 0x675A1011,
  0x791D4014, 0x7DDC5DA3, 0x709F7B7A, 0x745E66CD,  0x9823B6E0, 0x9CE2AB57, 0x91A18D8E,
  0x95609039,0x8B27C03C, 0x8FE6DD8B, 0x82A5FB52, 0x8664E6E5,  0xBE2B5B58, 0xBAEA46EF,
  0xB7A96036,0xB3687D81, 0xAD2F2D84, 0xA9EE3033, 0xA4AD16EA, 0xA06C0B5D,  0xD4326D90,
  0xD0F37027,0xDDB056FE, 0xD9714B49, 0xC7361B4C, 0xC3F706FB, 0xCEB42022, 0xCA753D95,
  0xF23A8028,0xF6FB9D9F, 0xFBB8BB46, 0xFF79A6F1, 0xE13EF6F4, 0xE5FFEB43, 0xE8BCCD9A,
  0xEC7DD02D,0x34867077, 0x30476DC0, 0x3D044B19, 0x39C556AE, 0x278206AB, 0x23431B1C,
  0x2E003DC5,0x2AC12072,  0x128E9DCF, 0x164F8078, 0x1B0CA6A1, 0x1FCDBB16, 0x18AEB13,
  0x54BF6A4,0x808D07D, 0xCC9CDCA,  0x7897AB07, 0x7C56B6B0, 0x71159069, 0x75D48DDE, 0x6B93DDDB,
  0x6F52C06C, 0x6211E6B5, 0x66D0FB02,  0x5E9F46BF, 0x5A5E5B08, 0x571D7DD1, 0x53DC6066,
  0x4D9B3063, 0x495A2DD4, 0x44190B0D, 0x40D816BA,  0xACA5C697, 0xA864DB20, 0xA527FDF9,
  0xA1E6E04E, 0xBFA1B04B, 0xBB60ADFC, 0xB6238B25, 0xB2E29692,  0x8AAD2B2F, 0x8E6C3698,
  0x832F1041, 0x87EE0DF6, 0x99A95DF3, 0x9D684044, 0x902B669D, 0x94EA7B2A,  0xE0B41DE7,
  0xE4750050, 0xE9362689, 0xEDF73B3E, 0xF3B06B3B, 0xF771768C, 0xFA325055, 0xFEF34DE2,
  0xC6BCF05F, 0xC27DEDE8, 0xCF3ECB31, 0xCBFFD686, 0xD5B88683, 0xD1799B34, 0xDC3ABDED,
  0xD8FBA05A,  0x690CE0EE, 0x6DCDFD59, 0x608EDB80, 0x644FC637, 0x7A089632, 0x7EC98B85,
  0x738AAD5C, 0x774BB0EB,  0x4F040D56, 0x4BC510E1, 0x46863638, 0x42472B8F, 0x5C007B8A,
  0x58C1663D, 0x558240E4, 0x51435D53,  0x251D3B9E, 0x21DC2629, 0x2C9F00F0, 0x285E1D47,
  0x36194D42, 0x32D850F5, 0x3F9B762C, 0x3B5A6B9B,  0x315D626, 0x7D4CB91, 0xA97ED48,
  0xE56F0FF, 0x1011A0FA, 0x14D0BD4D, 0x19939B94, 0x1D528623,  0xF12F560E, 0xF5EE4BB9,
  0xF8AD6D60, 0xFC6C70D7, 0xE22B20D2, 0xE6EA3D65, 0xEBA91BBC, 0xEF68060B,  0xD727BBB6,
  0xD3E6A601, 0xDEA580D8, 0xDA649D6F, 0xC423CD6A, 0xC0E2D0DD, 0xCDA1F604, 0xC960EBB3,
  0xBD3E8D7E, 0xB9FF90C9, 0xB4BCB610, 0xB07DABA7, 0xAE3AFBA2, 0xAAFBE615, 0xA7B8C0CC,
  0xA379DD7B,  0x9B3660C6, 0x9FF77D71, 0x92B45BA8, 0x9675461F, 0x8832161A, 0x8CF30BAD,
  0x81B02D74, 0x857130C3,  0x5D8A9099, 0x594B8D2E, 0x5408ABF7, 0x50C9B640, 0x4E8EE645,
  0x4A4FFBF2, 0x470CDD2B, 0x43CDC09C,  0x7B827D21, 0x7F436096, 0x7200464F, 0x76C15BF8,
  0x68860BFD, 0x6C47164A, 0x61043093, 0x65C52D24,  0x119B4BE9, 0x155A565E, 0x18197087,
  0x1CD86D30, 0x29F3D35, 0x65E2082, 0xB1D065B, 0xFDC1BEC,  0x3793A651, 0x3352BBE6,
  0x3E119D3F, 0x3AD08088, 0x2497D08D, 0x2056CD3A, 0x2D15EBE3, 0x29D4F654,  0xC5A92679,
  0xC1683BCE, 0xCC2B1D17, 0xC8EA00A0, 0xD6AD50A5, 0xD26C4D12, 0xDF2F6BCB, 0xDBEE767C,
  0xE3A1CBC1, 0xE760D676, 0xEA23F0AF, 0xEEE2ED18, 0xF0A5BD1D, 0xF464A0AA, 0xF9278673,
  0xFDE69BC4,  0x89B8FD09, 0x8D79E0BE, 0x803AC667, 0x84FBDBD0, 0x9ABC8BD5, 0x9E7D9662,
  0x933EB0BB, 0x97FFAD0C,  0xAFB010B1, 0xAB710D06, 0xA6322BDF, 0xA2F33668,0xBCB4666D,
  0xB8757BDA, 0xB5365D03, 0xB1F740B4,
};
static mt_u32 CRC32(mt_u8 *buffer, mt_u32 size)
{
  mt_u32 Result = 0xFFFFFFFF;
  while (size--)
  {
    Result = (Result << 8) ^ m_Table[(Result >> 24) ^ *buffer ++];
  }
  return Result;
}
/*
static void dmx_spes_dumpts(u8 *head,u32 lens)
{
    int i;
    MT_FATAL_DEMUX("ts data:\n");
    for(i=0;i<lens;i++){
        if(!(i&7)){
            MT_FATAL_DEMUX("\n");
        }
        MT_FATAL_DEMUX("%02x ",head[i]);
    }
    MT_FATAL_DEMUX("\n");
}
static void dmx_spes_dumppes(u8 *head,u32 lens)
{
    int i;
    MT_FATAL_DEMUX("pes data:\n");
    for(i=0;i<lens;i++){
        if(!(i&7)){
            MT_FATAL_DEMUX("\n");
        }
        MT_FATAL_DEMUX("%02x ",head[i]);
    }
    MT_FATAL_DEMUX("\n");
}
*/
static void dmx_spes_resetheader(DMX_SOFT_PESHEADER_S *peshead)
{
    if (NULL != peshead)
    {
        //memset(peshead,0xff,sizeof(DMX_SOFT_PESHEADER_S)-12);
        peshead->pes_len = 0;
        peshead->ts_rp = 0;
        peshead->pes_buf_rd = 0;
        peshead->pes_buf_wr = 0;
    }
}

static mt_u32 GetQueueLenth(const mt_u32 Read, const mt_u32 Write, const mt_u32 Size)
{
    mt_u32 ret;

    if (Read > Write) {
	ret = Size + Write - Read;
    } else {
	ret = Write - Read;
    }

    return ret;
}

static mt_void DMXCheckFQBPStatus(DMX_Sub_DevInfo_S *DmxInfo, mt_u32 FQId)
{
    if (DMX_PORT_MODE_RAM == DmxInfo->PortMode) {
	mt_u32 FQ_WPtr, FQ_RPtr, ValidDes, tmp;
	DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;

	if (DmxHalGetIPBPStatus(DmxInfo->PortId)) {
	    DmxHalGetFQWORDx(FQId, DMX_FQ_SZWR_OFFSET, &tmp);
	    FQ_WPtr = tmp & 0xffff;
	    DmxHalGetFQWORDx(FQId, DMX_FQ_RDVD_OFFSET, &tmp);
	    FQ_RPtr = tmp & 0xffff;

	    if (FQ_WPtr >= FQ_RPtr) {
		ValidDes = FQ_WPtr - FQ_RPtr;
	    } else {
		ValidDes = (pDmxDevOsi->DmxFqInfo[FQId].u32FQDepth) - FQ_RPtr + FQ_WPtr;
	    }

	    if (ValidDes >= (pDmxDevOsi->DmxFqInfo[FQId].u32FQDepth / 10)) {
		DmxHalClrIPFqBPStatus(DmxInfo->PortId, FQId);
	    }
	}
    }
}

static mt_s32 DmxFlushChannel(mt_u32 ChanId, DMX_FLUSH_TYPE_E FlushType)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;

    DmxHalFlushChannel(ChanId, FlushType);

    for (i = 0; i < DMX_MAX_FLUSH_WAIT; i++) {
	if (DmxHalIsFlushChannelDone()) {
	    break;
	}
    }

    if (DMX_MAX_FLUSH_WAIT == i) {
	MT_ERR_DEMUX("flush channel %u failed\n", ChanId);

	ret = MT_FAILURE;
    }

    return ret;
}

static mt_s32 DmxFlushOq(mt_u32 OqId, DMX_OQ_CLEAR_TYPE_E ClearType)
{
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;

    DmxHalClearOq(OqId, ClearType);

    for (i = 0; i < DMX_MAX_FLUSH_WAIT; i++) {
	if (DmxHalIsClearOqDone()) {
	    break;
	}
    }

    if (DMX_MAX_FLUSH_WAIT == i) {
	MT_ERR_DEMUX("clear oq %u failed\n", OqId);

	ret = MT_FAILURE;
    }

    return ret;
}

//enable oq recive
//close channel dmx ->  disable oq -> flush dmx channel
//-> clear oq reg infor -> enable oq -> set channel dmx
static mt_s32 DMXOsiEnableOQRecive(mt_u32 ChanId, mt_u32 OqId)
{
    mt_u32 DmxId;
    mt_u32 Word;

    DmxHalGetChannelPlayDmxid(ChanId, &DmxId);
    DmxHalSetChannelPlayDmxid(ChanId, DMX_INVALID_DEMUX_ID);
    DmxHalDisableOQRecive(OqId);

    DmxFlushChannel(ChanId, DMX_FLUSH_TYPE_REC_PLAY);

    DmxHalSetOQWORDx(OqId, DMX_OQ_EOPWR_OFFSET, 0);
    DmxHalGetOQWORDx(OqId, DMX_OQ_RSV_OFFSET, &Word);
    Word &= 0xffffff00;
    DmxHalSetOQWORDx(OqId, DMX_OQ_RSV_OFFSET, Word);
    DmxHalGetOQWORDx(OqId, DMX_OQ_CTRL_OFFSET, &Word);
    Word &= 0x80;
    DmxHalSetOQWORDx(OqId, DMX_OQ_CTRL_OFFSET, Word);

    DmxHalEnableOQRecive(OqId);
    DmxHalSetChannelPlayDmxid(ChanId, DmxId);

    return 0;
}

static mt_s32 ResetOQ(mt_u32 u32DmxId, DMX_OQ_Info_S *pstOQInfo)
{
    mt_u32 i, u32PvrCtrl;
    mt_u32 u32WriteBlk, u32OqRead, u32OqWrite;
    ulong u32CurrentBlk;
		mt_u32 u32BufPhyAddr,u32BlkSize, u32BlkNum;
    mt_s32 s32OQEnableStatus;
    FQ_DescInfo_S *FQ_Desc;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_FQ_Info_S *FqInfo;
    mt_size_t u32LockFlag;

    IsDmxDevInit();

    s32OQEnableStatus = DmxHalGetOQEnableStatus(pstOQInfo->u32OQId);
    DmxHalDisableOQRecive(pstOQInfo->u32OQId);
    DMXOsiOQGetReadWrite(pstOQInfo->u32OQId, &u32OqWrite, &u32OqRead);

    if (!pDmxDevOsi->DmxFqInfo[pstOQInfo->u32FQId].u32FQVirAddr) {
	MT_ERR_DEMUX("fq not valid \n");

	DMXOsiEnableOQRecive(pstOQInfo->u32AttachId, pstOQInfo->u32OQId);
	return MT_FAILURE;
    }

    FqInfo = &pDmxDevOsi->DmxFqInfo[pstOQInfo->u32FQId];

    spin_lock_irqsave(&FqInfo->LockFq, u32LockFlag);

    u32WriteBlk = DmxHalGetFQWritePtr(pstOQInfo->u32FQId);

    u32BlkNum = GetQueueLenth(u32OqRead, u32OqWrite, pstOQInfo->u32OQDepth);
    for (i = 0; i < u32BlkNum; i++) {
	u32CurrentBlk = pstOQInfo->u32OQVirAddr + u32OqRead * DMX_OQ_DESC_SIZE;
	u32BufPhyAddr = *(mt_u32 *)u32CurrentBlk;
	u32BlkSize = *(mt_u32 *)(u32CurrentBlk + 4) & 0xffff;
	u32CurrentBlk = pDmxDevOsi->DmxFqInfo[pstOQInfo->u32FQId].u32FQVirAddr + u32WriteBlk * DMX_FQ_DESC_SIZE;
	FQ_Desc = (FQ_DescInfo_S *)u32CurrentBlk;
	FQ_Desc->start_addr = u32BufPhyAddr;
	FQ_Desc->buflen = u32BlkSize;
	DMXINC(u32WriteBlk, pDmxDevOsi->DmxFqInfo[pstOQInfo->u32FQId].u32FQDepth);
	DMXINC(u32OqRead, pstOQInfo->u32OQDepth);
    }

    DmxHalGetOQWORDx(pstOQInfo->u32OQId, DMX_OQ_CTRL_OFFSET, &u32PvrCtrl);
    if (u32PvrCtrl & DMX_MASK_BIT_7) {
	DmxHalGetOQWORDx(pstOQInfo->u32OQId, DMX_OQ_SADDR_OFFSET, &u32BufPhyAddr);
	DmxHalGetOQWORDx(pstOQInfo->u32OQId, DMX_OQ_SZUS_OFFSET, &u32BlkSize);
	u32BlkSize >>= 16;
	u32CurrentBlk = pDmxDevOsi->DmxFqInfo[pstOQInfo->u32FQId].u32FQVirAddr + u32WriteBlk * DMX_FQ_DESC_SIZE;
	FQ_Desc = (FQ_DescInfo_S *)u32CurrentBlk;
	FQ_Desc->start_addr = u32BufPhyAddr;
	FQ_Desc->buflen = u32BlkSize;

	DMXINC(u32WriteBlk, pDmxDevOsi->DmxFqInfo[pstOQInfo->u32FQId].u32FQDepth);
    }

    DmxHalSetFQWritePtr(pstOQInfo->u32FQId, u32WriteBlk);

    DMXCheckFQBPStatus(&pDmxDevOsi->SubDevInfo[u32DmxId], pstOQInfo->u32FQId); //CLR BP STATUS

    spin_unlock_irqrestore(&FqInfo->LockFq, u32LockFlag);

    pstOQInfo->u32ProcsBlk = 0;
    pstOQInfo->u32ProcsOffset = 0;
    pstOQInfo->u32ReleaseBlk = 0;
    pstOQInfo->u32ReleaseOffset = 0;

    DmxHalSetOQWORDx(pstOQInfo->u32OQId, DMX_OQ_RSV_OFFSET, 0);
    DmxHalSetOQWORDx(pstOQInfo->u32OQId, DMX_OQ_CTRL_OFFSET, 0);
    DmxHalSetOQWORDx(pstOQInfo->u32OQId, DMX_OQ_EOPWR_OFFSET, 0);
    DmxHalSetOQWORDx(pstOQInfo->u32OQId, DMX_OQ_SZUS_OFFSET, 0);
    DmxHalSetOQWORDx(pstOQInfo->u32OQId, DMX_OQ_SADDR_OFFSET, 0);
    DmxHalSetOQWORDx(pstOQInfo->u32OQId, DMX_OQ_RDWR_OFFSET, 0);

    //reset oq ts counter
    DmxHalResetOqCounter(pstOQInfo->u32OQId);
    if (s32OQEnableStatus) {
	DMXOsiEnableOQRecive(pstOQInfo->u32AttachId, pstOQInfo->u32OQId);
    }

    return MT_SUCCESS;
}

static mt_void DmxFqIdRelease(mt_u32 FqId)
{
    mt_size_t u32LockFlag;
    if (FqId < DMX_FQ_CNT) {
	DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
	DMX_FQ_Info_S *FqInfo = &DmxMgr->DmxFqInfo[FqId];
	spin_lock_irqsave(&FqInfo->LockFq, u32LockFlag);
	FqInfo->u32IsUsed = 0;
	spin_unlock_irqrestore(&FqInfo->LockFq, u32LockFlag);
    }
}

static mt_void DmxFqStart(mt_u32 FqId)
{
//Modify for MMZ 64M
#if defined(CONFIG_MT_CHIP_SYMPHONY1) || defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    MT_INFO_DEMUX("Warning, function not implement!\n");
#else
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_FQ_Info_S *FqInfo = &DmxMgr->DmxFqInfo[FqId];
    FQ_DescInfo_S *FqDesc = (FQ_DescInfo_S *)FqInfo->u32FQVirAddr;
    mt_u32 PhyAddr = FqInfo->u32BufPhyAddr;
    mt_u32 i;

    for (i = 0; i < FqInfo->u32FQDepth - 1; i++) {
	FqDesc->start_addr = PhyAddr;
	FqDesc->buflen = FqInfo->u32BlockSize;

	PhyAddr += (FqInfo->u32BlockSize + DMX_BUF_INTERVAL);

	++FqDesc;
    }

    FqDesc->start_addr = 0;
    FqDesc->buflen = 0;
//mondify  by yuwu

//DmxHalSetFQWORDx(FqId, DMX_FQ_CTRL_OFFSET, DMX_FQ_ALOVF_CNT << 24);
//DmxHalSetFQWORDx(FqId, DMX_FQ_RDVD_OFFSET, 0);
//DmxHalSetFQWORDx(FqId, DMX_FQ_SZWR_OFFSET, (FqInfo->u32FQDepth << 16) | ((FqInfo->u32FQDepth - 1) & 0xffff));
//DmxHalSetFQWORDx(FqId, DMX_FQ_START_OFFSET, FqInfo->u32FQPhyAddr);

#ifdef MT_DEMUX_PROC_SUPPORT
    FqInfo->FqOverflowCount = 0;

//DmxHalFQSetOverflowInt(FqId, DMX_ENABLE);
#endif

    //DmxHalFQEnableRecive(FqId, DMX_ENABLE);
#endif
}

static mt_void DmxFqStop(mt_u32 FqId)
{
    DmxHalFQEnableRecive(FqId, DMX_DISABLE);

#ifdef MT_DEMUX_PROC_SUPPORT
    DmxHalFQSetOverflowInt(FqId, DMX_DISABLE);
#endif

    DmxHalSetFQWORDx(FqId, DMX_FQ_CTRL_OFFSET, 0);
    DmxHalSetFQWORDx(FqId, DMX_FQ_RDVD_OFFSET, 0);
    DmxHalSetFQWORDx(FqId, DMX_FQ_SZWR_OFFSET, 0);
    DmxHalSetFQWORDx(FqId, DMX_FQ_START_OFFSET, 0);
}


#ifdef ParsePes
static mt_s32 ParsePesHeader(
    DMX_ChanInfo_S *pChanInfo,
    mt_u32 u32ParserAddr,
    mt_u32 PESLength,
    mt_u32 *pPESHeadLen,
    Disp_Control_t *pDispController)
{
    mt_u8 *pData;
    mt_u32 PTSFlag;
    mt_u32 PesHeaderLength;

    mt_u32 PTSLow, StreamId;

    pData = (mt_u8 *)(u32ParserAddr);
    if ((pData[0] != 0x00) || (pData[1] != 0x00) || (pData[2] != 0x01)) {
	MT_WARN_DEMUX("pes start byte =0x%x%x%x\n", pData[2], pData[1], pData[0]);
	return MT_FAILURE;
    }

    /* Get StreamId */
    StreamId = pData[3];

    //del pes header according to 13818-1
    if ((StreamId != 0xbc)     //1011 1100  1   program_stream_map
        && (StreamId != 0xbe)  //1011 1110       padding_stream
        && (StreamId != 0xbf)  //1011 1111   3   private_stream_2
        && (StreamId != 0xf0)  //1111 0000   3   ECM_stream
        && (StreamId != 0xf1)  //1111 0001   3   EMM_stream
        && (StreamId != 0xff)  //1111 1111   4   program_stream_directory
        && (StreamId != 0xf2)  //1111 0010   5   ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Annex A or ISO/IEC 13818-6_DSMCC_stream
        && (StreamId != 0xf8)) //1111 1000  6   ITU-T Rec. H.222.1 type E
    {
	/* if (PTS_DTS_flags =='10' ) {
            '0010'  4   bslbf
            PTS [32..30]    3   bslbf
            marker_bit  1   bslbf
            PTS [29..15]    15  bslbf
            marker_bit  1   bslbf
            PTS [14..0] 15  bslbf
            marker_bit  1   bslbf
        }
        if (PTS_DTS_flags =='11' )  {
            '0011'
            PTS [32..30]
            marker_bit
            PTS [29..15]
            marker_bit
            PTS [14..0]
            marker_bit
            '0001'
            DTS [32..30]
            marker_bit
            DTS [29..15]
            marker_bit
            DTS [14..0]
            marker_bit
        }
         */
	PTSFlag = (pData[7] & 0x80) >> 7;
	PesHeaderLength = pData[8];

	*pPESHeadLen = DMX_PES_HEADER_LENGTH + PesHeaderLength;

	if (PTSFlag && (PESLength > *pPESHeadLen)) {
	    PTSLow = (((pData[9] & 0x06) >> 1) << 30) | ((pData[10]) << 22) | (((pData[11] & 0xfe) >> 1) << 15) | ((pData[12]) << 7) | (((pData[13] & 0xfe)) >> 1);
	    if (pData[9] & 0x08) {
		pChanInfo->LastPts = (PTSLow / 90) + 0x2D82D83; //(1 << 32) / 90 ,使用科学计算器计算结果是0x2D82D83 (0x2D82D83);
	    } else {
		pChanInfo->LastPts = (PTSLow / 90);
	    }
	} else {
	    pChanInfo->LastPts = INVALID_PTS;
	}
	//add by yangchun 201302234
	if ((0xee == StreamId) && (PESLength > 24)) {
	    if ((0x70 == pData[13]) && (0x76 == pData[14]) && (0x72 == pData[15]) && (0x63 == pData[16])) {
		if ((0x75 == pData[17]) && (0x72 == pData[18]) && (0x74 == pData[19]) && (0x6d == pData[20])) {
		    pDispController->u32DispTime = *(mt_u32 *)&pData[21];
		    pDispController->u32DispEnableFlag = *(mt_u32 *)&pData[25];
		    pDispController->u32DispFrameDistance = *(mt_u32 *)&pData[29];
		    pDispController->u32DistanceBeforeFirstFrame = *(mt_u32 *)&pData[33];
		    pDispController->u32GopNum = *(mt_u32 *)&pData[37];
		    *pPESHeadLen = 184;
		    return -2;
		}
	    }
	}

	return MT_SUCCESS;
    } else {
	MT_ERR_DEMUX("streamID:%x did not del the pes header!\n", StreamId);
	return MT_FAILURE;
    }
}
#endif

static mt_s32 DMXCheckBuffer(ulong u32BufStartAddr,
                             mt_u32 u32BufLen,
                             mt_u32 u32Offset)
{
    mt_u32 u32SecLen, u32BufLenTmp;
		ulong u32PhyAddr;

    u32BufLenTmp = u32BufLen - u32Offset;
    while (u32BufLenTmp > 3) {
	u32SecLen = 3;
	u32PhyAddr = u32BufStartAddr + u32Offset;
	if (u32BufLen < u32Offset) {
	    MT_ERR_DEMUX("invalide offset,u32BufLen:%d,u32Offset:%d!\n", u32BufLen, u32Offset);
	    return -1;
	}

	u32SecLen += (((*(mt_u8 *)(u32PhyAddr + 1)) & 0xF) << 8);
	u32SecLen += *(mt_u8 *)(u32PhyAddr + 2);
	if ((u32SecLen > u32BufLenTmp) || (u32SecLen > DMX_MAX_SEC_LEN)) {
	    MT_ERR_DEMUX("invalid u32SecLen, u32SecLen:%d,u32BufLenTmp:%d!\n", u32SecLen, u32BufLenTmp);
	    return -1;
	}

	u32Offset += u32SecLen;
	u32BufLenTmp -= u32SecLen;
    }

    return 0;
}

mt_u32 GetSectionLength(ulong u32BufStartAddr, mt_u32 u32BufLen, mt_u32 u32Offset)
{
    mt_u32 u32PacketLen = 0;
    ulong u32BufAddr, u32EndAddr;

    if (0 == u32BufStartAddr) {
	MT_ERR_DEMUX("Buffer start addr is 0!\n");
	return 0;
    }

    if (0 == u32BufLen) {
	MT_ERR_DEMUX("Buffer len is 0!\n");
	return 0;
    }

    if (u32Offset >= u32BufLen) {
	MT_ERR_DEMUX("u32Offset is larger than buflen,%x!\n", u32Offset);
	return 0;
    }

    if (DMXCheckBuffer(u32BufStartAddr, u32BufLen, u32Offset) != 0) {
	return 0;
    }

    if (u32BufLen >= 3) {
	u32PacketLen = 3;
	u32BufAddr = u32BufStartAddr + u32Offset;
	u32EndAddr = u32BufStartAddr + u32BufLen;
	if (u32BufAddr >= u32EndAddr) {
	    MT_ERR_DEMUX("u32Offset is larger than buflen!\n");
	    return 0;
	}

	u32PacketLen += (((*(mt_u8 *)(u32BufAddr + 1)) & 0xF) << 8);
	if (u32BufAddr >= u32EndAddr) {
	    MT_ERR_DEMUX("u32Offset is larger than buflen!\n");
	    return 0;
	}

	u32PacketLen += *(mt_u8 *)(u32BufAddr + 2);
	if ((u32BufLen - u32Offset) < u32PacketLen) {
	    u32PacketLen = 0;
	}
    }

    return u32PacketLen;
}

mt_s32 DMX_OsiResetChannel(mt_u32 ChanId, DMX_FLUSH_TYPE_E eFlushType)
{
    DMX_DEV_OSI_S *pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo;
    DMX_OQ_Info_S *OqInfo;
    mt_s32 s32OQEnableStatus;

    IsDmxDevInit();
    pDmxDevOsi = g_pDmxDevOsi;

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    OqInfo = &pDmxDevOsi->DmxOqInfo[ChanInfo->ChanOqId];

    if (DMX_FLUSH_TYPE_REC & eFlushType) {
	DmxFlushChannel(ChanId, DMX_FLUSH_TYPE_REC);

	/*if only close rec channel and flush rec*/
	if (DMX_FLUSH_TYPE_REC == eFlushType) {
	    return MT_SUCCESS;
	}
    }

    s32OQEnableStatus = DmxHalGetOQEnableStatus(ChanInfo->ChanOqId);
    DmxHalDisableOQRecive(ChanInfo->ChanOqId);
    DmxFlushChannel(ChanId, eFlushType);
    DmxFlushOq(ChanInfo->ChanOqId, DMX_OQ_CLEAR_TYPE_PLAY);

    if (s32OQEnableStatus) {
	DmxHalEnableOQRecive(ChanInfo->ChanOqId);
    }

    ResetOQ(ChanInfo->DmxId, OqInfo);

    DMXCheckFQBPStatus(&pDmxDevOsi->SubDevInfo[ChanInfo->DmxId], OqInfo->u32FQId);

    //DMXCheckOQEnableStatus(pChanInfo->stChBuf.u32OQId, pChanInfo->stChBuf.u32OQDepth);
    return MT_SUCCESS;
}


static mt_s32 DMXOsiFindPes(DMX_ChanInfo_S *pChanInfo,
                            DMX_UserMsg_S *psMsgList,
                            mt_u32 u32AcqNum,
                            mt_u32 *pu32AcqedNum);

static DMX_SOFT_PES_NODE_S * DMX_OsiParsePes_new_node(mt_u32 ChanId)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = NULL;
    DMX_SOFT_PESHEADER_S *peshead = NULL;
    DMX_SOFT_PES_NODE_S *p_pes_node = NULL;
    mt_u8 *pes_buf = NULL;

    if (ChanId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("%s, %d, ChanId:0x%x!\n", __func__, __LINE__, ChanId);
        return p_pes_node;
    }

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    peshead = ChanInfo->softpeshead;
    pes_buf = peshead->pesdata;

    if (!list_empty_careful(&peshead->pes_free_node_list))
    {
        /*
        use a new pes node and del it from pes_free_node_list, then add it to pes_used_node_list
        */
        p_pes_node = list_entry(peshead->pes_free_node_list.next, DMX_SOFT_PES_NODE_S, pes_node);
        list_del_init(&p_pes_node->pes_node);
        list_add_tail(&p_pes_node->pes_node, &peshead->pes_used_node_list);
        p_pes_node->pes_node_len = 0;
        p_pes_node->pes_node_merged = 0;
        p_pes_node->pes_node_addr = (ulong)(pes_buf + peshead->pes_buf_wr);
    }
    else
    {
        MT_DBG_DEMUX("no free pes node\n");
    }

    return p_pes_node;

}


static DMX_SOFT_PES_NODE_S * DMX_OsiParsePes_get_current_node(mt_u32 ChanId)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = NULL;
    DMX_SOFT_PESHEADER_S *peshead = NULL;
    DMX_SOFT_PES_NODE_S *p_pes_node = NULL;

    if (ChanId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("ChanId:0x%x!\n", ChanId);
        return p_pes_node;
    }

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    peshead = ChanInfo->softpeshead;

    if (!list_empty_careful(&peshead->pes_used_node_list))
    {
	    p_pes_node = list_entry(peshead->pes_used_node_list.prev, DMX_SOFT_PES_NODE_S, pes_node);   //the latest node is to be used
    }
    else
    {
        MT_DBG_DEMUX("pes_used_node_list empty, %d\n", ChanInfo->ChanPid);
	}

    return p_pes_node;
}


static mt_s32 DMX_OsiParsePes_copy_data_to_node(DMX_ChanInfo_S *chan_info, DMX_SOFT_PES_NODE_S *p_pes_node, 
	mt_u8 *pes_data, mt_u32 pes_len)
{
    DMX_SOFT_PESHEADER_S *peshead = NULL;
    mt_u8 *p_node_data = NULL;
    mt_u32 free_size = 0;
    mt_u32 pid = 0x1fff;
    mt_s32 ret = MT_SUCCESS;

    peshead = chan_info->softpeshead;
    pid = chan_info->ChanPid;

    /*
              rd                      wr
       -----------------------------------------
       | | | |*|*|*|*|*|*|*|*|*|*|*|*| | | | | |
       -----------------------------------------
    */
	if (peshead->pes_buf_wr > peshead->pes_buf_rd)
    {
        free_size = peshead->pesphysiz - peshead->pes_buf_wr;

        if (free_size >= pes_len)
        {
            p_node_data = (mt_u8 *)(p_pes_node->pes_node_addr + p_pes_node->pes_node_len);

            memcpy(p_node_data, pes_data, pes_len);
            p_pes_node->pes_node_len += pes_len;
            peshead->pes_buf_wr += pes_len;
        }
        else
        {
        	MT_INFO_DEMUX("[%s %d]free_size=%d, p_pes_node->pes_node_len=%d, pes_len=%d, \n peshead->pes_buf_wr=%d, peshead->pes_buf_rd=%d\r\n", __FUNCTION__, __LINE__, free_size, p_pes_node->pes_node_len, pes_len, peshead->pes_buf_wr, peshead->pes_buf_rd);
            free_size = peshead->pes_buf_rd;

            /*
            if the free size is not enough for curent pes data, drv should check
            wether the size before pes_buf_rd is enough for p_pes_node->pes_node_len
            + pes_len, because drv should move all the data of current pes node to
            the start of pes buf, the purpose of that is to put a pes packet in a
            continuous memory space.
            */
            if (free_size >= (p_pes_node->pes_node_len + pes_len))
            {
                memcpy(peshead->pesdata, (mt_u8 *)p_pes_node->pes_node_addr, p_pes_node->pes_node_len);
                p_pes_node->pes_node_addr = (ulong)peshead->pesdata;
                p_node_data = (mt_u8 *)(p_pes_node->pes_node_addr + p_pes_node->pes_node_len);
                memcpy(p_node_data, pes_data, pes_len);
                p_pes_node->pes_node_len += pes_len;
                peshead->pes_buf_wr = p_pes_node->pes_node_len;
				MT_INFO_DEMUX("[%s %d]peshead->pes_buf_wr=%d\r\n", __FUNCTION__, __LINE__, peshead->pes_buf_wr);
            }
            else
            {
                /*
                if there is only one pes node in pes used list, and no memory 
                to put new data in, and pes_buf_wr > pes_buf_rd, that must be
                the reveived pes data length > pes_buf_rd, so the free memory 
                at the start of pes buf can not put the reveived pes data, that
                happens when parse video pes packet.
    			*/
                if ((p_pes_node->pes_node.next == &peshead->pes_used_node_list) &&
                    (p_pes_node->pes_node.prev == &peshead->pes_used_node_list) &&
                    (peshead->pes_buf_rd != 0) &&
                    ((p_pes_node->pes_node_len + pes_len) <= peshead->pesphysiz))
                {
                    memcpy(peshead->pesdata, (mt_u8 *)p_pes_node->pes_node_addr, p_pes_node->pes_node_len);
                    p_pes_node->pes_node_addr = (ulong)peshead->pesdata;
                    p_node_data = (mt_u8 *)(p_pes_node->pes_node_addr + p_pes_node->pes_node_len);
                    memcpy(p_node_data, pes_data, pes_len);
                    p_pes_node->pes_node_len += pes_len;
                    peshead->pes_buf_wr = p_pes_node->pes_node_len;
                    peshead->pes_buf_rd = 0;
                }
                else
                {
					//MT_ERR_DEMUX("[%s %d]p_pes_node->pes_node_len=%d, pes_len=%d, \n peshead->pesphysiz=%d, mem not enough for cur pes node\r\n", __FUNCTION__, __LINE__, p_pes_node->pes_node_len, pes_len, peshead->pesphysiz);
                    ret = MT_ERR_DMX_NOAVAILABLE_BUF;
                }
            }

        }
    }
    /*
                            wr        rd
       -----------------------------------------
       |*|*|*|*|*|*|*|*|*|*| | | | | |*|*|*|*|*|
       -----------------------------------------
    */
    else if (peshead->pes_buf_wr < peshead->pes_buf_rd)
    {
        free_size = peshead->pes_buf_rd - peshead->pes_buf_wr;

        if (free_size >= pes_len)
        {
            p_node_data = (mt_u8 *)(p_pes_node->pes_node_addr + p_pes_node->pes_node_len);

            memcpy(p_node_data, pes_data, pes_len);
            p_pes_node->pes_node_len += pes_len;
            peshead->pes_buf_wr += pes_len;
        }
        else
        {
        	MT_INFO_DEMUX("[%s %d]free_size=%d, p_pes_node->pes_node_len=%d, pes_len=%d, \n peshead->pes_buf_wr=%d, peshead->pes_buf_rd=%d\r\n", __FUNCTION__, __LINE__, free_size, p_pes_node->pes_node_len, pes_len, peshead->pes_buf_wr, peshead->pes_buf_rd);
            /*
            if there is only one pes node in pes used list, and no memory to put
            new data in, and pes_buf_rd > pes_buf_wr, that must be the case that
            current node at the start of pes buf, so pes_buf_rd should set to 0,
            if not, pes data could not put to pes buf for ever, that case happens
            when parse video pes packet.
			*/
            if ((p_pes_node->pes_node.next == &peshead->pes_used_node_list) &&
                (p_pes_node->pes_node.prev == &peshead->pes_used_node_list) &&
                (p_pes_node->pes_node_addr == (ulong)peshead->pesdata))
            {
                peshead->pes_buf_rd = 0;
            }

            //MT_ERR_DEMUX("[%s %d]mem not enough for cur pes node\n", __FUNCTION__, __LINE__);
            //MT_ERR_DEMUX("[%s %d]p_pes_node->pes_node_len=%d, pes_len=%d, peshead->pesphysiz=%d, \n peshead->pes_buf_rd=%d, mem not enough for cur pes node\r\n", __FUNCTION__, __LINE__, p_pes_node->pes_node_len, pes_len, peshead->pesphysiz, peshead->pes_buf_rd);
            ret = MT_ERR_DMX_NOAVAILABLE_BUF;
        }
    }
    else
    {
    	if (!list_empty_careful(&peshead->pes_used_node_list))
	    {
	        p_pes_node = list_entry(peshead->pes_used_node_list.next, DMX_SOFT_PES_NODE_S, pes_node);

	        if (0 == p_pes_node->pes_node_len)
	    	{
		    	MT_INFO_DEMUX("***************************************************************\r\n");
		    	MT_INFO_DEMUX("[%s %d]peshead->pes_buf_wr=%d, peshead->pes_buf_rd=%d, \n p_pes_node=0x%x, addr=0x%x, len=%d, pen_len=%d, merged=%d\r\n", __FUNCTION__, __LINE__, peshead->pes_buf_wr, peshead->pes_buf_rd, p_pes_node, p_pes_node->pes_node_addr, p_pes_node->pes_node_len, pes_len, p_pes_node->pes_node_merged);
		        MT_INFO_DEMUX("***************************************************************\r\n");
				
		        free_size = peshead->pesphysiz - peshead->pes_buf_wr;

		        if (free_size >= pes_len)
		        {
		            p_node_data = (mt_u8 *)(p_pes_node->pes_node_addr + p_pes_node->pes_node_len);

		            memcpy(p_node_data, pes_data, pes_len);
		            p_pes_node->pes_node_len += pes_len;
		            peshead->pes_buf_wr += pes_len;
		        }
		        else
		        {
		            /*
		            if the free size is not enough for curent pes data, drv should check
		            wether the size before pes_buf_rd is enough for p_pes_node->pes_node_len
		            + pes_len, because drv should move all the data of current pes node to
		            the start of pes buf, the purpose of that is to put a pes packet in a
		            continuous memory space
		            */
		            memcpy(peshead->pesdata, (mt_u8 *)p_pes_node->pes_node_addr, p_pes_node->pes_node_len);
		            p_pes_node->pes_node_addr = (ulong)peshead->pesdata;
		            p_node_data = (mt_u8 *)(p_pes_node->pes_node_addr + p_pes_node->pes_node_len);
		            memcpy(p_node_data, pes_data, pes_len);
		            p_pes_node->pes_node_len += pes_len;
		            peshead->pes_buf_wr = p_pes_node->pes_node_len;
				}
	    	}
			else
			{
				//MT_ERR_DEMUX("[%s %d]peshead->pes_buf_wr=%d, peshead->pes_buf_rd=%d\r\n", __FUNCTION__, __LINE__, peshead->pes_buf_wr, peshead->pes_buf_rd);
				//MT_ERR_DEMUX("[%s %d]mem not enough for cur pes node\r\n", __FUNCTION__, __LINE__, p_pes_node->pes_node_len, pes_len, peshead->pesphysiz, peshead->pes_buf_rd);
            	ret = MT_ERR_DMX_NOAVAILABLE_BUF;
			}
    	}
    }

    return ret;
}


mt_s32 DMX_OsiParsePes(mt_u32 ChanId)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = NULL;
    DMX_ChanSecBuff_S *sec_buf = NULL;
    DMX_SOFT_PESHEADER_S *peshead = NULL;
    mt_u32 ts_buf_id = 0;
    mt_u32 ts_buf_size = 0;    
    mt_u8 *ts_buf = NULL;
    mt_u32 ts_wp = 0;
    mt_u32 ts_rp = 0;
    mt_u8 conti = 0;
    mt_u8 *pdata = NULL;
    mt_u8 adapt_len = 0;
    mt_u8 *pes_data = NULL;
    mt_u8 adapt_field = 0;
    DMX_SOFT_PES_NODE_S *p_pes_node = NULL;
    mt_u32 cur_ts_pes_len = 0;
    mt_s32 ret = MT_SUCCESS;

    if (ChanId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("ChanId:%d!\n", ChanId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];

    if (NULL == ChanInfo)
    {
        MT_ERR_DEMUX("chan info invalid\n");
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (MT_UNF_DMX_CHAN_TYPE_PES == ChanInfo->ChanType)
    {
        ts_buf_id = ChanInfo->secBuffId;

        if (DMX_INVALID_BUF_ID == ts_buf_id)
        {
            return MT_ERR_DMX_INVALID_PARA;
        }

        sec_buf = &pDmxDevOsi->DmxChanSecBuff[ts_buf_id];
        ts_buf = (mt_u8 *)sec_buf->sec_data_buff.startVirAddr;

        if (!ts_buf)
        {
            MT_DBG_DEMUX("%s, %d, ts_buf is null, pid = %d\n", ChanInfo->ChanPid);
            return MT_ERR_DMX_NULL_PTR;
        }

        ts_buf_size = (reg_get_bufn_size_data_ch_size(ts_buf_id) << 10);
        peshead = ChanInfo->softpeshead;

        if (!peshead)
        {
            MT_DBG_DEMUX("%s, %d, peshead is null, pid = %d\n", ChanInfo->ChanPid);
            return MT_ERR_DMX_NULL_PTR;
        }

        ts_wp = reg_get_bufn_data_wptr_data_ch_wptr(ts_buf_id);
        ts_wp = ts_wp / DMX_SOFTPES_TSPACKLEN * DMX_SOFTPES_TSPACKLEN;
        ts_rp = peshead->ts_rp;

        while (ts_wp != ts_rp)
        {
            pdata = ts_buf + ts_rp;

            if (0x47 == pdata[0])
            {            	
                conti = (pdata[3] & 0xf);

                if (conti != peshead->nextpktid)
                {
                    MT_INFO_DEMUX("ts conti field err, cur = %d, expect = %d - %d - %d\n",
                        conti, peshead->nextpktid, ts_rp, current->pid);
                }
    
                /*
                *check adaptation filed
                */
                adapt_field = (pdata[3] & 0x30);
                /*
                *adaptation and payload
                */
                switch (adapt_field)
                {
                    case ADAPTATION_AND_PAYLOAD_TS:
                        adapt_len = pdata[4] + 1;
                    break;
					
                    case NO_ADAPTATION_AND_PAYLOAD_TS:
                        adapt_len = 0;
                    break;
					
                    case ADAPTATION_AND_NO_PAYLOAD_TS:
                        peshead->nextpktid = ((conti + 1) & 0xf);
                        ts_rp += DMX_SOFTPES_TSPACKLEN;
			
                        if (ts_rp >= ts_buf_size)
                        {
                            ts_rp = 0;
                        }

                        MT_DBG_DEMUX("ts packet with adaptation, no payload!!!\n");
                        continue;
                    break;
					
                    case NO_ADAPTATION_AND_NO_PAYLOAD_TS:
                        peshead->nextpktid = ((conti + 1) & 0xf);
                        ts_rp += DMX_SOFTPES_TSPACKLEN;
			
                        if (ts_rp >= ts_buf_size)
                        {
                            ts_rp = 0;
                        }

                        MT_DBG_DEMUX("ts packet with no adaptation no payload!!!\n");
                        continue;
                    break;
                    default:
                        MT_ERR_DEMUX("never happen!!!\n");
                    break;
                }

                if (adapt_len >= (DMX_SOFTPES_TSPACKLEN - 4))
                {
                    MT_INFO_DEMUX("adapt field error, [%x][%x][%x][%x][%x][%x][%x][%x][%x][%x] - [%d]\n",
                        pdata[0], pdata[1],pdata[2], pdata[3],pdata[4], pdata[5],pdata[6], pdata[7],pdata[8], pdata[9], adapt_len);

                    peshead->nextpktid = ((conti + 1) & 0xf);
                    ts_rp += DMX_SOFTPES_TSPACKLEN;

                    if (ts_rp >= ts_buf_size)
                    {
                        ts_rp = 0;
                    }

                    continue;
                }

                pes_data = pdata + 4 + adapt_len;
                p_pes_node = DMX_OsiParsePes_get_current_node(ChanId);
                /*
                *check wether a new pes head in current ts packet
                */
                if (pdata[1] & 0x40)
                {                    	
					MT_INFO_DEMUX("[%02x %02x %02x %02x %02x %02x %02x %02x %02x %02x %02x]\r\n", 
									pdata[0], pdata[1], pdata[2], pdata[3], pdata[4], pdata[5], 
									pdata[6], pdata[7], pdata[8], pdata[9], pdata[10]);											
                	
                    if (p_pes_node)
                    {
                    	//if the p_pes_node->pes_node_len not equal to 0
                    	//it indicates the node's data has been stored to buf.
                    	if (p_pes_node->pes_node_len != 0)
                    	{                    		
	                    	//receive a new pes, so we think the pre-pes has parsed completely,
	                    	//and set the pes_node_merged to 1.
	                        p_pes_node->pes_node_merged = 1;
							ChanInfo->pes_wait_flag = 1;
							MT_INFO_DEMUX("[%s %d]pre pes finished\r\n", __FUNCTION__, __LINE__);
                    	}
                    }

					/* If there is a node, but the pes_node_len is 0, don't get a new node.
					*  The reason is: if there is no mem to store this packet, the ts_rp will not change.
					*  when come here again, it is still the original packet, so need not get a new node.
					*  Otherwise a new node will be used to store data, but the pre node don't have data,
					*  DMX_OsiGetPesChnDataFlag will always return no data because p_pes_node->pes_node_len is 0.
					*/
					if ((NULL == p_pes_node) || (p_pes_node->pes_node_len != 0))
					{
	                    p_pes_node = DMX_OsiParsePes_new_node(ChanId);//get a new node to store the new pes.					
	                    if (!p_pes_node)
	                    {
	                        peshead->ts_rp = ts_rp;
	                        sec_buf->data_buff_write_p = peshead->ts_rp;
	                        reg_set_bufn_size_data_ch_rptr(ChanInfo->secBuffId, (peshead->ts_rp >> 10));
							MT_INFO_DEMUX("p_pes_node==NULL\n");
	                        return MT_ERR_DMX_NULL_PTR;
	                    }
						
	                    /*
	                    *check wether the pes start code is match
	                    */
	                    if ((0 != pes_data[0]) || (0 != pes_data[1]) || (1 != pes_data[2])) 
	                    {
	                        MT_INFO_DEMUX("pes start code [%#x][%#x][%#x] field error\n",
	                            pes_data[0], pes_data[1], pes_data[2]);
	                    }

	                    peshead->pes_len = ((pes_data[4] << 8) | pes_data[5]);
						
						MT_INFO_DEMUX("[%s %d]p_pes_node=0x%x, pes_node_addr=0x%x, peshead->pes_len=%d\n", __FUNCTION__, __LINE__, p_pes_node, p_pes_node->pes_node_addr, peshead->pes_len);

	                    if (0 == peshead->pes_len)
	                    {
	                        MT_WARN_DEMUX("pes packet length field is 0, that maybe, ChanId = %d\n", ChanId);
	                    }    
					}
                }
                else
                {
                    if (p_pes_node)
                    {
                        /*
                        *Low probability event
                        */
                        if (p_pes_node->pes_node_merged)
                        {
                            MT_WARN_DEMUX("Low probability event, %#x - %d\n", 
                                p_pes_node->pes_node_addr, p_pes_node->pes_node_len);

                            p_pes_node = DMX_OsiParsePes_new_node(ChanId);

                            if (!p_pes_node)
                            {
                                peshead->ts_rp = ts_rp;
                                sec_buf->data_buff_write_p = peshead->ts_rp;
                                reg_set_bufn_size_data_ch_rptr(ChanInfo->secBuffId, (peshead->ts_rp >> 10));
								MT_INFO_DEMUX("p_pes_node==NULL\n");
                                return MT_ERR_DMX_NULL_PTR;
                            }
                        }
                    }
                    else
                    {
                        p_pes_node = DMX_OsiParsePes_new_node(ChanId);
						
                        if (!p_pes_node)
                        {
                            peshead->ts_rp = ts_rp;
                            sec_buf->data_buff_write_p = peshead->ts_rp;
                            reg_set_bufn_size_data_ch_rptr(ChanInfo->secBuffId, (peshead->ts_rp >> 10));
							MT_INFO_DEMUX("p_pes_node==NULL\n");
                            return MT_ERR_DMX_NULL_PTR;
                        }
						MT_INFO_DEMUX("[%s %d]p_pes_node=0x%x, pes_node_addr=0x%x\r\n", __FUNCTION__, __LINE__, p_pes_node, p_pes_node->pes_node_addr);
                    }
                }


                cur_ts_pes_len = DMX_SOFTPES_TSPACKLEN - adapt_len - 4;
                ret = DMX_OsiParsePes_copy_data_to_node(ChanInfo, p_pes_node, pes_data, cur_ts_pes_len);
                if (MT_SUCCESS != ret)
                {
                    MT_INFO_DEMUX("ts_rp=0x%x, copy pes data to node Err, ret = %#x\n", ts_rp, ret);
                    peshead->ts_rp = ts_rp;
                    sec_buf->data_buff_write_p = peshead->ts_rp;
                    reg_set_bufn_size_data_ch_rptr(ChanInfo->secBuffId, (peshead->ts_rp >> 10));
                    return ret;
                }				
				
				if ((peshead->pes_len + 6) == p_pes_node->pes_node_len)
                {
                	MT_INFO_DEMUX("[%s %d]finished, p_pes_node->pes_node_len=%d\n", __FUNCTION__, __LINE__, p_pes_node->pes_node_len);
                    p_pes_node->pes_node_merged = 1;
					ChanInfo->pes_wait_flag = 1;
                }

            }
            else
            {
                MT_ERR_DEMUX("ts packet sync code field error\n");
            }

            peshead->nextpktid = ((conti + 1) & 0xf);
            ts_rp += DMX_SOFTPES_TSPACKLEN;

            if (ts_rp >= ts_buf_size)
            {
                ts_rp = 0;
            }
        }

        peshead->ts_rp = ts_rp;
        sec_buf->data_buff_write_p = peshead->ts_rp;
        reg_set_bufn_size_data_ch_rptr(ChanInfo->secBuffId, (peshead->ts_rp >> 10));
    }
    else
    {
        MT_ERR_DEMUX("%s, %d, error ChanType!!!\n", __func__, __LINE__);
        return MT_ERR_DMX_INVALID_PARA;
    }

    return ret;
}


mt_s32 DMX_OsiGetSecChnDataFlag(DMX_ChanInfo_S *ChanInfo)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanSecBuff_S *ChanBuff = NULL;
    mt_u32 discWP = 0;

    if (NULL == ChanInfo)
    {
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (ChanInfo->secBuffId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("invalid secBuffId = %#x\n", ChanInfo->secBuffId);
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    ChanBuff = &pDmxDevOsi->DmxChanSecBuff[ChanInfo->secBuffId];
    discWP = reg_get_bufn_disc_wptr_disc_ch_wptr(ChanInfo->secBuffId);

    if (ChanBuff->desc_buff_write_p != discWP)
    {
        MT_DBG_DEMUX("Sec pid = %d, hw_wp = %d, sf_wp = %d\n", ChanInfo->ChanPid,
            discWP, ChanBuff->desc_buff_write_p);
        return MT_SUCCESS;
    }
    else
    {
        MT_DBG_DEMUX("Sec pid = %d, hw_wp = %d, sf_wp = %d\n", ChanInfo->ChanPid,
            discWP, ChanBuff->desc_buff_write_p);
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }
}


mt_s32 DMX_OsiGetTsChnDataFlag(DMX_ChanInfo_S *ChanInfo)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanSecBuff_S *ChanBuff = NULL;
    mt_u32 dataWP = 0;

    if (NULL == ChanInfo)
    {
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (ChanInfo->secBuffId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("invalid secBuffId = %#x\n", ChanInfo->secBuffId);
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    ChanBuff = &pDmxDevOsi->DmxChanSecBuff[ChanInfo->secBuffId];
    dataWP = reg_get_bufn_data_wptr_data_ch_wptr(ChanInfo->secBuffId);
	
    if (ChanBuff->data_buff_write_p != dataWP)
    {
        MT_DBG_DEMUX("Sec pid = %d, hw_wp = %d, sf_wp = %d\n", ChanInfo->ChanPid,
            dataWP, ChanBuff->data_buff_write_p);
        return MT_SUCCESS;
    }
    else
    {
        MT_DBG_DEMUX("Sec pid = %d, hw_wp = %d, sf_wp = %d\n", ChanInfo->ChanPid,
            dataWP, ChanBuff->data_buff_write_p);
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }
}


mt_s32 DMX_OsiGetHwPesChnDataFlag(DMX_ChanInfo_S *ChanInfo)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanEsBuff_S *esBuff = NULL;
    mt_u32 desc_wr = 0;
    mt_u32 desc_rd = 0;
    mt_u32 desc_cnt = 0;
    mt_u32 desc_buf_size = 0;
    mt_u32 desc_data_size = 0;
    mt_u8 pes_chnum = 0;

    pes_chnum = ChanInfo->avChanId;
    desc_rd = reg_get_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(pes_chnum);
    desc_wr = reg_get_trpp_ch_dscrpt_wr_addr_trpp_ch_dscrpt_waddr(pes_chnum);

    esBuff = &pDmxDevOsi->DmxChanEsBuff[pes_chnum];
    desc_buf_size = esBuff->desc_buff.size;

    if (desc_wr >= desc_rd)
    {
        desc_data_size = desc_wr - desc_rd;
    }
    else
    {
        desc_data_size = desc_wr + desc_buf_size - desc_rd;
    }

    desc_cnt = desc_data_size / SEC_ES_DECS_DATA_LEN_PER_ITEM;

    /*
    *the descriptors needs a minimum of 2 before drv read pes parsed by HW,
    *because the diff of 2 descriptor's data addr is the length of the pre pes
	*/
    if (desc_cnt > 1)
    {
        MT_DBG_DEMUX("HWPES pid = %d, wr = %d, rd = %d, pesid = %d\n", ChanInfo->ChanPid,
            desc_wr, desc_rd, pes_chnum);
        return MT_SUCCESS;
    }
    else
    {
        MT_DBG_DEMUX("HWPES pid = %d, wr = %d, rd = %d, pesid = %d\n", ChanInfo->ChanPid,
            desc_wr, desc_rd, pes_chnum);
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }
}


mt_s32 DMX_OsiGetPesChnDataFlag(DMX_ChanInfo_S *ChanInfo)
{
    DMX_SOFT_PES_NODE_S *p_pes_node = NULL;

    mutex_lock(&ChanInfo->chan_mutex);

    if (!ChanInfo->softpeshead)
    {        
        mutex_unlock(&ChanInfo->chan_mutex);
        printk(KERN_ERR "peshead is null\n");
        return MT_ERR_DMX_NULL_PTR;
    }

    if (!list_empty_careful(&ChanInfo->softpeshead->pes_used_node_list))
    {
        p_pes_node = list_entry(ChanInfo->softpeshead->pes_used_node_list.next, DMX_SOFT_PES_NODE_S, pes_node);

        if ((p_pes_node->pes_node_len) && (p_pes_node->pes_node_addr) && (p_pes_node->pes_node_merged))
        {
            mutex_unlock(&ChanInfo->chan_mutex);
            return MT_SUCCESS;
        }
        else
        {
            mutex_unlock(&ChanInfo->chan_mutex);
            return MT_ERR_DMX_NOAVAILABLE_DATA;
        }
    }
    else
    {
        mutex_unlock(&ChanInfo->chan_mutex);
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }
}


mt_s32 DMX_OsiGetChnDataFlag(mt_u32 ChanId)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo;
	mt_s32 ret = MT_SUCCESS;

    if (ChanId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("invalid ChanId = %#x\n", ChanId);
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];

    switch (ChanInfo->ChanType)
    {
        case MT_UNF_DMX_CHAN_TYPE_SEC:
        case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
        {
            ret = DMX_OsiGetSecChnDataFlag(ChanInfo);
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_POST:
        {
            ret = DMX_OsiGetTsChnDataFlag(ChanInfo);
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_HW_PES:
        {
            ret = DMX_OsiGetHwPesChnDataFlag(ChanInfo);
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            ret = DMX_OsiGetPesChnDataFlag(ChanInfo);
            break;
        }

        default:
        {
            MT_ERR_DEMUX("Get type:%d flag not allowed, chanid:%d\n",
                ChanInfo->ChanType, ChanInfo->ChanId);
            ret = MT_ERR_DMX_INVALID_PARA;
            break;
        }
    }  	

    return ret;
}


mt_s32 DMX_OsiCheckChnDataFlag(mt_u32 ChanId, mt_u32 u32TimeOutMs)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo;
	mt_s32 ret = MT_SUCCESS;

    if (ChanId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("invalid ChanId = %#x\n", ChanId);
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];

    switch (ChanInfo->ChanType)
    {
        case MT_UNF_DMX_CHAN_TYPE_SEC:
        case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
        {
            ret = DMX_OsiGetSecChnDataFlag(ChanInfo);

            if (MT_SUCCESS == ret)
            {
                break;        
            }

            if (0 == u32TimeOutMs)
            {
                break;
            }

            ret = DMXOsiChannelWaitTimeOutForSec(ChanInfo, u32TimeOutMs);

            if (-ERESTARTSYS == ret)
            {
                MT_DBG_DEMUX("wait sec interrupted by signal, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            if (0 == ret)
            {
                MT_DBG_DEMUX("wait sec time out, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            ret = MT_SUCCESS;
            
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_POST:
        {
            ret = DMX_OsiGetTsChnDataFlag(ChanInfo);

            if (0 == u32TimeOutMs)
            {
                break;
            }

            ret = DMXOsiChannelWaitTimeOutForTS(ChanInfo, u32TimeOutMs);

            if (-ERESTARTSYS == ret)
            {
                MT_DBG_DEMUX("wait ts interrupted by signal, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            if (0 == ret)
            {
                MT_DBG_DEMUX("wait ts time out, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            ret = MT_SUCCESS;

            break;
        }
		
		case MT_UNF_DMX_CHAN_TYPE_VID:
		case MT_UNF_DMX_CHAN_TYPE_DSS:
        case MT_UNF_DMX_CHAN_TYPE_HW_PES:
        {
            ret = DMX_OsiGetHwPesChnDataFlag(ChanInfo);

            if (MT_SUCCESS == ret)
            {
                break;
            }

            if (0 == u32TimeOutMs)
            {
                break;
            }

            ret = DMXOsiChannelWaitTimeOutForHWPes(ChanInfo, u32TimeOutMs);

            if (-ERESTARTSYS == ret)
            {
                MT_DBG_DEMUX("wait hw pes interrupted by signal, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            if (0 == ret)
            {
                MT_DBG_DEMUX("wait hw pes time out, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            ret = MT_SUCCESS;

            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            ret = DMX_OsiGetPesChnDataFlag(ChanInfo);

            if (MT_SUCCESS == ret)
            {
                break;
            }

            ret = DMXOsiChannelWaitTimeOutForPes(ChanInfo, u32TimeOutMs);

            if (-ERESTARTSYS == ret)
            {
                MT_DBG_DEMUX("wait pes interrupted by signal, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            if (0 == ret)
            {
                MT_DBG_DEMUX("wait pes time out, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            ret = MT_SUCCESS;

            break;
        }

        default:
        {
            MT_ERR_DEMUX("Get type:%d flag not allowed, chanid:%d\n",
                ChanInfo->ChanType, ChanInfo->ChanId);
            ret = MT_ERR_DMX_INVALID_PARA;
            break;
        }
    }  	

    return ret;
}

static mt_void DMXOsiChnSectionDataGetReadWrite(mt_u32 Id, mt_u32 *BlockWrite, mt_u32 *BlockRead)
{
    if (BlockWrite)
    {
        *BlockWrite = reg_get_bufn_data_wptr_data_ch_wptr(Id);
    }

    if (BlockRead)
    {
        *BlockRead = reg_get_bufn_size_data_ch_rptr(Id) * 1024;
    }
}

static mt_void DMXOsiChnSectionDescDataGetReadWrite(mt_u32 Id, mt_u32 *BlockWrite, mt_u32 *BlockRead)
{
    if (BlockWrite)
    {
        *BlockWrite = reg_get_bufn_disc_wptr_disc_ch_wptr(Id);
    }

    if (BlockRead)
    {
        *BlockRead = reg_get_bufn_size_disc_ch_rptr(Id) * 1024;
    }
}

mt_void DMXOsiChnEsDescDataGetReadWrite(mt_u32 Id, mt_u32 *BlockWrite, mt_u32 *BlockRead)
{
    if (BlockRead)
    {
        *BlockRead = reg_get_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(Id);
    }

    if (BlockWrite)
    {
        *BlockWrite = reg_get_trpp_ch_dscrpt_wr_addr_trpp_ch_dscrpt_waddr(Id);
    }
}
EXPORT_SYMBOL(DMXOsiChnEsDescDataGetReadWrite);

mt_void DMXOsiChnEsDataGetReadWrite(mt_u32 Id, mt_u32 *BlockWrite, mt_u32 *BlockRead)
{
    /* FIX: 由于OS/CPU的读寄存器时序问题,应当先读RD指针寄存器 */

    if (BlockRead)
    {
        *BlockRead = reg_get_trpp_ch_data_rd_addr_trpp_ch_data_raddr(Id);
    }

    if (BlockWrite)
    {
        *BlockWrite = reg_get_trpp_ch_data_wr_addr_trpp_ch_data_waddr(Id);
    }
}
EXPORT_SYMBOL(DMXOsiChnEsDataGetReadWrite);

static mt_void DMXOsiOQGetReadWrite(mt_u32 OQId, mt_u32 *BlockWrite, mt_u32 *BlockRead)
{
    if (BlockWrite)
    {
        *BlockWrite = reg_get_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(OQId);
    }

    if (BlockRead)
    {
        *BlockRead = reg_get_trpp_ch_rec_rd_addr_trpp_ch_rec_raddr(OQId);
    }
}

static mt_void DMXOsiIndexGetReadWrite(mt_u32 Index, mt_u32 *BlockWrite, mt_u32 *BlockRead)
{
    if (BlockWrite)
    {
        *BlockWrite = reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(Index);
    }

    if (BlockRead)
    {
        *BlockRead = reg_get_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(Index);
    }
}
mt_s32 DMXOsiChnEsDescBufGetAddrSize(mt_u32 Id, ulong *EsAddr, ulong *EsSize, ulong *DescAddr, ulong *DescSize)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanEsBuff_S *esBuff = NULL;

    if (!EsAddr || !EsSize || !DescAddr || !DescSize)
    {
        return MT_ERR_DMX_INVALID_PARA;
    }

    esBuff = &pDmxDevOsi->DmxChanEsBuff[Id];

    if (esBuff)
    {
        *EsAddr = (ulong)esBuff->data_buff.startVirAddr;
        *EsSize = esBuff->data_buff.size;
        *DescAddr = (ulong)esBuff->desc_buff.startVirAddr;
        *DescSize = esBuff->desc_buff.size;
    }

    return MT_SUCCESS;
}
EXPORT_SYMBOL(DMXOsiChnEsDescBufGetAddrSize);

static mt_s32 DMXOsiFindPes(DMX_ChanInfo_S *pChanInfo,
                            DMX_UserMsg_S *psMsgList,
                            mt_u32 u32AcqNum, /*u32AcqNum = DMX_PES_MAX_SIZE*/
                            mt_u32 *pu32AcqedNum)
{
    mt_u32 u32DataLen;
    mt_u32 u32CurBlkNum;
    mt_u32 u32DescWriteBlk, u32DescReadBlk; //, u32CurReadBlk;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanEsBuff_S *EsBuffer = &pDmxDevOsi->DmxChanEsBuff[pChanInfo->avChanId];

    DMXOsiChnEsDescDataGetReadWrite(pChanInfo->avChanId, &u32DescWriteBlk, &u32DescReadBlk);
    if (u32DescReadBlk == u32DescWriteBlk) {
	u32CurBlkNum = 0;
    } else if (u32DescWriteBlk > u32DescReadBlk) {
	u32DataLen = u32DescWriteBlk - u32DescReadBlk;
	u32CurBlkNum = u32DataLen / SEC_ES_DECS_DATA_LEN_PER_ITEM;
    } else {
	mt_u32 u32DescBuffSize = EsBuffer->desc_buff.size;
	u32DataLen = u32DescBuffSize - u32DescReadBlk + u32DescWriteBlk;
	u32CurBlkNum = u32DataLen / SEC_ES_DECS_DATA_LEN_PER_ITEM;
    }

    if (u32CurBlkNum > 0) {
	mt_u32 i = 0;
	mt_u8 *desc_data = (mt_u8 *)(EsBuffer->desc_buff.startVirAddr + u32DescReadBlk);
	mt_u32 u32EsWritep = 0;
	mt_u32 u32EsReadp = 0;
	mt_u32 u32EsDataLen = 0;
	mt_u32 u32EsDataTotalLen = 0;

	DMXOsiChnEsDataGetReadWrite(pChanInfo->avChanId, &u32EsWritep, &u32EsReadp);
	if (u32EsWritep >= u32EsReadp)
	    u32EsDataLen = u32EsWritep - u32EsReadp;
	else
	    u32EsDataLen = EsBuffer->data_buff.size - u32EsReadp + u32EsWritep;
	if (u32CurBlkNum > u32AcqNum)
	    u32CurBlkNum = u32AcqNum;

	for (i = 0; i < u32CurBlkNum; i++) {
	    mt_u8 *temp = desc_data;
	    mt_u32 data_addr = 0;
	    if ((temp[0] & 0x1) == 1) //check pes_head
	    {
		data_addr = (temp[i * SEC_ES_DECS_DATA_LEN_PER_ITEM + 7] << 24) | (temp[i * SEC_ES_DECS_DATA_LEN_PER_ITEM + 6] << 16) | (temp[i * SEC_ES_DECS_DATA_LEN_PER_ITEM + 5] << 8) | (temp[i * SEC_ES_DECS_DATA_LEN_PER_ITEM + 4]);
		psMsgList[i].u32BufStartAddr = data_addr;
		psMsgList[i].u32MsgLen = 0;
		if (i >= 1) {
		    psMsgList[i - 1].u32MsgLen = psMsgList[i].u32BufStartAddr - psMsgList[i - 1].u32BufStartAddr;
		    u32EsDataTotalLen += psMsgList[i - 1].u32MsgLen;
		}
	    }
	}
	psMsgList[u32CurBlkNum - 1].u32MsgLen = u32EsDataLen - u32EsDataTotalLen;
//	for (i = 0; i < u32CurBlkNum; i++) {
//	    MT_DBG_DEMUX("psMsgList[%d].u32BufStartAddr=0x%x,  psMsgList[j].u32MsgLen=0x%x, startPhyAddr=0x%x, startVirAddr=0x%x\n",
//	                 i, psMsgList[i].u32BufStartAddr, psMsgList[i].u32MsgLen, EsBuffer->data_buff.startPhyAddr, EsBuffer->data_buff.startVirAddr);
//	}
	*pu32AcqedNum = u32CurBlkNum;
	reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(pChanInfo->avChanId, u32DescWriteBlk);
	reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(pChanInfo->avChanId, u32EsWritep);
	pChanInfo->u32PesBlkCnt += u32CurBlkNum;

	return MT_SUCCESS;
    }

    return MT_FAILURE;
}


static mt_s32 DMXOsiIsr_IntGlb(int irq, mt_void *devId)
{
    mt_u32 reg = 0;
    mt_u32 glb_reg = 0;

    glb_reg = reg_get_dmx_gglb_int_state();
    MT_DBG_DEMUX("%s, %d, glb_reg = %#x\n", __func__, __LINE__, glb_reg);

    /*
    record buf overflow
    */
    if (glb_reg & DMX_GGLB_INT_TRPP5)
    {
        reg = reg_get_trpp5_int_state();

        if (0x100 == (reg & 0x100))   //rec0 near full
        {
            g_pDmxDevOsi->DmxRecInfo[0].rec_data_overflow_cnt++;
            MT_FATAL_DEMUX("record 0 near full\n");
        }
        else if (0x200 == (reg & 0x200))   //rec1 near full
        {
            g_pDmxDevOsi->DmxRecInfo[1].rec_data_overflow_cnt++;
            MT_FATAL_DEMUX("record 1 near full\n");
        }
        else if (0x400 == (reg & 0x400))   //rec2 near full
        {
            g_pDmxDevOsi->DmxRecInfo[2].rec_data_overflow_cnt++;
            MT_FATAL_DEMUX("record 2 near full\n");
        }
        else if (0x800 == (reg & 0x800))   //rec3 near full
        {
            g_pDmxDevOsi->DmxRecInfo[3].rec_data_overflow_cnt++;
            MT_FATAL_DEMUX("record 3 near full\n");
        }
		else if (0x1000 == (reg & 0x1000))   //rec0 index overflow
        {
            g_pDmxDevOsi->DmxRecInfo[0].rec_index_overflow_cnt++;
            MT_FATAL_DEMUX("index 0 near full!\n");
        }
        else if (0x2000 == (reg & 0x2000))   //rec1 index near full
        {
            g_pDmxDevOsi->DmxRecInfo[1].rec_index_overflow_cnt++;
            MT_FATAL_DEMUX("index 1 near full!\n");
        }
        else if (0x4000 == (reg & 0x4000))   //rec2 index near full
        {
            g_pDmxDevOsi->DmxRecInfo[2].rec_index_overflow_cnt++;
            MT_FATAL_DEMUX("index 2 near full!\n");
        }
        else if (0x8000 == (reg & 0x8000))   //rec3 index near full
        {
            g_pDmxDevOsi->DmxRecInfo[3].rec_index_overflow_cnt++;
            MT_FATAL_DEMUX("index 3 near full!\n");
        }
    }
	
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	if (glb_reg & DMX_GGLB_INT_TRPP10)
	{
		reg = reg_get_trpp10_int_state();
		
        if (0x01 == (reg & 0x01))   //rec4 near full
        {
			g_pDmxDevOsi->DmxRecInfo[3].rec_data_overflow_cnt++;
            MT_FATAL_DEMUX("record 4 near full !\n");
        }
        else if (0x02 == (reg & 0x02))   //rec5 near full
        {
			g_pDmxDevOsi->DmxRecInfo[3].rec_data_overflow_cnt++;
            MT_FATAL_DEMUX("record 5 near full !\n");
        }
        else if (0x04 == (reg & 0x04))   //rec6 near full
        {
			g_pDmxDevOsi->DmxRecInfo[3].rec_data_overflow_cnt++;
            MT_FATAL_DEMUX("record 6 near full !\n");
        }       
	}
#endif

    return IRQ_HANDLED;
}

static mt_s32 DMXOsiIsr_SEC(int irq, mt_void *devId)
{
#if 1
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = NULL;
    DMX_ChanSecBuff_S *secBuff = NULL;
    mt_u8 chanId = 0;

    while (chanId < DMX_CHANNEL_CNT)
	{
		ChanInfo = &DmxMgr->DmxChanInfo[chanId];
		chanId++;
		if (ChanInfo->ChanType != MT_UNF_DMX_CHAN_TYPE_SEC) {
		    continue;
		}
		
		if (ChanInfo->ChanStatus == MT_UNF_DMX_CHAN_CLOSE) {
		    continue;
		}
		
		if (DMX_INVALID_BUF_ID != ChanInfo->secBuffId)
		{
		    mt_u32 reg = 0;
		    //MT_DBG_DEMUX("reg=0x%x, secBuff->u32IsUsed=%d\n", reg, secBuff->u32IsUsed);
		    //if (secBuff->u32IsUsed == DMX_ISUSED_FREE) {
			//continue;
		    //}
		    reg = reg_get_bufn_int_sta(ChanInfo->secBuffId);
		    //MT_DBG_DEMUX("reg=0x%x\n",reg);
		    if (reg & 0x1)
		    {
		       	secBuff = &g_pDmxDevOsi->DmxChanSecBuff[ChanInfo->secBuffId];
				s_u32DMXBufEop[ChanInfo->secBuffId] = 1;
				wake_up_interruptible(&secBuff->OqWaitQueue);
				secBuff->received_sec ++;
				MT_DBG_DEMUX("DMXOsiIsr_SEC ====> ChanInfo->pid=%d\n",ChanInfo->slot_reg0.bitc.pid);
		    }
			
		    if (reg & 0x2) {
			//printk("buf_sta[%d]: 0x%x  desc buff overflow\n", ChanInfo->secBuffId, reg);
		    }
			
		    if (reg & 0x4) {
			//printk("buf_sta[%d]: 0x%x  data buff overflow\n", ChanInfo->secBuffId, reg);
		    }
			
		    if (reg & 0x8) {
			//printk("buf_sta[%d]: 0x%x  data crc error\n", ChanInfo->secBuffId, reg);
		    }
			
		    if (reg & 0x10) {
			//printk("buf_sta[%d]: 0x%x  data short backet error\n", ChanInfo->secBuffId, reg);
		    }
			
		    if (reg & 0x20) {
			//printk("buf_sta[%d]: 0x%x  data pointer field len err\n", ChanInfo->secBuffId, reg);
		    }
			
		    if (reg & 0x40) {
			//printk("buf_sta[%d]: 0x%x  adaptation field err\n", ChanInfo->secBuffId, reg);
		    }
		}
	}
	
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    {//clear isr flag
        mt_u32 regv=HAL_GET_U32((volatile mt_u32 *)((ulong)reg_dmx_sf_in_clear));
        HAL_PUT_U32((volatile mt_u32 *)((ulong)reg_dmx_sf_in_clear),(regv|1));
        //printk("++.isr.ot %x,%x\n",reg_dmx_sf_in_clear,regv);
    }
#endif

#else
    DMX_OQ_Info_S *OqInfo = &g_pDmxDevOsi->DmxOqInfo[0];
    s_u32DMXBufEop[0] = 1;

    wake_up_interruptible(&OqInfo->OqWaitQueue);
#endif

    return IRQ_HANDLED;
}

static mt_s32 DMXOsiIsr_PVR(int irq, mt_void *devId)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &pDmxDevOsi->DmxRecInfo[0];
    DMX_RamPort_Info_S *PortInfo = NULL;

    DMX_OQ_Info_S *OqInfo = MT_NULL;

    u32 pvr_state = reg_get_pvr_int_state();

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    u32 pvr1_state = 0;
    u32 pvr2_state = reg_get_pvr2_int_state();
    pvr1_state = reg_get_pvr1_int_state();
#endif
    MT_DBG_DEMUX("pvr_state = 0x%x,  pvr1_state=0c%x, pvr_state2=0x%x\n",pvr_state,pvr1_state,pvr2_state);
    if(((pvr_state >> 1) & 0x1) == 0x1)
    {
        RecInfo = &pDmxDevOsi->DmxRecInfo[0];
        RecInfo->interrupt_count ++;
		
		OqInfo = &pDmxDevOsi->DmxOqInfo[0];
		OqInfo->OqWakeUp = MT_TRUE;
		wake_up(&OqInfo->OqWaitQueue);
    }
    if(((pvr_state >> 5) & 0x1) == 0x1)
    {
        RecInfo = &pDmxDevOsi->DmxRecInfo[1];
        RecInfo->interrupt_count ++;

		OqInfo = &pDmxDevOsi->DmxOqInfo[1];
		OqInfo->OqWakeUp = MT_TRUE;
		wake_up(&OqInfo->OqWaitQueue);
    }
    if(((pvr_state >> 9) & 0x1) == 0x1)
    {
        RecInfo = &pDmxDevOsi->DmxRecInfo[2];
        RecInfo->interrupt_count ++;

		OqInfo = &pDmxDevOsi->DmxOqInfo[2];
		OqInfo->OqWakeUp = MT_TRUE;
		wake_up(&OqInfo->OqWaitQueue);
    }
    if(((pvr_state >> 13) & 0x1) == 0x1)
    {
        RecInfo = &pDmxDevOsi->DmxRecInfo[3];
        RecInfo->interrupt_count ++;

		OqInfo = &pDmxDevOsi->DmxOqInfo[3];
		OqInfo->OqWakeUp = MT_TRUE;
		wake_up(&OqInfo->OqWaitQueue);
    }
    if(((pvr_state >> 16) & 0x1) == 0x1)
    {
        PortInfo = &g_pDmxDevOsi->RamPortInfo[0];
        PortInfo->WakeUp = MT_TRUE;	
		wake_up(&PortInfo->WaitQueue);
    }
    if(((pvr_state >> 20) & 0x1) == 0x1)
    {
        PortInfo = &g_pDmxDevOsi->RamPortInfo[1];
        PortInfo->WakeUp = MT_TRUE;
		wake_up(&PortInfo->WaitQueue);
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	if(((pvr1_state >> 1) & 0x1) == 0x1)
    {
        RecInfo = &pDmxDevOsi->DmxRecInfo[4];
        RecInfo->interrupt_count ++;

		OqInfo = &pDmxDevOsi->DmxOqInfo[4];
		OqInfo->OqWakeUp = MT_TRUE;
		wake_up(&OqInfo->OqWaitQueue);
    }
    if(((pvr1_state >> 5) & 0x1) == 0x1)
    {
        RecInfo = &pDmxDevOsi->DmxRecInfo[5];
        RecInfo->interrupt_count ++;

		OqInfo = &pDmxDevOsi->DmxOqInfo[5];
		OqInfo->OqWakeUp = MT_TRUE;
		wake_up(&OqInfo->OqWaitQueue);
    }
    if(((pvr1_state >> 9) & 0x1) == 0x1)
    {
        RecInfo = &pDmxDevOsi->DmxRecInfo[6];
        RecInfo->interrupt_count ++;

		OqInfo = &pDmxDevOsi->DmxOqInfo[6];
		OqInfo->OqWakeUp = MT_TRUE;
		wake_up(&OqInfo->OqWaitQueue);
    }
	
	if(((pvr2_state >> 12) & 0x1) == 0x1)
    {
        PortInfo = &g_pDmxDevOsi->RamPortInfo[2];
        PortInfo->WakeUp = MT_TRUE;
		wake_up(&PortInfo->WaitQueue);
    }
	
    if(((pvr2_state >> 16) & 0x1) == 0x1)
    {
        PortInfo = &g_pDmxDevOsi->RamPortInfo[3];
        PortInfo->WakeUp = MT_TRUE;
		wake_up(&PortInfo->WaitQueue);
    }
#endif

    return IRQ_HANDLED;
}


mt_s32 DMX_OsiCheckDataFlag(mt_u32 u32ChId, mt_u32 u32TimeOutMs)
{
    mt_s32 ret = MT_SUCCESS;

    if (u32ChId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("invalid ChanId = %#x\n", u32ChId);
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    ret = DMX_OsiCheckChnDataFlag(u32ChId, u32TimeOutMs);
    return ret;
}


static mt_void DMXOsiGetDataFlag(mt_u32 *pu32WatchCh, mt_u32 u32WatchNum, mt_u32 *pu32Flag)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *chan_info = NULL;
    DMX_ChanInfo_S *chan_next = NULL;
    struct list_head *dmx_list_head = NULL;
    mt_u32 ChanId = DMX_INVALID_CHAN_ID;

    dmx_list_head = &pDmxDevOsi->pes_chan_list;
    mutex_lock(&pDmxDevOsi->pes_chan_list_mutex);

    if  (!list_empty_careful(dmx_list_head))
    {
        list_for_each_entry_safe(chan_info, chan_next, dmx_list_head, chan_node)
        {
            ChanId = chan_info->ChanId;

            if (MT_SUCCESS == DMX_OsiGetChnDataFlag(ChanId)) 
            {
                pu32Flag[ChanId >> 5] |= (1 << (ChanId & 0x1F));
            }
        }
    }

    mutex_unlock(&pDmxDevOsi->pes_chan_list_mutex);

    dmx_list_head = &pDmxDevOsi->sec_chan_list;
    mutex_lock(&pDmxDevOsi->sec_chan_list_mutex);

    if  (!list_empty_careful(dmx_list_head))
    {
        list_for_each_entry_safe(chan_info, chan_next, dmx_list_head, chan_node)
        {
            ChanId = chan_info->ChanId;

            if (MT_SUCCESS == DMX_OsiGetChnDataFlag(ChanId)) 
            {
                pu32Flag[ChanId >> 5] |= (1 << (ChanId & 0x1F));
            }
        }
    }

    mutex_unlock(&pDmxDevOsi->sec_chan_list_mutex);

    dmx_list_head = &pDmxDevOsi->ts_chan_list;
    mutex_lock(&pDmxDevOsi->ts_chan_list_mutex);

    if  (!list_empty_careful(dmx_list_head))
    {
        list_for_each_entry_safe(chan_info, chan_next, dmx_list_head, chan_node)
        {
            ChanId = chan_info->ChanId;

            if (MT_SUCCESS == DMX_OsiGetChnDataFlag(ChanId)) 
            {
                pu32Flag[ChanId >> 5] |= (1 << (ChanId & 0x1F));
            }
        }
    }

    mutex_unlock(&pDmxDevOsi->ts_chan_list_mutex);

    dmx_list_head = &pDmxDevOsi->hw_pes_chan_list;
    mutex_lock(&pDmxDevOsi->hw_pes_chan_list_mutex);

    if  (!list_empty_careful(dmx_list_head))
    {
        list_for_each_entry_safe(chan_info, chan_next, dmx_list_head, chan_node)
        {
            ChanId = chan_info->ChanId;

            if (MT_SUCCESS == DMX_OsiGetChnDataFlag(ChanId)) 
            {
                pu32Flag[ChanId >> 5] |= (1 << (ChanId & 0x1F));
            }
        }
    }

    mutex_unlock(&pDmxDevOsi->hw_pes_chan_list_mutex);

}

static mt_s32 DMXOsiChannelWaitTimeOutForSec(DMX_ChanInfo_S *ChanInfo, mt_u32 u32TimeOutMs)
{
    mt_s32 ret;
    mt_u32 sec_buf_id = 0;
    DMX_ChanSecBuff_S *secBuff = NULL;

    sec_buf_id = ChanInfo->secBuffId;
    secBuff = &g_pDmxDevOsi->DmxChanSecBuff[sec_buf_id];

    MT_DBG_DEMUX("chid:%d, pid:%d\n", ChanInfo->ChanId, ChanInfo->ChanPid);
    ret = wait_event_interruptible_timeout(secBuff->OqWaitQueue,
              (reg_get_bufn_disc_wptr_disc_ch_wptr(sec_buf_id) != secBuff->desc_buff_write_p),
                                           (u32TimeOutMs*HZ/1000));
    MT_DBG_DEMUX("chid = %d, pid = %d, ret = %#x\n", ChanInfo->ChanId, ChanInfo->ChanPid, ret);
    return ret;
}

static mt_s32 DMXOsiChannelWaitTimeOutForPes(DMX_ChanInfo_S *ChanInfo, mt_u32 u32TimeOutMs)
{
    mt_s32 ret;
    DMX_SOFT_PESHEADER_S *peshead = NULL;
    peshead = ChanInfo->softpeshead;

    MT_DBG_DEMUX("chid:%d, pid:%d\n", ChanInfo->ChanId, ChanInfo->ChanPid);

    ret = wait_event_interruptible_timeout(ChanInfo->pes_wait,
                                           ChanInfo->pes_wait_flag,
                                           (u32TimeOutMs*HZ/1000));
	
    MT_DBG_DEMUX("chid = %d, pid = %d, ret = %#x\n", ChanInfo->ChanId, ChanInfo->ChanPid, ret);
    return ret;
}

static mt_s32 DMXOsiChannelWaitTimeOutForHWPes(DMX_ChanInfo_S *ChanInfo, mt_u32 u32TimeOutMs)
{
    mt_s32 ret;

    MT_DBG_DEMUX("chid:%d, pid:%d\n", ChanInfo->ChanId, ChanInfo->ChanPid);
    ret = wait_event_interruptible_timeout(ChanInfo->hw_pes_wait,
                                           (DMX_OsiGetHwPesChnDataFlag(ChanInfo) == MT_SUCCESS),
                                           (u32TimeOutMs*HZ/1000));
    MT_DBG_DEMUX("chid = %d, pid = %d, ret = %#x\n", ChanInfo->ChanId, ChanInfo->ChanPid, ret);
    return ret;
}


static mt_s32 DMXOsiChannelWaitTimeOutForTS(DMX_ChanInfo_S *ChanInfo, mt_u32 u32TimeOutMs)
{
    mt_s32 ret;
    mt_u32 sec_buf_id = 0;
    DMX_ChanSecBuff_S *secBuff = NULL;

    sec_buf_id = ChanInfo->secBuffId;
    secBuff = &g_pDmxDevOsi->DmxChanSecBuff[sec_buf_id];

    MT_DBG_DEMUX("chid:%d, pid:%d\n", ChanInfo->ChanId, ChanInfo->ChanPid);
    ret = wait_event_interruptible_timeout(secBuff->OqWaitQueue,
              (reg_get_bufn_data_wptr_data_ch_wptr(sec_buf_id) != secBuff->data_buff_write_p),
                                           (u32TimeOutMs*HZ/1000));
    MT_DBG_DEMUX("chid = %d, pid = %d, ret = %#x\n", ChanInfo->ChanId, ChanInfo->ChanPid, ret);
    return ret;
}

static mt_s32 DmxOsiReadSecToMsgBuffer(DMX_ChanInfo_S *pChanInfo, mt_u32 u32AcqNum,
                                    mt_u32 *pu32AcqedNum, DMX_UserMsg_S *psMsgList)
{
    mt_u32 flag_sec8ok = 0;
    mt_u32 u32BlkNum = 0;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanSecBuff_S *SecBuff = &pDmxDevOsi->DmxChanSecBuff[pChanInfo->secBuffId];
    mt_u8 buffId = SecBuff->BuffId;
    mt_u32 dataLen = 0;
    MT_BOOL soft_crc_check = MT_TRUE;

    mt_u32 bufStartAddr = (reg_get_bufn_staddr_buf_ch_staddr(buffId) << 3);
    mt_u32 dataBufSize = (reg_get_bufn_size_data_ch_size(buffId) << 10);
    mt_u32 descBufSize = (reg_get_bufn_size_disc_ch_size(buffId) << 10);
    mt_u8 *desc_data = (mt_u8 *)(SecBuff->sec_data_buff.startVirAddr + dataBufSize + SecBuff->desc_buff_write_p);
    mt_u32 discWP = reg_get_bufn_disc_wptr_disc_ch_wptr(buffId);
	mt_u32 dataWP;
    mt_u32 dataRP;
	mt_u8 *pu8Data = NULL;

    MT_DBG_DEMUX("pid = %d, sf_dswp = %d, hw_dswp = %d\n", pChanInfo->slot_reg0.bitc.pid, 
        SecBuff->desc_buff_write_p, reg_get_bufn_disc_wptr_disc_ch_wptr(buffId));

	if (discWP == SecBuff->desc_buff_write_p)
    {
        dataWP = reg_get_bufn_data_wptr_data_ch_wptr(buffId);
        dataRP = (reg_get_bufn_size_data_ch_rptr(buffId) << 10);

        if (dataRP > dataWP)
        {
            dataLen = dataBufSize - dataRP + dataWP;
            if (dataLen > (dataBufSize >> 1))
            {
                reg_set_bufn_size_data_ch_rptr(buffId, 0);
                reg_set_bufn_data_wptr_data_ch_wptr(buffId, 0);
                SecBuff->received_sec = 0;
                //SecBuff->desc_buff_write_p = 0;  
            }
        }
        else if (dataWP > dataRP)
        {
            dataLen = dataWP - dataRP;
            if(dataLen > (dataBufSize >> 1))
            {
                reg_set_bufn_size_data_ch_rptr(buffId, 0);
                reg_set_bufn_data_wptr_data_ch_wptr(buffId, 0);
                SecBuff->received_sec = 0;
                //SecBuff->desc_buff_write_p = 0;
            }
        }

        SecBuff->received_sec = 0;
    }
    else if (discWP >= SecBuff->desc_buff_write_p)
    {
    	dataLen = discWP - SecBuff->desc_buff_write_p;
    	u32BlkNum = dataLen / SEC_SECTION_DECS_DATA_LEN_PER_ITEM;      
    }
    else
    {
        dataLen = descBufSize - SecBuff->desc_buff_write_p;
        u32BlkNum = dataLen / SEC_SECTION_DECS_DATA_LEN_PER_ITEM;			
    }

    if (u32BlkNum > 0)
    {
        mt_u32 i = 0;
    	mt_u32 j = 0;
    	mt_u32 thisgood = 0;
    	mt_u32 acqNum = u32AcqNum;

        if (u32BlkNum <= acqNum)
        {
            acqNum = u32BlkNum;
        }
		
        MT_DBG_DEMUX("SEC BlkNum = %d, AcqNum = %d\n", u32BlkNum, u32AcqNum);
        SecBuff->received_sec = 0;

        for (i = 0; i < acqNum; i++)
        {
            mt_u8 sec_err = 0;
            mt_u32 data_addr = 0;
            mt_u8 *temp = NULL;
            mt_u8 filter_id = desc_data[i * SEC_SECTION_DECS_DATA_LEN_PER_ITEM];
            sec_err = (desc_data[i * SEC_SECTION_DECS_DATA_LEN_PER_ITEM + 1]) & 0x7;

            thisgood = 0;

            if (0 == sec_err)
            {
                data_addr = (desc_data[i * SEC_SECTION_DECS_DATA_LEN_PER_ITEM + 7] << 24) | (desc_data[i * SEC_SECTION_DECS_DATA_LEN_PER_ITEM + 6] << 16) | (desc_data[i * SEC_SECTION_DECS_DATA_LEN_PER_ITEM + 5] << 8) | (desc_data[i * SEC_SECTION_DECS_DATA_LEN_PER_ITEM + 4]);

                if ((data_addr >= bufStartAddr) && ((data_addr - bufStartAddr) <= dataBufSize))
                {
                    int offset = 0;

                    offset = ((data_addr - bufStartAddr) + 1) % dataBufSize;
                    temp = (mt_u8 *)(SecBuff->sec_data_buff.startVirAddr + offset);

                    psMsgList[j].filterid= filter_id;
                    psMsgList[j].u32MsgLen = ((temp[0] & 0xf) << 8);

                    offset = ((data_addr - bufStartAddr) + 2) % dataBufSize;
                    temp = (mt_u8 *)(SecBuff->sec_data_buff.startVirAddr + offset);
                    psMsgList[j].u32MsgLen +=  (temp[0] + 3);
					
					if ((psMsgList[j].u32MsgLen > 3) && (psMsgList[j].u32MsgLen <= 4096))
					{
	                    psMsgList[j].u32BufStartAddr = data_addr;                    

	                    SecBuff->data_buff_write_p = psMsgList[j].u32BufStartAddr + psMsgList[j].u32MsgLen - bufStartAddr;

	                    /*
	                    if next condition is true, means some data of a section 
	                    is in the end of the section data buf, and the other is
	                    in the start of the section data buf, so, SW should copy
	                    it to a continous new buf, the new buf size is
	                    DMX_SYMPHONY_SEC_ROLLBACK_BUF_SIZE, and follow the data
	                    buf and desc buf
	                    */
	                    if ((psMsgList[j].u32BufStartAddr + psMsgList[j].u32MsgLen) > (bufStartAddr + dataBufSize))
	                    {
	                        int len = 0;
	                        mt_u8 *tmpAddr = (mt_u8 *)(SecBuff->sec_data_buff.startVirAddr + dataBufSize + descBufSize);
	                        
	                        len = bufStartAddr + dataBufSize - data_addr;
	                        MT_DBG_DEMUX("Sec StartVirAddr = %#x, data_addr = %#x\n", SecBuff->sec_data_buff.startVirAddr, data_addr);
	                        MT_DBG_DEMUX("Sec len = %d, startAddr = %#x, msgLen = %d, StartAddr = %#x, dataBufSize = %#x, data_addr = %#x\n",
	                                     len, tmpAddr, psMsgList[j].u32MsgLen, bufStartAddr, dataBufSize, data_addr);
	                        memcpy((mt_u8 *)tmpAddr, ((mt_u8 *)SecBuff->sec_data_buff.startVirAddr + data_addr - bufStartAddr), len);
	                        memcpy((mt_u8 *)tmpAddr + len, (mt_u8 *)SecBuff->sec_data_buff.startVirAddr, psMsgList[j].u32MsgLen - len);
	                        psMsgList[j].u32BufStartAddr = SecBuff->sec_data_buff.startPhyAddr + dataBufSize + descBufSize;
	                        SecBuff->data_buff_write_p = psMsgList[j].u32MsgLen - len;						
	                    }

						pu8Data = (mt_u8 *)((ulong)(psMsgList[j].u32BufStartAddr - SecBuff->sec_data_buff.startPhyAddr + (ulong)SecBuff->sec_data_buff.startVirAddr));
	                    if ((1 == pChanInfo->slot_reg1.bitc.sec_discard_mode) ||
	                       (0 == pChanInfo->slot_reg1.bitc.sec_mode))
	                    {
	                        soft_crc_check = MT_FALSE;
	                    }
						else if (2 == pChanInfo->slot_reg1.bitc.sec_mode)
						{
							if (0 == (pu8Data[1] & 0x80))//bit7 is 0, there is no crc.
							{
								soft_crc_check = MT_FALSE;
							}
						}
						else
						{
						}

	                    if (MT_TRUE == soft_crc_check)
	                    {	                        
	                        mt_u32 k = psMsgList[j].u32MsgLen;
	                        mt_u32 crcSoftware = 0;
	                        mt_u32 crcStream = 0;

	                        if (k >= 8)
	                        {
	                            crcSoftware = CRC32(&(pu8Data[0]), k - 4);
	                            crcStream = (pu8Data[k-4]  << 24) | (pu8Data[k-3]  << 16) | (pu8Data[k-2]  << 8) | pu8Data[k-1];

	                            if (crcSoftware == crcStream )
	                            {
	                                j++;
	                                thisgood = 1;
	                            }
	                        }
	                    }
	                    else
	                    {
	                        j++;
	                        thisgood = 1;
	                    }
						
	                    if (1 == thisgood)    //fix bug106298
	                    {
	                        u32 datapos = 0; 
	                        u32 ii = 0;
							DMX_FilterInfo_S *Filter = &g_pDmxDevOsi->DmxFilterInfo[filter_id];
							
	                        pu8Data = (mt_u8 *)((ulong)(psMsgList[j-1].u32BufStartAddr - SecBuff->sec_data_buff.startPhyAddr + (ulong)SecBuff->sec_data_buff.startVirAddr));

	                        flag_sec8ok = 0;

	                        if ((Filter->AttachFlag) &&
	                            (Filter->ChanId == pChanInfo->ChanId) &&
	                            (Filter->config_reg.bitc.buf_id == pChanInfo->secBuffId))
	                        {
	                            if(Filter->Depth <= DMX_SECTION_SOFT_MASK_LEN)
	                            {
	                                flag_sec8ok = 1;
	                            }
	                            else
	                            {
	                                for (ii = 0; ii < Filter->Depth; ii++)
	                                {
	                                    datapos = ((0 == ii) ? 0 : (2 + ii));

	                                    if (Filter->Negate[ii])
	                                    {                          
	                                        if ((pu8Data[datapos]&(~Filter->Mask[ii])) == (Filter->Match[ii]&(~Filter->Mask[ii])))
	                                        {
	                                            break;
	                                        }
	                                    }
	                                    else
	                                    {                                          
	                                        if ((pu8Data[datapos]&(~Filter->Mask[ii])) != (Filter->Match[ii]&(~Filter->Mask[ii])))
	                                        {
	                                            break;
	                                        }
	                                    }
	                                }
									
	                                if (ii == Filter->Depth)
	                                {                               
	                                    flag_sec8ok = 1;
	                                }
	                            }
	                        }
							
	                        if (0 == flag_sec8ok)
	                        { 
	                            j--;
		                    }
		                }
	                }
                }
            }
            else
            {

            }

            /*
            patch for bug126144, SecBuff->desc_buff_write_p should only be
            updated after sw parsing a section package, if not, drv maybe
            loss section package
            */
            SecBuff->desc_buff_write_p += SEC_SECTION_DECS_DATA_LEN_PER_ITEM;

            if (SecBuff->desc_buff_write_p >= descBufSize)
            {
                SecBuff->desc_buff_write_p = 0;
            }				
        }
		
        *pu32AcqedNum = j;

    	MT_DBG_DEMUX("Read SEC SUCCESS, AcqedNum = %d\n", j);
		
    	if( 0 == j)
        {
            return  MT_ERR_DMX_NOAVAILABLE_DATA;
        }

        return MT_SUCCESS;
    }

    *pu32AcqedNum = 0;
    return MT_ERR_DMX_NOAVAILABLE_DATA;

}


static mt_s32 DmxOsiReadPesToMsgBuffer(DMX_ChanInfo_S *pChanInfo, mt_u32 u32AcqNum,
                                    mt_u32 *pu32AcqedNum, DMX_UserMsg_S *psMsgList)
{
    DMX_SOFT_PESHEADER_S *peshead = NULL;
    DMX_SOFT_PES_NODE_S *p_pes_node = NULL;
    DMX_SOFT_PES_NODE_S *p_next_node = NULL;
    struct list_head *p_pes_node_list_head = NULL;
    mt_u32 i = 0;

    mutex_lock(&pChanInfo->chan_mutex);

    peshead = pChanInfo->softpeshead;
    if (!peshead)
    {
        mutex_unlock(&pChanInfo->chan_mutex);
        MT_ERR_DEMUX("ReadPesToMsgBuffer: peshead is null\n");
        return MT_ERR_DMX_NULL_PTR;
    }

    if (!list_empty_careful(&peshead->pes_used_node_list))
    {
        p_pes_node_list_head = &peshead->pes_used_node_list;

		list_for_each_entry_safe(p_pes_node, p_next_node, p_pes_node_list_head, pes_node)
        {
            if ((p_pes_node->pes_node_addr) && (p_pes_node->pes_node_len) && (p_pes_node->pes_node_merged))
            {
                psMsgList[i].u32BufStartAddr = (p_pes_node->pes_node_addr - (ulong)peshead->pesdata) + peshead->pesphyadr;
                psMsgList[i].u32MsgLen = p_pes_node->pes_node_len;

                MT_DBG_DEMUX("node = %p, adr = %#x, len = %d\n", p_pes_node, p_pes_node->pes_node_addr, p_pes_node->pes_node_len);

                /*
                if the pes node read, del it from the pes_used_node_list
                */
                list_del_init(&p_pes_node->pes_node);
                /*
                then add it to the pes_read_node_list, the purpose of that
                is convenient for release pes node
                */
                list_add_tail(&p_pes_node->pes_node, &peshead->pes_read_node_list);

                i++;

                if (i >= u32AcqNum)
                {
                    break;                           
                }
            }
            else
            {
                MT_DBG_DEMUX("pes_used_node_list's node not merged, addr = %#x or len = %d, mer = %d\n", 
                    p_pes_node->pes_node_addr, p_pes_node->pes_node_len, p_pes_node->pes_node_merged);
            }
        }

        MT_DBG_DEMUX("PES BlkNum = %d, AcqNum = %d\n", i, u32AcqNum);

		if (i != 0)
		{
	        *pu32AcqedNum = i;
	        pChanInfo->u32countRef++;
			MT_INFO_DEMUX("[%s %d]*pu32AcqedNum=%d, pChanInfo->u32countRef=%d\r\n", __FUNCTION__, __LINE__, *pu32AcqedNum, pChanInfo->u32countRef);
			//the data has been read by user, so clear the flag.		
			pChanInfo->pes_wait_flag = 0;
	        mutex_unlock(&pChanInfo->chan_mutex);
	        return MT_SUCCESS;
		}
		else
		{
			mutex_unlock(&pChanInfo->chan_mutex);
        	return MT_ERR_DMX_NOAVAILABLE_DATA;
		}
    }
    else
    {
        MT_DBG_DEMUX("pes_used_node_list empty Err\n");
        mutex_unlock(&pChanInfo->chan_mutex);
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }

}


static mt_s32 DmxOsiReadTsToMsgBuffer(DMX_ChanInfo_S *pChanInfo, mt_u32 u32AcqNum,
                                    mt_u32 *pu32AcqedNum, DMX_UserMsg_S *psMsgList)
{
    mt_u32 u32BlkNum = 0;
  //  mt_s32 s32Ret = MT_SUCCESS;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanSecBuff_S *SecBuff = &pDmxDevOsi->DmxChanSecBuff[pChanInfo->secBuffId];
    mt_u8 buffId = SecBuff->BuffId;
    mt_u32 bufStartAddr = (reg_get_bufn_staddr_buf_ch_staddr(buffId) << 3);
    mt_u32 dataBufSize = (reg_get_bufn_size_data_ch_size(buffId) << 10);
    mt_u32 dataWP = reg_get_bufn_data_wptr_data_ch_wptr(buffId);
    mt_u32 data_addr = SecBuff->data_buff_write_p + bufStartAddr;
    mt_u32 dataLen = 0;

	
    if (dataWP == SecBuff->data_buff_write_p)
    {
        u32BlkNum = 0;
    }
    else if (dataWP > SecBuff->data_buff_write_p)
    {
    	dataLen = dataWP - SecBuff->data_buff_write_p;
        u32BlkNum = dataLen / SEC_SECTION_TS_DATA_LEN_PER_ITEM;       	
    }
    else
    {
        dataLen = dataBufSize - SecBuff->data_buff_write_p;
        u32BlkNum = dataLen / SEC_SECTION_TS_DATA_LEN_PER_ITEM;        	      	
    }
    MT_DBG_DEMUX("ts buffId = 0x%x dataWP = 0x%x, u32BlkNum=%d\n", buffId, dataWP, u32BlkNum);

	
    if (u32BlkNum > 0)
    {
        mt_u32 i = 0;
        mt_u32 acqNum = u32AcqNum;

        if (u32BlkNum < acqNum)
        {
            acqNum = u32BlkNum;
        }

        MT_DBG_DEMUX("TS BlkNum = %d, AcqNum = %d\n", u32BlkNum, u32AcqNum);
		
        for (i = 0; i < acqNum; i++)  //why update ts pkt one by one ?
        {
            if ((data_addr >= bufStartAddr) && ((data_addr - bufStartAddr) <= dataBufSize))
            {
                psMsgList[i].u32BufStartAddr = data_addr + i * 188;
                psMsgList[i].u32MsgLen = 188;
            }

            SecBuff->data_buff_write_p += SEC_SECTION_TS_DATA_LEN_PER_ITEM;

            if (SecBuff->data_buff_write_p >= dataBufSize)
            {
                SecBuff->data_buff_write_p = 0;
            }	
        }
		
        *pu32AcqedNum = acqNum;
        MT_DBG_DEMUX("Read TS MT_SUCCESS, AcqedNum = %d\n", acqNum);
        return MT_SUCCESS;
    }

    *pu32AcqedNum = 0;
    return MT_ERR_DMX_NOAVAILABLE_DATA;
}


static mt_s32 DmxOsiReadHwPesToMsgBuffer(DMX_ChanInfo_S *pChanInfo, mt_u32 u32AcqNum,
                                    mt_u32 *pu32AcqedNum, DMX_UserMsg_S *psMsgList)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanEsBuff_S *esBuff = NULL;
    mt_u32 desc_wr = 0;
    mt_u32 desc_rd = 0;
    mt_u32 desc_buf_size = 0;
    mt_u32 desc_data_size = 0;
	  mt_u8 pes_chnum = 0;
    dmx_es_desc_data_s *cur_desc = NULL;
	  dmx_es_desc_data_s *next_desc = NULL;
    phys_addr_t pes_phy_saddr = 0;
    ulong pes_vir_saddr = 0;
    mt_u32 pes_buf_size = 0;
  //  mt_u8 *pes_data = NULL;
    mt_u32 pes_offset = 0;
    mt_u32 desc_cnt = 0;
    mt_u32 need_cnt = 0;
    mt_u32 update_cnt = 0;
    mt_u32 pes_len = 0;
    mt_u32 i = 0;
  //  mt_s32 s32Ret = MT_SUCCESS;

    need_cnt = u32AcqNum;
    pes_chnum = pChanInfo->avChanId;

    esBuff = &pDmxDevOsi->DmxChanEsBuff[pes_chnum];
	
    pes_phy_saddr = esBuff->data_buff.startPhyAddr;
    pes_vir_saddr = (ulong)esBuff->data_buff.startVirAddr;
    pes_buf_size = esBuff->data_buff.size;

    desc_buf_size = esBuff->desc_buff.size;

    desc_rd = reg_get_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(pes_chnum);
    desc_wr = reg_get_trpp_ch_dscrpt_wr_addr_trpp_ch_dscrpt_waddr(pes_chnum);

    if (desc_wr >= desc_rd)
    {
        desc_data_size = desc_wr - desc_rd;
    }
    else
    {
        desc_data_size = desc_wr + desc_buf_size - desc_rd;
    }

    desc_cnt = desc_data_size / SEC_ES_DECS_DATA_LEN_PER_ITEM;

    if (desc_cnt < 2)
    {
        *pu32AcqedNum = 0;
        MT_DBG_DEMUX("HW PES no data\n");
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }

    if (desc_cnt > need_cnt)
    {
	    update_cnt = need_cnt;            
    }
    else
    {
        update_cnt = desc_cnt - 1;
    }

    for (i = 0; i < update_cnt; i++)
    {
        cur_desc = (dmx_es_desc_data_s *)(esBuff->desc_buff.startVirAddr + desc_rd);
        next_desc = (dmx_es_desc_data_s *)(esBuff->desc_buff.startVirAddr + ((desc_rd + SEC_ES_DECS_DATA_LEN_PER_ITEM) % desc_buf_size));

        if (next_desc->u32Addr > cur_desc->u32Addr)
        {
            pes_len = next_desc->u32Addr - cur_desc->u32Addr;
			psMsgList[i].u32MsgLen = pes_len;
			psMsgList[i].u32BufStartAddr = cur_desc->u32Addr;
        }
        else if (next_desc->u32Addr < cur_desc->u32Addr)
        {
            pes_len = pes_phy_saddr + pes_buf_size - cur_desc->u32Addr;

            pes_offset = cur_desc->u32Addr - pes_phy_saddr;

            psMsgList[i].u32MsgLen = pes_len;

            psMsgList[i].u32BufStartAddr = pes_phy_saddr + pes_buf_size;

			memcpy((mt_u8 *)(pes_vir_saddr + pes_buf_size), (mt_u8 *)(pes_vir_saddr + pes_offset), pes_len);

			pes_len = next_desc->u32Addr - pes_phy_saddr;

            memcpy((mt_u8 *)(pes_vir_saddr + pes_buf_size + psMsgList[i].u32MsgLen), (mt_u8 *)pes_vir_saddr, pes_len);

			psMsgList[i].u32MsgLen += pes_len;
		}
        else
        {
            MT_DBG_DEMUX("Error, current and next pes data addr are the same\n");
        }

		desc_rd += SEC_ES_DECS_DATA_LEN_PER_ITEM;

        desc_rd %= desc_buf_size;
    }

    esBuff->desc_buff_write_p = desc_rd;
    *pu32AcqedNum = update_cnt;
    return MT_SUCCESS;
}


static mt_s32 DmxOsiReadToMsgBuffer(DMX_ChanInfo_S *pChanInfo, mt_u32 u32AcqNum,
                                    mt_u32 *pu32AcqedNum, DMX_UserMsg_S *psMsgList, mt_u32 u32TimeOutMs)
{
    mt_s32 ret = MT_SUCCESS;

    if (NULL == pChanInfo)
    {
        return MT_ERR_DMX_INVALID_PARA;
	}

    switch (pChanInfo->ChanType)
    {
        case MT_UNF_DMX_CHAN_TYPE_SEC:
        case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
        {
            ret = DmxOsiReadSecToMsgBuffer(pChanInfo, u32AcqNum, pu32AcqedNum, psMsgList);

            /*
            *patch for bug131741, user may use MT_UNF_DMX_AcquireBuf to get data
            *directly, in order to avoid the user does not receive data, so drv
            *should add blocking here.
            */

            if (MT_SUCCESS == ret)
            {
                break;
            }

            if (0 == u32TimeOutMs)
            {
                break;
            }

            ret = DMXOsiChannelWaitTimeOutForSec(pChanInfo, u32TimeOutMs);

            if (-ERESTARTSYS == ret)
            {
                MT_DBG_DEMUX("wait sec interrupted by signal, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            if (0 == ret)
            {
                MT_DBG_DEMUX("wait sec time out, ret: %#x!\n", ret);
                return MT_ERR_DMX_NOAVAILABLE_DATA;
            }

            ret = DmxOsiReadSecToMsgBuffer(pChanInfo, u32AcqNum, pu32AcqedNum, psMsgList);

            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_POST:
        {
            ret = DmxOsiReadTsToMsgBuffer(pChanInfo, u32AcqNum, pu32AcqedNum, psMsgList);
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            ret = DmxOsiReadPesToMsgBuffer(pChanInfo, u32AcqNum, pu32AcqedNum, psMsgList);
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_HW_PES:
        {
            ret = DmxOsiReadHwPesToMsgBuffer(pChanInfo, u32AcqNum, pu32AcqedNum, psMsgList);
            break;
        }

        default:
        {
            MT_ERR_DEMUX("Read ChanType = %d is not allowed\n", pChanInfo->ChanType);
            ret = MT_ERR_DMX_INVALID_PARA;
			break;
        }		
	}

    MT_DBG_DEMUX("ret:%#x, acqed:%d, pid:%d\n", ret, *pu32AcqedNum, pChanInfo->ChanPid);
    return ret;
}

static mt_s32 DMXOsiReadSec(DMX_ChanInfo_S *pChanInfo,
                            mt_u32 u32AcqNum,
                            mt_u32 *pu32AcqedNum,
                            DMX_UserMsg_S *psMsgList,
                            DMX_OQ_Info_S *OqInfo,
                            mt_u32 u32TimeOutMs)
{
    mt_s32 ret = MT_SUCCESS;
	
    ret = DmxOsiReadToMsgBuffer(pChanInfo, u32AcqNum, pu32AcqedNum, psMsgList, u32TimeOutMs);

    if ((ret == MT_SUCCESS) && (*pu32AcqedNum > 0))
    {
	    pChanInfo->u32HitAcq++;
	    return MT_SUCCESS;
    }

	return ret;

}


static mt_s32 DMXOsiReadMsg(DMX_ChanInfo_S *pChanInfo,
                            mt_u32 u32AcqNum,
                            mt_u32 *pu32AcqedNum,
                            DMX_UserMsg_S *psMsgList,
                            mt_u32 u32TimeOutMs)
{
    mt_s32 ret = MT_SUCCESS;
	
    ret = DmxOsiReadToMsgBuffer(pChanInfo, u32AcqNum, pu32AcqedNum, psMsgList, u32TimeOutMs);

    if ((ret == MT_SUCCESS) && (*pu32AcqedNum > 0))
    {
	    pChanInfo->u32HitAcq++;
	    return MT_SUCCESS;
    }

	return ret;

}


static mt_u32 FilterGetValidDepth(const MT_UNF_DMX_FILTER_ATTR_S *FilterAttr)
{
    mt_u32 Depth = FilterAttr->u32FilterDepth;

    while (Depth)
    {
        if (0xff == FilterAttr->au8Mask[Depth - 1])
        {
            --Depth;
        }
        else
        {
            break;
        }
    }
	
    MT_DBG_DEMUX("Depth = %d\n", Depth);
    return Depth;
}


static mt_void UpdateProcRef(mt_s32 ref)
{
#ifdef MT_DEMUX_PROC_SUPPORT
    GlobalProcInfo.Ref = ref;
    if (ref >= GlobalProcInfo.MaxRef) {
	GlobalProcInfo.MaxRef = ref;
    }
#endif
}

mt_s32 DmxReset(mt_void)
{

    DmxHalConfigHardware();
    udelay(10);

    return MT_SUCCESS;
}

mt_void DMX_OsiInit_slot_init(mt_void)
{
    unsigned int i;
    for (i = 0; i < SLOT_REG_COUNT * 2; i++) {
	*((volatile unsigned int *)((ulong)reg_dmx_demux_slotn_cfg0 + i * 0x4)) = 0x0;
    }
}

mt_void DMX_OsiInitFilterInit(mt_void)
{
    unsigned int i;
    for (i = 0; i < TSI_SF_FILT_CHNUM; i++) {
	*((volatile unsigned int *)((ulong)reg_dmx_filtern_config + i * 0x4)) = 0x00000000;
    }
    for (i = 0; i < TSI_SF_FUNIT_NUM; i++) {
	*((volatile unsigned int *)((ulong)reg_dmx_funit_filter_data + i * 0x4)) = 0x00000000;
    }
    for (i = 0; i < TSI_SF_FUNIT_NUM; i++) {
	*((volatile unsigned int *)((ulong)reg_dmx_funit_filter_mask + i * 0x4)) = 0x00000000;
    }
    for (i = 0; i < TSI_SF_FUNIT_NUM; i++) {
	*((volatile unsigned int *)((ulong)reg_dmx_funit_filter_mode + i * 0x4)) = 0x00000000;
    }
}

mt_void DMX_OsiInitChInit(unsigned int tsp_ch0_ram, unsigned int tsp_ch1_ram, unsigned int tsp_ch2_ram, unsigned int gm_ch0_allow, unsigned int gm_ch1_allow, unsigned int gm_ch2_allow)
{
    *((volatile unsigned int *)((ulong)reg_dmx_tsp_pcrsetn + 0x10)) = ((tsp_ch2_ram & 0xf) << 8) | ((tsp_ch1_ram & 0xf) << 4) | ((tsp_ch0_ram & 0xf) << 0);

    *((volatile unsigned int *)((ulong)reg_dmx_gm_ch_ctrl + 0x0)) = ((gm_ch2_allow & 0x7) << 8) | ((gm_ch1_allow & 0x7) << 4) | ((gm_ch0_allow & 0x7) << 0);
}

mt_void DMX_OsiTsiRegisterInit(mt_void)
{
    //ch_tsp_gm_int
    DMX_OsiInitChInit(0x3, 0x3, 0x2, 0x4, 0x4, 0x2); //tsp ram distribute and gm req ctrl

    //slot filter initial
    DMX_OsiInit_slot_init();
    DMX_OsiInitFilterInit();
}

#define NOC_CFG_BASE_SCH 0xFFBE0000

mt_u32 noc_sch_cfg(
    mt_u32 DdrConf, mt_u32 ActToAct, mt_u32 RdToMiss, mt_u32 WrToMiss, mt_u32 BurstLen, mt_u32 RdToWr, mt_u32 WrToRd, mt_u32 BwRatio, mt_u32 AutoPrecharge, mt_u32 BwRatioExtended, mt_u32 ReadLatency, int Rrd, int Faw, int FawBank, int BusRdToRd, int BusRdToWr, int BusWrToRd, int CcdL, int WrToRdL, int RrdL)
{
    //MT_DBG_DEMUX("[INFO] Configuration for NoC Scheduler Parameters !!\n");
    dmx_outl(NOC_CFG_BASE_SCH + 0x08, 0);
    dmx_outl(NOC_CFG_BASE_SCH + 0x0c, ((0 << 31) + (7 << 26) + (3 << 21) + (2 << 18) + (21 << 12) + (12 << 6) + 20)); //0x1c695314
    dmx_outl(NOC_CFG_BASE_SCH + 0x10, ((BwRatioExtended << 1) + 0));
    dmx_outl(NOC_CFG_BASE_SCH + 0x14, ReadLatency);
    dmx_outl(NOC_CFG_BASE_SCH + 0x38, ((FawBank << 10) + (Faw << 4) + Rrd));              //0x503
    dmx_outl(NOC_CFG_BASE_SCH + 0x3c, ((BusWrToRd << 4) + (BusRdToWr << 2) + BusRdToRd)); //0x29
    dmx_outl(NOC_CFG_BASE_SCH + 0x40, ((RrdL << 8) + (WrToRdL << 3) + CcdL));
    return (0);
}

#if 0
static mt_u32 DMX_OsiHardwareInit(mt_u8 pool_mode)
{
    mt_u8 ts_input_num = 0;
    dmx_ts_input_t ts_input_cfg[4];

    //noc_sch_cfg(0,20,12,21,2,3,7,0,0,0,22,3,16,1,1,2,2,0,0,0);
    //dmx_outl(STI_TUNER_INIT, 0x1c);


    //MT_DBG_DEMUX("hal_dmx_hardware_init ------0\n");
    ts_input_cfg[0].input_way = MT_TRUE;
    ts_input_cfg[0].local_sel_edge = MT_TRUE;
    ts_input_cfg[0].error_indicator = MT_FALSE;
    ts_input_cfg[0].start_byte_mask = MT_FALSE;


    ts_input_cfg[1].input_way = MT_TRUE;
    ts_input_cfg[1].local_sel_edge = MT_TRUE;
    ts_input_cfg[1].error_indicator = MT_FALSE;
    ts_input_cfg[1].start_byte_mask = MT_FALSE;


    ts_input_cfg[2].input_way = MT_TRUE;
    ts_input_cfg[2].local_sel_edge = MT_TRUE;
    ts_input_cfg[2].error_indicator = MT_FALSE;
    ts_input_cfg[2].start_byte_mask = MT_FALSE;


    ts_input_cfg[3].input_way = MT_TRUE;
    ts_input_cfg[3].local_sel_edge = MT_TRUE;
    ts_input_cfg[3].error_indicator = MT_FALSE;
    ts_input_cfg[3].start_byte_mask = MT_FALSE;
    //MT_DBG_DEMUX("hal_dmx_hardware_init ------1\n");
    if(pool_mode == 1)
    {
        dmx_outl(reg_dmx_tsp_tag_clr, 0x3f);
        dmx_outl(reg_dmx_tsp_threshold0, 0x03030404);
        dmx_outl(reg_dmx_tsp_threshold1, 0x02);
        dmx_outl(reg_dmx_gm_ch_ctrl, 0x244);
    }
    else if(pool_mode == 0)
    {
        dmx_outl(reg_dmx_tsp_tag_clr, 0xf);
        //dmx_outl(TSP_THRESHOLD, 0x233);
        dmx_outl(reg_dmx_tsp_threshold0, 0x0);
        dmx_outl(reg_dmx_tsp_threshold1, 0x03030303);
        dmx_outl(reg_dmx_gm_ch_ctrl, 0x244);
    }
    else
    {
        dmx_outl(reg_dmx_tsp_tag_clr, 0xf);
        dmx_outl(reg_dmx_tsp_threshold0, 0x233);
        dmx_outl(reg_dmx_tsp_threshold1, 0x03030303);
        dmx_outl(reg_dmx_gm_ch_ctrl, 0x244);
    }
    dmx_outl(reg_dmx_tsp_pcrsetn, 0x11);
    //MT_DBG_DEMUX("hal_dmx_hardware_init ------2\n");
    //选择TS0作为输入源，该域置0；
    //选择TS1作为输入源，该域置1；

    for (ts_input_num = 0; ts_input_num < 4; ts_input_num++)
    {
      MT_DBG_DEMUX("TS1 input ts_input_num=%d\n",ts_input_num);
      if(ts_input_num == 1)
      {
        reg_ts1_sample_ctrl_t reg;

        reg.bitc.syncon_th       = 0xf;
        reg.bitc.syncoff_th      = 0xf;
        reg.bitc.ts_188_reg      = 0x1;//ts length 188
        reg.bitc.ts_188_reg_en   = 0x0;//hardware detect
        reg.bitc.brk_sel         = 0x0;
        if(ts_input_cfg[1].start_byte_mask == MT_TRUE)
          reg.bitc.sop_token       = 0x0;
        else
          reg.bitc.sop_token       = 0x0;

        reg.bitc.sync_bypass     = 0x0;
        reg.bitc.val_bypass      = 0x0;
        if(ts_input_cfg[1].error_indicator == MT_TRUE)
          reg.bitc.sync_err_bypass = 0x0;
        else
          reg.bitc.sync_err_bypass = 0x1;
        reg.bitc.tei_bypass      = 0x1;
        reg.bitc.err_bypass      = 0x1;
        reg.bitc.err_pol         = 0x1;
        reg.bitc.val_pol         = 0x1;
        reg.bitc.sync_pol        = 0x1;
        reg.bitc.serial_sel      = 0x0;
        if(ts_input_cfg[1].input_way == MT_TRUE)
        {
          //parrel
          reg.bitc.serial_en = 0;
          //reg_set_ts1_sample_ctrl(0x078001ff);
        }
        else
        {
          //serial
          reg.bitc.serial_en = 1;
          //reg_set_ts1_sample_ctrl(0x478001ff);
        }
        reg.bitc.ts_en = 0x1;
        reg_set_ts1_sample_ctrl(reg.all);
      }
      else if(ts_input_num == 2)
      {
        reg_ts2_sample_ctrl_t reg;

        reg.bitc.syncon_th       = 0xf;
        reg.bitc.syncoff_th      = 0xf;
        reg.bitc.ts_188_reg      = 0x1;//ts length 188
        reg.bitc.ts_188_reg_en   = 0x0;//hardware detect
        reg.bitc.brk_sel         = 0x0;
        if(ts_input_cfg[2].start_byte_mask == MT_TRUE)
          reg.bitc.sop_token       = 0x0;
        else
          reg.bitc.sop_token       = 0x0;

        reg.bitc.sync_bypass     = 0x1;
        reg.bitc.val_bypass      = 0x1;
        if(ts_input_cfg[2].error_indicator == MT_TRUE)
          reg.bitc.sync_err_bypass = 0x0;
        else
          reg.bitc.sync_err_bypass = 0x1;
        reg.bitc.tei_bypass      = 0x1;
        reg.bitc.err_bypass      = 0x1;
        reg.bitc.err_pol         = 0x1;
        reg.bitc.val_pol         = 0x1;
        reg.bitc.sync_pol        = 0x1;
        reg.bitc.serial_sel      = 0x0;
        if(ts_input_cfg[2].input_way == MT_TRUE)
        {
          //parrel
          reg.bitc.serial_en = 0;
          //reg_set_ts2_sample_ctrl(0x078001ff);
        }
        else
        {
          //serial
          reg.bitc.serial_en = 1;
          //reg_set_ts2_sample_ctrl(0x478001ff);
        }
        reg.bitc.ts_en = 0x1;
        reg_set_ts2_sample_ctrl(reg.all);
      }
      else if(ts_input_num == 3)
      {
        reg_ts3_sample_ctrl_t reg;

        reg.bitc.syncon_th       = 0xf;
        reg.bitc.syncoff_th      = 0xf;
        reg.bitc.ts_188_reg      = 0x1;//ts length 188
        reg.bitc.ts_188_reg_en   = 0x0;//hardware detect
        reg.bitc.brk_sel         = 0x0;
        if(ts_input_cfg[3].start_byte_mask == MT_TRUE)
          reg.bitc.sop_token       = 0x0;
        else
          reg.bitc.sop_token       = 0x0;

        reg.bitc.sync_bypass     = 0x1;
        reg.bitc.val_bypass      = 0x1;
        if(ts_input_cfg[3].error_indicator == MT_TRUE)
          reg.bitc.sync_err_bypass = 0x0;
        else
          reg.bitc.sync_err_bypass = 0x1;
        reg.bitc.tei_bypass      = 0x1;
        reg.bitc.err_bypass      = 0x1;
        reg.bitc.err_pol         = 0x1;
        reg.bitc.val_pol         = 0x1;
        reg.bitc.sync_pol        = 0x1;
        reg.bitc.serial_sel      = 0x0;
        if(ts_input_cfg[3].input_way == MT_TRUE)
        {
          //parrel
          reg.bitc.serial_en = 0;
          //reg_set_ts3_sample_ctrl(0x078001ff);
        }
        else
        {
          //serial
          reg.bitc.serial_en = 1;
          //reg_set_ts3_sample_ctrl(0x478001ff);
        }
        reg.bitc.ts_en = 0x1;
        reg_set_ts3_sample_ctrl(reg.all);
      }
      else if(ts_input_num == 0)
      {
        reg_ts0_sample_ctrl_t reg;

        reg.bitc.syncon_th       = 0xf;
        reg.bitc.syncoff_th      = 0xf;
        reg.bitc.ts_188_reg      = 0x1;//ts length 188
        reg.bitc.ts_188_reg_en   = 0x0;//hardware detect
        reg.bitc.brk_sel         = 0x0;
        if(ts_input_cfg[0].start_byte_mask == MT_TRUE)
          reg.bitc.sop_token     = 0x0;
        else
          reg.bitc.sop_token     = 0x0;

        reg.bitc.sync_bypass     = 0x1;//two line mode set 1
        reg.bitc.val_bypass      = 0x1;//two line mode set 1
        if(ts_input_cfg[0].error_indicator == MT_TRUE)
          reg.bitc.sync_err_bypass = 0x0;
        else
          reg.bitc.sync_err_bypass = 0x1;
        reg.bitc.tei_bypass      = 0x1;
        reg.bitc.err_bypass      = 0x1;
        reg.bitc.err_pol         = 0x1;
        reg.bitc.val_pol         = 0x1;
        reg.bitc.sync_pol        = 0x1;
        reg.bitc.serial_sel      = 0x0;
        if(ts_input_cfg[0].input_way == MT_TRUE)
        {
          //parrel
          reg.bitc.serial_en = 0;
          //reg_set_ts0_sample_ctrl(0x078001ff);
        }
        else
        {
          //serial
          reg.bitc.serial_en = 1;
          //reg_set_ts0_sample_ctrl(0x478001ff);
        }
        reg.bitc.ts_en = 0x1;
        reg_set_ts0_sample_ctrl(reg.all);
      }
      //MT_DBG_DEMUX("hal_dmx_hardware_init ------4\n");
    }
    //mtos_irq_request (IRQ_PTI_SF_ID, _hal_dmx_isr_handle, RISE_EDGE_TRIGGER);

    return MT_SUCCESS;
}
#else

/**************  by i2c config demod**********************
**  iicw 0x50 0x23 0x7  配置数据采样的上下沿
**  w 0xfff00158 0x33   --- config ts0\ts2\ts3  serial
**  w 0xfff00158 0x3c   --- config ts1          parrel
**********************************************************/
mt_u32 DMX_OsiHardwareInit(mt_u8 pool_mode,  mt_u8  id,  dmx_ts_input_t  *ts_cfg)
{
    mt_u8 ts_input_num = 0;
    dmx_ts_input_t ts_input_cfg[4];
    ulong tsi_clk_reg = mt_get_clk_base() + 0x24;
	mt_u32 reg_value;

    //noc_sch_cfg(0,20,12,21,2,3,7,0,0,0,22,3,16,1,1,2,2,0,0,0);
    //dmx_outl(STI_TUNER_INIT, 0x1c);
    if(ts_cfg == NULL)
    {
	    ts_input_cfg[0].input_way = MT_FALSE;
	    ts_input_cfg[0].local_sel_edge = MT_TRUE;
	    ts_input_cfg[0].error_indicator = MT_FALSE;
	    ts_input_cfg[0].start_byte_mask = MT_TRUE;

	    ts_input_cfg[1].input_way = MT_TRUE;
	    ts_input_cfg[1].local_sel_edge = MT_TRUE;
	    ts_input_cfg[1].error_indicator = MT_FALSE;
	    ts_input_cfg[1].start_byte_mask = MT_FALSE;

	    ts_input_cfg[2].input_way = MT_TRUE;
	    ts_input_cfg[2].local_sel_edge = MT_TRUE;
	    ts_input_cfg[2].error_indicator = MT_FALSE;
	    ts_input_cfg[2].start_byte_mask = MT_FALSE;

	    ts_input_cfg[3].input_way = MT_FALSE;
	    ts_input_cfg[3].local_sel_edge = MT_TRUE;
	    ts_input_cfg[3].error_indicator = MT_FALSE;
	    ts_input_cfg[3].start_byte_mask = MT_TRUE;
    }

    if((id >= 0  && id <= 3)  &&  (ts_cfg  != NULL))
    {
	    ts_input_cfg[id].input_way = ts_cfg->input_way;
	    ts_input_cfg[id].local_sel_edge = ts_cfg->local_sel_edge;
	    ts_input_cfg[id].error_indicator = ts_cfg->error_indicator;
	    ts_input_cfg[id].start_byte_mask = ts_cfg->start_byte_mask;

           if (id == 1)
           {
        	    reg_ts1_sample_ctrl_t reg;
        	    reg.all = 0;
        	    reg_set_ts1_sample_ctrl(0);
        	    reg.bitc.syncon_th = 0xf;
        	    reg.bitc.syncoff_th = 0xf;
        	    reg.bitc.ts_188_reg = 0x1;    //ts length 188
        	    reg.bitc.ts_188_reg_en = 0x0; //hardware detect
        	    reg.bitc.brk_sel = 0x3;
        	    if (ts_input_cfg[1].start_byte_mask == MT_TRUE) {
        		reg.bitc.sop_token = 0x0;
        		reg.bitc.sync_bypass = 0x1;
        		reg.bitc.val_bypass = 0x1;
        	    } else {
        		reg.bitc.sop_token = 0x0;
        		reg.bitc.sync_bypass = 0x0;
        		reg.bitc.val_bypass = 0x0;
        	    }

        	    if (ts_input_cfg[1].error_indicator == MT_TRUE) {
        		reg.bitc.sync_err_bypass = 0x0;
        		reg.bitc.tei_bypass = 0x0;
        		reg.bitc.err_bypass = 0x0;
        	    } else {
        		reg.bitc.sync_err_bypass = 0x1;
        		reg.bitc.tei_bypass = 0x1;
        		reg.bitc.err_bypass = 0x1;
        	    }

        	    reg.bitc.err_pol = 0x1;
        	    reg.bitc.val_pol = 0x1;
        	    reg.bitc.sync_pol = 0x1;
        	    reg.bitc.serial_sel = 0x0;
        	    if (ts_input_cfg[1].input_way == MT_TRUE) {
        		//parrel
        		reg.bitc.serial_en = 0;
        		reg.bitc.sync_bypass = 0x0;
        		reg.bitc.val_bypass = 0x0;
        		//reg_set_ts1_sample_ctrl(0x078001ff);
        	    } else {
        		//serial
        		reg.bitc.serial_en = 1;
        		//reg_set_ts1_sample_ctrl(0x478001ff);
        	    }
        	    reg.bitc.ts_en = 0x1;
        	    reg_set_ts1_sample_ctrl(reg.all);
        	    if(ts_input_cfg[1].local_sel_edge == MT_FALSE)
        	        {
        	             u32  reg_clk = dmx_inl(tsi_clk_reg);
        	    		reg_clk |= 0x20;
        	    		dmx_outl(tsi_clk_reg, reg_clk);
        	        }
        	}
        	else if (id == 2)
        	{
        	    reg_ts2_sample_ctrl_t reg;
        	    reg.all = 0;
        	    reg_set_ts2_sample_ctrl(0);
        	    reg.bitc.syncon_th = 0xf;
        	    reg.bitc.syncoff_th = 0xf;
        	    reg.bitc.ts_188_reg = 0x1;    //ts length 188
        	    reg.bitc.ts_188_reg_en = 0x0; //hardware detect
        	    reg.bitc.brk_sel = 0x3;
        	    if (ts_input_cfg[2].start_byte_mask == MT_TRUE) {
        		reg.bitc.sop_token = 0x0;
        		reg.bitc.sync_bypass = 0x1;
        		reg.bitc.val_bypass = 0x1;
        	    } else {
        		reg.bitc.sop_token = 0x0;
        		reg.bitc.sync_bypass = 0x0;
        		reg.bitc.val_bypass = 0x0;
        	    }

        	    if (ts_input_cfg[2].error_indicator == MT_TRUE) {
        		reg.bitc.sync_err_bypass = 0x0;
        		reg.bitc.tei_bypass = 0x0;
        		reg.bitc.err_bypass = 0x0;
        	    } else {
        		reg.bitc.sync_err_bypass = 0x1;
        		reg.bitc.tei_bypass = 0x1;
        		reg.bitc.err_bypass = 0x1;
        	    }

        	    reg.bitc.err_pol = 0x1;
        	    reg.bitc.val_pol = 0x1;
        	    reg.bitc.sync_pol = 0x1;
        	    reg.bitc.serial_sel = 0x0;
        	    if (ts_input_cfg[2].input_way == MT_TRUE) {
        		//parrel
        		reg.bitc.serial_en = 0;
        		reg.bitc.sync_bypass = 0x0;
        		reg.bitc.val_bypass = 0x0;
        		//reg_set_ts2_sample_ctrl(0x078001ff);
        	    } else {
        		//serial
        		reg.bitc.serial_en = 1;
        		//reg_set_ts2_sample_ctrl(0x478001ff);
        	    }
        	    reg.bitc.ts_en = 0x1;
        	    reg_set_ts2_sample_ctrl(reg.all);
        	    if(ts_input_cfg[2].local_sel_edge == MT_FALSE)
        	    {
        	             u32  reg_clk = dmx_inl(tsi_clk_reg);
        	    		reg_clk |= 0x40;
        	    		dmx_outl(tsi_clk_reg, reg_clk);
        	    }
        	}
        	else if (id == 3)
        	{
        	    reg_ts3_sample_ctrl_t reg;
        	    reg.all = 0;
        	    reg_set_ts3_sample_ctrl(0);
        	    reg.bitc.syncon_th = 0xf;
        	    reg.bitc.syncoff_th = 0xf;
        	    reg.bitc.ts_188_reg = 0x1;    //ts length 188
        	    reg.bitc.ts_188_reg_en = 0x0; //hardware detect
        	    reg.bitc.brk_sel = 0x3;
        	    if (ts_input_cfg[3].start_byte_mask == MT_TRUE) {
        		reg.bitc.sop_token = 0x0;
        		reg.bitc.sync_bypass = 0x1;
        		reg.bitc.val_bypass = 0x1;
        	    } else {
        		reg.bitc.sop_token = 0x0;
        		reg.bitc.sync_bypass = 0x0;
        		reg.bitc.val_bypass = 0x0;
        	    }

        	    if (ts_input_cfg[3].error_indicator == MT_TRUE) {
        		reg.bitc.sync_err_bypass = 0x0;
        		reg.bitc.tei_bypass = 0x0;
        		reg.bitc.err_bypass = 0x0;
        	    } else {
        		reg.bitc.sync_err_bypass = 0x1;
        		reg.bitc.tei_bypass = 0x1;
        		reg.bitc.err_bypass = 0x1;
        	    }

        	    reg.bitc.err_pol = 0x1;
        	    reg.bitc.val_pol = 0x1;
        	    reg.bitc.sync_pol = 0x1;
        	    reg.bitc.serial_sel = 0x0;
        	    if (ts_input_cfg[3].input_way == MT_TRUE) {
        		//parrel
        		reg.bitc.serial_en = 0;
        		reg.bitc.sync_bypass = 0x0;
        		reg.bitc.val_bypass = 0x0;
        		//reg_set_ts3_sample_ctrl(0x078001ff);
        	    } else {
        		//serial
        		reg.bitc.serial_en = 1;
        		//reg_set_ts3_sample_ctrl(0x478001ff);
        	    }
        	    reg.bitc.ts_en = 0x1;
        	    reg_set_ts3_sample_ctrl(reg.all);
        	    if(ts_input_cfg[3].local_sel_edge == MT_FALSE)
        	    {
        	             u32  reg_clk = dmx_inl(tsi_clk_reg);
        	    		reg_clk |= 0x80;
        	    		dmx_outl(tsi_clk_reg, reg_clk);
        	    }
        	}
        	else if (id == 0)
        	{
        	    reg_ts0_sample_ctrl_t reg;
        	    reg.all = 0;
        	    reg_set_ts0_sample_ctrl(0);
        	    reg.bitc.syncon_th = 0xf;
        	    reg.bitc.syncoff_th = 0xf;
        	    reg.bitc.ts_188_reg = 0x1;    //ts length 188
        	    reg.bitc.ts_188_reg_en = 0x0; //hardware detect
        	    reg.bitc.brk_sel = 0x3;
        	    if (ts_input_cfg[0].start_byte_mask == MT_TRUE) {
        		reg.bitc.sop_token = 0x0;
        		reg.bitc.sync_bypass = 0x1; //two line mode set 1
        		reg.bitc.val_bypass = 0x1;  //two line mode set 1
        	    } else {
        		reg.bitc.sop_token = 0x0;
        		reg.bitc.sync_bypass = 0x0;
        		reg.bitc.val_bypass = 0x0;
        	    }

        	    if (ts_input_cfg[0].error_indicator == MT_TRUE) {
        		reg.bitc.sync_err_bypass = 0x0;
        		reg.bitc.tei_bypass = 0x0;
        		reg.bitc.err_bypass = 0x0;
        	    } else {
        		reg.bitc.sync_err_bypass = 0x1;
        		reg.bitc.tei_bypass = 0x1;
        		reg.bitc.err_bypass = 0x1;
        	    }

        	    reg.bitc.err_pol = 0x1;
        	    reg.bitc.val_pol = 0x1;
        	    reg.bitc.sync_pol = 0x1;
        	    reg.bitc.serial_sel = 0x0;
        	    if (ts_input_cfg[0].input_way == MT_TRUE) {
        		//parrel
        		reg.bitc.serial_en = 0;
        		reg.bitc.sync_bypass = 0x0;
        		reg.bitc.val_bypass = 0x0;
        		//reg_set_ts0_sample_ctrl(0x078001ff);
        	    } else {
        		//serial
        		reg.bitc.serial_en = 1;
        		//reg_set_ts0_sample_ctrl(0x478001ff);
        	    }
        	    reg.bitc.ts_en = 0x1;
        	    reg_set_ts0_sample_ctrl(reg.all);
        	    if(ts_input_cfg[0].local_sel_edge == MT_FALSE)
        	    {
        	             u32  reg_clk = dmx_inl(tsi_clk_reg);
        	    		reg_clk |= 0x10;
        	    		dmx_outl(tsi_clk_reg, reg_clk);
        	    }
           }
           return MT_SUCCESS;
    }

    //MT_DBG_DEMUX("hal_dmx_hardware_init ------1\n");
    if (pool_mode == 1) {
		dmx_outl(reg_dmx_tsp_threshold0, 0x03030404);
		dmx_outl(reg_dmx_tsp_threshold1, 0x02);
		
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		reg_value = dmx_inl(reg_dmx_tsp_tag_clr);
		reg_value |= 0xff;
		dmx_outl(reg_dmx_tsp_tag_clr, reg_value);

		reg_value = dmx_inl(reg_dmx_gm_ch_ctrl);
		reg_value &= 0xFFFF0000;
		reg_value |= 0x8888;
		dmx_outl(reg_dmx_gm_ch_ctrl, reg_value);
#else 
		dmx_outl(reg_dmx_tsp_tag_clr, 0x3f);
		dmx_outl(reg_dmx_gm_ch_ctrl, 0x244);		
#endif
    } else if (pool_mode == 0) {		
		//dmx_outl(TSP_THRESHOLD, 0x233);
		dmx_outl(reg_dmx_tsp_threshold0, 0x0);
		dmx_outl(reg_dmx_tsp_threshold1, 0x03030303);
		
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		reg_value = dmx_inl(reg_dmx_tsp_tag_clr);
		reg_value |= 0xff;
		dmx_outl(reg_dmx_tsp_tag_clr, reg_value);
		
		reg_value = dmx_inl(reg_dmx_gm_ch_ctrl);
		reg_value &= 0xFFFF0000;
		reg_value |= 0x8888;
		dmx_outl(reg_dmx_gm_ch_ctrl, reg_value);
#else 
		dmx_outl(reg_dmx_tsp_tag_clr, 0xf);
		dmx_outl(reg_dmx_gm_ch_ctrl, 0x244);		
#endif
    } else {
		dmx_outl(reg_dmx_tsp_threshold0, 0x233);
		dmx_outl(reg_dmx_tsp_threshold1, 0x03030303);
		
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
		reg_value = dmx_inl(reg_dmx_tsp_tag_clr);
		reg_value |= 0xff;
		dmx_outl(reg_dmx_tsp_tag_clr, reg_value);
		
		reg_value = dmx_inl(reg_dmx_gm_ch_ctrl);
		reg_value &= 0xFFFF0000;
		reg_value |= 0x8888;
		dmx_outl(reg_dmx_gm_ch_ctrl, reg_value);
#else 
		dmx_outl(reg_dmx_tsp_tag_clr, 0xf);
		dmx_outl(reg_dmx_gm_ch_ctrl, 0x244);		
#endif
    }
	
    dmx_outl(reg_dmx_tsp_pcrsetn, 0x11);
    //MT_DBG_DEMUX("hal_dmx_hardware_init ------2\n");
    //config tsi clk
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
   {
     ulong clk_base_reg = mt_get_clk_base();
     dmx_outl((clk_base_reg + 0xb004), (dmx_inl(clk_base_reg + 0xb004)  | 0x28));
    }
#endif
    //选择TS0作为输入源，该域置0；
    //选择TS1作为输入源，该域置1；

    for (ts_input_num = 0; ts_input_num < 4; ts_input_num++)
    {
	if (ts_input_num == 1)
	{
	    reg_ts1_sample_ctrl_t reg;
	    reg.all = 0;
	    reg_set_ts1_sample_ctrl(0);
	    reg.bitc.syncon_th = 0xf;
	    reg.bitc.syncoff_th = 0xf;
	    reg.bitc.ts_188_reg = 0x1;    //ts length 188
	    reg.bitc.ts_188_reg_en = 0x0; //hardware detect
	    reg.bitc.brk_sel = 0x3;
	    if (ts_input_cfg[1].start_byte_mask == MT_TRUE) {
		reg.bitc.sop_token = 0x0;
		reg.bitc.sync_bypass = 0x1;
		reg.bitc.val_bypass = 0x1;
	    } else {
		reg.bitc.sop_token = 0x0;
		reg.bitc.sync_bypass = 0x0;
		reg.bitc.val_bypass = 0x0;
	    }

	    if (ts_input_cfg[1].error_indicator == MT_TRUE) {
		reg.bitc.sync_err_bypass = 0x0;
		reg.bitc.tei_bypass = 0x0;
		reg.bitc.err_bypass = 0x0;
	    } else {
		reg.bitc.sync_err_bypass = 0x1;
		reg.bitc.tei_bypass = 0x1;
		reg.bitc.err_bypass = 0x1;
	    }

	    reg.bitc.err_pol = 0x1;
	    reg.bitc.val_pol = 0x1;
	    reg.bitc.sync_pol = 0x1;
	    reg.bitc.serial_sel = 0x0;
	    if (ts_input_cfg[1].input_way == MT_TRUE) {
		//parrel
		reg.bitc.serial_en = 0;
		reg.bitc.sync_bypass = 0x0;
		reg.bitc.val_bypass = 0x0;
		//reg_set_ts1_sample_ctrl(0x078001ff);
	    } else {
		//serial
		reg.bitc.serial_en = 1;
		//reg_set_ts1_sample_ctrl(0x478001ff);
	    }
	    reg.bitc.ts_en = 0x1;
	     //add for dvbs2  sync-data error sometime //add by yuwu 20241219
		 /*fix issue 29991 demux should use sync mode*/
	    /*if(symphony_get_chip_rev() >= CHIP_SYMPHONY6_A1)
  	    {
	    	reg.bitc.sync_err_bypass = 0x0;   
	    	reg.bitc.sync_bypass = 0x1;       
	    	reg.bitc.ts_3line_mode = 0x1;	    	
	    }*/		
	    reg_set_ts1_sample_ctrl(reg.all);
	    if(ts_input_cfg[1].local_sel_edge == MT_FALSE)
	        {
	             u32  reg_clk = dmx_inl(tsi_clk_reg);
	    		reg_clk |= 0x20;
	    		dmx_outl(tsi_clk_reg, reg_clk);
	        }
	}
	else if (ts_input_num == 2)
	{
	    reg_ts2_sample_ctrl_t reg;
	    reg.all = 0;
	    reg_set_ts2_sample_ctrl(0);
	    reg.bitc.syncon_th = 0xf;
	    reg.bitc.syncoff_th = 0xf;
	    reg.bitc.ts_188_reg = 0x1;    //ts length 188
	    reg.bitc.ts_188_reg_en = 0x0; //hardware detect
	    reg.bitc.brk_sel = 0x3;
	    if (ts_input_cfg[2].start_byte_mask == MT_TRUE) {
		reg.bitc.sop_token = 0x0;
		reg.bitc.sync_bypass = 0x1;
		reg.bitc.val_bypass = 0x1;
	    } else {
		reg.bitc.sop_token = 0x0;
		reg.bitc.sync_bypass = 0x0;
		reg.bitc.val_bypass = 0x0;
	    }

	    if (ts_input_cfg[2].error_indicator == MT_TRUE) {
		reg.bitc.sync_err_bypass = 0x0;
		reg.bitc.tei_bypass = 0x0;
		reg.bitc.err_bypass = 0x0;
	    } else {
		reg.bitc.sync_err_bypass = 0x1;
		reg.bitc.tei_bypass = 0x1;
		reg.bitc.err_bypass = 0x1;
	    }

	    reg.bitc.err_pol = 0x1;
	    reg.bitc.val_pol = 0x1;
	    reg.bitc.sync_pol = 0x1;
	    reg.bitc.serial_sel = 0x0;
	    if (ts_input_cfg[2].input_way == MT_TRUE) {
		//parrel
		reg.bitc.serial_en = 0;
		reg.bitc.sync_bypass = 0x0;
		reg.bitc.val_bypass = 0x0;
		//reg_set_ts2_sample_ctrl(0x078001ff);
	    } else {
		//serial
		reg.bitc.serial_en = 1;
		//reg_set_ts2_sample_ctrl(0x478001ff);
	    }
	    reg.bitc.ts_en = 0x1;
	    reg_set_ts2_sample_ctrl(reg.all);
	    if(ts_input_cfg[2].local_sel_edge == MT_FALSE)
	    {
	             u32  reg_clk = dmx_inl(tsi_clk_reg);
	    		reg_clk |= 0x40;
	    		dmx_outl(tsi_clk_reg, reg_clk);
	    }
	}
	else if (ts_input_num == 3)
	{
	    reg_ts3_sample_ctrl_t reg;
	    reg.all = 0;
	    reg_set_ts3_sample_ctrl(0);
	    reg.bitc.syncon_th = 0xf;
	    reg.bitc.syncoff_th = 0xf;
	    reg.bitc.ts_188_reg = 0x1;    //ts length 188
	    reg.bitc.ts_188_reg_en = 0x0; //hardware detect
	    reg.bitc.brk_sel = 0x3;
	    if (ts_input_cfg[3].start_byte_mask == MT_TRUE) {
		reg.bitc.sop_token = 0x0;
		reg.bitc.sync_bypass = 0x1;
		reg.bitc.val_bypass = 0x1;
	    } else {
		reg.bitc.sop_token = 0x0;
		reg.bitc.sync_bypass = 0x0;
		reg.bitc.val_bypass = 0x0;
	    }

	    if (ts_input_cfg[3].error_indicator == MT_TRUE) {
		reg.bitc.sync_err_bypass = 0x0;
		reg.bitc.tei_bypass = 0x0;
		reg.bitc.err_bypass = 0x0;
	    } else {
		reg.bitc.sync_err_bypass = 0x1;
		reg.bitc.tei_bypass = 0x1;
		reg.bitc.err_bypass = 0x1;
	    }

	    reg.bitc.err_pol = 0x1;
	    reg.bitc.val_pol = 0x1;
	    reg.bitc.sync_pol = 0x1;
	    reg.bitc.serial_sel = 0x0;
	    if (ts_input_cfg[3].input_way == MT_TRUE) {
		//parrel
		reg.bitc.serial_en = 0;
		reg.bitc.sync_bypass = 0x0;
		reg.bitc.val_bypass = 0x0;
		//reg_set_ts3_sample_ctrl(0x078001ff);
	    } else {
		//serial
		reg.bitc.serial_en = 1;
		//reg_set_ts3_sample_ctrl(0x478001ff);
	    }
	    reg.bitc.ts_en = 0x1;
	    reg_set_ts3_sample_ctrl(reg.all);
	    if(ts_input_cfg[3].local_sel_edge == MT_FALSE)
	    {
	             u32  reg_clk = dmx_inl(tsi_clk_reg);
	    		reg_clk |= 0x80;
	    		dmx_outl(tsi_clk_reg, reg_clk);
	    }
	}
	else if (ts_input_num == 0)
	{
	    reg_ts0_sample_ctrl_t reg;
	    reg.all = 0;
	    reg_set_ts0_sample_ctrl(0);
	    reg.bitc.syncon_th = 0xf;
	    reg.bitc.syncoff_th = 0xf;
	    reg.bitc.ts_188_reg = 0x1;    //ts length 188
	    reg.bitc.ts_188_reg_en = 0x0; //hardware detect
	    reg.bitc.brk_sel = 0x3;
	    if (ts_input_cfg[0].start_byte_mask == MT_TRUE) {
		reg.bitc.sop_token = 0x0;
		reg.bitc.sync_bypass = 0x1; //two line mode set 1
		reg.bitc.val_bypass = 0x1;  //two line mode set 1
	    } else {
		reg.bitc.sop_token = 0x0;
		reg.bitc.sync_bypass = 0x0;
		reg.bitc.val_bypass = 0x0;
	    }

	    if (ts_input_cfg[0].error_indicator == MT_TRUE) {
		reg.bitc.sync_err_bypass = 0x0;
		reg.bitc.tei_bypass = 0x0;
		reg.bitc.err_bypass = 0x0;
	    } else {
		reg.bitc.sync_err_bypass = 0x1;
		reg.bitc.tei_bypass = 0x1;
		reg.bitc.err_bypass = 0x1;
	    }

	    reg.bitc.err_pol = 0x1;
	    reg.bitc.val_pol = 0x1;
	    reg.bitc.sync_pol = 0x1;
	    reg.bitc.serial_sel = 0x0;
	    if (ts_input_cfg[0].input_way == MT_TRUE) {
		//parrel
		reg.bitc.serial_en = 0;
		reg.bitc.sync_bypass = 0x0;
		reg.bitc.val_bypass = 0x0;
		//reg_set_ts0_sample_ctrl(0x078001ff);
	    } else {
		//serial
		reg.bitc.serial_en = 1;
		//reg_set_ts0_sample_ctrl(0x478001ff);
	    }
	    reg.bitc.ts_en = 0x1;
	    reg_set_ts0_sample_ctrl(reg.all);
	    if(ts_input_cfg[0].local_sel_edge == MT_FALSE)
	    {
	             u32  reg_clk = dmx_inl(tsi_clk_reg);
	    		reg_clk |= 0x10;
	    		dmx_outl(tsi_clk_reg, reg_clk);
	    }
	}
    }

    //mtos_irq_request (IRQ_PTI_SF_ID, _hal_dmx_isr_handle, RISE_EDGE_TRIGGER);
    if(symphony_get_chip_rev() >= CHIP_SYMPHONY6_A1)
    {
	u32 reg_0xbf200080 = reg_get_ts_stop_cnt_len();
	reg_set_ts_stop_cnt_len(reg_0xbf200080 | 0x2490);   //default set tsi0-3 data_bit_width : 1-bit mode	
    }
    return MT_SUCCESS;
}
#endif
static mt_u32 DMX_OsiSetFilterUnit(u8 *data, u8 *mask, u8 *mode, u8 id)
{
    u8 value = 0;
    reg_funit_filter_data_t data_reg = { 0 };
    reg_funit_filter_mask_t mask_reg = { 0 };
    reg_funit_filter_mode_t mode_reg = { 0 };
	
    if (data) {
		data_reg.bitc.filter_data_byte0 = data[3];
		data_reg.bitc.filter_data_byte1 = data[2];
		data_reg.bitc.filter_data_byte2 = data[1];
		data_reg.bitc.filter_data_byte3 = data[0];
    }
	
    if (mask) {
		mask_reg.bitc.filter_mask_byte0 = mask[3];
		mask_reg.bitc.filter_mask_byte1 = mask[2];
		mask_reg.bitc.filter_mask_byte2 = mask[1];
		mask_reg.bitc.filter_mask_byte3 = mask[0];
    }

    //DMX_TEST_PRINT("_hal_dmx_set_funit:%d!!!\n", pfunit->id);
    if (mode) {
		if (mode[3] != 0)
		    value |= 0x8;
		if (mode[2] != 0)
		    value |= 0x4;
		if (mode[1] != 0)
		    value |= 0x2;
		if (mode[0] != 0)
		    value |= 0x1;
		mode_reg.bitc.filter_mode = value & 0xf;
		mode_reg.bitc.filter_root_end = 0;
    }
    reg_set_funit_filter_data(id, data_reg.all);
    reg_set_funit_filter_mask(id, mask_reg.all);
    reg_set_funit_filter_mode(id, mode_reg.all);

    return MT_SUCCESS;
}


static s32 DMX_OsiChanCreateSectionBuffer(mt_u8 ChanId, mt_u8 ChanBuffId, mt_u32 buffSize)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    DMX_ChanSecBuff_S *SecrBuff = NULL;
    mmz_buffer_s RecMmzBuf = { 0 };

    if (buffSize > DMX_SYMPHONY_MAX_SEC_DESC_BUF_SIZE)
    {
        buffSize = DMX_SYMPHONY_MAX_SEC_DESC_BUF_SIZE;
    }
	
    if ((MT_UNF_DMX_CHAN_TYPE_POST == ChanInfo->ChanType) ||
        (MT_UNF_DMX_CHAN_TYPE_PES == ChanInfo->ChanType))
    {	
        if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_PoolBuf", MMZ_OTHERS, buffSize + DMX_SYMPHONY_SEC_DESC_BUF_SIZE, 8, &RecMmzBuf))
        {
        	MT_FATAL_DEMUX("memory allocate failed\n");
        	return MT_ERR_DMX_ALLOC_MEM_FAILED;
        }
    }
    else
    {
        /*
        there are 3 bufs for each section, one by one
        ------------------------------------------------------------------------------------------
        |                sec data buf                     |  sec desc buf  |  reassamble buf(4K) |          
        ------------------------------------------------------------------------------------------
        */
        if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_PoolBuf", MMZ_OTHERS, buffSize + DMX_SYMPHONY_SEC_DESC_BUF_SIZE + DMX_SYMPHONY_SEC_ROLLBACK_BUF_SIZE, 8, &RecMmzBuf))
        {
        	MT_FATAL_DEMUX("memory allocate failed\n");
        	return MT_ERR_DMX_ALLOC_MEM_FAILED;
        }
    }

    memset((mt_u8 *)RecMmzBuf.startVirAddr, 0, RecMmzBuf.size);

    SecrBuff = &pDmxDevOsi->DmxChanSecBuff[ChanBuffId];
    ChanInfo->secBuffId = SecrBuff->BuffId;
    SecrBuff->buf_staddr_reg.bitc.buf_ch_staddr = (RecMmzBuf.startPhyAddr >> 3);
    SecrBuff->buf_size_reg.bitc.disc_ch_size = (DMX_SYMPHONY_SEC_DESC_BUF_SIZE >> 10);
    SecrBuff->buf_size_reg.bitc.data_ch_size = (buffSize >> 10);
    SecrBuff->sec_data_buff.startPhyAddr = RecMmzBuf.startPhyAddr;
    SecrBuff->sec_data_buff.startVirAddr = RecMmzBuf.startVirAddr;
    SecrBuff->sec_data_buff.size = SecrBuff->buf_size_reg.bitc.data_ch_size;
    MT_DBG_DEMUX("StartPhyAddr = %#x, StartVirAddr = %#x, size = %d(k), disc_size = %d(k)\n",
                 SecrBuff->sec_data_buff.startPhyAddr, SecrBuff->sec_data_buff.startVirAddr,
                 SecrBuff->sec_data_buff.size, SecrBuff->buf_size_reg.bitc.disc_ch_size);
    SecrBuff->buf_size_reg.bitc.disc_ch_rptr = 0;
    SecrBuff->buf_size_reg.bitc.data_ch_rptr = 0;
    SecrBuff->desc_num = 0;
    SecrBuff->buf_data_reg.bitc.data_ch_wptr = 0;
    SecrBuff->buf_disc_reg.bitc.disc_ch_wptr = 0;
    SecrBuff->buf_sec_len_reg.bitc.vld_byte = 0;
    SecrBuff->buf_sec_len_reg.bitc.res_length = 0;
    SecrBuff->buf_sec_len_reg.bitc.syntax = 0;
    SecrBuff->buf_ts_int_reg.bitc.ts_inf_cfg = 0x0;//0x4;
    SecrBuff->data_buff_write_p = 0;
    ChanInfo->slot_reg1.bitc.play_ch = SecrBuff->BuffId;
    MT_DBG_DEMUX("SecrBuff->BuffId=%d\n", SecrBuff->BuffId);

    reg_set_bufn_staddr(SecrBuff->BuffId, SecrBuff->buf_staddr_reg.all);
    reg_set_bufn_size(SecrBuff->BuffId, SecrBuff->buf_size_reg.all);
    reg_set_bufn_disc_wptr(SecrBuff->BuffId, 0);
    reg_set_bufn_ts_int_cfg(SecrBuff->BuffId, SecrBuff->buf_ts_int_reg.all);
    reg_set_bufn_data_wptr(SecrBuff->BuffId, 0);
    reg_set_bufn_cursec_len(SecrBuff->BuffId, SecrBuff->buf_sec_len_reg.all);
    SecrBuff->u32IsUsed = DMX_ISUSED_BUSY;
    return MT_SUCCESS;
}

static s32 DMX_OsiChanDeleteSectionBuffer(mt_u8 ChanId, mt_u8 ChanBuffId, mt_u32 buffSize)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    //DMX_ChanInfo_S     *ChanInfo    = &pDmxDevOsi->DmxChanInfo[ChanId];
    DMX_ChanSecBuff_S *SecrBuff = &pDmxDevOsi->DmxChanSecBuff[ChanBuffId];
    //mmz_buffer_s    RecMmzBuf       = {0};
    mmz_buffer_s MmzBuf;
    MmzBuf.startPhyAddr = SecrBuff->sec_data_buff.startPhyAddr;
    MmzBuf.startVirAddr = SecrBuff->sec_data_buff.startVirAddr;
    MT_DBG_DEMUX("MmzBuf.startPhyAddr = 0x%x, MmzBuf.startVirAddr = 0x%x\n", MmzBuf.startPhyAddr, MmzBuf.startVirAddr);
    mt_drv_mmz_unmap_and_release(&MmzBuf);

    SecrBuff->sec_data_buff.startPhyAddr = 0;
    SecrBuff->sec_data_buff.startVirAddr = 0;
    SecrBuff->sec_data_buff.size = 0;

    reg_set_bufn_staddr(SecrBuff->BuffId, 0);
    reg_set_bufn_size(SecrBuff->BuffId, 0);
    reg_set_bufn_disc_wptr(SecrBuff->BuffId, 0);
    reg_set_bufn_ts_int_cfg(SecrBuff->BuffId, 0);
    reg_set_bufn_data_wptr(SecrBuff->BuffId, 0);
    reg_set_bufn_cursec_len(SecrBuff->BuffId, 0);
    SecrBuff->u32IsUsed = DMX_ISUSED_FREE;
    //SecrBuff->BuffId = DMX_INVALID_BUF_ID;
    SecrBuff->data_buff_write_p = 0;
    SecrBuff->desc_buff_write_p = 0;

    return MT_SUCCESS;
}

mt_u32 DMX_OsiChanSetStartCode(mt_u8 avChanId, MT_UNF_VCODEC_TYPE_E vcode_type)
{
    //mt_s32 i = 0;
    mt_u8 index = 0;
    /*
     fram_start_code        mpeg2:  00  00  01  00 ;
            avs:    00  00  01  b6 /  00  00  01  b3
            mpeg4:  00  00  01  b6
            mpeg4s: 00  00  s3    s3=100000xx
            h264: 00  00  01  s4  s5
                      s4=xxx00001 /xxx00101
                      s5=1xxxxxxx
            vc1:    00 00 01 0D
            h265:  00 00 01 x0xxxxxx xxxxxxxx 1xxxxxxx
  */
    //                       byte62 byte52 byte42 byte32   byte61 byte51 byte41 byte31
    mt_u8 FRM_DATA[8][8] = { { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 }, //0.DVB_Audio_Frame
	                     { 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01 }, //1.DVB_Video_MPG
	                     { 0x00, 0x00, 0xb3, 0x01, 0x00, 0x00, 0xb6, 0x01 }, //2.DVB_Video_AVS
	                     { 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0xb6, 0x01 }, //3.DVB_Video_MPG4
	                     { 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x80 }, //4.DVB_Video_MPG4_SHORT
	                     { 0x00, 0x80, 0x05, 0x01, 0x00, 0x80, 0x01, 0x01 }, //5.DVB_Video_H264
	                     { 0x00, 0x00, 0x0d, 0x01, 0x00, 0x00, 0x0d, 0x01 }, //6.DVB_Video_VC1
	                     { 0x80, 0x00, 0x00, 0x01, 0x80, 0x00, 0x00, 0x01 }  //7.DVB_Video_h265
    };

    mt_u8 FRM_MASK[8][8] = { { 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff },
	                     { 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 },
	                     { 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 },
	                     { 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0x00, 0x00 },
	                     { 0xff, 0xff, 0xff, 0x03, 0xff, 0xff, 0xff, 0x03 },
	                     { 0xff, 0x7f, 0xe0, 0x00, 0xff, 0x7f, 0xe0, 0x00 },
	                     { 0xff, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00 },
	                     { 0x7f, 0xff, 0xbf, 0x00, 0x7f, 0xff, 0xbf, 0x00 } };

    //reg_trpp_ch_property_t reg = { 0 };
    //reg_trpp_ch_parse_set_t reg1 = { 0 };
    reg_trpp_ch_start_code1_t reg21 = { 0 };
    reg_trpp_ch_start_code2_t reg22 = { 0 };
    reg_trpp_ch_frm_start_code_m1_t reg31 = { 0 };
    reg_trpp_ch_frm_start_code_m2_t reg32 = { 0 };

    if (vcode_type > MT_UNF_VCODEC_TYPE_BUTT || vcode_type < MT_UNF_VCODEC_TYPE_MPEG2)
	vcode_type = 0;
    switch (vcode_type) {
    case MT_UNF_VCODEC_TYPE_MPEG2:
	index = 1;
	break;
    case MT_UNF_VCODEC_TYPE_MPEG4:
	index = 3;
	break;
    case MT_UNF_VCODEC_TYPE_AVS:
    case MT_UNF_VCODEC_TYPE_AVS2:
	index = 2;
	break;
    case MT_UNF_VCODEC_TYPE_H264:
	index = 5;
	break;
    case MT_UNF_VCODEC_TYPE_VC1:
	index = 6;
	break;
    case MT_UNF_VCODEC_TYPE_HEVC:
	index = 7;
	break;
    default:
	index = 0;
	MT_DBG_DEMUX("no pts !!! \n");
	break;
    }
    reg21.bitc.trpp_ch_fsc_31 = FRM_DATA[index][7];
    reg21.bitc.trpp_ch_fsc_32 = FRM_DATA[index][3];
    reg21.bitc.trpp_ch_fsc_41 = FRM_DATA[index][6];
    reg21.bitc.trpp_ch_fsc_42 = FRM_DATA[index][2];

    reg22.bitc.trpp_ch_fsc_51 = FRM_DATA[index][5];
    reg22.bitc.trpp_ch_fsc_52 = FRM_DATA[index][1];
    reg22.bitc.trpp_ch_fsc_61 = FRM_DATA[index][4];
    reg22.bitc.trpp_ch_fsc_62 = FRM_DATA[index][0];

    reg_set_trpp_ch_start_code1(avChanId, reg21.all);
    reg_set_trpp_ch_start_code2(avChanId, reg22.all);

    reg31.bitc.trpp_ch_fscm_31 = FRM_MASK[index][7];
    reg31.bitc.trpp_ch_fscm_32 = FRM_MASK[index][3];
    reg31.bitc.trpp_ch_fscm_41 = FRM_MASK[index][6];
    reg31.bitc.trpp_ch_fscm_42 = FRM_MASK[index][2];

    reg32.bitc.trpp_ch_fscm_51 = FRM_MASK[index][5];
    reg32.bitc.trpp_ch_fscm_52 = FRM_MASK[index][1];
    reg32.bitc.trpp_ch_fscm_61 = FRM_MASK[index][4];
    reg32.bitc.trpp_ch_fscm_62 = FRM_MASK[index][0];

    reg_set_trpp_ch_frm_start_code_m1(avChanId, reg31.all);
    reg_set_trpp_ch_frm_start_code_m2(avChanId, reg32.all);

    return 0;
}

static s32 DMX_OsiChanSetEsBuffer(mt_u8 ChanId, mt_u32 buffSize, mt_u32 pip_en)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    DMX_ChanEsBuff_S *esBuff = NULL;
    mmz_buffer_s esMmzBuf = { 0 };
    mmz_buffer_s descMmzBuf = { 0 };
    mt_u8 i = 0;
    mt_u8 pes_chnum = 0;
    mt_u32 reg_data;
    mt_s32 align_size = 4096;
    mt_u32 buff_size = buffSize;

    if ((MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType) ||
	(MT_UNF_DMX_CHAN_TYPE_DSS == ChanInfo->ChanType) ||
        (MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
        (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType))
	{
	#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
                
		#ifdef CONFIG_MT_CHIP_SYMPHONY6
		//sym6 as solution size and addr must 256K align
		if(g_pDmxDevOsi->chip_fused_fta == MT_FALSE)
		{
			align_size = 0x40000;
			buff_size = DMX_ROUNDUP(buffSize,align_size);
			MT_WARN_DEMUX("es buffSize=0x%x align(0x%x) to 0x%x\n",buffSize,align_size,buff_size);    
		}
		#endif
        
		if (ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_AUD || ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_AUD_AD)
		{
			if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_ESPoolBuf", MMZ_ZONE_AUDIO, buff_size, align_size, &esMmzBuf))
			{
				MT_FATAL_DEMUX("memory allocate failed\n");
				return MT_ERR_DMX_ALLOC_MEM_FAILED;
			}
			MT_INFO_DEMUX("buff_size=0x%x align_size=0x%x\n",buff_size,align_size);
		} else
	#endif
		{
		#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
			if((MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType) && (pip_en == 1))
			{
				if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_ESPoolBuf_PIP", MMZ_ZONE_PIP, buff_size, align_size, &esMmzBuf))
				{
					MT_FATAL_DEMUX("memory allocate failed\n");
					return MT_ERR_DMX_ALLOC_MEM_FAILED;
        			}
			}
			else
			{
				if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_ESPoolBuf", MMZ_ZONE_AV, buff_size, align_size, &esMmzBuf))
				{
					MT_FATAL_DEMUX("memory allocate failed\n");
					return MT_ERR_DMX_ALLOC_MEM_FAILED;
				}
			}
		#else			
			if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_ESPoolBuf", MMZ_ZONE_AV, buff_size, align_size, &esMmzBuf))
			{
				MT_FATAL_DEMUX("memory allocate failed\n");
				return MT_ERR_DMX_ALLOC_MEM_FAILED;
			}
		#endif
		}
	}
	else if (MT_UNF_DMX_CHAN_TYPE_HW_PES == ChanInfo->ChanType)
    {
		if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_ESPoolBuf", MMZ_OTHERS, buffSize + DMX_SYMPHONY_PES_ROLLBACK_BUF_SIZE, 4096, &esMmzBuf))
		{
			MT_FATAL_DEMUX("memory allocate failed\n");
			return MT_ERR_DMX_ALLOC_MEM_FAILED;
		}
    }

    if ((ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_VID) || (ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_DSS))
    {
        if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_ESDescPoolBuf", MMZ_OTHERS, DMX_SYMPHONY_VES_DESC_BUF_SIZE, 4096, &descMmzBuf)) {
			MT_FATAL_DEMUX("memory allocate failed\n");
			return MT_ERR_DMX_ALLOC_MEM_FAILED;
        }
    }
    else if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
        (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType))
	{
	    if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_ESDescPoolBuf", MMZ_OTHERS, DMX_SYMPHONY_ES_DESC_BUF_SIZE, 4096, &descMmzBuf)) {
			MT_FATAL_DEMUX("memory allocate failed\n");
			return MT_ERR_DMX_ALLOC_MEM_FAILED;
	    }
	}
    else if(ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_HW_PES)
	{
	    if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_ESDescPoolBuf", MMZ_OTHERS, DMX_SYMPHONY_PES_DESC_BUF_SIZE, 4096, &descMmzBuf)) {
			MT_FATAL_DEMUX("memory allocate failed\n");
			return MT_ERR_DMX_ALLOC_MEM_FAILED;
        }
    }

	//audio channel start from 2 for pip
    if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
        (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType))
    {
	    for (i = 2; i < DMX_AV_CHANNEL_CNT; i++) 
	    {
		    esBuff = &pDmxDevOsi->DmxChanEsBuff[i];
		    MT_DBG_DEMUX("esBuff->u32IsUsed=%d\n", esBuff->u32IsUsed);

	        if (esBuff->u32IsUsed == DMX_ISUSED_FREE)
	            break;
	    }
    }
	else
	{
#ifdef CONFIG_MT_VDEC_PIP_SUPPORT
		if((MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType) && (pip_en == 1))
		{
		    esBuff = &pDmxDevOsi->DmxChanEsBuff[1];
		    
	        if (esBuff->u32IsUsed != DMX_ISUSED_FREE)
	        {
	        	MT_ERR_DEMUX("esBuff->u32IsUsed=%d, no free buf\n", esBuff->u32IsUsed);
	        	return MT_ERR_DMX_NOAVAILABLE_BUF;
	        }	            
		}
		else
		{
			/*get free av channel*/
		    for (i = 0; i < DMX_AV_CHANNEL_CNT; i++) 
		    {
		    	if (1 == i)//pDmxDevOsi->DmxChanEsBuff[1] is reserved for pip
		    	{
		    		continue;
		    	}
				
			    esBuff = &pDmxDevOsi->DmxChanEsBuff[i];
			    MT_DBG_DEMUX("esBuff->u32IsUsed=%d\n", esBuff->u32IsUsed);

		        if (esBuff->u32IsUsed == DMX_ISUSED_FREE)
		        {
		            break;
		        }
		    }
		}
#else	
	    /*get free av channel*/
	    for (i = 0; i < DMX_AV_CHANNEL_CNT; i++) 
	    {
		    esBuff = &pDmxDevOsi->DmxChanEsBuff[i];
		    MT_DBG_DEMUX("esBuff->u32IsUsed=%d\n", esBuff->u32IsUsed);

	        if (esBuff->u32IsUsed == DMX_ISUSED_FREE)
	            break;
	    }
#endif
	}

    ChanInfo->avChanId = esBuff->esBuffId;
    pes_chnum = ChanInfo->avChanId;

    esBuff->data_buff.startPhyAddr = esMmzBuf.startPhyAddr;
    esBuff->data_buff.startVirAddr = esMmzBuf.startVirAddr;
    esBuff->data_buff.size = esMmzBuf.size;
    esBuff->ch_id = ChanId;

	if (MT_UNF_DMX_CHAN_TYPE_HW_PES == ChanInfo->ChanType)
    {
        esBuff->data_buff.size = esMmzBuf.size - DMX_SYMPHONY_PES_ROLLBACK_BUF_SIZE;
	}

    esBuff->desc_buff.startPhyAddr = descMmzBuf.startPhyAddr;
    esBuff->desc_buff.startVirAddr = descMmzBuf.startVirAddr;
    esBuff->desc_buff.size = descMmzBuf.size;
    /*
        mt_u32 trpp_ch_es_mode             : 1;
        mt_u32                             : 3;
        mt_u32 trpp_ch_pusi_detect         : 1;
        mt_u32 trpp_ch_pusi_mode           : 1;
        mt_u32 trpp_ch_strid_mod           : 1;
        mt_u32                             : 1;
        mt_u32 trpp_ch_time_info           : 2;
        mt_u32                             : 2;
        mt_u32 trpp_ch_dscrpt_en           : 1;
        mt_u32 trpp_ch_pes_head_en         : 1;
        mt_u32 trpp_ch_fsc_en              : 1;
        mt_u32                             : 1;
        mt_u32 trpp_ch_stream_id           : 8;
        mt_u32 trpp_ch_str_id_msk          : 8;
    */
    if (ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_HW_PES)
    {
    	esBuff->ch_property_reg.bitc.trpp_ch_es_mode = 0; //hw pes
    	esBuff->ch_property_reg.bitc.trpp_ch_dscrpt_en = 1;
    	esBuff->ch_property_reg.bitc.trpp_ch_pes_head_en = 1;
    	esBuff->ch_property_reg.bitc.trpp_ch_fsc_en = 0;
    	esBuff->ch_property_reg.bitc.trpp_ch_stream_id = 0;
    	esBuff->ch_property_reg.bitc.trpp_ch_str_id_msk = DMX_SYMPHONY_STREAM_ID_MASK_HW_PES;
    	esBuff->ch_property_reg.bitc.trpp_ch_strid_mod = 1;
    }
    else 
    {
    	esBuff->ch_property_reg.bitc.trpp_ch_es_mode = 1; //es
    	esBuff->ch_property_reg.bitc.trpp_ch_dscrpt_en = 1;
    	esBuff->ch_property_reg.bitc.trpp_ch_pes_head_en = 1;
    	esBuff->ch_property_reg.bitc.trpp_ch_fsc_en = 1;
    	esBuff->ch_property_reg.bitc.trpp_ch_strid_mod = 1;

        if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType))
    	{
#ifndef CONFIG_MT_USE_DATAPIPE_FLOW
            esBuff->ch_property_reg.bitc.trpp_ch_fsc_en = 0;
#endif
    	    esBuff->ch_property_reg.bitc.trpp_ch_stream_id = DMX_SYMPHONY_STREAM_ID_AUDIO;
    	    esBuff->ch_property_reg.bitc.trpp_ch_str_id_msk = DMX_SYMPHONY_STREAM_ID_MASK_AUDIO;
        }
        else if (MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType)
        {
    	    esBuff->ch_property_reg.bitc.trpp_ch_stream_id = DMX_SYMPHONY_STREAM_ID_VIDEO;
    	    esBuff->ch_property_reg.bitc.trpp_ch_str_id_msk = DMX_SYMPHONY_STREAM_ID_MASK_VIDEO;
        }
		else if (MT_UNF_DMX_CHAN_TYPE_DSS == ChanInfo->ChanType)
        {
            esBuff->ch_property_reg.bitc.trpp_ch_stream_id = 0x00;
            esBuff->ch_property_reg.bitc.trpp_ch_str_id_msk = 0xff;
        }

		//enable trpp_ch_pusi_detect and trpp_ch_pusi_mode2
		//it will detect the pes's head(00 00 01), 
		//if the value is wrong(for example, the encrypt stream is not 00 00 01), 
		//the whole pes will be discarded
		esBuff->ch_property_reg.bitc.trpp_ch_pusi_detect = 1;
		esBuff->ch_property_reg.bitc.trpp_ch_pusi_mode2 = 1;
    }

    ChanInfo->slot_reg1.bitc.play_ch = esBuff->esBuffId;

    MT_DBG_DEMUX("esBuff->esBuffId=0x%x, phyAddr=0x%x, VirAddr=0x%x, Size=0x%x,buffSize=0x%x\n",
                 esBuff->esBuffId, esBuff->data_buff.startPhyAddr, esBuff->data_buff.startVirAddr, esBuff->data_buff.size, buffSize);
    reg_set_trpp_ch_property(esBuff->esBuffId, esBuff->ch_property_reg.all);
    esBuff->ch_parse_set_reg.bitc.trpp_ch_sh_detect  = 1;
    esBuff->ch_parse_set_reg.bitc.trpp_ch_sh_en = 1;
    reg_set_trpp_ch_parse_set(esBuff->esBuffId, esBuff->ch_parse_set_reg.all);

    /*set buffer*/
    reg_set_trpp_ch_data_start_addr_trpp_ch_data_saddr(pes_chnum, ((u32)(esBuff->data_buff.startPhyAddr) >> 3));
    reg_set_trpp_ch_data_end_addr_trpp_ch_data_eaddr(pes_chnum, (u32)(esBuff->data_buff.startPhyAddr) + esBuff->data_buff.size - 1);

    if ((pes_chnum == 0) || (pes_chnum == 1))
    {
        reg_set_trpp_ch_data_start_addr_trpp_ch_data_mem_th(pes_chnum, 0x7); //data_mem_th = 128kbyte
    }
    else
    {
        reg_set_trpp_ch_data_start_addr_trpp_ch_data_mem_th(pes_chnum, 0x2);
    }

    DMX_OsiClearEsBuf(pes_chnum, 0);

    /*
    patch for 128879, when do audio track, desc buf will be looped, if not memset
    desc buf, a wrong apts maybe selected
    */
    memset((u8 *)esBuff->desc_buff.startVirAddr, 0, esBuff->desc_buff.size);
    
    reg_set_trpp_ch_dscrpt_start_addr_trpp_ch_data_saddr(pes_chnum, ((u32)(esBuff->desc_buff.startPhyAddr) >> 3));
    reg_set_trpp_ch_dscrpt_end_addr_trpp_ch_data_eaddr(pes_chnum, (u32)(esBuff->desc_buff.startPhyAddr) + esBuff->desc_buff.size - 1);
    reg_set_trpp_ch_dscrpt_start_addr_trpp_ch_data_mem_th(pes_chnum, 1); //dscrpt_mem_th=1k byre
    DMX_OsiClearDscrptBuf(pes_chnum, 0);
    /*enable parse*/
    reg_data = reg_get_trpp_channel_parse_en();
    reg_set_trpp_channel_parse_en(reg_data | (1 << pes_chnum));

    //trpp_ch_ini_info1-9
    reg_set_trpp_ch1_ini_info1(pes_chnum, 0x8000000c);
    reg_set_trpp_ch1_ini_info2(pes_chnum, 0);
    reg_set_trpp_ch1_ini_info3(pes_chnum, 0);
    reg_set_trpp_ch1_ini_info4(pes_chnum, 0);
    reg_set_trpp_ch1_ini_info5(pes_chnum, 0);
    reg_set_trpp_ch1_ini_info6(pes_chnum, 0);
    reg_set_trpp_ch1_ini_info7_trpp_ch1_chnum(pes_chnum, pes_chnum);
    reg_set_trpp_ch1_ini_info7_trpp_ch1_gotstrid(pes_chnum, 1);
    //4700  1:discard current pes packe   0:do not discard current pes pes packet
    reg_set_trpp_ch1_ini_info7_trpp_ch1_discard_flag(pes_chnum, 1);
    reg_set_trpp_ch1_ini_info8(pes_chnum, 0);
    reg_set_trpp_ch1_ini_info9(pes_chnum, 0xff);

    //dmx_outl(reg_dmx_demux_debug_ctrl, 0x1);
    esBuff->u32IsUsed = DMX_ISUSED_BUSY;
    return MT_SUCCESS;
}

mt_s32 DMX_OsiReadRegister(mt_u32 dmxRegisterOffset)
{
    if (dmxRegisterOffset == 0)
    {
        #ifdef CONFIG_MT_CHIP_ARIA
            return 0xffc60000;
        #else
	     return 0x1f260000;
	 #endif
    }
    //    return reg_dmx_regs_base_vit;
    return (dmx_inl(reg_dmx_regs_base_vit + dmxRegisterOffset));
}

mt_void DMX_OsiWriteRegister(mt_u32 dmxRegisterOffset, mt_u32 value)
{
    dmx_outl(reg_dmx_regs_base_vit + dmxRegisterOffset, value);
}

/***********************************************************************************
* Function      : DMX_OsiInit
* Description   : Initialize demux module
* Input         :
* Output        :
* Return        : MT_SUCCESS:     success
*                     MT_FAILURE:     system error or allocated dma buffer size beyonds limit
* Others:
*                     iicw 0x50 0x23 0x3
***********************************************************************************/
mt_s32 DMX_OsiInit(mt_u32 PoolBufSize, mt_u32 BlockSize)
{
    DMX_DEV_OSI_S *pDmxDevOsi;
    MT_UNF_DMX_PORT_ATTR_S PortAttr;
    MT_UNF_DMX_TSO_PORT_ATTR_S TSOPortAttr;
    DMX_FQ_Info_S *FqInfo;
    mt_u32 BufSize;
    mt_u32 FqDepth;
    mt_u32 FqBufSize;    
    mt_s32 Ret;

    mt_u32 i;
    mt_u32 ProcessID;

    if (MT_NULL != g_pDmxDevOsi) {
        g_pDmxDevOsi->Reference++;
        UpdateProcRef(g_pDmxDevOsi->Reference);
        ProcessID = __task_pid_nr_ns(current, PIDTYPE_PID, NULL);
        MT_INFO_DEMUX("Process (ID : 0x%x )call DMX_OsiInit , g_pDmxDevOsi->Reference= %d\n", ProcessID, g_pDmxDevOsi->Reference);
        return MT_SUCCESS;
    }

    if (MT_SUCCESS != DmxReset()) {
        return MT_ERR_DMX_BUSY;
    }
    DMX_OsiTsiRegisterInit();
    DMX_OsiHardwareInit(1, 0xf,  NULL);

    BufSize = (PoolBufSize + MIN_MMZ_BUF_SIZE - 1) / MIN_MMZ_BUF_SIZE * MIN_MMZ_BUF_SIZE;
    FqDepth = BufSize / (BlockSize + DMX_BUF_INTERVAL) + 1;
    FqBufSize = FqDepth * DMX_FQ_DESC_SIZE;

    Ret = mt_drv_mmz_alloc_and_map("DMX_Dev", MMZ_OTHERS, sizeof(DMX_DEV_OSI_S), 0, &g_sDmxDevBuf);
    if (Ret != MT_SUCCESS || g_sDmxDevBuf.startVirAddr == 0)
    {
        MT_FATAL_DEMUX("Memory zone allocate failed for dmx dev\n");
        return MT_ERR_DMX_ALLOC_MEM_FAILED;
    }

    pDmxDevOsi = (DMX_DEV_OSI_S *)g_sDmxDevBuf.startVirAddr;

    memset((mt_void *)pDmxDevOsi, 0, sizeof(DMX_DEV_OSI_S));

    g_pDmxDevOsi = pDmxDevOsi;

    MT_INIT_MUTEX(&pDmxDevOsi->lock_Channel);
    MT_INIT_MUTEX(&pDmxDevOsi->lock_Filter);
    MT_INIT_MUTEX(&pDmxDevOsi->lock_Key);
    MT_INIT_MUTEX(&pDmxDevOsi->lock_OqBuf);
    MT_INIT_MUTEX(&pDmxDevOsi->lock_AVChan);
	MT_INIT_MUTEX(&pDmxDevOsi->lock_RecChan);
	
    spin_lock_init(&pDmxDevOsi->splock_OqBuf);

    FqInfo = &pDmxDevOsi->DmxFqInfo[DMX_FQ_COMMOM];

    FqInfo->u32IsUsed = 1;
    FqInfo->u32BufPhyAddr = 0;
    FqInfo->u32BufVirAddr = 0;
    FqInfo->u32BufSize = BufSize;
    FqInfo->u32BlockSize = BlockSize;
    FqInfo->u32FQDepth = FqDepth;
    FqInfo->u32FQPhyAddr = FqInfo->u32BufPhyAddr + BufSize;
    FqInfo->u32FQVirAddr = FqInfo->u32BufVirAddr + BufSize;

    //DmxFqStart(DMX_FQ_COMMOM);

    //#if 0
    for (i = 0; i < DMX_TUNERPORT_CNT; i++) {
		PortAttr.enPortMod = (i < DMX_IFPORT_CNT) ? MT_UNF_DMX_PORT_MODE_INTERNAL : MT_UNF_DMX_PORT_MODE_EXTERNAL;
		PortAttr.enPortType = MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188;
		PortAttr.u32SyncLostTh = 1;
		PortAttr.u32SyncLockTh = 5;
		PortAttr.u32TunerInClk = 0;
		PortAttr.u32SerialBitSelector = 0;
		PortAttr.u32TunerErrMod = 0;
		PortAttr.u32UserDefLen1 = 0;
		PortAttr.u32UserDefLen2 = 0;

		//DMX_OsiTunerPortSetAttr(i, &PortAttr);

		//DmxHalDvbPortSetTsCountCtrl(i, TS_COUNT_CRTL_START);
		//DmxHalDvbPortSetErrTsCountCtrl(i, TS_COUNT_CRTL_START);
    }

    for (i = 0; i < DMX_RAMPORT_CNT; i++) {
    	DMX_RamPort_Info_S *PortInfo = &pDmxDevOsi->RamPortInfo[i];

    	init_waitqueue_head(&PortInfo->WaitQueue);
    	PortInfo->DescWrite = 0;
		PortInfo->Read = 0;
		PortInfo->Write = 0;
		PortInfo->ReqAddr = 0;
		PortInfo->ReqLen = 0;
		PortInfo->WaitLen = 0;
		PortInfo->WakeUp = MT_FALSE;
       	PortInfo->GetCount = 0;
		PortInfo->GetValidCount = 0;
		PortInfo->PutCount = 0;

    	PortAttr.enPortMod = MT_UNF_DMX_PORT_MODE_RAM;
#ifdef DMX_RAM_PORT_AUTO_SCAN_SUPPORT
    	PortAttr.enPortType = MT_UNF_DMX_PORT_TYPE_AUTO;
#else
    	PortAttr.enPortType = MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188_204;
#endif
    	PortAttr.u32SyncLostTh = 3;
    	PortAttr.u32SyncLockTh = 7;
    	PortAttr.u32TunerInClk = 0;
    	PortAttr.u32SerialBitSelector = 0;
    	PortAttr.u32TunerErrMod = 0;
    	PortAttr.u32UserDefLen1 = 0;
    	PortAttr.u32UserDefLen2 = 0;

    	//DMX_OsiRamPortSetAttr(i, &PortAttr);

    	//DmxHalIPPortEnableInt(i);
    	//DmxHalIPPortSetIntCnt(i, DMX_DEFAULT_INT_CNT);

    	//DmxHalIPPortSetTsCountCtrl(i, MT_TRUE);
    }

    for (i = 0; i < DMX_TSOPORT_CNT; i++) {
		TSOPortAttr.bEnable = MT_TRUE;
		TSOPortAttr.bClkReverse = MT_TRUE;
		TSOPortAttr.enTSSource = MT_UNF_DMX_PORT_IF_0;
		TSOPortAttr.enClkMode = MT_UNF_DMX_TSO_CLK_MODE_NORMAL;
		TSOPortAttr.enValidMode = MT_UNF_DMX_TSO_VALID_ACTIVE_OUTPUT;
		TSOPortAttr.bBitSync = MT_TRUE;
		TSOPortAttr.bSerial = MT_TRUE;
		TSOPortAttr.enBitSelector = MT_UNF_DMX_TSO_SERIAL_BIT_0;
		TSOPortAttr.bLSB = MT_FALSE;
    }
	
	for (i = 0; i < DMX_CNT; i++) {
		DMX_Sub_DevInfo_S *DmxInfo = &pDmxDevOsi->SubDevInfo[i];
	
		DmxInfo->PortMode = DMX_PORT_MODE_BUTT;
		DmxInfo->PortId = DMX_INVALID_PORT_ID;
	}
	
    for (i = 0; i < DMX_REC_CNT; i++) {
		DMX_RecInfo_S *RecInfo = &pDmxDevOsi->DmxRecInfo[i];
			
		RecInfo->DmxId = DMX_INVALID_DEMUX_ID;
		RecInfo->RecId = DMX_INVALID_CHAN_ID;
	    RecInfo->interrupt_count = 0;
		
		MT_INIT_MUTEX(&RecInfo->LockRec);

		pDmxDevOsi->bPvrRecBuffZoneIsUsed[i] = MT_FALSE;
    }

    for (i = 0; i < DMX_CHANNEL_CNT; i++) {
    	DMX_ChanInfo_S *ChanInfo = &pDmxDevOsi->DmxChanInfo[i];
    
        ChanInfo->DmxId = DMX_INVALID_DEMUX_ID;
        ChanInfo->ChanPid = DMX_INVALID_PID;
        ChanInfo->ChanId = DMX_INVALID_CHAN_ID;
        ChanInfo->KeyId = DMX_INVALID_KEY_ID;
        ChanInfo->u32ChnResetLock = 0;
        ChanInfo->ChanStatus = MT_UNF_DMX_CHAN_CLOSE;
        ChanInfo->ChanOutMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_BUTT;
        mutex_init(&ChanInfo->chan_mutex);
        init_waitqueue_head(&ChanInfo->hw_pes_wait);
        init_waitqueue_head(&ChanInfo->pes_wait);
    }

    for (i = 0; i < DMX_FILTER_CNT; i++) {
		DMX_FilterInfo_S *FilterInfo = &pDmxDevOsi->DmxFilterInfo[i];

		FilterInfo->ChanId = DMX_INVALID_CHAN_ID;
		FilterInfo->FilterId = DMX_INVALID_FILTER_ID;
		FilterInfo->Depth = 0;
    }
	
    for (i = 0; i < DMX_FILTER_BUFF_CNT; i++) {
		DMX_FilterBuff_S *DmxFilterBuff = &pDmxDevOsi->DmxFilterBuff[i];
		DmxFilterBuff->FilterBuffId = i;
		DmxFilterBuff->u32IsUsed = DMX_ISUSED_FREE;
    }
	
#ifdef DMX_DESCRAMBLER_SUPPORT
    for (i = 0; i < DMX_KEY_CNT; i++) {
		pDmxDevOsi->DmxKeyInfo[i].DescramblerMode.keyslot_tab.even_key_slot_index = MT_KT_SLOT_ID_INVALID;
		pDmxDevOsi->DmxKeyInfo[i].DescramblerMode.keyslot_tab.odd_key_slot_index = MT_KT_SLOT_ID_INVALID;
		DescramblerReset(i, &pDmxDevOsi->DmxKeyInfo[i]);
    }
#endif

    for (i = 0; i < DMX_PCR_CHANNEL_CNT; i++) {
		pDmxDevOsi->DmxPcrInfo[i].DmxId = DMX_INVALID_DEMUX_ID;
    }

    for (i = 0; i < DMX_CHANNEL_CNT; i++) {
    	pDmxDevOsi->DmxChanSecBuff[i].BuffId = i;
    	pDmxDevOsi->DmxChanSecBuff[i].u32IsUsed = DMX_ISUSED_FREE;
    	pDmxDevOsi->DmxChanSecBuff[i].OqWakeUp = MT_FALSE;
    	init_waitqueue_head(&pDmxDevOsi->DmxChanSecBuff[i].OqWaitQueue);
    	MT_DBG_DEMUX("pDmxDevOsi->DmxChanSecBuff[%d].BuffId = %d\n", i, pDmxDevOsi->DmxChanSecBuff[i].BuffId);
    }

    for (i = 0; i < DMX_AV_CHANNEL_CNT; i++)
    {
        pDmxDevOsi->DmxChanEsBuff[i].esBuffId = i;
        pDmxDevOsi->DmxChanEsBuff[i].u32IsUsed = DMX_ISUSED_FREE;
        pDmxDevOsi->DmxChanEsBuff[i].ch_id = DMX_INVALID_CHAN_ID;
    }

    for (i = 0; i < DMX_OQ_CNT; i++) {
    	DMX_OQ_Info_S *OqInfo = &pDmxDevOsi->DmxOqInfo[i];

    	OqInfo->u32OQId = i;
    	OqInfo->OqWakeUp = MT_FALSE;

    	init_waitqueue_head(&OqInfo->OqWaitQueue);
    }

    for (i = 0; i < DMX_FQ_CNT; i++) {
		DMX_FQ_Info_S *FqInfo = &pDmxDevOsi->DmxFqInfo[i];

		spin_lock_init(&FqInfo->LockFq);
    }

	INIT_LIST_HEAD(&pDmxDevOsi->pes_chan_list);
    INIT_LIST_HEAD(&pDmxDevOsi->sec_chan_list);
    INIT_LIST_HEAD(&pDmxDevOsi->ts_chan_list);
    INIT_LIST_HEAD(&pDmxDevOsi->hw_pes_chan_list);
    mutex_init(&pDmxDevOsi->pes_chan_list_mutex);
    mutex_init(&pDmxDevOsi->sec_chan_list_mutex);
    mutex_init(&pDmxDevOsi->ts_chan_list_mutex);
    mutex_init(&pDmxDevOsi->hw_pes_chan_list_mutex);

// default global set
#if 0
    DmxHalEnableAllDavInt();
    DmxHalEnableAllChEopInt();
    DmxHalEnableAllChEnqueInt();

#ifdef MT_DEMUX_PROC_SUPPORT
    DmxHalFQEnableAllOverflowInt();
    memset(&GlobalProcInfo,0x0,sizeof(DMX_Proc_Global_Info_S));
#endif

    DmxHalSetFlushMaxWaitTime(0x2400);
    DmxHalSetDataFakeMod(DMX_ENABLE);
    init_waitqueue_head(&pDmxDevOsi->DmxWaitQueue);
#endif

#ifdef DMX_DESCRAMBLER_SUPPORT
#ifdef DMX_DESCRAMBLER_VERSION_1
    DescInitHardFlag();
#endif

#endif
#if 1
    for (i = 0; i < DMX_RAMPORT_CNT; i++){
        if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_PoolBuf", MMZ_OTHERS, sizeof(dmx_symphony_swtsi_t), 8, &(g_pDmxDevOsi->RamPortInfo[i].MmzBuf))) {
            MT_FATAL_DEMUX("memory allocate failed\n");

            return MT_ERR_DMX_ALLOC_MEM_FAILED;
        }
        g_pDmxDevOsi->RamPortInfo[i].DmxRamPortInfo = (dmx_symphony_swtsi_t *)g_pDmxDevOsi->RamPortInfo[i].MmzBuf.startVirAddr;

		memset((mt_void *)g_pDmxDevOsi->RamPortInfo[i].DmxRamPortInfo, 0, sizeof(dmx_symphony_swtsi_t));
		
        MT_DBG_DEMUX("alloc ->MmzBuf.startPhyAddr=0x%x  0x%x \n", g_pDmxDevOsi->RamPortInfo[i].MmzBuf.startPhyAddr, g_pDmxDevOsi->RamPortInfo[i].MmzBuf.startVirAddr);
        MT_DBG_DEMUX("g_pDmxDevOsi->RamPortInfo[0].DmxRamPortInfo = 0x%x,\n", g_pDmxDevOsi->RamPortInfo[i].DmxRamPortInfo);
    }
#else
    g_pDmxDevOsi->RamPortInfo[0].DmxRamPortInfo = (dmx_symphony_swtsi_t *)dma_alloc_coherent(NULL, sizeof(dmx_symphony_swtsi_t),
                                                                                             &(g_pDmxDevOsi->RamPortInfo[0].DmxRamPortInfoPhyAddr), GFP_KERNEL);
#endif

#if 1

    g_pDmxDevOsi->Reference++;
    UpdateProcRef(g_pDmxDevOsi->Reference);

    ProcessID = __task_pid_nr_ns(current, PIDTYPE_PID, NULL);
    MT_INFO_DEMUX("Process (ID : 0x%x )call DMX_OsiInit , g_pDmxDevOsi->Reference= %d\n", ProcessID, g_pDmxDevOsi->Reference);

#endif

#ifdef CONFIG_MT_CHIP_SYMPHONY6
    g_pDmxDevOsi->chip_fused_fta = DMX_GET_CHIP_FUSED_FTA();
    MT_INFO_DEMUX("chip_fused_fta=%d\n", g_pDmxDevOsi->chip_fused_fta);
#endif

    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DMX_OsiDeInit
* Description   : destroy demux module
* Input         :
* Output        :
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiDeInit(mt_void)
{
    mt_u32 ProcessID;

	if ( (0 == down_interruptible(&g_lock_av_sema)) 
	   &&(0 == down_interruptible(&g_lock_pes_sema)))
	{
	    if (g_pDmxDevOsi) {
			g_pDmxDevOsi->Reference--;
			UpdateProcRef(g_pDmxDevOsi->Reference);
			ProcessID = __task_pid_nr_ns(current, PIDTYPE_PID, NULL);
			MT_INFO_DEMUX("Process (ID : 0x%x )call DMX_OsiDeInit , g_pDmxDevOsi->Reference= %d\n", ProcessID, g_pDmxDevOsi->Reference);

			if (0 == g_pDmxDevOsi->Reference) {
			    DMX_FQ_Info_S *FqInfo = &g_pDmxDevOsi->DmxFqInfo[DMX_FQ_COMMOM];
			    mmz_buffer_s MmzBuf;
			    mt_u32 i;

			    //del_timer(&CheckChnTimeoutTimer);
			    for (i = 0; i < DMX_RAMPORT_CNT; i++) {
					if (0 != g_pDmxDevOsi->RamPortInfo[i].DescPhyAddr) {
					    mmz_buffer_s MmzBuf;

					    MmzBuf.startPhyAddr = g_pDmxDevOsi->RamPortInfo[i].DescPhyAddr;
					    MmzBuf.startVirAddr = (void *)g_pDmxDevOsi->RamPortInfo[i].DescKerAddr;

					    mt_drv_mmz_unmap_and_release(&MmzBuf);
					}

					DmxHalIPPortDisableInt(i);
					DmxHalIPPortSetTsCountCtrl(i, MT_FALSE);
			    }

			    for (i = 0; i < DMX_TUNERPORT_CNT; i++) {
					DmxHalDvbPortSetTsCountCtrl(i, TS_COUNT_CRTL_STOP);
					DmxHalDvbPortSetTsCountCtrl(i, TS_COUNT_CRTL_RESET);

					DmxHalDvbPortSetErrTsCountCtrl(i, TS_COUNT_CRTL_STOP);
					DmxHalDvbPortSetErrTsCountCtrl(i, TS_COUNT_CRTL_RESET);
			    }

			    DmxFqStop(DMX_FQ_COMMOM);

			    MmzBuf.startPhyAddr = FqInfo->u32BufPhyAddr;
			    MmzBuf.startVirAddr =(void *)FqInfo->u32BufVirAddr;

#if !defined(CONFIG_MT_CHIP_SYMPHONY4) && !defined(CONFIG_MT_CHIP_SYMPHONY2) && !defined(CONFIG_MT_CHIP_SYMPHONY6)
			    mt_drv_mmz_unmap_and_release(&MmzBuf);
#endif

                MmzBuf.startPhyAddr = g_sDmxDevBuf.startPhyAddr;
                MmzBuf.startVirAddr = g_sDmxDevBuf.startVirAddr;
                mt_drv_mmz_unmap_and_release(&MmzBuf);
                memset(&g_sDmxDevBuf, 0, sizeof(g_sDmxDevBuf));
			    g_pDmxDevOsi = MT_NULL;
			}
	    }
	}
    return MT_SUCCESS;
}
/*add a parameter: mt_u32 *VirAddr, according to  chipset test team's requirement(by xunrenyun 00214461)*/
mt_s32 DMX_OsiGetPoolBufAddr(mt_u32 *VirAddr, phys_addr_t *PhyAddr, mt_u32 *BufSize)
{
    DMX_FQ_Info_S *FqInfo = &g_pDmxDevOsi->DmxFqInfo[DMX_FQ_COMMOM];
    MT_DBG_DEMUX("--------DMX_OsiGetPoolBufAddr-------\n");
    *VirAddr = FqInfo->u32BufVirAddr;
    *PhyAddr = FqInfo->u32BufPhyAddr;
    *BufSize = FqInfo->u32BufSize;
    MT_DBG_DEMUX("FqInfo->u32BufVirAddr = 0x%x, FqInfo->u32BufPhyAddr = 0x%x,FqInfo->u32BufSize = 0x%x\n",
                 FqInfo->u32BufVirAddr, FqInfo->u32BufPhyAddr, FqInfo->u32BufSize);
    return MT_SUCCESS;
}

mt_void DMX_OsiSetNoPusiEn(MT_BOOL bNoPusiEn)
{
    DmxHalFilterSetSecStuffCtrl(bNoPusiEn);
}

mt_void DMX_OsiSetTei(mt_u32 u32DmxId, MT_BOOL bTei)
{
    DmxHalSetTei(u32DmxId, bTei);
}

mt_void DMX_OsiTSIAttashTSO(mt_u32 TunerPortID, MT_UNF_DMX_TSO_PORT_E TSO)
{
    DMX_TunerPort_Info_S *Tsi = &g_pDmxDevOsi->TunerPortInfo[TunerPortID];
    MT_UNF_DMX_TSO_PORT_ATTR_S *Tso;
    Tsi->bAttachWithTSO = MT_TRUE;
    Tsi->AttachedTSO = TSO;

    Tso = &g_pDmxDevOsi->TSOPortInfo[Tsi->AttachedTSO];

    if ((Tso->enTSSource >= MT_UNF_DMX_PORT_RAM_0) && (Tso->enTSSource < MT_UNF_DMX_PORT_RAM_0 + DMX_RAMPORT_CNT)) {
	Tsi->BPRamPort = Tso->enTSSource;
    }
    /*if attached TSO's source is not RAM port, should reset the Tsi->BPRamPort  as 0*/
    else {
	Tsi->BPRamPort = 0;
    }
}

MT_BOOL DMX_OsiIsTSIAttachTSO(mt_u32 PortId, MT_UNF_DMX_TSO_PORT_E *TSO)
{
    DMX_TunerPort_Info_S *Tsi = &g_pDmxDevOsi->TunerPortInfo[PortId];
    *TSO = Tsi->AttachedTSO;
    return Tsi->bAttachWithTSO;
}

mt_s32 DMX_OsiAttachPort(const mt_u32 DmxId, const DMX_PORT_MODE_E PortMode, const mt_u32 PortId)
{
    int i=0;
    DMX_Sub_DevInfo_S *DmxInfo = &g_pDmxDevOsi->SubDevInfo[DmxId];

    MT_DBG_DEMUX("DMX_OsiAttachPort >>> PortMode: %d, PortId: %d,dmxid:%d\n",PortMode,PortId,DmxId);
#ifdef CONFIG_EMU_FPGA
    HAL_PUT_U32((volatile mt_u32 *)(mt_get_public_base() + 8008), 0x1l);
#endif
    if(DmxInfo->PortId != PortId || DmxInfo->PortMode != PortMode)
    {
        for (i = 0; i < DMX_CHANNEL_CNT; i++) {
	        DMX_ChanInfo_S *ChanInfo = &g_pDmxDevOsi->DmxChanInfo[i];
	        if (ChanInfo->DmxId == DmxId)
	        {
                if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY != (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode)) {
                    continue;
                }
                if (MT_UNF_DMX_CHAN_TYPE_SEC == ChanInfo->ChanType)
                {
                    if (DMX_PORT_MODE_RAM == PortMode)
                    {
                        ChanInfo->slot_reg0.bitc.src = 4 + PortId;
                        reg_set_demux_slotn_cfg0_src(ChanInfo->ChanId, ChanInfo->slot_reg0.bitc.src);						
//                        MT_DBG_DEMUX("RAM:   DmxMgr->SubDevInfo[%x].PortId=0x%x\n", DmxId, DmxMgr->SubDevInfo[DmxId].PortId);
                    } else {
                        ChanInfo->slot_reg0.bitc.src = PortId;
                        reg_set_demux_slotn_cfg0_src(ChanInfo->ChanId, PortId);
                        MT_DBG_DEMUX("DMX_OsiAttachPort 11 >>> PortMode: %d, PortId: %d\n",PortMode,PortId);					
                    }
		        }
	        }
	    }
        DmxInfo->PortMode = PortMode;
        DmxInfo->PortId = PortId;
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiDetachPort(const mt_u32 DmxId)
{
    DMX_Sub_DevInfo_S *DmxInfo = &g_pDmxDevOsi->SubDevInfo[DmxId];
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = MT_NULL;
    mt_u32 i = 0;

    if ((DMX_PORT_MODE_RAM == DmxInfo->PortMode) && (DmxInfo->PortId < DMX_RAMPORT_CNT))
    {
        mt_u32 i;

        for (i = 0; i < DMX_CHANNEL_CNT; i++)
        {
            DMX_ChanInfo_S *ChanInfo = &g_pDmxDevOsi->DmxChanInfo[i];

            if (ChanInfo->DmxId == DmxId) 
            {
                if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY != (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode))
                {
                    continue;
                }

                if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
                    (MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType) ||
                    (MT_UNF_DMX_CHAN_TYPE_DSS == ChanInfo->ChanType) ||
                    (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType))
                {
                    ChanInfo->slot_reg0.bitc.src = 0x0; //Cable: 000/001/010/011   Swtsi: 100/101
                }
            }
        }
    }


    /*
    patch for bug130915, add support for record and live using the same dmx id, so when detach prot, 
    checking there is no channel attached on current port is needed.
    */
    for (i = 0; i < DMX_CHANNEL_CNT; i++)
    {
        ChanInfo = &DmxMgr->DmxChanInfo[i];

        if (ChanInfo->DmxId == DmxId)
        {
            return MT_SUCCESS;
        }
    }

    if (i >= DMX_CHANNEL_CNT)
    {
        DmxInfo->PortMode = DMX_PORT_MODE_BUTT;
        DmxInfo->PortId = DMX_INVALID_PORT_ID;
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiGetPortId(const mt_u32 DmxId, DMX_PORT_MODE_E *PortMode, mt_u32 *PortId)
{
    mt_s32 ret = MT_ERR_DMX_NOATTACH_PORT;
    DMX_Sub_DevInfo_S *DmxInfo = &g_pDmxDevOsi->SubDevInfo[DmxId];

    if (DMX_INVALID_PORT_ID != DmxInfo->PortId) {
	*PortMode = DmxInfo->PortMode;
	*PortId = DmxInfo->PortId;

	ret = MT_SUCCESS;
    } else {
	MT_WARN_DEMUX("the demux not attach with any port, DmxId=%d.\n", DmxId);
    }

    return ret;
}

mt_s32 DMX_OsiTSOPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr)
{
    MT_UNF_DMX_TSO_PORT_ATTR_S *PortInfo = &g_pDmxDevOsi->TSOPortInfo[PortId];

    PortAttr->bEnable = PortInfo->bEnable;
    PortAttr->bClkReverse = PortInfo->bClkReverse;
    PortAttr->enTSSource = PortInfo->enTSSource;
    PortAttr->enClkMode = PortInfo->enClkMode;
    PortAttr->enValidMode = PortInfo->enValidMode;
    PortAttr->bBitSync = PortInfo->bBitSync;
    PortAttr->bSerial = PortInfo->bSerial;
    PortAttr->enBitSelector = PortInfo->enBitSelector;
    PortAttr->bLSB = PortInfo->bLSB;
    PortAttr->enClk = PortInfo->enClk;
    PortAttr->u32ClkDiv = PortInfo->u32ClkDiv;

    return MT_SUCCESS;
}

mt_s32 DMX_OsiTSOPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_TSO_PORT_ATTR_S *PortAttr)
{
    MT_UNF_DMX_TSO_PORT_ATTR_S *PortInfo = &g_pDmxDevOsi->TSOPortInfo[PortId];

    if ((PortAttr->enClkMode >= MT_UNF_DMX_TSO_CLK_MODE_BUTT) ||
        (PortAttr->enValidMode >= MT_UNF_DMX_TSO_VALID_ACTIVE_BUTT) ||
        (PortAttr->enClk >= MT_UNF_DMX_TSO_CLK_BUTT)) {
	MT_ERR_DEMUX("enClkMode or enValidMode or enClk is invalid\n");
	return MT_ERR_DMX_INVALID_PARA;
    }

    if ((PortAttr->u32ClkDiv < 2) || (PortAttr->u32ClkDiv > 32) || (PortAttr->u32ClkDiv % 2 != 0)) {
	MT_ERR_DEMUX("u32ClkDiv = %d is invalid\n", PortAttr->u32ClkDiv);
	return MT_ERR_DMX_INVALID_PARA;
    }

    if ((PortAttr->enClkMode == MT_UNF_DMX_TSO_CLK_MODE_NORMAL) && (PortAttr->enValidMode == MT_UNF_DMX_TSO_VALID_ACTIVE_HIGH)) {
	MT_ERR_DEMUX("invalid tso port attr , can not config  (enClkMode == MT_UNF_DMX_TSO_CLK_MODE_NORMAL) \
                while (enValidMode == MT_UNF_DMX_TSO_VALID_ACTIVE_HIGH)\n");
	return MT_ERR_DMX_INVALID_PARA;
    }

    if (PortAttr->enTSSource < MT_UNF_DMX_PORT_TSI_0) {
	if (DMX_IFPORT_CNT == 0) {
	    MT_ERR_DEMUX("this chipset have no IF port ,so can not choose enTSSource as IF port\n");
	    return MT_ERR_DMX_INVALID_PARA;
	} else if (PortAttr->enTSSource - MT_UNF_DMX_PORT_IF_0 >= DMX_IFPORT_CNT) {
	    MT_ERR_DEMUX("if enTSSource is MT_UNF_DMX_PORT_IF_x , must less than 0x%x\n", DMX_IFPORT_CNT);
	    return MT_ERR_DMX_INVALID_PARA;
	}
    } else if ((PortAttr->enTSSource >= MT_UNF_DMX_PORT_TSI_0) && (PortAttr->enTSSource <= MT_UNF_DMX_PORT_TSI_7)) {
	if (PortAttr->enTSSource - MT_UNF_DMX_PORT_TSI_0 >= DMX_TSIPORT_CNT) {
	    MT_ERR_DEMUX("if enTSSource is MT_UNF_DMX_PORT_TS_x , must less than 0x%x\n", MT_UNF_DMX_PORT_TSI_0 + DMX_TSIPORT_CNT);
	    return MT_ERR_DMX_INVALID_PARA;
	}
    } else if ((PortAttr->enTSSource >= MT_UNF_DMX_PORT_RAM_0) && (PortAttr->enTSSource <= MT_UNF_DMX_PORT_RAM_7)) {
	if (PortAttr->enTSSource - MT_UNF_DMX_PORT_RAM_0 >= DMX_RAMPORT_CNT) {
	    MT_ERR_DEMUX("if enTSSource is MT_UNF_DMX_PORT_RAM_x , must less than 0x%x\n", MT_UNF_DMX_PORT_RAM_0 + DMX_RAMPORT_CNT);
	    return MT_ERR_DMX_INVALID_PARA;
	}
    } else {
	MT_ERR_DEMUX("enTSSource is invalid\n");
	return MT_ERR_DMX_INVALID_PARA;
    }

    PortInfo->bEnable = PortAttr->bEnable;
    PortInfo->bClkReverse = PortAttr->bClkReverse;
    PortInfo->enTSSource = PortAttr->enTSSource;
    PortInfo->enClkMode = PortAttr->enClkMode;
    PortInfo->enValidMode = PortAttr->enValidMode;
    PortInfo->bBitSync = PortAttr->bBitSync;
    PortInfo->bSerial = PortAttr->bSerial;
    PortInfo->enBitSelector = PortAttr->enBitSelector;
    PortInfo->bLSB = PortAttr->bLSB;
    PortInfo->enClk = PortAttr->enClk;
    PortInfo->u32ClkDiv = PortAttr->u32ClkDiv;

    DmxHalTSOPortSetAttr(PortId, PortInfo);
    DmxHalCfgTSOClk(PortId, PortInfo->bClkReverse, (mt_u32)PortInfo->enClk, PortInfo->u32ClkDiv);

    if (PortInfo->enTSSource >= MT_UNF_DMX_PORT_RAM_0) {
	//DMXConfigIPPortRate(PortInfo->enTSSource - MT_UNF_DMX_PORT_RAM_0);
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiTunerPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_PORT_ATTR_S *PortAttr)
{
    DMX_TunerPort_Info_S *PortInfo = &g_pDmxDevOsi->TunerPortInfo[PortId];

    if(PortId == 0)
    {
        PortInfo->PortType = (reg_get_ts0_sample_ctrl_serial_en() == 1) ? MT_UNF_DMX_PORT_TYPE_SERIAL : MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID;
        PortInfo->SyncLockTh = reg_get_ts0_sample_sta_sync_lock();
        PortInfo->SyncLostTh = (reg_get_ts0_sample_sta_sync_lock() == 1) ? 0:1;
        PortInfo->BitSelector = PortAttr->u32SerialBitSelector;
    }
    else if(PortId == 1)
    {
        PortInfo->PortType = (reg_get_ts1_sample_ctrl_serial_en() == 1) ? MT_UNF_DMX_PORT_TYPE_SERIAL : MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID;
        PortInfo->SyncLockTh = reg_get_ts1_sample_sta_sync_lock();
        PortInfo->SyncLostTh = (reg_get_ts1_sample_sta_sync_lock() == 1) ? 0:1;
        PortInfo->BitSelector = PortAttr->u32SerialBitSelector;
    }
    else if(PortId == 2)
    {
        PortInfo->PortType = (reg_get_ts2_sample_ctrl_serial_en() == 1) ? MT_UNF_DMX_PORT_TYPE_SERIAL : MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID;
        PortInfo->SyncLockTh = reg_get_ts2_sample_sta_sync_lock();
        PortInfo->SyncLostTh = (reg_get_ts2_sample_sta_sync_lock() == 1) ? 0:1;
        PortInfo->BitSelector = PortAttr->u32SerialBitSelector;
    }
    else if(PortId == 3)
    {
        PortInfo->PortType = (reg_get_ts3_sample_ctrl_serial_en() == 1) ? MT_UNF_DMX_PORT_TYPE_SERIAL : MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID;
        PortInfo->SyncLockTh = reg_get_ts3_sample_sta_sync_lock();
        PortInfo->SyncLostTh = (reg_get_ts3_sample_sta_sync_lock() == 1) ? 0:1;
        PortInfo->BitSelector = PortAttr->u32SerialBitSelector;
    }
    PortAttr->enPortMod = (PortId < DMX_IFPORT_CNT) ? MT_UNF_DMX_PORT_MODE_INTERNAL : MT_UNF_DMX_PORT_MODE_EXTERNAL;
    PortAttr->enPortType = PortInfo->PortType;
    PortAttr->u32SyncLostTh = PortInfo->SyncLostTh;
    PortAttr->u32SyncLockTh = PortInfo->SyncLockTh;
    PortAttr->u32TunerInClk = PortInfo->TunerInClk;
    PortAttr->u32SerialBitSelector = PortInfo->BitSelector;
    PortAttr->u32TunerErrMod = 0;
    PortAttr->u32UserDefLen1 = 0;
    PortAttr->u32UserDefLen2 = 0;

    return MT_SUCCESS;
}

mt_s32 DMX_OsiTunerPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_PORT_ATTR_S *PortAttr)
{
    dmx_ts_input_t ts_input_cfg = {0};

    ts_input_cfg.input_way = MT_FALSE;
    ts_input_cfg.local_sel_edge = MT_TRUE;
    ts_input_cfg.error_indicator = MT_FALSE;
    ts_input_cfg.start_byte_mask = MT_TRUE;

    switch (PortAttr->enPortType) {
	    case MT_UNF_DMX_PORT_TYPE_PARALLEL_BURST:
	    case MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID:
	    case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188:
	    case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_204:
	    case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188_204:
			ts_input_cfg.input_way = MT_TRUE;
			ts_input_cfg.start_byte_mask = MT_FALSE;
			break;

	    default :
			ts_input_cfg.input_way = MT_FALSE;
			ts_input_cfg.start_byte_mask = MT_TRUE;
			break;

    }

    if (PortAttr->u32TunerInClk > 0) {
	 ts_input_cfg.local_sel_edge = MT_TRUE;
    }

    if(PortId == 0)
    {
        DMX_OsiHardwareInit(1, 0, &ts_input_cfg);
    }
    else if(PortId == 1)
    {
        DMX_OsiHardwareInit(1, 1, &ts_input_cfg);
    }
    else if(PortId == 2)
    {
       DMX_OsiHardwareInit(1, 2, &ts_input_cfg);
    }
    else if(PortId == 3)
    {
        DMX_OsiHardwareInit(1, 3, &ts_input_cfg);
    }
    return MT_SUCCESS;
}

mt_s32 DMX_OsiRamPortGetAttr(const mt_u32 PortId, MT_UNF_DMX_PORT_ATTR_S *PortAttr)
{
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];

    PortAttr->enPortMod = MT_UNF_DMX_PORT_MODE_RAM;
    PortAttr->enPortType = PortInfo->PortType;
    PortAttr->u32SyncLostTh = PortInfo->SyncLostTh;
    PortAttr->u32SyncLockTh = PortInfo->SyncLockTh;
    PortAttr->u32TunerInClk = 0;
    PortAttr->u32SerialBitSelector = 0;
    PortAttr->u32TunerErrMod = 0;
    PortAttr->u32UserDefLen1 = PortInfo->MinLen;
    PortAttr->u32UserDefLen2 = PortInfo->MaxLen;

    return MT_SUCCESS;
}

mt_s32 DMX_OsiRamPortSetAttr(const mt_u32 PortId, const MT_UNF_DMX_PORT_ATTR_S *PortAttr)
{
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];

    switch (PortAttr->enPortType) {
    case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188:
    case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_204:
    case MT_UNF_DMX_PORT_TYPE_PARALLEL_NOSYNC_188_204:
	PortInfo->MinLen = 0;
	PortInfo->MaxLen = 0;

	break;

    case MT_UNF_DMX_PORT_TYPE_AUTO:
#ifdef DMX_RAM_PORT_AUTO_SCAN_SUPPORT
	PortInfo->MinLen = 0;
	PortInfo->MaxLen = 0;

	break;
#else
	MT_ERR_DEMUX("do not support auto scan!\n");
	return MT_ERR_DMX_NOT_SUPPORT;
#endif

    case MT_UNF_DMX_PORT_TYPE_USER_DEFINED:
#ifdef DMX_RAM_PORT_SET_LENGTH_SUPPORT
	if ((PortAttr->u32UserDefLen1 < DMX_RAM_PORT_MIN_LEN) || (PortAttr->u32UserDefLen1 > DMX_RAM_PORT_MAX_LEN) || (PortAttr->u32UserDefLen2 < DMX_RAM_PORT_MIN_LEN) || (PortAttr->u32UserDefLen2 > DMX_RAM_PORT_MAX_LEN)) {
	    MT_ERR_DEMUX("set len error. len1=%u, len2=%u\n", PortAttr->u32UserDefLen1, PortAttr->u32UserDefLen2);

	    return MT_ERR_DMX_INVALID_PARA;
	}

	PortInfo->MinLen = PortAttr->u32UserDefLen1;
	PortInfo->MaxLen = PortAttr->u32UserDefLen2;

	break;
#else
	return MT_ERR_DMX_NOT_SUPPORT;
#endif

    case MT_UNF_DMX_PORT_TYPE_PARALLEL_BURST:
    case MT_UNF_DMX_PORT_TYPE_PARALLEL_VALID:
    case MT_UNF_DMX_PORT_TYPE_SERIAL:
    case MT_UNF_DMX_PORT_TYPE_SERIAL2BIT:
    case MT_UNF_DMX_PORT_TYPE_SERIAL_NOSYNC:
    case MT_UNF_DMX_PORT_TYPE_SERIAL2BIT_NOSYNC:
	return MT_ERR_DMX_NOT_SUPPORT;

    default:
	MT_ERR_DEMUX("Invalid type %u\n", PortAttr->enPortType);

	return MT_ERR_DMX_INVALID_PARA;
    }

    PortInfo->PortType = PortAttr->enPortType;
    PortInfo->SyncLockTh = PortAttr->u32SyncLockTh > DMX_MAX_LOCK_TH ? DMX_MAX_LOCK_TH : PortAttr->u32SyncLockTh;
    PortInfo->SyncLostTh = PortAttr->u32SyncLostTh > DMX_MAX_LOST_TH ? DMX_MAX_LOST_TH : PortAttr->u32SyncLostTh;

    DmxHalIPPortSetAttr(PortId, PortInfo->PortType, PortInfo->SyncLockTh, PortInfo->SyncLostTh);

#ifdef DMX_RAM_PORT_SET_LENGTH_SUPPORT
    if (MT_UNF_DMX_PORT_TYPE_USER_DEFINED == PortAttr->enPortType) {
	DmxHalIPPortSetSyncLen(PortId, PortInfo->MinLen, PortInfo->MaxLen);
    } else {
	DmxHalIPPortSetSyncLen(PortId, DMX_TS_PACKET_LEN, DMX_TS_PACKET_LEN_204);
    }
#endif


#ifdef DMX_RAM_PORT_AUTO_SCAN_SUPPORT
    if (MT_UNF_DMX_PORT_TYPE_AUTO == PortAttr->enPortType) {
	DmxHalIPPortSetAutoScanRegion(PortId, DMX_RAM_PORT_AUTO_REGION, DMX_RAM_PORT_AUTO_STEP_4);
    }
#endif

    return MT_SUCCESS;
}

mt_s32 DMX_OsiTunerPortGetPacketNum(const mt_u32 PortId, mt_u32 *TsPackCnt, mt_u32 *ErrTsPackCnt)
{
    *TsPackCnt = DmxHalDvbPortGetTsPackCount(PortId);
    *ErrTsPackCnt = DmxHalDvbPortGetErrTsPackCount(PortId);

    return MT_SUCCESS;
}

mt_s32 DMX_OsiRamPortGetPacketNum(const mt_u32 PortId, mt_u32 *TsPackCnt)
{
    *TsPackCnt = DmxHalIPPortGetTsPackCount(PortId);

    return MT_SUCCESS;
}

mt_s32 DMX_OsiTsBufferCreate(const mt_u32 PortId, const mt_u32 Size, DMX_MMZ_BUF_S *TsBuf)
{
    mt_s32 ret;
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];
    mt_char BufName[16] = "DMX_TsBuf[0]";
    mt_u32 BufSize = Size;
    mmz_buffer_s MmzBuf;
    mt_s32 align_size = 8;

    if (PortInfo->PhyAddr)
    {
        MT_ERR_DEMUX("TSBuffer has been used by other process:PortId=%u\n", PortId);
        return MT_ERR_DMX_RECREAT_TSBUFFER;
    }

    if ((Size > DMX_MAX_TS_BUFFER_SIZE) || 
        (Size < DMX_MIN_TS_BUFFER_SIZE))
    {
        MT_ERR_DEMUX("TsBuffer size 0x%x invalid, bigger than 0x%x, or less than 0x%x\n",
            Size, DMX_MAX_TS_BUFFER_SIZE, DMX_MIN_TS_BUFFER_SIZE);

        return MT_ERR_DMX_INVALID_PARA;
    }

    /*
    fix bug127496, set av es buf and video descriptor buf to buf full care mode
    bit0: v es buf full mode
    bit1: a es buf full mode
    bit16: v descriptor buf full mode
    bit17: a descriptor buf full mode, if not sym4, do not set to 1, because audio decsriptor
    is not used in sym2
    */
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
    reg_set_swtsi_chn_af_cfg4(PortId, 0x70007);
    reg_set_swtsi_chn_af_cfg5(PortId, 0x7ffff);

	PortInfo->full_care_reg = reg_get_swtsi_chn_af_cfg4(PortId);
    //sym6 as solution size and addr must 256K align
    if(g_pDmxDevOsi->chip_fused_fta == MT_FALSE)
    {
        align_size = 0x40000;
        BufSize = DMX_ROUNDUP(Size,align_size);
        MT_WARN_DEMUX("ts buffer Size=0x%x align(0x%x) to 0x%x\n",Size,align_size,BufSize);    
    }
#elif defined(CONFIG_MT_CHIP_SYMPHONY4)
    reg_set_swtsi_chn_af_cfg4(PortId, 0x30003);
    reg_set_swtsi_chn_af_cfg5(PortId, 0xfff);
#else
    reg_set_swtsi_chn_af_cfg4(PortId, 0x10003);
    reg_set_swtsi_chn_af_cfg5(PortId, 0xfff);
#endif

    ret = mt_drv_mmz_alloc_and_map(BufName, MT_NULL, BufSize, align_size, &MmzBuf);
    if (MT_SUCCESS != ret)
    {
        MT_ERR_DEMUX("malloc 0x%x failed\n", BufSize);
        return MT_ERR_DMX_ALLOC_MEM_FAILED;
    }
    MT_INFO_DEMUX("ts buffer Size=0x%x align(0x%x) to 0x%x\n\n",Size,align_size,BufSize);
    
    PortInfo->PhyAddr = MmzBuf.startPhyAddr;
    PortInfo->KerAddr = (ulong)MmzBuf.startVirAddr;
    PortInfo->BufSize = BufSize;
    PortInfo->ReqAddr = PortInfo->PhyAddr;
	PortInfo->WakeUp = MT_FALSE;
	PortInfo->GetCount = 0;
	PortInfo->GetValidCount = 0;
	PortInfo->PutCount = 0;

    MT_INFO_DEMUX("MmzBuf.startPhyAddr = 0x%x,MmzBuf.startVirAddr=0x%x, BufSize=0x%x\n",
    MmzBuf.startPhyAddr, MmzBuf.startVirAddr, BufSize);
    
    TsBuf->u32BufPhyAddr = PortInfo->PhyAddr;
    TsBuf->u32BufKerVirAddr = PortInfo->KerAddr;
    TsBuf->u32BufSize = PortInfo->BufSize;
    
	//disable dma when create ts buf.
	//avoid configing the dma when it is still working.
	reg_set_swtsi_chn_control_ch_enable(PortId, 0);
    MT_INFO_DEMUX("port %d attach ip buffer success, buffer size=0x%x\n", PortId, PortInfo->BufSize);
  
    return ret;
}

mt_s32 DMX_OsiTsBufferDestroy(const mt_u32 PortId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];

	reg_set_swtsi_chn_af_cfg4(PortId, 0x0);
    reg_set_swtsi_chn_af_cfg5(PortId, 0x0);
    if (PortInfo->PhyAddr)
    {
        mmz_buffer_s MmzBuf;

        MmzBuf.startPhyAddr = PortInfo->PhyAddr;
        MmzBuf.startVirAddr = (void *)PortInfo->KerAddr;        
        mt_drv_mmz_unmap_and_release(&MmzBuf);        
        PortInfo->PhyAddr = 0;
        PortInfo->DescPhyAddr = 0;
        ret = MT_SUCCESS;
    }
	else
    {
        MT_ERR_DEMUX("invalid PhyAddr!\n");
    }

    return ret;
}

static DMX_ChanInfo_S * DMX_OsiGetAudioChanInfo(void)
{
    int i = 0;
    DMX_ChanInfo_S *ChanInfo = NULL;
	
    for (i = 0; i < DMX_CHANNEL_CNT; i++)
    {   
        ChanInfo = &g_pDmxDevOsi->DmxChanInfo[i];
        if (ChanInfo->DmxId >= DMX_CNT)
        {
            break;
        }
		
        if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode))
        {
            if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
                (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType))
            {
                break;
            }
        }
    }
    return ChanInfo;
}

#define WP_COUNT 20
void CheckAudioDesc(mt_u32 PortId)
{
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];

	mt_u32 reg_value;
	static mt_u32 same_count = 0;
	static mt_u32 last_desc_wp = 0;
	mt_u32 desc_wp, desc_rp;
	DMX_ChanInfo_S *ChanInfo = DMX_OsiGetAudioChanInfo();	
	//static mt_u32 rd_buf[WP_COUNT];
	//mt_u8 i = 0;
	
	if (ChanInfo != NULL)
	{		    	
		//check the source
		if ((PortId + 4) == ChanInfo->slot_reg0.bitc.src)
		{
			DMXOsiChnEsDescDataGetReadWrite(ChanInfo->avChanId, &desc_wp, &desc_rp);
			if (last_desc_wp == desc_wp)
			{
				//rd_buf[same_count] = desc_rp;
				same_count++;	
				MT_INFO_DEMUX("same_count=%d\n", same_count);
			}
			else
			{
				MT_INFO_DEMUX("src=%d, same_count=%d, PortInfo->full_care_reg=0x%x\n", ChanInfo->slot_reg0.bitc.src, same_count, PortInfo->full_care_reg);
				same_count = 0;		
				reg_set_swtsi_chn_af_cfg4(PortId, PortInfo->full_care_reg);
			}
			last_desc_wp = desc_wp;
			
			if (same_count >= WP_COUNT)
			{	
				//for some unkown reasons, the audio description's full care will stop ram input
				//so we check the desc_wp, if it don't move for a while, 
				//disable auido discription full care
				if (0x1 == ((reg_get_swtsi_chn_af_cfg4(PortId)>>18) & 0x1))
				{
					reg_value = reg_get_swtsi_chn_af_cfg4(PortId);
					reg_value &= (~(1<<18));
					reg_set_swtsi_chn_af_cfg4(PortId, reg_value);
					MT_INFO_DEMUX("PortId=%d, reg_value=0x%x, disable audio desc full care\n", PortId, reg_value);
					
				#if 0	
					pr_err("[%s %d]PortId=%d, reg_value=0x%x, disable audio desc full care\n", __FUNCTION__, __LINE__, PortId, reg_value);
					pr_err("[%s %d]desc_wp=0x%x, desc_rp=0x%x\n", __FUNCTION__, __LINE__, desc_wp, desc_rp);
					for (i=0; i<WP_COUNT; i++)
					{
						pr_err("0x%x ", rd_buf[i]);
					}
					pr_err("\n");
				#endif																					
				}

				same_count = 0;
				//pr_err("[%s %d]same_count = 0\n", __FUNCTION__, __LINE__);
			}			
	    }
		else
		{
			
		}
	}
}
mt_s32 DMX_OsiTsBufferGet(const mt_u32 PortId, const mt_u32 ReqLen1, DMX_DATA_BUF_S *Buf, const mt_u32 Timeout)
{
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];
    dmx_dma_config_t dma_config = { 0 };
    MT_BOOL state = MT_FALSE;
    mt_u32 ReqLen = ReqLen1;
	mt_s32 ret = 0;

    if (0 == PortInfo->PhyAddr)
    {
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (0 == ReqLen)
    {
        MT_ERR_DEMUX("port %d get buf len is 0\n", PortId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (ReqLen > PortInfo->BufSize)
    {
        MT_ERR_DEMUX("error, ReqLen(0x%x) > BufSize(0x%x)\n", ReqLen, PortInfo->BufSize);
        ReqLen = PortInfo->BufSize;
    }

    ++PortInfo->GetCount;
	/**
	call GetIpFreeDescAndBufLen
	1. check wether TS buffer have enough space for user to get (u32DataSize)
	2. check wether Desc queue have enough free Desc to description the (u32DataSize) TS buffer ,
	each Desc can descript 64K block of buffer.
	3. if this return MT_TRUE,
	the space of PortInfo->ReqAddr and PortInfo->ReqLen will have right value (be writen in this function )
	 */
    if(PortInfo->PutCount > 0)
    {
		CheckVdecHang(PortId);   	
		dma_config.sw_index = PortId;
    	state = DMX_OsiTsBufferPut_DmaStateEnhance(&dma_config);
    	if (state == MT_TRUE) 
		{
            PortInfo->PutCount = 0;
    	}
		else
		{
			if (Timeout > 0)
			{				
				ret = wait_event_interruptible_timeout(PortInfo->WaitQueue, PortInfo->WakeUp, msecs_to_jiffies(Timeout));
				if (-ERESTARTSYS == ret)
		        {
		            MT_INFO_DEMUX("[%s %d]wait interrupted by signal, ret: 0x%x!\n", __FUNCTION__, __LINE__, ret);
		        }
		        else if (0 == ret)
		        {
		            MT_INFO_DEMUX("[%s %d]wait time out\n", __FUNCTION__, __LINE__);
		        }
				else
				{
					MT_INFO_DEMUX("[%s %d]wakeup, PortInfo->WakeUp=%d, time remain: 0x%x!\n", __FUNCTION__, __LINE__, PortInfo->WakeUp, ret);
				}

				//check the state again after wait
				state = DMX_OsiTsBufferPut_DmaStateEnhance(&dma_config);
		    	if (state == MT_TRUE) 
				{
		            PortInfo->PutCount = 0;
		    	}
			}
		}
    }
    else
    {
        state = MT_TRUE;
    }

	//bug32522, the audio's desc is full because of unkown reason.
	//check the audio desc write pointer to disable desc full care 
	CheckAudioDesc(PortId);
	
    //printk("DMX_OsiTsBufferGet >>> state = %d\n",state);
    if(state == MT_TRUE)
    {
        //memset(PortInfo->KerAddr, 0, ReqLen);
        Buf->BufPhyAddr = PortInfo->PhyAddr;
        Buf->BufKerAddr = PortInfo->KerAddr;
        Buf->BufLen = ReqLen;
        PortInfo->ReqAddr = Buf->BufPhyAddr;
        //printk("Buf->BufLen =%x PortInfo->BufSize=%x\n",Buf->BufLen,PortInfo->BufSize);
        return MT_SUCCESS;
    }
    else
    {
        Buf->BufPhyAddr =0;
        Buf->BufKerAddr = 0;
        Buf->BufLen = 0;
        return MT_FAILURE;
    }
}

extern mt_void DMX_OsrSaveIPTs(mt_u8 *buf, mt_u32 len, mt_u32 u32PortID);

static MT_BOOL DMX_OsiTsBufferPut_DmaStateEnhance(dmx_dma_config_t *p_dma_config)
{
    //mt_u32 data = 0;
    //reg_swtsi_chn_state_t reg = { 0 };
    //data = reg_get_pvr_int_state();
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[p_dma_config->sw_index];

#if 0
    reg.all = reg_get_swtsi_chn_state(p_dma_config->sw_index);
    if(reg.bitc.ch_busy_state == 0
        && reg.bitc.ch_load_busy_state == 0
        && reg.bitc.ch_buf_data_valid == 0
        && reg.bitc.ch_lln_info_valid == 0)
    {
        reg_set_swtsi_chn_control_ch_enable(p_dma_config->sw_index, 0);
        return MT_TRUE;
    }
    else
#endif
    {
        //printk("reg_get_pvr_int_state >>> data = 0x%x\n",data);
        //if ((data & 0x10000) == 0x10000)
        if(PortInfo->WakeUp)
        {
            PortInfo->WakeUp = MT_FALSE;
            reg_set_swtsi_chn_control_ch_enable(p_dma_config->sw_index, 0);
            return MT_TRUE;
        }
        else
        {
    	    return MT_FALSE;
        }
    }
}

static mt_u32 DMX_OsiTsBufferPut_DmaConfig(const mt_u32 PortId, dmx_dma_config_t *p_dma_config)
{
    reg_swtsi_chn_control_t reg = { 0 };
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];
    //dmx_symphony_swtsi_t *ramPortInfo = NULL;

    if (NULL == p_dma_config) {
		MT_ERR_DEMUX("dmx_symphony_dma_config para error!\n");
		return MT_FAILURE;
    }

    if (p_dma_config->sw_index >= DMX_RAMPORT_CNT) {
		MT_ERR_DEMUX("dmx_symphony_dma_config sw_index error!\n");
		return MT_FAILURE;
    }

    //ramPortInfo = (dmx_symphony_swtsi_t *)mt_drv_mmz_alloc_and_map("DMX_RAMPORT", MT_NULL, sizeof(dmx_symphony_swtsi_t), 8, &MmzBuf);
    memset((mt_void *)PortInfo->DmxRamPortInfo, 0, sizeof(dmx_symphony_swtsi_t));
	
    PortInfo->DmxRamPortInfo->ch_lld_next.bitc.ch_lln_vld = 0;
    PortInfo->DmxRamPortInfo->ch_dbuf_staddr.all = (p_dma_config->mem_address / 8);
    //MT_DBG_DEMUX("p_dma_config->mem_address = 0x%x, 0x%x\n",p_dma_config->mem_address /8,p_dma_config->mem_address);
    if (p_dma_config->ts_format == DMX_TS_188) {
		PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_type = 0x0;
    } else if (p_dma_config->ts_format == DMX_TS_192) {
		PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_type = 0x1;
    } else if (p_dma_config->ts_format == DMX_TS_PES) {
		PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_type = 0x2;
		PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_pid = p_dma_config->pid;
    } else if (p_dma_config->ts_format == DMX_TS_ES) {
		PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_type = 0x3;
		PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_pid = p_dma_config->pid;

		if (p_dma_config->pts > 0) {
		    PortInfo->DmxRamPortInfo->ch_pts = p_dma_config->pts;
		    PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_vpts = MT_TRUE;
		} else {
		    PortInfo->DmxRamPortInfo->ch_pts = 0;
		    PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_vpts = MT_FALSE;
		}

		if (p_dma_config->dts > 0) {
		    PortInfo->DmxRamPortInfo->ch_dts = p_dma_config->dts;
		    PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_vdts = MT_TRUE;
		} else {
		    PortInfo->DmxRamPortInfo->ch_dts = 0;
		    PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_vdts = MT_FALSE;
		}
		PortInfo->DmxRamPortInfo->ch_dbuf_pid.bitc.ch_dbuf_streamid = p_dma_config->stream_id;
    }
	else if (p_dma_config->ts_format == DMX_TS_204) {
		PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_dbuf_type = 0x4;
    }
	
    PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_wpont_care_mode = 0x0;
    PortInfo->DmxRamPortInfo->ch_dbuf_cfg.bitc.ch_node_num = 0;

    PortInfo->DmxRamPortInfo->ch_dbuf_pid.bitc.ch_dbuf_length = p_dma_config->data_length;
	
	MT_INFO_DEMUX(KERN_ERR "PortInfo->MmzBuf.startPhyAddr=0x%llx \n", PortInfo->MmzBuf.startPhyAddr);
    reg_set_swtsi_chn_lln_addr(p_dma_config->sw_index, (mt_u32)(PortInfo->MmzBuf.startPhyAddr) / 8);

    reg.bitc.ch_enable = 0x1;
    reg.bitc.ch_load_en = 0x1;
    reg.bitc.ch_lln_reload_en = 0x0;
    reg.bitc.ch_src_endian = 0x1;//0:big endian, 1:little endian

    if (p_dma_config->ts_format == DMX_TS_188)
    {
		reg.bitc.ch_time_care = 0x0;
    }

    if (p_dma_config->ts_format == DMX_TS_192)
	{
		reg.bitc.ch_time_care = 0x1;
    }

    reg_set_swtsi_chn_control(p_dma_config->sw_index, reg.all);

    return MT_SUCCESS;
}

mt_s32 DMX_OsiTsBufferPut(const mt_u32 PortId, const mt_u32 DataLen, const mt_u32 StartPos, const mt_u16 pid,dmx_ts_data_t date_type, mt_u32  pts, mt_u32 SpecifiedDataAddr)
{
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];

    dmx_dma_config_t dma_config = { 0 };
    mt_u8 index = PortId;

    dma_config.sw_index = index;
    if((mt_u32)PortInfo->PhyAddr % 8 != 0)
    {
        MT_ERR_DEMUX("\r\n DMX_OsiTsBufferPut Error, addr short be 8 byte aligned \n\n\n\n");
        return MT_FAILURE;
    }
    /*
    if(DataLen > 128*1024)
    {
        printk("\r\n DMX_OsiTsBufferPut Error  DataLen > 128k,  too big\n\n");
        return MT_FAILURE;
    }
    */
    if ((StartPos + DataLen) > PortInfo->BufSize)
    {
        MT_ERR_DEMUX("\r\n DMX_OsiTsBufferPut  Error  DataLen = 0x%x,  StartPos = 0x%x, PortInfo->BufSize = 0x%x\n",
                DataLen, StartPos, PortInfo->BufSize);
        return MT_FAILURE;
    }
	
    if (DMX_TS_188 == date_type || DMX_TS_192 == date_type || DMX_TS_204 == date_type)
    {
        dma_config.ts_format = date_type;
        dma_config.data_length = DataLen;
        if(SpecifiedDataAddr != 0)
        {
            dma_config.mem_address =  SpecifiedDataAddr + StartPos;
        }
        else
        {
            dma_config.mem_address = (mt_u32)PortInfo->PhyAddr + StartPos;//src_addr;
        }
    }
    else if (DMX_TS_ES == date_type || DMX_TS_PES == date_type)
    {
        dma_config.ts_format = date_type;
        dma_config.data_length = DataLen;
        if(SpecifiedDataAddr != 0)
        {
            dma_config.mem_address =  SpecifiedDataAddr + StartPos;
        }
        else
        {
            dma_config.mem_address = (mt_u32)PortInfo->PhyAddr + StartPos;//src_addr;
        }
        dma_config.pts = pts;
        dma_config.dts = 0;
        dma_config.pid = pid;
        dma_config.stream_id = 0xe0;
    }
    else
    {
    	MT_ERR_DEMUX("[%s %d]error, date_type=%d\n", __FUNCTION__, __LINE__, date_type);
        return MT_FAILURE;
    }

    //printk("dataLen=%x,%x\n",DataLen,dma_config.mem_address);
    DMX_OsiTsBufferPut_DmaConfig(PortId, &dma_config);
    PortInfo->PutCount = DataLen;

    return MT_SUCCESS;
}

mt_s32 DMX_OsiTsBufferReset(const mt_u32 PortId)
{
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];

    /*
    patch for bug15516, checking swtsi interrupt status before disable DMA,
    if disable DMA directly, get ts buf will fail forever
    */
    if (PortInfo->WakeUp)
    {
        PortInfo->PutCount = 0;
        reg_set_swtsi_chn_control_ch_enable(PortId, 0);
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiTsBufferGetStatus(const mt_u32 PortId, MT_UNF_DMX_TSBUF_STATUS_S *Status)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];
    mt_u32 Head;
    mt_u32 Tail;
//    mt_size_t LockFlag;

//    spin_lock_irqsave(&PortInfo->LockRamPort, LockFlag);
    if (PortInfo->PhyAddr) {
	Status->u32BufAddr = PortInfo->PhyAddr;
	Status->u32BufSize = PortInfo->BufSize;
	Status->u32UsedSize = PortInfo->BufSize;

	if (PortInfo->Read < PortInfo->Write) {
	    Head = PortInfo->Read;
	    Tail = PortInfo->BufSize - PortInfo->Write;

	    Head = (Head <= DMX_TS_BUFFER_GAP) ? 0 : (Head - DMX_TS_BUFFER_GAP);
	    Tail = (Tail <= DMX_TS_BUFFER_GAP) ? 0 : (Tail - DMX_TS_BUFFER_GAP);

	    if (Head < Tail) {
		Status->u32UsedSize = PortInfo->BufSize - Tail;
	    } else {
		Status->u32UsedSize = PortInfo->BufSize - Head;
	    }
	} else if (PortInfo->Read > PortInfo->Write) {
	    Head = PortInfo->Read - PortInfo->Write;
	    if (Head > DMX_TS_BUFFER_GAP) {
		Status->u32UsedSize = DMX_TS_BUFFER_GAP + PortInfo->BufSize - Head;
	    }
	} else {
	    if (0 == PortInfo->Read) {
		Status->u32UsedSize = DMX_TS_BUFFER_GAP;
	    }
	}

	ret = MT_SUCCESS;
    }
//    spin_unlock_irqrestore(&PortInfo->LockRamPort, LockFlag);

    return ret;
}

mt_s32 DMX_OsiGetTsBufferFullCare(mt_u32 port_id, MT_UNF_DMX_AV_CHN_FULL_CARE_S *p_av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S *p_rec_care)
{
	pr_err("[%s %d]port_id=%d\n", __FUNCTION__, __LINE__, port_id);
	p_av_care->all  = reg_get_swtsi_chn_af_cfg4(port_id);
	p_rec_care->all = reg_get_swtsi_chn_af_cfg5(port_id);

	pr_err("[%s %d]p_av_care->all=0x%x, p_rec_care->all=0x%x\n", __FUNCTION__, __LINE__, p_av_care->all, p_rec_care->all);
    return MT_SUCCESS;
}

mt_s32 DMX_OsiSetTsBufferFullCare(mt_u32 port_id, MT_UNF_DMX_AV_CHN_FULL_CARE_S av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S rec_care)
{

	pr_err("[%s %d]port_id=%d, pAvFullCare.all=0x%x, RecFullCare.all=0x%x\n", __FUNCTION__, __LINE__, port_id, av_care.all, rec_care.all);
	reg_set_swtsi_chn_af_cfg4(port_id, av_care.all);
	reg_set_swtsi_chn_af_cfg5(port_id, rec_care.all);

    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DMX_OsiGetFreeFilterBuff
* Description   : apply a new filter
* Input         : DmxId
* Output        : FilterId
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
static mt_u32 DMX_OsiGetFilterBuff(void)
{
    mt_u32 i = 0;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_FilterBuff_S *DmxFilterBuff = NULL;
	
    for (i = 0; i < DMX_FILTER_BUFF_CNT; i++)
    {
        DmxFilterBuff = &DmxDevOsi->DmxFilterBuff[i];
		
        if (DMX_ISUSED_FREE == DmxFilterBuff->u32IsUsed)
        {
            return DmxFilterBuff->FilterBuffId;
        }
    }
	
    return DMX_INVALID_FILTER_ID;
}

/***********************************************************************************
* Function      : DMX_OsiNewFilter
* Description   : apply a new filter
* Input         : DmxId
* Output        : FilterId
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiNewFilter(const mt_u32 DmxId, mt_u32 *FilterId)
{
    mt_s32 ret = MT_ERR_DMX_NOFREE_FILTER;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_FilterInfo_S *FilterInfo = DmxDevOsi->DmxFilterInfo;
    mt_u32 RegionBegin = 0;
    mt_u32 RegionEnd = DMX_FILTER_CNT;
    mt_u32 i;

    if (0 == down_interruptible(&DmxDevOsi->lock_Filter)) 
    {
        for (i = RegionBegin; i < RegionEnd; i++)
        {
	        if (DMX_INVALID_FILTER_ID == FilterInfo[i].FilterId)
            {
                FilterInfo[i].FilterId = i;
                FilterInfo[i].ChanId = DMX_INVALID_CHAN_ID;
                FilterInfo[i].Depth = 0;
                FilterInfo[i].FilterBuffId[0] = DMX_INVALID_CHAN_ID;
                FilterInfo[i].FilterBuffId[1] = DMX_INVALID_CHAN_ID;
                FilterInfo[i].FilterBuffId[2] = DMX_INVALID_CHAN_ID;
                FilterInfo[i].FilterBuffId[3] = DMX_INVALID_CHAN_ID;
                *FilterId = i;
                ret = MT_SUCCESS;
                break;
            }
        }

        up(&DmxDevOsi->lock_Filter);
    }
    else
    {
        MT_DBG_DEMUX("down_interruptible failed!\n");
        ret = MT_ERR_DMX_BUSY;
    }

    return ret;
}

/***********************************************************************************
* Function      :  DMX_OsiDeleteFilter
* Description   :  delete a filter
* Input         :  FilterId
* Output        :
* Return        :  MT_SUCCESS:     success
*                  MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiDeleteFilter(const mt_u32 FilterId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_FilterInfo_S *Filter = &DmxDevOsi->DmxFilterInfo[FilterId];
    mt_s32 i = 0;

    if (0 == down_interruptible(&DmxDevOsi->lock_Filter))
    {
        if (DMX_INVALID_FILTER_ID != Filter->FilterId) 
        {
            DMX_FilterBuff_S *DmxFilterBuff = NULL;

            for (i = 0; i < 4; i++)
            {
                if (Filter->FilterBuffId[i] != DMX_INVALID_CHAN_ID)
                {
                    DmxHalDetachFilter(Filter->FilterBuffId[i], Filter->ChanId);
                    DmxFilterBuff = &DmxDevOsi->DmxFilterBuff[Filter->FilterBuffId[i]];
                    DmxFilterBuff->u32IsUsed = DMX_ISUSED_FREE;
                    Filter->FilterBuffId[i] = DMX_INVALID_CHAN_ID;
                }
            }
			
            reg_set_filtern_config(Filter->FilterId, 0);
            Filter->FilterId = DMX_INVALID_FILTER_ID;
            ret = MT_SUCCESS;            
        }

        up(&DmxDevOsi->lock_Filter);
        
    }
    else
    {
        ret = MT_ERR_DMX_BUSY;
        MT_ERR_DEMUX("down_interruptible failed!\n");
    }

    return ret;
}

/***********************************************************************************
* Function      :  DMX_OsiSetFilterAttr
* Description   :  set filter attr
* Input         :  FilterId, FilterAttr
* Output        :
* Return        :  MT_SUCCESS:     success
*                  MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiSetFilterAttr(const mt_u32 FilterId, const MT_UNF_DMX_FILTER_ATTR_S *FilterAttr)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_FilterInfo_S *Filter = &DmxDevOsi->DmxFilterInfo[FilterId];
    DMX_FilterBuff_S *DmxFilterBuff = NULL;
    mt_u32 i;
    mt_u8 Mask[DMX_FILTER_MAX_DEPTH] = { 0 };
	
    if (FilterAttr->u32FilterDepth > DMX_FILTER_MAX_DEPTH)
    {
        MT_ERR_DEMUX("filter %u set depth is %u too long\n", FilterId, FilterAttr->u32FilterDepth);    
        return MT_ERR_DMX_INVALID_PARA;
    }
	
    for (i = 0; i < FilterAttr->u32FilterDepth; i++)
    {
        if (FilterAttr->au8Negate[i] > 1)
        {
            MT_ERR_DEMUX("filter %u set negate is invalid\n", FilterId);        
            return MT_ERR_DMX_INVALID_PARA;
        }
    }
	
    if (0 == down_interruptible(&DmxDevOsi->lock_Filter))
    {
        mt_u8 closeFlag = 0;
        mt_u32 timeout = 0x10;
		
        if (DMX_INVALID_FILTER_ID != Filter->FilterId)
        {
            Filter->Depth = FilterGetValidDepth(FilterAttr);
			
#if ENBALE_FILTER
            if (1 == reg_get_filtern_config_filter_en(Filter->FilterId))
            {
                closeFlag = 1;
                reg_set_filtern_config_filter_en(Filter->FilterId, 0);
                while((!(dmx_inl(reg_dmx_sf_process_sta) & 0x1)) && timeout--)
                {
                    msleep(10);
                }
                reg_set_filtern_config_filt_sta(Filter->FilterId, 0);
            }
#endif

            for (i = 0; i < DMX_FILTER_MAX_DEPTH; i++)
            {
                MT_DBG_DEMUX("match=%x, mask=%x\n", FilterAttr->au8Match[i], FilterAttr->au8Mask[i]);
                if (i < FilterAttr->u32FilterDepth)
                {
                    mt_u32 Negate = FilterAttr->au8Negate[i];
                    
                    Filter->Match[i] = FilterAttr->au8Match[i];
                    Filter->Mask[i] = FilterAttr->au8Mask[i];
                    Mask[i] = ~(FilterAttr->au8Mask[i]);
                    Filter->Negate[i] = Negate;
                } 
                else
                {
                    Filter->Match[i] = 0;
                    Filter->Mask[i] = 0;
                    Filter->Negate[i] = 0;
                    Mask[i] = 0;
                }
            }

            MT_DBG_DEMUX("Filter->Depth=%d,FilterAttr->u32FilterDepth=%d\n", Filter->Depth, FilterAttr->u32FilterDepth);
    
            if (FilterAttr->u32FilterDepth > 0)
            {
                mt_u8 filterBuffId = 0;
                mt_u8 filterUnitCount = 0;

                if (Filter->Depth <= FilterAttr->u32FilterDepth)
                {
                    /*
                    fix bug134077, instead of giving each filter 4 funits,
                    consider how many funits it actually need.
                    */
                    filterUnitCount = ((Filter->Depth + 3) >> 2);
                    //printk(KERN_ERR "[%s %d] filterUnitCount=%d Filter->Depth=%d,FilterAttr->u32FilterDepth=%d\n",
                    //        __FUNCTION__, __LINE__,filterUnitCount, Filter->Depth, FilterAttr->u32FilterDepth);
                    if(filterUnitCount == 0) //fixed #29443, app may set mask all 0xff get all section data, the Depth will set to 0   
                    {
                        filterUnitCount = 1;//at least set one register for filter all data
                        //printk(KERN_ERR "[%s %d] ----filterUnitCount reset to 1\n",__FUNCTION__, __LINE__);
                    }

                    for (i = 0; i < filterUnitCount; i++)
                    {
                        if (Filter->FilterBuffId[i] == DMX_INVALID_CHAN_ID)
                        {
                            filterBuffId = DMX_OsiGetFilterBuff();

                            if (DMX_INVALID_CHAN_ID == filterBuffId)
                            {
                                MT_WARN_DEMUX("no free funit for filter:%d\n", Filter->FilterId);
								up(&DmxDevOsi->lock_Filter);
                                return MT_ERR_DMX_NOAVAILABLE_BUF;
                            }

                            Filter->FilterBuffId[i] = filterBuffId;
                            DmxFilterBuff = &DmxDevOsi->DmxFilterBuff[filterBuffId];
                            DmxFilterBuff->u32IsUsed = DMX_ISUSED_BUSY;
                        }
                    }
                    
                    for (i = 0; i < filterUnitCount; i++)
                    {
                        DMX_OsiSetFilterUnit(&(Filter->Match[i * 4]), &(Mask[i * 4]), &(Filter->Negate[i * 4]), Filter->FilterBuffId[i]);
                        if (i < (filterUnitCount - 1))
                        {
                            reg_set_funit_filter_mode_filter_next(Filter->FilterBuffId[i], Filter->FilterBuffId[i + 1]);
                        }
                        else
                        {
                            reg_set_funit_filter_mode_filter_root_end(Filter->FilterBuffId[i], 1);
                        }
    					
                        MT_DBG_DEMUX("[%d]set[%d] data[0x%08x],mask[0x%08x],mode[0x%08x]\n", i, Filter->FilterBuffId[i],
                        reg_get_funit_filter_data(Filter->FilterBuffId[i]),
                        reg_get_funit_filter_mask(Filter->FilterBuffId[i]),
                        reg_get_funit_filter_mode(Filter->FilterBuffId[i]));
                    }
        			
                    Filter->config_reg.bitc.filter_root = Filter->FilterBuffId[0];
                }
    			
                ret = MT_SUCCESS;
    	    } 
            else 
            {
                MT_DBG_DEMUX("Filter->Depth : 0 !\n");
                ret = MT_SUCCESS;
            }
	    }
		
#if ENBALE_FILTER

        if (1 == closeFlag)
        {
            reg_set_filtern_config_filter_en(Filter->FilterId, 1);
        }
		
#endif

        up(&DmxDevOsi->lock_Filter);
    }
    else
    {
        ret = MT_ERR_DMX_BUSY;
        MT_ERR_DEMUX("down_interruptible failed!\n");
    }

    return ret;
}

/***********************************************************************************
* Function      :  DMX_OsiGetFilterAttr
* Description   :  get filter attr
* Input         :  FilterId
* Output        :  FilterAttr
* Return        :  MT_SUCCESS:     success
*                  MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiGetFilterAttr(const mt_u32 FilterId, MT_UNF_DMX_FILTER_ATTR_S *FilterAttr)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_FilterInfo_S *Filter = &DmxDevOsi->DmxFilterInfo[FilterId];

    if (0 == down_interruptible(&DmxDevOsi->lock_Filter))
    {
        if (DMX_INVALID_FILTER_ID != Filter->FilterId)
        {
            FilterAttr->u32FilterDepth = Filter->Depth;            
            memcpy(FilterAttr->au8Match, Filter->Match, DMX_FILTER_MAX_DEPTH);
            memcpy(FilterAttr->au8Mask, Filter->Mask, DMX_FILTER_MAX_DEPTH);
            memcpy(FilterAttr->au8Negate, Filter->Negate, DMX_FILTER_MAX_DEPTH);            
            ret = MT_SUCCESS;
        }

	    up(&DmxDevOsi->lock_Filter);
    }
    else
    {
        ret = MT_ERR_DMX_BUSY;
    }

    return ret;
}

/***********************************************************************************
* Function      :  DMX_OsiAttachFilter
* Description   :  attach filter to channel
* Input         :  FilterId ChanId
* Output        :
* Return        :  MT_SUCCESS:     success
*                  MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiAttachFilter(const mt_u32 FilterId, const mt_u32 ChanId)
{
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *Chan = &DmxDevOsi->DmxChanInfo[ChanId];
    DMX_FilterInfo_S *Filter = &DmxDevOsi->DmxFilterInfo[FilterId];

    MT_DBG_DEMUX("%sm, %d, FilterId = %d, ChanId = %d, pid = %d\n", __func__, __LINE__, FilterId, ChanId, Chan->ChanPid);

    if ((DMX_INVALID_CHAN_ID == Chan->ChanId) ||
        (DMX_INVALID_FILTER_ID == Filter->FilterId))
    {
        MT_ERR_DEMUX("ChanId:%d or FilterId:%d invalid!\n", Chan->ChanId, Filter->FilterId);
        return MT_ERR_DMX_INVALID_PARA;
    }
	
    if (DMX_INVALID_CHAN_ID != Filter->ChanId)
    {
        MT_ERR_DEMUX("filter already attached!\n");
        return MT_ERR_DMX_ATTACHED_FILTER;
    }

    if ((MT_UNF_DMX_CHAN_TYPE_SEC != Chan->ChanType) &&
        (MT_UNF_DMX_CHAN_TYPE_ECM_EMM != Chan->ChanType) &&
        (MT_UNF_DMX_CHAN_TYPE_PES != Chan->ChanType) &&
        (MT_UNF_DMX_CHAN_TYPE_POST != Chan->ChanType))
    {
        MT_ERR_DEMUX("invalid channel type\n");
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    if (Chan->FilterCount >= DMX_MAX_FILTER_NUM_PER_CHANNEL)
    {
        MT_ERR_DEMUX("channel %u has maximum filters Chan->FilterCount = %d\n", ChanId, Chan->FilterCount);
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    if (0 != down_interruptible(&DmxDevOsi->lock_Channel))
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        return MT_ERR_DMX_BUSY;
    }

    ++Chan->FilterCount;

    up(&DmxDevOsi->lock_Channel);

    if (0 != down_interruptible(&DmxDevOsi->lock_Filter))
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        return MT_ERR_DMX_BUSY;
    }

    Filter->ChanId = ChanId;
    up(&DmxDevOsi->lock_Filter);	
    Filter->config_reg.bitc.buf_id = Chan->secBuffId;
    Filter->config_reg.bitc.filter_en = 1;
    Filter->AttachFlag = 1;
    reg_set_filtern_config(Filter->FilterId, Filter->config_reg.all);
    
    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DMX_OsiDetachFilter
* Description   : detach filter from a channel
* Input         : FilterId, ChanId
* Output        :
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiDetachFilter(const mt_u32 FilterId, const mt_u32 ChanId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *Chan = &DmxDevOsi->DmxChanInfo[ChanId];
    DMX_FilterInfo_S *Filter = &DmxDevOsi->DmxFilterInfo[FilterId];
//    DMX_FilterBuff_S *DmxFilterBuff = NULL;
//    mt_u32 i = 0;
    mt_u32 timeout = 0x10;
	
    if(Chan->FilterCount == 0)
    {
        return ret;
    }
	
    if (0 == down_interruptible(&DmxDevOsi->lock_Filter))
    {
	    if (DMX_INVALID_FILTER_ID != Filter->FilterId)
        {
	        if (DMX_INVALID_CHAN_ID != Filter->ChanId)
            {
		        if (ChanId == Filter->ChanId)
                {			
#if ENBALE_FILTER
                    reg_set_filtern_config_filter_en(Filter->FilterId, 0);
                    while((!(dmx_inl(reg_dmx_sf_process_sta) & 0x1)) && timeout--)
                    {
                        msleep(10);
                    }
					
                    reg_set_filtern_config_filt_sta(Filter->FilterId, 0);
#endif
                    reg_set_filtern_config(Filter->FilterId, 0);
                    Filter->ChanId = DMX_INVALID_CHAN_ID;
                    Filter->AttachFlag = 0;
                    if (Chan->FilterCount > 0)
                    {
                        Chan->FilterCount--;
                    }
                    ret = MT_SUCCESS;
                }
                else
                {
                    ret = MT_ERR_DMX_UNMATCH_FILTER;
                }
            }
            else
            {
                ret = MT_ERR_DMX_NOATTACH_FILTER;
            }
        }

        up(&DmxDevOsi->lock_Filter);
    }
    else
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        ret = MT_ERR_DMX_BUSY;
    }

    return ret;
}

/***********************************************************************************
* Function      :   DMX_OsiGetFilterChannel
* Description   :  Get  filter attached channel
* Input         : FilterId
* Output        : pChanId
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiGetFilterChannel(const mt_u32 FilterId, mt_u32 *ChanId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_FilterInfo_S *Filter = &DmxDevOsi->DmxFilterInfo[FilterId];

    if (0 == down_interruptible(&DmxDevOsi->lock_Filter)) 
    {
        if (DMX_INVALID_FILTER_ID != Filter->FilterId)
        {
            if ((DMX_INVALID_CHAN_ID != Filter->ChanId) && 
                (1 == Filter->AttachFlag))
            {
                *ChanId = Filter->ChanId;
                ret = MT_SUCCESS;
            }
            else
            {
                ret = MT_ERR_DMX_NOATTACH_FILTER;
            }
        }

        up(&DmxDevOsi->lock_Filter);
    }
    else
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        ret = MT_ERR_DMX_BUSY;
    }

     return ret;
}

/***********************************************************************************
* Function      : DMX_OsiGetFreeFilterNum
* Description   : Get free filter count
* Input         : DmxId
* Output        : FreeCount
* Return        : MT_SUCCESS
*                 MT_FAILURE
* Others:
***********************************************************************************/
mt_s32 DMX_OsiGetFreeFilterNum(const mt_u32 DmxId, mt_u32 *FreeCount)
{
    DMX_FilterInfo_S *FilterInfo = g_pDmxDevOsi->DmxFilterInfo;
    mt_u32 RegionBegin = 0;
    mt_u32 RegionEnd = DMX_FILTER_CNT;
    mt_u32 i;
    
    *FreeCount = 0;
    
    for (i = RegionBegin; i < RegionEnd; i++)
    {
        if (DMX_INVALID_FILTER_ID == FilterInfo[i].FilterId)
        {
            ++(*FreeCount);
        }
    }
    
    return MT_SUCCESS;
}

static mt_s32 DMX_OsiGetValidChannel(mt_u32 DmxId, MT_UNF_DMX_CHAN_ATTR_S *ChanAttr, mt_u32 *ch_id)
{
    int i;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    mt_u32 RegionBegin = 0;
    mt_u32 RegionEnd = DMX_CHANNEL_CNT;

    if ((NULL == ch_id) || (NULL == ChanAttr))
    {
    	return MT_ERR_DMX_INVALID_PARA;
    }
	
    *ch_id = -1;

    if (0 == down_interruptible(&DmxMgr->lock_Channel))
    {
        if (MT_UNF_DMX_CHAN_TYPE_AUD == ChanAttr->enChannelType ||
            MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanAttr->enChannelType || 
            MT_UNF_DMX_CHAN_TYPE_VID == ChanAttr->enChannelType ||
            MT_UNF_DMX_CHAN_TYPE_DSS == ChanAttr->enChannelType ||
            MT_UNF_DMX_CHAN_TYPE_HW_PES == ChanAttr->enChannelType)
        {
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
            mt_s32 ret = MT_ERR_DMX_NOFREE_CHAN;

            if(MT_UNF_DMX_CHAN_TYPE_VID == ChanAttr->enChannelType && ChanAttr->enOutputMode == MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY)
            {
                /* keep the first 2 channel used for video play because ch property register was 
                 * already setted in the auxcode statically on the solution of irdeto;   for pip support sym6
                 */
                if ((DmxMgr->DmxChanInfo[0].DmxId == DMX_INVALID_DEMUX_ID) && (DmxMgr->DmxChanInfo[0].ChanId == DMX_INVALID_CHAN_ID)
					&& (0 == ChanAttr->pip_en))
                {
                    DmxMgr->DmxChanInfo[0].DmxId = DmxId;
            		DmxMgr->DmxChanInfo[0].ChanId = 0;
        		    *ch_id = 0;
                    ret = MT_SUCCESS;
                }
                else if ((DmxMgr->DmxChanInfo[1].DmxId == DMX_INVALID_DEMUX_ID) && (DmxMgr->DmxChanInfo[1].ChanId == DMX_INVALID_CHAN_ID)
					&& (1 == ChanAttr->pip_en))
                {
                    DmxMgr->DmxChanInfo[1].DmxId = DmxId;
            		DmxMgr->DmxChanInfo[1].ChanId = 1;
        		    *ch_id = 1;
                    ret = MT_SUCCESS;
                } 
                else
                {
                    MT_ERR_DEMUX("no free vid channel\n");
                }
				
                up(&DmxMgr->lock_Channel);
	            return ret;
            } 
            else if(MT_UNF_DMX_CHAN_TYPE_DSS == ChanAttr->enChannelType)
            {
				RegionBegin = 8;
            }
			else
            {
                RegionBegin = 2;
            }

#else
            RegionBegin = 0;
#endif
        }
        else
        {
            RegionBegin = 16;
        }

    	for (i = RegionBegin; i < RegionEnd; i++)
        {
    	    if ((DMX_INVALID_DEMUX_ID == DmxMgr->DmxChanInfo[i].DmxId)  &&
                (DMX_INVALID_CHAN_ID == DmxMgr->DmxChanInfo[i].ChanId))
            {
                DmxMgr->DmxChanInfo[i].DmxId = DmxId;
                DmxMgr->DmxChanInfo[i].ChanId = i;
                *ch_id = i;
                break;
            }
        }

    	up(&DmxMgr->lock_Channel);
    }
    else
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        return MT_ERR_DMX_BUSY;
    }

    if (i >= RegionEnd)
    {
    	MT_ERR_DEMUX("no free channel\n");
    	return MT_ERR_DMX_NOFREE_CHAN;
    }

    return MT_SUCCESS;
}


/***********************************************************************************
* Function      : DMX_OsiCreateChannel
* Description   : create a channel
* Input         : DmxId
* Output        :
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiCreateChannel(mt_u32 DmxId, MT_UNF_DMX_CHAN_ATTR_S *ChanAttr, DMX_MMZ_BUF_S *ChanBuf, DMX_MMZ_BUF_S *ChanDescBuf, mt_u32 *ChanId)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = MT_NULL;
    MT_UNF_DMX_CHAN_CRC_MODE_E CrcMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
    mt_u32 BufSize;
    mt_u32 ch_id = 0xffffffff;
    mt_s32 ret;

    BufSize = ChanAttr->u32BufSize;
    MT_DBG_DEMUX("BufSize  = 0x%x\n", BufSize);

    if (BufSize < MIN_MMZ_BUF_SIZE)
    {
	    BufSize = MIN_MMZ_BUF_SIZE;
    }
	
    MT_DBG_DEMUX("ChannelType = %d, BufSize = %#x\n", ChanAttr->enChannelType, BufSize);    

    ret = DMX_OsiGetValidChannel(DmxId, ChanAttr, &ch_id);

    if ((-1 == ch_id) || (ret != MT_SUCCESS))
    {
        return MT_ERR_DMX_INVALID_PARA;
    }

    ChanInfo = &DmxMgr->DmxChanInfo[ch_id];
    ChanInfo->APPChanType = MT_UNF_DMX_CHAN_TYPE_BUTT;

    switch (ChanAttr->enChannelType)
    {
        case MT_UNF_DMX_CHAN_TYPE_SEC:
        case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
            MT_DBG_DEMUX("ChanAttr->enCRCMode = 0x%x\n", ChanAttr->enCRCMode);
            CrcMode = ChanAttr->enCRCMode;

            switch (ChanAttr->enCRCMode)
            {
                case MT_UNF_DMX_CHAN_CRC_MODE_FORBID:
                    ChanInfo->slot_reg1.bitc.sec_discard_mode = 0;
                    ChanInfo->slot_reg1.bitc.sec_mode = 0;
                    break;

                case MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD:
                    ChanInfo->slot_reg1.bitc.sec_discard_mode = 0;
                    ChanInfo->slot_reg1.bitc.sec_mode = 1;
                    break;

                case MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_SEND:
                    ChanInfo->slot_reg1.bitc.sec_discard_mode = 1;
                    ChanInfo->slot_reg1.bitc.sec_mode = 1;
                    break;

                case MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD:
                    ChanInfo->slot_reg1.bitc.sec_discard_mode = 0;
                   ChanInfo->slot_reg1.bitc.sec_mode = 2;
                    break;

                case MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_SEND:
                    ChanInfo->slot_reg1.bitc.sec_discard_mode = 1;
                    ChanInfo->slot_reg1.bitc.sec_mode = 2;
                    break;
                
                default:
                    MT_ERR_DEMUX("invalid crc mode %u\n", ChanAttr->enCRCMode);
                    return MT_ERR_DMX_INVALID_PARA;
            }
		
            ChanInfo->slot_reg1.bitc.process_type = DRV_DMX_SLOT_PROCESS_SECTION; // 1:ts  2:pes/es 4:section 8:record;
            ChanInfo->slot_reg0.bitc.errts_del_en = 1;
            break;

        case MT_UNF_DMX_CHAN_TYPE_PES:
        case MT_UNF_DMX_CHAN_TYPE_POST:
        {
            if (BufSize < DMX_PES_TSPACKET_MIN_SIZE)
            {
                BufSize = DMX_PES_TSPACKET_MIN_SIZE;
            }
            else
            {
                BufSize = (BufSize / DMX_PES_TSPACKET_MIN_SIZE) * DMX_PES_TSPACKET_MIN_SIZE;
            }

            ChanInfo->slot_reg1.bitc.process_type = DRV_DMX_SLOT_PROCESS_TS; // 1:ts  2:pes/es 4:section 8:record;
            ChanInfo->slot_reg0.bitc.errts_del_en = 0;

            break;
	    }

        case MT_UNF_DMX_CHAN_TYPE_AUD:
        case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
        case MT_UNF_DMX_CHAN_TYPE_VID:
        case MT_UNF_DMX_CHAN_TYPE_DSS:
        case MT_UNF_DMX_CHAN_TYPE_HW_PES:
        {
            if (BufSize < DMX_PES_CHANNEL_MIN_SIZE)
            {
                BufSize = DMX_PES_CHANNEL_MIN_SIZE;
            }

            ChanInfo->slot_reg1.bitc.process_type = DRV_DMX_SLOT_PROCESS_PESES; // 1:ts  2:pes/es 4:section 8:record;
            ChanInfo->slot_reg0.bitc.errts_del_en = 0;
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_REC:
        {
            ChanInfo->slot_reg1.bitc.process_type = DRV_DMX_SLOT_PROCESS_RECORD; // 1:ts  2:pes/es 4:section 8:record;
            break;
		}

        default:
            ChanInfo->slot_reg0.bitc.errts_del_en = 0;
            MT_ERR_DEMUX("invalid chan type %u\n", ChanAttr->enChannelType);        
            return MT_ERR_DMX_INVALID_PARA;
    }

    switch (ChanAttr->enOutputMode)
    {
        case MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY:
            ChanInfo->ChanOutMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
            MT_DBG_DEMUX("play output mode\n");
            break;

        case MT_UNF_DMX_CHAN_OUTPUT_MODE_REC:
            ChanInfo->ChanOutMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_REC;
            MT_DBG_DEMUX("rec output mode\n");
            break;

        case MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY_REC:
            MT_DBG_DEMUX("play and rec output mode\n");
            break;
        
        default:
            MT_ERR_DEMUX("Invalid output mode %u\n", ChanAttr->enOutputMode);
            return MT_ERR_DMX_INVALID_PARA;
    }

    ChanInfo->slot_reg0.bitc.pid = 0x1fff;
    ChanInfo->slot_reg0.bitc.pid_filter_en = 1;
    ChanInfo->slot_reg0.bitc.pid_filter_mode = 0;
    ChanInfo->slot_reg0.bitc.cc_judge_mode = 0;

    if (DMX_PORT_MODE_RAM == DmxMgr->SubDevInfo[DmxId].PortMode)
    {
		ChanInfo->slot_reg0.bitc.src = 4 + DmxMgr->SubDevInfo[DmxId].PortId;
        MT_DBG_DEMUX("RAM: DmxMgr->SubDevInfo[%x].PortId=0x%x\n", DmxId, DmxMgr->SubDevInfo[DmxId].PortId);
    }
    else
    {
    	ChanInfo->slot_reg0.bitc.src = DmxMgr->SubDevInfo[DmxId].PortId;
    	MT_DBG_DEMUX("TUNER: DmxMgr->SubDevInfo[%x].PortId=0x%x\n", DmxId, DmxMgr->SubDevInfo[DmxId].PortId);
    }

    ChanInfo->slot_reg0.bitc.slot_en = 0;
	ChanInfo->slot_reg0.bitc.rec_ch = 0;
	ChanInfo->slot_reg0.bitc.sc_fetch_ch = 0;
    ChanInfo->slot_reg1.bitc.cw_ch_0_3 = 0;
    ChanInfo->slot_reg1.bitc.cw_ch_4 = 0;
    ChanInfo->slot_reg1.bitc.descrambler_en = 0;
    ChanInfo->slot_reg1.bitc.rec_ch = 0;
    ChanInfo->slot_reg1.bitc.sec_filter_mode = 0;
    ChanInfo->slot_reg1.bitc.multisec_dis = 0;
    ChanInfo->slot_reg1.bitc.play_ch = ChanInfo->DmxId;
    ChanInfo->slot_reg1.bitc.buf_full_mode = 1;
    ChanInfo->slot_reg1.bitc.sc_fetch_en = 0;
    ChanInfo->slot_reg1.bitc.sc_fetch_ch = 0;

    mutex_lock(&ChanInfo->chan_mutex);
    ChanInfo->softpeshead = NULL;
    mutex_unlock(&ChanInfo->chan_mutex);
    ChanInfo->ChanType = ChanAttr->enChannelType;

    *ChanId = ChanInfo->ChanId;
    ChanInfo->secBuffId = DMX_INVALID_BUF_ID;
    ChanInfo->FilterCount = 0;
    ChanInfo->u32BuffWriteP = 0;
    MT_DBG_DEMUX("ChanInfo->ChanType = %d, ChanInfo->ChanId=%d\n", ChanInfo->ChanType, ChanInfo->ChanId);

    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode))
    {
        switch (ChanInfo->ChanType)
        {
            case MT_UNF_DMX_CHAN_TYPE_AUD:
            case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
            case MT_UNF_DMX_CHAN_TYPE_VID:
            case MT_UNF_DMX_CHAN_TYPE_DSS:
            {
                DMX_ChanEsBuff_S *esBuff = NULL;
    
                ret = DMX_OsiChanSetEsBuffer(ChanInfo->ChanId, BufSize, ChanAttr->pip_en);
    			
                if (MT_SUCCESS != ret)
                {
                    MT_FATAL_DEMUX("ChanSetEsBuffer failed\n");   
                    return ret;
                }
    			
        	    esBuff = &DmxMgr->DmxChanEsBuff[ChanInfo->avChanId];
        	    ChanBuf->u32BufPhyAddr = esBuff->data_buff.startPhyAddr;
        	    ChanBuf->u32BufKerVirAddr = (ulong)esBuff->data_buff.startVirAddr;
        	    ChanBuf->u32BufSize = esBuff->data_buff.size;
        	    ChanDescBuf->u32BufPhyAddr = esBuff->desc_buff.startPhyAddr;
        	    ChanDescBuf->u32BufKerVirAddr = (ulong)esBuff->desc_buff.startVirAddr;
        	    ChanDescBuf->u32BufSize = esBuff->desc_buff.size;
    
                MT_DBG_DEMUX("play av mode  ChanInfo->avChanId=%x\n", ChanInfo->avChanId);
                ChanInfo->ChanStatus = MT_UNF_DMX_CHAN_CLOSE;
                ChanInfo->ChanBufSize = ChanBuf->u32BufSize;
                ChanInfo->ChanCrcMode = 0;
                ChanInfo->aes_to_user = ChanAttr->bEsToUser;
                ChanInfo->pts_check_tick = 0;
    
                if (MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType)
                {
					if(copy_to_user(ChanAttr->esBuffId1, &ChanInfo->avChanId, sizeof(mt_u8)))
		            {
		                //return -EFAULT;
		            }
                }
    
               MT_INFO_DEMUX("avChanId = %u, BufPhyAddr = %llx, BufKerVirAddr = %lx, BufSize = %lx\n",
                         ChanInfo->avChanId, ChanBuf->u32BufPhyAddr, ChanBuf->u32BufKerVirAddr, ChanBuf->u32BufSize);

		
                break;
            }

            case MT_UNF_DMX_CHAN_TYPE_HW_PES:
            {
                DMX_ChanEsBuff_S *esBuff = NULL;
    
                ret = DMX_OsiChanSetEsBuffer(ChanInfo->ChanId, BufSize, 0);
    			
                if (MT_SUCCESS != ret)
                {
                    MT_FATAL_DEMUX("ChanSetEsBuffer failed\n");   
                    return ret;
                }
    			
        	    esBuff = &DmxMgr->DmxChanEsBuff[ChanInfo->avChanId];
        	    ChanBuf->u32BufPhyAddr = esBuff->data_buff.startPhyAddr;
        	    ChanBuf->u32BufKerVirAddr = (ulong)esBuff->data_buff.startVirAddr;
        	    ChanBuf->u32BufSize = esBuff->data_buff.size;
        	    ChanDescBuf->u32BufPhyAddr = esBuff->desc_buff.startPhyAddr;
        	    ChanDescBuf->u32BufKerVirAddr = (ulong)esBuff->desc_buff.startVirAddr;
        	    ChanDescBuf->u32BufSize = esBuff->desc_buff.size;
    
                MT_DBG_DEMUX("play av mode  ChanInfo->avChanId=%x\n", ChanInfo->avChanId);
                ChanInfo->ChanStatus = MT_UNF_DMX_CHAN_CLOSE;
                ChanInfo->ChanBufSize = ChanBuf->u32BufSize;
                ChanInfo->ChanCrcMode = 0;
    
                MT_DBG_DEMUX("HWPesChanId = %d, BufPhyAddr = %#x, BufKerVirAddr = %#x, BufSize = %#x\n",
        	                 ChanInfo->avChanId, ChanBuf->u32BufPhyAddr, ChanBuf->u32BufKerVirAddr, ChanBuf->u32BufSize);

                mutex_lock(&DmxMgr->hw_pes_chan_list_mutex);
                list_add_tail(&ChanInfo->chan_node, &DmxMgr->hw_pes_chan_list);
                mutex_unlock(&DmxMgr->hw_pes_chan_list_mutex);
                break;
            }
            case MT_UNF_DMX_CHAN_TYPE_SEC:
            case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
            {
                DMX_ChanSecBuff_S *secBuff = NULL;
    
        	    ret = DMX_OsiChanCreateSectionBuffer(ChanInfo->ChanId, ChanInfo->ChanId, BufSize);
    
                if (MT_SUCCESS != ret)
                {
                    MT_FATAL_DEMUX("ChanCreateSectionBuffer failed\n");   
                    return ret;
                }

                secBuff = &DmxMgr->DmxChanSecBuff[ChanInfo->secBuffId];
                secBuff->pWatchWaitQueue = MT_NULL;
                secBuff->OqWakeUp = MT_FALSE;

                ChanBuf->u32BufPhyAddr = secBuff->sec_data_buff.startPhyAddr;
                ChanBuf->u32BufKerVirAddr = (ulong)secBuff->sec_data_buff.startVirAddr;
                ChanBuf->u32BufSize = (secBuff->sec_data_buff.size << 10);
                MT_DBG_DEMUX("BufPhyAddr = 0x%x, BufKerVirAddr = %#x, BufSize = %#x\n",
                             ChanBuf->u32BufPhyAddr, ChanBuf->u32BufKerVirAddr, ChanBuf->u32BufSize);
    
                ChanInfo->ChanBufSize = ChanBuf->u32BufSize;
                mutex_lock(&DmxMgr->sec_chan_list_mutex);
                list_add_tail(&ChanInfo->chan_node, &DmxMgr->sec_chan_list);
                mutex_unlock(&DmxMgr->sec_chan_list_mutex);
                break;
        	}

            case MT_UNF_DMX_CHAN_TYPE_PES:
            {
                DMX_SOFT_PESHEADER_S *softpeshead = NULL;
                DMX_ChanSecBuff_S *secBuff = NULL;
                mmz_buffer_s MmzBuf;
                mt_u32 i = 0;

        	    ret = DMX_OsiChanCreateSectionBuffer(ChanInfo->ChanId, ChanInfo->ChanId, BufSize);
    
                if (MT_SUCCESS != ret)
                {
                    MT_FATAL_DEMUX("ChanCreateSectionBuffer failed\n");   
                    return ret;
                }

                secBuff = &DmxMgr->DmxChanSecBuff[ChanInfo->secBuffId];
                secBuff->pWatchWaitQueue = MT_NULL;
                secBuff->OqWakeUp = MT_FALSE;

                ChanBuf->u32BufPhyAddr = secBuff->sec_data_buff.startPhyAddr;
                ChanBuf->u32BufKerVirAddr = (ulong)secBuff->sec_data_buff.startVirAddr;
                ChanBuf->u32BufSize = (secBuff->sec_data_buff.size << 10);
        	    MT_DBG_DEMUX("BufPhyAddr = 0x%x, BufKerVirAddr = %#x, BufSize = %#x\n",
                             ChanBuf->u32BufPhyAddr, ChanBuf->u32BufKerVirAddr, ChanBuf->u32BufSize);

                ChanInfo->ChanBufSize = ChanBuf->u32BufSize;
                				
                softpeshead = MT_VMALLOC(MT_ID_DEMUX, sizeof(DMX_SOFT_PESHEADER_S));
                memset(&MmzBuf, 0, sizeof(mmz_buffer_s));

                if (!softpeshead)
                {
                    /*
                    release ts buf
                    */
                    MmzBuf.startPhyAddr = secBuff->sec_data_buff.startPhyAddr;
                    MmzBuf.startVirAddr = secBuff->sec_data_buff.startVirAddr;
                    mt_drv_mmz_unmap_and_release(&MmzBuf);
                    MT_FATAL_DEMUX("memory allocate failed\n");
                    return MT_ERR_DMX_ALLOC_MEM_FAILED;                    
                }

                if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_PoolBuf", MMZ_OTHERS, DMX_SYMPHONY_PES_PARSE_NODE_BUF_SIZE, 8, &MmzBuf))
                {
                    /*
                    release ts buf
                    */
                    MmzBuf.startPhyAddr = secBuff->sec_data_buff.startPhyAddr;
                    MmzBuf.startVirAddr = secBuff->sec_data_buff.startVirAddr;
                    mt_drv_mmz_unmap_and_release(&MmzBuf);
                    MT_FATAL_DEMUX("memory allocate failed\n");
                    return MT_ERR_DMX_ALLOC_MEM_FAILED;
                }

                softpeshead->pesdata = (mt_u8*)MmzBuf.startVirAddr;
                softpeshead->pesphyadr = MmzBuf.startPhyAddr;
                softpeshead->pesphysiz = DMX_SYMPHONY_PES_PARSE_NODE_BUF_SIZE;
                ChanInfo->u32countRef = 0;
				ChanInfo->pes_wait_flag = 0;
				
                ChanDescBuf->u32BufPhyAddr = MmzBuf.startPhyAddr;
                ChanDescBuf->u32BufKerVirAddr = (ulong)MmzBuf.startVirAddr;
                ChanDescBuf->u32BufSize = DMX_SYMPHONY_PES_PARSE_NODE_BUF_SIZE;
                MT_DBG_DEMUX("peshead = %#x, BufKerVirAddr = %#x, BufPhyAddr = %#x, BufSize = %#x\n", softpeshead,
                softpeshead->pesdata, softpeshead->pesphyadr, ChanDescBuf->u32BufSize);


                dmx_spes_resetheader(softpeshead);
                INIT_LIST_HEAD(&softpeshead->pes_free_node_list);  //free pes node list
                INIT_LIST_HEAD(&softpeshead->pes_used_node_list);  //pes data node list, not read by user 
                INIT_LIST_HEAD(&softpeshead->pes_read_node_list);  //pes data read by user, this list uesd for release

                /*
                init the pes_free_node_list, add the node to list one by one
                */
                for (i = 0; i < DMX_PES_NODE_NUM; i++)
                {
                    list_add_tail(&softpeshead->pes_arry[i].pes_node, &softpeshead->pes_free_node_list);
                }

                mutex_lock(&DmxMgr->pes_chan_list_mutex);
                list_add_tail(&ChanInfo->chan_node, &DmxMgr->pes_chan_list);
                mutex_unlock(&DmxMgr->pes_chan_list_mutex);

                ChanInfo->softpeshead = softpeshead;

                break;
            }

            case MT_UNF_DMX_CHAN_TYPE_POST:
            {
                DMX_ChanSecBuff_S *secBuff = NULL;
 
        	    ret = DMX_OsiChanCreateSectionBuffer(ChanInfo->ChanId, ChanInfo->ChanId, BufSize);
    
                if (MT_SUCCESS != ret)
                {
                    MT_FATAL_DEMUX("ChanCreateSectionBuffer failed\n");   
                    return ret;
                }
    			
                secBuff = &DmxMgr->DmxChanSecBuff[ChanInfo->secBuffId];
                secBuff->pWatchWaitQueue = MT_NULL;
                secBuff->OqWakeUp = MT_FALSE;

                ChanBuf->u32BufPhyAddr = secBuff->sec_data_buff.startPhyAddr;
                ChanBuf->u32BufKerVirAddr = (ulong)secBuff->sec_data_buff.startVirAddr;
                ChanBuf->u32BufSize = (secBuff->sec_data_buff.size << 10);
        	    MT_DBG_DEMUX("BufPhyAddr = 0x%x, BufKerVirAddr = %#x, BufSize = %#x\n",
                             ChanBuf->u32BufPhyAddr, ChanBuf->u32BufKerVirAddr, ChanBuf->u32BufSize);
    
                ChanInfo->ChanBufSize = ChanBuf->u32BufSize;

                mutex_lock(&DmxMgr->ts_chan_list_mutex);
                list_add_tail(&ChanInfo->chan_node, &DmxMgr->ts_chan_list);
                mutex_unlock(&DmxMgr->ts_chan_list_mutex);

                break;
            }
            default:
                MT_ERR_DEMUX("ChanInfo->ChanType : %d,  ERROR !!!\n", ChanInfo->ChanType);
                break;
		}

    }
    else
    {
        MT_DBG_DEMUX("rec mode\n");
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiDestroyChannel(mt_u32 ChanId)
{
    mt_u32 i, j;
    mt_s32 ret;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChanId];
    mt_u32 DmxId = ChanInfo->DmxId;
    DMX_FilterBuff_S *DmxFilterBuff = NULL;
	mt_u32 RegionBegin = 0;
	mt_u32 RegionEnd = 0;
	mt_u32 reg_value;
	
    CHECKDMXID(DmxId);
	
    if (MT_UNF_DMX_CHAN_CLOSE != ChanInfo->ChanStatus)
    {
        ret = DMX_OsiCloseChannel(ChanId);
        if (MT_SUCCESS != ret)
        {
            MT_ERR_DEMUX("close chan %d failed 0x%x\n", ChanId, ret);
            return ret;
        }
    }

#ifdef DMX_DESCRAMBLER_SUPPORT
    /*
    detatch all keys attatched to this channel
    */
    if (DMX_INVALID_KEY_ID != ChanInfo->KeyId)
    {
        DMX_OsiDescramblerDetach(ChanInfo->KeyId, ChanId);
        ChanInfo->KeyId = DMX_INVALID_KEY_ID;
    }
#endif

    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode))
    {
    	switch (ChanInfo->ChanType) 
        {
            case MT_UNF_DMX_CHAN_TYPE_AUD:
            case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
            case MT_UNF_DMX_CHAN_TYPE_VID:
            case MT_UNF_DMX_CHAN_TYPE_DSS:
            {
                mmz_buffer_s MmzBuf;

				if(ChanInfo->avChanId >= (DMX_AV_CHANNEL_CNT-1))
				{
					MT_ERR_DEMUX("ChanInfo->avChanId %d > DMX_AV_CHANNEL_CNT \n", ChanInfo->avChanId);
					break;
				}
					
                DMX_ChanEsBuff_S *ChanEsBuff = &DmxMgr->DmxChanEsBuff[ChanInfo->avChanId];

        	    MmzBuf.startPhyAddr = ChanEsBuff->data_buff.startPhyAddr;
        	    MmzBuf.startVirAddr = ChanEsBuff->data_buff.startVirAddr;
        	    MT_DBG_DEMUX("release ES Buff, PhyAddr = %#x, VirAddr = %#x\n", MmzBuf.startPhyAddr, MmzBuf.startVirAddr);
        	    mt_drv_mmz_unmap_and_release(&MmzBuf);

        	    MmzBuf.startPhyAddr = ChanEsBuff->desc_buff.startPhyAddr;
        	    MmzBuf.startVirAddr = ChanEsBuff->desc_buff.startVirAddr;
        	    MT_DBG_DEMUX("release Desc Buff, PhyAddr = %#x, VirAddr = %#x\n", MmzBuf.startPhyAddr, MmzBuf.startVirAddr);
        	    mt_drv_mmz_unmap_and_release(&MmzBuf);

                ChanEsBuff->u32IsUsed = DMX_ISUSED_FREE;
                ChanEsBuff->ch_id = DMX_INVALID_CHAN_ID;
                break;
            }

            case MT_UNF_DMX_CHAN_TYPE_POST:
            {
                DMX_ChanSecBuff_S *secbuf = NULL;

                mutex_lock(&DmxMgr->ts_chan_list_mutex);

                if (!list_empty_careful(&DmxMgr->ts_chan_list))
                {
                    list_del_init(&ChanInfo->chan_node);
                }

                mutex_unlock(&DmxMgr->ts_chan_list_mutex);

                secbuf = &DmxMgr->DmxChanSecBuff[ChanInfo->secBuffId];
                DMX_OsiChanDeleteSectionBuffer(ChanId, secbuf->BuffId, 0);

                break;
            }

            case MT_UNF_DMX_CHAN_TYPE_PES:
            {
                DMX_ChanSecBuff_S *secbuf = NULL;

                if (NULL != ChanInfo->softpeshead)
                {
                    mmz_buffer_s MmzBuf;

                    MmzBuf.startPhyAddr = ChanInfo->softpeshead->pesphyadr;
                    MmzBuf.startVirAddr = (void *)(ChanInfo->softpeshead->pesdata);
                    mutex_lock(&DmxMgr->pes_chan_list_mutex);
                    mutex_lock(&ChanInfo->chan_mutex);

                    if (ChanInfo->u32countRef > 0)
                    {
                        ret = MT_FAILURE;
                        mutex_unlock(&ChanInfo->chan_mutex);
                        mutex_unlock(&DmxMgr->pes_chan_list_mutex);
                        MT_ERR_DEMUX("destroy chan %d failed 0x%x\n", ChanId, ret);
                        return ret;
                    }

                    mt_drv_mmz_unmap_and_release(&MmzBuf);
                    MT_DBG_DEMUX("release PES Buff, PhyAddr = %#x, VirAddr = %#x\n", MmzBuf.startPhyAddr, MmzBuf.startVirAddr);
                    MT_VFREE(MT_ID_DEMUX, ChanInfo->softpeshead);
                    ChanInfo->softpeshead = NULL;
                    ChanInfo->u32countRef = 0;
                }

                if (!list_empty_careful(&DmxMgr->pes_chan_list))
                {
                    list_del_init(&ChanInfo->chan_node);
                }

                secbuf = &DmxMgr->DmxChanSecBuff[ChanInfo->secBuffId];
                DMX_OsiChanDeleteSectionBuffer(ChanId, secbuf->BuffId, 0);
                mutex_unlock(&ChanInfo->chan_mutex);
                mutex_unlock(&DmxMgr->pes_chan_list_mutex);
                break;
            }

            case MT_UNF_DMX_CHAN_TYPE_HW_PES:
            {
                mmz_buffer_s MmzBuf;
                DMX_ChanEsBuff_S *ChanEsBuff = &DmxMgr->DmxChanEsBuff[ChanInfo->avChanId];
    
        	    MmzBuf.startPhyAddr = ChanEsBuff->data_buff.startPhyAddr;
        	    MmzBuf.startVirAddr = ChanEsBuff->data_buff.startVirAddr;
        	    MT_DBG_DEMUX("release HW PES Buff, PhyAddr = %#x, VirAddr = %#x\n", MmzBuf.startPhyAddr, MmzBuf.startVirAddr);
        	    mt_drv_mmz_unmap_and_release(&MmzBuf);
        	    
        	    MmzBuf.startPhyAddr = ChanEsBuff->desc_buff.startPhyAddr;
        	    MmzBuf.startVirAddr = ChanEsBuff->desc_buff.startVirAddr;
        	    MT_DBG_DEMUX("release HW PES Desc Buff, PhyAddr = %#x, VirAddr = %#x\n", MmzBuf.startPhyAddr, MmzBuf.startVirAddr);
        	    mt_drv_mmz_unmap_and_release(&MmzBuf);
    
                ChanEsBuff->u32IsUsed = DMX_ISUSED_FREE;
                ChanEsBuff->ch_id = DMX_INVALID_CHAN_ID;
                mutex_lock(&DmxMgr->hw_pes_chan_list_mutex);

                if (!list_empty_careful(&DmxMgr->hw_pes_chan_list))
                {
                    list_del_init(&ChanInfo->chan_node);
                }

                mutex_unlock(&DmxMgr->hw_pes_chan_list_mutex);

                break;
            }

            case MT_UNF_DMX_CHAN_TYPE_SEC:
            case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
            {
                mutex_lock(&DmxMgr->sec_chan_list_mutex);

                if (!list_empty_careful(&DmxMgr->sec_chan_list))
                {
                    list_del_init(&ChanInfo->chan_node);
                }

                mutex_unlock(&DmxMgr->sec_chan_list_mutex);

                if (0 == down_interruptible(&DmxMgr->lock_Filter))
                {
                    RegionEnd = DMX_CHANNEL_CNT;

                    for (i = RegionBegin; i < RegionEnd; i++)
                    {
                        DMX_ChanSecBuff_S *Buff = &DmxMgr->DmxChanSecBuff[i];

                        if ((ChanId == Buff->BuffId) && (Buff->BuffId != DMX_INVALID_BUF_ID))
                        {
                            DMX_OsiChanDeleteSectionBuffer(ChanId, Buff->BuffId, 0);
                        }
                    }

                    up(&DmxMgr->lock_Filter);
                }

                if (ChanInfo->FilterCount)
                {
                    RegionEnd = DMX_FILTER_CNT;

                    if (0 == down_interruptible(&DmxMgr->lock_Filter))
                    {
                        for (i = RegionBegin; i < RegionEnd; i++)
                        {
                            DMX_FilterInfo_S *Filter = &DmxMgr->DmxFilterInfo[i];

                            if (ChanId == Filter->ChanId)
                            {
                                Filter->ChanId = DMX_INVALID_CHAN_ID;

                                for (j = 0; j < 4; j++)
                                {
                                    if (Filter->FilterBuffId[j] != DMX_INVALID_CHAN_ID)
                                    {
                                        DmxHalDetachFilter(Filter->FilterBuffId[j], ChanId);
                                        DmxFilterBuff = &DmxMgr->DmxFilterBuff[Filter->FilterBuffId[j]];
                                        DmxFilterBuff->u32IsUsed = DMX_ISUSED_FREE;
                                        Filter->FilterBuffId[j] = DMX_INVALID_CHAN_ID;
                                    }
                                }
                            }
                        }

                        up(&DmxMgr->lock_Filter);
                    }

                    ChanInfo->FilterCount = 0;
                }

                break;
            }

            default:
           	{
                break;
            }    			
        }
    }
	
    if (0 == down_interruptible(&DmxMgr->lock_Channel))
    {
        reg_set_demux_slotn_cfg0(ChanInfo->ChanId, 0);
        reg_set_demux_slotn_cfg1(ChanInfo->ChanId, 0);
        
        if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_DSS == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_HW_PES == ChanInfo->ChanType)) 
        {
            reg_set_trpp_ch_property(ChanInfo->avChanId, 0);
            reg_set_trpp_ch_parse_set(ChanInfo->avChanId, 0);
        //    DMX_OsiClearEsBuf(ChanInfo->avChanId, 0);
        //    DMX_OsiClearDscrptBuf(ChanInfo->avChanId, 0);
        }

		if (MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType) 
		{
			reg_value = reg_get_trpp_esbuf_ch();
			reg_value |= (0x0F << (ChanInfo->avChanId*4));//set to invalid value
			reg_set_trpp_esbuf_ch(reg_value);
        }
			
        ChanInfo->APPChanType = MT_UNF_DMX_CHAN_TYPE_BUTT;
        ChanInfo->DmxId = DMX_INVALID_DEMUX_ID;
        ChanInfo->ChanId = DMX_INVALID_CHAN_ID;
        ChanInfo->ChanPid = DMX_INVALID_PID;
        ChanInfo->ChanOutMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_BUTT;
        ChanInfo->u32BuffWriteP = 0;
        up(&DmxMgr->lock_Channel);
    }

    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DMX_OsiOpenChannel
* Description   : open a channel and start to work
* Input         : ChanId
* Output        :
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiCalcFQALOVFBPTso(mt_u32 ChanId)
{
    mt_s32 FqBpThd = 0;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    mt_s32 BlockSize;
    mt_u32 FQId = 1;

    //BlockSize   = (MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ? DMX_FQ_AUD_BLOCK_SIZE : DMX_FQ_VID_BLOCK_SIZE;
    FQId = pDmxDevOsi->DmxOqInfo[ChanInfo->ChanOqId].u32FQId;
    BlockSize = pDmxDevOsi->DmxFqInfo[FQId].u32BlockSize;

    /*used persent must > avplay reset checking  persent (85%)*/
    FqBpThd = (ChanInfo->ChanBufSize / BlockSize) * FQ_BP_TSO_PERSENT / 100;

    return FqBpThd;
}
 
mt_s32 DMX_OsiOpenChannel(mt_u32 ChanId)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    DMX_OQ_Info_S *OqInfo;
    DMX_Sub_DevInfo_S *DmxInfo;
    mt_s32 FqBpThd = 0;
		 DMX_TunerPort_Info_S *Tsi = NULL;

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];

    CHECKDMXID(ChanInfo->DmxId);
    OqInfo = &pDmxDevOsi->DmxOqInfo[ChanInfo->ChanOqId];
    DmxInfo = &pDmxDevOsi->SubDevInfo[ChanInfo->DmxId];

    if ((DMX_PORT_MODE_TUNER != DmxInfo->PortMode) && (DMX_PORT_MODE_RAM != DmxInfo->PortMode)) 
	{
		MT_ERR_DEMUX("not attach port\n");
		return MT_ERR_DMX_NOATTACH_PORT;
    }

    if (MT_UNF_DMX_CHAN_CLOSE != ChanInfo->ChanStatus) 
	{
		MT_WARN_DEMUX("channel %d is already opened\n", ChanId);
		return MT_SUCCESS;
    }

    //0. Reset channel counter
    //DmxHalResetChannelCounter(ChanId);

    //1. if ch is used for play
    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode)
    {
        DmxHalSetChannelPlayDmxid(ChanId, ChanInfo->DmxId);

        if ((ChanInfo->ChanType != MT_UNF_DMX_CHAN_TYPE_SEC) && (ChanInfo->ChanType != MT_UNF_DMX_CHAN_TYPE_ECM_EMM))
        {
            DmxHalSetChannelFltMode(ChanId, DMX_DISABLE);
            if (MT_UNF_DMX_CHAN_TYPE_PES == ChanInfo->ChanType)
            {
                ChanInfo->u32PesBlkCnt = 0;
            }
        } 
        else
        {
            //DmxHalSetChannelFltMode(ChanId, DMX_ENABLE);
        }

        if ((ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_AUD) ||
            (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_DSS == ChanInfo->ChanType) ||
            (ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_VID))
        {
            if ((DMX_PORT_MODE_RAM == DmxInfo->PortMode) && (DmxInfo->PortId < DMX_RAMPORT_CNT))
            {
                //open aud and vid fq bp
                DmxHalAttachIPBPFQ(DmxInfo->PortId, OqInfo->u32FQId);
            }
	    }
    }

    //2. if ch is used for rec
    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_REC & ChanInfo->ChanOutMode) 
	{
		DMX_RecInfo_S *RecInfo = &pDmxDevOsi->DmxRecInfo[ChanInfo->rec_id];

		if (MT_UNF_DMX_REC_TYPE_SELECT_PID == RecInfo->RecType) 
		{
		    DmxHalSetChannelRecBufId(ChanId, RecInfo->RecOqId);
		}
		if (MT_UNF_DMX_REC_TYPE_ALL_PID == RecInfo->RecType) 
		{
		    ChanInfo->slot_reg0.bitc.pid =  RecInfo->slot_reg0.bitc.pid;
		    ChanInfo->slot_reg0.bitc.pid_filter_en =  RecInfo->slot_reg0.bitc.pid_filter_en;
		    ChanInfo->slot_reg0.bitc.pid_filter_mode =  RecInfo->slot_reg0.bitc.pid_filter_mode;
		}
    }

    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_REC & ChanInfo->ChanOutMode) 
	{
		ChanInfo->ChanStatus |= MT_UNF_DMX_CHAN_REC_EN;
    }

    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode) 
	{
		ChanInfo->ChanStatus |= MT_UNF_DMX_CHAN_PLAY_EN;

		//DmxOqStart(ChanInfo->ChanOqId);

	    if ((ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_AUD) ||
	        (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType) ||
        	(MT_UNF_DMX_CHAN_TYPE_DSS == ChanInfo->ChanType) ||
        	(ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_VID))
	    {
		      DmxFqStart(OqInfo->u32FQId);

	        Tsi = &g_pDmxDevOsi->TunerPortInfo[DmxInfo->PortId];
			/*this tsi should back presure some ram port ,default is 0,means invalid (ram port >=128),
	                in situation as : ram128------>tso---->tsi---->demux*/
			if (Tsi->BPRamPort)
			{
				FqBpThd = DMX_OsiCalcFQALOVFBPTso(ChanId);
				DmxHalSetFQWORDx(OqInfo->u32FQId, DMX_FQ_CTRL_OFFSET, FqBpThd << 24);
				DmxHalAttachIPBPFQ(Tsi->BPRamPort - MT_UNF_DMX_PORT_RAM_0, OqInfo->u32FQId);
			}
			/*enable parse*/
	        reg_set_trpp_channel_parse_en(reg_get_trpp_channel_parse_en() | (1 << ChanInfo->avChanId));

			//trpp_ch_ini_info1-9
			reg_set_trpp_ch1_ini_info1(ChanInfo->avChanId, 0x8000000c);
			reg_set_trpp_ch1_ini_info2(ChanInfo->avChanId, 0);
			reg_set_trpp_ch1_ini_info3(ChanInfo->avChanId, 0);
			reg_set_trpp_ch1_ini_info4(ChanInfo->avChanId, 0);
			reg_set_trpp_ch1_ini_info5(ChanInfo->avChanId, 0);
			reg_set_trpp_ch1_ini_info6(ChanInfo->avChanId, 0);
			reg_set_trpp_ch1_ini_info7_trpp_ch1_chnum(ChanInfo->avChanId, ChanInfo->avChanId);
			reg_set_trpp_ch1_ini_info7_trpp_ch1_gotstrid(ChanInfo->avChanId, 1);
			//4700  1:discard current pes packe   0:do not discard current pes pes packet
			reg_set_trpp_ch1_ini_info7_trpp_ch1_discard_flag(ChanInfo->avChanId, 1);
			reg_set_trpp_ch1_ini_info8(ChanInfo->avChanId, 0);
			reg_set_trpp_ch1_ini_info9(ChanInfo->avChanId, 0xff);

	    }
		else if(MT_UNF_DMX_CHAN_TYPE_SEC == ChanInfo->ChanType)
		{
			if (ChanInfo->FilterCount) 
			{
				mt_u32 RegionBegin = 0;
				mt_u32 RegionEnd = DMX_FILTER_CNT;
				int i=0;
				int ret=0;
				//mt_u32 timeout = 0x10;
	
				ret = down_interruptible(&pDmxDevOsi->lock_Filter);
				for (i = RegionBegin; i < RegionEnd; i++) 
				{
					DMX_FilterInfo_S *Filter = &pDmxDevOsi->DmxFilterInfo[i];
					if (ChanId == Filter->ChanId) 
					{
#if ENBALE_FILTER
						if(reg_get_filtern_config_filter_en(Filter->FilterId) == 0)
						{
						 	if(Filter->FilterId != DMX_INVALID_CHAN_ID)
						 	{
								 Filter->config_reg.bitc.buf_id = ChanInfo->secBuffId;
								 Filter->config_reg.bitc.filter_en = 1;
								 Filter->AttachFlag = 1;
								 reg_set_filtern_config(Filter->FilterId, Filter->config_reg.all);
							}
						}
#endif
					}
				}
				up(&pDmxDevOsi->lock_Filter);
			}
		}
	}

    ChanInfo->u32AcqTimeInterval = 0;
    ChanInfo->u32RelTimeInterval = 0;

    MT_DRV_SYS_GetTimeStampMs(&ChanInfo->u32AcqTime);
    ChanInfo->u32RelTime = ChanInfo->u32AcqTime;

    ChanInfo->slot_reg0.bitc.slot_en = 1;
    ChanInfo->slot_reg1.all = ChanInfo->slot_reg1.all | 0x8000;

    MT_DBG_DEMUX("slot start >>>>>>>>>>>>  ChanInfo->ChanId=%d\n", ChanInfo->ChanId);
    reg_set_demux_slotn_cfg0(ChanInfo->ChanId, ChanInfo->slot_reg0.all);
    reg_set_demux_slotn_cfg1(ChanInfo->ChanId, ChanInfo->slot_reg1.all);
	//printk(KERN_INFO "chanid:%d,CFG0:0x%x,CFG1:0x%x\n",ChanInfo->ChanId,ChanInfo->slot_reg0.all,ChanInfo->slot_reg0.all);
    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DMX_OsiCloseChannel
* Description   : close a channel and stop working
* Input         : ChanId
* Output        :
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiCloseChannel(mt_u32 ChanId)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    DMX_OQ_Info_S *OqInfo;
    DMX_FLUSH_TYPE_E FlushType;
    mt_s32 time_out=1000;

    CHECKDMXID(ChanInfo->DmxId);
    OqInfo = &pDmxDevOsi->DmxOqInfo[ChanInfo->ChanOqId];

    ChanInfo->ChanStatus = MT_UNF_DMX_CHAN_CLOSE;
    ChanInfo->u32TotolAcq = 0;
    ChanInfo->u32HitAcq = 0;
    ChanInfo->u32Release = 0;
    ChanInfo->u32PesLength = 0;
    ChanInfo->ChanEosFlag = MT_FALSE;
    ChanInfo->enDiscardErrorPUSIPacket = 0;

    // close channel
    //DmxHalSetChannelPlayDmxid(ChanId, DMX_INVALID_DEMUX_ID);

    reg_set_demux_slotn_cfg0_slot_en(ChanInfo->ChanId, 0);
    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode))
    {
        if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_DSS == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType))
        {
            reg_set_demux_slotn_cfg0_slot_en(ChanInfo->ChanId, 0);
        }
    }

    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == ChanInfo->ChanOutMode)
    {
        FlushType = DMX_FLUSH_TYPE_REC_PLAY;
    }
    else if (MT_UNF_DMX_CHAN_OUTPUT_MODE_REC == ChanInfo->ChanOutMode)
    {
        FlushType = DMX_FLUSH_TYPE_REC;
    }
    else
    {
        FlushType = DMX_FLUSH_TYPE_REC_PLAY;
    }

    //DMX_OsiResetChannel(ChanId, FlushType);

    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode))
	{
		reg_set_demux_slotn_cfg0_slot_en(ChanInfo->ChanId, 0);
		if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
			(MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_DSS == ChanInfo->ChanType) ||
            (MT_UNF_DMX_CHAN_TYPE_VID == ChanInfo->ChanType))
		{
			DMX_ChanEsBuff_S * EsBuff = &pDmxDevOsi->DmxChanEsBuff[ChanInfo->avChanId]; // Toby
			EsBuff->p_cur_desc = NULL;
			EsBuff->p_next_desc = NULL;
			reg_set_demux_slotn_cfg0(ChanInfo->ChanId, 0);
			reg_set_demux_slotn_cfg1(ChanInfo->ChanId, 0);
			/*disable parse*/
			reg_set_trpp_channel_parse_en(reg_get_trpp_channel_parse_en() & (~(1 << ChanInfo->avChanId)));
			while(reg_get_trpp_ch_clear_status() == 0 && (time_out--))
			{
				udelay(1);
			}
			reg_set_trpp_ch_clear_status(0);

			//20200604: move to here, fix WR ptr != 0 issue.
			DMX_OsiClearEsBuf(ChanInfo->avChanId, 0);
			DMX_OsiClearDscrptBuf(ChanInfo->avChanId, 0);
		}
		else if(MT_UNF_DMX_CHAN_TYPE_SEC == ChanInfo->ChanType)
		{
			// close all filters attatched to this channel
			if (ChanInfo->FilterCount) 
			{
				mt_u32 RegionBegin = 0;
				mt_u32 RegionEnd = DMX_FILTER_CNT;
				int i=0;
				int ret=0;
				mt_u32 timeout = 0x10;

				ret = down_interruptible(&pDmxDevOsi->lock_Filter);
				for (i = RegionBegin; i < RegionEnd; i++) 
				{
					DMX_FilterInfo_S *Filter = &pDmxDevOsi->DmxFilterInfo[i];
					if (ChanId == Filter->ChanId) 
					{
						DMX_ChanSecBuff_S *SecBuff = &pDmxDevOsi->DmxChanSecBuff[ChanInfo->secBuffId];

						//mt_u32 discWP = reg_get_bufn_disc_wptr_disc_ch_wptr(ChanInfo->secBuffId);
						//mt_u32 dataWP = reg_get_bufn_data_wptr_data_ch_wptr(ChanInfo->secBuffId) ;
						//mt_u32 dataRP = reg_get_bufn_size_data_ch_rptr(ChanInfo->secBuffId) * 1024;

#if ENBALE_FILTER
						if(Filter->FilterId != DMX_INVALID_CHAN_ID)
						{
							reg_set_filtern_config_filter_en(Filter->FilterId, 0);
							while((!(dmx_inl(reg_dmx_sf_process_sta) & 0x1)) && timeout--)
							{
								msleep(10);
							}
							reg_set_filtern_config_filt_sta(Filter->FilterId, 0);
						}
#endif
						// buff size = SecBuff->sec_data_buff.size * 1024 + DMX_SYMPHONY_SEC_DESC_BUF_SIZE + 4096;
						memset((mt_u8 *)SecBuff->sec_data_buff.startVirAddr, 0, (SecBuff->sec_data_buff.size * 1024 + DMX_SYMPHONY_SEC_DESC_BUF_SIZE + 4096));

						reg_set_bufn_disc_wptr_disc_ch_wptr(ChanInfo->secBuffId, 0);
						reg_set_bufn_data_wptr_data_ch_wptr(ChanInfo->secBuffId, 0);
						reg_set_bufn_size_data_ch_rptr(ChanInfo->secBuffId, 0);
						reg_set_bufn_size_disc_ch_rptr(ChanInfo->secBuffId, 0);
						SecBuff->desc_buff_write_p = 0;
						SecBuff->received_sec = 0;
					}
				}
				up(&pDmxDevOsi->lock_Filter);
				//ChanInfo->FilterCount = 0;
			}
            else
            {
                DMX_ChanSecBuff_S *SecBuff = &pDmxDevOsi->DmxChanSecBuff[ChanInfo->secBuffId];

                memset((mt_u8 *)SecBuff->sec_data_buff.startVirAddr, 0, (SecBuff->sec_data_buff.size * 1024 + DMX_SYMPHONY_SEC_DESC_BUF_SIZE + 4096));

				reg_set_bufn_disc_wptr_disc_ch_wptr(ChanInfo->secBuffId, 0);
				reg_set_bufn_data_wptr_data_ch_wptr(ChanInfo->secBuffId, 0);
				reg_set_bufn_size_data_ch_rptr(ChanInfo->secBuffId, 0);
				reg_set_bufn_size_disc_ch_rptr(ChanInfo->secBuffId, 0);
				SecBuff->desc_buff_write_p = 0;
				SecBuff->received_sec = 0;
            }
		}
	}
    return MT_SUCCESS;
}

/***********************************************************************************
* Function      : DMX_OsiGetChannelAttr
* Description   : Get Channel Attr
* Input         : ChanId, ChanAttr
* Output        :
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiGetChannelAttr(mt_u32 ChanId, MT_UNF_DMX_CHAN_ATTR_S *ChanAttr)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChanId];

    if (0 == down_interruptible(&DmxMgr->lock_Channel))
    {
        if (ChanInfo->DmxId < DMX_CNT)
        {
            ChanAttr->u32BufSize = ChanInfo->ChanBufSize;
            ChanAttr->enChannelType = ChanInfo->ChanType;
            ChanAttr->enCRCMode = ChanInfo->ChanCrcMode;
            ChanAttr->enOutputMode = ChanInfo->ChanOutMode;
            ChanAttr->AVSyncFlag = MT_TRUE;
            ChanAttr->vCodecType = ChanInfo->vCodecType;
            ChanAttr->enDiscardErrorPUSIPacket = ChanInfo->enDiscardErrorPUSIPacket;
            ChanAttr->bEsToUser = ChanInfo->aes_to_user;

            if (ChanAttr->esBuffId1 != NULL)
            {
                *ChanAttr->esBuffId1 = ChanInfo->avChanId;
            }
            ret = MT_SUCCESS;
        }

        up(&DmxMgr->lock_Channel);
    } 
    else
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        ret = MT_ERR_DMX_BUSY;
    }

    return ret;
}

mt_u8*  DMX_OsiGetChannel_Descbuf(mt_u32 ChanId)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
	DMX_ChanEsBuff_S * esBuff = &DmxMgr->DmxChanEsBuff[ChanId];

	return  esBuff->desc_buff.startVirAddr;
}

/***********************************************************************************
* Function      : DMX_OsiSetChannelAttr
* Description   : Set Channel Attr
* Input         : ChanId, ChanAttr
* Output        :
* Return        : MT_SUCCESS:     success
*                 MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiSetChannelAttr(mt_u32 ChanId, MT_UNF_DMX_CHAN_ATTR_S *ChanAttr)
{
    mt_s32 ret = MT_SUCCESS;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChanId];

    CHECKDMXID(ChanInfo->DmxId);

    if ((ChanAttr->enChannelType != ChanInfo->ChanType) 
		|| (ChanAttr->enOutputMode != ChanInfo->ChanOutMode)) 
	{
		MT_ERR_DEMUX("do not suppor change attr of buffer size, channel type and channel output mode!");
		return MT_ERR_DMX_NOT_SUPPORT;
    }

    switch (ChanAttr->enChannelType) {
    case MT_UNF_DMX_CHAN_TYPE_SEC:
    case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
	switch (ChanAttr->enCRCMode) {
       	case MT_UNF_DMX_CHAN_CRC_MODE_FORBID:
       	case MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD:
       	case MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_SEND:
       	case MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD:
       	case MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_SEND: {
	       ChanInfo->ChanCrcMode = ChanAttr->enCRCMode;

    	       switch (ChanAttr->enCRCMode) {
                	case MT_UNF_DMX_CHAN_CRC_MODE_FORBID:
                	    ChanInfo->slot_reg1.bitc.sec_discard_mode = 0;
                	    ChanInfo->slot_reg1.bitc.sec_mode = 0;
                	    reg_set_demux_slotn_cfg1_sec_discard_mode(ChanId, 0);
                	    reg_set_demux_slotn_cfg1_sec_mode(ChanId, 0);
                	    break;
                	case MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD:
                	    ChanInfo->slot_reg1.bitc.sec_discard_mode = 0;
                	    ChanInfo->slot_reg1.bitc.sec_mode = 1;
                	    reg_set_demux_slotn_cfg1_sec_discard_mode(ChanId, 0);
                	    reg_set_demux_slotn_cfg1_sec_mode(ChanId, 1);
                	    break;
                	case MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_SEND:
                	    ChanInfo->slot_reg1.bitc.sec_discard_mode = 1;
                	    ChanInfo->slot_reg1.bitc.sec_mode = 1;
                	    reg_set_demux_slotn_cfg1_sec_discard_mode(ChanId, 1);
                	    reg_set_demux_slotn_cfg1_sec_mode(ChanId, 1);
                	    break;
                	case MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD:
                	    ChanInfo->slot_reg1.bitc.sec_discard_mode = 0;
                	    ChanInfo->slot_reg1.bitc.sec_mode = 2;
                	    reg_set_demux_slotn_cfg1_sec_discard_mode(ChanId, 0);
                	    reg_set_demux_slotn_cfg1_sec_mode(ChanId, 2);
                	    break;
                	case MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_SEND:
                	    ChanInfo->slot_reg1.bitc.sec_discard_mode = 1;
                	    ChanInfo->slot_reg1.bitc.sec_mode = 2;
                	    reg_set_demux_slotn_cfg1_sec_discard_mode(ChanId, 1);
                	    reg_set_demux_slotn_cfg1_sec_mode(ChanId, 2);
                	    break;

                	default:
                	    MT_ERR_DEMUX("invalid crc mode %u\n", ChanAttr->enCRCMode);
                	    return MT_ERR_DMX_INVALID_PARA;
        	 }

	        break;
	    }

	default:
	    MT_ERR_DEMUX("invalid crc mode %u\n", ChanAttr->enCRCMode);

	    ret = MT_ERR_DMX_INVALID_PARA;
	}

	break;

    case MT_UNF_DMX_CHAN_TYPE_AUD:
    case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
    //	reg_set_trpp_esbuf_ch_trpp_esbufwp0_chsel(0);

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	    reg_set_trpp_ch_property_trpp_ch_time_info(ChanInfo->avChanId, 0x0); //do not insert pts after start_code
    	reg_set_trpp_ch_parse_set_trpp_ch_insrt_nbytes(ChanInfo->avChanId, 0x0);
#else
	    reg_set_trpp_ch_property_trpp_ch_time_info(ChanInfo->avChanId, 0x1); //insert pts after start_code
	    reg_set_trpp_ch_parse_set_trpp_ch_insrt_nbytes(ChanInfo->avChanId, 0x6);
#endif
	        
        if(ChanAttr->enDiscardErrorPUSIPacket != ChanInfo->enDiscardErrorPUSIPacket) 
		{
            ChanInfo->enDiscardErrorPUSIPacket = ChanAttr->enDiscardErrorPUSIPacket;
            if(ChanAttr->enDiscardErrorPUSIPacket) 
			{
                MT_DBG_DEMUX("%s:%d:enable watermark filter ch %u\n", __func__, __LINE__, ChanInfo->avChanId);
                reg_set_trpp_ch_property_trpp_ch_pusi_detect(ChanInfo->avChanId, 0x1);
                reg_set_trpp_ch_property_trpp_ch_pusi_mode2(ChanInfo->avChanId, 0x1);
            } 
			else 
			{
                MT_DBG_DEMUX("%s:%d:disable watermark filter ch %u\n", __func__, __LINE__, ChanInfo->avChanId);
                reg_set_trpp_ch_property_trpp_ch_pusi_detect(ChanInfo->avChanId, 0);
                reg_set_trpp_ch_property_trpp_ch_pusi_mode2(ChanInfo->avChanId, 0);
            }
        }

    	if (ChanAttr->enCRCMode != ChanInfo->ChanCrcMode) {
    	    MT_ERR_DEMUX("PES/VID/AUD/POST channels do not support change their crc mode: %u\n", ChanAttr->enCRCMode);
    	    ret = MT_ERR_DMX_NOT_SUPPORT;
    	}

        ChanInfo->aes_to_user = ChanAttr->bEsToUser;
    	break;

    case MT_UNF_DMX_CHAN_TYPE_VID:
        ChanInfo->vCodecType = ChanAttr->vCodecType;

		/*if((ChanInfo->avChanId == 0) && (DmxMgr->DmxChanInfo[1].ChanId == DMX_INVALID_CHAN_ID))
		{
	    	reg_set_trpp_esbuf_ch(0);
		}*/
		
	    reg_set_trpp_ch_property_trpp_ch_time_info(ChanInfo->avChanId, 0x2); //insert pts after start_code
	    MT_DBG_DEMUX("ChanAttr->vCodecType = %d\n", ChanAttr->vCodecType);
	    DMX_OsiChanSetStartCode(ChanInfo->avChanId, ChanAttr->vCodecType);
	    if (MT_UNF_VCODEC_TYPE_H264 == ChanAttr->vCodecType || MT_UNF_VCODEC_TYPE_HEVC == ChanAttr->vCodecType) 
		{
			reg_set_trpp_ch_parse_set_trpp_ch_fsc_cp_en(ChanInfo->avChanId, 0x1);
	    } 
		else 
		{
			reg_set_trpp_ch_parse_set_trpp_ch_fsc_cp_en(ChanInfo->avChanId, 0x0);
	    }
	        
        if(ChanAttr->enDiscardErrorPUSIPacket != ChanInfo->enDiscardErrorPUSIPacket) 
		{
            ChanInfo->enDiscardErrorPUSIPacket = ChanAttr->enDiscardErrorPUSIPacket;
            if(ChanAttr->enDiscardErrorPUSIPacket) 
			{
                MT_DBG_DEMUX("%s:%d:enable watermark filter ch %u\n", __func__, __LINE__, ChanInfo->avChanId);
                reg_set_trpp_ch_property_trpp_ch_pusi_detect(ChanInfo->avChanId, 0x1);
                reg_set_trpp_ch_property_trpp_ch_pusi_mode2(ChanInfo->avChanId, 0x1);
            } 
			else 
			{
                MT_DBG_DEMUX("%s:%d:disable watermark filter ch %u\n", __func__, __LINE__, ChanInfo->avChanId);
                reg_set_trpp_ch_property_trpp_ch_pusi_detect(ChanInfo->avChanId, 0);
                reg_set_trpp_ch_property_trpp_ch_pusi_mode2(ChanInfo->avChanId, 0);
            }
        }

    	if (ChanAttr->enCRCMode != ChanInfo->ChanCrcMode)
    	{
    	    MT_ERR_DEMUX("PES/VID/AUD/POST channels do not support change their crc mode: %u\n", ChanAttr->enCRCMode);
    	    ret = MT_ERR_DMX_NOT_SUPPORT;
    	}
    	break;

    case MT_UNF_DMX_CHAN_TYPE_PES:
    case MT_UNF_DMX_CHAN_TYPE_HW_PES:
    case MT_UNF_DMX_CHAN_TYPE_POST:
	case MT_UNF_DMX_CHAN_TYPE_DSS:
	if (ChanAttr->enCRCMode != ChanInfo->ChanCrcMode) {
	    MT_ERR_DEMUX("PES/VID/AUD/POST channels do not support change their crc mode: %u\n", ChanAttr->enCRCMode);
	    ret = MT_ERR_DMX_NOT_SUPPORT;
	}
	break;

    default:
	MT_ERR_DEMUX("invalid chan type %u\n", ChanAttr->enChannelType);

	ret = MT_ERR_DMX_INVALID_PARA;
    }

    return ret;
}

mt_s32 DMX_OsiGetChannelPid(mt_u32 ChanId, mt_u32 *Pid)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChanId];

    if (ChanInfo->DmxId < DMX_CNT) {
	*Pid = ChanInfo->ChanPid;

	ret = MT_SUCCESS;
    }

    return ret;
}

mt_s32 DMX_OsiSetChannelPid(mt_u32 ChanId, mt_u32 Pid)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChanId];
    DMX_Sub_DevInfo_S *DmxInfo = &g_pDmxDevOsi->SubDevInfo[ChanInfo->DmxId];
	DMX_RecInfo_S *rec_info = NULL;
	DMX_KeyInfo_S *KeyInfo = &DmxMgr->DmxKeyInfo[0];
    mt_u32 i;

	//If the pid be greater than DMX_INVALID_PID, return direct.
	//If the pid be equal DMX_INVALID_PID, 
	//only the post and record channel are allowed to set,
	//other channels will return.
	if ((Pid > DMX_INVALID_PID)
		|| ((DMX_INVALID_PID == Pid) 
	        && (ChanInfo->ChanType != MT_UNF_DMX_CHAN_TYPE_POST)
		    && (ChanInfo->ChanType != MT_UNF_DMX_CHAN_TYPE_REC)))
	{
		MT_ERR_DEMUX("[%s %d]pid(0x%x) is invalid for channel type(%d)\n", 
					__FUNCTION__, __LINE__, Pid,
					ChanInfo->ChanType);
		return MT_ERR_DMX_INVALID_PARA;
	}
			
	if (ChanInfo->DmxId >= DMX_CNT)
    {
		MT_ERR_DEMUX("invalid param, dmxid:%d,Pid:0x%x\n", ChanInfo->DmxId, Pid);
		return MT_ERR_DMX_INVALID_PARA;
    }
	
    if (MT_UNF_DMX_CHAN_CLOSE != ChanInfo->ChanStatus) 
	{
		MT_ERR_DEMUX("close channel first when set channel pid!\n");
		return MT_ERR_DMX_OPENING_CHAN;
    }

	rec_info = &g_pDmxDevOsi->DmxRecInfo[ChanInfo->rec_id];
	
    if (Pid < DMX_INVALID_PID) 
	{
		mt_u32 i;

		//if the channel is not rec channel, 
		//or if the channel is rec channel, but don't support a pid record to multi channel
		//we check if the pid has been occupied by other channel in the same dmx
		if ((ChanInfo->ChanType != MT_UNF_DMX_CHAN_TYPE_REC) || (0 == rec_info->multi_chan_flag))
		{
			for (i = 0; i < DMX_CHANNEL_CNT; i++) 
			{
			    if (DmxMgr->DmxChanInfo[i].DmxId == ChanInfo->DmxId) 
				{
					// 
					if ((DmxMgr->DmxChanInfo[i].ChanPid == Pid) && (DmxMgr->DmxChanInfo[i].ChanType == ChanInfo->ChanType)) 
					{
					    if(DmxInfo->PortMode == DMX_PORT_MODE_RAM)
					    {
			            	DmxMgr->DmxChanInfo[i].slot_reg0.bitc.src = 4 + DmxInfo->PortId;
					    }
					    else
					    {
			                DmxMgr->DmxChanInfo[i].slot_reg0.bitc.src = DmxInfo->PortId;
					    }
						
					    //DmxHalSetChannelPid(DmxMgr->DmxChanInfo[i].ChanId, DMX_INVALID_PID);
					    DmxMgr->DmxChanInfo[i].ChanPid = DMX_INVALID_PID;
			            DmxMgr->DmxChanInfo[i].slot_reg0.bitc.pid = DMX_INVALID_PID;
			            DmxMgr->DmxChanInfo[i].slot_reg0.bitc.slot_en = 0;
			            reg_set_demux_slotn_cfg0(DmxMgr->DmxChanInfo[i].ChanId, DmxMgr->DmxChanInfo[i].slot_reg0.all);
					    MT_DBG_DEMUX("Attention: occupied channel pid on same DMX,already changed the pid of channel %u, pid: 0x%x\n",
					                 DmxMgr->DmxChanInfo[i].ChanId, Pid);
					    break;
					}
			    }
			}
		}
    }

    ChanInfo->ChanPid = Pid;
    ChanInfo->slot_reg0.bitc.pid = Pid;
    if((Pid == 0x1fff) && (ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_POST))
    {
		ChanInfo->slot_reg0.bitc.pid_filter_en = 0;
    }
	
    if(DmxInfo->PortMode == DMX_PORT_MODE_RAM)
    {
        ChanInfo->slot_reg0.bitc.src = 4 + DmxInfo->PortId;
    }
    else
    {
        ChanInfo->slot_reg0.bitc.src = DmxInfo->PortId;
    }

    /* do dsc attach if need, PMT PID should no attach descrambler */
    if(Pid != 0) {
        for (i = 0; i < DMX_KEY_CNT; i++) {
            mt_u32 j = 0;
            
            if(KeyInfo->DmxId != DMX_INVALID_DEMUX_ID && KeyInfo->DmxId == ChanInfo->DmxId) {
                while(j < DMX_AV_CHANNEL_CNT) {
                    MT_DBG_DEMUX("keyinfo pid[%d] %u with demux %u\n", j, KeyInfo->Pid[j], KeyInfo->DmxId);

                    if(KeyInfo->Pid[j] == Pid) {
                        MT_INFO_DEMUX("attach pid %u to key %u with channel %d\n", Pid, i, ChanId);
                        DMX_OsiDescramblerAttach(i, ChanId);
                        return MT_SUCCESS;
                    }
                    j++;
                };
            }
            KeyInfo++;
        }
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiGetChannelStatus(mt_u32 ChanId, MT_UNF_DMX_CHAN_STATUS_E *ChanStatus)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChanId];

    if (0 == down_interruptible(&DmxMgr->lock_Channel)) {
	if (ChanInfo->DmxId < DMX_CNT) {
	    *ChanStatus = ChanInfo->ChanStatus;

	    ret = MT_SUCCESS;
	}
	up(&DmxMgr->lock_Channel);
    } else {
	MT_ERR_DEMUX("down_interruptible failed!\n");
	ret = MT_ERR_DMX_BUSY;
    }

    return ret;
}

mt_s32 DMX_OsiGetFreeChannelNum(mt_u32 DmxId, mt_u32 *FreeCount)
{
    DMX_ChanInfo_S *ChanInfo = g_pDmxDevOsi->DmxChanInfo;
    mt_u32 RegionBegin = 0;
    mt_u32 RegionEnd = DMX_CHANNEL_CNT;
    mt_u32 i;

    *FreeCount = 0;

    for (i = RegionBegin; i < RegionEnd; i++) {
	if (ChanInfo[i].DmxId >= DMX_CNT) {
	    ++(*FreeCount);
	}
    }

    return MT_SUCCESS;
}

/***********************************************************************************
* Function      :   DMX_OsiGetChannelScrambleFlag
* Description   :  get channel scrambe flag
* Input         :  u32ChannelId ,
* Output        :  penScrambleFlag
* Return        :  MT_SUCCESS:     success
*                  MT_FAILURE:
* Others:
***********************************************************************************/
mt_s32 DMX_OsiGetChannelScrambleFlag(mt_u32 u32ChannelId, MT_UNF_DMX_SCRAMBLED_FLAG_E *penScrambleFlag)
{
    MT_BOOL bTSScramble, bPesScramble;
    DMX_ChanInfo_S *pChInofo;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;

    IsDmxDevInit();

    pChInofo = &pDmxDevOsi->DmxChanInfo[u32ChannelId];

    CHECKDMXID(pChInofo->DmxId);

    if (MT_UNF_DMX_CHAN_CLOSE == pChInofo->ChanStatus) {
	MT_ERR_DEMUX("channel %d is not opened!\n", u32ChannelId);
	return MT_ERR_DMX_NOT_OPEN_CHAN;
    }

    DmxHalGetChannelTSScrambleFlag(u32ChannelId, &bTSScramble);
    DmxHalGetChannelPesScrambleFlag(u32ChannelId, &bPesScramble);
    if (MT_TRUE == bTSScramble) {
	*penScrambleFlag = MT_UNF_DMX_SCRAMBLED_FLAG_TS;
    } else if (MT_TRUE == bPesScramble) {
	*penScrambleFlag = MT_UNF_DMX_SCRAMBLED_FLAG_PES;
    } else {
	*penScrambleFlag = MT_UNF_DMX_SCRAMBLED_FLAG_NO;
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiSetChannelEosFlag(mt_u32 ChanId)
{
    DMX_ChanInfo_S *ChanInfo = &g_pDmxDevOsi->DmxChanInfo[ChanId];

    CHECKDMXID(ChanInfo->DmxId);

    if (MT_UNF_DMX_CHAN_CLOSE == ChanInfo->ChanStatus) {
	MT_ERR_DEMUX("channel %d is not opened!\n", ChanId);
	return MT_ERR_DMX_NOT_OPEN_CHAN;
    }

    ChanInfo->ChanEosFlag = MT_TRUE;

    return MT_SUCCESS;
}

mt_s32 DMX_OsiGetChannelTsCnt(mt_u32 ChanId, mt_u32 *pTsCnt)
{
    DMX_ChanInfo_S *pChanInfo;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;

    IsDmxDevInit();

    pChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    if (pChanInfo->DmxId >= DMX_CNT) {
	MT_ERR_DEMUX("channel %d is no existed\n", ChanId);

	return MT_ERR_DMX_INVALID_PARA;
    }

    *pTsCnt = DmxHalGetChannelCounter(ChanId);

    return MT_SUCCESS;
}

mt_s32 DMX_OsiSetChannelCCRepeat(mt_u32 ChanId, MT_UNF_DMX_CHAN_CC_REPEAT_SET_S *pstChCCReaptSet)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *pChanInfo;

    IsDmxDevInit();

    CHECKPOINTER(pstChCCReaptSet);

    pChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    CHECKDMXID(pChanInfo->DmxId);

    if (pstChCCReaptSet->enCCRepeatMode == MT_UNF_DMX_CHAN_CC_REPEAT_MODE_DROP) {
	DmxHalSetChannelCCRepeatCtrl(ChanId, DMX_DISABLE);
    } else {
	DmxHalSetChannelCCRepeatCtrl(ChanId, DMX_ENABLE);
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiGetChannelId(mt_u32 DmxId, mt_u32 Pid, MT_UNF_DMX_CHAN_TYPE_E  ChanType, mt_u32 *ChanId)
{
    mt_s32 ret = MT_ERR_DMX_UNMATCH_CHAN;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = DmxDevOsi->DmxChanInfo;
    mt_u32 i;

    if (Pid >= DMX_INVALID_PID)
    {
        MT_ERR_DEMUX("invalid pid:%d!\n", Pid);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (0 == down_interruptible(&DmxDevOsi->lock_Channel)) 
    {
        for (i = 0; i < DMX_CHANNEL_CNT; i++) 
        {
            if(ChanType == MT_UNF_DMX_CHAN_TYPE_BUTT)
            {
                if ((ChanInfo[i].DmxId == DmxId) && (ChanInfo[i].ChanPid == Pid) &&
                    ((ChanInfo[i].ChanType == MT_UNF_DMX_CHAN_TYPE_VID) ||
                    (ChanInfo[i].ChanType == MT_UNF_DMX_CHAN_TYPE_AUD) ||
                    (MT_UNF_DMX_CHAN_TYPE_DSS == ChanInfo->ChanType) ||
                    (ChanInfo[i].ChanType == MT_UNF_DMX_CHAN_TYPE_AUD_AD)))
                {
                    *ChanId = ChanInfo[i].ChanId;

                    ret = MT_SUCCESS;
                    break;
                }
            }
            else
            {
                if ((ChanInfo[i].DmxId == DmxId) && (ChanInfo[i].ChanPid == Pid) && (ChanInfo[i].ChanType == ChanType))
                {
                    *ChanId = ChanInfo[i].ChanId;
                    ret = MT_SUCCESS;
                    break;
                }
            }
        }
        up(&DmxDevOsi->lock_Channel);
    }
    else
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        ret = MT_ERR_DMX_BUSY;
    }

    return ret;
}


mt_s32 DMX_OsiGetAVChannelId(mt_u32 DmxId, mt_u32 *vChanId, mt_u32 *aChanId)
{
    mt_s32 ret = MT_ERR_DMX_UNMATCH_CHAN;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = DmxDevOsi->DmxChanInfo;
    mt_u32 i;

    *aChanId = DMX_INVALID_CHAN_ID;
    *vChanId = DMX_INVALID_CHAN_ID;

    if (0 == down_interruptible(&DmxDevOsi->lock_Channel))
    {
        for (i = 0; i < DMX_CHANNEL_CNT; i++)
        {
            if ((ChanInfo[i].DmxId == DmxId) && (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == ChanInfo[i].ChanOutMode))
            {
                if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo[i].ChanType) || (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo[i].ChanType))
                {
                    *aChanId = ChanInfo[i].ChanId;
                }

                if (ChanInfo[i].ChanType == MT_UNF_DMX_CHAN_TYPE_VID)
                {
                    *vChanId = ChanInfo[i].ChanId;
                }
            }

            if ((*vChanId != DMX_INVALID_CHAN_ID) && (*aChanId != DMX_INVALID_CHAN_ID))
            {
                ret = MT_SUCCESS;
                break;
            }
        }

        up(&DmxDevOsi->lock_Channel);
    }
    else
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        ret = MT_ERR_DMX_BUSY;
    }

    return ret;
}




mt_s32 DMX_OsiReadDataRequest(mt_u32 u32ChId,
                              mt_u32 u32AcqNum,
                              mt_u32 *pu32AcqedNum,
                              DMX_UserMsg_S *psMsgList,
                              mt_u32 u32TimeOutMs)
{
    DMX_ChanInfo_S *pChanInfo;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;

    mt_s32 s32Ret = MT_SUCCESS;

    IsDmxDevInit();

    if (0 == u32AcqNum)
    {
        MT_ERR_DEMUX("channel %d acquire data num = 0!\n", u32ChId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    pChanInfo = &pDmxDevOsi->DmxChanInfo[u32ChId];
	pChanInfo->u32TotolAcq++;


    if (MT_UNF_DMX_CHAN_PLAY_EN != (MT_UNF_DMX_CHAN_PLAY_EN & pChanInfo->ChanStatus)) 
    {
        MT_WARN_DEMUX("channel %d is not open yet or not play channel\n", u32ChId);
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }

    switch (pChanInfo->ChanType)
    {
        case MT_UNF_DMX_CHAN_TYPE_SEC:
        case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
        case MT_UNF_DMX_CHAN_TYPE_POST:
        case MT_UNF_DMX_CHAN_TYPE_HW_PES:
        case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            s32Ret = DMXOsiReadMsg(pChanInfo, u32AcqNum, pu32AcqedNum, psMsgList, u32TimeOutMs);
            break;
        }

        default:
        {
            MT_ERR_DEMUX("Read type:%d not allowed, chanid:%d\n", pChanInfo->ChanType, u32ChId);
            s32Ret = MT_ERR_DMX_INVALID_PARA;
            break;
        }
    }

	return s32Ret;
}


mt_u32 DMX_OsiGetChType(mt_u32 u32ChId)
{
    DMX_ChanInfo_S *pChanInfo;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;

    IsDmxDevInit();
    pChanInfo = &pDmxDevOsi->DmxChanInfo[u32ChId];
    return pChanInfo->ChanType;
}


mt_s32 DMX_OsiReleaseReadPes(DMX_ChanInfo_S *pChanInfo)
{
    DMX_SOFT_PESHEADER_S *peshead = NULL;
    DMX_SOFT_PES_NODE_S *p_pes_node = NULL;
    DMX_SOFT_PES_NODE_S *p_next_node = NULL;
    struct list_head *p_pes_node_list_head = NULL;
    mt_s32 s32Ret = MT_SUCCESS;

    peshead = pChanInfo->softpeshead;

    if (!peshead)
    {
        MT_ERR_DEMUX("peshead is null\n");
        return MT_ERR_DMX_NULL_PTR;
    }

    mutex_lock(&pChanInfo->chan_mutex);

    if (!list_empty_careful(&peshead->pes_read_node_list))
    {
        p_pes_node_list_head = &peshead->pes_read_node_list;

		    list_for_each_entry_safe(p_pes_node, p_next_node, p_pes_node_list_head, pes_node)
        {
            peshead->pes_buf_rd = (p_pes_node->pes_node_addr - (ulong)peshead->pesdata + p_pes_node->pes_node_len);
			if ((peshead->pesphysiz == peshead->pes_buf_wr) && (peshead->pes_buf_rd == peshead->pes_buf_wr))
			{				
				peshead->pes_buf_wr = 0;
				peshead->pes_buf_rd = 0;
				MT_INFO_DEMUX(KERN_ERR "[%s %d]the memory is empty\r\n", __FUNCTION__, __LINE__);
			}
			else
			{
            	peshead->pes_buf_rd %= peshead->pesphysiz;
				MT_INFO_DEMUX("[%s %d]peshead->pes_buf_rd=%d\r\n", __FUNCTION__, __LINE__, peshead->pes_buf_rd);
			}
            /*
            del the node from the pes_read_node_list
            */
			      list_del_init(&p_pes_node->pes_node);
            /*
            then add it to the pes_free_node_list
            */
            list_add_tail(&p_pes_node->pes_node, &peshead->pes_free_node_list);
        }

        if (pChanInfo->u32countRef > 0)
        {
            pChanInfo->u32countRef--;
        }

        mutex_unlock(&pChanInfo->chan_mutex);
    }
    else
    {
        MT_ERR_DEMUX("pes_read_node_list empty Err\n");
        mutex_unlock(&pChanInfo->chan_mutex);
        s32Ret = MT_FAILURE;
    }

    return s32Ret;
}

mt_s32 DMX_OsiReleaseReadHwPes(DMX_ChanInfo_S *pChanInfo, mt_u32 u32RelNum, DMX_UserMsg_S *psMsgList)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanEsBuff_S *esBuff = NULL;
    mt_u8 pes_chnum = 0;
    DMX_UserMsg_S *phwpes_msg = NULL;
    mt_u32 hwpes_rls_len = 0;
    mt_u32 data_rp = 0;
    mt_u32 i = 0;

    pes_chnum = pChanInfo->avChanId;
    esBuff = &pDmxDevOsi->DmxChanEsBuff[pes_chnum];
    reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(pes_chnum, esBuff->desc_buff_write_p);
    phwpes_msg = psMsgList;

    for (i = 0; i < u32RelNum; i++)
    {
        hwpes_rls_len += phwpes_msg[i].u32MsgLen;
    }

    data_rp = reg_get_trpp_ch_data_rd_addr_trpp_ch_data_raddr(pes_chnum);
    data_rp += hwpes_rls_len;
    data_rp %= esBuff->data_buff.size;
    reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(pes_chnum, data_rp);
    return MT_SUCCESS;
}

mt_s32 DMX_OsiReleaseReadData(mt_u32 u32ChId, mt_u32 u32RelNum, DMX_UserMsg_S *psMsgList)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *pChanInfo = NULL;
    DMX_ChanSecBuff_S *SecBuff = NULL;
    mt_u8 buffId = 0;
    mt_s32 s32Ret = MT_SUCCESS;

    pChanInfo = &pDmxDevOsi->DmxChanInfo[u32ChId];

    if (MT_UNF_DMX_CHAN_PLAY_EN != (MT_UNF_DMX_CHAN_PLAY_EN & pChanInfo->ChanStatus))
    {
         MT_ERR_DEMUX("channel %d is not opechannel\n", u32ChId);
         return MT_FAILURE;
    }

    pChanInfo->u32Release++;

    switch (pChanInfo->ChanType)
    {
        case MT_UNF_DMX_CHAN_TYPE_SEC:
        case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
        {
            SecBuff = &pDmxDevOsi->DmxChanSecBuff[pChanInfo->secBuffId];
            buffId = SecBuff->BuffId;
            reg_set_bufn_size_data_ch_rptr(buffId, (SecBuff->data_buff_write_p >> 10));
            reg_set_bufn_size_disc_ch_rptr(buffId, (SecBuff->desc_buff_write_p >> 10));
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_POST:
        {
            SecBuff = &pDmxDevOsi->DmxChanSecBuff[pChanInfo->secBuffId];
            buffId = SecBuff->BuffId;
            reg_set_bufn_size_data_ch_rptr(buffId, (SecBuff->data_buff_write_p >> 10));
            MT_DBG_DEMUX("TYPE_POST :: channel %d, buffId=%d\n", u32ChId, buffId);
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            s32Ret = DMX_OsiReleaseReadPes(pChanInfo);
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_HW_PES:
        {
            s32Ret = DMX_OsiReleaseReadHwPes(pChanInfo, u32RelNum, psMsgList);
            break;
        }

        default:
        {
            MT_ERR_DEMUX("not support type:%d, chanid:%d\n", pChanInfo->ChanType, u32ChId);
            s32Ret = MT_ERR_DMX_INVALID_PARA;
            break;
        }
    }

    return s32Ret;
}

/*
 * This is used to peek the first u32PeekLen of a SEC or PES packet. It does not consume the data in
 * the internal demux section buffer, i.e. it simply copies the requested amount of data
 * but does not move the consumer pointer.
 */
mt_s32 DMX_OsiPeekDataRequest(mt_u32 u32ChId,
                              mt_u32 u32PeekLen,
                              DMX_UserMsg_S *psMsgList)
{
    DMX_ChanInfo_S *pChanInfo;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S stTmpChanInfo;
    DMX_OQ_Info_S tmpOqInfo;
    mt_u32 u32AcqedNum = 0;
    mt_s32 s32Ret;

    IsDmxDevInit();
    pChanInfo = &pDmxDevOsi->DmxChanInfo[u32ChId];
    if (MT_UNF_DMX_CHAN_PLAY_EN != (MT_UNF_DMX_CHAN_PLAY_EN & pChanInfo->ChanStatus)) 
    {
        MT_WARN_DEMUX("channel %d is not open yet or not play channel\n", u32ChId);
        return MT_ERR_DMX_NOT_SUPPORT;
    }
	
    if ((MT_UNF_DMX_CHAN_TYPE_AUD == pChanInfo->ChanType) ||
        (MT_UNF_DMX_CHAN_TYPE_AUD_AD == pChanInfo->ChanType) ||
        (MT_UNF_DMX_CHAN_TYPE_VID == pChanInfo->ChanType))
    {
        return MT_ERR_DMX_NOT_SUPPORT; // do not receive vid and aud channel
    }
    
    if (!DMX_OsiGetChnDataFlag(u32ChId)) //channel have no data
    {
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }
	
    if (MT_UNF_DMX_CHAN_TYPE_PES == pChanInfo->ChanType) {
	mt_u32 u32CurDropCnt;
	u32CurDropCnt = s_u32DMXDropCnt[pChanInfo->ChanId];
	memcpy(&stTmpChanInfo, pChanInfo, sizeof(DMX_ChanInfo_S));
	pChanInfo->u32ChnResetLock = 1;
	s32Ret = DMXOsiFindPes(&stTmpChanInfo, psMsgList, 16, &u32AcqedNum);
	if (u32CurDropCnt != s_u32DMXDropCnt[pChanInfo->ChanId]) //drop occurs
	{
	    pChanInfo->u32PesBlkCnt = 0;
	    pChanInfo->u32PesLength = 0;
	    pChanInfo->u32ProcsOffset = 0;
	}
	pChanInfo->u32ChnResetLock = 0;
    } else {
	memcpy(&stTmpChanInfo, pChanInfo, sizeof(DMX_ChanInfo_S));
	memcpy(&tmpOqInfo, &g_pDmxDevOsi->DmxOqInfo[pChanInfo->ChanOqId], sizeof(DMX_OQ_Info_S));
	pChanInfo->u32ChnResetLock = 1;
	s32Ret = DMXOsiReadSec(&stTmpChanInfo, 1, &u32AcqedNum, psMsgList, &tmpOqInfo, 0);
	pChanInfo->u32ChnResetLock = 0;
    }

    if (MT_SUCCESS == s32Ret && u32AcqedNum) {
	return MT_SUCCESS;
    }
    return MT_ERR_DMX_NOAVAILABLE_DATA;
}

static mt_s32 DMX_ESDescriptorParse(dmx_es_pkt_info *pkt_info, dmx_es_desc_data_s *buf)
{
    if (buf == NULL) {
        MT_ERR_DEMUX("invalid buffer(%p), size(%u)!\n", buf);
        return MT_FAILURE;
    }

    if ((buf->u32Pts & 0x80000000) == 0) {
        //printk(KERN_DEBUG "%s: invalid PTS(0x%x) flag!\n",__FUNCTION__, buf->u32Pts);
        //return MT_FAILURE;
    }

    pkt_info->origPTS = ((buf->u32Info >> 24) & 0x03);            /* bit[32:31] */
    pkt_info->origPTS <<= 31;
    pkt_info->origPTS |= (buf->u32Pts & 0x7FFFFFFF);              /* bit[30:0] */
    pkt_info->u64PTS = pkt_info->origPTS * 1000;
    do_div(pkt_info->u64PTS, 90);
    pkt_info->u32EsPhyAddr = buf->u32Addr;
    pkt_info->pts_valid = (buf->u32Pts >> 31) & 0x1;

    return MT_SUCCESS;
}

#define CHANGE_BETWEEN_LIT_AND_BIG_END(X) ((X >> 24) | \
    (((X >> 16) & 0XFF) << 8) | (((X >> 8) & 0XFF) << 16) | (X << 24))
    
#if !defined(CONFIG_MT_CHIP_SYMPHONY4) && !defined(CONFIG_MT_CHIP_SYMPHONY6)
static mt_u32 get_pkt_cnt(mt_u8 *data, mt_u32 len, mt_u32 u32pts)
{
    mt_u32 u32pts_tmp;
    mt_u32 offset = DMX_1_PACK_LEN_PTS;

    while (offset < len)
    {
    	u32pts_tmp = *(mt_u32 *)(data + offset + DMX_1_PACK_LEN_NOPTS);

    	if (u32pts_tmp != u32pts)
    		break;

    	offset += DMX_1_PACK_LEN_PTS;
    }

	//printk(KERN_DEBUG "%s: data %p, len %u, offset %u\n",__FUNCTION__, data, len, offset);

    return (offset / DMX_1_PACK_LEN_PTS);
}
#endif
void *DMX_OsiNextPktDesc(DMX_ChanEsBuff_S *chan_buf, mt_u32 chan_id)
{
    dmx_es_desc_data_s *pkt_desc = NULL;
    mt_u32 valid = 0, data_rp = 0, data_wp = 0;
    dmx_es_pkt_info *p_pkt_info = &chan_buf->next_desc;
    mt_s32 ret = 0;

	do
	{
        DMXOsiChnEsDescDataGetReadWrite(chan_id, &data_wp, &data_rp);
        if(data_wp >= data_rp)
            valid = data_wp - data_rp;
        else
            valid = chan_buf->desc_buff.size - data_rp;
        
        if(valid < sizeof(dmx_es_desc_data_s))
            return NULL;

        pkt_desc = chan_buf->desc_buff.startVirAddr + data_rp;
		
        ret = DMX_ESDescriptorParse(p_pkt_info, pkt_desc);
        
        /* update desc rp */
        data_rp = (data_rp + sizeof(dmx_es_desc_data_s)) % chan_buf->desc_buff.size;
        reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(chan_id, data_rp);

        if(ret == MT_SUCCESS)
		    return p_pkt_info;

	} while (1);
}

static dmx_es_pkt_info *DMX_OsiGetAudPktDescByESAddr(DMX_ChanEsBuff_S *chan_buf,
                                        mt_u32 chan_id,
                                        mt_u32 phy_es_addr,
                                        mt_u32 *size)
{
	mt_u32 es_addr1, es_addr2;
	//mt_s32 ret;
  
    if(chan_id >= DMX_AV_CHANNEL_CNT)
        return NULL;

    if(chan_buf->p_cur_desc == NULL) {
        dmx_es_pkt_info *p;
        
        p = DMX_OsiNextPktDesc(chan_buf, chan_id);
        if(p == NULL) {
            MT_ERR_DEMUX("no current pts desc\n");
            return NULL;
        }
        memcpy(&chan_buf->cur_desc, p, sizeof(chan_buf->cur_desc));
        chan_buf->p_cur_desc = &chan_buf->cur_desc;
    }

	do
	{
		if (chan_buf->p_next_desc == NULL) {
            dmx_es_pkt_info *p;
        
            p = DMX_OsiNextPktDesc(chan_buf, chan_id);
            if(p == NULL) {
                //printk(KERN_DEBUG "no next pts desc\n");
                break;
            }
            chan_buf->p_next_desc = p;
		}
        
        es_addr1 = chan_buf->p_cur_desc->u32EsPhyAddr;
        es_addr2 = chan_buf->p_next_desc->u32EsPhyAddr;
		if (es_addr1 < es_addr2) {
			if (phy_es_addr >= es_addr1 && phy_es_addr < es_addr2) {
				//debug packet size
				//printf("%s: esAddr2(0x%x)-phy_es_addr(0x%x)=0x%x\n",__FUNCTION__,
				//	esAddr2,phy_es_addr,esAddr2-phy_es_addr);

				/* packet size */
				if (size != NULL)
					*size = es_addr2 - phy_es_addr;

				break;
			}
		} else if (es_addr2 < es_addr1) { //rewind
			if (phy_es_addr >= es_addr1 || phy_es_addr < es_addr2) {
				/* packet size */
				if (phy_es_addr < es_addr2 && size != NULL)
					*size = es_addr2 - phy_es_addr;
				break;
			}
		}

        memcpy(&chan_buf->cur_desc, chan_buf->p_next_desc, sizeof(chan_buf->cur_desc));
        chan_buf->p_next_desc = NULL;
	} while (1);

	return chan_buf->p_cur_desc;
}

static mt_s32 DMX_OsiGetAudPtsByESAddr(mt_u32 chan_id,
                                    DMX_ChanEsBuff_S *chan_buf,
                                    mt_u32 phy_es_addr,
                                    mt_u32 *pts32,
                                    mt_u64 *pts64,
                                    mt_u32 *size,
                                    mt_u32 *pts_valid)
{
    dmx_es_pkt_info *desc = NULL;

    desc = DMX_OsiGetAudPktDescByESAddr(chan_buf, chan_id, phy_es_addr, size);
    if (desc == NULL) {
        return MT_FAILURE;
    }
  
    *pts32 = (mt_u32)(desc->origPTS >> 1);
    *pts64 = desc->u64PTS;
    *pts_valid = desc->pts_valid;

    //debug
    if (0) {
        MT_ALWAYS_PRINT("found ES(0x%08x) - DESC(0x%08x, PTS33 0x%llx, PTS64 0x%llx)\n",
            phy_es_addr,
            desc->u32EsPhyAddr,
            desc->origPTS,
            *pts64);
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiReadPesRequest(DMX_ChanInfo_S *ChanInfo, DMX_Stream_S *EsData)
{
    DMX_SOFT_PESHEADER_S *peshead = NULL;
    DMX_SOFT_PES_NODE_S *p_pes_node = NULL;

    peshead = ChanInfo->softpeshead;

    if (!peshead)
    {
        MT_ERR_DEMUX("peshead is null\n");
        return MT_ERR_DMX_NULL_PTR;
    }

    mutex_lock(&ChanInfo->chan_mutex);

    if (!list_empty_careful(&ChanInfo->softpeshead->pes_used_node_list))
    {
        p_pes_node = list_entry(ChanInfo->softpeshead->pes_used_node_list.next, DMX_SOFT_PES_NODE_S, pes_node);

        if ((p_pes_node->pes_node_len) && (p_pes_node->pes_node_addr) && (p_pes_node->pes_node_merged))
        {
            EsData->u32BufPhyAddr = (p_pes_node->pes_node_addr - (ulong)peshead->pesdata + peshead->pesphyadr);
            EsData->u32BufVirAddr = p_pes_node->pes_node_addr;
            EsData->u32BufLen = p_pes_node->pes_node_len;
			MT_INFO_DEMUX("[%s %d]p_pes_node=0x%x, p_pes_node->pes_node_len=%d\r\n", __FUNCTION__, __LINE__, p_pes_node, p_pes_node->pes_node_len);
            /*
            if the pes node read, del it from the pes_used_node_list
            */
            list_del_init(&p_pes_node->pes_node);
            /*
            then add it to the pes_read_node_list, the purpose of that
            is convenient for release pes node
            */
            list_add_tail(&p_pes_node->pes_node, &peshead->pes_read_node_list);

            ChanInfo->u32countRef++;
			//the data has been read by user, so clear the flag.		
			ChanInfo->pes_wait_flag = 0;
            mutex_unlock(&ChanInfo->chan_mutex);
            return MT_SUCCESS;
        }
        else
        {
            MT_DBG_DEMUX("pes_used_node_list's node not merged, addr = %#x or len = %d, mer = %d\n", 
                p_pes_node->pes_node_addr, p_pes_node->pes_node_len, p_pes_node->pes_node_merged);
            mutex_unlock(&ChanInfo->chan_mutex);
            return MT_ERR_DMX_NOAVAILABLE_DATA;
        }
    }
    else
    {
        MT_DBG_DEMUX("pes_used_node_list empty Err\n");
        mutex_unlock(&ChanInfo->chan_mutex);
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }
}

mt_s32 DMX_OsiReadAudesRequest(DMX_ChanInfo_S *ChanInfo, DMX_Stream_S *EsData)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanEsBuff_S * es_buf = &pDmxDevOsi->DmxChanEsBuff[ChanInfo->avChanId];
    mt_u32 data_wp = 0;
    mt_u32 data_rp = 0;
    mt_u32 data_valid = 0;
    mt_u32 pkt_cnt = 1;
    mt_u32 cur_pts = 0;
    static mt_u32 pre_pts = 0;
    mt_u32 u32pts;
    mt_u64 u64pts_us;
    mt_u32 pkt_size = data_valid;
	mt_s32 ret;
	mt_u32 pts_valid = 0;

    if (MT_TRUE == ChanInfo->aes_to_user)
    {
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }
    DMXOsiChnEsDataGetReadWrite(ChanInfo->avChanId, &data_wp, &data_rp);

    if (data_wp >= data_rp)
    {
        data_valid = data_wp - data_rp;
    } 
    else
    {
        data_valid = es_buf->data_buff.size - data_rp;
    }

    data_valid = data_valid & ((mt_u32)(~(DMX_1_PACK_LEN_PTS-1)));
    
    if (data_valid < DMX_1_PACK_LEN_PTS)
    {
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }

    EsData->u32BufVirAddr = (ulong)es_buf->data_buff.startVirAddr + data_rp;
    EsData->u32BufPhyAddr = es_buf->data_buff.startPhyAddr + data_rp;

    pkt_size = data_valid;
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
	ret = DMX_OsiGetAudPtsByESAddr(ChanInfo->avChanId, es_buf, EsData->u32BufPhyAddr, &u32pts, &u64pts_us, &pkt_size, &pts_valid);

    if (ret != MT_SUCCESS)
    {
        u64pts_us = MT_INVALID_PTS_U64;
    }

    EsData->u64PtsMs = u64pts_us;
	EsData->u32PtsMs = u32pts;

    if (pkt_size >= DMX_1_PACK_LEN_PTS)
    {
        pkt_cnt = pkt_size / DMX_1_PACK_LEN_PTS;
    }
#else
    mt_u8 *pdata = (mt_u8 *)EsData->u32BufVirAddr;

    u32pts = *((mt_u32 *)(&pdata[DMX_1_PACK_LEN_NOPTS]));

    pkt_cnt = get_pkt_cnt(pdata, data_valid, u32pts);

    u32pts = CHANGE_BETWEEN_LIT_AND_BIG_END(u32pts);
    pts_valid = (u32pts >> 31) & 0x1;
    u32pts = u32pts << 1;
    u64pts_us = u32pts;
    u64pts_us *= 1000;
    do_div(u64pts_us, 45);
    EsData->u64PtsMs = u64pts_us;
#endif
    
    if (pkt_cnt > 96)
    {
        pkt_cnt = 96;
    }

    EsData->u32BufLen = (DMX_1_PACK_LEN_PTS * pkt_cnt);
	EsData->pts_valid = pts_valid;
	
    cur_pts = EsData->u32PtsMs;
#if 0
    if (pre_pts != cur_pts)
    {
		mt_u32 pts_diff = 0;
        pts_diff = abs(cur_pts - pre_pts);
        do_div(pts_diff, 45);
        DMX_LOGAP("dmx apts: 0x%x, dif: %d\n", cur_pts, pts_diff);
    }
#endif
    pre_pts = cur_pts;

   return MT_SUCCESS;
}


mt_s32 DMX_OsiReadVidesRequest(DMX_ChanInfo_S *ChanInfo, DMX_Stream_S *EsData)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanEsBuff_S *EsBuff = &pDmxDevOsi->DmxChanEsBuff[ChanInfo->avChanId];
    mt_u32 u32EsBuffSize = EsBuff->data_buff.size;
    mt_u32 DataLen = 0;
    phys_addr_t DataPhyAddr = 0;
    ulong DataKerAddr = 0;
    mt_u32 u32EsBuffWritePointer = 0;
    mt_u32 u32EsBuffReadPointer = 0;

    DMXOsiChnEsDataGetReadWrite(ChanInfo->avChanId, &u32EsBuffWritePointer, &u32EsBuffReadPointer);

    if (u32EsBuffReadPointer == u32EsBuffWritePointer)
    {
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }
    else if (u32EsBuffWritePointer > u32EsBuffReadPointer)
    {
        DataLen = u32EsBuffWritePointer - u32EsBuffReadPointer;
		ChanInfo->u32BuffWriteP = u32EsBuffWritePointer;
        //reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ChanInfo->avChanId, u32EsBuffWritePointer);
    }
    else
    {
        DataLen = u32EsBuffSize - u32EsBuffReadPointer;
        //reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ChanInfo->avChanId, 0);
        ChanInfo->u32BuffWriteP = 0;
    }
    
    DataPhyAddr = EsBuff->data_buff.startPhyAddr + u32EsBuffReadPointer;
    DataKerAddr = (ulong)EsBuff->data_buff.startVirAddr + u32EsBuffReadPointer;
    
    EsData->u32BufPhyAddr = DataPhyAddr;
    EsData->u32BufVirAddr = DataKerAddr;
    EsData->u32BufLen = DataLen;
    EsData->u32PtsMs = 0;
    ChanInfo->LastPts = INVALID_PTS;    
    ChanInfo->u32HitAcq++;
    return MT_SUCCESS;
}


mt_s32 DMX_OsiReadEsRequest(mt_u32 ChanId, DMX_Stream_S *EsData)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = NULL;
    DMX_Sub_DevInfo_S *DmxInfo = MT_NULL;
    mt_s32 ret = MT_SUCCESS;

    if (ChanId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("invalid ChanId = %#x\n", ChanId);
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];

    if (ChanInfo->DmxId >= DMX_CNT) 
    {
        MT_WARN_DEMUX("invalid demux id:%d\n", ChanInfo->DmxId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    DmxInfo = &pDmxDevOsi->SubDevInfo[ChanInfo->DmxId];

    if (DMX_INVALID_PORT_ID == DmxInfo->PortId)
    {
        MT_WARN_DEMUX("demux id:%d not attached with any port\n", ChanInfo->DmxId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (MT_UNF_DMX_CHAN_PLAY_EN != (MT_UNF_DMX_CHAN_PLAY_EN & ChanInfo->ChanStatus))
    {
        MT_WARN_DEMUX("channel %d is not open yet or not play channel\n", ChanId);
        return MT_ERR_DMX_NOT_OPEN_CHAN;
    }

    ChanInfo->u32TotolAcq++;

    switch (ChanInfo->ChanType)
    {
        case MT_UNF_DMX_CHAN_TYPE_AUD:
        case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
        {
            ret = DMX_OsiReadAudesRequest(ChanInfo, EsData);
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_VID:
        case MT_UNF_DMX_CHAN_TYPE_DSS:
        {
            ret = DMX_OsiReadVidesRequest(ChanInfo, EsData);
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            ret = DMX_OsiReadPesRequest(ChanInfo, EsData);
            break;
        }

        default:
        {
            MT_ERR_DEMUX("Read type:%d not allowed, chanid:%d\n",
                ChanInfo->ChanType, ChanInfo->ChanId);
            ret = MT_ERR_DMX_INVALID_PARA;
            break;
        }
    }

    if (MT_SUCCESS == ret)
    {
        ChanInfo->u32HitAcq++;
    }

    return ret;
}


#define AUDIO_TRACK_AV_PTS_OFFSET    (300*45)

mt_s32 DMX_OsiReadEsRequestByPtsForAudTrack(mt_u32 ChanId, DMX_Stream_S *EsData, mt_u32 pts)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    DMX_Sub_DevInfo_S *DmxInfo = MT_NULL;
    mt_u32 vpts = 0;
	mt_u32 pkt_size = 0;
	
#if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6))	
	dmx_es_desc_data_s *p_desc = NULL;
	dmx_es_pkt_info pkt_info; 
	mt_u32 desc_rd = 0;
	mt_u32 min_desc_rd = 0;
	mt_u32 es_phy_start = 0;
	mt_u32 max0_desc_rd = 0;
	mt_u32 min1_desc_rd = 0;
	mt_s32 ret = MT_SUCCESS;
#else
	mt_u8 *p_data = NULL;
    mt_u32 len = 0;
    mt_u32 pre_apts = 0;
	mt_u32 u32pts;
    mt_u64 u64pts_us;
	mt_u8 *pdata = NULL;
#endif

    if (MT_TRUE == ChanInfo->aes_to_user)
    {
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }

    if (ChanInfo->DmxId >= DMX_CNT)
    {
        MT_WARN_DEMUX("invalid demux %d\n", ChanInfo->DmxId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (NULL == EsData)
    {
        MT_WARN_DEMUX("invalid es para\n");
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (0xffffffff == pts)
    {
        MT_WARN_DEMUX("invalid input pts\n");
        return MT_ERR_DMX_INVALID_PARA;       
    }

	DmxInfo = &pDmxDevOsi->SubDevInfo[ChanInfo->DmxId];
	
    if (DMX_INVALID_PORT_ID == DmxInfo->PortId)
    {
        MT_WARN_DEMUX("demux(%d) not attached with any port .\n", ChanInfo->DmxId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (MT_UNF_DMX_CHAN_PLAY_EN != (MT_UNF_DMX_CHAN_PLAY_EN & ChanInfo->ChanStatus))
    {
        MT_WARN_DEMUX("channel %d is not open yet or not play channel\n", ChanId);
        return MT_ERR_DMX_NOT_OPEN_CHAN;
    }

    if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) || (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType))
    {
        DMX_ChanEsBuff_S * es_buf = &pDmxDevOsi->DmxChanEsBuff[ChanInfo->avChanId];
        mt_u32 apts = 0;
        mt_u32 diff = 0;
        mt_u32 min_diff = 0xffffffff;
        mt_u32 min_pts = 0;
        mt_u32 es_rd = 0;
        mt_u64 min_u64pts_us = 0;
        mt_u32 pkt_cnt = 1;
        mt_u32 data_wp = 0;
        mt_u32 data_rp = 0;
        mt_u32 data_valid = 0;

        mt_u32 max0_diff = 0;
        mt_u32 max0_pts = 0;
        mt_u64 max0_u64pts_us = 0;
        mt_u32 es0_rd = 0;

        mt_u32 min1_diff = 0xffffffff;
        mt_u32 min1_pts = 0;
        mt_u64 min1_u64pts_us = 0;
        mt_u32 es1_rd = 0;

        vpts = pts;

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
        p_desc = (dmx_es_desc_data_s *)es_buf->desc_buff.startVirAddr + desc_rd;
        es_phy_start = es_buf->data_buff.startPhyAddr;

        while (1)
        {
            ret = DMX_ESDescriptorParse(&pkt_info, p_desc);
            if (MT_SUCCESS == ret)
            {
                apts = (mt_u32)(pkt_info.origPTS >> 1); //get the high 32 bit value

                if (apts >= (vpts + AUDIO_TRACK_AV_PTS_OFFSET))
                {
                    diff = apts - vpts;
                    MT_DBG_DEMUX("apts = %#x, dif0: %d\n", apts, diff/45);

                    if (diff < min_diff)
                    {
                        min_diff = diff;
                        min_pts = apts;
                        min_u64pts_us = pkt_info.u64PTS;
                        es_rd = (pkt_info.u32EsPhyAddr - es_phy_start);
                        min_desc_rd = desc_rd;
                    }
                    else
                    {
                        //do nothing
                    }
                    
                }
                else if (apts >= vpts)
                {
                    diff = apts - vpts;
                    MT_DBG_DEMUX("apts = %#x, dif1: %d\n", apts, diff/45);

                    if (diff >= max0_diff)
                    {
                        max0_diff = diff;
                        max0_pts = apts;
                        max0_u64pts_us = pkt_info.u64PTS;
                        es0_rd = (pkt_info.u32EsPhyAddr - es_phy_start);
                        max0_desc_rd = desc_rd;
                    }
                    else
                    {
                        //do nothing
                    }
                }                
                else  //apts < vpts
                {
                    if (apts != 0)
                    {
                        diff = vpts - apts;
                        MT_DBG_DEMUX("apts = %#x, dif2: %d\n", apts, diff/45);

                        if (diff < min1_diff)
                        {
                            min1_diff = diff;
                            min1_pts = apts;
                            min1_u64pts_us = pkt_info.u64PTS;    
                            es1_rd = (pkt_info.u32EsPhyAddr - es_phy_start);
                            min1_desc_rd = desc_rd;
                        }
                        else
                        {
                            //do nothing
                        }  
                    }
                    else
                    {
                        //do nothing
                    }
                }

            }
            else
            {
                MT_ERR_DEMUX("audio descriptor pase failed!!\n");
            }

            if (desc_rd < (es_buf->desc_buff.size - sizeof(dmx_es_desc_data_s)))
            {
                desc_rd += sizeof(dmx_es_desc_data_s);
				p_desc++;
            }
            else
            {
                /* update desc rp, next logic is not the perfect, if min_pts and
                max0_pts are both not 0, drv need more check, keep watching
                */
                if (min_pts)
                {
                    reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(ChanInfo->avChanId, min_desc_rd);
                    MT_DBG_DEMUX("find min pts, es_rd: %#x - %#lx, avpts: %#x - %#x, dif: %d\n",
                        es_rd, (ulong)es_buf->data_buff.startVirAddr + es_rd, min_pts, vpts, abs(min_pts - vpts)/45);
                }
                else if (max0_pts)
                {
                    reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(ChanInfo->avChanId, max0_desc_rd);
                    min_u64pts_us = max0_u64pts_us;
					min_pts = max0_pts;
                    es_rd = es0_rd;
                    MT_DBG_DEMUX("find min pts, es0_rd: %#x - %#lx, avpts: %#x - %#x, dif: %d\n",
                        es0_rd, (ulong)es_buf->data_buff.startVirAddr + es0_rd, max0_pts, vpts, abs(max0_pts - vpts)/45);
                }
                else if (min1_pts)
                {
                    reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(ChanInfo->avChanId, min1_desc_rd);
                    min_u64pts_us = min1_u64pts_us;
					min_pts = min1_pts;
                    es_rd = es1_rd;
                    MT_DBG_DEMUX("find min pts, es1_rd: %#x - %#lx, avpts: %#x - %#x, dif: %d\n",
                        es1_rd, (ulong)es_buf->data_buff.startVirAddr + es1_rd, min1_pts, vpts, abs(min1_pts - vpts)/45);
                }
                else
                {
                    MT_ERR_DEMUX("%s, %d, find apts error\n", __func__, __LINE__);
                }
				
                break;
			}			
        }

#else            	
    	
        p_data = (mt_u8 *)es_buf->data_buff.startVirAddr;
        len = DMX_1_PACK_LEN_PTS;
		
        while (1)
        {
            /*
            because apts is inserted in audio es data, the default configuration is inserting a apts every 64 bytes
            so next logic is not perfect, it should read the configuration in dmx registor to get how many bytes
            inserting a apts.
            */
            apts = *((mt_u32 *)(&p_data[len - 8]));    //the pts at pkt_len_with_pts*n +56
            apts = CHANGE_BETWEEN_LIT_AND_BIG_END(apts);    
			pkt_info.pts_valid = (apts >> 31) & 0x1;
            apts = apts << 1;     //the  32bit is  valid-flag
    	
            if (pre_apts != apts) 
            {
                if (apts >= (vpts + AUDIO_TRACK_AV_PTS_OFFSET))
                {
                    diff = apts - vpts;
    			
                    //printk(KERN_ERR"apts = %#x, dif: %d\n", apts, abs(apts - vpts)/45);
    			
                    if (diff < min_diff)
                    {
                        min_diff = diff;
                        es_rd = len;
                        min_pts = apts;
                    }
                    else
                    {
                        //do nothing
                    }
                    
                }
                else
                {
                    //should handle for develop
                    //printk(KERN_ERR "cur found apts:%x < vpts:%x, dif: %d\n", apts, vpts, abs(vpts - apts)/45);
                }
            }
            else
            {
               //do nothing, there are many same apts in audio es data
            }

            pre_apts = apts;
    	
            if (len < (es_buf->data_buff.size - DMX_1_PACK_LEN_PTS))
            {
                len += DMX_1_PACK_LEN_PTS;
            }
            else
            {
                MT_DBG_DEMUX("find min pts, es_rd: %#x, avpts: %#x - %#x, dif: %d\n",
                    es_rd, min_pts, vpts, abs(min_pts - vpts)/45);
                break;
            }			
        }

#endif

        /*
        patch for bug133944, after check the audio es buf, es_rd may not 64 byte
        aligned, when read at the bottom of the es buf, the left data will less
        than 64 byte, and drv will not update data to adec
        */
        if (es_rd & (DMX_1_PACK_LEN_PTS - 1))
        {
            es_rd = (es_rd + DMX_1_PACK_LEN_PTS - 1) / DMX_1_PACK_LEN_PTS * DMX_1_PACK_LEN_PTS;
        }

        reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ChanInfo->avChanId, es_rd % es_buf->data_buff.size);

        /* get valid data size */
        DMXOsiChnEsDataGetReadWrite(ChanInfo->avChanId, &data_wp, &data_rp);
        if (data_wp >= data_rp)
        {
            data_valid = data_wp - data_rp;
        }
        else
        {
            data_valid = es_buf->data_buff.size - data_rp;
        }
        
        /* align data size */
        data_valid = data_valid & ((mt_u32)(~(DMX_1_PACK_LEN_PTS-1)));
        
        if(data_valid < DMX_1_PACK_LEN_PTS)
            return MT_ERR_DMX_NOAVAILABLE_DATA;
        
        EsData->u32BufVirAddr = (ulong)es_buf->data_buff.startVirAddr + data_rp;
        EsData->u32BufPhyAddr = (phys_addr_t)es_buf->data_buff.startPhyAddr + data_rp;
        EsData->pts_valid = pkt_info.pts_valid;
                
        pkt_size = data_valid;
            
#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)

        EsData->u64PtsMs = min_u64pts_us;
		EsData->u32PtsMs = min_pts;

        if (pkt_size >= DMX_1_PACK_LEN_PTS)
        {
            pkt_cnt = pkt_size / DMX_1_PACK_LEN_PTS;
        }
#else
        pdata = (mt_u8 *)EsData->u32BufVirAddr;

        // get pts data form es buffer
        u32pts = *((mt_u32 *)(&pdata[DMX_1_PACK_LEN_NOPTS]));

        pkt_cnt = get_pkt_cnt(pdata, data_valid, u32pts);

        u32pts = CHANGE_BETWEEN_LIT_AND_BIG_END(u32pts);
        u32pts = u32pts << 1;     //the  32bit is  valid-flag

        // change to 64 bit pts
        u64pts_us = u32pts;
        u64pts_us *= 1000;// change unit to us
        do_div(u64pts_us, 45);

        EsData->u64PtsMs = u64pts_us;
#endif       
    
        /* max TS Packet count limit */
        if (pkt_cnt > 96)
        {
            pkt_cnt = 96;
        }

        EsData->u32BufLen = (DMX_1_PACK_LEN_PTS * pkt_cnt);

        return MT_SUCCESS;
    }    
    else
    {
        return MT_ERR_DMX_NOT_SUPPORT;
    }
	
    return MT_ERR_DMX_NOT_SUPPORT;
}

#define SEEK_TO_PLAY_AV_PTS_OFFSET      (150*45)
#define SEEK_TO_PLAY_AV_PTS_MAX_OFFSET  (8000*45)
#define SEEK_TO_PLAY_CHECK_PTS_TIME     (3000)

mt_s32 DMX_OsiReadEsRequestByPtsForTrickSeek(mt_u32 ChanId, DMX_Stream_S *EsData, mt_u32 pts)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
    DMX_Sub_DevInfo_S *DmxInfo = MT_NULL;
    mt_u32 vpts = 0;
    mt_u32 pkt_size = 0;
    dmx_es_desc_data_s *p_desc = NULL;
    dmx_es_pkt_info pkt_info;
    mt_u32 desc_rd = 0;
    mt_u32 desc_wr = 0;
    mt_u32 min_desc_rd = 0;
    mt_u32 es_phy_start = 0;
    mt_u32 max0_desc_rd = 0;
    mt_u32 min1_desc_rd = 0;
    mt_s32 ret = MT_SUCCESS;

    if (MT_TRUE == ChanInfo->aes_to_user)
    {
        return MT_ERR_DMX_NOAVAILABLE_DATA;
    }

    if (ChanInfo->DmxId >= DMX_CNT)
    {
        MT_WARN_DEMUX("invalid demux %d\n", ChanInfo->DmxId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (NULL == EsData)
    {
        MT_WARN_DEMUX("invalid es para\n");
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (0xffffffff == pts)
    {
        MT_WARN_DEMUX("invalid input pts\n");
        return MT_ERR_DMX_INVALID_PARA;
    }

    DmxInfo = &pDmxDevOsi->SubDevInfo[ChanInfo->DmxId];

    if (DMX_INVALID_PORT_ID == DmxInfo->PortId)
    {
        MT_WARN_DEMUX("demux(%d) not attached with any port .\n", ChanInfo->DmxId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (MT_UNF_DMX_CHAN_PLAY_EN != (MT_UNF_DMX_CHAN_PLAY_EN & ChanInfo->ChanStatus))
    {
        MT_WARN_DEMUX("channel %d is not open yet or not play channel\n", ChanId);
        return MT_ERR_DMX_NOT_OPEN_CHAN;
    }

    if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) || (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType))
    {
        DMX_ChanEsBuff_S *es_buf = &pDmxDevOsi->DmxChanEsBuff[ChanInfo->avChanId];
        mt_u32 apts = 0;
        mt_u32 diff = 0;
        mt_u32 min_diff = 0xffffffff;
        mt_u32 min_pts = 0;
        mt_u32 es_rd = 0;
        mt_u64 min_u64pts_us = 0;
        mt_u32 pkt_cnt = 1;
        mt_u32 data_wp = 0;
        mt_u32 data_rp = 0;
        mt_u32 data_valid = 0;

        mt_u32 max0_diff = 0;
        mt_u32 max0_pts = 0;
        mt_u64 max0_u64pts_us = 0;
        mt_u32 es0_rd = 0;

        mt_u32 min1_diff = 0xffffffff;
        mt_u32 min1_pts = 0;
        mt_u64 min1_u64pts_us = 0;
        mt_u32 es1_rd = 0;

        vpts = pts;
        DMXOsiChnEsDescDataGetReadWrite(ChanInfo->avChanId, &desc_wr, &desc_rd);
        if (0 == desc_wr)
        {
            return MT_ERR_DMX_NOAVAILABLE_DATA;
        }
        desc_rd = 0;
        p_desc = (dmx_es_desc_data_s *)es_buf->desc_buff.startVirAddr + desc_rd;
        es_phy_start = es_buf->data_buff.startPhyAddr;

        if (0 == ChanInfo->pts_check_tick)
        {
            ChanInfo->pts_check_tick = ktime_to_ms(ktime_get_boottime());
        }

        while (1)
        {
            ret = DMX_ESDescriptorParse(&pkt_info, p_desc);
            if (MT_SUCCESS == ret)
            {
                apts = (mt_u32)(pkt_info.origPTS >> 1); //get the high 32 bit value

                //fix 26277, when seek in/out, can't sync at sometime for special stream
				if ((apts >= (vpts + SEEK_TO_PLAY_AV_PTS_OFFSET)) && (apts <= (vpts + SEEK_TO_PLAY_AV_PTS_MAX_OFFSET)))	
                {
                    diff = apts - vpts;
                    MT_DBG_DEMUX("apts = %#x, dif0: %d - %#x\n", apts, diff/45, desc_rd);

                    if (diff < min_diff)
                    {
                        min_diff = diff;
                        min_pts = apts;
                        min_u64pts_us = pkt_info.u64PTS;
                        es_rd = (pkt_info.u32EsPhyAddr - es_phy_start);
                        min_desc_rd = desc_rd;
                    }
                    else
                    {
                        //do nothing
                    }
                }
                else if (apts >= vpts)
                {
                    diff = apts - vpts;
                    MT_DBG_DEMUX("apts = %#x, dif1: %d - %#x\n", apts, diff/45, desc_rd);

                    if (diff >= max0_diff)
                    {
                        max0_diff = diff;
                        max0_pts = apts;
                        max0_u64pts_us = pkt_info.u64PTS;
                        es0_rd = (pkt_info.u32EsPhyAddr - es_phy_start);
                        max0_desc_rd = desc_rd;
                    }
                    else
                    {
                        //do nothing
                    }
                }
                else  //apts < vpts
                {
                    if (apts != 0)
                    {
                        diff = vpts - apts;
                        MT_DBG_DEMUX("apts = %#x, dif2: %d - %#x\n", apts, diff/45, desc_rd);

                        if (diff < min1_diff)
                        {
                            min1_diff = diff;
                            min1_pts = apts;
                            min1_u64pts_us = pkt_info.u64PTS;
                            es1_rd = (pkt_info.u32EsPhyAddr - es_phy_start);
                            min1_desc_rd = desc_rd;
                        }
                        else
                        {
                            //do nothing
                        }
                    }
                    else
                    {
                        //do nothing
                    }
                }
            }
            else
            {
                MT_ERR_DEMUX("audio descriptor pase failed!!:%#x\n", desc_rd);
            }

            if (desc_rd < (desc_wr - sizeof(dmx_es_desc_data_s)))
            {
                desc_rd += sizeof(dmx_es_desc_data_s);
                p_desc++;
            }
            else
            {
                if (min_pts)
                {
                    reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(ChanInfo->avChanId, min_desc_rd);
                    MT_DBG_DEMUX("find min pts, es_rd: %#x - %#x, aavpts: %#x - %#x - %#x, dif: %d\n",
                        es_rd, es_buf->data_buff.u32StartVirAddr + es_rd, min_pts, max0_pts, vpts, abs(min_pts - vpts)/45);
					pr_err("[%s %d]find min pts, es_rd: %#x, aavpts: %#x - %#x - %#x, dif: %d\n", __FUNCTION__, __LINE__,
                        es_rd, min_pts, max0_pts, vpts, abs(min_pts - vpts)/45);
                        
                }
                else if (max0_pts)
                {
                    reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(ChanInfo->avChanId, max0_desc_rd);
                    min_u64pts_us = max0_u64pts_us;
                    min_pts = max0_pts;
                    es_rd = es0_rd;
                    MT_DBG_DEMUX("find min pts, es0_rd: %#x - %#x, avpts: %#x - %#x, dif: %d\n",
                        es0_rd, es_buf->data_buff.u32StartVirAddr + es0_rd, max0_pts, vpts, abs(max0_pts - vpts)/45);
                    if ((ktime_to_ms(ktime_get_boottime()) - ChanInfo->pts_check_tick) < SEEK_TO_PLAY_CHECK_PTS_TIME)
                        return MT_ERR_DMX_NOAVAILABLE_DATA;
                }
                else if (min1_pts)
                {
                    reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(ChanInfo->avChanId, min1_desc_rd);
                    min_u64pts_us = min1_u64pts_us;
                    min_pts = min1_pts;
                    es_rd = es1_rd;
                    MT_DBG_DEMUX("find min pts, es1_rd: %#x - %#x, avpts: %#x - %#x, dif: %d\n",
                        es1_rd, es_buf->data_buff.u32StartVirAddr + es1_rd, min1_pts, vpts, abs(min1_pts - vpts)/45);
                    if ((ktime_to_ms(ktime_get_boottime()) - ChanInfo->pts_check_tick) < SEEK_TO_PLAY_CHECK_PTS_TIME)
                        return MT_ERR_DMX_NOAVAILABLE_DATA;
                }
                else
                {
                    MT_ERR_DEMUX("%s, %d, find apts error\n", __func__, __LINE__);
                    if ((ktime_to_ms(ktime_get_boottime()) - ChanInfo->pts_check_tick) < SEEK_TO_PLAY_CHECK_PTS_TIME)
                        return MT_ERR_DMX_NOAVAILABLE_DATA;
                }

                break;
            }
        }

        ChanInfo->pts_check_tick = 0;
        /*
        patch for bug133944, after check the audio es buf, es_rd may not 64 byte
        aligned, when read at the bottom of the es buf, the left data will less
        than 64 byte, and drv will not update data to adec
        */
        if (es_rd & (DMX_1_PACK_LEN_PTS - 1))
        {
            es_rd = (es_rd + DMX_1_PACK_LEN_PTS - 1) / DMX_1_PACK_LEN_PTS * DMX_1_PACK_LEN_PTS;
        }

        reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ChanInfo->avChanId, es_rd % es_buf->data_buff.size);

        /* get valid data size */
        DMXOsiChnEsDataGetReadWrite(ChanInfo->avChanId, &data_wp, &data_rp);

        if (data_wp >= data_rp)
        {
            data_valid = data_wp - data_rp;
        }
        else
        {
            data_valid = es_buf->data_buff.size - data_rp;
        }

        /* align data size */
        data_valid = data_valid & ((mt_u32)(~(DMX_1_PACK_LEN_PTS-1)));

        if(data_valid < DMX_1_PACK_LEN_PTS)
            return MT_ERR_DMX_NOAVAILABLE_DATA;

        EsData->u32BufVirAddr = (ulong)(es_buf->data_buff.startVirAddr + data_rp);
        EsData->u32BufPhyAddr = es_buf->data_buff.startPhyAddr + data_rp;
        pkt_size = data_valid;
        EsData->u64PtsMs = min_u64pts_us;
        EsData->u32PtsMs = min_pts;

        if (pkt_size >= DMX_1_PACK_LEN_PTS)
        {
            pkt_cnt = pkt_size / DMX_1_PACK_LEN_PTS;
        }

        /* max TS Packet count limit */
        if (pkt_cnt > 96)
        {
            pkt_cnt = 96;
        }

        EsData->u32BufLen = (DMX_1_PACK_LEN_PTS * pkt_cnt);

        return MT_SUCCESS;
    }
    else
    {
        return MT_ERR_DMX_NOT_SUPPORT;
    }
}

mt_s32 DMX_OsiReleaseReadEs(mt_u32 ChanId, DMX_Stream_S *EsData)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = NULL;
    mt_s32 ret = MT_SUCCESS;

    if (ChanId >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("invalid ChanId = %#x\n", ChanId);
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];

    if (ChanInfo->DmxId >= DMX_CNT)
    {
        MT_WARN_DEMUX("invalid demux id:%d\n", ChanInfo->DmxId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (MT_UNF_DMX_CHAN_PLAY_EN != (MT_UNF_DMX_CHAN_PLAY_EN & ChanInfo->ChanStatus))
    {
        MT_WARN_DEMUX("channel %d is not open yet or not play channel\n", ChanId);
        return MT_ERR_DMX_NOT_OPEN_CHAN;
    }

    switch (ChanInfo->ChanType)
    {
		case MT_UNF_DMX_CHAN_TYPE_VID:
		case MT_UNF_DMX_CHAN_TYPE_DSS:
		{
		    DMX_ChanEsBuff_S * EsBuff = &pDmxDevOsi->DmxChanEsBuff[ChanInfo->avChanId];
		    if((EsBuff != NULL) && (EsBuff->data_buff.size != 0)) 
	            {
	                    mt_u32 data_rp = 0;
	                    data_rp = reg_get_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ChanInfo->avChanId);
	                    data_rp += EsData->u32BufLen;
						MT_ERR_DEMUX("EsData->u32BufLen=0x%x, 0x%x -- 0x%x = 0x%x\n",EsData->u32BufLen,data_rp,EsBuff->desc_buff.size,(data_rp % EsBuff->data_buff.size));
	                    reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ChanInfo->avChanId, data_rp % EsBuff->data_buff.size);
	            }
		    break;
		}
		
		case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            ret = DMX_OsiReleaseReadPes(ChanInfo);
            break;
        }
    
        case MT_UNF_DMX_CHAN_TYPE_AUD:
        case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
        {
            DMX_ChanEsBuff_S * EsBuff = &pDmxDevOsi->DmxChanEsBuff[ChanInfo->avChanId];        
            
            if (EsBuff != NULL)
            {
            
#if !defined(CONFIG_MT_CHIP_SYMPHONY4) && !defined(CONFIG_MT_CHIP_SYMPHONY6)

                if (EsBuff->desc_buff.size != 0) 
                {
                    mt_u32 desc_rp = 0;
    
                    desc_rp = reg_get_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(ChanInfo->avChanId);
    
                    desc_rp += sizeof(dmx_es_desc_data_s);        
                    reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(ChanInfo->avChanId, desc_rp % EsBuff->desc_buff.size);
                }
#endif
                if (EsBuff->data_buff.size != 0) 
                {
                    mt_u32 data_rp = 0;

                    data_rp = reg_get_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ChanInfo->avChanId);
                    data_rp += EsData->u32BufLen;
                    reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ChanInfo->avChanId, data_rp % EsBuff->data_buff.size);
                }
            }

            break;
        }

        default:
        {
            MT_ERR_DEMUX("Release type:%d not allowed, chanid:%d\n",
                ChanInfo->ChanType, ChanInfo->ChanId);
            ret = MT_ERR_DMX_INVALID_PARA;
            break;
        }
    }

    if (MT_SUCCESS == ret)
    {
        ChanInfo->u32Release++;
    }

    return ret;
}


mt_s32 DMX_OsiGetFQBufStatus(mt_u32 FQId, MT_MPI_DMX_BUF_STATUS_S *pBufStat)
{
    mt_u32 regVal, FQval, FQ_WPtr, FQ_RPtr;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_FQ_Info_S *FqInfo;

    FqInfo = &pDmxDevOsi->DmxFqInfo[FQId];

    DmxHalGetFQWORDx(FQId, DMX_FQ_SZWR_OFFSET, &regVal);
    FQ_WPtr = regVal & 0xffff;

    DmxHalGetFQWORDx(FQId, DMX_FQ_RDVD_OFFSET, &regVal);
    FQ_RPtr = regVal & 0xffff;

    if (FQ_WPtr >= FQ_RPtr) {
	FQval = FQ_WPtr - FQ_RPtr;
    } else {
	FQval = FqInfo->u32FQDepth - FQ_RPtr + FQ_WPtr;
    }

    pBufStat->u32BufSize = FqInfo->u32BlockSize * FqInfo->u32FQDepth;
    pBufStat->u32UsedSize = pBufStat->u32BufSize - FQval * FqInfo->u32BlockSize;
    pBufStat->u32BufRptr = FQ_RPtr;
    pBufStat->u32BufWptr = FQ_WPtr;

    return MT_SUCCESS;
}

#if 0
mt_s32 DMX_OsiGetChanBufStatus(mt_u32 ChanId, MT_MPI_DMX_BUF_STATUS_S *BufStatus)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo;
    DMX_OQ_Info_S *OqInfo;
    DMX_ChanEsBuff_S *ChanEsBuff;
    mt_u32 u32ChanEsBuffWptr = 0;
    mt_u32 u32ChanEsBuffRptr = 0;

    IsDmxDevInit();

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];

    CHECKDMXID(ChanInfo->DmxId);

    if(ChanInfo->ChanStatus == MT_UNF_DMX_CHAN_CLOSE)
    {
        MT_ERR_DEMUX("channel %d OQ status not used.\n", ChanId);
	 return MT_ERR_DMX_INVALID_PARA;
    }

    ChanEsBuff = &pDmxDevOsi->DmxChanEsBuff[ChanInfo->avChanId];
    DMXOsiChnEsDataGetReadWrite(ChanInfo->avChanId, &u32ChanEsBuffWptr, &u32ChanEsBuffRptr);
    BufStatus->u32BufSize = ChanEsBuff->data_buff.size;
    BufStatus->u32UsedSize = 0;
    BufStatus->u32BufRptr = u32ChanEsBuffRptr;
    BufStatus->u32BufWptr = u32ChanEsBuffWptr;

    return MT_SUCCESS;
}
#else
mt_s32 DMX_OsiGetChanBufStatus(mt_u32 ChanId, MT_MPI_DMX_BUF_STATUS_S *BufStatus)
{
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo;
    DMX_ChanEsBuff_S *ChanEsBuff;
    mt_u32 u32ChanEsBuffWptr = 0;
    mt_u32 u32ChanEsBuffRptr = 0;
	mt_u32 vdec_rp = 0;
    mt_u32 u32ChanEsDescWptr = 0;
    mt_u32 u32ChanEsDescRptr = 0;

    IsDmxDevInit();

    ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];

    CHECKDMXID(ChanInfo->DmxId);

    if(ChanInfo->ChanStatus == MT_UNF_DMX_CHAN_CLOSE)
    {
        MT_ERR_DEMUX("channel %d OQ status not used.\n", ChanId);
        return MT_ERR_DMX_INVALID_PARA;
    }

	/* FIX: 由于OS/CPU的读寄存器时序问题,应当先读RD指针寄存器 */
	/* 然后再调用 DMXOsiChnEsDataGetReadWrite() 读DMX的写指针寄存器 */
    if(ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_VID)
    {
		vdec_rp = vdec_get_ves_rd(ChanId);
//        reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ChanInfo->avChanId, vdec_rp); //jace add for 121557
        CheckVdecHang(ChanId);
    }

    ChanEsBuff = &pDmxDevOsi->DmxChanEsBuff[ChanInfo->avChanId];
    DMXOsiChnEsDataGetReadWrite(ChanInfo->avChanId, &u32ChanEsBuffWptr, &u32ChanEsBuffRptr);
	DMXOsiChnEsDescDataGetReadWrite(ChanInfo->avChanId, &u32ChanEsDescWptr, &u32ChanEsDescRptr);

    BufStatus->u32BufSize = ChanEsBuff->data_buff.size;
    BufStatus->u32UsedSize = 0;
    BufStatus->u32BufRptr = u32ChanEsBuffRptr;
    BufStatus->u32BufWptr = u32ChanEsBuffWptr;

    if(ChanInfo->ChanType == MT_UNF_DMX_CHAN_TYPE_VID)
    {
        //mt_u32 vdec_rp = dmx_inl(mt_get_vdec_base() + 0xb0) * 8;
        mt_u32 unused_len = 0;
		mt_u32 VidBufPercent = 0;
		if((u32ChanEsBuffWptr != 0) && (u32ChanEsDescWptr != 0))
		{
	        if(u32ChanEsBuffWptr >=  vdec_rp)
	        {
	            unused_len = u32ChanEsBuffWptr - vdec_rp;
	        }
	        else
	        {
	            unused_len =  (ChanEsBuff->data_buff.size - vdec_rp) + u32ChanEsBuffWptr;
	        }
		}
        BufStatus->u32UsedSize = unused_len;
		
		VidBufPercent = BufStatus->u32UsedSize * 100 / BufStatus->u32BufSize;
		if( VidBufPercent > 90)
		{
			vdec_rp = vdec_get_ves_rd(ChanId);
			if((vdec_rp == 0) &&  (u32ChanEsDescRptr != 0))
			{
				BufStatus->u32UsedSize = 0;
			}
		}
	
    }
    else if ((MT_UNF_DMX_CHAN_TYPE_AUD == ChanInfo->ChanType) ||
        (MT_UNF_DMX_CHAN_TYPE_AUD_AD == ChanInfo->ChanType))
    {
        mt_u32 unused_len = 0;
        if(u32ChanEsBuffWptr >=  u32ChanEsBuffRptr)
        {
            unused_len = u32ChanEsBuffWptr - u32ChanEsBuffRptr;
        }
        else
        {
            unused_len =  (ChanEsBuff->data_buff.size - u32ChanEsBuffRptr) + u32ChanEsBuffWptr;
        }
        BufStatus->u32UsedSize = unused_len;
    }

    return MT_SUCCESS;
}

#endif

mt_s32 DMX_OsiSelectDataFlag(mt_u32 *pu32WatchCh, mt_u32 u32WatchNum, mt_u32 *pu32Flag, mt_u32 u32TimeOutMs)
{
    mt_u32 *pu32ChGroup = MT_NULL;

    IsDmxDevInit();

    pu32Flag[0] = 0;
    pu32Flag[1] = 0;
    pu32Flag[2] = 0;
    pu32Flag[3] = 0;

    DMXOsiGetDataFlag(pu32ChGroup, u32WatchNum, pu32Flag);

    if (pu32Flag[0] || pu32Flag[1] || pu32Flag[2] || pu32Flag[3])
    {
	    return MT_SUCCESS;
    }

    return MT_ERR_DMX_NOAVAILABLE_DATA;
}

mt_s32 DMX_OsiPcrChannelCreate(const mt_u32 DmxId, mt_u32 *PcrId)
{
    mt_s32 ret = MT_ERR_DMX_NOFREE_CHAN;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    mt_u32 i;
    mt_u8 pcr_ch = 0;

    for (i = 0; i < DMX_PCR_CHANNEL_CNT; i++) {
	DMX_PCR_Info_S *PcrInfo = &DmxDevOsi->DmxPcrInfo[i];

	if (PcrInfo->DmxId >= DMX_CNT) {
	    PcrInfo->DmxId = DmxId;
	    PcrInfo->PcrPid = DMX_INVALID_PID;
	    PcrInfo->SyncHandle = DMX_INVALID_SYNC_HANDLE;
	    PcrInfo->PcrValue = 0xffffffff;
	    PcrInfo->ScrValue = 0xffffffff;

		if (DmxDevOsi->SubDevInfo[DmxId].PortMode == DMX_PORT_MODE_RAM)
		{
			pcr_ch = DmxDevOsi->SubDevInfo[DmxId].PortId + 4;
		} else {
			pcr_ch = DmxDevOsi->SubDevInfo[DmxId].PortId;

		}

	    DmxHalSetPcrPid(i, DMX_INVALID_PID);
	    reg_set_dmx_tsp_pcrsetn_pcr_ch(i, pcr_ch);
	    *PcrId = i;
	    ret = MT_SUCCESS;

	    break;
	}
    }

    return ret;
}

mt_s32 DMX_OsiPcrChannelDestroy(const mt_u32 PcrId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_PCR_Info_S *PcrInfo = &g_pDmxDevOsi->DmxPcrInfo[PcrId];

    if (PcrInfo->DmxId < DMX_CNT) {
	PcrInfo->DmxId = DMX_INVALID_DEMUX_ID;

	DmxHalSetPcrIntEnable(PcrId, MT_FALSE);

	ret = MT_SUCCESS;
    }

    return ret;
}


mt_s32 DMX_OsiPcrChannelSetPid(const mt_u32 PcrId, const mt_u32 PcrPid)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_PCR_Info_S *PcrInfo = &DmxMgr->DmxPcrInfo[PcrId];

    if ((PcrPid > DMX_INVALID_PID) || (PcrInfo->DmxId >= DMX_CNT)) {
	MT_ERR_DEMUX("Invalid parameter: PcrPid=0x%x, DmxId=%u\n", PcrPid, PcrInfo->DmxId);

	return MT_ERR_DMX_INVALID_PARA;
    }

    if (PcrPid < DMX_INVALID_PID) {
	mt_u32 i;

	for (i = 0; i < DMX_PCR_CHANNEL_CNT; i++) {
	    if ((DmxMgr->DmxPcrInfo[i].DmxId == PcrInfo->DmxId) && (i != PcrId)) {
		if (DmxMgr->DmxPcrInfo[i].PcrPid == PcrPid) {
		    PcrInfo->PcrPid = PcrPid;
		    MT_ERR_DEMUX("chanel(%x) pcr pid(%x) already used.\n", PcrId, PcrPid);
		    return MT_SUCCESS;
		}
	    }
	}
    }

    PcrInfo->PcrPid = PcrPid;
    DmxHalSetPcrPid(PcrId, PcrPid);
    //DmxHalSetPcrDmxId(PcrId, PcrInfo->DmxId);
    DmxHalSetPcrIntEnable(PcrId, MT_TRUE);

    return MT_SUCCESS;
}


mt_s32 DMX_OsiPcrChannelGetPid(const mt_u32 PcrId, mt_u32 *PcrPid)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_PCR_Info_S *PcrInfo = &g_pDmxDevOsi->DmxPcrInfo[PcrId];

    if (PcrInfo->DmxId < DMX_CNT) {
	*PcrPid = PcrInfo->PcrPid;

	ret = MT_SUCCESS;
    }

    return ret;
}

mt_s32 DMX_OsiPcrChannelGetClock(const mt_u32 PcrId, mt_u64 *PcrValue, mt_u64 *ScrValue)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_PCR_Info_S *PcrInfo = &DmxDevOsi->DmxPcrInfo[PcrId];

    if (PcrInfo->DmxId < DMX_CNT) {
	//*PcrValue = PcrInfo->PcrValue;
	//*ScrValue = PcrInfo->ScrValue;
	*PcrValue = 0;
	*ScrValue = 0;
	ret = MT_SUCCESS;
    }

    return ret;
}

mt_s32 DMX_OsiPcrChannelAttachSync(const mt_u32 PcrId, const mt_u32 SyncHadle)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_PCR_Info_S *PcrInfo = &g_pDmxDevOsi->DmxPcrInfo[PcrId];

    if (PcrInfo->DmxId < DMX_CNT) {
	PcrInfo->SyncHandle = SyncHadle;

	ret = MT_SUCCESS;
    }

    return ret;
}

mt_s32 DMX_OsiPcrChannelDetachSync(const mt_u32 PcrId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_PCR_Info_S *PcrInfo = &g_pDmxDevOsi->DmxPcrInfo[PcrId];

    if (PcrInfo->DmxId < DMX_CNT) {
	PcrInfo->SyncHandle = DMX_INVALID_SYNC_HANDLE;

	ret = MT_SUCCESS;
    }

    return ret;
}

static RET_CODE dmx_trpp_set_rec_mode(s_trpp_rec_info trpp_rec_info)
{
  reg_trpp_ch_rec_set_t reg = { 0 };
  MT_DBG_DEMUX("\r\n   dmx_symphony_trpp_set_rec_mode ==> ch=%x\n",trpp_rec_info.rec_chnum);
  if (trpp_rec_info.rec_chnum < DMX_REC_CNT)
  {
    //reg_set_trpp_ch_rec_start_addr(trpp_rec_info.rec_chnum,  trpp_rec_info.data_buf_staddr);
    //reg_set_trpp_ch_rec_end_addr(trpp_rec_info.rec_chnum, trpp_rec_info.data_buf_endaddr);
    u32 int_ts_count = (DMX_REC_CIRCLE_RANGE_UNIT / (188 *16)) -1;
    u32 rec_cut_per_interrupt = 0;
    reg_set_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(trpp_rec_info.rec_chnum, 0);

    reg.bitc.trpp_ch_rec_mod = trpp_rec_info.rec_mode;
    reg.bitc.trpp_ch_rec_sel = trpp_rec_info.rec_sel;
    //reg.bitc.trpp_ch_rec_cnt_th = trpp_rec_info.rec_cnt_th;
    if(int_ts_count >=  63)    rec_cut_per_interrupt = 63;
    else if(int_ts_count >= 31)    rec_cut_per_interrupt = 31;
    else if(int_ts_count >= 15)    rec_cut_per_interrupt = 15;
    else if(int_ts_count >= 7)    rec_cut_per_interrupt = 7;
    else if(int_ts_count >= 3)    rec_cut_per_interrupt = 3;
    else  rec_cut_per_interrupt = 1;

    reg.bitc.trpp_ch_rec_cnt_th = rec_cut_per_interrupt;//TST_PTI_REC_INT_TS_COUNT;
    reg_set_trpp_ch_rec_set(trpp_rec_info.rec_chnum, reg.all);

    //data = reg_get_trpp_channel_record_en();
    //reg_set_trpp_channel_record_en(data | (1 << trpp_rec_info.rec_chnum));
    /*init reg_set_trpp_ch11_ini_info1-4*/
    reg_set_trpp_ch11_ini_info1(trpp_rec_info.rec_chnum, 0);
    reg_set_trpp_ch11_ini_info2(trpp_rec_info.rec_chnum, 0);
    // delete  for bug 71888
    //reg_set_trpp_ch11_ini_info3(trpp_rec_info.rec_chnum, 0);
    //reg_set_trpp_ch_ts_sn(trpp_rec_info.rec_chnum, 0);
  }
  else
  {
    return MT_FAILURE;
  }

  return MT_SUCCESS;
}

static RET_CODE dmx_rec_chan_set_buffer(void *p_dmx,
                                              u8 index,u8 *p_buf, u32 size)
{
  phys_addr_t real_addr = 0;

  if(p_buf == NULL)  return MT_FAILURE;
  #if 0
  if((u32)p_buf > 0xa0000000)    real_addr = (u8*)(p_buf - 0xa0000000);
  else  real_addr = p_buf;
  #else
  real_addr = (phys_addr_t)(ulong)p_buf;
  #endif

  if(index < DMX_REC_CNT)
  {
    if(((size % 4096) != 0) ||  (((u32)real_addr % 4096) != 0))
    {
        MT_DBG_DEMUX("memory addr should be 4k aligned ! size=0x%x, real_addr=0x%x\n",size,real_addr);
        MT_ASSERT(0);
        return MT_FAILURE;
    }
    MT_INFO_DEMUX("[%s %d]index = %d, p_buf=0x%x, size=0x%x\n", __FUNCTION__, __LINE__, 
          			index,real_addr,size);
    reg_set_trpp_ch_rec_start_addr_trpp_ch_rec_saddr(index, ((u32)real_addr >> 3));
    if(size >= 1*1024*1024)
    {
        reg_set_trpp_ch_rec_start_addr_trpp_ch_rec_mem_th(index, 7);
    }
    reg_set_trpp_ch_rec_end_addr_trpp_ch_rec_eaddr(index,(u32)real_addr  + size -1);
    if(0)
    {
        reg_set_dmx_bus_debug_chn_staddr_bus_debug_chn_staddr(2+index,  (u32)(real_addr) >> 3);
        reg_set_dmx_bus_debug_chn_endaddr_bus_debug_chn_endaddr(2+index, ((u32)(real_addr) + size) >> 3);
    }

    DMX_OsiClearRecBuf(index, 0);
  }
  else
  {
    MT_DBG_DEMUX("\r\ndmx_symphony_rec_chan_set_buffer failure.(index %d)\n ",index);
    return MT_FAILURE;
  }

  return MT_SUCCESS;
}

//all the idx channel use a shared start code filter											  
static RET_CODE dmx_trpp_start_code_filter_cfg(s_trpp_global_info trpp_global_info)
{
	u32 data = 0;
	
	reg_set_trpp_bus_urgent_trpp_urgent_mod(trpp_global_info.bus_urgent_mode);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	reg_set_trpp_bus_urgent_trpp_burst_len_mod(0);
#endif

	//start_code byte4 value
	data = ((trpp_global_info.sc_index_flt[0] << 24) | (trpp_global_info.sc_index_flt[1] << 16) |
	(trpp_global_info.sc_index_flt[2] << 8) | (trpp_global_info.sc_index_flt[3] << 0));
	reg_set_trpp_sc_index_flt1_4(data);

	data = ((trpp_global_info.sc_index_flt[4] << 24) | (trpp_global_info.sc_index_flt[5] << 16) |
	(trpp_global_info.sc_index_flt[6] << 8) | (trpp_global_info.sc_index_flt[7] << 0));
	reg_set_trpp_sc_index_flt5_8(data);

	data = ((trpp_global_info.sc_index_flt[8] << 24) | (trpp_global_info.sc_index_flt[9] << 16) |
	(trpp_global_info.sc_index_flt[10] << 8) | (trpp_global_info.sc_index_flt[11] << 0));
	reg_set_trpp_sc_index_flt9_10(data);

	data = ((trpp_global_info.sc_index_flt[12] << 24) | (trpp_global_info.sc_index_flt[13] << 16) |
	(trpp_global_info.sc_index_flt[14] << 8) | (trpp_global_info.sc_index_flt[15] << 0));
	reg_set_trpp_sc_index_flt11_12(data);
	//start_code byte3 value
	reg_set_trpp_sc_index_flt0(0x8001);
	return MT_SUCCESS;
}

//the idx channel use its own start code filter
static RET_CODE dmx_set_idx_chn_start_code_filter(mt_u8 index, s_trpp_global_info trpp_global_info)
{
	u32 data = 0;
	
	reg_set_trpp_bus_urgent_trpp_urgent_mod(trpp_global_info.bus_urgent_mode);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	reg_set_trpp_bus_urgent_trpp_burst_len_mod(0);
#endif

	//start_code byte4 value
	data = ((trpp_global_info.sc_index_flt[0] << 24) | (trpp_global_info.sc_index_flt[1] << 16) |
	(trpp_global_info.sc_index_flt[2] << 8) | (trpp_global_info.sc_index_flt[3] << 0));
	reg_set_trpp_ch15_sc_flt_set0(index, data);

	data = ((trpp_global_info.sc_index_flt[4] << 24) | (trpp_global_info.sc_index_flt[5] << 16) |
	(trpp_global_info.sc_index_flt[6] << 8) | (trpp_global_info.sc_index_flt[7] << 0));
	reg_set_trpp_ch15_sc_flt_set1(index, data);

	data = ((trpp_global_info.sc_index_flt[8] << 24) | (trpp_global_info.sc_index_flt[9] << 16) |
	(trpp_global_info.sc_index_flt[10] << 8) | (trpp_global_info.sc_index_flt[11] << 0));
	reg_set_trpp_ch15_sc_flt_set2(index, data);

	data = ((trpp_global_info.sc_index_flt[12] << 24) | (trpp_global_info.sc_index_flt[13] << 16) |
	(trpp_global_info.sc_index_flt[14] << 8) | (trpp_global_info.sc_index_flt[15] << 0));
	reg_set_trpp_ch15_sc_flt_set3(index, data);
	
	//start_code byte3 value
	reg_set_trpp_ch15_sc_flt_set4(index, 0x8001);
	return MT_SUCCESS;
}

static RET_CODE dmx_trpp_set_index_mode(u8 es_type, s_trpp_index_info trpp_index_info)
{
  u32 data = 0;
  u8 stream_id_mask=0;
  u8 stream_id_value=0;

  reg_trpp_ch_idx_mode_t reg = {0};
  reg_trpp_ch_idx_enable_t reg1 = {0};

  switch(es_type)
  {
      case DMX_VIDEO_TYPE:
          stream_id_mask = 0x1f;    //0x5f;    //0x0f;
          stream_id_value = 0xe0;   //0xa0;   //0xe0;
          break;
      case DMX_AUDIO_TYPE:
          stream_id_mask = 0x1f;    //0x1f;
          stream_id_value = 0xc0;   //0xc0;
          break;
      case DMX_VBI_TYPE:
          stream_id_mask = 0x00;
          stream_id_value = 0xbd;
          break;
      default:
          break;
  }

  if(trpp_index_info.index_chnum < DMX_REC_INDEX_CNT)
  {
    DMX_OsiClearIndexBuf(trpp_index_info.index_chnum, 0);
    reg.bitc.trpp_ch_pes_len_mod = trpp_index_info.pes_len_mode;
    reg.bitc.trpp_ch_sc_3byte_en = trpp_index_info.sc_3byte_en;
    reg.bitc.trpp_ch_stream_id = stream_id_value;
    reg.bitc.trpp_ch_str_id_msk = stream_id_mask;
    reg.bitc.trpp_ch_str_id_mod = trpp_index_info.stream_id_mode;
    //reg.all = reg.all | 0x10000000;
    MT_INFO_DEMUX("\nreg.all :(%x)\n",reg.all);
    reg_set_trpp_ch_idx_mode(trpp_index_info.index_chnum,reg.all);

    reg1.bitc.trpp_ch_flt_ens = trpp_index_info.filt_en;
    reg1.bitc.trpp_ch_pau_en = trpp_index_info.pause_en;
    reg1.bitc.trpp_ch_pusi_en = trpp_index_info.pusi_en;
    reg1.bitc.trpp_ch_hd_en = trpp_index_info.pes_head_en;
    reg1.bitc.trpp_ch_pts_en = trpp_index_info.pts_en;
    reg1.bitc.trpp_ch_sc_en = trpp_index_info.sc_en;
    reg1.bitc.trpp_ch_afld_en = trpp_index_info.afld_en;
    MT_INFO_DEMUX("\nreg1.all :(%x)\n",reg1.all);
    reg_set_trpp_ch_idx_enable(trpp_index_info.index_chnum,reg1.all);

    data = reg_get_trpp_channel_record_en();
    reg_set_trpp_channel_record_en(data |(1 << (8 + trpp_index_info.index_chnum)));

    reg_set_trpp_ch15_ini_info1(trpp_index_info.index_chnum, 0x00008000);
    reg_set_trpp_ch15_ini_info2(trpp_index_info.index_chnum, 0x0);
    reg_set_trpp_ch15_ini_info3(trpp_index_info.index_chnum, 0x0);
    reg_set_trpp_ch15_ini_info4(trpp_index_info.index_chnum, 0x000000ff);
    //add for bug 71888
    reg_set_trpp_ch15_ini_info5(trpp_index_info.index_chnum, 0x0);
    reg_set_trpp_ch15_ini_info6(trpp_index_info.index_chnum, 0x0);
    reg_set_trpp_ch15_ini_info7(trpp_index_info.index_chnum, 0x0);
  }
  else
  {
    return MT_FAILURE;
  }

  return MT_SUCCESS;
}

static RET_CODE dmx_rec_index_set_buffer(void *p_dmx,
                                                            u8 index,
                                                            u8 *p_buf,
                                                            u32 size)
{
  phys_addr_t real_addr = 0;  //jace need check

  if(p_buf == NULL)  return MT_FAILURE;
  #if 0
  if((u32)p_buf > 0xa0000000)    real_addr = (u8*)(p_buf - 0xa0000000);
  else  real_addr = p_buf;
  #else
  real_addr = (phys_addr_t)((ulong)p_buf);
  #endif

  if(index < DMX_REC_INDEX_CNT)
  {
    if(((size % 4096) != 0) ||  (((u32)real_addr % 4096) != 0))
    {
        MT_DBG_DEMUX("memory addr should be 4k aligned !\n");
        MT_ASSERT(0);
        return MT_FAILURE;
    }
    reg_set_trpp_ch_idx_start_addr_trpp_ch_idx_saddr(index,  ((mt_u32)real_addr >> 3));
    reg_set_trpp_ch_idx_end_addr_trpp_ch_idx_eaddr(index, (mt_u32)(real_addr + size -1));

    reg_set_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(index, 0);
  }
  else
  {
    MT_DBG_DEMUX("\r\ndmx_symphony_rec_index_chan_set_buffer failure.(index %d)\n ",index);
    return MT_FAILURE;
  }

  return MT_SUCCESS;
}

mt_s32 DMX_DRV_REC_CreateChannel(MT_UNF_DMX_REC_ATTR_S *RecAttr, DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp, mt_u32 *RecId, phys_addr_t *BufPhyAddr, mt_u32 *BufSize, phys_addr_t *IdxBufPhyAddr, mt_u32 *IdxBufSize)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = MT_NULL;
    mt_u32 RecFqBlockSize = DMX_REC_VID_TS_PACKETS_PER_BLOCK;
    mmz_buffer_s RecMmzBuf = { 0 };
    mmz_buffer_s RecIdxMmzBuf = { 0 };
    mt_u32 RecBufSize = 0;
    s_trpp_index_info trpp_index_info;
    s_trpp_rec_info trpp_rec_info;
    s_trpp_global_info global_info;
    unsigned int i;
    char default_index_data[] = {0x0d,0xb3,0x0f,0xb0,0x41,0x00,0xb6,0x40,0x13,0x00,0x2f,0x20,0x47,0x41,0x67,0x61};
                                //bit3  2    1   0     7    6    5    4     10        9        12          11
    MT_BOOL  index_enable = 0;
    mt_u32 rec_align_size = 0;
    mt_u32 rec_round_size = 0;
    mt_u32 page_block = (PAGE_SIZE << 4); 
	mt_u8 index_start, index_end;
	mt_u32 reg_value;

    CHECKDMXID(RecAttr->u32DmxId);
	
    if (DMX_CNT <= RecAttr->u32DmxId)
    {
        MT_ERR_DEMUX("u32DmxId(%d) >= DMX_CNT\n", RecAttr->u32DmxId);
        return MT_ERR_DMX_INVALID_PARA;
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	if (RecAttr->multi_chan_flag)
	{
		reg_value = reg_get_demux_multi_rec_en();
		reg_value |= 0x01;//bit0: 1, enable a pid record to multi channel
		reg_set_demux_multi_rec_en(reg_value);

		reg_value = reg_get_trpp_mode();
		reg_value |= (1<<31);//bit31:1, enable a pid record to multi channel
		reg_set_trpp_mode(reg_value);
		
		if (RecAttr->first_chan_flag)//in multi-channel record function, only a channel is in 0~3
		{
			index_start = 0;
			index_end   = 4;
		}
		else
		{
			index_start = 4;
			index_end   = DMX_REC_CNT;
		}
	}
	else//only channel 0-3 is allowed when disable the multi channel record function
	{
		reg_value = reg_get_demux_multi_rec_en();
		reg_value &= (~(1<<0));//bit0 clear to 0, disable a pid record to multi channel
		reg_set_demux_multi_rec_en(reg_value);
		
		index_start = 0;
		index_end   = 4;
	}
#else
	index_start = 0;
	index_end   = DMX_REC_CNT;
#endif

	if (0 == down_interruptible(&DmxMgr->lock_RecChan)) 
    {
	    for (i=index_start; i<index_end; i++)
		{
			if (DmxMgr->DmxRecInfo[i].RecId == DMX_INVALID_CHAN_ID)
	        {
	        	RecInfo = &DmxMgr->DmxRecInfo[i];
	            RecInfo->RecId = i;
			    *RecId = i;			
	            break;
	        }
		}
    }
    else
    {
    	up(&DmxMgr->lock_RecChan);
        MT_ERR_DEMUX("[%s %d]down_interruptible failed!\n", __FUNCTION__, __LINE__);
        return MT_ERR_DMX_BUSY;
    }	

	up(&DmxMgr->lock_RecChan);
	if (i == DMX_REC_CNT)
	{
		MT_ERR_DEMUX("[%s %d]no available record channel\n", __FUNCTION__, __LINE__);
		return MT_ERR_DMX_NOFREE_CHAN;
	}
	RecInfo->multi_chan_flag = RecAttr->multi_chan_flag;
	
#ifndef DMX_REC_TIME_STAMP_SUPPORT
    //enRecTimeStamp = DMX_REC_TIMESTAMP_NONE;
#endif	

    memset(&trpp_rec_info, 0, sizeof(s_trpp_rec_info));
    memset(&trpp_index_info, 0, sizeof(s_trpp_index_info));
	
    *IdxBufPhyAddr = 0;
	
    if ((DMX_PARTIAL_TS_PACKET_DESCRAMBLE == RecAttr->type_mode) ||
        (DMX_PARTIAL_TS_PACKET_PES_DESCRAMBLE == RecAttr->type_mode))
    {
        trpp_rec_info.rec_sel = 0;
    }
    else
    {
        trpp_rec_info.rec_sel = 1;
    }

   if(RecAttr->bDescramed == MT_TRUE)    trpp_rec_info.rec_sel = 0;
	
    switch (RecAttr->enRecType)
    {
        case MT_UNF_DMX_REC_TYPE_SELECT_PID:
        {
            switch (RecAttr->enIndexType)
            {
                case MT_UNF_DMX_REC_INDEX_TYPE_VIDEO:
                    if (RecAttr->u32IndexSrcPid >= DMX_INVALID_PID)
                    {
                        MT_ERR_DEMUX("invalid index pid:0x%x\n", RecAttr->u32IndexSrcPid);
                        return MT_ERR_DMX_INVALID_PARA;
                    }
        			
                    MT_DBG_DEMUX("%s : %d\n", __FILE__, __LINE__);
                    trpp_index_info.pts_en = 1;
                    trpp_index_info.sc_en = 1;                	  
                        
                    if (enRecTimeStamp >= DMX_REC_TIMESTAMP_ZERO)
                    {
                    	trpp_rec_info.rec_mode = 1; //1:192 bytes, insert time information before ts's head, 0:188 bytes
                        RecFqBlockSize = DMX_REC_VID_TS_WITH_TIMESTAMP_BLOCK_SIZE;
                        MT_DBG_DEMUX("%s : %d\n", __FILE__, __LINE__);
                    }
                	break;
    
                case MT_UNF_DMX_REC_INDEX_TYPE_AUDIO:
                    if (RecAttr->u32IndexSrcPid >= DMX_INVALID_PID)
                    {
                        MT_ERR_DEMUX("invalid index pid:0x%x\n", RecAttr->u32IndexSrcPid);
                        return MT_ERR_DMX_INVALID_PARA;
                    }
    				
                    if (enRecTimeStamp >= DMX_REC_TIMESTAMP_ZERO)
                    {
                    	trpp_rec_info.rec_mode = 1; //1:192 bytes, insert time information before ts's head, 0:188 bytes
                        RecFqBlockSize = DMX_REC_AUD_TS_WITH_TIMESTAMP_BLOCK_SIZE;
                    }
                    else
                    {
                        RecFqBlockSize = DMX_REC_AUD_TS_PACKETS_PER_BLOCK;
                    }
                	break;
                
                case MT_UNF_DMX_REC_INDEX_TYPE_NONE:
                    MT_ERR_DEMUX("invalid enIndexType = %d!!!\n", RecAttr->enIndexType);
                	break;
                
                default:
                    MT_ERR_DEMUX("enIndexType = %d\n", RecAttr->enIndexType);
                	return MT_ERR_DMX_INVALID_PARA;
            }
			break;
        }
            
        case MT_UNF_DMX_REC_TYPE_ALL_PID: 
			MT_INFO_DEMUX("[%s %d]MT_UNF_DMX_REC_TYPE_ALL_PID, enRecTimeStamp=%d\n", __FUNCTION__, __LINE__, enRecTimeStamp);
			if (enRecTimeStamp >= DMX_REC_TIMESTAMP_ZERO)
			{
				trpp_rec_info.rec_mode = 1; //1:192 bytes, insert time information before ts's head, 0:188 bytes				
				MT_INFO_DEMUX("[%s %d]trpp_rec_info.rec_mode=%d\n", __FUNCTION__, __LINE__, trpp_rec_info.rec_mode);
			}
        	break;
    
        default:
            MT_ERR_DEMUX("invalid RecAttr->enRecType:%d\n", RecAttr->enRecType);
        	return MT_ERR_DMX_INVALID_PARA;
    }

    if (RecAttr->u32RecBufSize < DMX_REC_MIN_BUF_SIZE)
    {
        MT_ERR_DEMUX("invalid RecAttr->u32RecBufSize:0x%x\n", RecAttr->u32RecBufSize);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (0 == down_interruptible(&RecInfo->LockRec)) 
    {
	    RecInfo->DmxId = RecAttr->u32DmxId;
		
        /*
        patch for bug127944, RecInfo->RecOqId must be inited by RecInfo->DmxId, if not,
        if mutil thread work for record(Record and Timeshift), DMX_DRV_REC_AcquireRecData
        may return wrong data addr to userspace.
        */
        RecInfo->RecOqId = RecInfo->RecId;
        
    	up(&RecInfo->LockRec);
    }
    else
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        return MT_ERR_DMX_BUSY;
    }

    trpp_rec_info.rec_chnum = RecInfo->RecId;   //for dual rec,set recid

    for (i = 0; i < DMX_CHANNEL_NUM_PER_RECORD; i++)
    {
        RecInfo->ChannelId[i] = DMX_INVALID_CHAN_ID;
    }

    RecInfo->RecBuffer.startPhyAddr = 0;
    RecInfo->RecBuffer.startVirAddr = 0;
    RecInfo->RecIdxBuffer.startPhyAddr = 0;
    RecInfo->RecIdxBuffer.startVirAddr = 0;
    RecInfo->RecType = RecAttr->enRecType;
    RecInfo->IndexPid = RecAttr->u32IndexSrcPid;
    RecInfo->VCodecType = RecAttr->enVCodecType;
    RecInfo->rec_data_overflow_cnt = 0;
    RecInfo->rec_index_overflow_cnt = 0;
	RecInfo->sync_with_index = RecAttr->sync_with_index;
	
    if (DMX_PORT_MODE_RAM == DmxMgr->SubDevInfo[RecInfo->DmxId].PortMode)
    {
		RecInfo->slot_reg0.bitc.src = 4 + DmxMgr->SubDevInfo[RecInfo->DmxId].PortId;

    	MT_INFO_DEMUX("RAM:DmxMgr->SubDevInfo[%x].PortId=0x%x\n", RecInfo->DmxId, DmxMgr->SubDevInfo[RecInfo->DmxId].PortId);
    }
    else
    {
        RecInfo->slot_reg0.bitc.src = DmxMgr->SubDevInfo[RecInfo->DmxId].PortId;
        MT_INFO_DEMUX("TUNER:DmxMgr->SubDevInfo[%x].PortId=0x%x\n", RecInfo->DmxId, DmxMgr->SubDevInfo[RecInfo->DmxId].PortId);
    }

    MT_INFO_DEMUX("[%s %d]recid=%d, dmxid = %d, indextype = %d, rectype = %d\n", __FILE__, __LINE__, RecInfo->RecId, RecInfo->DmxId, RecInfo->IndexType, RecInfo->RecType);

    RecBufSize = RecAttr->u32RecBufSize;

    if ((RecBufSize & 0xfff) || (RecBufSize%188))  //if RecBufSize is not the integer times of 4k and 188, make it to be.
    {
        mt_u32 tmp = 188 << 10;
        if (RecBufSize <= tmp)
        {
            RecBufSize = tmp;
        }
        else
        {
            RecBufSize = ((RecBufSize/tmp)+1)*tmp;
        }
    }
	
    dmx_trpp_set_rec_mode(trpp_rec_info);
    RecInfo->rec_init_thrd = reg_get_trpp_ch_rec_set_trpp_ch_rec_cnt_th(RecInfo->DmxId);
	
    /*set bus_urgent_mode*/
    global_info.bus_urgent_mode = 0x2;
	
    if (MT_UNF_DMX_REC_TYPE_SELECT_PID == RecAttr->enRecType)
    {
        if (RecAttr->filt_en != 0)
        {
            memcpy(default_index_data, RecAttr->start_code_filter, 16);
            trpp_index_info.filt_en = RecAttr->filt_en;
            MT_INFO_DEMUX("\ntrpp_index_info.filt_en flt=(%x)\n", trpp_index_info.filt_en);
        }
        else
        {
            /* for default table*/
            switch (RecAttr->enVCodecType)
            {
                case MT_UNF_VCODEC_TYPE_MPEG2:
                    trpp_index_info.filt_en = 0x44;  //0x00\0xb3 for header
                	break;
                case MT_UNF_VCODEC_TYPE_MPEG4:
                    trpp_index_info.filt_en = 0x21;  //0xb6\0xb0 for header
                	break;
                case MT_UNF_VCODEC_TYPE_AVS:
                    trpp_index_info.filt_en = 0x25;  //0xb6 0xb3 \0xb0 for sequence header
                	break;
                case MT_UNF_VCODEC_TYPE_H264:
                    trpp_index_info.filt_en=0xf00;  //0xff;
                	break;
                case MT_UNF_VCODEC_TYPE_VC1:
                    trpp_index_info.filt_en = 0x0a;  //0x0d\0x0f for header
                	break;
                case MT_UNF_VCODEC_TYPE_HEVC:
                    trpp_index_info.filt_en = 0x390;  //0x40\0x41 active sps
                	break;
                default:
                	break;
            }
            MT_INFO_DEMUX("\n trpp_index_info.filt_en flt=(%x)\n",__func__,__LINE__,trpp_index_info.filt_en);
        }
		
        memcpy(global_info.sc_index_flt, default_index_data, 16);
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
      if(RecAttr->AlignToPageBlock){
          page_block = ((1 << pageblock_order) * PAGE_SIZE);
          rec_round_size = ((RecBufSize+(page_block-1))&(~(page_block-1))); //aligned to page_block
          rec_align_size = page_block;
      }else{
          rec_round_size=RecBufSize;
          rec_align_size=page_block;
      }
      
    #if defined(CONFIG_MT_CHIP_SYMPHONY6)
    //sym6 as solution size and addr must 256K align
    if(g_pDmxDevOsi->chip_fused_fta == MT_FALSE)
    {
        rec_round_size = DMX_ROUNDUP(rec_round_size,0x40000);
        rec_align_size = DMX_ROUNDUP(page_block,0x40000);
    }
    #endif
#else
      rec_round_size=RecBufSize;
      rec_align_size=page_block;
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
    for(i = 0; i < DMX_REC_CNT; i++)
    {
        char *mmz_pvr[DMX_REC_CNT] = {MMZ_ZONE_PVR0,MMZ_ZONE_PVR1,MMZ_ZONE_PVR2,MMZ_ZONE_PVR3};
        if(DmxMgr->bPvrRecBuffZoneIsUsed[i] == MT_FALSE)
        {
            if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_REC", mmz_pvr[i], rec_round_size, rec_align_size, &RecMmzBuf))
            {
                MT_FATAL_DEMUX("ts packet memory allocate failed for pvr%d\n",i);
                /*
                patch for bug126772, if request mem failed, release current record dmx id
                */
                RecInfo->DmxId = DMX_INVALID_DEMUX_ID;
				RecInfo->RecId = DMX_INVALID_CHAN_ID;
                return MT_ERR_DMX_ALLOC_MEM_FAILED;
            }
            else
            {
                DmxMgr->bPvrRecBuffZoneIsUsed[i] = MT_TRUE;
                RecInfo->pvrRecBuffZoneId = i;
                break;
            }
        }
    }
    if(i == DMX_REC_CNT)
    {
        MT_FATAL_DEMUX("memory allocate zone error !!\n");
        return MT_ERR_DMX_ALLOC_MEM_FAILED;
    }
#else
    if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_REC", MMZ_OTHERS, rec_round_size, rec_align_size, &RecMmzBuf))
    {
        MT_FATAL_DEMUX("ts packet memory allocate failed\n");
        /*
        patch for bug126772, if request mem failed, release current record dmx id
        */
        RecInfo->DmxId = DMX_INVALID_DEMUX_ID;
		RecInfo->RecId = DMX_INVALID_CHAN_ID;
        return MT_ERR_DMX_ALLOC_MEM_FAILED;
    }
#endif

    MT_INFO_DEMUX("\n RecBufSize(%x) RecMmzBuf.size(%x)\n",RecBufSize,RecMmzBuf.size);

    memset((mt_u8 *)RecMmzBuf.startVirAddr, 0, RecMmzBuf.size);
    // *RecId = RecAttr->u32DmxId;
    *BufPhyAddr = RecMmzBuf.startPhyAddr;
    *BufSize = RecMmzBuf.size;
    RecInfo->RecBuffer.startPhyAddr = RecMmzBuf.startPhyAddr;
    RecInfo->RecBuffer.startVirAddr = RecMmzBuf.startVirAddr;
	RecInfo->RecBuffer.size = RecMmzBuf.size;
    RecInfo->rp_globe=0;
    RecInfo->overflow_cnt=0;
    RecInfo->recbuf_size=RecBufSize;
    RecInfo->recbuf_align_size=rec_round_size;
    memset(&RecInfo->overflowinfo ,0x00 ,sizeof(DMX_RecInfo_Overflow_S));

    //printk(KERN_ERR "[%s %d] ------RecBufSize=0x%x rec_round_size=0x%x rec_align_size=0x%x\n",__FUNCTION__, __LINE__,RecBufSize,rec_round_size,rec_align_size);

    MT_INFO_DEMUX("[%s %d]RecInfo->RecId=%d, RecInfo->DmxId=%d, RecMmzBuf.size=0x%lx, RecMmzBuf.u32BufPhyAddr=0x%llx, RecMmzBuf.startVirAddr=0x%lx\n",
                 __FUNCTION__, __LINE__, RecInfo->RecId, RecInfo->DmxId, RecMmzBuf.size, RecMmzBuf.startPhyAddr, (ulong)RecMmzBuf.startVirAddr);
    /*set recorded ts packet len,added by l00188263 ,Hi3719 support 192 Byte ts packet length recording*/
    dmx_rec_chan_set_buffer(NULL, RecInfo->RecId, (mt_u8 *)((ulong)RecMmzBuf.startPhyAddr), (u32)RecBufSize);    

	if (RecInfo->RecId <= 3)
	{
		trpp_index_info.index_chnum = RecInfo->RecId;
	}
	else//record channel 4-6 use 8-11 for index channel
	{
		trpp_index_info.index_chnum = RecInfo->RecId + 4;		
	}
	
	MT_INFO_DEMUX("[%s %d]trpp_index_info.index_chnum=%d\n", __FUNCTION__, __LINE__, trpp_index_info.index_chnum);
	if (RecAttr->idx_filter_mode)
	{
		reg_value = reg_get_trpp_mode();
		reg_value &= (~(1<<28));//bit28:0, index use unshare mode
		reg_set_trpp_mode(reg_value);
		dmx_set_idx_chn_start_code_filter(trpp_index_info.index_chnum, global_info);
	}
	else
	{
		reg_value = reg_get_trpp_mode();
		reg_value |= (1<<28);//bit28:1, index use share mode
		reg_set_trpp_mode(reg_value);
    	dmx_trpp_start_code_filter_cfg(global_info);
	}
	
    if (((1 == trpp_index_info.afld_en) ||
        (1 == trpp_index_info.pause_en) ||
        (1 == trpp_index_info.pes_head_en) ||
        (1 == trpp_index_info.pts_en) ||
        (1 == trpp_index_info.pusi_en) ||
        (1 == trpp_index_info.sc_en)) &&
        (0 != trpp_index_info.filt_en))
    {
        if ((trpp_index_info.index_chnum < DMX_REC_INDEX_CNT) && (trpp_index_info.index_chnum >= 0))
        {
            //set trpp_index_start
            trpp_index_info.data_buf_staddr = 0;
            trpp_index_info.data_buf_endaddr = 0;
            index_enable = 1;
            dmx_trpp_set_index_mode(DMX_VIDEO_TYPE, trpp_index_info);
        }
    }

    if (index_enable)
    {
        RecInfo->IndexType = RecAttr->enIndexType;
        if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_RecIdxBuf", MMZ_OTHERS, TRPP_REC_INDEX_BUFFER_SIZE, MIN_MMZ_BUF_SIZE, &RecIdxMmzBuf))
        {
            MT_FATAL_DEMUX("index data memory allocate failed\n");
            /*
            patch for bug126772, if request mem failed, release current record dmx id
            */
            RecInfo->DmxId = DMX_INVALID_DEMUX_ID;
			RecInfo->RecId = DMX_INVALID_CHAN_ID;
            return MT_ERR_DMX_ALLOC_MEM_FAILED;
        }
		
        memset((mt_u8 *)RecIdxMmzBuf.startVirAddr, 0, RecIdxMmzBuf.size);
        *IdxBufPhyAddr = RecIdxMmzBuf.startPhyAddr;
        *IdxBufSize = RecIdxMmzBuf.size;
        RecInfo->RecIdxBuffer.startPhyAddr = RecIdxMmzBuf.startPhyAddr;
        RecInfo->RecIdxBuffer.startVirAddr = RecIdxMmzBuf.startVirAddr;
        MT_DBG_DEMUX("RecInfo->RecId=%d, RecInfo->DmxId=%d : RecIdxMmzBuf.size = %x,RecIdxMmzBuf.u32BufPhyAddr=%x,RecIdxMmzBuf.startVirAddr=%x\n",
                     RecInfo->RecId, RecInfo->DmxId, RecIdxMmzBuf.size, RecIdxMmzBuf.startPhyAddr, RecIdxMmzBuf.startVirAddr);
        /*set recorded ts packet len,added by l00188263 ,Hi3719 support 192 Byte ts packet length recording*/
        if (1 == index_enable)
        {
            RecInfo->slot_reg1.bitc.sc_fetch_en = 1;
			RecInfo->slot_reg0.bitc.sc_fetch_ch = (trpp_index_info.index_chnum & 0x08)>>3;
            RecInfo->slot_reg1.bitc.sc_fetch_ch = (trpp_index_info.index_chnum & 0x07);
        }
        else
        {
            RecInfo->slot_reg1.bitc.sc_fetch_en = 0;
            RecInfo->slot_reg1.bitc.sc_fetch_ch = 0;
        }

        dmx_rec_index_set_buffer(NULL, trpp_index_info.index_chnum, (mt_u8 *)((ulong)RecIdxMmzBuf.startPhyAddr), (u32)RecIdxMmzBuf.size);
    }
    else
    {
        RecInfo->IndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
    }
	
    if (MT_UNF_DMX_REC_TYPE_ALL_PID == RecAttr->enRecType)
    {
        if (DMX_FULL_TS_WITHOUT_NULL_PACKET == RecAttr->type_mode)
        {
            mt_u32 rec_channl = 0;
            RecInfo->slot_reg0.bitc.pid = 0x1fff;
            RecInfo->slot_reg0.bitc.pid_filter_en = 1;
            RecInfo->slot_reg0.bitc.pid_filter_mode = 1;
            DMX_DRV_REC_AddRecPid(RecInfo->RecId, 0x1fff, &rec_channl);
        }
		else
		{
            mt_u32 rec_channl = 0;
            RecInfo->slot_reg0.bitc.pid_filter_en = 0;  //rec all ts
            RecInfo->slot_reg0.bitc.pid_filter_mode = 0;
            DMX_DRV_REC_AddRecPid(RecInfo->RecId, 0x1fff, &rec_channl);
        }
    }

    return MT_SUCCESS;
}

mt_s32 DMX_DRV_LinkREC_CreateChannel(MT_UNF_DMX_REC_ATTR_S *RecAttr, DMX_REC_TIMESTAMP_MODE_E enRecTimeStamp, mt_u32 *RecId, DMX_LinkRec_CreateChan_S *link_rec_param)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = MT_NULL;
    mt_u32 RecFqBlockSize = DMX_REC_VID_TS_PACKETS_PER_BLOCK;
    mmz_buffer_s RecMmzBuf = { 0 };
    mmz_buffer_s RecIdxMmzBuf = { 0 };
    mt_u32 RecBufSize = 0;
    s_trpp_index_info trpp_index_info;
    s_trpp_rec_info trpp_rec_info;
    s_trpp_global_info global_info;
    unsigned int i;
    char default_index_data[] = {0x0d,0xb3,0x0f,0xb0,0x41,0x00,0xb6,0x40,0x13,0x00,0x2f,0x20,0x47,0x41,0x67,0x61};
                                //bit3  2    1   0     7    6    5    4     10        9        12          11
    MT_BOOL  index_enable = 0;
    mt_u32 rec_align_size = 0;
    mt_u32 rec_round_size = 0;
    mt_u32 page_block = (PAGE_SIZE << 4); 

	unsigned int j;
	mt_u8 index_start, index_end;
	mt_u32 reg_value;

    CHECKDMXID(RecAttr->u32DmxId);
	
    if (DMX_CNT <= RecAttr->u32DmxId)
    {
        MT_ERR_DEMUX("u32DmxId(%d) >= DMX_CNT\n", RecAttr->u32DmxId);
        return MT_ERR_DMX_INVALID_PARA;
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	if (RecAttr->multi_chan_flag)
	{
		reg_value = reg_get_demux_multi_rec_en();
		reg_value |= 0x01;//bit0 set to 1, enable a pid record to multi channel
		reg_set_demux_multi_rec_en(reg_value);

		reg_value = reg_get_trpp_mode();
		reg_value |= (1<<31);//bit31:1, enable a pid record to multi channel
		reg_set_trpp_mode(reg_value);
		
		if (RecAttr->first_chan_flag)//in multi-channel record function, only a channel is in 0~3
		{
			index_start = 0;
			index_end	= 4;
		}
		else
		{
			index_start = 4;
			index_end	= DMX_REC_CNT;
		}
	}
	else//only channel 0-3 is allowed when the multi channel record function don't be enabled
	{
		reg_value = reg_get_demux_multi_rec_en();
		reg_value &= (~(1<<0));//bit0 clear to 0, disable a pid record to multi channel
		reg_set_demux_multi_rec_en(reg_value);

		index_start = 0;
		index_end	= 4;
	}
#else
	index_start = 0;
	index_end	= DMX_REC_CNT;
#endif

	if (0 == down_interruptible(&DmxMgr->lock_RecChan)) 
    {
	    for (i=index_start; i<index_end; i++)
		{
			if (DmxMgr->DmxRecInfo[i].RecId == DMX_INVALID_CHAN_ID)
	        {
	        	RecInfo = &DmxMgr->DmxRecInfo[i];
	            RecInfo->RecId = i;
			    *RecId = i;			
	            break;
	        }
		}
    }
    else
    {
    	up(&DmxMgr->lock_RecChan);
        MT_ERR_DEMUX("[%s %d]down_interruptible failed!\n", __FUNCTION__, __LINE__);
        return MT_ERR_DMX_BUSY;
    }	

	up(&DmxMgr->lock_RecChan);
	if (i == DMX_REC_CNT)
	{
		MT_ERR_DEMUX("[%s %d]no available record channel\n", __FUNCTION__, __LINE__);
		return MT_ERR_DMX_NOFREE_CHAN;
	}
	printk(KERN_ERR "[%s %d]RecInfo->RecId=%d\n", __FUNCTION__, __LINE__, RecInfo->RecId);	
	RecInfo->multi_chan_flag = RecAttr->multi_chan_flag;
	RecInfo->link_mode = 1;
	
#ifndef DMX_REC_TIME_STAMP_SUPPORT
    //enRecTimeStamp = DMX_REC_TIMESTAMP_NONE;
#endif	

    memset(&trpp_rec_info, 0, sizeof(s_trpp_rec_info));
    memset(&trpp_index_info, 0, sizeof(s_trpp_index_info));
	
    if ((DMX_PARTIAL_TS_PACKET_DESCRAMBLE == RecAttr->type_mode) ||
        (DMX_PARTIAL_TS_PACKET_PES_DESCRAMBLE == RecAttr->type_mode))
    {
        trpp_rec_info.rec_sel = 0;
    }
    else
    {
        trpp_rec_info.rec_sel = 1;
    }

   	if(RecAttr->bDescramed == MT_TRUE)    trpp_rec_info.rec_sel = 0;

   	printk(KERN_ERR "[%s %d]trpp_rec_info.rec_sel=%d\n", __FUNCTION__, __LINE__, trpp_rec_info.rec_sel);
    switch (RecAttr->enRecType)
    {
        case MT_UNF_DMX_REC_TYPE_SELECT_PID:
        {
        	printk(KERN_ERR "[%s %d]MT_UNF_DMX_REC_TYPE_SELECT_PID, enRecTimeStamp=%d\n", __FUNCTION__, __LINE__, enRecTimeStamp);
            switch (RecAttr->enIndexType)
            {
                case MT_UNF_DMX_REC_INDEX_TYPE_VIDEO:
                    if (RecAttr->u32IndexSrcPid >= DMX_INVALID_PID)
                    {
                        MT_ERR_DEMUX("invalid index pid:0x%x\n", RecAttr->u32IndexSrcPid);
                        return MT_ERR_DMX_INVALID_PARA;
                    }
        			
                    MT_DBG_DEMUX("%s : %d\n", __FILE__, __LINE__);
                    trpp_index_info.pts_en = 1;
                    trpp_index_info.sc_en = 1;                	  
                        
                    if (enRecTimeStamp >= DMX_REC_TIMESTAMP_ZERO)
                    {
                    	trpp_rec_info.rec_mode = 1; //1:192 bytes, insert time information before ts's head, 0:188 bytes
                        RecFqBlockSize = DMX_REC_VID_TS_WITH_TIMESTAMP_BLOCK_SIZE;
                        MT_DBG_DEMUX("%s : %d\n", __FILE__, __LINE__);
						printk(KERN_ERR "[%s %d]trpp_rec_info.rec_mode=%d\n", __FUNCTION__, __LINE__, trpp_rec_info.rec_mode);
                    }
                	break;
    
                case MT_UNF_DMX_REC_INDEX_TYPE_AUDIO:
                    if (RecAttr->u32IndexSrcPid >= DMX_INVALID_PID)
                    {
                        MT_ERR_DEMUX("invalid index pid:0x%x\n", RecAttr->u32IndexSrcPid);
                        return MT_ERR_DMX_INVALID_PARA;
                    }
    				
                    if (enRecTimeStamp >= DMX_REC_TIMESTAMP_ZERO)
                    {
                    	trpp_rec_info.rec_mode = 1; //1:192 bytes, insert time information before ts's head, 0:188 bytes
                        RecFqBlockSize = DMX_REC_AUD_TS_WITH_TIMESTAMP_BLOCK_SIZE;
                    }
                    else
                    {
                        RecFqBlockSize = DMX_REC_AUD_TS_PACKETS_PER_BLOCK;
                    }
                	break;
                
                case MT_UNF_DMX_REC_INDEX_TYPE_NONE:
                    MT_ERR_DEMUX("invalid enIndexType = %d!!!\n", RecAttr->enIndexType);
                	break;
                
                default:
                    MT_ERR_DEMUX("enIndexType = %d\n", RecAttr->enIndexType);
                	return MT_ERR_DMX_INVALID_PARA;
            }
			break;
        }
            
        case MT_UNF_DMX_REC_TYPE_ALL_PID: 
			printk(KERN_ERR "[%s %d]MT_UNF_DMX_REC_TYPE_ALL_PID, enRecTimeStamp=%d\n", __FUNCTION__, __LINE__, enRecTimeStamp);
			if (enRecTimeStamp >= DMX_REC_TIMESTAMP_ZERO)
			{
				trpp_rec_info.rec_mode = 1; //1:192 bytes, insert time information before ts's head, 0:188 bytes				
				printk(KERN_ERR "[%s %d]trpp_rec_info.rec_mode=%d\n", __FUNCTION__, __LINE__, trpp_rec_info.rec_mode);
			}
        	break;
    
        default:
            MT_ERR_DEMUX("invalid RecAttr->enRecType:%d\n", RecAttr->enRecType);
        	return MT_ERR_DMX_INVALID_PARA;
    }

    if (RecAttr->u32RecBufSize < DMX_REC_MIN_BUF_SIZE)
    {
        MT_ERR_DEMUX("invalid RecAttr->u32RecBufSize:0x%x\n", RecAttr->u32RecBufSize);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (0 == down_interruptible(&RecInfo->LockRec)) 
    {
	    RecInfo->DmxId = RecAttr->u32DmxId;
		
        /*
        patch for bug127944, RecInfo->RecOqId must be inited by RecInfo->DmxId, if not,
        if mutil thread work for record(Record and Timeshift), DMX_DRV_REC_AcquireRecData
        may return wrong data addr to userspace.
        */
        RecInfo->RecOqId = RecInfo->RecId;
        
    	up(&RecInfo->LockRec);
    }
    else
    {
        MT_ERR_DEMUX("down_interruptible failed!\n");
        return MT_ERR_DMX_BUSY;
    }

    trpp_rec_info.rec_chnum = RecInfo->RecId;   //for dual rec,set recid

    for (i = 0; i < DMX_CHANNEL_NUM_PER_RECORD; i++)
    {
        RecInfo->ChannelId[i] = DMX_INVALID_CHAN_ID;
    }

	RecInfo->RecIdxBuffer.startPhyAddr = 0;
    RecInfo->RecIdxBuffer.startVirAddr = 0;
	link_rec_param->RecIdxBufPhyAddr = 0;
	link_rec_param->RecIdxBufSize = 0;	
	for (i=0; i<DMX_LINKREC_NODE_NUM; i++)
	{
	    RecInfo->link_rec_buf[i].startPhyAddr = 0;
	    RecInfo->link_rec_buf[i].startVirAddr = 0;	
		link_rec_param->RecBufPhyAddr[i] = 0;
		link_rec_param->RecBufSize[i] = 0;		
	}
	
    RecInfo->RecType = RecAttr->enRecType;
    RecInfo->IndexPid = RecAttr->u32IndexSrcPid;
    RecInfo->VCodecType = RecAttr->enVCodecType;

    if (DMX_PORT_MODE_RAM == DmxMgr->SubDevInfo[RecInfo->DmxId].PortMode)
    {
		RecInfo->slot_reg0.bitc.src = 4 + DmxMgr->SubDevInfo[RecInfo->DmxId].PortId;

    	MT_INFO_DEMUX("RAM:DmxMgr->SubDevInfo[%x].PortId=0x%x\n", RecInfo->DmxId, DmxMgr->SubDevInfo[RecInfo->DmxId].PortId);
    }
    else
    {
        RecInfo->slot_reg0.bitc.src = DmxMgr->SubDevInfo[RecInfo->DmxId].PortId;
        MT_INFO_DEMUX("TUNER:DmxMgr->SubDevInfo[%x].PortId=0x%x\n", RecInfo->DmxId, DmxMgr->SubDevInfo[RecInfo->DmxId].PortId);
    }

    MT_INFO_DEMUX("[%s %d]recid=%d, dmxid = %d, indextype = %d, rectype = %d\n", __FILE__, __LINE__, RecInfo->RecId, RecInfo->DmxId, RecInfo->IndexType, RecInfo->RecType);

    RecBufSize = RecAttr->u32RecBufSize;

    if ((RecBufSize & 0xfff) || (RecBufSize%188))  //if RecBufSize is not the integer times of 4k and 188, make it to be.
    {
        mt_u32 tmp = 188 << 10;
        if (RecBufSize <= tmp)
        {
            RecBufSize = tmp;
        }
        else
        {
            RecBufSize = ((RecBufSize/tmp)+1)*tmp;
        }
    }
	
    dmx_trpp_set_rec_mode(trpp_rec_info);
	
    /*set bus_urgent_mode*/
    global_info.bus_urgent_mode = 0x2;
	
    if (MT_UNF_DMX_REC_TYPE_SELECT_PID == RecAttr->enRecType)
    {
        if (RecAttr->filt_en != 0)
        {
            memcpy(default_index_data, RecAttr->start_code_filter, 16);
            trpp_index_info.filt_en = RecAttr->filt_en;
            MT_INFO_DEMUX("\ntrpp_index_info.filt_en flt=(%x)\n", trpp_index_info.filt_en);
        }
        else
        {
            /* for default table*/
            switch (RecAttr->enVCodecType)
            {
                case MT_UNF_VCODEC_TYPE_MPEG2:
                    trpp_index_info.filt_en = 0x44;  //0x00\0xb3 for header
                	break;
                case MT_UNF_VCODEC_TYPE_MPEG4:
                    trpp_index_info.filt_en = 0x21;  //0xb6\0xb0 for header
                	break;
                case MT_UNF_VCODEC_TYPE_AVS:
                    trpp_index_info.filt_en = 0x25;  //0xb6 0xb3 \0xb0 for sequence header
                	break;
                case MT_UNF_VCODEC_TYPE_H264:
                    trpp_index_info.filt_en=0xf00;  //0xff;
                	break;
                case MT_UNF_VCODEC_TYPE_VC1:
                    trpp_index_info.filt_en = 0x0a;  //0x0d\0x0f for header
                	break;
                case MT_UNF_VCODEC_TYPE_HEVC:
                    trpp_index_info.filt_en = 0x390;  //0x40\0x41 active sps
                	break;
                default:
                	break;
            }
            MT_INFO_DEMUX("\n trpp_index_info.filt_en flt=(%x)\n",__func__,__LINE__,trpp_index_info.filt_en);
        }
		
        memcpy(global_info.sc_index_flt, default_index_data, 16);
    }

	if(RecAttr->AlignToPageBlock)
	{
		page_block = ((1 << pageblock_order) * PAGE_SIZE);
		rec_round_size = ((RecBufSize+(page_block-1))&(~(page_block-1))); //aligned to page_block
		rec_align_size = page_block;
	}
	else
	{
		rec_round_size=RecBufSize;
		rec_align_size=page_block;
	}
			  
    for(i = 0; i < DMX_REC_CNT; i++)
    {
        char *mmz_pvr[DMX_REC_CNT] = {MMZ_ZONE_PVR0,MMZ_ZONE_PVR1,MMZ_ZONE_PVR2,MMZ_ZONE_PVR3};
        if(DmxMgr->bPvrRecBuffZoneIsUsed[i] == MT_FALSE)
        {
        	MT_INFO_DEMUX("[%s %d]RecAttr->link_node_num =%d\n", __FUNCTION__, __LINE__, RecAttr->link_node_num);
        	//the link_node_num's range is between 1 and 8, otherwise use 8 default.
        	if ((RecAttr->link_node_num >= 1) && (RecAttr->link_node_num <= 8))
        	{
				RecInfo->link_node_num = RecAttr->link_node_num;        		
        	}
			else
			{
				RecInfo->link_node_num = DMX_LINKREC_NODE_NUM;				
			}
			
			MT_INFO_DEMUX("[%s %d]RecInfo->link_node_num=%d\n", __FUNCTION__, __LINE__, RecInfo->link_node_num);
			link_rec_param->link_node_num = RecInfo->link_node_num;
			rec_round_size = RecBufSize/RecInfo->link_node_num;
            rec_align_size = page_block;
        	for (j=0; j<RecInfo->link_node_num; j++)
        	{
        		if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_REC", mmz_pvr[i], rec_round_size, rec_align_size, &RecMmzBuf))
	            {
	                MT_FATAL_DEMUX("ts packet memory allocate failed for pvr%d\n",i);
	                /*
	                patch for bug126772, if request mem failed, release current record dmx id
	                */
	                RecInfo->DmxId = DMX_INVALID_DEMUX_ID;
					RecInfo->RecId = DMX_INVALID_CHAN_ID;
	                return MT_ERR_DMX_ALLOC_MEM_FAILED;
	            }

				MT_INFO_DEMUX("[%s %d]j=%d, RecInfo->RecId=%d, RecInfo->DmxId=%d : RecMmzBuf.size = %x,RecMmzBuf.u32BufPhyAddr=%x,RecMmzBuf.startVirAddr=%x\n",
                 			  __FUNCTION__, __LINE__, j, RecInfo->RecId, RecInfo->DmxId, RecMmzBuf.size, RecMmzBuf.startPhyAddr, RecMmzBuf.startVirAddr);
				
				memset((mt_u8 *)RecMmzBuf.startVirAddr, 0, RecMmzBuf.size);
		//		printk("[%s %d]RecMmzBuf.startPhyAddr=0x%llx, RecMmzBuf.startVirAddr=0x%llx, RecMmzBuf.size=0x%lx(%ld)\n", 
		//			__FUNCTION__, __LINE__, RecMmzBuf.startPhyAddr, (MT_U64)RecMmzBuf.startVirAddr, RecMmzBuf.size, RecMmzBuf.size);
				RecInfo->link_rec_buf[j].startPhyAddr = RecMmzBuf.startPhyAddr;
				RecInfo->link_rec_buf[j].startVirAddr = RecMmzBuf.startVirAddr;
				RecInfo->link_rec_buf[j].size = RecMmzBuf.size;
				link_rec_param->RecBufPhyAddr[j] = RecMmzBuf.startPhyAddr;
				link_rec_param->RecBufSize[j] = RecMmzBuf.size;
				
				reg_set_trpp_ch11_lln_idx_set0(RecInfo->RecId, j, RecMmzBuf.startPhyAddr);
				reg_value = reg_get_trpp_ch11_lln_idx_set1(RecInfo->RecId, j);
				reg_value &= (~((7<<28) | 0x7FFFFFF));
				if (RecInfo->link_node_num-1 == j)
				{
					//the size unit is 8bytes
					reg_value |= ((0<<28) | ((RecMmzBuf.size>>3) & 0x7FFFFFF));
				}
				else
				{
					reg_value |= (((j+1)<<28) | ((RecMmzBuf.size>>3) & 0x7FFFFFF));
				}
				
				reg_set_trpp_ch11_lln_idx_set1(RecInfo->RecId, j, reg_value);
        	}
            
            if (RecInfo->link_node_num == j)
            {
                DmxMgr->bPvrRecBuffZoneIsUsed[i] = MT_TRUE;
                RecInfo->pvrRecBuffZoneId = i;
                break;
            }
        }
    }
    if(i == DMX_REC_CNT)
    {
        MT_FATAL_DEMUX("memory allocate zone error !!\n");
        return MT_ERR_DMX_ALLOC_MEM_FAILED;
    }

    MT_INFO_DEMUX("\n RecBufSize(%x) RecMmzBuf.size(%x)\n", RecBufSize, RecMmzBuf.size);

    RecInfo->rp_globe=0;
    RecInfo->overflow_cnt=0;
    RecInfo->recbuf_size=RecBufSize;
    memset(&RecInfo->overflowinfo, 0x00, sizeof(DMX_RecInfo_Overflow_S));

	//trpp_index_info.index_chnum = RecInfo->RecId;
	if (RecInfo->RecId <= 3)
	{
		trpp_index_info.index_chnum = RecInfo->RecId;
	}
	else//record channel 4-6 use 8-11 for index channel
	{
		trpp_index_info.index_chnum = RecInfo->RecId + 4;		
	}
	MT_INFO_DEMUX("[%s %d]trpp_index_info.index_chnum=%d\n", __FUNCTION__, __LINE__, trpp_index_info.index_chnum);
	if (RecAttr->idx_filter_mode)
	{
		reg_value = reg_get_trpp_mode();
		reg_value &= (~(1<<28));//bit28:0, index use unshare mode
		reg_set_trpp_mode(reg_value);
		dmx_set_idx_chn_start_code_filter(trpp_index_info.index_chnum, global_info);
	}
	else
	{
		reg_value = reg_get_trpp_mode();
		reg_value |= (1<<28);//bit28:1, index use share mode
		reg_set_trpp_mode(reg_value);
    	dmx_trpp_start_code_filter_cfg(global_info);
	}
	
    if (((1 == trpp_index_info.afld_en) ||
        (1 == trpp_index_info.pause_en) ||
        (1 == trpp_index_info.pes_head_en) ||
        (1 == trpp_index_info.pts_en) ||
        (1 == trpp_index_info.pusi_en) ||
        (1 == trpp_index_info.sc_en)) &&
        (0 != trpp_index_info.filt_en))
    {
        if ((trpp_index_info.index_chnum < DMX_REC_INDEX_CNT) && (trpp_index_info.index_chnum >= 0))
        {
            //set trpp_index_start
            trpp_index_info.data_buf_staddr = 0;
            trpp_index_info.data_buf_endaddr = 0;
            index_enable = 1;
            dmx_trpp_set_index_mode(DMX_VIDEO_TYPE, trpp_index_info);
        }
    }

	printk("[%s %d]index_enable = %d\n", __FUNCTION__, __LINE__, index_enable);
    if (index_enable)
    {        
       	RecInfo->IndexType = RecAttr->enIndexType;
        if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("DMX_RecIdxBuf", MMZ_OTHERS, TRPP_REC_INDEX_BUFFER_SIZE, MIN_MMZ_BUF_SIZE, &RecIdxMmzBuf))
        {
            MT_FATAL_DEMUX("index data memory allocate failed\n");
            /*
            patch for bug126772, if request mem failed, release current record dmx id
            */
            RecInfo->DmxId = DMX_INVALID_DEMUX_ID;
			RecInfo->RecId = DMX_INVALID_CHAN_ID;
            return MT_ERR_DMX_ALLOC_MEM_FAILED;
        }
		
        memset((mt_u8 *)RecIdxMmzBuf.startVirAddr, 0, RecIdxMmzBuf.size);
        link_rec_param->RecIdxBufPhyAddr = RecIdxMmzBuf.startPhyAddr;
        link_rec_param->RecIdxBufSize = RecIdxMmzBuf.size;
        RecInfo->RecIdxBuffer.startPhyAddr = RecIdxMmzBuf.startPhyAddr;
        RecInfo->RecIdxBuffer.startVirAddr = RecIdxMmzBuf.startVirAddr;
        MT_INFO_DEMUX("RecInfo->RecId=%d, RecInfo->DmxId=%d : RecIdxMmzBuf.size = %x,RecIdxMmzBuf.u32BufPhyAddr=%x,RecIdxMmzBuf.startVirAddr=%x\n",
                     RecInfo->RecId, RecInfo->DmxId, RecIdxMmzBuf.size, RecIdxMmzBuf.startPhyAddr, RecIdxMmzBuf.startVirAddr);
        /*set recorded ts packet len,added by l00188263 ,Hi3719 support 192 Byte ts packet length recording*/
        if (1 == index_enable)
        {
            RecInfo->slot_reg1.bitc.sc_fetch_en = 1;
            RecInfo->slot_reg0.bitc.sc_fetch_ch = (trpp_index_info.index_chnum & 0x08)>>3;
            RecInfo->slot_reg1.bitc.sc_fetch_ch = (trpp_index_info.index_chnum & 0x07);
        }
        else
        {
            RecInfo->slot_reg1.bitc.sc_fetch_en = 0;
            RecInfo->slot_reg1.bitc.sc_fetch_ch = 0;
        }

        dmx_rec_index_set_buffer(NULL, trpp_index_info.index_chnum, (mt_u8 *)((ulong)RecIdxMmzBuf.startPhyAddr), (u32)RecIdxMmzBuf.size);
    }
    else
    {
        RecInfo->IndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
    }
	
    if (MT_UNF_DMX_REC_TYPE_ALL_PID == RecAttr->enRecType)
    {
    	if (DMX_FULL_TS_WITHOUT_NULL_PACKET == RecAttr->type_mode)
        {
            mt_u32 rec_channl = 0;
            RecInfo->slot_reg0.bitc.pid = 0x1fff;
            RecInfo->slot_reg0.bitc.pid_filter_en = 1;
            RecInfo->slot_reg0.bitc.pid_filter_mode = 1;
            DMX_DRV_REC_AddRecPid(RecInfo->RecId, 0x1fff, &rec_channl);
        }
		else
		{
            mt_u32 rec_channl = 0;
            RecInfo->slot_reg0.bitc.pid_filter_en = 0;  //rec all ts
            RecInfo->slot_reg0.bitc.pid_filter_mode = 0;
            DMX_DRV_REC_AddRecPid(RecInfo->RecId, 0x1fff, &rec_channl);
        }
    }

	reg_value = reg_get_trpp_bus_urgent();
	reg_value |= (1<<30);//set TRPP_RAM_WR_MODE to 1
	reg_set_trpp_bus_urgent(reg_value);

	reg_value = reg_get_trpp_mode();
	reg_value |= (1<<31);//set REC_IDX_MODE to 1
	reg_set_trpp_mode(reg_value);

	reg_value = reg_get_trpp_ch11_lln_set0(RecInfo->RecId);
	reg_value |= (1<<30) | ((rec_round_size * RecInfo->link_node_num)>>3);//set LLN_TYPE to 1 and total size
	reg_set_trpp_ch11_lln_set0(RecInfo->RecId, reg_value);

	reg_value = reg_get_trpp_ch11_lln_set1(RecInfo->RecId);
	reg_value &= (~(7<<0));
	reg_value |= (7<<0);
	reg_set_trpp_ch11_lln_set1(RecInfo->RecId, reg_value);
	
	reg_value = reg_get_trpp_ch11_lln_set3(RecInfo->RecId);
	reg_value &= (~0x07);//set LLN_NEXT_ID to 0
	reg_set_trpp_ch11_lln_set3(RecInfo->RecId, reg_value);
    return MT_SUCCESS;
}

mt_s32 DMX_DRV_REC_DestroyChannel(mt_u32 RecId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
	mmz_buffer_s MmzBuf;
	int i=0;

	if (RecInfo->RecId < DMX_REC_CNT) {
		if (DMX_REC_STATUS_STOP == RecInfo->RecStatus)
		{
			for(i=0; i<DMX_CHANNEL_NUM_PER_RECORD;i++)
			{
				if(RecInfo->ChannelId[i] != DMX_INVALID_CHAN_ID)
				{
					DMX_DRV_REC_DelRecPid(RecId,  RecInfo->ChannelId[i]);
					//printk("DMX_DRV_REC_DestroyChannel >>>>>>> RecInfo->ChannelId[i]=0x%x\n",RecInfo->ChannelId[i]);
					RecInfo->ChannelId[i] = DMX_INVALID_CHAN_ID;
				}
			}

		    DmxFqIdRelease(RecInfo->RecFqId);

		    if(RecInfo->RecBuffer.startPhyAddr > 0 && RecInfo->RecBuffer.startVirAddr > 0)
		    {
			    MmzBuf.startPhyAddr = RecInfo->RecBuffer.startPhyAddr;
			    MmzBuf.startVirAddr = RecInfo->RecBuffer.startVirAddr;

			    mt_drv_mmz_unmap_and_release(&MmzBuf);
			    RecInfo->RecBuffer.startPhyAddr = 0;
			    RecInfo->RecBuffer.startVirAddr = 0;
			#if defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)
			    if(RecInfo->pvrRecBuffZoneId >=0 && RecInfo->pvrRecBuffZoneId < 4)
			    {
			    	 DmxMgr->bPvrRecBuffZoneIsUsed[RecInfo->pvrRecBuffZoneId] = MT_FALSE;
			    }	
			#endif //#ifdef CONFIG_MT_CHIP_SYMPHONY4
		    }

		    if (MT_UNF_DMX_REC_INDEX_TYPE_NONE != RecInfo->IndexType) {
				if((RecInfo->RecIdxBuffer.startPhyAddr > 0)  &&  (RecInfo->RecIdxBuffer.startVirAddr > 0 ))
				{
					MmzBuf.startPhyAddr = RecInfo->RecIdxBuffer.startPhyAddr;
					MmzBuf.startVirAddr = RecInfo->RecIdxBuffer.startVirAddr;

					mt_drv_mmz_unmap_and_release(&MmzBuf);
					RecInfo->RecIdxBuffer.startPhyAddr  = 0;
					RecInfo->RecIdxBuffer.startVirAddr = 0;
				}
		    }

		    if (0 == down_interruptible(&RecInfo->LockRec)) {
				RecInfo->DmxId = DMX_INVALID_DEMUX_ID;
				RecInfo->RecId = DMX_INVALID_CHAN_ID;
				RecInfo->IndexChanID = DMX_INVALID_CHAN_ID;
				up(&RecInfo->LockRec);
		    }

		    ret = MT_SUCCESS;
		} else {
		    MT_ERR_DEMUX("stop the rec channel first!\n");
		    ret = MT_ERR_DMX_STARTING_REC_CHAN;
		}
	}

    return ret;
}

mt_s32 DMX_DRV_LinkREC_DestroyChannel(mt_u32 RecId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
	mmz_buffer_s MmzBuf;
	int i=0;

	if (RecInfo->RecId < DMX_REC_CNT) {
		if (DMX_REC_STATUS_STOP == RecInfo->RecStatus)
		{
			for(i=0; i<DMX_CHANNEL_NUM_PER_RECORD;i++)
			{
				if(RecInfo->ChannelId[i] != DMX_INVALID_CHAN_ID)
				{
					DMX_DRV_REC_DelRecPid(RecId,  RecInfo->ChannelId[i]);
					//printk("DMX_DRV_REC_DestroyChannel >>>>>>> RecInfo->ChannelId[i]=0x%x\n",RecInfo->ChannelId[i]);
					RecInfo->ChannelId[i] = DMX_INVALID_CHAN_ID;
				}
			}

		    DmxFqIdRelease(RecInfo->RecFqId);

			for (i=0; i<RecInfo->link_node_num; i++)
			{
				if(RecInfo->link_rec_buf[i].startPhyAddr > 0 && RecInfo->link_rec_buf[i].startVirAddr > 0)
			    {
				    MmzBuf.startPhyAddr = RecInfo->link_rec_buf[i].startPhyAddr;
				    MmzBuf.startVirAddr = RecInfo->link_rec_buf[i].startVirAddr;

				    mt_drv_mmz_unmap_and_release(&MmzBuf);
				    RecInfo->link_rec_buf[i].startPhyAddr = 0;
				    RecInfo->link_rec_buf[i].startVirAddr = 0;				    	
			    }
			}
			
			if(RecInfo->pvrRecBuffZoneId >=0 && RecInfo->pvrRecBuffZoneId < 4)
		    {
		    	 DmxMgr->bPvrRecBuffZoneIsUsed[RecInfo->pvrRecBuffZoneId] = MT_FALSE;
		    }

		    if (MT_UNF_DMX_REC_INDEX_TYPE_NONE != RecInfo->IndexType) {
				if((RecInfo->RecIdxBuffer.startPhyAddr > 0)  &&  (RecInfo->RecIdxBuffer.startVirAddr > 0 ))
				{
					MmzBuf.startPhyAddr = RecInfo->RecIdxBuffer.startPhyAddr;
					MmzBuf.startVirAddr = RecInfo->RecIdxBuffer.startVirAddr;

					mt_drv_mmz_unmap_and_release(&MmzBuf);
					RecInfo->RecIdxBuffer.startPhyAddr  = 0;
					RecInfo->RecIdxBuffer.startVirAddr = 0;
				}
		    }

		    if (0 == down_interruptible(&RecInfo->LockRec)) {
				RecInfo->DmxId = DMX_INVALID_DEMUX_ID;
				RecInfo->RecId = DMX_INVALID_CHAN_ID;
				RecInfo->IndexChanID = DMX_INVALID_CHAN_ID;
				up(&RecInfo->LockRec);
		    }

		    ret = MT_SUCCESS;
		} else {
		    MT_ERR_DEMUX("stop the rec channel first!\n");
		    ret = MT_ERR_DMX_STARTING_REC_CHAN;
		}
	}

    return ret;
}


mt_s32 DMX_DRV_REC_AddRecPid(mt_u32 RecId, mt_u32 Pid, mt_u32 *ChanId)
{
    mt_s32 ret;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
    DMX_ChanInfo_S *ChanInfo=NULL;
    mt_s32 i = 0;

    MT_UNF_DMX_CHAN_ATTR_S ChanAttr;
    DMX_MMZ_BUF_S ChanBuf;
    if (Pid > DMX_INVALID_PID) {
		MT_ERR_DEMUX("Invalid pid:0x%x\n", Pid);
		return MT_ERR_DMX_INVALID_PARA;
    }

    CHECKDMXID(RecInfo->DmxId);

    ChanAttr.u32BufSize = 0;
    ChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_REC;
    ChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
    ChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_REC;

    ret = DMX_OsiCreateChannel(RecInfo->DmxId, &ChanAttr, &ChanBuf, NULL, ChanId);
    if (MT_SUCCESS != ret) {
       	MT_ERR_DEMUX("RecInfo->DmxId=%d\n", RecInfo->DmxId);
		return ret;
    }
	
    ret = DMX_OsiSetChannelPid(*ChanId, Pid);
    if (MT_SUCCESS != ret) {
		DMX_OsiDestroyChannel(*ChanId);
       	MT_ERR_DEMUX("RecInfo->DmxId=%d\n", RecInfo->DmxId);
		return ret;
    }

	ChanInfo = &DmxMgr->DmxChanInfo[*ChanId];
    if(CHECKVALIDEPID(RecInfo->IndexPid))
    {
        if(RecInfo->IndexPid == Pid && CHECKVALIDEPID(Pid))
	    {
	        if(ChanInfo)
	        {
		        ChanInfo->slot_reg1.bitc.sc_fetch_en = RecInfo->slot_reg1.bitc.sc_fetch_en;
				ChanInfo->slot_reg0.bitc.sc_fetch_ch = RecInfo->slot_reg0.bitc.sc_fetch_ch;
		        ChanInfo->slot_reg1.bitc.sc_fetch_ch = RecInfo->slot_reg1.bitc.sc_fetch_ch;
	        }
	    }
    }
	        
    if(ChanInfo)
    {
    	ChanInfo->ChanType = MT_UNF_DMX_CHAN_TYPE_REC;
    	ChanInfo->slot_reg0.bitc.rec_ch = (RecInfo->RecId & 0x04) >> 2;//set rec_ch[2]
        ChanInfo->slot_reg1.bitc.rec_ch = (RecInfo->RecId & 0x03);	//set rec_ch[1:0]
        ChanInfo->rec_id = RecId;
    }
	
    ret = DMX_OsiOpenChannel(*ChanId);
    if (MT_SUCCESS != ret) {
		DMX_OsiDestroyChannel(*ChanId);
        MT_ERR_DEMUX("RecInfo->DmxId=%d\n", RecInfo->DmxId);
		return ret;
    }

    for(i=0; i<DMX_CHANNEL_NUM_PER_RECORD;i++)
    {
        if(RecInfo->ChannelId[i] == DMX_INVALID_CHAN_ID)
        {
            RecInfo->ChannelId[i] = *ChanId;			
			break;
        }
    }

    return MT_SUCCESS;
}

mt_s32 DMX_DRV_REC_DelRecPid(mt_u32 RecId, mt_u32 ChanId)
{
    mt_s32 ret;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
    DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChanId];
    int i=0;

    if ((RecInfo->DmxId >= DMX_CNT) || (ChanInfo->DmxId >= DMX_CNT)) {
		MT_ERR_DEMUX("RecInfo->DmxId:%d or ChanInfo->DmxId:%d invalid\n", RecInfo->DmxId, ChanInfo->DmxId);
		return MT_ERR_DMX_INVALID_PARA;
    }

    if (RecInfo->DmxId != ChanInfo->DmxId) {
		MT_ERR_DEMUX("RecInfo->DmxId:%d or ChanInfo->DmxId:%d not equal!\n", RecInfo->DmxId, ChanInfo->DmxId);
		return MT_ERR_DMX_UNMATCH_CHAN;
    }

    ret = DMX_OsiCloseChannel(ChanId);
    if (MT_SUCCESS != ret) {
		return ret;
    }

    ret = DMX_OsiDestroyChannel(ChanId);
    if (MT_SUCCESS != ret) {
		return ret;
    }

    for(i=0; i<DMX_CHANNEL_NUM_PER_RECORD;i++)
    {
        if(RecInfo->ChannelId[i] == ChanId)
        {
            RecInfo->ChannelId[i] = DMX_INVALID_CHAN_ID;
			break;
        }
    }
    return MT_SUCCESS;
}

mt_s32 DMX_DRV_REC_DelAllRecPid(mt_u32 RecId)
{
    mt_s32 ret = MT_SUCCESS;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
    DMX_ChanInfo_S *ChanInfo;
    mt_u32 i;

    CHECKDMXID(RecInfo->DmxId);

    for (i = 0; i < DMX_CHANNEL_CNT; i++) {
	ChanInfo = &DmxMgr->DmxChanInfo[i];

	if ((RecInfo->DmxId == ChanInfo->DmxId) && (MT_UNF_DMX_CHAN_REC_EN == ChanInfo->ChanStatus)) {
	    ret = DMX_OsiCloseChannel(i);
	    if (MT_SUCCESS != ret) {
			break;
	    }

	    ret = DMX_OsiDestroyChannel(i);
	    if (MT_SUCCESS != ret) {
			break;
	    }
	}
    }

    for(i=0; i<DMX_CHANNEL_NUM_PER_RECORD;i++)
    {
        if(RecInfo->ChannelId[i] != DMX_INVALID_CHAN_ID)
        {
            RecInfo->ChannelId[i] = DMX_INVALID_CHAN_ID;
        }
    }

    return ret;
}

mt_s32 DMX_DRV_REC_GetTsCnt(mt_u32 RecId, mt_u32 *TSCnt)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];

    if (RecInfo->DmxId >= DMX_CNT) {
	return MT_ERR_DMX_INVALID_PARA;
    }

    *TSCnt = DmxHalGetOqCounter(RecInfo->RecOqId);
    return MT_SUCCESS;
}

mt_s32 DMX_DRV_REC_AddExcludeRecPid(mt_u32 RecId, mt_u32 Pid)
{
#ifdef DMX_REC_EXCLUDE_PID_SUPPORT
    mt_u32 i;
    mt_u32 tmpRecDmxID, tmpPid;
#endif

    if (Pid > DMX_INVALID_PID) {
		MT_ERR_DEMUX("Invalid pid 0x%x\n", Pid);

		return MT_ERR_DMX_INVALID_PARA;
    }

#ifdef DMX_REC_EXCLUDE_PID_SUPPORT
    for (i = 0; i < DMX_REC_EXCLUDE_PID_NUM; i++) {
		DmxHalGetAllRecExcludePid(i, &tmpRecDmxID, &tmpPid);
		if ((tmpRecDmxID - 1) == RecId && tmpPid == Pid) {
		    return MT_SUCCESS; //already added the pid
		}
    }

    for (i = 0; i < DMX_REC_EXCLUDE_PID_NUM; i++) {
		DmxHalGetAllRecExcludePid(i, &tmpRecDmxID, &tmpPid);
		if (tmpRecDmxID == 0) {
		    DmxHalSetAllRecExcludePid(i, RecId + 1, Pid);
		    return MT_SUCCESS;
		}
    }
    MT_ERR_DEMUX("no available exclude pid resource: 0x%x\n", Pid);
    return MT_ERR_DMX_NOAVAILABLE_EXCLUDEPID;
#else
    return MT_ERR_DMX_NOT_SUPPORT;
#endif
}

mt_s32 DMX_DRV_REC_DelExcludeRecPid(mt_u32 RecId, mt_u32 Pid)
{
#ifdef DMX_REC_EXCLUDE_PID_SUPPORT
    mt_u32 i;
    mt_u32 tmpRecDmxID, tmpPid;
#endif
    if (Pid > DMX_INVALID_PID) {
		MT_ERR_DEMUX("Invalid pid 0x%x\n", Pid);

		return MT_ERR_DMX_INVALID_PARA;
    }

#ifdef DMX_REC_EXCLUDE_PID_SUPPORT
    for (i = 0; i < DMX_REC_EXCLUDE_PID_NUM; i++) {
		DmxHalGetAllRecExcludePid(i, &tmpRecDmxID, &tmpPid);
		if ((tmpRecDmxID - 1) == RecId && tmpPid == Pid) {
		    DmxHalSetAllRecExcludePid(i, 0, DMX_INVALID_PID);
		    return MT_SUCCESS;
		}
    }
    return MT_SUCCESS;
#else
    return MT_ERR_DMX_NOT_SUPPORT;
#endif
}

mt_s32 DMX_DRV_REC_DelAllExcludeRecPid(mt_u32 RecId)
{
#ifdef DMX_REC_EXCLUDE_PID_SUPPORT
    mt_u32 i;
    mt_u32 tmpRecDmxID, tmpPid;
#endif

#ifdef DMX_REC_EXCLUDE_PID_SUPPORT
    for (i = 0; i < DMX_REC_EXCLUDE_PID_NUM; i++) {
		DmxHalGetAllRecExcludePid(i, &tmpRecDmxID, &tmpPid);
		if ((tmpRecDmxID - 1) == RecId) {
		    DmxHalSetAllRecExcludePid(i, 0, DMX_INVALID_PID);
		}
    }
    return MT_SUCCESS;
#else
    return MT_ERR_DMX_NOT_SUPPORT;
#endif
}

mt_s32 DMX_DRV_REC_StartRecChn(mt_u32 RecId)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
	
    CHECKDMXID(RecInfo->DmxId);

    if (DMX_REC_STATUS_START == RecInfo->RecStatus)
    {
        return MT_SUCCESS;
    }

    RecInfo->FirstFrameMs = 0;
    RecInfo->AddUpMs = 0;
    RecInfo->RecStatus = DMX_REC_STATUS_START;
    RecInfo->ScrambleDetectTime = 0;
    RecInfo->ScrambleDetectOffset = 0;
    RecInfo->ClearDetectTime = 0;
    RecInfo->ScrambleDetectCnt = 0;
    RecInfo->ClearDetectCnt = 0;
    RecInfo->bSCDBufIsEmpty = MT_TRUE;

    memset(&RecInfo->LastFrameInfo, 0, sizeof(MT_UNF_DMX_REC_INDEX_S));
    up(&RecInfo->LockRec);

    MT_DBG_DEMUX("RecInfo->DmxId=%d\n", RecInfo->DmxId);
    if (RecId >= 0 && RecId < DMX_REC_CNT)
    {
        mt_u32 data = reg_get_trpp_channel_record_en();

        DMX_OsiClearRecBuf(RecId, 0);
        RecInfo->interrupt_count = 0;
		if ((RecId >=0) && (RecId <=3))
		{
			//bit0,1,2,3,4
        	reg_set_trpp_channel_record_en(data | (1 << RecId));
		}
		else if ((RecId >=4) && (RecId <=6))
		{
			//bit20,21,22
			reg_set_trpp_channel_record_en(data | (1 << (16+RecId)));
		}
        RecInfo->interrupt_count = 0;
	    /*init reg_set_trpp_ch11_ini_info1-4*/
        //reg_set_trpp_ch11_ini_info1(RecId, 0);
        //reg_set_trpp_ch11_ini_info2(RecId, 0);
    }
    return MT_SUCCESS;
}

mt_s32 DMX_DRV_REC_StopRecChn(mt_u32 RecId)
{
    mt_s32 ret;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
    DMX_OQ_Info_S *RecOqInfo = &DmxMgr->DmxOqInfo[RecInfo->RecOqId];
	mt_u32 reg_value;
#if 1
    CHECKDMXID(RecInfo->DmxId);

    if (DMX_REC_STATUS_STOP == RecInfo->RecStatus) {
		return MT_SUCCESS;
    }

    ret = down_interruptible(&RecInfo->LockRec);
    RecInfo->RecStatus = DMX_REC_STATUS_STOP;
    up(&RecInfo->LockRec);

    RecOqInfo->OqWakeUp = MT_TRUE;

    wake_up_interruptible(&RecOqInfo->OqWaitQueue);

    if (MT_UNF_DMX_REC_INDEX_TYPE_NONE != RecInfo->IndexType) {
		DMX_OQ_Info_S *ScdOqInfo = &DmxMgr->DmxOqInfo[RecInfo->ScdOqId];

		ScdOqInfo->OqWakeUp = MT_TRUE;

		wake_up_interruptible(&ScdOqInfo->OqWaitQueue);

		DmxFqStop(RecInfo->ScdFqId);

		DmxHalDisableEsSCD(RecInfo->ScdId);
		DmxHalDisablePesSCD(RecInfo->ScdId);

		DmxHalScdFilterClear(RecInfo->ScdId);
		DmxHalScdRangeFilterClear(RecInfo->ScdId);
		DmxHalScdNewRangeFilterClear(RecInfo->ScdId);

		DmxHalSetSCDAttachChannel(RecInfo->ScdId, DMX_REC_SCD_INVALID_CHANNEL);
    }


//DmxRecFlush(RecId);
//DmxHalSetRecType(RecInfo->DmxId, DMX_REC_TYPE_NONE);
#ifdef DMX_REC_EXCLUDE_PID_SUPPORT
    if (MT_UNF_DMX_REC_TYPE_ALL_PID == RecInfo->RecType) {
	// DmxHalDisableAllRecExcludePid(RecInfo->DmxId);
    }
#endif
    {
        reg_set_trpp_ch_idx_mode(RecId, 0);
        reg_set_trpp_ch_idx_enable(RecId, 0);
        //reg_set_trpp_ch_rec_set(RecInfo->DmxId, 0);
        //reg_set_trpp_channel_record_en(0);
        {
            int time_out=3;
            reg_set_trpp_ch_rec_set(RecId, 0);
            //reg_set_trpp_channel_record_en(0);
            RecInfo->interrupt_count = 0;
			reg_value = reg_get_trpp_channel_record_en();
            switch(RecId)
            {
                case 0:
				case 1:
				case 2:
				case 3:
					reg_value &= (~(1<<RecId));//bit0-3 for trpp_chx_rec_en(x=0,1,2,3)
                	break;
					
                case 4:
				case 5:
				case 6:
					reg_value &= (~(1<<(16+RecId)));//bit20-22 for trpp_chx_rec_en(x=4,5,6)
	                break;
					
                default:
                	break;
            }

			reg_value &= (~(1<<(8+RecId)));//bit8-19 for trpp_chx_idx_en(x=0,1,...,11)
			reg_set_trpp_channel_record_en(reg_value);            

            while(reg_get_trpp_ch_clear_status() == 0 && (time_out--))
            {
                mdelay(1);
            }
            reg_set_trpp_ch_clear_status(0);
        }

        DMX_OsiClearRecBuf(RecId, 0);
        DMX_OsiClearIndexBuf(RecId, 0);
    }
#endif
    return MT_SUCCESS;
}

mt_s32 DMX_DRV_REC_AcquireRecData(mt_u32 RecId, phys_addr_t *PhyAddr, ulong *KerAddr, mt_u32 *Len, mt_u32 Timeout)
{
    mt_s32 ret;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
    DMX_FQ_Info_S *FqInfo = MT_NULL;
    DMX_OQ_Info_S *OqInfo = MT_NULL;
    mt_u32 Read;
    mt_u32 Write;
//#ifdef CONFIG_MT_PVR	
#if 0//need realize in pvr, so comment it temp
	mt_u32 rec_num = 0;				//how many ts packets have been read by index
	mt_u32 idx_read_pointer = 0;	//the position in rec buf which is read by index
#endif
    CHECKDMXID(RecInfo->DmxId);

    if (DMX_REC_STATUS_START != RecInfo->RecStatus) 
	{
		MT_ERR_DEMUX("[%s %d]RecInfo->RecStatus=%d\n", __FUNCTION__, __LINE__, RecInfo->RecStatus);
		return MT_ERR_DMX_NOT_START_REC_CHAN;
    }

    FqInfo = &DmxMgr->DmxFqInfo[RecInfo->RecFqId];
    OqInfo = &DmxMgr->DmxOqInfo[RecInfo->RecOqId];
    OqInfo->u32OQId = RecId; //fixbug dxmid=1 for rec
    //MT_DBG_DEMUX("RecInfo->RecOqId==================11=========%d, OqInfo=0x%x\n",RecInfo->RecOqId, OqInfo);
    DMXOsiOQGetReadWrite(OqInfo->u32OQId, &Write, &Read);

    if (Read == Write) 
	{
		if (0 == Timeout) 
		{
		    return MT_ERR_DMX_NOAVAILABLE_DATA;
		}

		//DmxHalOQEnableOutputInt(OqInfo->u32OQId, MT_TRUE);

		OqInfo->OqWakeUp = MT_FALSE;

		ret = wait_event_interruptible_hrtimeout(OqInfo->OqWaitQueue, OqInfo->OqWakeUp, ms_to_ktime(Timeout));
		if (!OqInfo->OqWakeUp) 
		{
		    DmxHalOQEnableOutputInt(OqInfo->u32OQId, MT_FALSE);

		    return MT_ERR_DMX_TIMEOUT;
		}

		OqInfo->OqWakeUp = MT_FALSE;

		DMXOsiOQGetReadWrite(OqInfo->u32OQId, &Write, &Read);
    }

    if (DMX_REC_STATUS_START != RecInfo->RecStatus) 
	{
		return MT_ERR_DMX_NOAVAILABLE_DATA;
    }	
    if (Read != Write) 
	{
		*PhyAddr =(phys_addr_t)( (reg_get_trpp_ch_rec_start_addr_trpp_ch_rec_saddr(RecId) << 3) + Read);
		*KerAddr = (ulong)RecInfo->RecBuffer.startVirAddr; 
		*Len = 0;
	//#ifdef CONFIG_MT_PVR		
	#if 0//need realize in pvr, so comment it temp		
		if (RecInfo->sync_with_index)
		{	
			rec_num = PVR_GetRecNum(RecId);
			MT_INFO_DEMUX("[%s %d]rec_num=%d\n", __FUNCTION__, __LINE__, rec_num);
			idx_read_pointer = (rec_num % (RecInfo->recbuf_size/188)) * 188;			
			if (idx_read_pointer >= Read) 
			{
			    *Len = ((idx_read_pointer - Read) / 188) * 188;
			} 
			else 
			{
			    *Len = RecInfo->recbuf_size - Read;
			}
			MT_INFO_DEMUX("[%s %d]idx_read_pointer=%d, Read=%d, len=%d\n", __FUNCTION__, __LINE__, idx_read_pointer, Read, *Len);
		}
		else
	#endif	
		if (Write >= Read) 
		{
		    *Len = ((Write - Read) / 188) * 188;
		} 
		else 
		{
		    *Len = RecInfo->recbuf_size - Read;
		    MT_DBG_DEMUX("2 len = %x , %x, %x, %x\n", RecInfo->recbuf_size - Read, *Len, Read, Write);
		}
		
		//MT_DBG_DEMUX("Read=%x,  Write=%x, KerAddr=0x%x, *Len=0x%x\n",Read, Write, *KerAddr,*Len);
		return MT_SUCCESS;
    }
    return MT_ERR_DMX_TIMEOUT;
}

mt_s32 DMX_DRV_LinkREC_AcquireRecData(mt_u32 RecId, phys_addr_t *PhyAddr, ulong *KerAddr, mt_u32 *Len, mt_u32 Timeout, mt_u8 *p_node_index)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];   

	mt_u32 reg_value;
	mt_u32 lln_write_num;
	mt_u32 lln_read_num;
	//mt_u8 node_index = RecInfo->node_index;
	mt_u32 count = Timeout/10;
	
    CHECKDMXID(RecInfo->DmxId);

    if (DMX_REC_STATUS_START != RecInfo->RecStatus) 
	{
		MT_ERR_DEMUX("[%s %d]RecInfo->RecStatus=%d\n", __FUNCTION__, __LINE__, RecInfo->RecStatus);
		return MT_ERR_DMX_NOT_START_REC_CHAN;
    }
	
	lln_write_num = reg_get_trpp_ch11_lln_data_byte_num(RecId);//the unit is 8bytes
	MT_INFO_DEMUX("RecId=%d, lln_write_num=%d\n", RecId, lln_write_num);
	lln_write_num &= 0x3FFFFFFF;
	lln_write_num <<= 3;//how many byte
    while (0 == lln_write_num) 
	{		
		if (0 == count) 
		{
			MT_ERR_DEMUX("[%s %d]no available data\n", __FUNCTION__, __LINE__);
		    return MT_ERR_DMX_NOAVAILABLE_DATA;
		}

		count--;		
		msleep(10);		
		
		lln_write_num = reg_get_trpp_ch11_lln_data_byte_num(RecId);
		//printk("2:lln_write_num=%d(0x%x)\n", lln_write_num, lln_write_num);
		lln_write_num &= 0x3FFFFFFF;
		lln_write_num <<= 3;
    }

    if (lln_write_num != 0) 
	{
		MT_INFO_DEMUX("[%s %d]recid=%d, lln_write_num=%d, node_index=%d, RecInfo->link_rec_buf[node_index].size=%ld, RecInfo->read_num[node_index]=%d\n", 
				__FUNCTION__, __LINE__, RecId, lln_write_num, RecInfo->node_index, 
				RecInfo->link_rec_buf[RecInfo->node_index].size, RecInfo->read_num[RecInfo->node_index]);
		if (lln_write_num > RecInfo->link_rec_buf[RecInfo->node_index].size - RecInfo->read_num[RecInfo->node_index])
		{
			lln_read_num = RecInfo->link_rec_buf[RecInfo->node_index].size - RecInfo->read_num[RecInfo->node_index];
		}	
		else
		{
			lln_read_num = lln_write_num;
		}
				
		*PhyAddr = (phys_addr_t)(RecInfo->link_rec_buf[RecInfo->node_index].startPhyAddr + RecInfo->read_num[RecInfo->node_index]);
		*KerAddr = (ulong)RecInfo->link_rec_buf[RecInfo->node_index].startVirAddr; 		
		*Len = lln_read_num;
//		MT_INFO_DEMUX("[%s %d]*Len=%d, *PhyAddr=0x%llx, KerAddr=0x%llx, *KerAddr=0x%lx\n", __FUNCTION__, __LINE__, *Len, *PhyAddr, (MT_U64)KerAddr, *KerAddr);

		//reg_set_trpp_ch11_lln_rd_byte_set(RecId, lln_read_num>>3);
		*p_node_index = RecInfo->node_index;		

		RecInfo->read_num[RecInfo->node_index] += lln_read_num;
		if (RecInfo->read_num[RecInfo->node_index] >= RecInfo->link_rec_buf[RecInfo->node_index].size)//all bytes on this node have been read 
		{
			if ((RecInfo->node_index >= 0) && (RecInfo->node_index <=3))
			{
				reg_value = reg_get_trpp_ch11_lln_full_0(RecId);
				reg_value &= (~(1<<(RecInfo->node_index*8)));
				MT_INFO_DEMUX("[%s %d]reg_value=0x%x\n", __FUNCTION__, __LINE__, reg_value);
				reg_set_trpp_ch11_lln_full_0(RecId, reg_value);
			}
			else if ((RecInfo->node_index >= 4) && (RecInfo->node_index <=7))
			{
				reg_value = reg_get_trpp_ch11_lln_full_1(RecId);
				reg_value &= (~(1<<((RecInfo->node_index-4)*8)));
				MT_INFO_DEMUX("[%s %d]reg_value=0x%x\n", __FUNCTION__, __LINE__, reg_value);
				reg_set_trpp_ch11_lln_full_1(RecId, reg_value);
			}
			
			RecInfo->read_num[RecInfo->node_index] = 0;//set current node_index's num to 0
			RecInfo->node_index++;
			if (RecInfo->node_index == RecInfo->link_node_num)
			{
				RecInfo->node_index = 0;
			}
			RecInfo->read_num[RecInfo->node_index] = 0;//set next node_index's num to 0
		}
		return MT_SUCCESS;
    }
    return MT_ERR_DMX_TIMEOUT;
}

static void push_overflow(DMX_RecInfo_S *RecInfo,mt_u32 cnt,mt_u64 s, mt_u64 e)
{
    if(RecInfo){
        mt_u32 wp=RecInfo->overflowinfo.wp;
        RecInfo->overflowinfo.overflow_allcnt=cnt;
        RecInfo->overflowinfo.overflow_times[wp]=cnt;
        RecInfo->overflowinfo.overflow_s[wp]=s;
        RecInfo->overflowinfo.overflow_e[wp]=e;
        RecInfo->overflowinfo.wp=((wp+1)%DMX_OVERFLOW_MAX);
    }
}

mt_s32 DMX_DRV_REC_ReleaseRecData(mt_u32 RecId, phys_addr_t PhyAddr, mt_u32 Len)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];

    CHECKDMXID(RecInfo->DmxId);

    if (DMX_REC_STATUS_START != RecInfo->RecStatus)
    {
        MT_ERR_DEMUX("start rec channel first  !");
        return MT_ERR_DMX_NOT_START_REC_CHAN;
    }

    if (Len > 0)
    {
        u32 rec_tsbuffer_size = RecInfo->recbuf_size;
        u32 Read = reg_get_trpp_ch_rec_rd_addr_trpp_ch_rec_raddr(RecId);
        u32 overflow_tmp_cnt = 0;
        u32 diff = 0;
        u64 allrecord = 0;

        reg_set_trpp_ch_rec_rd_addr_trpp_ch_rec_raddr(RecId, ((Read + Len) % rec_tsbuffer_size));

		allrecord = reg_get_trpp_ch11_ini_info2(RecId);		
		allrecord *= 188;	
        RecInfo->rp_globe += Len;
        		
        if (allrecord >= RecInfo->rp_globe)
        {
            diff = (u32)(allrecord - RecInfo->rp_globe);
            overflow_tmp_cnt = (diff / rec_tsbuffer_size);

            if (overflow_tmp_cnt > 0)
            {
                mt_u32 discard_len = overflow_tmp_cnt*rec_tsbuffer_size;
                RecInfo->overflow_cnt += overflow_tmp_cnt;
                push_overflow(RecInfo, RecInfo->overflow_cnt, RecInfo->rp_globe,
                    (RecInfo->rp_globe+discard_len));
                MT_ERR_DEMUX("++k.overflow0 %llx,%llx\n", allrecord, RecInfo->rp_globe);
                MT_ERR_DEMUX("++k.overflow1:%x,%x,%llx,%llx\n",
                    RecInfo->overflow_cnt, overflow_tmp_cnt, RecInfo->rp_globe, (RecInfo->rp_globe+discard_len));
                RecInfo->rp_globe += discard_len;
            }
        }
        else
        {
            MT_ERR_DEMUX("overtest: RecId=%d, allrecord = 0x%llx, rp_globe = 0x%llx, Len = 0x%x\n",
                RecId, allrecord, RecInfo->rp_globe, Len);
            MT_ERR_DEMUX("overtest: RecId=%d, rec_tsbuffer_size = 0x%x\n",
                RecId, rec_tsbuffer_size);
        }
    }

    return MT_SUCCESS;
}

mt_s32 DMX_DRV_LinkREC_ReleaseRecData(mt_u32 RecId, phys_addr_t PhyAddr, mt_u32 Len)
{
	reg_set_trpp_ch11_lln_rd_byte_set(RecId, Len>>3);
	return MT_SUCCESS;
}

static RET_CODE dmx_rec_get64(void* vaddr, ulong rp, mt_u32 off, ulong wp, mt_u32 size, mt_u64 *data)
{
    int i;
    mt_u64 idx_data;
    int len;

    BEGIN:
    if(wp >= rp){
        len = wp - rp;
    }else{
        len = (size - rp) + wp;
    }

    if(len < 8 || data == NULL || off >= len)
        return MT_FAILURE;

    if(off > 0){
        if(rp + off >= size){
            rp = off - (size - rp);
        }else{
            rp += off;
        }
        off = 0;
        goto BEGIN;
    }

    if(size - rp >= 8){
        idx_data = *((mt_u64 *)(rp + vaddr));
    }else{
        idx_data = 0;
        for(i = 0; i<size - rp; i++)
            idx_data = (idx_data << 8) | *((mt_u8 *)(rp + i+ vaddr));
        for(i = 0; i<(8-(size-rp)); i++)
            idx_data = (idx_data << 8) | *((mt_u8 *)(i + vaddr));
    }
    *data = idx_data;
    return MT_SUCCESS;
}

static RET_CODE dmx_rec_convert_byte_order(char* data, u32 bcnt)
{
    u32 i;
    char tmp;
    for(i=0; i<(bcnt >> 1); i++){
        tmp = *(data + i);
        *(data + i) = *(data + bcnt - 1 -i);
        *(data + bcnt - 1 -i) =  tmp;
    }
    return MT_SUCCESS;
}


static mt_u32 dmx_rec_get_remain_stc(mt_u32 pid,mt_u8 *ts_align,mt_u8 *ts_end,mt_u8 *buf, mt_u32 need)
{
    mt_u8 *ts_head=ts_align;
    mt_u32 pid_loop=0x1fff;
    mt_u32 adaptation_control=0;
    mt_u32 skip=0;
    mt_u32 cpyed=0;
    mt_u32 tmpcpyed=0;

    while(ts_head < ts_end){
        if(0x47 != ts_head[0]){
            //printk("+++drv.stc out1\n");
            break;
        }
        pid_loop=ts_head[1];
        pid_loop=(0x1fff&((pid_loop<<8)|ts_head[2]));
        if(pid_loop != pid){                    //no this pid
            ts_head+=188;
            continue;
        }
        adaptation_control=((ts_head[3]>>4)&0x03);
        if(0x01&adaptation_control){
            if(1==adaptation_control){          //no adapt    .only head.4
                skip=4;
                memcpy(buf+cpyed,ts_head+skip,need);
                //dmx_x_dumpts("snd1:",buf+cpyed,need);
                return need;
            }else if(3==adaptation_control){    //after atapt .head.4+len+len.data
                skip=(4+1+ts_head[4]);
                if(skip+need <= 188){
                    memcpy(buf+cpyed,ts_head+skip,need);
                    //dmx_x_dumpts("snd2:",buf+cpyed,need);
                    return need;
                }else{
                    tmpcpyed=(188-skip);
                    memcpy(buf+cpyed,ts_head+skip,tmpcpyed);
                    //dmx_x_dumpts("snd3:",buf+cpyed,need);
                    cpyed+=tmpcpyed;
                    need -=tmpcpyed;
                }
            }
        }
        ts_head+=188;
    }
    return cpyed;
}
#define VC1_CPYLEN  12
#define NORMAL_CPYLEN  8
static RET_CODE mdx_rec_softfilter_stc(u32 vtype,u32 stc)
{//soft filter.while double-rec,maybe other startcode
    if(MT_UNF_VCODEC_TYPE_MPEG2==vtype){
        if((stc!=0x1b3) && (stc!=0x100)){
            return -1;
        }
    }else if(MT_UNF_VCODEC_TYPE_AVS==vtype){
        if((stc!=0x1b3) && (stc!=0x1b0) && (stc!=0x1b6)){
            return -1;
        }
    }else if(MT_UNF_VCODEC_TYPE_MPEG4==vtype){
        if((stc!=0x1b0) && (stc!=0x1b6)){
            return -1;
        }
    }else if(MT_UNF_VCODEC_TYPE_VC1==vtype){
        if((stc!=0x10d) && (stc!=0x10f)){
            return -1;
        }
    }else if(MT_UNF_VCODEC_TYPE_H264==vtype){ //1.2.5.
        MT_U32 T5;
        if((stc&0xffffff80)!=0x00000100){     //high1.=0
            return -1;
        }
        T5=(stc&0x1f);
        if((T5!=1) && (T5!=5) && (T5!=7)){  //low5=1 5 7
            return -1;
        }
    }else if(MT_UNF_VCODEC_TYPE_HEVC==vtype){ //1.6.1
        MT_U32 T6;
        if((stc&0xffffff80)!=0x00000100){     //high1.=0
            return -1;
        }
        T6=((stc>>1)&0x3f);
        if(!(((0<=T6)&&(9>=T6))||((16<=T6)&&(32>=T6)))){    //mid6=0-9 16-32
            return -1;
        }
    }else{
        return -1;
    }
    return 0;
}

static RET_CODE dmx_rec_get_index_data(u32 vtype, u8 rec_id, u8 index, void *addr, u32 *chn_num,u32 *thiscode)
{
    u32 wp = reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(index);
    u32 rp = reg_get_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(index);
    u32 rec_indexbuffer_startPoint = reg_get_trpp_ch_idx_start_addr_trpp_ch_idx_saddr(index) << 3;
    u32 rec_indexbuffer_endPoint = reg_get_trpp_ch_idx_end_addr_trpp_ch_idx_eaddr(index);
    u32 rec_indexbuffer_size = rec_indexbuffer_endPoint - rec_indexbuffer_startPoint + 1;
    u32 len=0,flag_moverp=0;
    u32 dsaddr,deaddr,dbuf_size;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[rec_id];
    reg_trpp_ch_idx_data_t idx_data;
    mt_u32 rd_len = 0;
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
    dsaddr = reg_get_trpp_ch_rec_start_addr_trpp_ch_rec_saddr(rec_id) << 3;
    deaddr = reg_get_trpp_ch_rec_end_addr_trpp_ch_rec_eaddr(rec_id);
    dbuf_size = deaddr - dsaddr + 1;
    *chn_num = 0;

    //u32 datawp=reg_get_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(index);

    if(wp >= rp)
	{
        len = wp - rp;
    }
	else
	{
        len = ( rec_indexbuffer_size - rp ) + wp;
    }
	
    *thiscode=0xffffffff;
    if(len >=8 && (len & 0x07) == 0){
        ret = dmx_rec_get64(RecInfo->RecIdxBuffer.startVirAddr, rp, 0, wp, rec_indexbuffer_size, (mt_u64 *)&idx_data);
        if(ret != MT_SUCCESS){
            return MT_ERR_DMX_NOAVAILABLE_DATA;
        }
        *thiscode=idx_data.dts.idx_num;
        switch(idx_data.dts.idx_num)
        {
            case TRPP_INDEX_DTS:
                rd_len = 8;
                ret = MT_ERR_DMX_NOAVAILABLE_DATA;
                flag_moverp=1;
            	break;
				
            case TRPP_INDEX_PTS:
                if(len >=16){
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset = idx_data.pts.rec_ch_num ;
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset *= 188;
                    i = idx_data.pts.rec_ch_num;
                    *chn_num = idx_data.pts.rec_ch_num;
                    /* the first frame size may be error */
                    //((MT_UNF_DMX_REC_INDEX_S *)addr)->u32FrameSize = ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset - *reg_off;
                    ret = dmx_rec_get64(RecInfo->RecIdxBuffer.startVirAddr, rp, 8, wp, rec_indexbuffer_size, (mt_u64 *)&idx_data);
                    if(ret != MT_SUCCESS){
                        rd_len = len;
                        ret = MT_ERR_DMX_NOAVAILABLE_DATA;
                        break;
                    }
                }else{
                    rd_len = len;
                    ret = MT_ERR_DMX_NOAVAILABLE_DATA;
                    break;
                }
				
                if(idx_data.dts.idx_num == TRPP_INDEX_DTS && idx_data.dts.pts_vld && idx_data.dts.pts){
                    mt_u32 pts32;
                    pts32 = idx_data.dts.pts >> 1;
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u32PtsMs = pts32/45;
                }
                rd_len = 16;
                flag_moverp=1;
            	break;
				
            case TRPP_INDEX_ES_SC:
                {
                    mt_u8 *rdaddr;
                    mt_u32 pnum;
                    mt_u8 poff;
		    		mt_u8 *u8ptr = NULL;
                    mt_u32 firstcpy=0;
		    		mt_u32 cpymax=0;

		    		*chn_num = idx_data.es_sc.rec_ch_num;
                    pnum = idx_data.es_sc.rec_ch_num%(dbuf_size/188);
                    poff = idx_data.es_sc.sc_offset;
                    rdaddr = (mt_u8 *)(RecInfo->RecBuffer.startVirAddr + pnum*188 + poff);
                    #if 0
                    //printk("num=%x,off=%x,%x,%x\n",idx_data.es_sc.rec_ch_num,idx_data.es_sc.sc_offset,dbuf_size,*((mt_u8 *)(RecInfo->RecBuffer.startVirAddr + pnum*188)));
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[0] = *((mt_u32 *)rdaddr);
                    (mt_void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[0]), 4);
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[1] = *((mt_u32 *)rdaddr + 1);
                    (mt_void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[1]), 4);
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset = idx_data.es_sc.rec_ch_num;
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset *= 188;
                    //((MT_UNF_DMX_REC_INDEX_S *)addr)->u32FrameSize = ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset - *reg_off;
                    //*reg_off=((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset;
                    //if(((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[0]&0xffff0000){
                    //printk("0:rp=%x,wp=%x,sz=%x,%x,%x\n",rp,wp,rec_indexbuffer_size,datawp,pnum*188);
                    //dmx_spes_dumpts((mt_u8 *)(RecInfo->RecBuffer.startVirAddr + pnum*188),188);
                    //}
                    //printk("\n%s_%d,chn=%d,dsize=%d,pnum=%d\n",__func__,__LINE__,*chn_num,dbuf_size,pnum);
                    printk("++stc=%llx,%x,%x__%x\n",((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset,poff,*((mt_u32 *)rdaddr), *((mt_u32 *)rdaddr + 1));
                    #else //bug115099.startcode cross 2 or more ts packts. 80 00 00 00 47 00 BF 1E 00 01 61 88 E4 68 54 96
                    u8ptr = (mt_u8 *)(((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData);
                    firstcpy=(188-poff);       //poff=[0,187]
                    cpymax=NORMAL_CPYLEN;

		    		if(MT_UNF_VCODEC_TYPE_VC1==vtype){
                        cpymax=VC1_CPYLEN;
                    }
					
                    if(cpymax < firstcpy){
                        firstcpy = cpymax;
                    }
					
                    memcpy(u8ptr, rdaddr, firstcpy);
                    if(cpymax > firstcpy){
                        mt_s32 cpyed=0;
                        mt_u32 pid=0x1fff;
                        ulong tsend=(ulong)RecInfo->RecBuffer.startVirAddr+dbuf_size;
                        //dmx_x_dumpts("first:",u8ptr,firstcpy);
                        rdaddr = (mt_u8 *)(RecInfo->RecBuffer.startVirAddr + pnum*188);
                        pid = rdaddr[1];
                        pid = (0x1fff&((pid<<8) | rdaddr[2]));
                        //printk("+++do second:%x,%x\n",pnum*188+188,dbuf_size);
                        cpyed = dmx_rec_get_remain_stc(pid,rdaddr+188,(mt_u8*)tsend,u8ptr+firstcpy,cpymax-firstcpy); //check from next ts-packet
                        if(cpyed != (cpymax-firstcpy)){  //must be ts-reloop,so check from buff_head
                            //printk("+++do third\n");
                            dmx_rec_get_remain_stc(pid,(mt_u8*)RecInfo->RecBuffer.startVirAddr,rdaddr,u8ptr+firstcpy+cpyed,cpymax-firstcpy-cpyed);
                        }
                    }
					
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset = idx_data.es_sc.rec_ch_num;
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset *= 188;
                    if(MT_UNF_VCODEC_TYPE_MPEG2 == vtype){  //mpg2.2frames share 1tspacket
                        ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset+=poff;
                    }
					
                    (mt_void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[0]), 4);
                    (mt_void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[1]), 4);
                    if(MT_UNF_VCODEC_TYPE_VC1==vtype){
                        (mt_void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_S *)addr)->u32BufOwner), 4);     //reuse it
                    }
                    //printk("++stc=%x,%x_%x_%x\n",poff,((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[0],((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[1],((MT_UNF_DMX_REC_INDEX_S *)addr)->u32BufOwner);
                    #endif
                    rd_len = 8;
                    flag_moverp=1;
                }
            	break;
			
            default:
                rd_len = 8;
                ret = MT_ERR_DMX_NOAVAILABLE_DATA;
                flag_moverp=1;
            	break;
        }
    }
	else
	{
        rd_len =  len & 0x07;
        ret = MT_ERR_DMX_NOAVAILABLE_DATA;
    }

    if(1==flag_moverp)
	{
        if(rec_indexbuffer_size - rp > rd_len)
		{
            wp = rp + rd_len;
        }
		else
		{
            wp = rd_len - (rec_indexbuffer_size - rp);
        }
        //printk("\n%s_%d,%d,%d\n",__func__,__LINE__,wp,reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(index));
        //printk("dmx_symphony_rec_get_index_data wp=%x, rp=%x,len=%d,itype0=%llx,itype1=%llx\n",wp,rp,len,*((u64 *)addr),*((u64 *)addr + 1));
        reg_set_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(index, wp);
    }
    return ret;
}

static RET_CODE dmx_linkrec_get_index_data(u32 vtype, u8 rec_id, u8 index, void *addr, u32 *chn_num,u32 *thiscode)
{
    u32 wp = reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(index);
    u32 rp = reg_get_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(index);
    u32 rec_indexbuffer_startPoint = reg_get_trpp_ch_idx_start_addr_trpp_ch_idx_saddr(index) << 3;
    u32 rec_indexbuffer_endPoint = reg_get_trpp_ch_idx_end_addr_trpp_ch_idx_eaddr(index);
    u32 rec_indexbuffer_size = rec_indexbuffer_endPoint - rec_indexbuffer_startPoint + 1;
    u32 len=0,flag_moverp=0;
    u32 dbuf_size;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[rec_id];
    reg_trpp_ch_idx_data_t idx_data;
    mt_u32 rd_len = 0;
    mt_s32 ret = MT_SUCCESS;
    mt_u32 i;
	mt_u8 node_index = 0;
	mt_u32 node_offset;
	mt_u32 pre_node_data = 0;
	mt_u8 *rdaddr, *p_temp_addr;
    mt_u32 pnum;//the packet num in record channel
    mt_u8 poff;	//the start code offset in a ts-packet
	mt_u8 *u8ptr = NULL;
    mt_u32 firstcpy=0;
	mt_u32 cpymax=0;
	mt_s32 cpyed=0;
    mt_u32 pid=0x1fff;
	ulong tsend = 0;

	//printk("[%s %d]index=%d, enter\n", __FUNCTION__, __LINE__, index);
	dbuf_size = RecInfo->link_rec_buf[0].size * 8 * RecInfo->link_node_num;
    *chn_num = 0;

    if (wp >= rp)
	{
        len = wp - rp;
    }
	else
	{
        len = (rec_indexbuffer_size - rp) + wp;
    }
	
    *thiscode=0xffffffff;
    if (len >=8 && (len & 0x07) == 0)
	{
        ret = dmx_rec_get64(RecInfo->RecIdxBuffer.startVirAddr, rp, 0, wp, rec_indexbuffer_size, (mt_u64 *)&idx_data);
        if (ret != MT_SUCCESS)
		{
			//printk("[%s %d]index=%d, no data\n", __FUNCTION__, __LINE__, index);
            return MT_ERR_DMX_NOAVAILABLE_DATA;
        }
		
        *thiscode=idx_data.dts.idx_num;
        switch(idx_data.dts.idx_num)
        {
            case TRPP_INDEX_DTS:
                rd_len = 8;
                ret = MT_ERR_DMX_NOAVAILABLE_DATA;
                flag_moverp=1;
            	break;
				
            case TRPP_INDEX_PTS:
                if (len >=16)
				{
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset = idx_data.pts.rec_ch_num ;
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset *= 188;
                    i = idx_data.pts.rec_ch_num;
                    *chn_num = idx_data.pts.rec_ch_num;
                    /* the first frame size may be error */
                    //((MT_UNF_DMX_REC_INDEX_S *)addr)->u32FrameSize = ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset - *reg_off;
                    ret = dmx_rec_get64(RecInfo->RecIdxBuffer.startVirAddr, rp, 8, wp, rec_indexbuffer_size, (mt_u64 *)&idx_data);
                    if (ret != MT_SUCCESS)
					{
                        rd_len = len;
                        ret = MT_ERR_DMX_NOAVAILABLE_DATA;
                        break;
                    }
                }
				else
				{
                    rd_len = len;
                    ret = MT_ERR_DMX_NOAVAILABLE_DATA;
                    break;
                }
				
                if (idx_data.dts.idx_num == TRPP_INDEX_DTS && idx_data.dts.pts_vld && idx_data.dts.pts)
				{
                    mt_u32 pts32;
                    pts32 = idx_data.dts.pts >> 1;
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u32PtsMs = pts32/45;
                }
                rd_len = 16;
                flag_moverp=1;
            	break;
				
            case TRPP_INDEX_ES_SC:
                {                    
		    		*chn_num = idx_data.es_sc.rec_ch_num;//the packet num in record channel
                    pnum = idx_data.es_sc.rec_ch_num%(dbuf_size/188);
                    poff = idx_data.es_sc.sc_offset;

					//fisrt calculate how many nodes, then mod RecInfo->link_node_num to find out node_index
					node_index = ((pnum*188 + poff)/RecInfo->link_rec_buf[0].size)%RecInfo->link_node_num;
					node_offset = (pnum*188 + poff)%RecInfo->link_rec_buf[0].size;				

					MT_INFO_DEMUX("[%s %d]pnum=%d, poff=%d, pnum*188 + poff=%d\n", __FUNCTION__, __LINE__, pnum, poff, pnum*188 + poff);
					MT_INFO_DEMUX("[%s %d]index=%d, node_index=%d, node_offset=%d\n", __FUNCTION__, __LINE__, index, node_index, node_offset);
                    rdaddr = (mt_u8 *)(RecInfo->link_rec_buf[node_index].startVirAddr + node_offset);
           
                    u8ptr = (mt_u8 *)(((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData);
                    firstcpy=(188-poff);       //poff=[0,187]
                    cpymax=NORMAL_CPYLEN;

		    		if (MT_UNF_VCODEC_TYPE_VC1==vtype)
					{
                        cpymax=VC1_CPYLEN;
                    }
					
                    if (cpymax < firstcpy)
					{
                        firstcpy = cpymax;
                    }

					if ((node_offset + firstcpy) > RecInfo->link_rec_buf[0].size)//the data in different node
					{
						MT_INFO_DEMUX("[%s %d]in different node\n", __FUNCTION__, __LINE__);
						pre_node_data = RecInfo->link_rec_buf[0].size - node_offset;
						memcpy(u8ptr, rdaddr, pre_node_data);

						p_temp_addr = (mt_u8 *)(RecInfo->link_rec_buf[node_index+1].startVirAddr);
						memcpy(u8ptr + pre_node_data, p_temp_addr, firstcpy - pre_node_data);
					}
					else
					{
                    	memcpy(u8ptr, rdaddr, firstcpy);
					}
					
                    if (cpymax > firstcpy)
					{                        													
                        tsend = (ulong)RecInfo->link_rec_buf[node_index].startVirAddr + RecInfo->link_rec_buf[0].size;

                        rdaddr = (mt_u8 *)(RecInfo->link_rec_buf[node_index].startVirAddr + (pnum*188%RecInfo->link_rec_buf[0].size));
                        pid = rdaddr[1];
                        pid = (0x1fff&((pid<<8) | rdaddr[2]));
                        if ((ulong)(rdaddr+188) >= tsend)
                        {
                        	MT_INFO_DEMUX("[%s %d]in different node\n", __FUNCTION__, __LINE__);
                        	//check from next ts-packet, and next ts-packet in the next node
                        	p_temp_addr = (mt_u8 *)(RecInfo->link_rec_buf[node_index+1].startVirAddr);
							tsend = (ulong)RecInfo->link_rec_buf[node_index+1].startVirAddr + RecInfo->link_rec_buf[0].size;
							MT_INFO_DEMUX("[%s %d]p_temp_addr=0x%lx, tsend=0x%lx\n", __FUNCTION__, __LINE__, (ulong)p_temp_addr, tsend);
                        	cpyed = dmx_rec_get_remain_stc(pid, p_temp_addr, (mt_u8*)tsend, u8ptr+firstcpy, cpymax-firstcpy); 
                        }
						else
						{
                        	cpyed = dmx_rec_get_remain_stc(pid, rdaddr+188, (mt_u8*)tsend, u8ptr+firstcpy, cpymax-firstcpy); //check from next ts-packet
						}
						
                        if(cpyed != (cpymax-firstcpy))
						{  //must be ts-reloop,so check from buff_head
                            dmx_rec_get_remain_stc(pid, (mt_u8*)RecInfo->link_rec_buf[node_index].startVirAddr, rdaddr, u8ptr+firstcpy+cpyed, cpymax-firstcpy-cpyed);
                        }
                    }
					
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset = idx_data.es_sc.rec_ch_num;
                    ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset *= 188;
                    if (MT_UNF_VCODEC_TYPE_MPEG2 == vtype)
					{  //mpg2.2frames share 1tspacket
                        ((MT_UNF_DMX_REC_INDEX_S *)addr)->u64GlobalOffset+=poff;
                    }
					
                    (mt_void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[0]), 4);
                    (mt_void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_S *)addr)->u32HdrData[1]), 4);
                    if (MT_UNF_VCODEC_TYPE_VC1==vtype)
					{
                        (mt_void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_S *)addr)->u32BufOwner), 4);     //reuse it
                    }
                   
                    rd_len = 8;
                    flag_moverp=1;
                }
            	break;
			
            default:
                rd_len = 8;
                ret = MT_ERR_DMX_NOAVAILABLE_DATA;
                flag_moverp=1;
            	break;
        }
    }
	else
	{
        rd_len =  len & 0x07;
        ret = MT_ERR_DMX_NOAVAILABLE_DATA;
    }

    if (1==flag_moverp)
	{
        if(rec_indexbuffer_size - rp > rd_len)
		{
            wp = rp + rd_len;
        }
		else
		{
            wp = rd_len - (rec_indexbuffer_size - rp);
        }
        //printk("\n%s_%d,%d,%d\n",__func__,__LINE__,wp,reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(index));
        //printk("dmx_symphony_rec_get_index_data wp=%x, rp=%x,len=%d,itype0=%llx,itype1=%llx\n",wp,rp,len,*((u64 *)addr),*((u64 *)addr + 1));
        reg_set_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(index, wp);
    }
    return ret;
}

static VID_STD_E CodecTypeUnfToFmw(MT_UNF_VCODEC_TYPE_E unfType)
{
    switch (unfType) {
    case MT_UNF_VCODEC_TYPE_MPEG2:
	return STD_MPEG2;
    case MT_UNF_VCODEC_TYPE_MPEG4:
	return STD_MPEG4;
    case MT_UNF_VCODEC_TYPE_AVS:
	return STD_AVS;
    case MT_UNF_VCODEC_TYPE_H263:
	return STD_H263;
    case MT_UNF_VCODEC_TYPE_VP6:
	return STD_VP6;
    case MT_UNF_VCODEC_TYPE_VP6F:
	return STD_VP6F;
    case MT_UNF_VCODEC_TYPE_VP6A:
	return STD_VP6A;
    case MT_UNF_VCODEC_TYPE_VP8:
	return STD_VP8;
    case MT_UNF_VCODEC_TYPE_SORENSON:
	return STD_SORENSON;
    case MT_UNF_VCODEC_TYPE_H264:
	return STD_H264;
    case MT_UNF_VCODEC_TYPE_HEVC:
	return STD_HEVC;
    case MT_UNF_VCODEC_TYPE_REAL9:
	return STD_REAL9;
    case MT_UNF_VCODEC_TYPE_REAL8:
	return STD_REAL8;
    case MT_UNF_VCODEC_TYPE_VC1:
	return STD_VC1;
    case MT_UNF_VCODEC_TYPE_DIVX3:
	return STD_DIVX3;
    case MT_UNF_VCODEC_TYPE_MVC:
	return STD_MVC;
    case MT_UNF_VCODEC_TYPE_RAW:
	return STD_RAW;
    case MT_UNF_VCODEC_TYPE_MJPEG:
	return STD_USER;
    default:
	return STD_END_RESERVED;
    }
}
typedef struct indexinfor
{
    mt_u64 offset;
    mt_u32 u32HdrData[2];
    mt_u32 pts;
    mt_u16 type;
    mt_u16 sps;
}INDEXINFOR;
#define TYPE_PTS 1      //pts
#define TYPE_STC 2      //startcode
#define CACHEDINDEX_NUM 3

static int x_check_get_ue(unsigned char buffer[],int totbitoffset,int *info, int bytecount)
{
    int inf = 0;
    long byteoffset = 0;      // byte from start of buffer
    int bitoffset = 0;      // bit from start of byte
    int ctr_bit = 0;      // control bit for current bit posision
    int bitcounter = 1;
    int len = 1;
    int info_bit = 0;

    byteoffset = totbitoffset >> 3;
    bitoffset = 7 - (totbitoffset & 7);
    ctr_bit = (buffer[byteoffset] & (0x01 << bitoffset));   // set up control bit

    while (ctr_bit == 0)
    {
        // find leading 1 bit
        len++;
        bitoffset -= 1;
        bitcounter++;
        if (bitoffset < 0)
        {
            // finish with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        ctr_bit = buffer[byteoffset] & (0x01 << (bitoffset));
    }
    // make infoword
    inf = 0;                          // shortest possible code is 1, then info is always 0
    for(info_bit = 0; info_bit < (len - 1); info_bit++)
    {
        bitcounter++;
        bitoffset -= 1;
        if(bitoffset < 0)
        {
            // finished with current byte ?
            bitoffset = bitoffset + 8;
            byteoffset++;
        }
        if (byteoffset > bytecount)
        {
            return -1;
        }
        inf = (inf << 1);
        if(buffer[byteoffset] & (0x01 << bitoffset))
        inf |= 1;
    }
    *info = (int)(1 << (bitcounter >> 1)) + inf - 1;

    return bitcounter;
}

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#define SYM_MAXHARD_RECNUM   7
#else
#define SYM_MAXHARD_RECNUM   4
#endif

static INDEXINFOR g_cachedindex[SYM_MAXHARD_RECNUM][CACHEDINDEX_NUM]={0};
static mt_u32 g_cachednum[SYM_MAXHARD_RECNUM]={0};
static mt_u32 g_nowpts[SYM_MAXHARD_RECNUM]={0};
static mt_u32 g_flag_265sps[SYM_MAXHARD_RECNUM]={0};
static mt_u32 g_avs_profile[SYM_MAXHARD_RECNUM]={0};

mt_s32 Dmx_Drv_Idx_GetVaddr_ByRecid(mt_u32 RecId,ulong *idxv,ulong *tsv)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
    if(4 <= RecId){
        return MT_FAILURE;
    }
    if(idxv){
        *idxv=(ulong )RecInfo->RecIdxBuffer.startVirAddr;
    }
    if(tsv){
        *tsv=(ulong )RecInfo->RecBuffer.startVirAddr;
    }
    return MT_SUCCESS;  
}
mt_s32 DMX_DRV_REC_AcquireRecIndex(mt_u32 RecId, MT_UNF_DMX_REC_INDEX_S *RecIndex, mt_u32 Timeout)
{
    mt_s32 ret1 = MT_FAILURE;
    mt_u32 thiscode=0xffffffff;
    //mt_u32 nextcode=0xffffffff;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
    MT_BOOL bUseTimeStamp = MT_FALSE;
    mt_u32 chn_num0 = 0;
    mt_u8  rec_idx;
    
    CHECKDMXID(RecInfo->DmxId);
	
    if(DMX_REC_CNT <= RecId){
        return MT_ERR_DMX_INVALID_PARA;
    }

	if (RecId >= 4)//record channel 4-6 use 8-11 for index channel
	{
		rec_idx = RecId + 4;
	}
	else
	{
		rec_idx = RecId;
	}
	
	//printk("[%s %d]recid=%d, RecIndex->u32DataTimeMs=%d, RecInfo->IndexChanID=0x%x\n", __FUNCTION__, __LINE__, RecId, RecIndex->u32DataTimeMs, RecInfo->IndexChanID);	
    if(1==RecIndex->u32DataTimeMs){     //change programme , init it
        memset(g_cachedindex[RecId],0x00,sizeof(INDEXINFOR)*CACHEDINDEX_NUM);
        g_cachednum[RecId]=0;
        g_nowpts[RecId]=0;
        g_flag_265sps[RecId]=0;
        g_avs_profile[RecId]=0;
    }

    if (DMX_REC_STATUS_START != RecInfo->RecStatus) {
		MT_ERR_DEMUX("start rec channel first!");
		return MT_ERR_DMX_NOT_START_REC_CHAN;
    }

    if (MT_UNF_DMX_REC_INDEX_TYPE_NONE == RecInfo->IndexType) {
		return MT_ERR_DMX_NOAVAILABLE_DATA;
    }

    if (RecInfo->enRecTimeStamp >= DMX_REC_TIMESTAMP_ZERO) {
		bUseTimeStamp = MT_TRUE;
    }

    if (DMX_INVALID_CHAN_ID == RecInfo->IndexChanID) {
        ret1 = DMX_OsiGetChannelId(RecInfo->DmxId, RecInfo->IndexPid, MT_UNF_DMX_CHAN_TYPE_REC, &RecInfo->IndexChanID);
		if (MT_SUCCESS != ret1) {
	            MT_ERR_DEMUX("DMX_OsiGetChannelId failed, demux id: %d,  IndexPid: %d ,ret : 0x%x\n", RecInfo->DmxId, RecInfo->IndexPid, ret1);
		    return ret1;
		}
    }

    #if 0       //change for h264 multi-slice
    for(;;){    //first to get ptsdts
        ret1 = dmx_rec_get_index_data(NULL, RecInfo->DmxId, (void *)RecIndex, &chn_num0,&thiscode);
        //printk("0:%s_%d,%x,pts=%x_fs=%x_goff=%llx_h0=%x_h1=%x,%x\n",__func__,__LINE__,ret1,RecIndex->u32PtsMs,
        //                RecIndex->u32FrameSize,RecIndex->u64GlobalOffset,RecIndex->u32HdrData[0],RecIndex->u32HdrData[1],thiscode);
        if(MT_SUCCESS != ret1){
            return MT_ERR_DMX_TIMEOUT;
        }
        if(TRPP_INDEX_PTS==thiscode){
            break;
        }
    }
    ret2=dmx_rec_get_index_data(NULL, RecInfo->DmxId, (void *)RecIndex, &chn_num0,&nextcode);
    //printk("1:%s_%d,%x,pts=%x_fs=%x_goff=%llx_h0=%x_h1=%x,%x\n",__func__,__LINE__,ret1,RecIndex->u32PtsMs,
    //                RecIndex->u32FrameSize,RecIndex->u64GlobalOffset,RecIndex->u32HdrData[0],RecIndex->u32HdrData[1],nextcode);
    if(MT_SUCCESS!=ret2){   //first error!
        msleep(20);         //udelay(500);
        ret2=dmx_rec_get_index_data(NULL, RecInfo->DmxId, (void *)RecIndex, &chn_num0,&nextcode);
        //printk("2:%s_%d,%x,pts=%x_fs=%x_goff=%llx_h0=%x_h1=%x,%x\n",__func__,__LINE__,ret1,RecIndex->u32PtsMs,
        //                RecIndex->u32FrameSize,RecIndex->u64GlobalOffset,RecIndex->u32HdrData[0],RecIndex->u32HdrData[1],nextcode);
    }
    if((MT_UNF_VCODEC_TYPE_AVS==RecInfo->VCodecType)&&
       ((MT_SUCCESS==ret2)&&(TRPP_INDEX_ES_SC==nextcode))&&
       (RecIndex->u32HdrData[0]==0x1b0)){   //this is squence-header.to recieve another one!
        avs_profile=((RecIndex->u32HdrData[1]>>24) & 0xff);
        ret2=dmx_rec_get_index_data(NULL, RecInfo->DmxId, (void *)RecIndex, &chn_num0,&nextcode);
        //printk("3:%s_%d,%x,pts=%x_fs=%x_goff=%llx_h0=%x_h1=%x,%x\n",__func__,__LINE__,ret1,RecIndex->u32PtsMs,
        //                RecIndex->u32FrameSize,RecIndex->u64GlobalOffset,RecIndex->u32HdrData[0],RecIndex->u32HdrData[1],nextcode);
        if(MT_SUCCESS!=ret2){   //first error!
            msleep(20);         //udelay(500);
            ret2=dmx_rec_get_index_data(NULL, RecInfo->DmxId, (void *)RecIndex, &chn_num0,&nextcode);
            //printk("4:%s_%d,%x,pts=%x_fs=%x_goff=%llx_h0=%x_h1=%x,%x\n",__func__,__LINE__,ret1,RecIndex->u32PtsMs,
            //                RecIndex->u32FrameSize,RecIndex->u64GlobalOffset,RecIndex->u32HdrData[0],RecIndex->u32HdrData[1],nextcode);
        }
    }
    if((MT_SUCCESS==ret2)&&(TRPP_INDEX_ES_SC==nextcode)){
        if(MT_UNF_VCODEC_TYPE_AVS==RecInfo->VCodecType){
            RecIndex->u32PrivatePara=(mt_u32)avs_profile;
        }
        RecIndex->CodecType = (mt_u32)CodecTypeUnfToFmw(RecInfo->VCodecType);
        return MT_SUCCESS;
    }else{
        //printk("+++timeout\n");
        return MT_ERR_DMX_TIMEOUT;
    }
    #elif 1     //cache[3].
    REDEAL:
    for(;;){
        if(CACHEDINDEX_NUM==g_cachednum[RecId]){
            break;
        }
        ret1 = dmx_rec_get_index_data(RecInfo->VCodecType, RecId, rec_idx, (void *)RecIndex, &chn_num0,&thiscode);
        MT_INFO_DEMUX("[%s %d]ret1=%d, recid=%x,pts=%x,fs=%x,goff=%llx,h0=%x,h1=%x,%x\n", __FUNCTION__, __LINE__, ret1, RecId, RecIndex->u32PtsMs,
                        RecIndex->u32FrameSize, RecIndex->u64GlobalOffset, RecIndex->u32HdrData[0], RecIndex->u32HdrData[1], thiscode);
        if(MT_SUCCESS != ret1){
            if(g_cachednum[RecId]<CACHEDINDEX_NUM){        //return bad
                return MT_ERR_DMX_TIMEOUT;
            }else{                                              //deal cached
                break;
            }
        }
        //deal sequnce header begin
        if(TRPP_INDEX_ES_SC==thiscode){
            //soft filter.while double-rec,maybe other startcode
            RET_CODE ret_sflt=mdx_rec_softfilter_stc(RecInfo->VCodecType,RecIndex->u32HdrData[0]);
            if(0 != ret_sflt){
                continue;
            }
            
            if((((MT_UNF_VCODEC_TYPE_MPEG4==RecInfo->VCodecType)||
                 (MT_UNF_VCODEC_TYPE_AVS==RecInfo->VCodecType))&&
                      (RecIndex->u32HdrData[0]==0x1b0))||
                     ((MT_UNF_VCODEC_TYPE_MPEG2==RecInfo->VCodecType)&&
                      (RecIndex->u32HdrData[0]==0x1b3))||
                      ((MT_UNF_VCODEC_TYPE_HEVC==RecInfo->VCodecType)&&
                     ((RecIndex->u32HdrData[0]&0xffffff7e)==0x140))||     //1.6.1
                     ((MT_UNF_VCODEC_TYPE_VC1==RecInfo->VCodecType)&&
                     (RecIndex->u32HdrData[0]==0x10f))||
                     ((MT_UNF_VCODEC_TYPE_H264==RecInfo->VCodecType)&&
                     ((RecIndex->u32HdrData[0]&0xffffff1f)==0x107))){     //1.2.5 sequnce header
                g_cachedindex[RecId][g_cachednum[RecId]].pts=g_nowpts[RecId];
                g_cachedindex[RecId][g_cachednum[RecId]].type=TYPE_PTS;
                g_cachedindex[RecId][g_cachednum[RecId]].offset=RecIndex->u64GlobalOffset;
                g_cachednum[RecId]++;
                //printk("+++head:%x\n",g_nowpts[RecId]);
                if(MT_UNF_VCODEC_TYPE_AVS==RecInfo->VCodecType){
                    g_avs_profile[RecId]=((RecIndex->u32HdrData[1]>>24) & 0xff);
                }else if(MT_UNF_VCODEC_TYPE_HEVC==RecInfo->VCodecType){     //head
                    g_flag_265sps[RecId]=1;
                }else if(MT_UNF_VCODEC_TYPE_VC1==RecInfo->VCodecType){      //vc1 sequnce
                    u32 vc1_profile=((RecIndex->u32HdrData[1]>>30) & 0x03);
                    if(vc1_profile <= 1){ //simple/main
                        g_avs_profile[RecId]=RecIndex->u32HdrData[1];
                    }else{
                        g_avs_profile[RecId]=((RecIndex->u32HdrData[1]&0xffff0000)|(RecIndex->u32BufOwner>>16));  //advance.32bit is not enough
                    }
                }
                continue;
            }
        }
		
        //deal sequnce header over
        if(TRPP_INDEX_PTS==thiscode){
            g_nowpts[RecId]=RecIndex->u32PtsMs;
            continue;
        }else{
            if(MT_UNF_VCODEC_TYPE_H264==RecInfo->VCodecType){         //multislice,now don't deal
                mt_u8 slice[4] = {0};
                int offset=0,firstmbinslice=0;
                slice[0]=(0xff&(RecIndex->u32HdrData[1]>>24));
                slice[1]=(0xff&(RecIndex->u32HdrData[1]>>16));
                slice[2]=(0xff&(RecIndex->u32HdrData[1]>>8));
                slice[3]=(0xff&(RecIndex->u32HdrData[1]));
                x_check_get_ue(slice,offset,&firstmbinslice,4);
                if(0 != firstmbinslice){
                    continue;
                }
            }else if(MT_UNF_VCODEC_TYPE_HEVC==RecInfo->VCodecType){     //multislice,now don't deal
                int first_sliceseg_inpic_flag=(RecIndex->u32HdrData[1]&0x00800000);
                //printk("%x,%x\n",RecIndex->u32HdrData[1],first_sliceseg_inpic_flag);
                if(0x00800000!=first_sliceseg_inpic_flag){
                    continue;
                }
                g_cachedindex[RecId][g_cachednum[RecId]].sps=g_flag_265sps[RecId];     //the first slice after sps.
                //MT_ERR_DEMUX("+++set sps %d\n",flag_265sps);
            }
            memcpy(g_cachedindex[RecId][g_cachednum[RecId]].u32HdrData,RecIndex->u32HdrData,(sizeof(mt_u32)<<1));
            g_cachedindex[RecId][g_cachednum[RecId]].type=TYPE_STC;
            g_cachedindex[RecId][g_cachednum[RecId]].pts=g_nowpts[RecId];
            g_cachedindex[RecId][g_cachednum[RecId]].offset=RecIndex->u64GlobalOffset;
            //printk("+++stc:%x\n",g_nowpts[RecId]);
        }
        g_flag_265sps[RecId]=0;
        g_cachednum[RecId]++;
    }
    if(CACHEDINDEX_NUM==g_cachednum[RecId]){                 //cached full
        if(TYPE_STC==g_cachedindex[RecId][1].type){          //x stc x
            if(0xffffffff==g_cachedindex[RecId][1].pts){
                g_cachedindex[RecId][0]=g_cachedindex[RecId][1];
                g_cachedindex[RecId][1]=g_cachedindex[RecId][2];
                g_cachednum[RecId]--;
                goto REDEAL;
            }
            if(TYPE_STC==g_cachedindex[RecId][0].type){      //stc stc x
                RecIndex->u64GlobalOffset=g_cachedindex[RecId][1].offset;
                RecIndex->u32FrameSize=(mt_u32)(g_cachedindex[RecId][2].offset-g_cachedindex[RecId][1].offset);
                RecIndex->u32PtsMs=g_cachedindex[RecId][1].pts;
                //printk("av.nomerg=%llx,%x,%x\n",RecIndex->u64GlobalOffset,RecIndex->u32FrameSize,RecIndex->u32PtsMs);
            }else{                                                //pts stc x .first frame include pts
                RecIndex->u64GlobalOffset=g_cachedindex[RecId][0].offset;
                RecIndex->u32FrameSize=(mt_u32)(g_cachedindex[RecId][2].offset-g_cachedindex[RecId][0].offset);
                RecIndex->u32PtsMs=g_cachedindex[RecId][1].pts;
                if((MT_UNF_VCODEC_TYPE_HEVC==RecInfo->VCodecType)&&(1==g_cachedindex[RecId][1].sps)){  //hevc-patch
                    RecIndex->u32PrivatePara=1;
                }else{
                    RecIndex->u32PrivatePara=0;
                }
                //printk("av.domerg=%llx,%x,%x\n",RecIndex->u64GlobalOffset,RecIndex->u32FrameSize,RecIndex->u32PtsMs);
            }
            memcpy(RecIndex->u32HdrData,g_cachedindex[RecId][1].u32HdrData,(sizeof(mt_u32)<<1));
            //printk("++dmx.out:off=%llx,%x\n",RecIndex->u64GlobalOffset,RecIndex->u32FrameSize);
            if((MT_UNF_VCODEC_TYPE_AVS==RecInfo->VCodecType)||(MT_UNF_VCODEC_TYPE_VC1==RecInfo->VCodecType)){
                RecIndex->u32PrivatePara=(mt_u32)g_avs_profile[RecId];
            }
            RecIndex->CodecType = (mt_u32)CodecTypeUnfToFmw(RecInfo->VCodecType);
            //printk("offset=%llx,%llx,%llx\n",g_cachedindex[0].offset,g_cachedindex[1].offset,g_cachedindex[2].offset);
            //printk("\npts=%x_fs=%x_goff=%llx_h0=%x_h1=%x,Vtype=%d\n",RecIndex->u32PtsMs,
            //     RecIndex->u32FrameSize,RecIndex->u64GlobalOffset,RecIndex->u32HdrData[0],RecIndex->u32HdrData[1],RecIndex->CodecType);
            g_cachedindex[RecId][0]=g_cachedindex[RecId][1];
            g_cachedindex[RecId][1]=g_cachedindex[RecId][2];
            g_cachednum[RecId]--;
            //RecIndex->u32DataTimeMs=(jiffies*(1000/HZ));  //ms
	     RecIndex->u32DataTimeMs=ktime_to_ms(ktime_get_boottime()); //ms
            return MT_SUCCESS;
        }else{                                      //x pts x
            g_cachedindex[RecId][0]=g_cachedindex[RecId][1];
            g_cachedindex[RecId][1]=g_cachedindex[RecId][2];
            g_cachednum[RecId]--;
            goto REDEAL;
        }
    }
    #endif
    //printk("\n%s_%d,pts=%x_fs=%x_goff=%llx_h0=%x_h1=%x,Vtype=%d\n",__func__,__LINE__,RecIndex->u32PtsMs,
    //     RecIndex->u32FrameSize,RecIndex->u64GlobalOffset,RecIndex->u32HdrData[0],RecIndex->u32HdrData[1],RecIndex->CodecType);

    return ret1;//MT_ERR_DMX_TIMEOUT;
}

mt_s32 DMX_DRV_LinkREC_AcquireRecIndex(mt_u32 RecId, MT_UNF_DMX_REC_INDEX_S *RecIndex, mt_u32 Timeout)
{
    mt_s32 ret1 = MT_FAILURE;
    mt_u32 thiscode=0xffffffff;
    //mt_u32 nextcode=0xffffffff;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
    MT_BOOL bUseTimeStamp = MT_FALSE;
    mt_u32 chn_num0 = 0;
    mt_u32 rec_idx = 0;

	CHECKDMXID(RecInfo->DmxId);
	
    if (DMX_REC_CNT <= RecId)
	{
        return MT_ERR_DMX_INVALID_PARA;
    }

	if (RecId >= 4)//record channel 4-6 use 8-11 for index channel
	{
		rec_idx = RecId + 4;
	}
	else
	{
		rec_idx = RecId;
	}
	
	//printk("[%s %d]recid=%d, RecIndex->u32DataTimeMs=%d, RecInfo->IndexChanID=0x%x\n", __FUNCTION__, __LINE__, RecId, RecIndex->u32DataTimeMs, RecInfo->IndexChanID);	
    if (1==RecIndex->u32DataTimeMs)
	{     //change programme , init it
        memset(g_cachedindex[RecId],0x00,sizeof(INDEXINFOR)*CACHEDINDEX_NUM);
        g_cachednum[RecId]=0;
        g_nowpts[RecId]=0;
        g_flag_265sps[RecId]=0;
        g_avs_profile[RecId]=0;
    }

    if (DMX_REC_STATUS_START != RecInfo->RecStatus) 
	{
		MT_ERR_DEMUX("start rec channel first!");
		return MT_ERR_DMX_NOT_START_REC_CHAN;
    }

    if (MT_UNF_DMX_REC_INDEX_TYPE_NONE == RecInfo->IndexType) 
	{
		//printk("[%s %d]recid=%d, no data\n", __FUNCTION__, __LINE__, RecId);
		return MT_ERR_DMX_NOAVAILABLE_DATA;
    }

    if (RecInfo->enRecTimeStamp >= DMX_REC_TIMESTAMP_ZERO) 
	{
		bUseTimeStamp = MT_TRUE;
    }

    if (DMX_INVALID_CHAN_ID == RecInfo->IndexChanID) 
	{
        ret1 = DMX_OsiGetChannelId(RecInfo->DmxId, RecInfo->IndexPid, MT_UNF_DMX_CHAN_TYPE_REC, &RecInfo->IndexChanID);
		if (MT_SUCCESS != ret1) 
		{
	            MT_ERR_DEMUX("DMX_OsiGetChannelId failed, demux id: %d,  IndexPid: %d ,ret : 0x%x\n", RecInfo->DmxId, RecInfo->IndexPid, ret1);
		    return ret1;
		}
    }
	
REDEAL:    
    for(;;)
	{
        if(CACHEDINDEX_NUM==g_cachednum[RecId])
		{
			//printk("[%s %d]break\n", __FUNCTION__, __LINE__);
            break;
        }
		
        ret1 = dmx_linkrec_get_index_data(RecInfo->VCodecType, RecId, rec_idx, (void *)RecIndex, &chn_num0,&thiscode);
        MT_INFO_DEMUX("[%s %d]ret1=%d, recid=%x,pts=%x,fs=%x,goff=%llx,h0=%x,h1=%x,%x\n", __FUNCTION__, __LINE__, ret1, RecId, RecIndex->u32PtsMs,
                        RecIndex->u32FrameSize, RecIndex->u64GlobalOffset, RecIndex->u32HdrData[0], RecIndex->u32HdrData[1], thiscode);
        if(MT_SUCCESS != ret1)
		{
            if(g_cachednum[RecId]<CACHEDINDEX_NUM)
			{        //return bad
				//printk("[%s %d]recid=%d, no data\n", __FUNCTION__, __LINE__, RecId);
                return MT_ERR_DMX_TIMEOUT;
            }
			else
			{ //deal cached
				//printk("[%s %d]recid=%d, no data\n", __FUNCTION__, __LINE__, RecId);
                break;
            }
        }
		
        //deal sequnce header begin
        if(TRPP_INDEX_ES_SC==thiscode)
		{
            //soft filter.while double-rec,maybe other startcode
            RET_CODE ret_sflt=mdx_rec_softfilter_stc(RecInfo->VCodecType,RecIndex->u32HdrData[0]);
            if(0 != ret_sflt)
			{
                continue;
            }
            
            if((((MT_UNF_VCODEC_TYPE_MPEG4==RecInfo->VCodecType)||
                 (MT_UNF_VCODEC_TYPE_AVS==RecInfo->VCodecType))&&
                      (RecIndex->u32HdrData[0]==0x1b0))||
                     ((MT_UNF_VCODEC_TYPE_MPEG2==RecInfo->VCodecType)&&
                      (RecIndex->u32HdrData[0]==0x1b3))||
                      ((MT_UNF_VCODEC_TYPE_HEVC==RecInfo->VCodecType)&&
                     ((RecIndex->u32HdrData[0]&0xffffff7e)==0x140))||     //1.6.1
                     ((MT_UNF_VCODEC_TYPE_VC1==RecInfo->VCodecType)&&
                     (RecIndex->u32HdrData[0]==0x10f))||
                     ((MT_UNF_VCODEC_TYPE_H264==RecInfo->VCodecType)&&
                     ((RecIndex->u32HdrData[0]&0xffffff1f)==0x107)))
           {     //1.2.5 sequnce header
                g_cachedindex[RecId][g_cachednum[RecId]].pts=g_nowpts[RecId];
                g_cachedindex[RecId][g_cachednum[RecId]].type=TYPE_PTS;
                g_cachedindex[RecId][g_cachednum[RecId]].offset=RecIndex->u64GlobalOffset;
                g_cachednum[RecId]++;
                //printk("+++head:%x\n",g_nowpts[RecId]);
                if (MT_UNF_VCODEC_TYPE_AVS==RecInfo->VCodecType)
				{
                    g_avs_profile[RecId]=((RecIndex->u32HdrData[1]>>24) & 0xff);
                }
				else if (MT_UNF_VCODEC_TYPE_HEVC==RecInfo->VCodecType)
				{     //head
                    g_flag_265sps[RecId]=1;
                }
				else if (MT_UNF_VCODEC_TYPE_VC1==RecInfo->VCodecType)
				{      //vc1 sequnce
                    u32 vc1_profile=((RecIndex->u32HdrData[1]>>30) & 0x03);
                    if (vc1_profile <= 1)
					{ //simple/main
                        g_avs_profile[RecId]=RecIndex->u32HdrData[1];
                    }
					else
					{
                        g_avs_profile[RecId]=((RecIndex->u32HdrData[1]&0xffff0000)|(RecIndex->u32BufOwner>>16));  //advance.32bit is not enough
                    }
                }
                continue;
            }
        }
		
        //deal sequnce header over
        if(TRPP_INDEX_PTS==thiscode)
		{
            g_nowpts[RecId]=RecIndex->u32PtsMs;
            continue;
        }
		else
		{
            if(MT_UNF_VCODEC_TYPE_H264==RecInfo->VCodecType)
			{         //multislice,now don't deal
                mt_u8 slice[4] = {0};
                int offset=0,firstmbinslice=0;
                slice[0]=(0xff&(RecIndex->u32HdrData[1]>>24));
                slice[1]=(0xff&(RecIndex->u32HdrData[1]>>16));
                slice[2]=(0xff&(RecIndex->u32HdrData[1]>>8));
                slice[3]=(0xff&(RecIndex->u32HdrData[1]));
                x_check_get_ue(slice,offset,&firstmbinslice,4);
                if(0 != firstmbinslice)
				{
                    continue;
                }
            }
			else if(MT_UNF_VCODEC_TYPE_HEVC==RecInfo->VCodecType)
			{     //multislice,now don't deal
                int first_sliceseg_inpic_flag=(RecIndex->u32HdrData[1]&0x00800000);
                //printk("%x,%x\n",RecIndex->u32HdrData[1],first_sliceseg_inpic_flag);
                if(0x00800000!=first_sliceseg_inpic_flag)
				{
                    continue;
                }
                g_cachedindex[RecId][g_cachednum[RecId]].sps=g_flag_265sps[RecId];     //the first slice after sps.
                //MT_ERR_DEMUX("+++set sps %d\n",flag_265sps);
            }
            memcpy(g_cachedindex[RecId][g_cachednum[RecId]].u32HdrData,RecIndex->u32HdrData,(sizeof(mt_u32)<<1));
            g_cachedindex[RecId][g_cachednum[RecId]].type=TYPE_STC;
            g_cachedindex[RecId][g_cachednum[RecId]].pts=g_nowpts[RecId];
            g_cachedindex[RecId][g_cachednum[RecId]].offset=RecIndex->u64GlobalOffset;
            //printk("+++stc:%x\n",g_nowpts[RecId]);
        }
        g_flag_265sps[RecId]=0;
        g_cachednum[RecId]++;
    }
	
    if(CACHEDINDEX_NUM==g_cachednum[RecId])
	{                 //cached full
        if(TYPE_STC==g_cachedindex[RecId][1].type)
		{          //x stc x
            if(0xffffffff==g_cachedindex[RecId][1].pts)
			{
                g_cachedindex[RecId][0]=g_cachedindex[RecId][1];
                g_cachedindex[RecId][1]=g_cachedindex[RecId][2];
                g_cachednum[RecId]--;
                goto REDEAL;
            }
			
            if(TYPE_STC==g_cachedindex[RecId][0].type)
			{      //stc stc x
                RecIndex->u64GlobalOffset=g_cachedindex[RecId][1].offset;
                RecIndex->u32FrameSize=(mt_u32)(g_cachedindex[RecId][2].offset-g_cachedindex[RecId][1].offset);
                RecIndex->u32PtsMs=g_cachedindex[RecId][1].pts;
                //printk("av.nomerg=%llx,%x,%x\n",RecIndex->u64GlobalOffset,RecIndex->u32FrameSize,RecIndex->u32PtsMs);
            }
			else
			{                                                //pts stc x .first frame include pts
                RecIndex->u64GlobalOffset=g_cachedindex[RecId][0].offset;
                RecIndex->u32FrameSize=(mt_u32)(g_cachedindex[RecId][2].offset-g_cachedindex[RecId][0].offset);
                RecIndex->u32PtsMs=g_cachedindex[RecId][1].pts;
                if((MT_UNF_VCODEC_TYPE_HEVC==RecInfo->VCodecType)&&(1==g_cachedindex[RecId][1].sps))
				{  //hevc-patch
                    RecIndex->u32PrivatePara=1;
                }
				else
				{
                    RecIndex->u32PrivatePara=0;
                }
                //printk("av.domerg=%llx,%x,%x\n",RecIndex->u64GlobalOffset,RecIndex->u32FrameSize,RecIndex->u32PtsMs);
            }
            memcpy(RecIndex->u32HdrData,g_cachedindex[RecId][1].u32HdrData,(sizeof(mt_u32)<<1));
            //printk("++dmx.out:off=%llx,%x\n",RecIndex->u64GlobalOffset,RecIndex->u32FrameSize);
            if((MT_UNF_VCODEC_TYPE_AVS==RecInfo->VCodecType)||(MT_UNF_VCODEC_TYPE_VC1==RecInfo->VCodecType))
			{
                RecIndex->u32PrivatePara=(mt_u32)g_avs_profile[RecId];
            }
            RecIndex->CodecType = (mt_u32)CodecTypeUnfToFmw(RecInfo->VCodecType);
            //printk("offset=%llx,%llx,%llx\n",g_cachedindex[0].offset,g_cachedindex[1].offset,g_cachedindex[2].offset);
            //printk("\npts=%x_fs=%x_goff=%llx_h0=%x_h1=%x,Vtype=%d\n",RecIndex->u32PtsMs,
            //     RecIndex->u32FrameSize,RecIndex->u64GlobalOffset,RecIndex->u32HdrData[0],RecIndex->u32HdrData[1],RecIndex->CodecType);
            g_cachedindex[RecId][0]=g_cachedindex[RecId][1];
            g_cachedindex[RecId][1]=g_cachedindex[RecId][2];
            g_cachednum[RecId]--;
            //RecIndex->u32DataTimeMs=(jiffies*(1000/HZ));  //ms
	     	RecIndex->u32DataTimeMs=ktime_to_ms(ktime_get_boottime()); //ms
            return MT_SUCCESS;
        }
		else//x pts x
		{  
            g_cachedindex[RecId][0]=g_cachedindex[RecId][1];
            g_cachedindex[RecId][1]=g_cachedindex[RecId][2];
            g_cachednum[RecId]--;
            goto REDEAL;
        }
    }

    return ret1;
}

mt_s32 DMX_DRV_REC_GetRecBufferStatus(mt_u32 RecId, MT_UNF_DMX_RECBUF_STATUS_S *BufStatus)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_RecInfo_S *RecInfo = &DmxMgr->DmxRecInfo[RecId];
    
    CHECKDMXID(RecInfo->DmxId);
    BufStatus->u32BufSize = RecInfo->recbuf_size;//FqInfo->u32BufSize;
    BufStatus->u32BufSizeAlign = RecInfo->recbuf_align_size;
    BufStatus->u32UsedSize = 0;
    BufStatus->u64BufDataLen = reg_get_trpp_ch11_ini_info2(RecId);
    BufStatus->u64BufDataLen *= 188;
    BufStatus->u64RpGlobe= RecInfo->rp_globe;
    memcpy(&BufStatus->overflowinfo,&RecInfo->overflowinfo,sizeof(DMX_RecInfo_Overflow_S));
    /*MT_ERR_DEMUX("++w0:%x,%llx,wp=%llx\n",int_ts_count,RecInfo->interrupt_count,BufStatus->u64BufDataLen);
    tmp=BufStatus->u64BufDataLen;
    mod=do_div(tmp,RecInfo->recbuf_size);
    MT_ERR_DEMUX("++w1=%x,%x,lwp=%x,\n",Write,mod,BufStatus->u32BufSize);*/
    if (DMX_REC_STATUS_START == RecInfo->RecStatus)
    {
		mt_u32 Read = 0;
		mt_u32 Write = 0;
       DMXOsiOQGetReadWrite(RecId, &Write, &Read);
	if (Read < Write) {
	    BufStatus->u32UsedSize = Write - Read;
	} else if (Read > Write) {
	    BufStatus->u32UsedSize = BufStatus->u32BufSize - (Read - Write);
	}
    }

    return MT_SUCCESS;
}

mt_s32 DMX_OsiDeviceInit(mt_u32 PoolBufSize, mt_u32 BlockSize)
{
    mt_s32 ret;
    unsigned long irqflags = IRQF_TRIGGER_RISING;
    mt_u32 glb_mask = 0;

    reg_dmx_regs_base_vit = mt_get_tsi_base();
    MT_DBG_DEMUX("DMX =========%s, reg_dmx_regs_base_vit = 0x%x\n", __FILE__, reg_dmx_regs_base_vit);
    ret = DMX_OsiInit(PoolBufSize, BlockSize);

    if (MT_SUCCESS != ret)
    {
        return ret;
    }
	
#if  defined(CONFIG_MT_CHIP_SYMPHONY2)

    irqflags = IRQF_TRIGGER_RISING;

#else
    /*
    ARM interrupt need high level, so sym4 should configure to IRQF_TRIGGER_HIGH
    */
    irqflags = IRQF_TRIGGER_HIGH;

#endif

    reg_set_ts_sample_int_mask(0x42108);
    reg_set_ts_sample_int_clr(1 << 20);
    reg_set_pvr_int_mask(0x333333);
    reg_set_pvr_int_clr(1 << 24);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	reg_set_pvr2_int_mask((3<<16)|(3<<12));
    reg_set_pvr2_int_rd(1);
#endif

    if (0 != request_irq(DMX_INT_PVR, (irq_handler_t)DMXOsiIsr_PVR, irqflags, "mt_dmx_irq_pvr", MT_NULL))
    {
        DMX_OsiDeInit();
        MT_DBG_DEMUX("request_irq error !!!! \n");
        return MT_FAILURE;
    }

    reg_set_trpp5_int_mask(0xffff);
    reg_set_trpp5_int_clr(1 << 20);
	
#if  defined(CONFIG_MT_CHIP_SYMPHONY6)
	reg_set_trpp10_int_mask(0x07);
	reg_set_trpp10_int_clr(1 << 31);
#endif
    /*
    because PVR INT would cause GLB INT, so, after register PVR INT, clearing the
    bit13 in registor GGLB_INT_MASK is needed, if not, one interrupt source will
    cause two interrupt
    */
    glb_mask = reg_get_dmx_gglb_int_mask();
    glb_mask &= ~(DMX_GGLB_INT_PVR);
    
    reg_set_dmx_gglb_int_mask(glb_mask);
    reg_set_dmx_gglb_int_clr(1 << 24);
	
    if (0 != request_irq(DMX_INT_GLB, (irq_handler_t)DMXOsiIsr_IntGlb, irqflags, "mt_dmx_irq_glb", MT_NULL))
    {
        DMX_OsiDeInit();
        MT_DBG_DEMUX("request_irq error !!!! \n");
        return MT_FAILURE;
    }

    if (0 != request_irq(DMX_INT_SEC, (irq_handler_t)DMXOsiIsr_SEC, irqflags, "mt_dmx_irq_sec", MT_NULL))
    {
        DMX_OsiDeInit();
        MT_DBG_DEMUX("request_irq error !!!! \n");
        return MT_FAILURE;
    }

#if !defined(CONFIG_MT_CHIP_SYMPHONY4) && !defined(CONFIG_MT_CHIP_SYMPHONY6)
    /*
    init the delayed work, it's used for check wether the IC is working normal in
    the weak signal environment
    */
    INIT_DELAYED_WORK(&dmx_hw_reset_delayed_work, DMX_OsiRestDmxForWeakSignal);
    schedule_delayed_work(&dmx_hw_reset_delayed_work, DMX_HW_RESET_DELAYED_TIME*HZ);
#endif

    return MT_SUCCESS;
}

mt_void DMX_OsiDeviceDeInit(mt_void)
{
#ifndef CONFIG_MT_CHIP_SYMPHONY4
    cancel_delayed_work(&dmx_hw_reset_delayed_work);
#endif
    free_irq(DMX_INT_PVR, MT_NULL);
    free_irq(DMX_INT_GLB, MT_NULL);
    free_irq(DMX_INT_SEC, MT_NULL);
    DMX_OsiDeInit();
}

/*****************************************************************************
 Prototype    : DMX_OsiSuspend
 Description  : Demux
 Input        : None
 Output       : None
 Return Value : None
*****************************************************************************/
mt_s32 DMX_OsiSuspend(basedev_s *himd, pm_message_t state)
{
	int i = 0;
    
    if (NULL == g_dmx_str_reg)
    {
        g_dmx_str_reg = kzalloc(sizeof(dmx_all_reg_s), GFP_KERNEL);//malloc and set to 0.
        if (NULL == g_dmx_str_reg)
        {
            MT_ERR_DEMUX("kzalloc dmx_reg failed\n");
            return MT_ERR_DMX_ALLOC_MEM_FAILED;
        }
    }

	MT_INFO_DEMUX("enter\n");
    g_dmx_str_reg->tsi_in[0] = reg_get_ts0_sample_ctrl();        
    g_dmx_str_reg->tsi_in[1] = reg_get_ts1_sample_ctrl();
    g_dmx_str_reg->tsi_in[2] = reg_get_ts2_sample_ctrl();
    g_dmx_str_reg->tsi_in[3] = reg_get_ts3_sample_ctrl();
    MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", 0xbf200000, g_dmx_str_reg->tsi_in[0]);
    MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", 0xbf200010, g_dmx_str_reg->tsi_in[1]);
    MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", 0xbf200020, g_dmx_str_reg->tsi_in[2]);
    MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", 0xbf200030, g_dmx_str_reg->tsi_in[3]);
#ifdef CONFIG_MT_CHIP_SYMPHONY4
    {
         u32 clk_base_reg = mt_get_clk_base();
         g_dmx_str_reg->tsi_clksel_reg = dmx_inl(clk_base_reg + 0xb004);
    }
    msleep(100);    
#endif  //CONFIG_MT_CHIP_SYMPHONY4   
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->slotn_cfg0[i] = reg_get_demux_slotn_cfg0(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_demux_slotn_cfg0 + i*8, g_dmx_str_reg->slotn_cfg0[i]);
    }
	
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->slotn_cfg1[i] = reg_get_demux_slotn_cfg1(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_demux_slotn_cfg1 + i*8, g_dmx_str_reg->slotn_cfg1[i]);
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY1)	
    g_dmx_str_reg->ds.cw_op = reg_get_tsi_ds_cw_op();
    MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ds_cw_op, g_dmx_str_reg->ds.cw_op);
#endif

    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->ds.tscfg[i] = reg_get_tsi_ds_chn_tscfg(i); //?????odd key, is the reg addr is right?
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ds_chn_tscfg + i*0x80, g_dmx_str_reg->ds.tscfg[i]);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->ds.core[i] = reg_get_tsi_ds_core(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ds_core + i*0x80, g_dmx_str_reg->ds.core[i]);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->ds.ive[i] = reg_get_tsi_aes_ive(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_aes_ive + i*0x80, g_dmx_str_reg->ds.ive[i]);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->ds.mode[i] = reg_get_tsi_ades_disc_mode(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ades_disc_mode + i*0x80,g_dmx_str_reg->ds.mode[i]);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->ds.pktmode[i] = reg_get_tsi_ades_pktmode(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ades_pktmode + i*0x80,g_dmx_str_reg->ds.pktmode[i]);
    }

    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->sf.staddr[i] = reg_get_bufn_staddr(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_bufn_staddr + i*0x04,g_dmx_str_reg->sf.staddr[i]);
    }
	
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->sf.size[i] = reg_get_bufn_size(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_bufn_size + i*0x04,g_dmx_str_reg->sf.size[i]);
    }

	for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->sf.disc_wptr[i] = reg_get_bufn_disc_wptr(i);
        MT_INFO_DEMUX("i=%d, reg[0x%x] tmp save val=[0x%x]\n", i, reg_dmx_bufn_disc_wptr + i*0x04, g_dmx_str_reg->sf.disc_wptr[i]);
    }
   
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->sf.int_cfg[i] = reg_get_bufn_ts_int_cfg(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_bufn_ts_int_cfg + i*0x04,g_dmx_str_reg->sf.int_cfg[i]);
    }

    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->sf.filter_config[i] = reg_get_filtern_config(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_filtern_config + i*0x04,g_dmx_str_reg->sf.filter_config[i]);
    }
	
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->sf.filter_data[i] = reg_get_funit_filter_data(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_funit_filter_data + i*0x04,g_dmx_str_reg->sf.filter_data[i]);
    }
	
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->sf.filter_mask[i] = reg_get_funit_filter_mask(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_funit_filter_mask + i*0x04,g_dmx_str_reg->sf.filter_mask[i]);
    }
	
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        g_dmx_str_reg->sf.filter_mode[i] = reg_get_funit_filter_mode(i);
        MT_INFO_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_funit_filter_mode + i*0x04,g_dmx_str_reg->sf.filter_mode[i]);
    }
    
    g_dmx_str_reg->trpp.trpp_global[0] = reg_get_trpp_channel_parse_en();
    g_dmx_str_reg->trpp.trpp_global[1] = reg_get_trpp_bus_urgent();
    g_dmx_str_reg->trpp.trpp_global[2] = reg_get_trpp_channel_record_en();
    g_dmx_str_reg->trpp.trpp_global[3] = reg_get_trpp_sc_index_flt1_4();
    g_dmx_str_reg->trpp.trpp_global[4] = reg_get_trpp_sc_index_flt5_8();
    g_dmx_str_reg->trpp.trpp_global[5] = reg_get_trpp_sc_index_flt9_10();
    g_dmx_str_reg->trpp.trpp_global[6] = reg_get_trpp_sc_index_flt11_12();
    g_dmx_str_reg->trpp.trpp_global[7] = reg_get_trpp_sc_index_flt0();
    g_dmx_str_reg->trpp.trpp_global[8] = reg_get_trpp_esbuf_ch();

    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_property[i] = reg_get_trpp_ch_property(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_parse_set[i] = reg_get_trpp_ch_parse_set(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_start_code1[i] = reg_get_trpp_ch_start_code1(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_frm_start_code_m1[i] = reg_get_trpp_ch_frm_start_code_m1(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_start_code2[i] = reg_get_trpp_ch_start_code2(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_frm_start_code_m2[i] = reg_get_trpp_ch_frm_start_code_m2(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_dscrpt_start_addr[i] = reg_get_trpp_ch_dscrpt_start_addr(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_data_start_addr[i] = reg_get_trpp_ch_data_start_addr(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_dscrpt_end_addr[i] = reg_get_trpp_ch_dscrpt_end_addr(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_data_end_addr[i] = reg_get_trpp_ch_data_end_addr(i);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch1_ini_info1[i] = reg_get_trpp_ch1_ini_info1(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch1_ini_info2[i] = reg_get_trpp_ch1_ini_info2(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch1_ini_info3[i] = reg_get_trpp_ch1_ini_info3(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch1_ini_info4[i] = reg_get_trpp_ch1_ini_info4(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch1_ini_info5[i] = reg_get_trpp_ch1_ini_info5(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch1_ini_info6[i] = reg_get_trpp_ch1_ini_info6(i);
    }
	
    for (i=0; i<CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch1_ini_info7[i] = reg_get_trpp_ch1_ini_info7(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch1_ini_info8[i] = reg_get_trpp_ch1_ini_info8(i);
    }
	
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch1_ini_info9[i] = reg_get_trpp_ch1_ini_info9(i);
    }
	
    for (i = 0; i < REC_CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.ch_rec_set[i] = reg_get_trpp_ch_rec_set(i);
    }
	
    for (i = 0; i < REC_CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.rec_start_addr[i] = reg_get_trpp_ch_rec_start_addr(i);
    }
	
    for (i = 0; i < REC_CHANNEL_REG_COUNT; i++)
    {
        g_dmx_str_reg->trpp.rec_end_addr[i] = reg_get_trpp_ch_rec_end_addr(i);
    }

	g_dmx_str_reg->global.sample_int_mask = reg_get_ts_sample_int_mask();
	g_dmx_str_reg->global.sample_int_edge = reg_get_ts_sample_int_edge();
	g_dmx_str_reg->global.sample_int_clr  = reg_get_ts_sample_int_clr();

	g_dmx_str_reg->global.ds_int_mask = reg_get_ds_int_mask();
	g_dmx_str_reg->global.ds_int_edge = reg_get_ds_int_edge();
	g_dmx_str_reg->global.ds_int_clr  = reg_get_ds_int_clr();

	for (i=0; i < CHANNEL_REG_COUNT; i++)
	{
		g_dmx_str_reg->global.trpp_int_mask[i] = reg_get_trpp_int_mask(i);
		g_dmx_str_reg->global.trpp_int_edge[i] = reg_get_trpp_int_edge(i);
		g_dmx_str_reg->global.trpp_int_clr[i]  = reg_get_trpp_int_clr(i);
	}

	g_dmx_str_reg->global.pvr_int_mask = reg_get_pvr_int_mask();
	g_dmx_str_reg->global.pvr_int_edge = reg_get_pvr_int_edge();
	g_dmx_str_reg->global.pvr_int_clr  = reg_get_pvr_int_clr();

	g_dmx_str_reg->global.gglb_int_mask = reg_get_dmx_gglb_int_mask();
	g_dmx_str_reg->global.gglb_int_edge = reg_get_dmx_gglb_int_edge();
	g_dmx_str_reg->global.gglb_int_clr  = reg_get_dmx_gglb_int_clr();
	
	MT_ALWAYS_PRINT("DEMUX suspend OK\n");

    return MT_SUCCESS;
}

/*****************************************************************************
 Prototype    : DMX_OsiResume
 Description  : Demux
 Input        : None
 Output       : None
 Return Value : None
 Others       :
*****************************************************************************/
mt_s32 DMX_OsiResume(basedev_s *himd)
{
	int i = 0;
    mt_s32 ret = MT_SUCCESS;

	//init dmx first
	reg_dmx_init();
        DMX_OsiInitChInit(0x3, 0x3, 0x2, 0x4, 0x4, 0x2);
	DMX_OsiHardwareInit(1, 0xf,  NULL);

	if (NULL == g_dmx_str_reg)
	{
		MT_ERR_DEMUX("g_dmx_str_reg is NULL, the register don't stored before, pls check\n");
		return ret;
	}

#ifdef CONFIG_MT_CHIP_SYMPHONY4
     {
         u32 clk_base_reg = mt_get_clk_base();
         dmx_outl((clk_base_reg + 0xb004), g_dmx_str_reg->tsi_clksel_reg);    //resume tsi_clock
     }
#endif
	
	//restore the register
	for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_demux_slotn_cfg0(i, g_dmx_str_reg->slotn_cfg0[i] );
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_demux_slotn_cfg0 + i*0x08, g_dmx_str_reg->slotn_cfg0[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_demux_slotn_cfg1(i, g_dmx_str_reg->slotn_cfg1[i] );
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_demux_slotn_cfg1 + i*0x08, g_dmx_str_reg->slotn_cfg1[i]);
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY1)	
    reg_set_tsi_ds_cw_op(g_dmx_str_reg->ds.cw_op);
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ds_cw_op, g_dmx_str_reg->ds.cw_op);
#endif

    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_tsi_ds_chn_tscfg(i, g_dmx_str_reg->ds.tscfg[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ds_chn_tscfg + i*0x80, g_dmx_str_reg->ds.tscfg[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
         reg_set_tsi_ds_core(i, g_dmx_str_reg->ds.core[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ds_core + i*0x80, g_dmx_str_reg->ds.core[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_tsi_aes_ive(i, g_dmx_str_reg->ds.ive[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_aes_ive + i * 0x80, g_dmx_str_reg->ds.ive[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_tsi_ades_disc_mode(i, g_dmx_str_reg->ds.mode[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ades_disc_mode + i*0x80, g_dmx_str_reg->ds.mode[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_tsi_ades_pktmode(i, g_dmx_str_reg->ds.pktmode[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ades_pktmode + i*0x80, g_dmx_str_reg->ds.pktmode[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_bufn_staddr(i, g_dmx_str_reg->sf.staddr[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_bufn_staddr + i*0x04, g_dmx_str_reg->sf.staddr[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_bufn_size(i, (g_dmx_str_reg->sf.size[i] & 0xffff));
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_bufn_size + i*0x04, g_dmx_str_reg->sf.size[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_bufn_ts_int_cfg(i, g_dmx_str_reg->sf.int_cfg[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_bufn_ts_int_cfg + i*0x04, g_dmx_str_reg->sf.int_cfg[i]);
    } 
    
	for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_bufn_disc_wptr(i, (g_dmx_str_reg->sf.disc_wptr[i] & 0xffff));
        MT_INFO_DEMUX("i=%d, reg[0x%x] restore from val=[0x%x], real_value=0x%x\n", i, reg_dmx_bufn_disc_wptr + i*0x04, g_dmx_str_reg->sf.disc_wptr[i], reg_get_bufn_disc_wptr(i));
    }

    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_filtern_config(i, g_dmx_str_reg->sf.filter_config[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_filtern_config + i*0x04, g_dmx_str_reg->sf.filter_config[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_funit_filter_data(i, g_dmx_str_reg->sf.filter_data[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_funit_filter_data + i*0x04, g_dmx_str_reg->sf.filter_data[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_funit_filter_mask(i, g_dmx_str_reg->sf.filter_mask[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_funit_filter_mask + i*0x04, g_dmx_str_reg->sf.filter_mask[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        reg_set_funit_filter_mode(i, g_dmx_str_reg->sf.filter_mode[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_funit_filter_mode + i*0x04, g_dmx_str_reg->sf.filter_mode[i]);
    }
    
    reg_set_trpp_channel_parse_en(g_dmx_str_reg->trpp.trpp_global[0]);
    reg_set_trpp_bus_urgent(g_dmx_str_reg->trpp.trpp_global[1]);
    reg_set_trpp_channel_record_en(g_dmx_str_reg->trpp.trpp_global[2]);
    reg_set_trpp_sc_index_flt1_4(g_dmx_str_reg->trpp.trpp_global[3]);
    reg_set_trpp_sc_index_flt5_8(g_dmx_str_reg->trpp.trpp_global[4]);
    reg_set_trpp_sc_index_flt9_10(g_dmx_str_reg->trpp.trpp_global[5]);
    reg_set_trpp_sc_index_flt11_12(g_dmx_str_reg->trpp.trpp_global[6]);
    reg_set_trpp_sc_index_flt0(g_dmx_str_reg->trpp.trpp_global[7]);
    reg_set_trpp_esbuf_ch(g_dmx_str_reg->trpp.trpp_global[8]);
    
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_channel_parse_en, g_dmx_str_reg->trpp.trpp_global[0]);
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_bus_urgent, g_dmx_str_reg->trpp.trpp_global[1]);
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_channel_record_en, g_dmx_str_reg->trpp.trpp_global[2]);
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt1_4, g_dmx_str_reg->trpp.trpp_global[3]);
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt5_8, g_dmx_str_reg->trpp.trpp_global[4]);
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt9_10, g_dmx_str_reg->trpp.trpp_global[5]);       
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt11_12, g_dmx_str_reg->trpp.trpp_global[6]);       
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt0, g_dmx_str_reg->trpp.trpp_global[7]);
    MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_esbuf_ch, g_dmx_str_reg->trpp.trpp_global[8]);
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_property(i, g_dmx_str_reg->trpp.ch_property[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_property + i*0x100, g_dmx_str_reg->trpp.ch_property[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_parse_set(i, g_dmx_str_reg->trpp.ch_parse_set[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_parse_set + i*0x100, g_dmx_str_reg->trpp.ch_parse_set[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_start_code1(i, g_dmx_str_reg->trpp.ch_start_code1[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_start_code1  + i*0x100, g_dmx_str_reg->trpp.ch_start_code1[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_frm_start_code_m1(i, g_dmx_str_reg->trpp.ch_frm_start_code_m1[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_frm_start_code_m1  + i*0x100, g_dmx_str_reg->trpp.ch_frm_start_code_m1[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_start_code2(i, g_dmx_str_reg->trpp.ch_start_code2[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_start_code2  + i*0x100, g_dmx_str_reg->trpp.ch_start_code2[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_frm_start_code_m2(i, g_dmx_str_reg->trpp.ch_frm_start_code_m2[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_frm_start_code_m2  + i*0x100, g_dmx_str_reg->trpp.ch_frm_start_code_m2[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_dscrpt_start_addr(i, g_dmx_str_reg->trpp.ch_dscrpt_start_addr[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_dscrpt_start_addr  + i*0x100, g_dmx_str_reg->trpp.ch_dscrpt_start_addr[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_data_start_addr(i, g_dmx_str_reg->trpp.ch_data_start_addr[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_data_start_addr  + i*0x100, g_dmx_str_reg->trpp.ch_data_start_addr[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_dscrpt_end_addr(i, g_dmx_str_reg->trpp.ch_dscrpt_end_addr[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_dscrpt_end_addr  + i*0x100, g_dmx_str_reg->trpp.ch_dscrpt_end_addr[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_data_end_addr(i, g_dmx_str_reg->trpp.ch_data_end_addr[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_data_end_addr  + i*0x100, g_dmx_str_reg->trpp.ch_data_end_addr[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info1(i, g_dmx_str_reg->trpp.ch1_ini_info1[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info1  + i*0x100, g_dmx_str_reg->trpp.ch1_ini_info1[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info2(i, g_dmx_str_reg->trpp.ch1_ini_info2[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info2  + i*0x100, g_dmx_str_reg->trpp.ch1_ini_info2[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info3(i, g_dmx_str_reg->trpp.ch1_ini_info3[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info3  + i*0x100, g_dmx_str_reg->trpp.ch1_ini_info3[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info4(i, g_dmx_str_reg->trpp.ch1_ini_info4[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info4  + i*0x100, g_dmx_str_reg->trpp.ch1_ini_info4[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info5(i, g_dmx_str_reg->trpp.ch1_ini_info5[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info5  + i*0x100, g_dmx_str_reg->trpp.ch1_ini_info5[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info6(i, g_dmx_str_reg->trpp.ch1_ini_info6[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info6  + i*0x100, g_dmx_str_reg->trpp.ch1_ini_info6[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info7(i, g_dmx_str_reg->trpp.ch1_ini_info7[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info7 + i*0x100, g_dmx_str_reg->trpp.ch1_ini_info7[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info8(i, g_dmx_str_reg->trpp.ch1_ini_info8[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info8 + i*0x100, g_dmx_str_reg->trpp.ch1_ini_info8[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info9(i, g_dmx_str_reg->trpp.ch1_ini_info9[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info9 + i*0x100, g_dmx_str_reg->trpp.ch1_ini_info9[i]);
    }
    
    for (i = 0; i < REC_CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_rec_set(i, g_dmx_str_reg->trpp.ch_rec_set[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_rec_set + i*0x100, g_dmx_str_reg->trpp.ch_rec_set[i]);
    }
    
    for (i = 0; i < REC_CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_rec_start_addr(i, g_dmx_str_reg->trpp.rec_start_addr[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_rec_start_addr + i*0x100, g_dmx_str_reg->trpp.rec_start_addr[i]);
    }
    
    for (i = 0; i < REC_CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_rec_end_addr(i, g_dmx_str_reg->trpp.rec_end_addr[i]);
        MT_INFO_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_rec_end_addr + i*0x100, g_dmx_str_reg->trpp.rec_end_addr[i]);
    }

	reg_set_ts_sample_int_mask(g_dmx_str_reg->global.sample_int_mask);
	reg_set_ts_sample_int_edge(g_dmx_str_reg->global.sample_int_edge);
	reg_set_ts_sample_int_clr(g_dmx_str_reg->global.sample_int_clr);

	reg_set_ds_int_mask(g_dmx_str_reg->global.ds_int_mask);
	reg_set_ds_int_edge(g_dmx_str_reg->global.ds_int_edge);
	reg_set_ds_int_clr(g_dmx_str_reg->global.ds_int_clr);

	for (i=0; i < CHANNEL_REG_COUNT; i++)
	{
		reg_set_trpp_int_mask(i, g_dmx_str_reg->global.trpp_int_mask[i]);
		reg_set_trpp_int_edge(i, g_dmx_str_reg->global.trpp_int_edge[i]);
		reg_set_trpp_int_clr(i, g_dmx_str_reg->global.trpp_int_clr[i]);
	}

	reg_set_pvr_int_mask(g_dmx_str_reg->global.pvr_int_mask);
	reg_set_pvr_int_edge(g_dmx_str_reg->global.pvr_int_edge);
	reg_set_pvr_int_clr(g_dmx_str_reg->global.pvr_int_clr);

	reg_set_dmx_gglb_int_mask(g_dmx_str_reg->global.gglb_int_mask);
	reg_set_dmx_gglb_int_edge(g_dmx_str_reg->global.gglb_int_edge);
	reg_set_dmx_gglb_int_clr(g_dmx_str_reg->global.gglb_int_clr);
	
	//set ts sample ctrl
	reg_set_ts0_sample_ctrl(g_dmx_str_reg->tsi_in[0]);
    reg_set_ts1_sample_ctrl(g_dmx_str_reg->tsi_in[1]);
    reg_set_ts2_sample_ctrl(g_dmx_str_reg->tsi_in[2]);
    reg_set_ts3_sample_ctrl(g_dmx_str_reg->tsi_in[3]);
	
	kfree(g_dmx_str_reg);
	g_dmx_str_reg = NULL;

    MT_ALWAYS_PRINT("DEMUX resume OK\n");
    return MT_SUCCESS;
}

#ifdef MT_DEMUX_PROC_SUPPORT
mt_s32 DMX_OsiRamPortGetBufInfo(mt_u32 PortId, DMX_Proc_RamPort_BufInfo_S *BufInfo)
{
    mt_s32 ret = MT_FAILURE;
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];

    if (PortInfo->PhyAddr) {
	BufInfo->PhyAddr = PortInfo->PhyAddr;
	BufInfo->BufSize = PortInfo->BufSize;
	BufInfo->Read = PortInfo->Read;
	BufInfo->Write = PortInfo->Write;
	BufInfo->UsedSize = GetQueueLenth(BufInfo->Read, BufInfo->Write, BufInfo->BufSize);
	BufInfo->GetCount = PortInfo->GetCount;
	BufInfo->GetValidCount = PortInfo->GetValidCount;
	BufInfo->PutCount = PortInfo->PutCount;

	ret = MT_SUCCESS;
    }

    return ret;
}

mt_s32 DMX_OsiRamPortGetDescInfo(mt_u32 PortId, DMX_Proc_RamPort_DescInfo_S *DescInfo)
{
    mt_s32 ret = MT_FAILURE;
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];

    if (PortInfo->PhyAddr) {
	DmxHalIPPortGetDescInfo(PortId, DescInfo);
	return MT_SUCCESS;
    }

    return ret;
}

mt_s32 DMX_OsiRamPortGetBPStatus(mt_u32 PortId, DMX_Proc_RamPort_BPStatus_S *BPStatus)
{
    mt_s32 ret = MT_FAILURE;
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[PortId];

    if (PortInfo->PhyAddr) {
	DmxHalIPPortGetBPStatus(PortId, BPStatus);
	return MT_SUCCESS;
    }

    return ret;
}

mt_void DMX_OsiGetFQInfo(mt_u32 FQId, FQ_HeaderInfor_t *FQInfo)
{
    mt_u32 FQWord[4];
    mt_u32 i = 0;

    for (i = 0; i < 4; i++) {
	DmxHalGetFQWORDx(FQId, i, &FQWord[i]);
    }
    /*
    [127:96]:FQStartAddr(32bit);
    [96:64]:{FQSize(16bit),FQWPtr(16bit)};
    [63:32]:{FQVal(16bit),FQRPtr(16bit)}
    [31:0]:{FQAlovfl_TH(8bit),FQIntCfg(4bit),FQIntCnt(4bit),FQUse(16bit)}
    */

    FQInfo->FQUse = FQWord[0] & 0xffff;
    FQInfo->FQRPtr = FQWord[1] & 0xffff;
    FQInfo->FQVal = (FQWord[1] >> 16) & 0xffff;
    FQInfo->FQWPtr = FQWord[2] & 0xffff;
    FQInfo->FQSize = (FQWord[2] >> 16) & 0xffff;
    FQInfo->FQStartAddr = FQWord[3];

    return;
}

mt_void DMX_OsiGetOQInfo(mt_u32 OQId, OQ_HeaderInfor_t *OQInfo)
{
    mt_u32 OQWord[8];
    mt_u32 i = 0;

    for (i = 0; i < 8; i++) {
	DmxHalGetOQWORDx(OQId, i, &OQWord[i]);
    }
    /*
    [255:224]:BQSAddr(32bit)
    [223:192]:{Rsv(2bit),BQIntCfg(4bit),BQSize(10bit),BQAlovfl_TH(8bit),BQCfg(8bit)};
    [191:160]:{Rsv(6bit),BQRPtr(10bit),BQIntCnt(4bit),Rsv(2bit),BQWPtr(10bit)}
    [159:128]:BBSAddr(32bit)
    [127:96]:{BBSize(16bit),BQUse(16bit)}
    [96:64]:{BBEopAddr(16bit),BBWaddr(16bit)}
    [63:32]:{WResByte(24bit),PVRCtrl(8bit)}
    [31:0]:{EopResByte(24bit),Rsv(8bit)}

    */
    //OQInfo->EopResByte        = OQWord[0];
    //OQInfo->WResByte      = (OQWord[1] >> 8) & 0xffffff;
    OQInfo->OQID = OQId;
    OQInfo->BBWAddr = OQWord[2] & 0xffff;
    OQInfo->BBEopAddr = (OQWord[2] >> 16) & 0xffff;
    OQInfo->OQUse = OQWord[3] & 0x3fff;
    OQInfo->BBSize = (OQWord[3] >> 16) & 0xffff;
    OQInfo->BBSAddr = OQWord[4];
    OQInfo->OQWPtr = OQWord[5] & 0x3ff;
    OQInfo->OQRPtr = (OQWord[5] >> 16) & 0x3ff;
    OQInfo->FQCfg = OQWord[6] & 0xff;
    OQInfo->OQSize = (OQWord[6] >> 16) & 0x3ff;
    OQInfo->OQSAddr = OQWord[7];
    OQInfo->OQWAddr = OQInfo->OQSAddr + 16 * OQInfo->OQWPtr;
    OQInfo->OQRAddr = OQInfo->OQSAddr + 16 * OQInfo->OQRPtr;

    return;
}

mt_void DMX_OsiGetChannelDataFlow(mt_u32 ChannelId, ChannelDataFlow_info_t *ChannelDF)
{
    DMXHalGetChannelDataFlow(ChannelId, ChannelDF);
    return;
}

mt_s32 DMX_OsiGetDmxRecProc(mt_u32 RecId, DMX_Proc_Rec_BufInfo_S *BufInfo)
{
    mt_s32 ret = MT_FAILURE;
    DMX_RecInfo_S *RecInfo = &g_pDmxDevOsi->DmxRecInfo[RecId];

    if (RecInfo->DmxId < DMX_CNT)
    {
        switch (RecInfo->RecType)
        {
            case MT_UNF_DMX_REC_TYPE_SELECT_PID:
            case MT_UNF_DMX_REC_TYPE_ALL_PID:
            {
                DMX_FQ_Info_S *FqInfo = &g_pDmxDevOsi->DmxFqInfo[RecInfo->RecFqId];

                BufInfo->RecType = RecInfo->RecType;
                BufInfo->Descramed = RecInfo->Descramed;
                BufInfo->BlockCnt = FqInfo->u32FQDepth - 1;
                BufInfo->BlockSize = FqInfo->u32BlockSize;
                BufInfo->RecStatus = (DMX_REC_STATUS_START == RecInfo->RecStatus) ? 1 : 0;
                BufInfo->Overflow = RecInfo->rec_data_overflow_cnt;
                BufInfo->Overflow = RecInfo->rec_index_overflow_cnt;
                BufInfo->BufSize = RecInfo->recbuf_size;
                BufInfo->IndexSize = TRPP_REC_INDEX_BUFFER_SIZE;
                DMXOsiOQGetReadWrite(RecId, &BufInfo->BufWrite, &BufInfo->BufRead);				
                DMXOsiIndexGetReadWrite(RecId, &BufInfo->IndexWrite, &BufInfo->IndexRead);
                ret = MT_SUCCESS;
                break;
            }

            default:
                ret = MT_FAILURE;
                break;
        }
    }

    return ret;
}

mt_s32 DMX_OsiGetDmxRecScdProc(mt_u32 RecId, DMX_Proc_RecScd_BufInfo_S *BufInfo)
{
    mt_s32 ret = MT_FAILURE;
    DMX_RecInfo_S *RecInfo = &g_pDmxDevOsi->DmxRecInfo[RecId];

    if (RecInfo->DmxId < DMX_CNT) {
	memset(BufInfo, 0, sizeof(DMX_Proc_RecScd_BufInfo_S));

	switch (RecInfo->RecType) {
	case MT_UNF_DMX_REC_TYPE_SELECT_PID: {
	    BufInfo->IndexType = RecInfo->IndexType;
	    BufInfo->IndexPid = DMX_INVALID_PID;

	    if (MT_UNF_DMX_REC_INDEX_TYPE_NONE != RecInfo->IndexType) {
		DMX_FQ_Info_S *FqInfo = &g_pDmxDevOsi->DmxFqInfo[RecInfo->ScdFqId];

		BufInfo->IndexPid = RecInfo->IndexPid;
		BufInfo->BlockCnt = FqInfo->u32FQDepth - 1;
		BufInfo->BlockSize = FqInfo->u32BlockSize;
		BufInfo->Overflow = FqInfo->FqOverflowCount;

		DMXOsiOQGetReadWrite(RecInfo->ScdOqId, &BufInfo->BufWrite, &BufInfo->BufRead);
	    }

	    ret = MT_SUCCESS;

	    break;
	}

	case MT_UNF_DMX_REC_TYPE_ALL_PID: {
	    BufInfo->IndexType = MT_UNF_DMX_REC_INDEX_TYPE_NONE;
	    BufInfo->IndexPid = DMX_INVALID_PID;

	    ret = MT_SUCCESS;

	    break;
	}

	default:
	    ret = MT_FAILURE;
	}
    }

    return ret;
}

DMX_ChanInfo_S *DMX_OsiGetChannelProc(mt_u32 ChanId)
{
    DMX_ChanInfo_S *ChanInfo = MT_NULL;

    ChanInfo = &g_pDmxDevOsi->DmxChanInfo[ChanId];
    if (ChanInfo->DmxId >= DMX_CNT) {
	ChanInfo = MT_NULL;
    }

    return ChanInfo;
}
DMX_ChanEsBuff_S *DMX_OsiGetChannelEsBufProc(mt_u32 ChanId)
{
    DMX_ChanEsBuff_S *esChanInfo;
    esChanInfo = &g_pDmxDevOsi->DmxChanEsBuff[ChanId];
    if (esChanInfo->esBuffId >= DMX_AV_CHANNEL_CNT) {
	esChanInfo = MT_NULL;
    }
    if (esChanInfo->u32IsUsed == DMX_ISUSED_FREE) {
	esChanInfo = MT_NULL;
    }
    return esChanInfo;
}

mt_s32 DMX_OsiGetChanBufProc(mt_u32 ChanId, DMX_Proc_ChanBuf_S *BufInfo)
{
	if(ChanId >=  DMX_CHANNEL_CNT)
	{
		return MT_FAILURE;
	}
	
    DMX_ChanInfo_S *ChanInfo = &g_pDmxDevOsi->DmxChanInfo[ChanId];

    if (ChanInfo->DmxId >= DMX_CNT) {
	return MT_FAILURE;
    }

    memset(BufInfo, 0, sizeof(DMX_Proc_ChanBuf_S));

    if (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY == (MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY & ChanInfo->ChanOutMode))
    {
        switch (ChanInfo->ChanType)
        {
            case MT_UNF_DMX_CHAN_TYPE_AUD:
            case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
            {
                DMXOsiChnEsDataGetReadWrite(ChanInfo->avChanId, &BufInfo->DataWrite, &BufInfo->DataRead);
                DMXOsiChnEsDescDataGetReadWrite(ChanInfo->avChanId, &BufInfo->DescWrite, &BufInfo->DescRead);
                BufInfo->DescDepth = 1;
                BufInfo->BlockSize = ChanInfo->ChanBufSize;
                BufInfo->Overflow = 0;
                BufInfo->DescOverflow = 0;
                BufInfo->DescSize = DMX_SYMPHONY_ES_DESC_BUF_SIZE;
            }
            break;

            case MT_UNF_DMX_CHAN_TYPE_VID:
            {
                DMXOsiChnEsDataGetReadWrite(ChanInfo->avChanId, &BufInfo->DataWrite, &BufInfo->DataRead);
                DMXOsiChnEsDescDataGetReadWrite(ChanInfo->avChanId, &BufInfo->DescWrite, &BufInfo->DescRead);
                BufInfo->DescDepth = 1;
                BufInfo->BlockSize = ChanInfo->ChanBufSize;
                BufInfo->Overflow = 0;
                BufInfo->DescOverflow = 0;
                BufInfo->DescSize = DMX_SYMPHONY_VES_DESC_BUF_SIZE;
            }
            break;

            case MT_UNF_DMX_CHAN_TYPE_HW_PES:
            {
                DMXOsiChnEsDataGetReadWrite(ChanInfo->avChanId, &BufInfo->DataWrite, &BufInfo->DataRead);
                DMXOsiChnEsDescDataGetReadWrite(ChanInfo->avChanId, &BufInfo->DescWrite, &BufInfo->DescRead);
                BufInfo->DescDepth = 1;
                BufInfo->BlockSize = ChanInfo->ChanBufSize;
                BufInfo->Overflow = 0;
                BufInfo->DescOverflow = 0;
                BufInfo->DescSize = DMX_SYMPHONY_PES_DESC_BUF_SIZE;
            }
            break;

            case MT_UNF_DMX_CHAN_TYPE_SEC:
            case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
            case MT_UNF_DMX_CHAN_TYPE_POST:
            case MT_UNF_DMX_CHAN_TYPE_PES:
            {
                DMX_ChanSecBuff_S *SecBuff = &g_pDmxDevOsi->DmxChanSecBuff[ChanInfo->secBuffId];

                DMXOsiChnSectionDataGetReadWrite(ChanInfo->secBuffId, &BufInfo->DataWrite, &BufInfo->DataRead);
                DMXOsiChnSectionDescDataGetReadWrite(ChanInfo->secBuffId, &BufInfo->DescWrite, &BufInfo->DescRead);
                BufInfo->DescDepth = 1;
                BufInfo->BlockSize = ChanInfo->ChanBufSize;
                BufInfo->Overflow = 0;
                BufInfo->DescOverflow = 0;
                BufInfo->DescSize = DMX_SYMPHONY_SEC_DESC_BUF_SIZE;
                BufInfo->SoftDataRead = SecBuff->data_buff_write_p;
                BufInfo->SoftDescRead = SecBuff->desc_buff_write_p;               
            }
            break;

            default:
            {
                MT_ERR_DEMUX("Get ChanType = %d is not allowed\n", ChanInfo->ChanType);
                return MT_ERR_DMX_INVALID_PARA;
            }
        }

        return MT_SUCCESS;
    }
    else if (MT_UNF_DMX_CHAN_OUTPUT_MODE_REC == (MT_UNF_DMX_CHAN_OUTPUT_MODE_REC & ChanInfo->ChanOutMode))
    {
        if (MT_UNF_DMX_CHAN_TYPE_REC == ChanInfo->ChanType)
        {
            return MT_SUCCESS;
        }
		else
		{
			return MT_FAILURE;
		}
    }
    else
    {
        MT_ERR_DEMUX("Chan %d sta is not play\n", ChanInfo->ChanId);
        return MT_FAILURE;
    }

	return MT_SUCCESS;
}


DMX_FilterInfo_S *DMX_OsiGetFilterProc(mt_u32 FilterId)
{
    DMX_FilterInfo_S *FilterInfo;

    FilterInfo = &g_pDmxDevOsi->DmxFilterInfo[FilterId];
    if (DMX_INVALID_FILTER_ID == FilterInfo->FilterId) {
	FilterInfo = MT_NULL;
    }

    return FilterInfo;
}

DMX_PCR_Info_S *DMX_OsiGetPcrChannelProc(const mt_u32 PcrId)
{
    DMX_PCR_Info_S *PcrInfo;

    PcrInfo = &g_pDmxDevOsi->DmxPcrInfo[PcrId];
    if (PcrInfo->DmxId >= DMX_CNT) {
	PcrInfo = MT_NULL;
    }

    return PcrInfo;
}

DMX_RamPort_Info_S *DMX_OsiGetSwTsiProc(const mt_u32 SwtsiId)
{
    DMX_RamPort_Info_S *RamPortInfo = NULL;

    RamPortInfo = &g_pDmxDevOsi->RamPortInfo[SwtsiId];

    return RamPortInfo;
}


DMX_Sub_DevInfo_S *DMX_OsiGetTsiPortProc(const mt_u32 DmxId)
{
     DMX_Sub_DevInfo_S *TsiPortInfo = NULL;

     TsiPortInfo = &g_pDmxDevOsi->SubDevInfo[DmxId];

    return TsiPortInfo;
}

//just for record PMT
#define PAT_TABLEID 0
#define MAX_PMT_NUMBER 16

typedef struct
{
    mt_u32 u32ProgNum;
    mt_u32 u32PmtPID[MAX_PMT_NUMBER];
} PmtPid;

static PmtPid stPmtPid;
static void insert_pmt_pid(mt_u32 u32PmtPid)
{
    mt_u32 i;
    for (i = 0; i < stPmtPid.u32ProgNum; i++) {
	if (stPmtPid.u32PmtPID[i] == u32PmtPid) {
	    return;
	}
    }
    if (stPmtPid.u32ProgNum < MAX_PMT_NUMBER) {
	stPmtPid.u32PmtPID[stPmtPid.u32ProgNum] = u32PmtPid;
	stPmtPid.u32ProgNum++;
    }
    return;
}

static mt_s32 parse_pat_table(mt_u8 *pu8Data, ulong u32Len)
{
    mt_u32 u32SecLen, u32CurSecNum, u32LastSecNum;
    mt_u32 u32ProgramNum, u32PmtPid;
    u32SecLen = (pu8Data[1] & 0xf) << 8 | pu8Data[2];
    u32CurSecNum = pu8Data[6];
    u32LastSecNum = pu8Data[7];
    if (u32SecLen < 12 || u32SecLen > u32Len) {
	return -1;
    }
    pu8Data += 8; //jump to program_number
    u32SecLen -= 9;
    while (u32SecLen > 4) {
	u32ProgramNum = (pu8Data[0] << 8) | pu8Data[1];
	u32PmtPid = ((pu8Data[2] & 0x1f) << 8) | pu8Data[3];
	if (u32ProgramNum != 0) {
	    insert_pmt_pid(u32PmtPid);
	}
	u32SecLen -= 4;
	pu8Data += 4;
    }
    return 0;
}

static mt_s32 DMX_OsiGetPmtPid(mt_u32 DmxId)
{
    mt_s32 ret;
    MT_UNF_DMX_CHAN_ATTR_S ChanAttr;
    DMX_MMZ_BUF_S ChanBuf;
    MT_UNF_DMX_FILTER_ATTR_S FilterAttr;
    mt_u32 ChanId;
    mt_u32 FilterId;
    mt_u32 SectNum;
    DMX_UserMsg_S SectData[20];

    ChanAttr.u32BufSize = 0x4000;
    ChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORCE_AND_DISCARD;
    ChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    ChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;

    ret = DMX_OsiCreateChannel(DmxId, &ChanAttr, &ChanBuf, NULL, &ChanId);
    if (MT_SUCCESS != ret) {
	return ret;
    }

    ret = DMX_OsiSetChannelPid(ChanId, 0);
    if (MT_SUCCESS != ret) {
	DMX_OsiDestroyChannel(ChanId);

	return ret;
    }

    ret = DMX_OsiNewFilter(DmxId, &FilterId);
    if (MT_SUCCESS != ret) {
	DMX_OsiDestroyChannel(ChanId);

	return ret;
    }

    FilterAttr.u32FilterDepth = 1;

    FilterAttr.au8Match[0] = PAT_TABLEID;
    FilterAttr.au8Mask[0] = 0x00;
    FilterAttr.au8Negate[0] = 0;

    ret = DMX_OsiSetFilterAttr(FilterId, &FilterAttr);
    if (MT_SUCCESS != ret) {
	DMX_OsiDeleteFilter(FilterId);
	DMX_OsiDestroyChannel(ChanId);

	return ret;
    }

    ret = DMX_OsiAttachFilter(FilterId, ChanId);
    if (MT_SUCCESS != ret) {
	DMX_OsiDeleteFilter(FilterId);
	DMX_OsiDestroyChannel(ChanId);

	return ret;
    }

    ret = DMX_OsiOpenChannel(ChanId);
    if (MT_SUCCESS != ret) {
	DMX_OsiDeleteFilter(FilterId);
	DMX_OsiDestroyChannel(ChanId);

	return ret;
    }

    msleep(2000);

    stPmtPid.u32ProgNum = 0;

    ret = DMX_OsiReadDataRequest(ChanId, 20, &SectNum, SectData, 2000);
    if ((MT_SUCCESS == ret) && SectNum) {
	DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
	DMX_FQ_Info_S *FqInfo = &DmxMgr->DmxFqInfo[DMX_FQ_COMMOM];
	ulong KerAddr;
	mt_u32 Offset;
	mt_u32 i;

	for (i = 0; i < SectNum; i++) {
	    Offset = SectData[i].u32BufStartAddr - FqInfo->u32BufPhyAddr;
	    KerAddr = FqInfo->u32BufVirAddr + Offset;

	    parse_pat_table((mt_u8 *)KerAddr, SectData[i].u32MsgLen);
	}

	DMX_OsiReleaseReadData(ChanId, SectNum, SectData);
    }

    DMX_OsiCloseChannel(ChanId);
    DMX_OsiDetachFilter(FilterId, ChanId);
    DMX_OsiDeleteFilter(FilterId);
    DMX_OsiDestroyChannel(ChanId);

    return ret;
}

static mt_s32 DMX_OsiAddRecPid(mt_u32 DmxId, mt_u32 Pid, mt_u32 *ChanId)
{
    mt_s32 ret;
    MT_UNF_DMX_CHAN_ATTR_S ChanAttr;
    DMX_MMZ_BUF_S ChanBuf;

    ChanAttr.u32BufSize = 0x4000;
    ChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
    ChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_POST;
    ChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_REC;

    ret = DMX_OsiCreateChannel(DmxId, &ChanAttr, &ChanBuf, NULL, ChanId);
    if (MT_SUCCESS != ret) {
	return ret;
    }

    ret = DMX_OsiSetChannelPid(*ChanId, Pid);
    if (MT_SUCCESS != ret) {
	DMX_OsiDestroyChannel(*ChanId);

	return ret;
    }

    ret = DMX_OsiOpenChannel(*ChanId);
    if (MT_SUCCESS != ret) {
	DMX_OsiDestroyChannel(*ChanId);

	return ret;
    }

    return MT_SUCCESS;
}

static mt_void DelAllDmxChannel(mt_u32 DmxId)
{
    mt_u32 i;

    for (i = 0; i < DMX_CHANNEL_CNT; i++) {
	DMX_ChanInfo_S *ChanInfo = &g_pDmxDevOsi->DmxChanInfo[i];

	if (ChanInfo->DmxId == DmxId) {
	    DMX_OsiCloseChannel(ChanInfo->ChanId);
	    DMX_OsiDestroyChannel(ChanInfo->ChanId);
	}
    }
}

mt_s32 DMX_OsiSaveDmxTs_Start(mt_u32 DmxId, mt_u32 u32RecDmxId)
{
    mt_s32 ret;
    DMX_DEV_OSI_S *DmxDevOsi = g_pDmxDevOsi;
    DMX_Sub_DevInfo_S *DmxInfo = &DmxDevOsi->SubDevInfo[DmxId];
    DMX_PCR_Info_S *PcrInfo = DmxDevOsi->DmxPcrInfo;
    DMX_ChanInfo_S *ChanInfo = DmxDevOsi->DmxChanInfo;
    mt_u32 ChanId = DMX_INVALID_CHAN_ID;
    mt_u32 i;

    ret = DMX_OsiAttachPort(u32RecDmxId, DmxInfo->PortMode, DmxInfo->PortId);
    if (MT_SUCCESS != ret) {
	return ret;
    }

    DMX_OsiGetPmtPid(u32RecDmxId);

    for (i = 0; i < DMX_PCR_CHANNEL_CNT; i++) {
	if ((PcrInfo[i].DmxId == DmxId) && (PcrInfo[i].PcrPid < DMX_INVALID_PID)) {
	    DMX_OsiAddRecPid(u32RecDmxId, PcrInfo[i].PcrPid, &ChanId);
	}
    }

    DMX_OsiAddRecPid(u32RecDmxId, 0, &ChanId); // record pat table

    for (i = 0; i < stPmtPid.u32ProgNum; i++) {
	DMX_OsiAddRecPid(u32RecDmxId, stPmtPid.u32PmtPID[i], &ChanId); // record pmt table
    }

    for (i = 0; i < DMX_CHANNEL_CNT; i++) {
	if ((ChanInfo[i].DmxId == DmxId) && (MT_UNF_DMX_CHAN_CLOSE != ChanInfo[i].ChanStatus)) {
	    DMX_OsiAddRecPid(u32RecDmxId, ChanInfo[i].ChanPid, &ChanId);

#ifdef DMX_DESCRAMBLER_SUPPORT
	    if (DMX_INVALID_KEY_ID != ChanInfo[i].KeyId && DMX_INVALID_CHAN_ID != ChanId) {
		DMX_OsiDescramblerAttach(ChanInfo[i].KeyId, ChanId);
	    }
#endif
	}
    }

    return 0;
}

mt_s32 DMX_OsiSaveDmxTs_Stop(mt_u32 u32RecDmx)
{
    DMX_OsiDetachPort(u32RecDmx);
    DelAllDmxChannel(u32RecDmx);
    return MT_SUCCESS;
}


mt_s32 DMX_OsiSetChannelBufFullCare(mt_u32 ChanId, MT_UNF_DMX_BUF_FULL_CARE_S *ChanBufFull)
{
    mt_s32 ret = MT_SUCCESS;
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = NULL;
    DMX_Sub_DevInfo_S *DmxInfo = NULL;
    mt_u32 buf_id = 0;
    mt_u32 val = 0;

    if (!ChanBufFull || !DmxMgr)
    {
        MT_ERR_DEMUX("%s invalid input param\n", __func__);
        return MT_ERR_DMX_INVALID_PARA;
    }

    ChanInfo = &DmxMgr->DmxChanInfo[ChanId];
    CHECKDMXID(ChanInfo->DmxId);
    DmxInfo = &DmxMgr->SubDevInfo[ChanInfo->DmxId];

    switch (ChanInfo->ChanType)
    {
        case MT_UNF_DMX_CHAN_TYPE_AUD:
        case MT_UNF_DMX_CHAN_TYPE_VID:
		case MT_UNF_DMX_CHAN_TYPE_DSS:
            buf_id = ChanInfo->avChanId;

            if ((DMX_PORT_MODE_RAM == DmxInfo->PortMode) &&
                (DmxInfo->PortId < DMX_RAMPORT_CNT) &&
                (ChanBufFull->pid == ChanInfo->ChanPid))
            {
                val = reg_get_swtsi_chn_af_cfg4(DmxInfo->PortId);

                if (ChanBufFull->es_buf_full)
                {
                    val |= (1 << buf_id);
                }
                else
                {
                    val &= ~(1 << buf_id);
                }

                if (ChanBufFull->dsc_buf_full)
                {
                    val |= (1 << (buf_id + 16));    //av descriptor buf id = av es buf id + 16
                }
                else
                {
                    val &= ~(1 << (buf_id + 16));
                }

                reg_set_swtsi_chn_af_cfg4(DmxInfo->PortId, val);
            }
            else
            {
                MT_ERR_DEMUX("%s set error, PortMode:%d, PortId:%d, pid:%d-%d\n", __func__, DmxInfo->PortMode, DmxInfo->PortId, ChanBufFull->pid, ChanInfo->ChanPid);
            }
        break;

        case MT_UNF_DMX_CHAN_TYPE_SEC:
        case MT_UNF_DMX_CHAN_TYPE_PES:
        case MT_UNF_DMX_CHAN_TYPE_POST:

            if (ChanBufFull->pid == ChanInfo->ChanPid)
            {
                if (ChanBufFull->sec_buf_full)
                {
                    ChanInfo->slot_reg1.bitc.buf_full_mode = 1;
                }
                else
                {
                    ChanInfo->slot_reg1.bitc.buf_full_mode = 0;
                }

                reg_set_demux_slotn_cfg1(ChanInfo->ChanId, ChanInfo->slot_reg1.all);
            }
            else
            {
                MT_ERR_DEMUX("%s set error, pid:%d-%d\n", __func__, ChanBufFull->pid, ChanInfo->ChanPid);
            }

        break;
        default:
            MT_ERR_DEMUX("%s, error ChanType\n", __func__);
        break;

	}

	return ret;
}
/*******************************************************************/
#endif

/*
Function used for clearing the buf, but when do it, software must set the write
pointer, and then set the read pointer, because if setting the read pointer
first, CPU will uses this new read pointer to compare with the write pointer,
it may cases some unexpected result, for example: user gets the value of read
and write pointer register, the buf is empty, but the CPU tell user the buf is
full
*/
mt_s32 DMX_OsiClearEsBuf(mt_u8 index, mt_u32 data)
{
	DMX_ChanEsBuff_S * EsBuff = &g_pDmxDevOsi->DmxChanEsBuff[index]; 
    reg_set_trpp_ch_data_wr_addr_trpp_ch_data_waddr(index, data);
    reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(index, data);
	if((index < 2)&& (EsBuff != NULL))
	{
	  memset((mt_u8 *)EsBuff->data_buff.startVirAddr,  0, EsBuff->data_buff.size);
	}
    return MT_SUCCESS;
}


/*
Function used for clearing the buf, but when do it, software must set the write
pointer, and then set the read pointer, because if setting the read pointer
first, CPU will uses this new read pointer to compare with the write pointer,
it may cases some unexpected result, for example: user gets the value of read
and write pointer register, the buf is empty, but the CPU tell user the buf is
full
*/
mt_s32 DMX_OsiClearDscrptBuf(mt_u8 index, mt_u32 data)
{
    reg_set_trpp_ch_dscrpt_wr_addr_trpp_ch_dscrpt_waddr(index, data);
    reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(index, data);
    return MT_SUCCESS;
}


/*
Function used for clearing the buf, but when do it, software must set the write
pointer, and then set the read pointer, because if setting the read pointer
first, CPU will uses this new read pointer to compare with the write pointer,
it may cases some unexpected result, for example: user gets the value of read
and write pointer register, the buf is empty, but the CPU tell user the buf is
full
*/
mt_s32 DMX_OsiClearRecBuf(mt_u8 index, mt_u32 data)
{
    reg_set_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(index, data);
    reg_set_trpp_ch_rec_rd_addr_trpp_ch_rec_raddr(index, data);
    return MT_SUCCESS;
}


/*
Function used for clearing the buf, but when do it, software must set the write
pointer, and then set the read pointer, because if setting the read pointer
first, CPU will uses this new read pointer to compare with the write pointer,
it may cases some unexpected result, for example: user gets the value of read
and write pointer register, the buf is empty, but the CPU tell user the buf is
full
*/
mt_s32 DMX_OsiClearIndexBuf(mt_u8 index, mt_u32 data)
{
    reg_set_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(index, data);
    reg_set_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(index, data);
    return MT_SUCCESS;
}

#if !defined(CONFIG_MT_CHIP_SYMPHONY4) && !defined(CONFIG_MT_CHIP_SYMPHONY6)
static mt_s32 DMX_OsiDmxRegRevert(int tag)
{
    mt_u32 reg_tsi_reset = 0;
    int i = 0;
    dmx_all_reg_s *dmx_reg = NULL;
    mt_u32 data = 0;
    mt_u32 reg = 0;
    mt_s32 ret = MT_SUCCESS;
    
    if (NULL == dmx_reg)
    {
        dmx_reg = kzalloc(sizeof(dmx_all_reg_s), GFP_KERNEL);
        if (NULL == dmx_reg)
        {
            MT_ERR_DEMUX("kzalloc dmx_reg failed\n");
            return ;
        }
    }

    if (0 == tag)
    {
        memset(dmx_reg, 0, sizeof(dmx_all_reg_s));
        dmx_reg->tsi_in[0] = reg_get_ts0_sample_ctrl();        
        dmx_reg->tsi_in[1] = reg_get_ts1_sample_ctrl();
        dmx_reg->tsi_in[2] = reg_get_ts2_sample_ctrl();
        dmx_reg->tsi_in[3] = reg_get_ts3_sample_ctrl();
        MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", 0xbf200000, dmx_reg->tsi_in[0]);
        MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", 0xbf200010, dmx_reg->tsi_in[1]);
        MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", 0xbf200020, dmx_reg->tsi_in[2]);
        MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", 0xbf200030, dmx_reg->tsi_in[3]);

        reg_set_ts0_sample_ctrl(0);
        reg_set_ts1_sample_ctrl(0);
        reg_set_ts2_sample_ctrl(0);
        reg_set_ts3_sample_ctrl(0);
        msleep(100);    
        
        for (i = 0; i < SLOT_REG_COUNT; i++)
        {
            dmx_reg->slotn_cfg0[i] = reg_get_demux_slotn_cfg0(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_demux_slotn_cfg0 + i*8, dmx_reg->slotn_cfg0[i]);
        }
		
        for (i = 0; i < SLOT_REG_COUNT; i++)
        {
            dmx_reg->slotn_cfg1[i] = reg_get_demux_slotn_cfg1(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_demux_slotn_cfg1 + i*8, dmx_reg->slotn_cfg1[i]);
        }

#if defined(CONFIG_MT_CHIP_SYMPHONY1)        
        dmx_reg->ds.cw_op = reg_get_tsi_ds_cw_op();
        MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ds_cw_op, dmx_reg->ds.cw_op);
#endif

        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->ds.tscfg[i] = reg_get_tsi_ds_chn_tscfg(i); //?????odd key, is the reg addr is right?
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ds_chn_tscfg + i*0x80, dmx_reg->ds.tscfg[i]);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->ds.core[i] = reg_get_tsi_ds_core(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ds_core + i*0x80, dmx_reg->ds.core[i]);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->ds.ive[i] = reg_get_tsi_aes_ive(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_aes_ive + i*0x80, dmx_reg->ds.ive[i]);
        }
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->ds.mode[i] = reg_get_tsi_ades_disc_mode(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ades_disc_mode + i*0x80,dmx_reg->ds.mode[i]);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->ds.pktmode[i] = reg_get_tsi_ades_pktmode(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_tsi_ades_pktmode + i*0x80,dmx_reg->ds.pktmode[i]);
        }

        for (i = 0; i < SLOT_REG_COUNT; i++)
        {
            dmx_reg->sf.staddr[i] = reg_get_bufn_staddr(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_bufn_staddr + i*0x04,dmx_reg->sf.staddr[i]);
        }
		
        for (i = 0; i < SLOT_REG_COUNT; i++)
        {
            dmx_reg->sf.size[i] = reg_get_bufn_size(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_bufn_size + i*0x04,dmx_reg->sf.size[i]);
        }
       
        for (i = 0; i < SLOT_REG_COUNT; i++)
        {
            dmx_reg->sf.int_cfg[i] = reg_get_bufn_ts_int_cfg(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_bufn_ts_int_cfg + i*0x04,dmx_reg->sf.int_cfg[i]);
        }

        for (i = 0; i < SLOT_REG_COUNT; i++)
        {
            dmx_reg->sf.filter_config[i] = reg_get_filtern_config(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_filtern_config + i*0x04,dmx_reg->sf.filter_config[i]);
        }
		
        for (i = 0; i < SLOT_REG_COUNT; i++)
        {
            dmx_reg->sf.filter_data[i] = reg_get_funit_filter_data(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_funit_filter_data + i*0x04,dmx_reg->sf.filter_data[i]);
        }
		
        for (i = 0; i < SLOT_REG_COUNT; i++)
        {
            dmx_reg->sf.filter_mask[i] = reg_get_funit_filter_mask(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_funit_filter_mask + i*0x04,dmx_reg->sf.filter_mask[i]);
        }
		
        for (i = 0; i < SLOT_REG_COUNT; i++)
        {
            dmx_reg->sf.filter_mode[i] = reg_get_funit_filter_mode(i);
            MT_DBG_DEMUX("reg[0x%x] tmp save val=[0x%x]\n", reg_dmx_funit_filter_mode + i*0x04,dmx_reg->sf.filter_mode[i]);
        }
        
        dmx_reg->trpp.trpp_global[0] = reg_get_trpp_channel_parse_en();
        dmx_reg->trpp.trpp_global[1] = reg_get_trpp_bus_urgent();
        dmx_reg->trpp.trpp_global[2] = reg_get_trpp_channel_record_en();
        dmx_reg->trpp.trpp_global[3] = reg_get_trpp_sc_index_flt1_4();
        dmx_reg->trpp.trpp_global[4] = reg_get_trpp_sc_index_flt5_8();
        dmx_reg->trpp.trpp_global[5] = reg_get_trpp_sc_index_flt9_10();
        dmx_reg->trpp.trpp_global[6] = reg_get_trpp_sc_index_flt11_12();
        dmx_reg->trpp.trpp_global[7] = reg_get_trpp_sc_index_flt0();
        dmx_reg->trpp.trpp_global[8] = reg_get_trpp_esbuf_ch();
    
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_property[i] = reg_get_trpp_ch_property(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_parse_set[i] = reg_get_trpp_ch_parse_set(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_start_code1[i] = reg_get_trpp_ch_start_code1(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_frm_start_code_m1[i] = reg_get_trpp_ch_frm_start_code_m1(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_start_code2[i] = reg_get_trpp_ch_start_code2(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_frm_start_code_m2[i] = reg_get_trpp_ch_frm_start_code_m2(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_dscrpt_start_addr[i] = reg_get_trpp_ch_dscrpt_start_addr(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_data_start_addr[i] = reg_get_trpp_ch_data_start_addr(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_dscrpt_end_addr[i] = reg_get_trpp_ch_dscrpt_end_addr(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch_data_end_addr[i] = reg_get_trpp_ch_data_end_addr(i);
        }
        
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch1_ini_info1[i] = reg_get_trpp_ch1_ini_info1(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch1_ini_info2[i] = reg_get_trpp_ch1_ini_info2(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch1_ini_info3[i] = reg_get_trpp_ch1_ini_info3(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch1_ini_info4[i] = reg_get_trpp_ch1_ini_info4(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch1_ini_info5[i] = reg_get_trpp_ch1_ini_info5(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch1_ini_info6[i] = reg_get_trpp_ch1_ini_info6(i);
        }
		
        for (i=0; i<CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch1_ini_info7[i] = reg_get_trpp_ch1_ini_info7(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch1_ini_info8[i] = reg_get_trpp_ch1_ini_info8(i);
        }
		
        for (i = 0; i < CHANNEL_REG_COUNT; i++)
        {
            dmx_reg->trpp.ch1_ini_info9[i] = reg_get_trpp_ch1_ini_info9(i);
        }
		
        for (i = 0; i < 4; i++)
        {
            dmx_reg->trpp.ch_rec_set[i] = reg_get_trpp_ch_rec_set(i);
        }
		
        for (i = 0; i < 4; i++)
        {
            dmx_reg->trpp.rec_start_addr[i] = reg_get_trpp_ch_rec_start_addr(i);
        }
		
        for (i = 0; i < 4; i++)
        {
            dmx_reg->trpp.rec_end_addr[i] = reg_get_trpp_ch_rec_end_addr(i);
        }

        reg_tsi_reset = dmx_inl(0xbf51001c);
		
        dmx_outl(0xbf51001c, (reg_tsi_reset & 0xfffff000));
        
        MT_DBG_DEMUX("inl(0xbf51001c) ====close==== 0x%x\n", dmx_inl(0xbf51001c));

    }

    
    msleep(100);
    dmx_outl(0xbf51001c, (reg_tsi_reset | 0xfff));
    reg_dmx_init();
    DMX_OsiInitChInit(0x3, 0x3, 0x2, 0x4, 0x4, 0x2); 
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        if (0 == reg_get_demux_slotn_cfg0(i))
        {
            reg_set_demux_slotn_cfg0(i, dmx_reg->slotn_cfg0[i] );
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_demux_slotn_cfg0 + i*0x08, dmx_reg->slotn_cfg0[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        if (0 == reg_get_demux_slotn_cfg1(i))
        {
            reg_set_demux_slotn_cfg1(i, dmx_reg->slotn_cfg1[i] );
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_demux_slotn_cfg1 + i*0x08, dmx_reg->slotn_cfg1[i]);
    }

#if defined(CONFIG_MT_CHIP_SYMPHONY1)	
    reg_set_tsi_ds_cw_op(dmx_reg->ds.cw_op);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ds_cw_op, dmx_reg->ds.cw_op);
#endif

    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        if (0 == reg_get_tsi_ds_chn_tscfg(i))
        {
            reg_set_tsi_ds_chn_tscfg(i, dmx_reg->ds.tscfg[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ds_chn_tscfg + i*0x80, dmx_reg->ds.tscfg[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        if (0 == reg_get_tsi_ds_core(i))
        {
            reg_set_tsi_ds_core(i, dmx_reg->ds.core[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ds_core + i*0x80, dmx_reg->ds.core[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        if(reg_get_tsi_aes_ive(i) == 0)
        {
            reg_set_tsi_aes_ive(i, (u32)dmx_reg->ds.ive[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_aes_ive + i * 0x80, dmx_reg->ds.ive[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        if (0 == reg_get_tsi_ades_disc_mode(i))
        {
            reg_set_tsi_ades_disc_mode(i, (u32)dmx_reg->ds.mode[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ades_disc_mode + i*0x80, dmx_reg->ds.mode[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        if (0 == reg_get_tsi_ades_pktmode(i))
        {
            reg_set_tsi_ades_pktmode(i, (u32)dmx_reg->ds.pktmode[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_tsi_ades_pktmode + i*0x80, dmx_reg->ds.pktmode[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        if (0 == reg_get_bufn_staddr(i))
        {
            reg_set_bufn_staddr(i, (u32)dmx_reg->sf.staddr[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_bufn_staddr + i*0x04, dmx_reg->sf.staddr[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        if (0 == reg_get_bufn_size(i))
        {
            reg_set_bufn_size(i, (u32)(dmx_reg->sf.size[i] & 0xffff));
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_bufn_size + i*0x04, dmx_reg->sf.size[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        if (0 == reg_get_bufn_ts_int_cfg(i))
        {
            reg_set_bufn_ts_int_cfg(i, (u32)dmx_reg->sf.int_cfg[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_bufn_ts_int_cfg + i*0x04, dmx_reg->sf.int_cfg[i]);
    } 
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        if (0 == reg_get_filtern_config(i))
        {
            reg_set_filtern_config(i, (u32)dmx_reg->sf.filter_config[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_filtern_config + i*0x04, dmx_reg->sf.filter_config[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        if (0 == reg_get_funit_filter_data(i))
        {
            reg_set_funit_filter_data(i, (u32)dmx_reg->sf.filter_data[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_funit_filter_data + i*0x04, dmx_reg->sf.filter_data[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        if (0 == reg_get_funit_filter_mask(i) == 0)
        {
            reg_set_funit_filter_mask(i, (u32)dmx_reg->sf.filter_mask[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_funit_filter_mask + i*0x04, dmx_reg->sf.filter_mask[i]);
    }
    
    for (i = 0; i < SLOT_REG_COUNT; i++)
    {
        if (0 == reg_get_funit_filter_mode(i))
        {
            reg_set_funit_filter_mode(i, (u32)dmx_reg->sf.filter_mode[i]);
        }
    	
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_funit_filter_mode + i*0x04, dmx_reg->sf.filter_mode[i]);
    }
    
    reg_set_trpp_channel_parse_en(dmx_reg->trpp.trpp_global[0]);
    reg_set_trpp_bus_urgent(dmx_reg->trpp.trpp_global[1]);
    reg_set_trpp_channel_record_en(dmx_reg->trpp.trpp_global[2]);
    reg_set_trpp_sc_index_flt1_4(dmx_reg->trpp.trpp_global[3]);
    reg_set_trpp_sc_index_flt5_8(dmx_reg->trpp.trpp_global[4]);
    reg_set_trpp_sc_index_flt9_10(dmx_reg->trpp.trpp_global[5]);
    reg_set_trpp_sc_index_flt11_12(dmx_reg->trpp.trpp_global[6]);
    reg_set_trpp_sc_index_flt0(dmx_reg->trpp.trpp_global[7]);
    reg_set_trpp_esbuf_ch(dmx_reg->trpp.trpp_global[8]);
    
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_channel_parse_en, dmx_reg->trpp.trpp_global[0]);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_bus_urgent, dmx_reg->trpp.trpp_global[1]);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_channel_record_en, dmx_reg->trpp.trpp_global[2]);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt1_4, dmx_reg->trpp.trpp_global[3]);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt5_8, dmx_reg->trpp.trpp_global[4]);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt9_10, dmx_reg->trpp.trpp_global[5]);       
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt11_12, dmx_reg->trpp.trpp_global[6]);       
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_sc_index_flt0, dmx_reg->trpp.trpp_global[7]);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_esbuf_ch, dmx_reg->trpp.trpp_global[8]);
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_property(i, dmx_reg->trpp.ch_property[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_property, dmx_reg->trpp.ch_property[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_parse_set(i, dmx_reg->trpp.ch_parse_set[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_parse_set, dmx_reg->trpp.ch_parse_set[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_start_code1(i, (u32)dmx_reg->trpp.ch_start_code1[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_start_code1, dmx_reg->trpp.ch_start_code1[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_frm_start_code_m1(i, (u32)dmx_reg->trpp.ch_frm_start_code_m1[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_frm_start_code_m1, dmx_reg->trpp.ch_frm_start_code_m1[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_start_code2(i, (u32)dmx_reg->trpp.ch_start_code2[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_start_code2, dmx_reg->trpp.ch_start_code2[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_frm_start_code_m2(i, (u32)dmx_reg->trpp.ch_frm_start_code_m2[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_frm_start_code_m2, dmx_reg->trpp.ch_frm_start_code_m2[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_dscrpt_start_addr(i, (u32)dmx_reg->trpp.ch_dscrpt_start_addr[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_dscrpt_start_addr, dmx_reg->trpp.ch_dscrpt_start_addr[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_data_start_addr(i, (u32)dmx_reg->trpp.ch_data_start_addr[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_data_start_addr, dmx_reg->trpp.ch_data_start_addr[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_dscrpt_end_addr(i, (u32)dmx_reg->trpp.ch_dscrpt_end_addr[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_dscrpt_end_addr, dmx_reg->trpp.ch_dscrpt_end_addr[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch_data_end_addr(i, (u32)dmx_reg->trpp.ch_data_end_addr[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_data_end_addr, dmx_reg->trpp.ch_data_end_addr[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info1(i, (u32)dmx_reg->trpp.ch1_ini_info1[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info1, dmx_reg->trpp.ch1_ini_info1[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info2(i, (u32)(u32)dmx_reg->trpp.ch1_ini_info2[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info2, dmx_reg->trpp.ch1_ini_info2[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info3(i, (u32)(u32)dmx_reg->trpp.ch1_ini_info3[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info3, dmx_reg->trpp.ch1_ini_info3[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info4(i, (u32)(u32)dmx_reg->trpp.ch1_ini_info4[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info4, dmx_reg->trpp.ch1_ini_info4[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info5(i, (u32)(u32)dmx_reg->trpp.ch1_ini_info5[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info5, dmx_reg->trpp.ch1_ini_info5[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info6(i, (u32)(u32)dmx_reg->trpp.ch1_ini_info6[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info6, dmx_reg->trpp.ch1_ini_info6[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info7(i, (u32)(u32)dmx_reg->trpp.ch1_ini_info7[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info7, dmx_reg->trpp.ch1_ini_info7[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info8(i, (u32)(u32)dmx_reg->trpp.ch1_ini_info8[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info8, dmx_reg->trpp.ch1_ini_info8[i]);
    }
    
    for (i = 0; i < CHANNEL_REG_COUNT; i++)
    {
        reg_set_trpp_ch1_ini_info9(i, (u32)(u32)dmx_reg->trpp.ch1_ini_info9[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch1_ini_info9, dmx_reg->trpp.ch1_ini_info9[i]);
    }
    
    for (i = 0; i < 4; i++)
    {
        reg_set_trpp_ch_rec_set(i, (u32)(u32)dmx_reg->trpp.ch_rec_set[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_rec_set, dmx_reg->trpp.ch_rec_set[i]);
    }
    
    for (i = 0; i < 4; i++)
    {
        reg_set_trpp_ch_rec_start_addr(i, (u32)(u32)dmx_reg->trpp.rec_start_addr[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_rec_start_addr, dmx_reg->trpp.rec_start_addr[i]);
    }
    
    for (i = 0; i < 4; i++)
    {
        reg_set_trpp_ch_rec_end_addr(i, (u32)(u32)dmx_reg->trpp.rec_end_addr[i]);
        MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_trpp_ch_rec_end_addr, dmx_reg->trpp.rec_end_addr[i]);
    }
    
    data = reg_get_pvr_int_mask();		
    DMX_OsiInitChInit(0x3, 0x3, 0x2, 0x4, 0x4, 0x2); 
    reg_set_pvr_int_mask(data | 0x110022);
    reg_set_pvr_int_clr(1 << 24);
    reg_set_trpp4_int_clr(1 << 24);
    
    DMX_OsiHardwareInit(1, 0xf,  NULL);
    //MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", TSP_THRESHOLD, 0x03030404);
    /*
    *  set hardware check es overflow for swsti
    */
    reg_set_swtsi_chn_af_cfg4(0, 0x0);
    reg_set_swtsi_chn_af_cfg5(0, 0x0);
    reg_set_swtsi_chn_af_cfg4(1, 0x0);
    reg_set_swtsi_chn_af_cfg5(1, 0x0); 
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_swtsi_chn_af_cfg4, 0);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_swtsi_chn_af_cfg5, 0);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_swtsi_chn_af_cfg4+0x100, 0);
    MT_DBG_DEMUX("reg[0x%x] restore from val=[0x%x]\n", reg_dmx_swtsi_chn_af_cfg5+0x100, 0);
	
    /*
    *     add for Bug 95689 - [通久]弱信号老化死机
    */   
	
	reg = dmx_inl(0xbf2702bc);
    reg = reg | 0x1;
    dmx_outl(0xbf2702bc, reg);        
    
    if (dmx_reg)    
    {
        kfree(dmx_reg);
    }

	return MT_SUCCESS;
}


/*
function is created for the IC bug, used for store the dmx register sta, clean
the register of ts_sample_sta err bit(24) and revert the dmx register
*/
static void DMX_OsiRestDmxForWeakSignal(struct work_struct *pdmx_reset_work)
{
    mt_u32  err_mask = 0;

    if (NULL == pdmx_reset_work)
    {
        MT_ERR_DEMUX("pdmx_reset_work is null\n");
        return;
    }

    err_mask = (TS_SAMPLE_STA_TS_LOCK | TS_SAMPLE_STA_SYNC_LOCK | TS_SAMPLE_STA_IC_ERROR);
	
    if ((err_mask == (reg_get_ts0_sample_sta() & err_mask)) ||
        (err_mask == (reg_get_ts1_sample_sta() & err_mask)) ||
        (err_mask == (reg_get_ts2_sample_sta() & err_mask)) ||
        (err_mask == (reg_get_ts3_sample_sta() & err_mask)))
    {
        DMX_OsiDmxRegRevert(0);   
		
        MT_ERR_DEMUX("%x-%x-%x-%x\n",
			reg_get_ts0_sample_sta(), reg_get_ts1_sample_sta(),
			reg_get_ts2_sample_sta(), reg_get_ts3_sample_sta());
    }

    schedule_delayed_work(&dmx_hw_reset_delayed_work, DMX_HW_RESET_DELAYED_TIME*HZ);
	
    return;
}
#endif

EXPORT_SYMBOL(DMX_OsiClearIndexBuf);

static int dmx_get_ves_buffer_frame_count(int playch)
{
    mt_s32 cnt  = 0;
    ulong start1, end1, start2, end2;
    ulong* dscrpt;
    mt_u32 descriptor_readpointer = 0;
    mt_u32 descriptor_writepointer = 0;
    ulong end = 0;
    DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
    DMX_ChanEsBuff_S *esBuff = NULL;
    ulong descriptor_virt_start = 0;

    esBuff = &pDmxDevOsi->DmxChanEsBuff[playch];
    descriptor_virt_start = (ulong)esBuff->desc_buff.startVirAddr;
    end = esBuff->desc_buff.size;

    DMXOsiChnEsDescDataGetReadWrite(playch, &descriptor_writepointer, &descriptor_readpointer);

    if (descriptor_readpointer < descriptor_writepointer)
    {
        start1 = descriptor_readpointer;
        end1 = descriptor_writepointer;
        start2 = end2 = 0;
    }
    else if (descriptor_readpointer > descriptor_writepointer)
    {
        start1 = descriptor_readpointer;
        end1 = end;
        start2 = 0;
        end2 = descriptor_writepointer;
    }
    else
    {
        return 0;
    }

    while (start1 < end1)
    {
        dscrpt = (ulong*)(descriptor_virt_start + start1);

        if (0 == (dscrpt[0] & 1))
        {
            cnt++;
        }

        start1 += 16;
    }

    while (start2 < end2)
    {
        dscrpt = (ulong*)(descriptor_virt_start + start2);

        if (0 == (dscrpt[0] & 1))
        {
            cnt++;
        }

        start2 += 16;
    }

    return cnt;
}

void CheckVdecHang(unsigned long data)
{
    DMX_RamPort_Info_S *PortInfo = &g_pDmxDevOsi->RamPortInfo[data];
    mt_u32 vdec_rp = 0;
    int PortId = data;
    DMX_ChanInfo_S *ChanInfo = NULL;
	mt_u32 reg_value = 0xffffffff;
    mt_u8 ves_ch = 0xff;
    mt_u8 aves_ch = 0xff;
    mt_u32 v_ch_id = DMX_INVALID_CHAN_ID;
	mt_u8  ram_num = 0;
	int i=0;
	
	reg_value = dmx_inl(reg_dmx_regs_base_vit + 0x6002c);

	if (0x1 == (reg_get_swtsi_chn_af_cfg4(PortId) & 0x1))
	{
		for (i=0; i<4; i++)
		{
			//the value of 0x0F is invalid for dmx's es buf channel, refer to DMX_AV_CHANNEL_CNT
			//bit3-bit0 is for vdec0, bit7-bit4 is for vdec1, bit11-bit8 is for vdec2, bit15-bit12 is for vdec3
			ves_ch = (reg_value >> (i*4)) & 0x0F;
			if (ves_ch != 0x0F) 
			{
				v_ch_id = g_pDmxDevOsi->DmxChanEsBuff[ves_ch].ch_id; //find which slot is used according to ves_ch 
		        if ((DMX_INVALID_CHAN_ID == v_ch_id) || (v_ch_id >= DMX_CHANNEL_CNT))
		        {
		            MT_INFO_DEMUX("vid chid invalid, v_ch_id=0x%x\n", v_ch_id);
		            continue;
		        }

				ram_num = reg_get_demux_slotn_cfg0_src(v_ch_id) - 4;//the ram's num is start from 4 in register
				if (ram_num != PortId)
				{
					MT_INFO_DEMUX("this channel's source is not match with PortId, ram_num=%d, PortId=%d\n", ram_num, PortId);
					continue;
				}

		        ChanInfo = &g_pDmxDevOsi->DmxChanInfo[v_ch_id];
		        if (ChanInfo->ChanType != MT_UNF_DMX_CHAN_TYPE_VID)
		        {
		        	MT_INFO_DEMUX("ChanInfo->ChanType=%d, not MT_UNF_DMX_CHAN_TYPE_VID\n", ChanInfo->ChanType);
		            return;
		        }

				aves_ch = reg_get_trpp_channel_parse_en();
		        if (!(aves_ch & (1 << ves_ch)))
		        {
		        	MT_INFO_DEMUX("there is no video channel, aves_ch=0x%x, ves_ch=0x%x\n", aves_ch, ves_ch);
		            return;
		        }			
				
		        vdec_rp = vdec_get_ves_rd(i);//get the read point according to vdec 		
				DMX_Proc_ChanBuf_S	BufInfo;
				DMX_OsiGetChanBufProc(ChanInfo->ChanId, &BufInfo);
				if (BufInfo.DataWrite != 0)
				{
		       		reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ves_ch, vdec_rp);
				}
				
		        if (PortInfo->LastVdecRp == vdec_rp)
		        {
		            PortInfo->VdecRpNotMoveCount++;
		        }
		        else
		        {
		            PortInfo->VdecRpNotMoveCount = 0;
		            PortInfo->Lasttime = jiffies;
		        }

		        PortInfo->LastVdecRp = vdec_rp;
		        if ((PortInfo->VdecRpNotMoveCount >= 50) && (jiffies - PortInfo->Lasttime > 1000) )
		        {
		            DMX_ChanInfo_S *aChanInfo = DMX_OsiGetAudioChanInfo();
		            mt_u32  UsedPercent = 0;
		            mt_u32  aUsedPercent = 0;
		            mt_u32  vPread = 0;
		            DMX_Proc_ChanBuf_S  BufInfo;
		            DMX_OsiGetChanBufProc(ChanInfo->ChanId, &BufInfo);

		            if (BufInfo.DescDepth && (BufInfo.BlockSize != 0))
		            {
		                if (BufInfo.DataRead <= BufInfo.DataWrite)
		                {
		                    UsedPercent = (BufInfo.DataWrite - BufInfo.DataRead) * 100 / BufInfo.BlockSize;
		                }
		                else
		                {
		                    UsedPercent = (BufInfo.BlockSize + BufInfo.DataWrite - BufInfo.DataRead) * 100 / BufInfo.BlockSize;
		                }

		                vPread = BufInfo.DataRead;
		            }

		            if (aChanInfo != NULL)
		            {
		                DMX_OsiGetChanBufProc(aChanInfo->ChanId, &BufInfo);

		                if (BufInfo.DescDepth && (BufInfo.BlockSize != 0))
		                {
		                    if (BufInfo.DataRead <= BufInfo.DataWrite)
		                    {
		                        aUsedPercent = (BufInfo.DataWrite - BufInfo.DataRead) * 100 / BufInfo.BlockSize;
		                    }
		                    else
		                    {
		                        aUsedPercent = (BufInfo.BlockSize + BufInfo.DataWrite - BufInfo.DataRead) * 100 / BufInfo.BlockSize;
		                    }
		                }
		            }

		            if ((UsedPercent > 50) || (aUsedPercent > 50))
		            {
		                if (dmx_get_ves_buffer_frame_count(ves_ch) < 4)
		                {
							DMX_OsiGetChanBufProc(ChanInfo->ChanId, &BufInfo);
		                    DMX_OsiClearEsBuf(ves_ch, BufInfo.DataWrite);
							DMX_OsiClearDscrptBuf(ves_ch, BufInfo.DescWrite);
							
		                    if (aChanInfo != NULL)
		                    {
								DMX_OsiGetChanBufProc(aChanInfo->ChanId, &BufInfo);
		                        DMX_OsiClearEsBuf(aChanInfo->avChanId, BufInfo.DataWrite);
								DMX_OsiClearDscrptBuf(aChanInfo->avChanId, BufInfo.DescWrite);
								mutex_lock(&g_demux_ch_mutex);
		                        g_audioEsBufCleared = 1;
								mutex_unlock(&g_demux_ch_mutex);
		                    }

		                    MT_ERR_DEMUX("no valid vdec data vdec not move %d ms  BufInfo.DataRead %d, BufInfo.DataWrite %d , UsedPercent %d aUsedPercent %d \n",
		                        jiffies - PortInfo->Lasttime,  BufInfo.DataRead, BufInfo.DataWrite, UsedPercent, aUsedPercent );

		                    PortInfo->VdecRpNotMoveCount = 0;
		                    PortInfo->Lasttime = jiffies;
		                }
		            }
		        }
			}
			else
			{
				continue;
			}
		}    
	}
	else
	{
		for (i=0; i<4; i++)
		{
			//the value of 0x0F is invalid for dmx's es buf channel, refer to DMX_AV_CHANNEL_CNT
			//bit3-bit0 is for vdec0, bit7-bit4 is for vdec1, bit11-bit8 is for vdec2, bit15-bit12 is for vdec3
			ves_ch = (reg_value >> (i*4)) & 0x0F;
			if (ves_ch != 0x0F) 
			{
				v_ch_id = g_pDmxDevOsi->DmxChanEsBuff[ves_ch].ch_id; //find which slot is used according to ves_ch 
		        if ((DMX_INVALID_CHAN_ID == v_ch_id) || (v_ch_id >= DMX_CHANNEL_CNT))
		        {
		            MT_INFO_DEMUX("vid chid invalid, v_ch_id=0x%x\n", v_ch_id);
		            continue;
		        }


		        ChanInfo = &g_pDmxDevOsi->DmxChanInfo[v_ch_id];
		        if (ChanInfo->ChanType != MT_UNF_DMX_CHAN_TYPE_VID)
		        {
		        	MT_INFO_DEMUX("ChanInfo->ChanType=%d, not MT_UNF_DMX_CHAN_TYPE_VID\n", ChanInfo->ChanType);
		            return;
		        }

				aves_ch = reg_get_trpp_channel_parse_en();
		        if (!(aves_ch & (1 << ves_ch)))
		        {
		        	MT_INFO_DEMUX("there is no video channel, aves_ch=0x%x, ves_ch=0x%x\n", aves_ch, ves_ch);
		            return;
		        }			
				
		        vdec_rp = vdec_get_ves_rd(i);//get the read point according to vdec 		
				DMX_Proc_ChanBuf_S	BufInfo;
				DMX_OsiGetChanBufProc(ChanInfo->ChanId, &BufInfo);
				if (BufInfo.DataWrite != 0)
				{
		       		reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(ves_ch, vdec_rp);
				}
			}
		}
	}
}

mt_void DMX_OsiT2miSoftReset()
{
	ulong crm_base = 0;
	mt_u32 value = 0;
	
	//reset t2mi before set pid
	crm_base = mt_get_crm_base();
	value = (*(volatile mt_u32 *)(crm_base + 0xB00C));
	value &= (~(1<<18)); //bit18 clear to 0
	(*(volatile mt_u32 *)(crm_base + 0xB00C)) = value;
	msleep(1);
	value |= (1<<18); //bit18 set to 1
	(*(volatile mt_u32 *)(crm_base + 0xB00C)) = value;		
}

mt_s32 DMX_OsiSoftReset(void)
{
	mt_u8 i = 0;
	mt_u8 wait_flag = 1;
	mt_u8 wait_count = 0;

	mt_u32 value;
	ulong crm_base = mt_get_crm_base();
	
	// 1. close ts sample
	reg_set_ts0_sample_ctrl_syncoff_th(0);

	//close swtsi channel
	for (i=0; i<DMX_RAMPORT_CNT; i++)
	{
		reg_set_swtsi_chn_control_ch_enable(i, 0);
	}

	//close slot
	for (i=0; i<SLOT_REG_COUNT; i++) 
	{
		reg_set_demux_slotn_cfg0_slot_en(i, 0);
	}

	// 2. wait CH_BUSY_STATE is idle
	wait_count = 0;
	while (wait_flag)
	{
		wait_count++;
		wait_flag = 0;
		for (i=0; i<DMX_RAMPORT_CNT; i++)
		{
			// if one of the channel is 1, still wait
			if (reg_get_swtsi_chn_state_ch_busy_state(i))// 0:idle, 1:busy
			{
				wait_flag = 1;
				break;
			}
		}
		if (wait_count > 200)
		{
			break;
		}
		msleep(10);
	}

	// 3. wait dmx state is idle	
	wait_count = 0;
	wait_flag = 1;
	while (wait_flag)
	{
		wait_count++;		
		wait_flag = reg_get_demux_state_demux_busy();//0:idle, 1:busy
		
		if (wait_count > 200)
		{
			break;
		}
		msleep(10);
	}

	// 4. wait ds state is idle
	//TODO

	// 5. wait ts parse and record channel clear finished
	wait_count = 0;
	wait_flag = 1;
	while (wait_flag)
	{
		wait_count++;		
		wait_flag = (reg_get_trpp_ch_clear_status_trpp_ch_clr_ok()) ? 0 : 1;//0:busy, 1:idle
		
		if (wait_count > 200)
		{
			break;
		}
		msleep(10);
	}

	// 6. wait sf parse channel
	wait_count = 0;
	wait_flag = 1;
	while (wait_flag)
	{
		wait_count++;		
		wait_flag = (reg_get_dmx_sf_process_sta() & 0x01) ? 0 : 1;//0:busy, 1:idle
		
		if (wait_count > 200)
		{
			break;
		}
		msleep(10);
	}

	value = (*(volatile mt_u32 *)(crm_base + 0xB00C));
	value &= (~(0x07)); //bit2-bit0 clear to 0
	(*(volatile mt_u32 *)(crm_base + 0xB00C)) = value;
	msleep(1);
	value |= 0x07; //bit2-bit0 set to 1
	(*(volatile mt_u32 *)(crm_base + 0xB00C)) = value;

	value = reg_get_ts0_sample_ctrl();
	if (0x470001FF == value)
	{
		reg_dmx_init();
		printk(KERN_ERR "[%s %d]softreset success\n", __FUNCTION__, __LINE__);
		return MT_SUCCESS;
	}
	else
	{
		printk(KERN_ERR "[%s %d]softreset failure\n", __FUNCTION__, __LINE__);
		return MT_FAILURE;
	}
}

mt_void DMX_OsiDeviceHardInit(mt_void)
{
	mt_u32 glb_mask = 0;
	
	DmxReset();    
    DMX_OsiTsiRegisterInit();
    DMX_OsiHardwareInit(1, 0xf,  NULL);

    reg_set_ts_sample_int_mask(0x42108);
    reg_set_ts_sample_int_clr(1 << 20);
    reg_set_pvr_int_mask(0x333333);
    reg_set_pvr_int_clr(1 << 24);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	reg_set_pvr2_int_mask((3<<16)|(3<<12));
    reg_set_pvr2_int_rd(1);
#endif

    reg_set_trpp5_int_mask(0xffff);
    reg_set_trpp5_int_clr(1 << 20);
	
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	reg_set_trpp10_int_mask(0x07);
	reg_set_trpp10_int_clr(1 << 31);
#endif

#if 0
	reg_set_trpp0_int_mask(0x7ffffff);
    reg_set_trpp0_int_clr(1 << 28);
#endif

    /*
    because PVR INT would cause GLB INT, so, after register PVR INT, clearing the
    bit13 in registor GGLB_INT_MASK is needed, if not, one interrupt source will
    cause two interrupt
    */
    glb_mask = reg_get_dmx_gglb_int_mask();
    glb_mask &= ~(DMX_GGLB_INT_PVR);
    
    reg_set_dmx_gglb_int_mask(glb_mask);
    reg_set_dmx_gglb_int_clr(1 << 24);
}

mt_s32 DMX_DRV_FastPlay_Start(mt_u8 swtsi_num, mt_u8 rec_id, mt_u8 node_num)
{
	mt_s32 ret = MT_SUCCESS;
	mt_u32 reg_value;
	
	reg_value = reg_get_demux_lln_num_start();
	reg_value &= (~(7<<0));//bit2-0 clear to 0
	reg_value |= (node_num&0x07);
	reg_set_demux_lln_num_start(reg_value);

	reg_value = reg_get_demux_fp_swtsi_ch_set();
	reg_value |= 0x10;//use big eddian
	reg_set_demux_fp_swtsi_ch_set(reg_value);

	reg_value = reg_get_demux_fp_set_cfg();
	reg_value &= (~((1<<16) | (7<<8) | (3<<4) | (7<<0)));//bit16, bit10-8, bit5-4, bit2-0 clear to 0
	reg_value |= (1<<16) | (5<<8) | ((swtsi_num&0x03)<<4) | (rec_id&0x07);//start FP, interval time is 1.966ms
	reg_set_demux_fp_set_cfg(reg_value);
	
	return ret;
}

mt_s32 DMX_DRV_FastPlay_Stop(mt_void)
{
	mt_s32 ret = MT_SUCCESS;
	mt_u32 reg_value;
	mt_u32 time_out = 10;

	reg_value = reg_get_demux_fp_set_cfg();
	reg_value &= (~(1<<16));//bit16 clear to 0, stop FP
	reg_set_demux_fp_set_cfg(reg_value);

	reg_value = reg_get_demux_fp_status();
	while(((reg_value & 0x01) == 0) && (time_out--))
    {
        msleep(2);
    }
	
	return ret;
}

mt_s32 DMX_GetChanRef(mt_u32 ChannelId)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
 	DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChannelId];

	if (0 == ChanInfo->u32countRef)
	{
		return MT_SUCCESS;
	}
	else
	{
		return MT_FAILURE;
	}
}

mt_s32 DMX_GetChanAVId(mt_u32 ChannelId)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
 	DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChannelId];
	if(ChanInfo != NULL)
	{
		return ChanInfo->avChanId;
	}
	else
	{
		return 0xFF;
	}
}

mt_s32 DMX_OsiSetPortId(mt_u32 ChanId, const DMX_PORT_MODE_E PortMode, const mt_u32 PortId)
{
	DMX_DEV_OSI_S *pDmxDevOsi = g_pDmxDevOsi;
	DMX_ChanInfo_S *ChanInfo = &pDmxDevOsi->DmxChanInfo[ChanId];
	mt_u8 ts_src = 0;

    CHECKDMXID(ChanInfo->DmxId);

	if (DMX_PORT_MODE_RAM == PortMode)
    {
		ts_src = 4 + PortId;
    }
	else
	{
		ts_src = PortId;
	}
	
	reg_set_demux_slotn_cfg0_src(ChanInfo->ChanId, ts_src);

	return MT_SUCCESS;
}

mt_s32 DMX_OsiSetTsiClk(MT_UNF_DMX_TSI_CLK_E value)
{
	mt_u32 reg_value = 0;

	reg_value = reg_dmx_get_clk();
	reg_value &= 0xFFFFFFFC;//clear bit1-bit0	
	
	switch (value)
	{
		case CLK_400M:
			reg_value |= 0x00;		
			break;

		case CLK_360M:
			reg_value |= 0x01;
			break;

		case CLK_262M:
			reg_value |= 0x02;
			break;

		case CLK_131M:
			reg_value |= 0x03;
			break;

		default:
			return MT_ERR_DMX_NOT_IN_RANGE;
	}
	
	reg_dmx_set_clk(reg_value);	

	return MT_SUCCESS;
}

mt_s32 DMX_OsiGetTsiClk(MT_UNF_DMX_TSI_CLK_E *value)
{
	mt_u32 reg_value = 0;

	reg_value = reg_dmx_get_clk();
	reg_value &= 0x03;

	switch (reg_value)
	{
		case 0:
			*value = CLK_400M;	
			break;

		case 1:
			*value = CLK_360M;
			break;

		case 2:
			*value = CLK_262M;
			break;

		case 3:
			*value = CLK_131M;
			break;

		default:
			return MT_ERR_DMX_NOT_IN_RANGE;
	}

	return MT_SUCCESS;
}

mt_s32 DMX_OsiSetPortCtrl(mt_u32 port, mt_u32 value)
{
	switch(port)
	{
		case 0://TS0
			reg_set_ts0_sample_ctrl(value);
			break;

		case 1://TS1
			reg_set_ts1_sample_ctrl(value);
			break;

		case 2://TS2
			reg_set_ts2_sample_ctrl(value);
			break;

		case 3://TS3
			reg_set_ts3_sample_ctrl(value);
			break;

		default:
			return MT_ERR_DMX_NOT_IN_RANGE;
	}

	return MT_SUCCESS;
}

mt_s32 DMX_OsiGetPortCtrl(mt_u32 port, mt_u32 *value)
{
	switch(port)
	{
		case 0://TS0
			*value = reg_get_ts0_sample_ctrl();
			break;

		case 1://TS1
			*value = reg_get_ts1_sample_ctrl();
			break;

		case 2://TS2
			*value = reg_get_ts2_sample_ctrl();
			break;

		case 3://TS3
			*value = reg_get_ts3_sample_ctrl();
			break;

		default:
			return MT_ERR_DMX_NOT_IN_RANGE;
	}

	return MT_SUCCESS;
}

mt_s32 DMX_OsiSetPortClk(mt_u32 port, MT_UNF_DMX_PORT_CLK_SEL_E value)
{
	mt_u32 reg_value = 0;

	reg_value = reg_dmx_get_clk();
	
	switch(port)
	{
		case 0://TS0
			reg_value &= (~(7<<12));
			switch(value)
			{
				case TS0_PAD_CLK:
					reg_value |= (0<<12);
					break;

				case TS0_PAD_CLK_INVERT:
					reg_value |= (1<<12);
					break;

				case TS0_DVBC_CLK:
					reg_value |= (2<<12);
					break;

				case TS0_DVBC_CLK_INVERT:
					reg_value |= (3<<12);
					break;

				case TS0_DVBS2_1_CLK:
					reg_value |= (4<<12);
					break;

				case TS0_DVBS2_1_CLK_INVERT:
					reg_value |= (5<<12);
					break;

				default:
					return MT_ERR_DMX_NOT_IN_RANGE;
			}			
			break;

		case 1://TS1
			reg_value &= (~(3<<8));
			switch(value)
			{
				case TS1_PAD_CLK:
					reg_value |= (0<<8);
					break;

				case TS1_DVBS_CLK:
					reg_value |= (1<<8);
					break;

				case TS1_PAD_CLK_INVERT:
					reg_value |= (2<<8);
					break;

				case TS1_DVBS_CLK_INVERT:
					reg_value |= (3<<8);
					break;				

				default:
					return MT_ERR_DMX_NOT_IN_RANGE;
			}
			break;

		case 2://TS2
			reg_value &= (~(7<<4));
			switch(value)
			{
				case TS2_PAD_CLK:
					reg_value |= (0<<4);
					break;

				case TS2_PAD_CLK_INVERT:
					reg_value |= (1<<4);
					break;

				case TS2_DVBC_CLK:
					reg_value |= (2<<4);
					break;

				case TS2_DVBC_CLK_INVERT:
					reg_value |= (3<<4);
					break;

				case TS2_CICAM_OUT_CLK:
					reg_value |= (4<<4);
					break;

				case TS2_CICAM_OUT_CLK_INVERT:
					reg_value |= (5<<4);
					break;

				default:
					return MT_ERR_DMX_NOT_IN_RANGE;
			}
			break;

		case 3://TS3
			reg_value &= (~(3<<2));
			switch(value)
			{
				case TS3_PAD_CLK:
					reg_value |= (0<<2);
					break;

				case TS3_DVBS2_1_CLK:
					reg_value |= (1<<2);
					break;

				case TS3_PAD_CLK_INVERT:
					reg_value |= (2<<2);
					break;

				case TS3_DVBS2_1_CLK_INVERT:
					reg_value |= (3<<2);
					break;				

				default:
					return MT_ERR_DMX_NOT_IN_RANGE;
			}
			break;

		default:
			return MT_ERR_DMX_NOT_IN_RANGE;
	}

	reg_dmx_set_clk(reg_value);
	
	return MT_SUCCESS;
}

mt_s32 DMX_OsiGetPortClk(mt_u32 port, MT_UNF_DMX_PORT_CLK_SEL_E *value)
{
	mt_s32 ret = MT_SUCCESS;
	mt_u32 reg_value = 0;

	reg_value = reg_dmx_get_clk();
	
	switch(port)
	{
		case 0://TS0
			reg_value >>= 12;
			reg_value &= 0x07;
			switch(reg_value)
			{
				case 0:
					*value = TS0_PAD_CLK;
					break;

				case 1:
					*value = TS0_PAD_CLK_INVERT;
					break;

				case 2:
					*value = TS0_DVBC_CLK;
					break;

				case 3:
					*value = TS0_DVBC_CLK_INVERT;
					break;

				case 4:
					*value = TS0_DVBS2_1_CLK;
					break;

				case 5:				
					*value = TS0_DVBS2_1_CLK_INVERT;
					break;

				default:
					ret = MT_ERR_DMX_NOT_IN_RANGE;
					break;

			}
			break;

		case 1://TS1
			reg_value >>= 8;
			reg_value &= 0x03;
			switch(reg_value)
			{
				case 0:
					*value = TS1_PAD_CLK;
					break;

				case 1:
					*value = TS1_DVBS_CLK;
					break;

				case 2:
					*value = TS1_PAD_CLK_INVERT;
					break;

				case 3:
					*value = TS1_DVBS_CLK_INVERT;
					break;				

				default:
					ret = MT_ERR_DMX_NOT_IN_RANGE;
					break;

			}
			break;

		case 2://TS2
			reg_value >>= 4;
			reg_value &= 0x07;
			switch(reg_value)
			{
				case 0:
					*value = TS2_PAD_CLK;
					break;

				case 1:
					*value = TS2_PAD_CLK_INVERT;
					break;

				case 2:
					*value = TS2_DVBC_CLK;
					break;

				case 3:
					*value = TS2_DVBC_CLK_INVERT;
					break;

				case 4:
					*value = TS2_CICAM_OUT_CLK;
					break;

				case 5:				
					*value = TS2_CICAM_OUT_CLK_INVERT;
					break;

				default:
					ret = MT_ERR_DMX_NOT_IN_RANGE;
					break;

			}
			break;

		case 3://TS3
			reg_value >>= 2;
			reg_value &= 0x03;
			switch(reg_value)
			{
				case 0:
					*value = TS3_PAD_CLK;
					break;

				case 1:
					*value = TS3_DVBS2_1_CLK;
					break;

				case 2:
					*value = TS3_PAD_CLK_INVERT;
					break;

				case 3:
					*value = TS3_DVBS2_1_CLK_INVERT;
					break;				

				default:
					ret = MT_ERR_DMX_NOT_IN_RANGE;
					break;

			}
			break;

		default:
			ret = MT_ERR_DMX_NOT_IN_RANGE;
			break;
	}

	return ret;
}

mt_s32 DMX_OsiSetPortSrc(mt_u32 port, MT_UNF_DMX_PORT_SRC_SEL_E value)
{
	mt_u32 reg_value = 0;

	reg_value = reg_dmx_get_src();
	
	switch(port)
	{
		case 0://TS0
			
			switch(value)
			{
				case TS0_EXTERNAL_DEMOD:
					reg_value |= 0x1;
					break;

				case TS0_INTERNAL_DEMOD_S2_1:
					reg_value &= (~(1<<0));
					reg_value |= (1<<24);
					break;

				case TS0_INTERNAL_DEMOD_C_J83B_S2_2:
					reg_value &= (~(1<<0));
					reg_value &= (~(1<<24));
					break;

				default:
					return MT_ERR_DMX_NOT_IN_RANGE;
			}
			break;

		case 1://TS1
			switch(value)
			{
				case TS1_EXTERNAL_DEMOD:
					reg_value |= (1<<4);
					break;

				case TS1_INTERNAL_DEMOD_S2_0:
					reg_value &= (~(1<<4));
					break;

				default:
					return MT_ERR_DMX_NOT_IN_RANGE;
			}
			break;

		case 2://TS2
			switch(value)
			{
				case TS2_EXTERNAL_DEMOD:
					reg_value |= (1<<12);
					break;
				
				case TS2_INTERNAL_DEMOD_C_J83B_S2_2:
					reg_value &= (~(1<<12));
					reg_value |= (1<<20);
					break;
					
				case TS2_INTERNAL_CICAM:
					reg_value &= (~(1<<12));
					reg_value &= (~(1<<20));
					break;

				default:
					return MT_ERR_DMX_NOT_IN_RANGE;
			}
			break;

		case 3://TS3
			switch(value)
			{
				case TS3_EXTERNAL_DEMOD:
					reg_value |= (1<<16);
					break;

				case TS3_INTERNAL_DEMOD_S2_1:
					reg_value &= (~(1<<16));
					break;

				default:
					return MT_ERR_DMX_NOT_IN_RANGE;
			}
			break;

		default:
			return MT_ERR_DMX_NOT_IN_RANGE;
	}

	reg_dmx_set_src(reg_value);
	return MT_SUCCESS;
}

mt_s32 DMX_OsiGetPortSrc(mt_u32 port, MT_UNF_DMX_PORT_SRC_SEL_E *value)
{
	mt_u32 reg_value = 0;

	reg_value = reg_dmx_get_src();
	
	switch(port)
	{
		case 0://TS0
			if (reg_value&0x01)
			{
				*value = TS0_EXTERNAL_DEMOD;
			}
			else
			{
				if ((reg_value>>24) & 0x01)
				{
					*value = TS0_INTERNAL_DEMOD_S2_1;
				}
				else
				{
					*value = TS0_INTERNAL_DEMOD_C_J83B_S2_2;
				}
			}
			break;

		case 1://TS1
			if ((reg_value>>4)&0x01)
			{
				*value = TS1_EXTERNAL_DEMOD;
			}
			else
			{
				*value = TS1_INTERNAL_DEMOD_S2_0;
			}
			break;

		case 2://TS2
			if ((reg_value>>12)&0x01)
			{
				*value = TS2_EXTERNAL_DEMOD;
			}
			else
			{
				if ((reg_value>>20) & 0x01)
				{
					*value = TS2_INTERNAL_DEMOD_C_J83B_S2_2;
				}
				else
				{
					*value = TS2_INTERNAL_CICAM;
				}
			}
			break;

		case 3://TS3
			if ((reg_value>>16)&0x01)
			{
				*value = TS3_EXTERNAL_DEMOD;
			}
			else
			{
				*value = TS3_INTERNAL_DEMOD_S2_1;
			}
			break;

		default:
			return MT_ERR_DMX_NOT_IN_RANGE;
	}

	return MT_SUCCESS;
}

/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 DMX_OsiChannelCiEnable(mt_u32 ChanId, mt_u32 enable)
{
    DMX_DEV_OSI_S *DmxMgr = g_pDmxDevOsi;
    DMX_ChanInfo_S *ChanInfo = &DmxMgr->DmxChanInfo[ChanId];
			
	if (ChanInfo->DmxId >= DMX_CNT)
    {
		MT_ERR_DEMUX("invalid param, dmxid:%d,chanid:%d\n", ChanInfo->DmxId, ChanId);
		return MT_ERR_DMX_INVALID_PARA;
    }
	ChanInfo->slot_reg1.bitc.ci_en = enable;
	reg_set_demux_slotn_cfg1(ChanId,ChanInfo->slot_reg1.all);	
	MT_WARN_DEMUX("chan id:%d,reg:0x%x\n",ChanId,reg_get_demux_slotn_cfg1(ChanId));
    return MT_SUCCESS;	
}

mt_s32 DMX_OsiCiRecChanCfg(mt_u32 rec_chanid)
{
	reg_tsi_ci_set_cfg_t cfg;
	cfg.all = reg_get_tsi_ci_set_cfg();
	cfg.bitc.ci_reg_ch = rec_chanid;
	reg_set_tsi_ci_set_cfg(cfg.all);
	return MT_SUCCESS;
}

mt_s32 DMX_OsiCiSwtsiChanCfg(mt_u32 swtsi_chanid)
{
	reg_tsi_ci_set_cfg_t cfg;
	cfg.all = reg_get_tsi_ci_set_cfg();
	cfg.bitc.ci_swtsi_ch = swtsi_chanid;	
	printk(KERN_INFO "SwtsiChanCfg:0x%x,swtsi_chanid:%d\n",cfg.all,swtsi_chanid);
	reg_set_tsi_ci_set_cfg(cfg.all);
	return MT_SUCCESS;
}

mt_s32 DMX_OsiCiBufCfg(mt_u8 usecache)
{
	reg_tsi_ci_set_cfg_t cfg;
	cfg.all = reg_get_tsi_ci_set_cfg();
	cfg.bitc.reg_ci_buf_en = usecache;
	reg_set_tsi_ci_set_cfg(cfg.all);
	return MT_SUCCESS;
}

mt_s32 DMX_OsiCiEnableCfg(mt_u8 enable)
{
	reg_tsi_ci_set_cfg_t cfg;
	cfg.all = reg_get_tsi_ci_set_cfg();
	cfg.bitc.reg_ci_en = enable;
	reg_set_tsi_ci_set_cfg(cfg.all);
	return MT_SUCCESS;
}


mt_s32 DMX_OsiCiAhbDelayCfg(mt_u32 ahb_rd_delay)
{
	reg_tsi_ci_set_cfg_t cfg;
	cfg.all = reg_get_tsi_ci_set_cfg();
	cfg.bitc.ci_ahb_rd_delay_set = ahb_rd_delay;
	reg_set_tsi_ci_set_cfg(cfg.all);
	return MT_SUCCESS;
}

mt_s32 DMX_OsiCiCamClkCfg(mt_u32 clkdiv)
{
	reg_tsi_ci_cicam_set_t cfg;
	cfg.all = reg_get_tsi_ci_cicam_cfg();
	cfg.bitc.cicam_clk_div = clkdiv;
	reg_set_tsi_ci_cicam_cfg(cfg.all);
	return MT_SUCCESS;
}

mt_s32 DMX_OsiCiTsIntervalCfg(mt_u32 tsinterval)
{
	reg_tsi_ci_cicam_set_t cfg;
	cfg.all = reg_get_tsi_ci_cicam_cfg();
	cfg.bitc.ts_pkt_interval = tsinterval;
	reg_set_tsi_ci_cicam_cfg(cfg.all);
	return MT_SUCCESS;
}

mt_s32 DMX_OsiCiLlnNumStartCfg(mt_u32 lln_num_start)
{
	reg_tsi_ci_lln_num_start_t cfg;
	cfg.all = reg_get_tsi_ci_lln_num_start_cfg();
	cfg.bitc.ci_lln_num_start  = lln_num_start;
	reg_set_tsi_ci_lln_num_start_cfg(cfg.all);
	return MT_SUCCESS;
}

mt_s32 DMX_OsiSwtsiByteorderCfg(mt_u32 byteorder)
{
	reg_tsi_ci_swtsi_ch_set_t cfg;
	cfg.all = reg_get_tsi_ci_swtsich_cfg();
	cfg.bitc.ci_swtsi_ch_src_endian  = byteorder;
	printk(KERN_INFO "swtsich:0x%x,byteorder:%d\n",cfg.all,byteorder);
	reg_set_tsi_ci_swtsich_cfg(cfg.all);
	return MT_SUCCESS;
}

mt_s32 DMX_OsiSwtsiFullcareCfg(mt_u32 ChanId, mt_u32 full)
{
	mt_u32 val = 0; 		
	
	if (full) {		
		val = val | (1<<27);
	} else {	
		val = val & (~(1<<27));
	}
	
	printk(KERN_INFO "iFullcare:0x%x,full:%d\n",ChanId,full);
	reg_set_swtsi_chn_af_cfg5(ChanId,val);	
    return MT_SUCCESS;	
}

mt_s32 DMX_OsiTsi2SourceCfg(mt_u8 from_cam,mt_u8 serial)
{
	ulong reg = SYMPHONY_IO_VA(reg_ts_src_sel);
	mt_u32 val = HAL_GET_U32((volatile u32 *)reg);
	if (from_cam == 1) {    //from cicam
		val = val & (~(1<<20 | 1<<12));
	} else {   //from internal demod-c/jb3b/s2-2
		val |= (1<<20 | 1<<12);
	}
	HAL_PUT_U32((volatile u32 *)reg,val);

	
	MT_WARN_DEMUX("reg_ts_src_sel(0xbf138008):0x%x\n", HAL_GET_U32((volatile u32 *)reg));

	//ts2 clock cfg
	reg = SYMPHONY_IO_VA(reg_tsi_clk_sel);
	val = HAL_GET_U32((volatile u32 *)reg);
	if (from_cam == 1) {
		val = val & (~(0xf<<4));
		val |= (0x04 << 4);
	} else {
		val = val & (~(0xf<<4));
		val |= (0x02 << 4);
	}	
	HAL_PUT_U32((volatile u32 *)reg,val);
	
	MT_WARN_DEMUX("reg_tsi_clk_sel(0xbf50b004):0x%x\n", HAL_GET_U32((volatile u32 *)reg));

	val = HAL_GET_U32((volatile u32 *)reg_dmx_ts2_sample_ctrl);
	val = val & (~(1<<31));
	HAL_PUT_U32((volatile u32 *)reg_dmx_ts2_sample_ctrl,val);  //disable tsi2

	if (serial) {
		val = val | (1 << 30);
	} else {
		val = val & (~(1<<30));
	}	
	HAL_PUT_U32((volatile u32 *)reg_dmx_ts2_sample_ctrl,val);

	val = val | (1<<31);	
	HAL_PUT_U32((volatile u32 *)reg_dmx_ts2_sample_ctrl,val);  //enable tsi2
	
	MT_WARN_DEMUX("reg_dmx_ts2_sample_ctrl:0x%x\n",  HAL_GET_U32((volatile u32 *)reg_dmx_ts2_sample_ctrl));
	return 0;
}


mt_s32 DMX_OsiGetCiStatus(mt_u8 *bufstatus,mt_u8 *cistatus)
{
	reg_tsi_ci_status_t st;
	st.all = reg_get_tsi_ci_status();
	*bufstatus = st.bitc.ci_buf_busy;
	*cistatus = st.bitc.ci_busy;
	return MT_SUCCESS;
}
#endif

/*add end*/
