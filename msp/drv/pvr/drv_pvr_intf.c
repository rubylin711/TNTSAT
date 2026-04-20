/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/kernel.h>
#include <linux/version.h>
#include <linux/module.h>
#include <linux/errno.h>

#include "mt_type.h"
#include "mt_debug.h"
#include "mt_module.h"
#include "mt_drv_module.h"
#include "mt_drv_dev.h"
#include "mt_kernel_adapt.h"

//#include "pvr_debug.h"
//#include "drv_pvr_ext.h"
#include "mt_drv_pvr.h"
#include "mt_error_mpi.h"
#include "fw_api_pvr.h"
#include "mt_drv_mmz.h"
#include "mt_unf_demux.h"
#include "mt_module_debug.h"
#include "mt_unf_video.h"

#if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
#include "hal_demux_regs.h"
#include "../demux/drv_demux_index.h"
#include <linux/sched.h>
#include <uapi/linux/sched/types.h>
#include "pvrconfig.h"

#if 0
#define PVR_INTFD(fmt,...)    printk(KERN_ERR "[PVR D]: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)
#else
#define PVR_INTFD(fmt,...)
#endif

#define PVR_INTFP(fmt,...)    printk(KERN_ERR "[PVR P]: %s(%d) "fmt,__FUNCTION__, __LINE__,##__VA_ARGS__)


#define TYPE_PTS 1      //pts
#define TYPE_STC 2      //startcode
#define CACHEDINDEX_NUM 3
#endif
#define DMXINDEX_MAX  4

#define PVR_NAME                "MT_PVR"

typedef struct mtDMX_REC_INDEX_S
{
    MT_U64 u64GlobalOffset;
    MT_UNF_VIDEO_FRAME_TYPE_E enFrameType; 
    MT_U32 u32PtsMs;
    MT_U32 u32FrameSize; 
    MT_U32 u32PrivatePara;         //gobal.pos.avs,265
} DMX_REC_INDEX_S;

#if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
static struct task_struct  *PvrIdxThread  = MT_NULL;
#endif

static mt_device_s            PvrDev;
static struct file_operations   PvrFileOps;
static MT_U32                   PvrPlayChan[PVR_PLAY_MAX_CHN_NUM];
static MT_U32                   PvrRecChan[PVR_REC_MAX_CHN_NUM];
extern PVR_INDEX_INST_S *pvr_idx_buffer_inst;
MT_DECLARE_MUTEX(PvrMutex);

static MT_S32 PVR_CreatePlay(const MT_U32 file, MT_U32 *pChanId)
{
    MT_S32 ret = MT_ERR_PVR_NO_CHN_LEFT;
    MT_U32 i;

    for (i = 0; i < PVR_PLAY_MAX_CHN_NUM; i++)
    {
        if (0 == PvrPlayChan[i])
        {
            PvrPlayChan[i] = file;

            *pChanId = i;

            ret = MT_SUCCESS;

            break;
        }
    }

    return ret;
}

static MT_S32 PVR_DestroyPlay(const MT_U32 file, const MT_U32 ChanId)
{
    MT_S32 ret = MT_FAILURE;

    if (ChanId < PVR_PLAY_MAX_CHN_NUM)
    {
        if (file == PvrPlayChan[ChanId])
        {
            PvrPlayChan[ChanId] = 0;

            ret = MT_SUCCESS;
        }
    }

    return ret;
}

static MT_S32 PVR_CreateRec(const MT_U32 file, MT_U32 *pChanId)
{
    MT_S32 ret = MT_ERR_PVR_NO_CHN_LEFT;
    MT_U32 i;

    for (i = 0; i < PVR_REC_MAX_CHN_NUM; i++)
    {
        if (0 == PvrRecChan[i])
        {
            PvrRecChan[i] = file;

            *pChanId = i;

            ret = MT_SUCCESS;

            break;
        }
    }

    return ret;
}

static MT_S32 PVR_DestroyRec(const MT_U32 file, const MT_U32 ChanId)
{
    MT_S32 ret = MT_FAILURE;

    if (ChanId < PVR_REC_MAX_CHN_NUM)
    {
        if (file == PvrRecChan[ChanId])
        {
            PvrRecChan[ChanId] = 0;

            ret = MT_SUCCESS;
        }
    }

    return ret;
}
static MT_S32 PVR_Config_Shm(PVRINDEXCFG_S *idx)
{
    mt_u32 id=idx->idxid;
    if(id < 4){
        if(0==pvr_idx_buffer_inst[id].pu8KnlVirAddr){
            return MT_FAILURE;
        }
        #if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
          down(&pvr_idx_buffer_inst[id].IdxSem);
        #endif
        pvr_idx_buffer_inst[id].rec_dmxid =idx->dmxid;
        pvr_idx_buffer_inst[id].video_type=idx->vdectype;
        pvr_idx_buffer_inst[id].status=1;                 //prepared
        #if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
          up(&pvr_idx_buffer_inst[id].IdxSem);
        #endif
    }

    return MT_SUCCESS;
}
static MT_S32 PVR_Create_Shm(MT_U32 size, mtPVRShmDataType_T type)
{
    PVR_IDX_OPERATION_S args;
    MT_S32 ret = MT_ERR_DMX_ALLOC_MEM_FAILED;
    mmz_buffer_s stMMZAllocBuf;
    MT_U32 i;
    MT_U32 chn_num;

    switch (type){
        case PVR_SHM_DATA_IDX:
	          MT_INFO_PVR("-------------------\n");
            args.PvrIdxCallback = NULL;
            args.inputParam.PvrIdxCallback = NULL;
            PVR_InitWithOperation(&args);
            size=(size/sizeof(DMX_REC_INDEX_S)*sizeof(DMX_REC_INDEX_S));
            if (MT_SUCCESS != mt_drv_mmz_alloc_and_map("PVR_IdxShmBuf", MMZ_OTHERS, size, 4096, &stMMZAllocBuf)) {
            	ret = MT_ERR_DMX_ALLOC_MEM_FAILED;
            }else{
                for(i=0; i<4; i++){
                    if(NULL==pvr_idx_buffer_inst[i].pu8KnlVirAddr){
                        chn_num = i;
                        break;
                    }
                }
                if(4==i){
                    ret = MT_ERR_DMX_ALLOC_MEM_FAILED;
                }else{
                    memset(&pvr_idx_buffer_inst[chn_num], 0, sizeof(DMX_REC_INDEX_S));
                    memset((mt_u8*)stMMZAllocBuf.startVirAddr, 0 , size);
                    #if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
                      down(&pvr_idx_buffer_inst[chn_num].IdxSem);
                    #endif
					
                    pvr_idx_buffer_inst[chn_num].u32PhyAddr = stMMZAllocBuf.startPhyAddr;
                    pvr_idx_buffer_inst[chn_num].pu8KnlVirAddr = (mt_u8*)stMMZAllocBuf.startVirAddr;
                    pvr_idx_buffer_inst[chn_num].u32Size = size;
                    pvr_idx_buffer_inst[chn_num].index_num = (size/sizeof(DMX_REC_INDEX_S));
                    pvr_idx_buffer_inst[chn_num].index_rp=0;
                    pvr_idx_buffer_inst[chn_num].index_wp=0;
                    pvr_idx_buffer_inst[chn_num].rec_dmxid=0xffffffff;
                    pvr_idx_buffer_inst[chn_num].video_type=0;
                    pvr_idx_buffer_inst[chn_num].status=0;
                    ret=chn_num;
                    #if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
                      up(&pvr_idx_buffer_inst[chn_num].IdxSem);
                    #endif
                    //printk("+++idx:PVR_Create_Shm.chn_num=%x,%x,%x\n",chn_num,pvr_idx_buffer_inst,pvr_idx_buffer_inst[chn_num].pu8KnlVirAddr);
                }
            }
            break;
        default:
            break;
    }
    return ret;
}

static MT_S32 PVR_Destroy_Shm(MT_U32 chn_num, mtPVRShmDataType_T type)
{
    MT_S32 ret = MT_FAILURE;

    switch (type){
        case PVR_SHM_DATA_IDX:
            //printk("+++idx:PVR_Destroy_Shm.chn_num=%x\n",chn_num);
            PVR_Exit(chn_num);
            ret = MT_SUCCESS;
            break;
        default:
            break;
    }
    return ret;
}

static MT_S32 PVR_Read_Data(void* data, MT_U32 len, mtPVRShmDataType_T type)
{
    MT_S32 ret = MT_FAILURE;
    MT_U32 chn_id=0xffffffff;
    MT_U32 index_rpnxt=0;
    PVR_INDEX_INST_S *bufferlist=NULL;
    DMX_REC_INDEX_S *inptr=NULL;
    MT_UNF_DMX_REC_INDEX_S *outptr=(MT_UNF_DMX_REC_INDEX_S *)data;

    if((NULL==pvr_idx_buffer_inst) || (NULL==data)){
        return MT_FAILURE;
    }
    chn_id=((MT_UNF_DMX_REC_INDEX_S*)data)->u32PrivatePara;
    bufferlist=&pvr_idx_buffer_inst[chn_id];
    if(NULL==bufferlist){
        return MT_FAILURE;
    }
    if(NULL==bufferlist->pu8KnlVirAddr){
        return MT_FAILURE;
    }
    if(0xffffffff == bufferlist->rec_dmxid){
        return MT_FAILURE;
    }
    if(0==bufferlist->status){  //not prepared
        return MT_FAILURE;
    }
    memset(data,0x00,sizeof(MT_UNF_DMX_REC_INDEX_S));

    switch (type){
        case PVR_SHM_DATA_IDX:
            { //to check buffer,whether empty
                if(bufferlist->index_rp == bufferlist->index_wp){ //rec no start
                    ret = MT_FAILURE;
                    break;
                }
                index_rpnxt=(bufferlist->index_rp+1);
                if(index_rpnxt == bufferlist->index_num){
                    index_rpnxt=0;
                }
            }
            inptr=&((DMX_REC_INDEX_S*)bufferlist->pu8KnlVirAddr)[bufferlist->index_rp];
            outptr->enFrameType=inptr->enFrameType;
            outptr->u32PtsMs=inptr->u32PtsMs;
            outptr->u64GlobalOffset=inptr->u64GlobalOffset;
            outptr->u32FrameSize=inptr->u32FrameSize;
            outptr->u32DataTimeMs=inptr->u32PrivatePara;//(jiffies*(1000/HZ));  //ms
            bufferlist->index_rp=index_rpnxt;
            ret = MT_SUCCESS;
            break;
        default:
            break;
    }
    return ret;
}


static MT_S32 PVR_Read_Last_index(void* data, MT_U32 len, mtPVRShmDataType_T type)
{
    MT_S32 ret = MT_FAILURE;
    MT_U32 chn_id=0xffffffff;
	static MT_U32 last_data_time_ms[DMXINDEX_MAX] = {0};
    PVR_INDEX_INST_S *bufferlist=NULL;
    DMX_REC_INDEX_S *inptr=NULL;
    MT_UNF_DMX_REC_INDEX_S *outptr=(MT_UNF_DMX_REC_INDEX_S *)data;
    MT_U32 last_index = 0;

    if((NULL==pvr_idx_buffer_inst) || (NULL==data)){
        return MT_FAILURE;
    }
    chn_id=((MT_UNF_DMX_REC_INDEX_S*)data)->u32PrivatePara;
    if(chn_id >=4)
    {
        MT_ERR_PVR("Error index:%d\n",chn_id);
        return MT_FAILURE;
    }
    bufferlist=&pvr_idx_buffer_inst[chn_id];
    if(NULL==bufferlist){
        return MT_FAILURE;
    }
    if(NULL==bufferlist->pu8KnlVirAddr){
        return MT_FAILURE;
    }
    if(0xffffffff == bufferlist->rec_dmxid){
        return MT_FAILURE;
    }
    if(0==bufferlist->status){  //not prepared
        return MT_FAILURE;
    }
    memset(data,0x00,len);

    switch (type){
        case PVR_SHM_DATA_IDX:
            {
                ret = MT_SUCCESS;
#if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
                down(&bufferlist->IdxSem);
#endif
                if (bufferlist->index_wp){
                  last_index = bufferlist->index_wp-1;
                }
                else{
                  if (bufferlist->index_rp || (bufferlist->status == 2)){
                    last_index = bufferlist->index_num - 1;
                  }
                  else
                  {
                    ret = MT_FAILURE;
                  }
                }
                inptr=&((DMX_REC_INDEX_S*)bufferlist->pu8KnlVirAddr)[last_index];
                outptr->enFrameType=inptr->enFrameType;
                outptr->u32PtsMs=inptr->u32PtsMs;
                outptr->u64GlobalOffset=inptr->u64GlobalOffset;
                outptr->u32FrameSize=inptr->u32FrameSize;
                outptr->u32DataTimeMs=inptr->u32PrivatePara;

                if (inptr->u32PtsMs == 0 && outptr->u32DataTimeMs == 0)
                {
                  printk("\n##PVR_Read_Last_index  last_index=%d,[%d, %d, %d,%lld]\n",last_index,
                    inptr->u32PtsMs,outptr->u32FrameSize,outptr->u32DataTimeMs, outptr->u64GlobalOffset);
                  ret = MT_FAILURE;
                }
                if (last_data_time_ms[chn_id] > outptr->u32DataTimeMs){
                  inptr->u32PrivatePara = last_data_time_ms[chn_id];
                }
                last_data_time_ms[chn_id] = outptr->u32DataTimeMs;
#if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
                up(&bufferlist->IdxSem);
#endif
            }
            break;
        default:
            break;
    }
    return ret;

}
static MT_S32 PVR_Read_Data_Ex(void* data, MT_U32 len, mtPVRShmDataType_T type)
{
    MT_S32 ret = MT_FAILURE;
    MT_U32 chn_id=0xffffffff;
    MT_U32 maxnum=(len/sizeof(MT_UNF_DMX_REC_INDEX_S));
    PVR_INDEX_INST_S *bufferlist=NULL;
    DMX_REC_INDEX_S *inptr=NULL;
    MT_UNF_DMX_REC_INDEX_S *outptr=(MT_UNF_DMX_REC_INDEX_S *)data;

    if((NULL==pvr_idx_buffer_inst) || (NULL==data)){
        return MT_FAILURE;
    }
    chn_id=((MT_UNF_DMX_REC_INDEX_S*)data)->u32PrivatePara;
    bufferlist=&pvr_idx_buffer_inst[chn_id];
    if(NULL==bufferlist){
        return MT_FAILURE;
    }
    if(NULL==bufferlist->pu8KnlVirAddr){
        return MT_FAILURE;
    }
    if(0xffffffff == bufferlist->rec_dmxid){
        return MT_FAILURE;
    }
    if(0==bufferlist->status){  //not prepared
        return MT_FAILURE;
    }
    memset(data,0x00,len);
	#if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
    down(&bufferlist->IdxSem);
	#endif
    switch (type){
        case PVR_SHM_DATA_IDX:
            { //to check buffer,whether empty
                mt_u32  index_rp=bufferlist->index_rp;
                mt_u32  index_wp=bufferlist->index_wp;
                mt_u32  index_nm=bufferlist->index_num;
                mt_u32  index_cp=0;
                mt_u32  i=0;
                if(index_rp == index_wp){ //rec no start
                    //printk("++drv.6 %x,%x,%x,%x\n",index_rp,bufferlist->index_rp,index_wp,bufferlist->index_wp);
                    ret = MT_FAILURE;
                    break;
                }
                index_cp=(index_wp+index_nm-index_rp)%index_nm;
                if(index_cp>maxnum){
                    index_cp=maxnum;
                }
                while(i<index_cp){
                    inptr=&((DMX_REC_INDEX_S*)bufferlist->pu8KnlVirAddr)[index_rp];
                    outptr[i].enFrameType=inptr->enFrameType;
                    outptr[i].u32PtsMs=inptr->u32PtsMs;
                    outptr[i].u64GlobalOffset=inptr->u64GlobalOffset;
                    outptr[i].u32FrameSize=inptr->u32FrameSize;
 					outptr[i].u32DataTimeMs=inptr->u32PrivatePara;//(jiffies*(1000/HZ));  //ms
                    index_rp++;
                    index_rp=(index_rp%index_nm);
                    i++;
                }
                bufferlist->index_rp=index_rp;
                if(i!=maxnum){  //set end
                    outptr[i].enFrameType=0xffffffff;
                }
            }
            ret = MT_SUCCESS;
            break;
        default:
            break;
    }
	#if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
    up(&bufferlist->IdxSem);
	#endif
    return ret;
}

static MT_S32 PVR_Ioctl(struct inode *inode, struct file *file, MT_U32 cmd, MT_VOID *arg)
{
    MT_S32 ret;

    ret = down_interruptible(&PvrMutex);

    switch (cmd)
    {
        case CMD_PVR_CREATE_PLAY_CHN:
        {
            ret = PVR_CreatePlay((ulong)file, (MT_U32*)arg);

            break;
        }

        case CMD_PVR_DESTROY_PLAY_CHN:
        {
            ret = PVR_DestroyPlay((ulong)file, *((MT_U32 *)arg));

            break;
        }

        case CMD_PVR_CREATE_REC_CHN:
        {
            ret = PVR_CreateRec((ulong)file, (MT_U32*)arg);

            break;
        }

        case CMD_PVR_DESTROY_REC_CHN:
        {
            ret = PVR_DestroyRec((ulong)file, *((MT_U32 *)arg));

            break;
        }

        case CMD_PVR_REC_CREATE_IDX_SHM:
        {
            ret = PVR_Create_Shm(*((MT_U32 *)arg), PVR_SHM_DATA_IDX);

            break;
        }
        case CMD_PVR_REC_CONFIG_IDX_SHM:
        {
            ret = PVR_Config_Shm((PVRINDEXCFG_S *)arg);

            break;
        }

        case CMD_PVR_REC_DESTROY_IDX_SHM:
        {
            ret = PVR_Destroy_Shm(*((MT_U32 *)arg), PVR_SHM_DATA_IDX);

            break;
        }

        case CMD_PVR_REC_WRITE_DATA:
        {
            
            ret = MT_FAILURE;//PVR_Write_Data(arg, sizeof(MT_UNF_DMX_REC_INDEX_S),PVR_SHM_DATA_IDX);

            break;
        }

        case CMD_PVR_REC_READ_DATA:
        {
            ret = PVR_Read_Data(arg, sizeof(MT_UNF_DMX_REC_INDEX_S), PVR_SHM_DATA_IDX);

            break;
        }
        case CMD_PVR_REC_READ_DATA_EX:
        {
            ret = PVR_Read_Data_Ex(arg, sizeof(MT_UNF_DMX_REC_INDEX_ARRY_S), PVR_SHM_DATA_IDX);

            break;
        }
        case CMD_PVR_REC_READ_LAST_INDEX:
        {
            ret = PVR_Read_Last_index(arg, sizeof(MT_UNF_DMX_REC_INDEX_S), PVR_SHM_DATA_IDX);
            break;
        }
        default:
            ret = -ENOIOCTLCMD;
    }

    up(&PvrMutex);

    return ret;
}

static atomic_t pvr_open_cnt_atomic = ATOMIC_INIT(0);

static int PVR_DRV_Open(struct inode *inode, struct file *file)
{
    atomic_inc(&pvr_open_cnt_atomic);
    MT_INFO_PVR("pvr drv open count %d\n", atomic_read(&pvr_open_cnt_atomic));
    return 0;
}

static int PVR_DRV_Close(struct inode *inode, struct file *file)
{
    MT_S32 ret;
    MT_U32 i;
    MT_INFO_PVR("pvr drv close count %d\n",  atomic_read(&pvr_open_cnt_atomic));
    if(atomic_dec_and_test(&pvr_open_cnt_atomic))	{
        MT_INFO_PVR("+++pvr idx exit\n");

        for(i=0;i<4;i++){
            PVR_Destroy_Shm(i, PVR_SHM_DATA_IDX);
        }

        ret = down_interruptible(&PvrMutex);

        for (i = 0; i < PVR_PLAY_MAX_CHN_NUM; i++)
        {
            PVR_DestroyPlay((ulong)file, i);
        }

        for (i = 0; i < PVR_REC_MAX_CHN_NUM; i++)
        {
            PVR_DestroyRec((ulong)file, i);
        }

        up(&PvrMutex);
    }
    return 0;
}

static long PVR_DRV_Ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    return (long)mt_drv_usercopy(file->f_path.dentry->d_inode, file, cmd, arg, PVR_Ioctl);
}

#if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)

typedef struct DMX_REC_INDEX_RAW
{
    DMX_REC_INDEX_S publicindex;
    mt_u32  u32HdrData[3];
} MT_UNF_DMX_REC_INDEX_RAW;

typedef struct indexinfor
{
    mt_u64 offset;
    mt_u32  u32HdrData[2];
    mt_u32  pts;
    u16 type;
    u16 sps;
}INDEXINFOR;

typedef struct indexmanage
{
    mt_u32  nowpts;
    mt_u32  cachednum;
    mt_u32  avs_profile;
    mt_u32  flag_265sps;        //first frame, it's I frame
    mt_u32  last_get;
    mt_u32  last_update;
    INDEXINFOR cachedindex[CACHEDINDEX_NUM];
}INDEXMANAGE;

INDEXMANAGE g_indexmanage[DMXINDEX_MAX]={{0,0,0,0,0,0,{{0}}}};
MT_UNF_DMX_REC_INDEX_RAW g_pvr_index[DMXINDEX_MAX] = {0, };
static MT_U32 g_rec_waddr[DMXINDEX_MAX] = {0};
static MT_BOOL g_rec_parseiframe[DMXINDEX_MAX] = {0};


#define vc1_SCFrameHeader 0x0000010D
#define VOP_START_CODE            0x1b6
#define SHORT_VIDEO_START_MARKER  0x020
#define MPEG_PICTURE_START_CODE      0x100
#define AVS_I_PICTURE_START_CODE    0x1B3
#define AVS_PB_PICTURE_START_CODE   0x1B6
extern mt_s32 Dmx_Drv_Idx_GetVaddr_ByRecid(mt_u32 RecId,ulong *idxv,ulong *tsv);
static __inline int bit_mask(int set_bitnumber_high, int set_bitnumber_low)
{
  return (((1<<(set_bitnumber_high - set_bitnumber_low + 1)) - 1) << set_bitnumber_low);
}
static __inline int get_b (unsigned int input, int get_bitnumber)//0~31
{
  unsigned int input_temp;

  input_temp = input;

  input_temp <<= (31 - get_bitnumber);
  input_temp >>= 31;

  return input_temp;
}

static __inline int get_nb(unsigned int input, int get_bitnumber_high, int get_bitnumber_low)//get_bitnumber:0~31
{
  unsigned int input_temp;

  input_temp = input;

  input_temp &= bit_mask(get_bitnumber_high, get_bitnumber_low);

  input_temp >>= get_bitnumber_low;

  return input_temp;
}

static int code_check_get_ue (unsigned char buffer[],int totbitoffset,int *info, int bytecount)
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

static mt_s32 dmx_rec_get64(ulong vaddr, mt_u32  rp, mt_u32  off, mt_u32  wp, mt_u32  size, mt_u64 *data)
{
    int i;
    mt_u64 idx_data;
    int len;

    BEGIN:
    if(wp >= rp){
        len = wp - rp; 
    }else{
        len = ( size - rp ) + wp;
    }

    if(len < 8 || data == NULL || off >= len)
        return -1;

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
        idx_data = *((mt_u64 *)(ulong)(rp + vaddr));
    }else{
        idx_data = 0;
        for(i = 0; i<size - rp; i++)
            idx_data = (idx_data << 8) | *((mt_u8 *)(ulong)(rp + i+ vaddr));
        for(i = 0; i<(8-(size-rp)); i++)
            idx_data = (idx_data << 8) | *((mt_u8 *)(ulong)(i + vaddr));
    }
    *data = idx_data;
    return 0;
}
#define VC1_CPYLEN  12
#define NORMAL_CPYLEN  8
static RET_CODE dmx_rec_convert_byte_order(char* data, mt_u32  bcnt)
{
    mt_u32  i;
    char tmp;
    for(i=0; i<(bcnt >> 1); i++){
        tmp = *(data + i);
        *(data + i) = *(data + bcnt - 1 -i);
        *(data + bcnt - 1 -i) =  tmp;
    }
    return 0;
}
static mt_u32  dmx_rec_get_remain_stc(mt_u32  pid,mt_u8 *ts_align,mt_u8 *ts_end,mt_u8 *buf, mt_u32  need)
{
    mt_u8 *ts_head=ts_align;
    mt_u32  pid_loop=0x1fff;
    mt_u32  adaptation_control=0;
    mt_u32  skip=0;
    mt_u32  cpyed=0;
    mt_u32  tmpcpyed=0;

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
                return (cpyed+need);
            }else if(3==adaptation_control){    //after atapt .head.4+len+len.data
                skip=(4+1+ts_head[4]);
                if(188 <= skip){                //if ad.len >=183 ,it is error ts-packet
                    ts_head+=188;
                    continue;
                }
                if(skip+need <= 188){
                    memcpy(buf+cpyed,ts_head+skip,need);
                    //dmx_x_dumpts("snd2:",buf+cpyed,need);
                    return (cpyed+need);
                }else{
                    tmpcpyed=(188-skip);
                    if(tmpcpyed>need){    //error ts
                        //printk(KERN_EMERG "+++dox1:%x,%x,%x,%x,%x\n",ts_head[1],ts_head[4],skip,tmpcpyed,need);
                        tmpcpyed=need;
                    }
                    memcpy(buf+cpyed,ts_head+skip,tmpcpyed);
                    //dmx_x_dumpts("snd3:",buf+cpyed,need);
                    cpyed+=tmpcpyed;
                    need -=tmpcpyed;
                    if(0==need){
                        return (cpyed+tmpcpyed);
                    }
                }
            }
        }
        ts_head+=188;
    }
    return cpyed;
}

static mt_s32 dmx_rec_get_index_data(mt_u32  vtype,mt_u32 index, MT_UNF_DMX_REC_INDEX_RAW *addr, mt_u32  *chn_num,mt_u32  *thiscode)
{
    ulong  wp = reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(index);
    ulong  rp = reg_get_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(index);
    ulong  rec_indexbuffer_startPoint = reg_get_trpp_ch_idx_start_addr_trpp_ch_idx_saddr(index) << 3;
    ulong  rec_indexbuffer_endPoint = reg_get_trpp_ch_idx_end_addr_trpp_ch_idx_eaddr(index);
    ulong  rec_indexbuffer_size = rec_indexbuffer_endPoint - rec_indexbuffer_startPoint + 1;
    ulong  len=0,flag_moverp=0;
    ulong  dsaddr,deaddr,dbuf_size;
    reg_trpp_ch_idx_data_t idx_data;
    ulong  rd_len = 0;
    mt_s32 ret = 0;
    ulong  i;

	mt_u8 	*rdaddr = NULL;
    mt_u32  pnum = 0;
    mt_u8 	poff = 0;
	mt_u8 	*u8ptr = NULL;
    mt_u32  firstcpy = 0;
    mt_u32  cpymax = 0;
					
    dsaddr = reg_get_trpp_ch_rec_start_addr_trpp_ch_rec_saddr(index) << 3;
    deaddr = reg_get_trpp_ch_rec_end_addr_trpp_ch_rec_eaddr(index);
    dbuf_size = deaddr - dsaddr + 1;
    // *chn_num = 0;

    if(wp >= rp){
        len = wp - rp;
    }else{
        len = (rec_indexbuffer_size - rp) + wp;
    }
    
    ret=Dmx_Drv_Idx_GetVaddr_ByRecid(index,&rec_indexbuffer_startPoint,&dsaddr);
    if(MT_SUCCESS != ret){
        return -1;
    }
    *thiscode=0xffffffff;
    if(16 <= len){  //more than 2 stc
        //mtos_printk1("+0 %x\n",rec_indexbuffer_startPoint);
        ret = dmx_rec_get64(rec_indexbuffer_startPoint, rp, 0, wp, rec_indexbuffer_size, (mt_u64 *)&idx_data);
        if(ret != 0){
            //mtos_printk1("+1\n");
            return -1;
        }
        //mtos_printk1("+2\n");
        *thiscode=idx_data.dts.idx_num;
        switch(idx_data.dts.idx_num)
        {
            case TRPP_INDEX_DTS:
                //mtos_printk1("+3\n");
                rd_len = 8;
                ret = -1;
                flag_moverp=1;
            break;
            case TRPP_INDEX_PTS:
                if(len >=16){
                    ((DMX_REC_INDEX_S *)addr)->u64GlobalOffset = idx_data.pts.rec_ch_num ;
                    ((DMX_REC_INDEX_S *)addr)->u64GlobalOffset *= 188;
                    i = idx_data.pts.rec_ch_num;
                    *chn_num = idx_data.pts.rec_ch_num;
                    //mtos_printk1("+4 %x\n",rec_indexbuffer_startPoint);
                    ret = dmx_rec_get64(rec_indexbuffer_startPoint, rp, 8, wp, rec_indexbuffer_size, (mt_u64 *)&idx_data);
                    if(ret != 0){
                        rd_len = len;
                        ret = -1;
                        break;
                    }
                }else{
                    rd_len = len;
                    ret = -1;
                    break;
                }
                if(idx_data.dts.idx_num == TRPP_INDEX_DTS && idx_data.dts.pts_vld && idx_data.dts.pts){
                    mt_u32  pts32;
                    pts32 = idx_data.dts.pts >> 1;
                    ((DMX_REC_INDEX_S *)addr)->u32PtsMs = pts32/45;
                }
                rd_len = 16;
                flag_moverp=1;
            break;
            case TRPP_INDEX_ES_SC:
                {                    
                    *chn_num = idx_data.es_sc.rec_ch_num;
                    pnum = idx_data.es_sc.rec_ch_num%(dbuf_size/188);
                    poff = idx_data.es_sc.sc_offset;
                    rdaddr = (mt_u8 *)(dsaddr + pnum*188 + poff);
                    //mtos_printk1("++5.0 %x,%x,%x,%x,%x\n",dsaddr,rdaddr,addr,poff,idx_data.es_sc.sc_offset);
                    #if 0
                    addr->u32HdrData[0] = rdaddr[0];
                    addr->u32HdrData[0] = (addr->u32HdrData[0]<<8)|rdaddr[1];
                    addr->u32HdrData[0] = (addr->u32HdrData[0]<<8)|rdaddr[2];
                    addr->u32HdrData[0] = (addr->u32HdrData[0]<<8)|rdaddr[3];
                    addr->u32HdrData[1] = rdaddr[4];
                    addr->u32HdrData[1] = (addr->u32HdrData[1]<<8)|rdaddr[5];
                    addr->u32HdrData[1] = (addr->u32HdrData[1]<<8)|rdaddr[6];
                    addr->u32HdrData[1] = (addr->u32HdrData[1]<<8)|rdaddr[7];                    
                    //addr->u32HdrData[0] = *((mt_u32  *)rdaddr);
                    //(void)dmx_rec_convert_byte_order((char *)(&addr->u32HdrData[0]), 4);
                    //addr->u32HdrData[1] = *((mt_u32  *)rdaddr + 1);
                    //(void)dmx_rec_convert_byte_order((char *)(&addr->u32HdrData[1]), 4);
                    #else
                    u8ptr=(mt_u8 *)((ulong)((MT_UNF_DMX_REC_INDEX_RAW *)addr)->u32HdrData);
                    firstcpy=(188-poff);       //poff=[0,187]
                    cpymax=NORMAL_CPYLEN;
                    if(MT_UNF_VCODEC_TYPE_VC1==vtype){
                        cpymax=VC1_CPYLEN;
                    }
                    if(cpymax < firstcpy){
                        firstcpy = cpymax;
                    }
                    memcpy(u8ptr,rdaddr,firstcpy);
                    if(cpymax > firstcpy){
                        mt_s32 cpyed=0;
                        mt_u32  pid=0x1fff;
                        ulong  tsend=(dsaddr+dbuf_size);
                        //dmx_x_dumpts("first:",u8ptr,firstcpy);
                        rdaddr = (mt_u8 *)(dsaddr + pnum*188);
                        pid = rdaddr[1];
                        pid = (0x1fff&((pid<<8) | rdaddr[2]));
                        //printk(KERN_EMERG "+++do1:%x,%x,%x,%x,%x\n",pnum*188+188,dbuf_size,rdaddr+188,tsend,cpymax-firstcpy); // 
                        cpyed = dmx_rec_get_remain_stc(pid,rdaddr+188,(mt_u8*)tsend,u8ptr+firstcpy,cpymax-firstcpy); //check from next ts-packet
                        if(cpyed != (cpymax-firstcpy)){  //must be ts-reloop,so check from buff_head
                            //printk("+++do third\n");
                            //printk(KERN_EMERG "+++do2:%x,%x,%x,%x,%x,%x\n",pnum*188+188,dbuf_size,dsaddr,rdaddr,firstcpy+cpyed,cpymax-firstcpy-cpyed);
                            dmx_rec_get_remain_stc(pid,(mt_u8*)dsaddr,rdaddr,u8ptr+firstcpy+cpyed,cpymax-firstcpy-cpyed);
                        }
                        if(0){
                            //mt_u32  *next=(mt_u32  *)(rdaddr+188);
                            //printk("++nxt=%x,%x,%x,%x\n",*next,*(next+1),reg_get_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(RecInfo->DmxId),pnum*188);
                            //printk("%x,%x,%x,%x,%x,%x,%x,%x\n",u8ptr[0],u8ptr[1],u8ptr[2],u8ptr[3],u8ptr[4],u8ptr[5],u8ptr[6],u8ptr[7]);
                        }
                    }
                    (void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_RAW *)addr)->u32HdrData[0]), 4);
                    (void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_RAW *)addr)->u32HdrData[1]), 4);
                    if(MT_UNF_VCODEC_TYPE_VC1==vtype){
                        (void)dmx_rec_convert_byte_order((char *)(&((MT_UNF_DMX_REC_INDEX_RAW *)addr)->u32HdrData[2]), 4);     //reuse it
                    }                    
                    #endif
                    ((DMX_REC_INDEX_S *)addr)->u64GlobalOffset = idx_data.es_sc.rec_ch_num;
                    ((DMX_REC_INDEX_S *)addr)->u64GlobalOffset *= 188;
                    if(MT_UNF_VCODEC_TYPE_MPEG2 == vtype){  //mpg2.2frames share 1tspacket
                        ((DMX_REC_INDEX_S *)addr)->u64GlobalOffset+=poff;
                    }
                    
                    rd_len = 8;
                    flag_moverp=1;
                    //mtos_printk1("++5.1 %x,%x\n",addr->u32HdrData[0],addr->u32HdrData[1]);
                }
            break;
            default:
                //mtos_printk1("+6\n");
                rd_len = 8;
                ret = -1;
                flag_moverp=1;
            break;
        }
    }else{
        rd_len =  len & 0x07;
        ret = -1;
    }

    if(1==flag_moverp){
        if(rec_indexbuffer_size - rp > rd_len){
            wp = rp + rd_len;
        }else{
            wp = rd_len - (rec_indexbuffer_size - rp);
        }
        //mtos_printk1("+7\n");
        reg_set_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(index, wp);
    }
    return ret; 
}
static mt_s32 mdx_rec_softfilter_stc(mt_u32  vtype,mt_u32  stc)
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
        mt_u32  T5;
        if((stc&0xffffff80)!=0x00000100){     //high1.=0
            return -1;
        }
        T5=(stc&0x1f);
        if((T5!=1) && (T5!=5) && (T5!=7)){  //low5=1 5 7
            return -1;
        }
    }else if(MT_UNF_VCODEC_TYPE_HEVC==vtype){ //1.6.1
        mt_u32  T6;
        if((stc&0xffffff80)!=0x00000100){     //high1.=0
            return -1;
        }
        T6=((stc>>1)&0x3f); //6bit bit1~bit6
        if(!(((0<=T6)&&(9>=T6))||((16<=T6)&&(32>=T6)))){    //mid6=0-9 16-32
            return -1;
        }
    }else{
        return -1;
    }
    return 0;
}

static mt_s32 Dmx_AcquireRecIndex(PVR_INDEX_INST_S *RecInfo,mt_u32  id,MT_UNF_DMX_REC_INDEX_RAW *RecIndex)
{
    //int i=0;
    //mt_u8 hevctype;
    //mt_u32  flag_265sps=0;
    mt_s32 ret1 = -1/*,ret2 = -1*/;
    mt_u32  thiscode=0xffffffff;
    //mt_u32  nextcode=0xffffffff;
    mt_u32  chn_num0 = 0;

    REDEAL:
    for(;;){
        if(CACHEDINDEX_NUM==g_indexmanage[id].cachednum){
            //mtos_printk1("++cache full\n");
            break;
        }
        ret1 = dmx_rec_get_index_data(RecInfo->video_type,RecInfo->rec_dmxid, RecIndex, &chn_num0,&thiscode);
        if(0 != ret1){
            if(g_indexmanage[id].cachednum<CACHEDINDEX_NUM){        //return bad
                return -1;
            }else{                                                  //deal cached
                break;
            }
        }
        PVR_INTFD("thiscode=%d cachednum=%d\n",thiscode,g_indexmanage[id].cachednum);
        //deal sequnce header begin
        if(TRPP_INDEX_ES_SC==thiscode){
            //mtos_printk1("+++av[%d].sft %x,%x,%x\n",id,RecInfo->rec_dmxid,RecInfo->video_type,RecIndex->u32HdrData[0]);
            PVR_INTFD("IDX SC video_type=%d scdata=0x%x cachednum=%d\n",RecInfo->video_type,RecIndex->u32HdrData[0],g_indexmanage[id].cachednum);
            {//soft filter.while double-rec,maybe other startcode
                RET_CODE ret_sflt=mdx_rec_softfilter_stc(RecInfo->video_type,RecIndex->u32HdrData[0]);
                if(0 != ret_sflt){
                    continue;
                }
            }

            if((((MT_UNF_VCODEC_TYPE_MPEG4==RecInfo->video_type)||
                 (MT_UNF_VCODEC_TYPE_AVS==RecInfo->video_type))&&
                      (RecIndex->u32HdrData[0]==0x1b0))||
                     ((MT_UNF_VCODEC_TYPE_MPEG2==RecInfo->video_type)&&
                      (RecIndex->u32HdrData[0]==0x1b3))||
                      ((MT_UNF_VCODEC_TYPE_HEVC==RecInfo->video_type)&&
                     ((RecIndex->u32HdrData[0]&0xffffff7e)==0x140))||     //1.6.1
                     ((MT_UNF_VCODEC_TYPE_VC1==RecInfo->video_type)&&
                     (RecIndex->u32HdrData[0]==0x10f))||
                     ((MT_UNF_VCODEC_TYPE_H264==RecInfo->video_type)&&
                     ((RecIndex->u32HdrData[0]&0xffffff1f)==0x107))){     //1.2.5
                g_indexmanage[id].cachedindex[g_indexmanage[id].cachednum].pts=g_indexmanage[id].nowpts;
                g_indexmanage[id].cachedindex[g_indexmanage[id].cachednum].type=TYPE_PTS;
                g_indexmanage[id].cachedindex[g_indexmanage[id].cachednum].offset=((DMX_REC_INDEX_S*)RecIndex)->u64GlobalOffset;
                g_indexmanage[id].cachednum++;
                //mtos_printk1("+++av[%d].add head:%x\n",id,g_indexmanage[id].nowpts);
                if(MT_UNF_VCODEC_TYPE_AVS==RecInfo->video_type){
                    g_indexmanage[id].avs_profile=((RecIndex->u32HdrData[1]>>24) & 0xff);
                }else if(MT_UNF_VCODEC_TYPE_HEVC==RecInfo->video_type){     //head
                    g_indexmanage[id].flag_265sps=1;
                    PVR_INTFD("cachednum=%d flag_265sps set to 1\n",g_indexmanage[id].cachednum);
                }else if(MT_UNF_VCODEC_TYPE_VC1==RecInfo->video_type){      //vc1 sequnce
                    mt_u32  vc1_profile=((RecIndex->u32HdrData[1]>>30) & 0x03);
                    if(vc1_profile <= 1){ //simple/main
                        g_indexmanage[id].avs_profile=RecIndex->u32HdrData[1];
                    }else{
                        g_indexmanage[id].avs_profile=((RecIndex->u32HdrData[1]&0xffff0000)|(RecIndex->u32HdrData[2]>>16));  //advance.32bit is not enough
                    }
                }
                continue;
            }
        }
        //deal sequnce header over
        if(TRPP_INDEX_PTS==thiscode){
            g_indexmanage[id].nowpts=((DMX_REC_INDEX_S*)RecIndex)->u32PtsMs;
            PVR_INTFD("IDX PTS cachednum=%d\n",g_indexmanage[id].cachednum);
          continue;
        }else{
            //mtos_printk1("++add stc\n");
            if(MT_UNF_VCODEC_TYPE_H264==RecInfo->video_type){         //multislice,now don't deal
                mt_u8 slice[4] = {0};
                int offset=0,firstmbinslice=0;
                slice[0]=(0xff&(RecIndex->u32HdrData[1]>>24));
                slice[1]=(0xff&(RecIndex->u32HdrData[1]>>16));
                slice[2]=(0xff&(RecIndex->u32HdrData[1]>>8));
                slice[3]=(0xff&(RecIndex->u32HdrData[1]));
                code_check_get_ue(slice,offset,&firstmbinslice,4);
                if(0 != firstmbinslice){
                    continue;
                }
            }else if(MT_UNF_VCODEC_TYPE_HEVC==RecInfo->video_type){     //multislice,now don't deal
                int first_sliceseg_inpic_flag=(RecIndex->u32HdrData[1]&0x00800000);
                //printk("%x,%x\n",RecIndex->u32HdrData[1],first_sliceseg_inpic_flag);
                if(0x00800000!=first_sliceseg_inpic_flag){
                    continue;
                }
                g_indexmanage[id].cachedindex[g_indexmanage[id].cachednum].sps=g_indexmanage[id].flag_265sps;     //the first slice after sps.
                PVR_INTFD("cachedindex[%d].sps set to 1\n",g_indexmanage[id].cachednum);
                //MT_ERR_DEMUX("+++set sps %d\n",flag_265sps);
            }
            memcpy(g_indexmanage[id].cachedindex[g_indexmanage[id].cachednum].u32HdrData,RecIndex->u32HdrData,(sizeof(mt_u32 )<<1));
            g_indexmanage[id].cachedindex[g_indexmanage[id].cachednum].type=TYPE_STC;
            g_indexmanage[id].cachedindex[g_indexmanage[id].cachednum].pts=g_indexmanage[id].nowpts;
            g_indexmanage[id].cachedindex[g_indexmanage[id].cachednum].offset=((DMX_REC_INDEX_S*)RecIndex)->u64GlobalOffset;
        }
        g_indexmanage[id].flag_265sps=0;
        PVR_INTFD("cachednum=%d flag_265sps set to 0\n",g_indexmanage[id].cachednum);
        g_indexmanage[id].cachednum++;
    }
    if(CACHEDINDEX_NUM==g_indexmanage[id].cachednum){                 //cached full
        if(TYPE_STC==g_indexmanage[id].cachedindex[1].type){          //x stc x
            //mtos_printk1("++1.is stc\n");
            if(0xffffffff==g_indexmanage[id].cachedindex[1].pts){
                g_indexmanage[id].cachedindex[0]=g_indexmanage[id].cachedindex[1];
                g_indexmanage[id].cachedindex[1]=g_indexmanage[id].cachedindex[2];
                g_indexmanage[id].cachednum--;
                goto REDEAL;
            }
            if(TYPE_STC==g_indexmanage[id].cachedindex[0].type){      //stc stc x
                //mtos_printk1("++1.0 is stc\n");
                ((DMX_REC_INDEX_S*)RecIndex)->u64GlobalOffset=g_indexmanage[id].cachedindex[1].offset;
                ((DMX_REC_INDEX_S*)RecIndex)->u32FrameSize=(mt_u32 )(g_indexmanage[id].cachedindex[2].offset-g_indexmanage[id].cachedindex[1].offset);
                ((DMX_REC_INDEX_S*)RecIndex)->u32PtsMs=g_indexmanage[id].cachedindex[1].pts;
            }else{                                                    //pts stc x .first frame include pts
                //mtos_printk1("++1.0 is pts\n");
                ((DMX_REC_INDEX_S*)RecIndex)->u64GlobalOffset=g_indexmanage[id].cachedindex[0].offset;
                ((DMX_REC_INDEX_S*)RecIndex)->u32FrameSize=(mt_u32 )(g_indexmanage[id].cachedindex[2].offset-g_indexmanage[id].cachedindex[0].offset);
                ((DMX_REC_INDEX_S*)RecIndex)->u32PtsMs=g_indexmanage[id].cachedindex[1].pts;
                if((MT_UNF_VCODEC_TYPE_HEVC==RecInfo->video_type)&&(1==g_indexmanage[id].cachedindex[1].sps)){  //hevc-patch
                    ((DMX_REC_INDEX_S*)RecIndex)->u32PrivatePara=1;
                }else{
                    ((DMX_REC_INDEX_S*)RecIndex)->u32PrivatePara=0;
                }
            }
            memcpy(RecIndex->u32HdrData,g_indexmanage[id].cachedindex[1].u32HdrData,(sizeof(mt_u32 )<<1));
            if((MT_UNF_VCODEC_TYPE_AVS==RecInfo->video_type) || (MT_UNF_VCODEC_TYPE_VC1==RecInfo->video_type)){
                ((DMX_REC_INDEX_S*)RecIndex)->u32PrivatePara=(mt_u32 )g_indexmanage[id].avs_profile;
            }
            //RecIndex->CodecType = (mt_u32 )CodecTypeUnfToFmw(RecInfo->video_type);
            //printk("offset=%llx,%llx,%llx\n",cachedindex[0].offset,cachedindex[1].offset,cachedindex[2].offset);
            //mtos_printk1("\n+++av[%d].pts=%x_fs=%x_goff=%llx_h0=%x_h1=%x,Vtype=%d\n",id,((DMX_REC_INDEX_S*)RecIndex)->u32PtsMs,
            //              ((DMX_REC_INDEX_S*)RecIndex)->u32FrameSize,((DMX_REC_INDEX_S*)RecIndex)->u64GlobalOffset,
            //              RecIndex->u32HdrData[0],RecIndex->u32HdrData[1],
            //              RecInfo->video_type);
            g_indexmanage[id].cachedindex[0]=g_indexmanage[id].cachedindex[1];
            g_indexmanage[id].cachedindex[1]=g_indexmanage[id].cachedindex[2];
            g_indexmanage[id].cachednum--;
            //mtos_printk1("++mk ok\n");
            return 0;
        }else{                                      //x pts x
            //mtos_printk1("++1not stc\n");
            g_indexmanage[id].cachedindex[0]=g_indexmanage[id].cachedindex[1];
            g_indexmanage[id].cachedindex[1]=g_indexmanage[id].cachedindex[2];
            g_indexmanage[id].cachednum--;
            goto REDEAL;
        }
    }
    return ret1;
}
static s32 h264_get_frame_type_by_sc_(MT_UNF_DMX_REC_INDEX_RAW *pHdrInfo)
{//00 00 01 61 a1 a2 a3 a4->0x00000161 0xa1a2a3a4
  //FW_UINT8 be_ref = get_nb(pHdrInfo->u32HdrData[0], 6, 5);          //bit 6-5
  //FW_UINT8 nal_type = get_nb(pHdrInfo->u32HdrData[0], 4, 0);        //bit 4-0
  u8 slice[4];
  int offset=0,slice_type=MT_UNF_FRAME_TYPE_B;
  slice[0]=(0xff&(pHdrInfo->u32HdrData[1]>>24));
  slice[1]=(0xff&(pHdrInfo->u32HdrData[1]>>16));
  slice[2]=(0xff&(pHdrInfo->u32HdrData[1]>>8));
  slice[3]=(0xff&(pHdrInfo->u32HdrData[1]));
  //mtos_printk1("%x,%x\n",pHdrInfo->u32HdrData[0],pHdrInfo->u32HdrData[1]);
  offset+=code_check_get_ue(slice,offset,&slice_type,4);
  //mtos_printk1("++s1=%x\n",slice_type);
  if(!slice_type){
    offset+=code_check_get_ue(slice,offset,&slice_type,4);
    //mtos_printk1("++s2=%x\n",slice_type);
    switch(slice_type)
    {
        case 0: case 5: /* P */
            ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_P;
            break;
        case 1: case 6: /* B */
            ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_B;
            break;
        case 3: case 8: /* SP */
            ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_P;
            break;
        case 2: case 7: /* I */
            ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_I;
            break;
        case 4: case 9: /* SI */
            ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_I;
            break;
        default:
            ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_B;
            break;
    }
  }else{
      ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_B;
  }

  return 0;
}

static s32 vc1_get_frame_type_by_sc_(MT_UNF_DMX_REC_INDEX_RAW *pHdrInfo)
{
    u32 profile=0,FINTERPFLAG=0,RANGERED=0,MAXFRAMES=0,INTERLACE=0,skipbis=0;
    u32 temp=0;

    ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_UNKNOWN;
    if(vc1_SCFrameHeader == pHdrInfo->u32HdrData[0])
    {
        profile=(((DMX_REC_INDEX_S*)pHdrInfo)->u32PrivatePara>>30);
        if(profile <= 1){ //simple&main
            FINTERPFLAG=((((DMX_REC_INDEX_S*)pHdrInfo)->u32PrivatePara>>1)&0x01);
            RANGERED=((((DMX_REC_INDEX_S*)pHdrInfo)->u32PrivatePara>>7)&0x01);
            MAXFRAMES=((((DMX_REC_INDEX_S*)pHdrInfo)->u32PrivatePara>>4)&0x07);
            if(FINTERPFLAG){
                skipbis++;
            }
            skipbis+=2;
            if(RANGERED){
                skipbis++;
            }
            /*
                if(0==MAXFRAMES){
                    0 I
                    1 P
                }else{
                    1b P
                    01b I
                    00b B or BI
                }
            */
            temp=get_b(pHdrInfo->u32HdrData[1],(31-skipbis));
            if(0==MAXFRAMES){
                if(temp){ //1
                    ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_P;
                }else{    //0
                    ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_I;
                }
            }else{
                if(temp){ //1
                    ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_P;
                }else{
                    skipbis++;
                    temp=get_b(pHdrInfo->u32HdrData[1],(31-skipbis));
                    if(temp){ //01
                        ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_I;
                    }else{    //00
                        ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_B;
                    }
                }
            }
            //printf("++vc1.m=%x,%x,%x\n",pHdrInfo->u32HdrData[1],pHdrInfo->u32PrivatePara,((MT_UNF_DMX_REC_INDEX_S*)pHdrInfo)->enFrameType);
        }else{    //advanced:[0-15]+[31-46]
            INTERLACE=((((DMX_REC_INDEX_S*)pHdrInfo)->u32PrivatePara>>6)&0x01);
            if(INTERLACE){
                /*
                0b Progressive
                10b Frame-Interlace
                11b Field-Interlace
                */
                temp=get_b(pHdrInfo->u32HdrData[1],31);
                if(temp){
                    skipbis=2;
                }else{
                    skipbis=1;
                }
            }
            /*
                110b  I
                0b    P
                10b   B
                1110b BI
                1111b Skipped
            */
            temp=get_b(pHdrInfo->u32HdrData[1],(31-skipbis));
            if(!temp){  //0
                ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_P;
            }else{      //1
                skipbis++;
                temp=get_b(pHdrInfo->u32HdrData[1],(31-skipbis));
                if(!temp){  //10
                    ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_B;
                }else{
                    skipbis++;
                    temp=get_b(pHdrInfo->u32HdrData[1],(31-skipbis));
                    if(!temp){  //110
                        ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_I;
                    }else{      //111
                        ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_B;
                    }
                }
            }
            //printf("++vc1.a=%x,%x,%x\n",pHdrInfo->u32HdrData[1],pHdrInfo->u32PrivatePara,((MT_UNF_DMX_REC_INDEX_S*)pHdrInfo)->enFrameType);
        }
        return 0;
    }
    return -1;
}

static s32 mpeg4_get_frame_type_by_sc_(MT_UNF_DMX_REC_INDEX_RAW *pHdrInfo)
{
  u32 start_code = pHdrInfo->u32HdrData[0];
  u8 short_header = 0;

  if(VOP_START_CODE == start_code)
  {
    short_header = 0;
  }
  else
  {
    start_code = get_nb(start_code, 31, 10);
    if(SHORT_VIDEO_START_MARKER == start_code)
    {
      short_header = 1;
    }
  }

  ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_UNKNOWN;

  if(short_header)
  {
      ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = get_b(pHdrInfo->u32HdrData[1], 25) + 1;
  }
  else
  {
    if(VOP_START_CODE == pHdrInfo->u32HdrData[0])
    {
        ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = get_nb(pHdrInfo->u32HdrData[1], 31, 30) + 1;
    }
  }

  return 0;
}
static s32 mpeg2_get_frame_type_by_sc_(MT_UNF_DMX_REC_INDEX_RAW *pHdrInfo)
{
  ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_UNKNOWN;
  if(MPEG_PICTURE_START_CODE == pHdrInfo->u32HdrData[0])
  {
      ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = get_nb(pHdrInfo->u32HdrData[1], 21, 19);
  }

  return 0;
}
static s32 avs_get_frame_type_by_sc_(MT_UNF_DMX_REC_INDEX_RAW *pHdrInfo)
{
  u32 type = 0;
  u8 profile_id = ((DMX_REC_INDEX_S*)pHdrInfo)->u32PrivatePara;

  ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_UNKNOWN;

  if(AVS_I_PICTURE_START_CODE == pHdrInfo->u32HdrData[0])
  {
    ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_I;
  }
  else if(AVS_PB_PICTURE_START_CODE == pHdrInfo->u32HdrData[0])
  {
    if(profile_id == 0x48)
    {
      type = get_nb(pHdrInfo->u32HdrData[1], 7, 6);
    }
    else
    {
      type = get_nb(pHdrInfo->u32HdrData[1], 15, 14);
    }

    if(type == 1)
    {
        ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_P;
    }
    else
    {
        ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_B;
    }
  }
  else
  {
    return -1;
  }

  return 0;
}
static s32 hevc_get_frame_type_by_sc_(MT_UNF_DMX_REC_INDEX_RAW *pHdrInfo)
{
  u32 nal_unit_type = get_nb(pHdrInfo->u32HdrData[0], 6, 1);
  //PVR_INTFD("nal_unit_type=%d\n",nal_unit_type);  
  switch(nal_unit_type)
  {
    case 16://NAL_UNIT_CODED_SLICE_BLA_W_LP:
    case 17://NAL_UNIT_CODED_SLICE_BLA_W_RADL:
    case 18://NAL_UNIT_CODED_SLICE_BLA_N_LP:
    case 19://NAL_UNIT_CODED_SLICE_IDR_W_RADL:
    case 20://NAL_UNIT_CODED_SLICE_IDR_N_LP:
    case 21://NAL_UNIT_CODED_SLICE_CRA:
    case 22://NAL_UNIT_RESERVED_IRAP_VCL22:
    case 23://NAL_UNIT_RESERVED_IRAP_VCL23:
      ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_I;
      break;

    case 1://NAL_UNIT_CODED_SLICE_TRAIL_R:
    case 3://NAL_UNIT_CODED_SLICE_TSA_R:
    case 5://NAL_UNIT_CODED_SLICE_STSA_R:
    case 7://NAL_UNIT_CODED_SLICE_RADL_R:
    case 9://NAL_UNIT_CODED_SLICE_RASL_R:
      ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_P;
      break;

    default:
      ((DMX_REC_INDEX_S*)pHdrInfo)->enFrameType = MT_UNF_FRAME_TYPE_B;
      break;
  }

  return 0;
}

#define MAX_FRAME_INERTVAL_TIME 60

static mt_void av_pvr_process_index_data(mt_u32 chn_num)
{
    mt_s32 ret;
    mt_s32 index_wpnxt=0;
    PVR_INDEX_INST_S *bufferlist=NULL;
    MT_UNF_DMX_REC_INDEX_RAW index;
    mt_u32 ktime = 0;
    mt_u32 lost_pts_offset = 0;
    mt_u32 rec_waddr = 0;

    if(NULL==pvr_idx_buffer_inst){
        return;
    }
    bufferlist = &pvr_idx_buffer_inst[chn_num];
    if(NULL==bufferlist){
        return;
    }
    down(&bufferlist->IdxSem);
    if(NULL==bufferlist->pu8KnlVirAddr){
        //FATAL_ERROR("++++av.set stop\n");
        bufferlist->status=3;   //stoped
        up(&bufferlist->IdxSem);
        return;
    }
    if(0xffffffff == bufferlist->rec_dmxid){
        up(&bufferlist->IdxSem);
        return;
    }
    if(0==bufferlist->status){  //not prepared
        up(&bufferlist->IdxSem);
        return;
    }
    if(1==bufferlist->status){  //first
        memset(&g_indexmanage[chn_num],0x00,sizeof(INDEXMANAGE));
        memset(&g_pvr_index[chn_num],0x00,sizeof(MT_UNF_DMX_REC_INDEX_RAW));
        g_indexmanage[chn_num].last_get = ktime_to_ms(ktime_get_boottime());
        g_rec_waddr[chn_num] = 0;
        g_rec_parseiframe[chn_num] = MT_FALSE;
        PVR_INTFP("rec[%d] first init\n",chn_num);
        bufferlist->status=2;   //running
    }
    { //to check buffer,whether full
        index_wpnxt=(bufferlist->index_wp+1);
        if(index_wpnxt == bufferlist->index_num){
            index_wpnxt=0;
        }
        if(index_wpnxt==bufferlist->index_rp){
            up(&bufferlist->IdxSem);
            return;
        }
    }
    if (bufferlist->video_type == MT_UNF_VCODEC_TYPE_BUTT){
      up(&bufferlist->IdxSem);
      return;
    }
    memset(&index,0,sizeof(MT_UNF_DMX_REC_INDEX_RAW));
    ret=Dmx_AcquireRecIndex(bufferlist,chn_num,&index);
    if(0==ret){ //get frame-type. and save it
        switch(bufferlist->video_type){
            case MT_UNF_VCODEC_TYPE_HEVC:
                hevc_get_frame_type_by_sc_(&index);
            break;
            case MT_UNF_VCODEC_TYPE_VC1:
                vc1_get_frame_type_by_sc_(&index);
            break;
            case MT_UNF_VCODEC_TYPE_H264:
                h264_get_frame_type_by_sc_(&index);
            break;
            case MT_UNF_VCODEC_TYPE_MPEG4:
                mpeg4_get_frame_type_by_sc_(&index);
            break;
            case MT_UNF_VCODEC_TYPE_MPEG2:
                mpeg2_get_frame_type_by_sc_(&index);
            break;
            case MT_UNF_VCODEC_TYPE_AVS:
                avs_get_frame_type_by_sc_(&index);
            break;
            default:
                mpeg2_get_frame_type_by_sc_(&index);
            break;
        }
        { //patch for hevc h264
            if((MT_UNF_VCODEC_TYPE_HEVC==bufferlist->video_type)||
               (MT_UNF_VCODEC_TYPE_H264==bufferlist->video_type)){   //this for 264/265
                if(MT_UNF_FRAME_TYPE_I != index.publicindex.enFrameType){    //ignore P-frame
                    index.publicindex.enFrameType=MT_UNF_FRAME_TYPE_B;
                }
            }
            PVR_INTFD("enFrameType=%d u32PrivatePara=%d u32HdrData=0x%x 0x%x\n",
                index.publicindex.enFrameType,index.publicindex.u32PrivatePara,index.u32HdrData[0],index.u32HdrData[1]);
            /*
            fixed #30933, stream has vps/sps before very frame,if change frame type by u32PrivatePara will all frame is Iframe,trick play will failed
            the stream of #110148 can parse Iframe, not need to change frame by u32PrivatePara(vps will set to 1)
            #31108: HEVC_TP_CLEAR_STREAM.ts can't parse Iframe
            */
            #if 1
            if((MT_UNF_VCODEC_TYPE_HEVC==bufferlist->video_type)){
                if(MT_UNF_FRAME_TYPE_I == index.publicindex.enFrameType){
                    if(g_rec_parseiframe[chn_num] == MT_FALSE){
                        PVR_INTFP("rec[%d] hevc can parse iframe\n",chn_num);
                        g_rec_parseiframe[chn_num] = MT_TRUE;
                    }
                }
                if(1==index.publicindex.u32PrivatePara && g_rec_parseiframe[chn_num] == MT_FALSE){
                    index.publicindex.enFrameType=MT_UNF_FRAME_TYPE_I;
                }
            }
            #endif
        }
        
        ktime = ktime_to_ms(ktime_get_boottime());
        //no signal
        #if 1
        if (g_pvr_index[chn_num].publicindex.u32PtsMs
          && ((ktime - g_indexmanage[chn_num].last_get) > 1000)
          && (abs(index.publicindex.u32PtsMs - g_pvr_index[chn_num].publicindex.u32PtsMs) > 1000)){
          if (index.publicindex.u32PtsMs > g_pvr_index[chn_num].publicindex.u32PtsMs)
          {
            lost_pts_offset = index.publicindex.u32PtsMs - g_pvr_index[chn_num].publicindex.u32PtsMs;
            printk(KERN_ERR "Dmx_AcquireRecIndex_lost_signal, pts_offset[%d][%x, %x][%d]\n",
              lost_pts_offset, 
              index.publicindex.u32PtsMs,
              g_pvr_index[chn_num].publicindex.u32PtsMs,
              ktime - g_indexmanage[chn_num].last_get);
          }
          else
          {
            //ring stream
            printk(KERN_ERR "ring stream back to front!\n");
          }
        }
        #endif
        if (((ktime - g_indexmanage[chn_num].last_get) >= MAX_FRAME_INERTVAL_TIME) && (g_pvr_index[chn_num].publicindex.enFrameType != MT_UNF_FRAME_TYPE_UNKNOWN))
        {
          if ((ktime - g_indexmanage[chn_num].last_get) >= 500)
            MT_WARN_PVR("Dmx_AcquireRecIndex cost time[%d]pts_offset[%d][%x, %x]\n",
              ktime - g_indexmanage[chn_num].last_get,
              index.publicindex.u32PtsMs - g_pvr_index[chn_num].publicindex.u32PtsMs, 
              index.publicindex.u32PtsMs,
              g_pvr_index[chn_num].publicindex.u32PtsMs); //may print too much when pts rewind,default disable 
          if (lost_pts_offset){
            index.publicindex.u32PrivatePara = ktime_to_ms(ktime_get_boottime());
            g_indexmanage[chn_num].last_get = index.publicindex.u32PrivatePara;
          }
          else
          {
            index.publicindex.u32PrivatePara = g_pvr_index[chn_num].publicindex.u32PrivatePara + MAX_FRAME_INERTVAL_TIME;
            g_indexmanage[chn_num].last_get += MAX_FRAME_INERTVAL_TIME;
          }
        }
        else
        {
          if ((ktime - g_indexmanage[chn_num].last_get) > 3000){
            printk(KERN_ERR "Dmx_AcquireRecIndex_err cost long time[%d]\n", ktime - g_indexmanage[chn_num].last_get);
          }
          index.publicindex.u32PrivatePara = ktime_to_ms(ktime_get_boottime());
          g_indexmanage[chn_num].last_get = index.publicindex.u32PrivatePara;
        }
        memcpy(&(((DMX_REC_INDEX_S *)(bufferlist->pu8KnlVirAddr))[bufferlist->index_wp]),&index.publicindex,sizeof(DMX_REC_INDEX_S));
        bufferlist->index_wp=index_wpnxt;
        #if 0
  			printk(KERN_ERR "index[%d]time %d enFrameType %d u32FrameSize %d  u32PtsMs %x u64GlobalOffset %lld pts_off[%d]\n",
  				index_wpnxt, index.publicindex.u32PrivatePara, 
  				index.publicindex.enFrameType, 
  				index.publicindex.u32FrameSize, 
  				index.publicindex.u32PtsMs,
  				index.publicindex.u64GlobalOffset,
  				index.publicindex.u32PtsMs - g_pvr_index[chn_num].publicindex.u32PtsMs);
        #endif
    		g_indexmanage[chn_num].last_update = g_indexmanage[chn_num].last_get;
    		memcpy(&g_pvr_index[chn_num].publicindex,&index.publicindex,sizeof(DMX_REC_INDEX_S));
    		g_rec_waddr[chn_num] = reg_get_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(bufferlist->rec_dmxid);
    }
    else{
      #if 1
      if (g_pvr_index[chn_num].publicindex.enFrameType != MT_UNF_FRAME_TYPE_UNKNOWN){
        ktime = ktime_to_ms(ktime_get_boottime());
        if ((ktime - g_indexmanage[chn_num].last_update >= 200) && (ktime - g_indexmanage[chn_num].last_update <= 900))
        {
          rec_waddr = reg_get_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(bufferlist->rec_dmxid);
          if (rec_waddr != g_rec_waddr[chn_num]){
            g_rec_waddr[chn_num] = rec_waddr;
            memcpy(&index.publicindex, &g_pvr_index[chn_num].publicindex,sizeof(DMX_REC_INDEX_S));
            index.publicindex.enFrameType = MT_UNF_FRAME_TYPE_UNKNOWN;
            //index.publicindex.u32FrameSize = 188;
            index.publicindex.u32PrivatePara=ktime_to_ms(ktime_get_boottime());
            //index.publicindex.u32PtsMs += (index.publicindex.u32PrivatePara - g_pvr_index[chn_num].publicindex.u32PrivatePara);
            //index.publicindex.u64GlobalOffset += 188;
#if 1
      			printk(KERN_ERR "insert time %d enFrameType %d u32PtsMs %x u64GlobalOffset %lld\n",
      				index.publicindex.u32PrivatePara, 
      				index.publicindex.enFrameType, 
      				index.publicindex.u32PtsMs,
      				index.publicindex.u64GlobalOffset);
#endif
            memcpy(&(((DMX_REC_INDEX_S *)(bufferlist->pu8KnlVirAddr))[bufferlist->index_wp]),&index.publicindex,sizeof(DMX_REC_INDEX_S));
            bufferlist->index_wp=index_wpnxt;
            g_indexmanage[chn_num].last_update = index.publicindex.u32PrivatePara;
          }
        }
      }
      #endif
    }
    up(&bufferlist->IdxSem);
}

static mt_s32 MT_PVR_Idx_Parse(mt_void *arg)
{
    while (MT_TRUE){
        int i;
        if (kthread_should_stop()){
            break;
        }
        for(i=0;i < CFG_PVR_IDX_INS_COUNT; i++)
        {
            if(NULL!=pvr_idx_buffer_inst[i].pu8KnlVirAddr)
                av_pvr_process_index_data(i);
        }
        set_current_state(TASK_UNINTERRUPTIBLE);
        schedule_timeout(HZ/100);               //sleep 10ms
    }
    return 0;
}
#endif
MT_S32 __init PVR_DRV_ModInit(MT_VOID)
{
    MT_S32 ret;
    MT_U32 i;
    MT_INFO_PVR("-------------------pvr.mod.init\n");
    PVR_INTFP("pvr drv ver=1.0.1\n");
    ret = mt_drv_module_register(MT_ID_PVR, PVR_NAME, MT_NULL);
    if (MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }

    PvrFileOps.owner            = THIS_MODULE;
    PvrFileOps.open             = PVR_DRV_Open;
    PvrFileOps.release          = PVR_DRV_Close;
	  PvrFileOps.unlocked_ioctl   = PVR_DRV_Ioctl;

    strncpy(PvrDev.devfs_name, UMAP_DEVNAME_PVR, strlen(UMAP_DEVNAME_PVR)+1);
    PvrDev.minor  = UMAP_MIN_MINOR_PVR;
    PvrDev.owner  = THIS_MODULE;
    PvrDev.fops   = &PvrFileOps;
    PvrDev.drvops = MT_NULL;

    if (mt_drv_dev_register(&PvrDev) < 0)
    {
        mt_drv_module_unregister(MT_ID_PVR);
        return MT_FAILURE;
    }

    for (i = 0 ; i < PVR_PLAY_MAX_CHN_NUM; i++)
    {
        PvrPlayChan[i] = 0;
    }

    for (i = 0 ; i < PVR_REC_MAX_CHN_NUM; i++)
    {
        PvrRecChan[i] = 0;
    }
    #if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
    {//create thread && set RR 95 level
        struct sched_param param;
        pvr_idx_buffer_inst=vmalloc(CFG_PVR_IDX_INS_COUNT*sizeof(PVR_INDEX_INST_S));
        if(NULL==pvr_idx_buffer_inst){
            mt_drv_module_unregister(MT_ID_PVR);
            return MT_FAILURE;
        }
        memset(pvr_idx_buffer_inst,0x00,CFG_PVR_IDX_INS_COUNT*sizeof(PVR_INDEX_INST_S));
        for(i=0;i<CFG_PVR_IDX_INS_COUNT;i++){
            sema_init(&pvr_idx_buffer_inst[i].IdxSem, 1);
        }
        PvrIdxThread = kthread_create(MT_PVR_Idx_Parse, MT_NULL, "KrPvrIdx");
        if (IS_ERR(PvrIdxThread)){
            MT_ERR_PVR("create kthread failed\n");
            vfree(pvr_idx_buffer_inst);
            pvr_idx_buffer_inst=NULL;
            mt_drv_module_unregister(MT_ID_PVR);
            return MT_FAILURE;
        }
        param.sched_priority = 95;
        sched_setscheduler(PvrIdxThread, SCHED_RR, &param);
        wake_up_process(PvrIdxThread);
    }
    #endif
    MT_INFO_PVR("-------------------\n");
    return MT_SUCCESS;
}

MT_VOID __exit PVR_DRV_ModExit(MT_VOID)
{
    #if (defined(CONFIG_MT_CHIP_SYMPHONY4) || defined(CONFIG_MT_CHIP_SYMPHONY6)) && !defined(CONFIG_MT_SECURE_MEDIA_PATH)
        if(PvrIdxThread){
            kthread_stop(PvrIdxThread);
            PvrIdxThread=NULL;
        }
        if(pvr_idx_buffer_inst){
            vfree(pvr_idx_buffer_inst);
            pvr_idx_buffer_inst=NULL;
        }
    #endif
    mt_drv_dev_unregister(&PvrDev);

    mt_drv_module_unregister(MT_ID_PVR);
}

