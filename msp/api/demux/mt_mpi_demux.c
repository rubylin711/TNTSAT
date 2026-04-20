/* Copyright (c) 2015 Montage Technology Group Limited and its affiliated companies         */

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/mman.h>		/* mmap */

#include "mt_type.h"
#include "mt_debug.h"

#include "mt_drv_struct.h"
#include "mt_module.h"

#include "mt_module_debug.h"
#include "mt_mpi_demux.h"
#include "drv_demux_config.h"
#include "drv_demux_ioctl.h"
#include "mt_mpi_mem.h"
#include "mpi_memdev.h"
#include "mt_common.h"

#define DMX_DEFAULT_CHANBUF_SIZE        (16 * 1024)
#define DMX_INVALID_CHAN_ID             (0xffff)

/********************** Global Variable define **************************/
#define DMX_COPY_PES                          0
#define DMX_TRPP_REGRISTER_SIZE     0x10000

mt_s32 g_s32DmxFd = -1;     /* the file discreption for DEMUX module */

static const mt_char    DmxDevName[]  = "/dev/" UMAP_DEVNAME_DEMUX;

static DMX_MMZ_BUF_S    g_stTsBuf[DMX_RAMPORT_CNT];
static DMX_MMZ_BUF_S    g_stChanBuf[DMX_CHANNEL_CNT];
static DMX_MMZ_BUF_S    g_stChanDescBuf[DMX_CHANNEL_CNT];
static DMX_MMZ_BUF_S    g_stChanPesBuf[DMX_CHANNEL_CNT];
static DMX_MMZ_BUF_S    g_RecMmzBuf[DMX_CNT];
static DMX_MMZ_BUF_S    g_RecIdxMmzBuf[DMX_CNT];

static DMX_MMZ_BUF_S    g_LinkRecMmzBuf[DMX_CNT][8];
static mt_u8 			g_LinkRecNodeNum;

static pthread_mutex_t g_stDmxMutex = PTHREAD_MUTEX_INITIALIZER;

static const mt_char s_szDmxVersion[] __attribute__((used)) = "SDK_VERSION:["\
                            MKMARCOTOSTR(SDK_VERSION)"] Build Time:["\
                            __DATE__", "__TIME__"]";

static ulong g_dmxTrppRegristerBaseAddr = 0;


#define MPIDmxCheckDeviceFd()           \
    do                                  \
    {                                   \
        if (-1 == g_s32DmxFd)           \
        {                               \
            MT_ERR_DEMUX("Dmx not init!\n"); \
            return MT_ERR_DMX_NOT_INIT; \
        }                               \
    } while (0)

#define MPIDmxCheckPointer(p)           \
    do                                  \
    {                                   \
        if (MT_NULL == p)               \
        {                               \
            MT_ERR_DEMUX("Null Pointer!\n"); \
            return MT_ERR_DMX_NULL_PTR; \
        }                               \
    } while (0)

#define MPIDmxCheckDmxId(u32DmxId) \
    do {\
        if (!(u32DmxId < DMX_CNT))\
        {\
            MT_ERR_DEMUX("Invalid DmxId %u\n", u32DmxId); \
            return MT_ERR_DMX_INVALID_PARA; \
        } \
    } while (0)

#define DMX_BUFFERHANDLE2PORTID(BufferHandle)   (BufferHandle & 0xff)
#define DMX_PORTID2BUFFERHANDLE(PortId)         (PortId | 0x00000400 | (MT_ID_DEMUX << 16))
#define DMX_CHECK_BUFFERHANDLE(BufferHandle)    \
do{\
    if(((BufferHandle & 0xffffff00) != DMX_PORTID2BUFFERHANDLE(0)) \
        || (DMX_BUFFERHANDLE2PORTID(BufferHandle) >= DMX_RAMPORT_CNT))\
    {\
        MT_ERR_DEMUX("Invalid buffer handle:0x%x\n", BufferHandle); \
        return MT_ERR_DMX_INVALID_PARA;\
    }\
}while(0)

static inline mt_u32 dmx_inl_new(ulong port)
{
	return mpi_read_reg32((void *)port);
}

static inline mt_void dmx_outl_new(ulong port , mt_u32 val)
{
	mpi_write_reg32((void *)port, val);
}

/************************************************************************************
 ************************************************************************************

                         Initialize  Module

 ************************************************************************************
 *************************************************************************************/
#if  DMX_COPY_PES

static mt_u32 MPIPesMemSize[DMX_CHANNEL_CNT];

/**
 \brief check whether pes pUserMsg is valid or not.
return -1 if invalid.
return MT_UNF_DMX_TYPE_WHOLE if included all the pes
return MT_UNF_DMX_DATA_TYPE_HEAD if just only included the head of pes.
return MT_UNF_DMX_DATA_TYPE_TAIL if just only included the tail of pes
return MT_UNF_DMX_DATA_TYPE_BODY if not included head and tail

 \attention
 \none
 \param[in] pUserMsg
 \param[in] u32PmsgLen

 \retval none
 \return none

 \see
 \li ::
 */
static mt_s32 MPICheckPesUserMsg(const MT_UNF_DMX_DATA_S* pstBuf, mt_u32 u32BufSize)
{
    mt_u32 u32HeadType, u32TailType;

    if (pstBuf[0].enDataType == MT_UNF_DMX_DATA_TYPE_WHOLE)
    {
        return MT_UNF_DMX_DATA_TYPE_WHOLE;
    }

    u32HeadType = pstBuf[0].enDataType;
    u32TailType = pstBuf[u32BufSize - 1].enDataType;
    if (u32HeadType == MT_UNF_DMX_DATA_TYPE_HEAD)
    {
        if (u32TailType == MT_UNF_DMX_DATA_TYPE_TAIL)
        {
            return MT_UNF_DMX_DATA_TYPE_WHOLE;
        }
        else
        {
            return MT_UNF_DMX_DATA_TYPE_HEAD;
        }
    }
    else
    {
        if (u32TailType == MT_UNF_DMX_DATA_TYPE_TAIL)
        {
            return MT_UNF_DMX_DATA_TYPE_TAIL;
        }
        else
        {
            return MT_UNF_DMX_DATA_TYPE_BODY;
        }
    }
}

/**
 \brief return  the total length of pes
 \attention
\none
 \param[in] pu8Dst
 \param[in] pUserMsg
 \param[in] u32PmsgLen

 \retval none
 \return none

 \see
 \li ::
 */
static mt_u32 MPICopyPesTogether(mt_u8* pu8Dst, MT_UNF_DMX_DATA_S* pstBuf, mt_u32 u32BufSize)
{
    mt_u32 u32CopyedLen = 0;
    mt_u32 i;

    for (i = 0; i < u32BufSize; i++)
    {
        memcpy(pu8Dst + u32CopyedLen, pstBuf[i].pu8Data, pstBuf[i].u32Size);
        u32CopyedLen += pstBuf[i].u32Size;
    }

    return u32CopyedLen;
}

/**
 \brief malloc for pes, return MT_SUCCESS if successful. return MT_FAILURE if failure
 \attention
\none
 \param[in] hChannel
 \param[in] u32Size
 \param[out] pu8PesAddr

 \retval none
 \return none

 \see
 \li ::
 */
static mt_s32 MPIPesMemMalloc(mt_handle hChannel, mt_u32 u32Size, mt_u8** pu8PesAddr)
{
    MT_UNF_DMX_CHAN_ATTR_S stChAttr;
    mt_u32 u32ChnID;
	stChAttr.u32BufSize= 0;
    u32ChnID = DMX_CHANID(hChannel);
    if (u32ChnID >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("channel handle error:%d\n",u32ChnID);
        return MT_FAILURE;
    }
    MT_MPI_DMX_GetChannelAttr(hChannel, &stChAttr);
    *pu8PesAddr = (mt_u8*) mt_malloc(MT_ID_DEMUX, u32Size);
    if (!(*pu8PesAddr))
    {
        MT_ERR_DEMUX("Pes mem malloc failed!\n");
        *pu8PesAddr = MT_NULL;
        return MT_FAILURE;
    }

    MPIPesMemSize[u32ChnID] += u32Size;
    return MT_SUCCESS;
}

/**
 \brief free pes buffer
 \attention
none
 \param[in] hChannel
 \param[in] u32Size
 \param[in] pu8PesAddr

 \retval none
 \return none

 \see
 \li ::
 */
static mt_s32 MPIPesMemFree(mt_handle hChannel, mt_u32 u32Size, mt_u8* pu8PesAddr)
{
    mt_u32 u32ChnID;

    MPIDmxCheckPointer(pu8PesAddr);

    u32ChnID = DMX_CHANID(hChannel);
    if (u32ChnID >= DMX_CHANNEL_CNT)
    {
        MT_ERR_DEMUX("channel handle error:%d\n",u32ChnID);
        return MT_FAILURE;
    }

    mt_free(MT_ID_DEMUX, pu8PesAddr);

    MPIPesMemSize[u32ChnID] -= u32Size;

    return MT_SUCCESS;
}

#endif /* #if DMX_COPY_PES */

static MT_UNF_DMX_CHAN_TYPE_E MPIGetChnType(mt_handle hChannel)
{
    MT_UNF_DMX_CHAN_ATTR_S stChAttr;
    mt_s32 ret;

    ret = MT_MPI_DMX_GetChannelAttr(hChannel, &stChAttr);
    if(ret != MT_SUCCESS)
    {
        return MT_UNF_DMX_CHAN_TYPE_BUTT;
    }
    return stChAttr.enChannelType;
}



static mt_s32 DMX_MPI_PortGetTypeAndID(const MT_UNF_DMX_PORT_E Port, DMX_PORT_MODE_E *PortMode, mt_u32 *PortId)
{
    mt_s32 ret = MT_ERR_DMX_INVALID_PARA;
    //printf("============================%s  %d\n",__FILE__,__LINE__);
    switch (Port)
    {
        case MT_UNF_DMX_PORT_IF_0 :
        case MT_UNF_DMX_PORT_IF_1 :
        case MT_UNF_DMX_PORT_IF_2 :
        case MT_UNF_DMX_PORT_IF_3 :
        case MT_UNF_DMX_PORT_IF_4 :
        case MT_UNF_DMX_PORT_IF_5 :
        case MT_UNF_DMX_PORT_IF_6 :
        case MT_UNF_DMX_PORT_IF_7 :

        {
            mt_u32 Id = (mt_u32)Port - (mt_u32)MT_UNF_DMX_PORT_IF_0;
            if (Id < DMX_IFPORT_CNT)
            {
                *PortMode   = DMX_PORT_MODE_TUNER;
                *PortId     = Id;

                ret = MT_SUCCESS;
            }

            break;
        }

        case MT_UNF_DMX_PORT_TSI_0 :
        case MT_UNF_DMX_PORT_TSI_1 :
        case MT_UNF_DMX_PORT_TSI_2 :
        case MT_UNF_DMX_PORT_TSI_3 :
        case MT_UNF_DMX_PORT_TSI_4 :
        case MT_UNF_DMX_PORT_TSI_5 :
        case MT_UNF_DMX_PORT_TSI_6 :
        case MT_UNF_DMX_PORT_TSI_7 :
        {
            mt_u32 Id = (mt_u32)Port - (mt_u32)MT_UNF_DMX_PORT_TSI_0;

            if (Id < DMX_TSIPORT_CNT)
            {
                *PortMode   = DMX_PORT_MODE_TUNER;
                /*I want design both IF port and TSIN port  as DMX_PORT_MODE_TUNER in driver ,so that the driver will be change little,
                (history reason ,both if and TSIN port areTUNER port), so ,the TSIN port have offset of DMX_IFPORT_CNT*/
                *PortId     = Id + DMX_IFPORT_CNT;

                ret = MT_SUCCESS;
            }
            else if (Id == DMX_TSIPORT_CNT)
            {
                *PortMode   = DMX_PORT_MODE_TUNER;
                /*I want design both IF port and TSIN port  as DMX_PORT_MODE_TUNER in driver ,so that the driver will be change little,
                (history reason ,both if and TSIN port areTUNER port), so ,the TSIN port have offset of DMX_IFPORT_CNT*/
                *PortId     = Id - DMX_TSIPORT_CNT;

                ret = MT_SUCCESS;
            }
            break;
        }

        case MT_UNF_DMX_PORT_RAM_0 :
        case MT_UNF_DMX_PORT_RAM_1 :
        case MT_UNF_DMX_PORT_RAM_2 :
        case MT_UNF_DMX_PORT_RAM_3 :
        case MT_UNF_DMX_PORT_RAM_4 :
        case MT_UNF_DMX_PORT_RAM_5 :
        case MT_UNF_DMX_PORT_RAM_6 :
        case MT_UNF_DMX_PORT_RAM_7 :
        {
            mt_u32 Id = (mt_u32)Port - (mt_u32)MT_UNF_DMX_PORT_RAM_0;

            if (Id < DMX_RAMPORT_CNT)
            {
                *PortMode   = DMX_PORT_MODE_RAM;
                *PortId     = Id;

                ret = MT_SUCCESS;
            }

            break;
        }

        default :
        {
            MT_ERR_DEMUX("Invalid port 0x%x\n", Port);
        }
    }
    //printf("============================*PortMode=%d, *PortId=%d\n",*PortMode,*PortId);
    return ret;
}

#ifdef DMX_USE_ECM
static mt_u32 MPIGetSwFlag(mt_handle hChannel)
{
    mt_s32 ret;
    DMX_ChanSwGet_S Param;

	if (g_s32DmxFd == -1)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return 0;
	}

    Param.hChannel = hChannel;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_GET_CHAN_SWFLAG, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        return Param.u32SwFlag;
    }
    return 0;
}
#endif

static mt_s32 MPIReleaseBuf(mt_handle hChannel, mt_u32 u32ReleaseNum, MT_UNF_DMX_DATA_S *pstBuf)
{
    DMX_RelMsg_S Param;
    ulong u32UsrAddr = 0;
    mt_u32 u32ChId = DMX_INVALID_CHAN_ID;
    mt_u32 i = 0;
    MT_UNF_DMX_CHAN_TYPE_E chan_type = MT_UNF_DMX_CHAN_TYPE_BUTT;

    u32ChId = DMX_CHANID(hChannel);
    chan_type = MPIGetChnType(hChannel);

    switch (chan_type)
    {
        case MT_UNF_DMX_CHAN_TYPE_HW_PES:
        case MT_UNF_DMX_CHAN_TYPE_POST:
        {
            for (i = 0; i < u32ReleaseNum; i++)
            {
                u32UsrAddr = (ulong)pstBuf[i].pu8Data;

                if ((u32UsrAddr >= g_stChanBuf[u32ChId].u32BufUsrVirAddr) &&
                    ((u32UsrAddr - g_stChanBuf[u32ChId].u32BufUsrVirAddr) <= g_stChanBuf[u32ChId].u32BufSize))
                {
                    pstBuf[i].data_phy_addr = (u32UsrAddr - g_stChanBuf[u32ChId].u32BufUsrVirAddr) + g_stChanBuf[u32ChId].u32BufPhyAddr;
                }
                else
                {
                    MT_ERR_DEMUX("Invalid HW PES Usraddr = %#x of channel data: ChanId = %d\n", u32UsrAddr, u32ChId);
                    return MT_ERR_DMX_INVALID_PARA;                  
                }
            }            
            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_SEC:
        case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
        {
            for (i = 0; i < u32ReleaseNum; i++)
            {
                u32UsrAddr = (ulong)pstBuf[i].pu8Data;

                if ((u32UsrAddr >= g_stChanBuf[u32ChId].u32BufUsrVirAddr) &&
                    ((u32UsrAddr - g_stChanBuf[u32ChId].u32BufUsrVirAddr) < g_stChanBuf[u32ChId].u32BufSize))
                {
                    pstBuf[i].data_phy_addr = (u32UsrAddr - g_stChanBuf[u32ChId].u32BufUsrVirAddr) + g_stChanBuf[u32ChId].u32BufPhyAddr;
                }
                else if (u32UsrAddr == (g_stChanBuf[u32ChId].u32BufUsrVirAddr + DMX_SYMPHONY_SEC_DESC_BUF_SIZE +  g_stChanBuf[u32ChId].u32BufSize))
                {
                    pstBuf[i].data_phy_addr = g_stChanBuf[u32ChId].u32BufPhyAddr + DMX_SYMPHONY_SEC_DESC_BUF_SIZE +  g_stChanBuf[u32ChId].u32BufSize;
                    MT_DBG_DEMUX("reassamble buf u32UsrAddr = %#x\n", u32UsrAddr);
                }
                else
                {
                    MT_ERR_DEMUX("Invalid SEC Usraddr = %#x of channel data: ChanId = %d\n", u32UsrAddr, u32ChId);
                    return MT_ERR_DMX_INVALID_PARA;
                }                
            }

            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            for (i = 0; i < u32ReleaseNum; i++)
            {
                u32UsrAddr = (ulong)pstBuf[i].pu8Data;

                if ((u32UsrAddr >= g_stChanDescBuf[u32ChId].u32BufUsrVirAddr) &&
                    ((u32UsrAddr - g_stChanDescBuf[u32ChId].u32BufUsrVirAddr) < g_stChanDescBuf[u32ChId].u32BufSize))
                {
                    pstBuf[i].data_phy_addr = (u32UsrAddr - g_stChanDescBuf[u32ChId].u32BufUsrVirAddr) + g_stChanDescBuf[u32ChId].u32BufPhyAddr;
                }
                else
                {
                    MT_ERR_DEMUX("Invalid PES Usraddr = %#x of channel data: ChanId = %d\n", u32UsrAddr, u32ChId);
                    return MT_ERR_DMX_INVALID_PARA;
                }
            }

            break;
        }

        default:
			MT_ERR_DEMUX("Release Buf Err chantype:%d\n", chan_type);
            return MT_ERR_DMX_INVALID_PARA;
    }

    Param.hChannel = hChannel;
    Param.u32ReleaseNum = u32ReleaseNum;
    Param.pstBuf = pstBuf;

    /*given the address and length of buffer for this interface, given data in kernel mode */
    return ioctl(g_s32DmxFd, CMD_DEMUX_RELEASE_MSG, (ulong)&Param);
}

static mt_s32 MPIDMXDestroyTSBuffer(mt_u32 PortId)
{
    mt_s32 ret;

    if (0 == g_stTsBuf[PortId].u32BufPhyAddr)
    {
        MT_ERR_DEMUX("invalid buffer addr!\n");
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (MT_SUCCESS != mt_munmap((mt_void*)g_stTsBuf[PortId].u32BufUsrVirAddr))
    {
        MT_ERR_DEMUX("TS buffer unmap failed\n");

        return MT_ERR_DMX_MUNMAP_FAILED;
    }

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_DEINIT, (ulong)&PortId);
    if (MT_SUCCESS == ret)
    {
        memset(&g_stTsBuf[PortId], 0, sizeof(DMX_MMZ_BUF_S));
    }

    return ret;
}

static mt_s32 MPIDMXDestroyChannel(mt_u32 ChanId)
{
    mt_handle hChannel = DMX_CHANHANDLE(ChanId);
	mt_s32 ret = MT_SUCCESS;

	//bug 129285, check u32countRef before destroy channel.
	//If it is not 0, don't destroy.
	ret = ioctl(g_s32DmxFd, CMD_DEMUX_GET_CHAN_REF, (ulong)&hChannel);
	if (ret != MT_SUCCESS)
	{
		return ret ;
	}
	
    /*the record-only channel dosen't have channel buffer */
    if (0 != g_stChanBuf[ChanId].u32BufPhyAddr)
    {
        if (MT_SUCCESS != mt_munmap((mt_void*)g_stChanBuf[ChanId].u32BufUsrVirAddr))
        {
            MT_ERR_DEMUX("channel %u buffer unmap failed\n", ChanId);
            return MT_ERR_DMX_MUNMAP_FAILED;
        }

        memset(&g_stChanBuf[ChanId], 0, sizeof(DMX_MMZ_BUF_S));
    }

    if (0 != g_stChanDescBuf[ChanId].u32BufUsrVirAddr)
    {
        if (MT_SUCCESS != mt_munmap((mt_void*)g_stChanDescBuf[ChanId].u32BufUsrVirAddr))
        {
            MT_ERR_DEMUX("channel %u desc buffer unmap failed\n", ChanId);
            return MT_ERR_DMX_MUNMAP_FAILED;
        }

        memset(&g_stChanDescBuf[ChanId], 0, sizeof(DMX_MMZ_BUF_S));
    }

    if(NULL  !=  (mt_u8 *)g_stChanPesBuf[ChanId].u32BufUsrVirAddr)
    {
        if (MT_SUCCESS != mt_munmap((mt_void*)g_stChanPesBuf[ChanId].u32BufUsrVirAddr)){
            MT_ERR_DEMUX("channel %u pes unmap failed\n", ChanId);
            return MT_ERR_DMX_MUNMAP_FAILED;
        }
        memset(&g_stChanPesBuf[ChanId],0x00,sizeof(DMX_MMZ_BUF_S));
    }

	//unmap memory first, then release it
	ret = ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_DEL, (ulong)&hChannel);
	if(ret != MT_SUCCESS)
	{
		return ret ;
	}
	return ret ;

}

mt_u32 MT_MPI_DMX_GetRegister(mt_u32 registerOffset)
{
#if 0
    //mt_s32 regs = registerOffset;

    //return ioctl(g_s32DmxFd, CMD_DEMUX_GET_REGISTER, (ulong)&regs);
    MT_INFO_DEMUX("g_dmxTrppRegristerBaseAddr = 0x%x\n",g_dmxTrppRegristerBaseAddr);
    if(g_dmxTrppRegristerBaseAddr == 0) return 0;
    mt_u32 value = dmx_inl_new(g_dmxTrppRegristerBaseAddr + registerOffset);
    MT_INFO_DEMUX("g_dmxTrppRegristerBaseAddr = 0x%x,  value=0x%x\n",g_dmxTrppRegristerBaseAddr,value);
    return  value;
#else
	mt_s32       ret;
	ret =  ioctl(g_s32DmxFd, CMD_DEMUX_GET_REGISTER, &registerOffset);
	MT_INFO_DEMUX("[%s %d]reg(0x%x)=0x%x\n", __FUNCTION__, __LINE__, registerOffset, ret);
	return ret;
#endif
}

mt_void MT_MPI_DMX_SetRegister(mt_u32 registerOffset, mt_u32 value)
{
#if 0
    //DMX_SetRegister_S Param;

	if ((-1 == g_s32DmxFd) || g_dmxTrppRegristerBaseAddr == 0)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return;
	}

    //Param.u32RegisterOffset = registerOffset;
    //Param.u32RegisterValue = value;
    //ioctl(g_s32DmxFd, CMD_DEMUX_SET_REGISTER, (ulong)&Param);
    return  dmx_outl_new(g_dmxTrppRegristerBaseAddr + registerOffset,  value);
#else
	mt_s32       ret;
	DMX_SetRegister_S reg_rw = {0};

	reg_rw.u32RegisterOffset = registerOffset;
	reg_rw.u32RegisterValue = value;
	
	ret =  ioctl(g_s32DmxFd, CMD_DEMUX_SET_REGISTER, &reg_rw);
	MT_INFO_DEMUX("[%s %d]ret=0x%x, offset=0x%x, value=0x%x\n", __FUNCTION__, __LINE__, ret, reg_rw.u32RegisterOffset, reg_rw.u32RegisterValue);
#endif
}

mt_void MT_MPI_DMX_GetEsBuffAddr(mt_handle hChannel,
						ulong *esBuffAddr,  ulong *esBuffSize,  ulong *kerVirEsBuffAddr,
						ulong *kerVirDescBuffAddr, ulong *descBuffSize)
{

    mt_u32 u32ChId = DMX_CHANID(hChannel);
    //printf("MT_MPI_DMX_GetEsBuffAddr >>> u32ChId = 0x%x\n",u32ChId);
    if(esBuffAddr) *esBuffAddr = g_stChanBuf[u32ChId].u32BufUsrVirAddr;
    if(kerVirEsBuffAddr) *kerVirEsBuffAddr = g_stChanBuf[u32ChId].u32BufKerVirAddr;
    if(esBuffSize) *esBuffSize = g_stChanBuf[u32ChId].u32BufSize;
    if(kerVirDescBuffAddr) *kerVirDescBuffAddr = g_stChanDescBuf[u32ChId].u32BufKerVirAddr;
    if(descBuffSize)  *descBuffSize = g_stChanDescBuf[u32ChId].u32BufSize;
    //printf("\n\n\n\n  *esBuffAddr = 0x%x,  *esBuffSize=0x%x\n\n\n\n",*esBuffAddr,*esBuffSize);

}

mt_void MT_MPI_DMX_GetEsBuffAddrEx(mt_handle hChannel,
						ulong *esBuffAddr,  ulong *kerVirEsBuffAddr,  ulong *esBuffSize,
						ulong *descBuffAdrr, ulong *kerVirDescBuffAddr,  ulong *descBuffSize)
{

    mt_u32 u32ChId = DMX_CHANID(hChannel);

    //printf("MT_MPI_DMX_GetEsBuffAddr >>> u32ChId = 0x%x\n",u32ChId);
    if(esBuffAddr) *esBuffAddr = g_stChanBuf[u32ChId].u32BufUsrVirAddr;
    if(kerVirEsBuffAddr) *kerVirEsBuffAddr = g_stChanBuf[u32ChId].u32BufKerVirAddr;
    if(esBuffSize) *esBuffSize = g_stChanBuf[u32ChId].u32BufSize;

    if(descBuffAdrr) *descBuffAdrr = g_stChanDescBuf[u32ChId].u32BufUsrVirAddr;
    if(kerVirDescBuffAddr) *kerVirDescBuffAddr = g_stChanDescBuf[u32ChId].u32BufKerVirAddr;
    if(descBuffSize)  *descBuffSize = g_stChanDescBuf[u32ChId].u32BufSize;

    //printf("\n\n\n\n  *esBuffAddr = 0x%x,  *esBuffSize=0x%x\n\n\n\n",*esBuffAddr,*esBuffSize);
    return;
}

mt_void MT_MPI_DMX_GetEsBuffKerVirAddr(mt_handle hChannel, ulong *esBuffKerVirAddr,  mt_u32 *esBuffSize)
{

    mt_u32 u32ChId = DMX_CHANID(hChannel);
    //printf("MT_MPI_DMX_GetEsBuffKerVirAddr >>> u32ChId = 0x%x\n",u32ChId);
    *esBuffKerVirAddr = g_stChanBuf[u32ChId].u32BufKerVirAddr;
    *esBuffSize = g_stChanBuf[u32ChId].u32BufSize;
    //printf("\n\n\n\n  *esBuffAddr = 0x%x,  *esBuffSize=0x%x\n\n\n\n",*esBuffKerVirAddr,*esBuffSize);

}

mt_void MT_MPI_DMX_GetEsBuffPhyAddr(mt_handle hChannel, ulong *esBuffPhyAddr, mt_u32 *esBuffSize)
{
    mt_u32 u32ChId = DMX_CHANID(hChannel);
    //printf("MT_MPI_DMX_GetEsBuffPhysAddr >>> u32ChId = 0x%x\n",u32ChId);
    *esBuffPhyAddr = g_stChanBuf[u32ChId].u32BufPhyAddr;
    *esBuffSize = g_stChanBuf[u32ChId].u32BufSize;
}

mt_void MT_MPI_DMX_GetDescBuffAddr(mt_handle hChannel,
								phys_addr_t *phyDescBuffAddr,
								ulong *kerVirDescBuffAddr,
								ulong *usrVirDescBuffAddr,
								ulong *descBuffSize)
{
    mt_u32 u32ChId = DMX_CHANID(hChannel);
    if(phyDescBuffAddr) *phyDescBuffAddr = g_stChanDescBuf[u32ChId].u32BufPhyAddr;
    if(kerVirDescBuffAddr) *kerVirDescBuffAddr = g_stChanDescBuf[u32ChId].u32BufKerVirAddr;
    if(usrVirDescBuffAddr) *usrVirDescBuffAddr = g_stChanDescBuf[u32ChId].u32BufUsrVirAddr;
    if(descBuffSize)  *descBuffSize = g_stChanDescBuf[u32ChId].u32BufSize;
}

mt_u32 MT_MPI_DMX_GetAVsync(mt_handle hChannel)
{
    mt_u32 value = 0;
    mt_u32 playCh;
	if ((-1 == g_s32DmxFd) || g_dmxTrppRegristerBaseAddr == 0)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return 0;
	}

    playCh =  MT_MPI_DMX_GetAVChaneId(hChannel);
	if(playCh > 16)
	{
		return 0;
	}

    value = dmx_inl_new(g_dmxTrppRegristerBaseAddr + 0x0100 + playCh * 0x100);
    return (value & 0x100);
}
 
mt_u32 MT_MPI_DMX_GetCurrentEsBufferWritePoint(mt_handle hChannel)
{
    mt_u32 playCh;
	if ((-1 == g_s32DmxFd) || g_dmxTrppRegristerBaseAddr == 0)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return 0;
	}

    playCh =  MT_MPI_DMX_GetAVChaneId(hChannel);
	if(playCh > 16)
	{
		return 0;
	}
    return  dmx_inl_new(g_dmxTrppRegristerBaseAddr + 0x0134 + playCh * 0x100);
}

mt_void MT_MPI_DMX_SetCurrentEsBufferReadPoint(mt_handle hChannel, mt_u32 rp)
{
    mt_u32 playCh;
	if ((-1 == g_s32DmxFd) || g_dmxTrppRegristerBaseAddr == 0)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return;
	}

    playCh =  MT_MPI_DMX_GetAVChaneId(hChannel);
	if(playCh > 16)
	{
		return ;
	}
    return  dmx_outl_new(g_dmxTrppRegristerBaseAddr + 0x012c + playCh * 0x100,  rp);
}

mt_u32 MT_MPI_DMX_GetCurrentDescBufferWritePoint(mt_handle hChannel)
{
    mt_u32 playCh;
	if ((-1 == g_s32DmxFd) || g_dmxTrppRegristerBaseAddr == 0)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return 0;
	}

    playCh =  MT_MPI_DMX_GetAVChaneId(hChannel);
	if(playCh > 16)
	{
		return 0;
	}
    return  dmx_inl_new(g_dmxTrppRegristerBaseAddr + 0x0130 + playCh * 0x100);
}

mt_void MT_MPI_DMX_SetCurrentDescBufferReadPoint(mt_handle hChannel, mt_u32 rp)
{
    mt_u32 playCh;
	if ((-1 == g_s32DmxFd) || g_dmxTrppRegristerBaseAddr == 0)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return;
	}

    playCh =  MT_MPI_DMX_GetAVChaneId(hChannel);
	if(playCh > 16)
	{
		return ;
	}
    return  dmx_outl_new(g_dmxTrppRegristerBaseAddr + 0x0128 + playCh * 0x100,  rp);
}

mt_u32 MT_MPI_DMX_GetCurrentDescBufferReadPoint(mt_handle hChannel)
{
    mt_u32 playCh;
	if ((-1 == g_s32DmxFd) || g_dmxTrppRegristerBaseAddr == 0)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return 0;
	}

    playCh =  MT_MPI_DMX_GetAVChaneId(hChannel);
	if(playCh > 16)
	{
		return 0;
	}
    return  dmx_inl_new(g_dmxTrppRegristerBaseAddr + 0x0128 + playCh * 0x100);
}

mt_u32 MT_MPI_DMX_GetRegristerValue(mt_handle hChannel, mt_u32 offset)
{
    mt_u32 playCh;
	if ((-1 == g_s32DmxFd) || g_dmxTrppRegristerBaseAddr == 0)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return 0;
	}

    playCh =  MT_MPI_DMX_GetAVChaneId(hChannel);
	if(playCh > 16)
	{
		return 0;
	}

    return  dmx_inl_new(g_dmxTrppRegristerBaseAddr + offset + playCh * 0x100);
}

mt_void MT_MPI_DMX_SetRegristerValue(mt_handle hChannel, mt_u32 offset, mt_u32 value)
{
    mt_u32 playCh;
	if ((-1 == g_s32DmxFd) || g_dmxTrppRegristerBaseAddr == 0)
	{
		MT_ERR_DEMUX("Dmx not init!\n");
		return;
	}

    playCh =  MT_MPI_DMX_GetAVChaneId(hChannel);
	if(playCh > 16)
	{
		return ;
	}
    
    return  dmx_outl_new(g_dmxTrppRegristerBaseAddr + offset + playCh * 0x100,  value);
}

/***********************************************************************************
* Function:      MT_MPI_DMX_Init
* Description:   Open DEMUX device.
* Data Accessed: (Optional)
* Data Updated:  (Optional)
* Input:
* Output:
* Return:    MT_SUCCESS:                         Success
*                MT_FAILURE                             failure
* Others:
***********************************************************************************/
mt_s32 MT_MPI_DMX_Init(mt_void)
{
    int fd;

    MT_INFO_DEMUX("MT_MPI_DMX_Init ----------------------start\n");
#if  DMX_COPY_PES
    memset(MPIPesMemSize, 0, sizeof(mt_u32) * DMX_CHANNEL_CNT);
#endif /* #if DMX_COPY_PES */

    pthread_mutex_lock(&g_stDmxMutex);
    if (g_s32DmxFd == -1)
    {
       fd = open (DmxDevName, O_RDWR | O_CLOEXEC, 0);
        if (fd < 0)
        {
            MT_FATAL_DEMUX("Cannot open '%s'\n", DmxDevName);
            (void)pthread_mutex_unlock(&g_stDmxMutex);
            return MT_FAILURE;
        }

        g_s32DmxFd = fd;

        memset(g_stTsBuf, 0, sizeof(g_stTsBuf));
        memset(g_stChanBuf, 0, sizeof(g_stChanBuf));
        memset(g_stChanDescBuf, 0, sizeof(g_stChanDescBuf));
        memset(g_stChanPesBuf, 0, sizeof(g_stChanPesBuf));
        memset(g_RecMmzBuf, 0, sizeof(g_RecMmzBuf));
        MT_INFO_DEMUX("MT_MPI_DMX_Init ------------g_s32DmxFd=%d----------1\n", g_s32DmxFd);

        #ifdef CONFIG_MT_CHIP_ARIA
        g_dmxTrppRegristerBaseAddr = (ulong)mt_mmap(0xffc60000, DMX_TRPP_REGRISTER_SIZE);
        #elif defined(CONFIG_MT_CHIP_SYMPHONY4)
        mt_sys_map_register(SYMPHONY_IO_PA(0xbf260000), DMX_TRPP_REGRISTER_SIZE, &g_dmxTrppRegristerBaseAddr);
        #else
        g_dmxTrppRegristerBaseAddr = (ulong)mt_mmap(SYMPHONY_IO_PA(0xbf260000), DMX_TRPP_REGRISTER_SIZE);
        #endif
        if ((void *)g_dmxTrppRegristerBaseAddr == NULL)
        {
            perror(" map failed\n");
            pthread_mutex_unlock(&g_stDmxMutex);
            return MT_FAILURE;
        }

        pthread_mutex_unlock(&g_stDmxMutex);
        return MT_SUCCESS;
    }
    else
    {
        pthread_mutex_unlock(&g_stDmxMutex);
        return MT_SUCCESS; //again open it, so return success
    }
}

/***********************************************************************************
* Function:      MT_MPI_DMX_DeInit
* Description:   Close DEMUX device.
* Data Accessed: (Optional)
* Data Updated:  (Optional)
* Input:
* Output:
* Return:        MT_SUCCESS:                         Success
*                MT_FAILURE                          failure
* Others:
***********************************************************************************/
mt_s32 MT_MPI_DMX_DeInit(mt_void)
{
    mt_s32 ret;
    mt_u32 i;

    pthread_mutex_lock(&g_stDmxMutex);
    if (g_s32DmxFd != -1)
    {
        for (i = 0; i < DMX_CNT; i++)
        {
            if (g_RecMmzBuf[i].u32BufPhyAddr)
            {
                ret = MT_MPI_DMX_StopRecChn(DMX_RECHANDLE(i));
                if (MT_SUCCESS != ret)
                {
                    pthread_mutex_unlock(&g_stDmxMutex);

                    MT_ERR_DEMUX("stop rec failed 0x%x\n", ret);

                    return ret;
                }

                ret = MT_MPI_DMX_DestroyRecChn(DMX_RECHANDLE(i));
                if (MT_SUCCESS != ret)
                {
                    pthread_mutex_unlock(&g_stDmxMutex);

                    MT_ERR_DEMUX("destroy rec failed 0x%x\n", ret);

                    return ret;
                }
            }
        }

        for (i = 0; i < DMX_RAMPORT_CNT; i++)
        {
            if (g_stTsBuf[i].u32BufUsrVirAddr)
            {
                ret = MPIDMXDestroyTSBuffer(i);
                if (MT_SUCCESS != ret)
                {
                    MT_ERR_DEMUX("TS buffer destroy failed:PortId=%d\n", i);
                    (void)pthread_mutex_unlock(&g_stDmxMutex);
                    return MT_FAILURE;
                }
            }
        }

        for (i = 0; i < DMX_CHANNEL_CNT; i++)
        {
            if (0 == g_stChanBuf[i].u32BufPhyAddr)
            {
                continue;
            }

            ret = MPIDMXDestroyChannel(i);
            if (MT_SUCCESS != ret)
            {
                (void)pthread_mutex_unlock(&g_stDmxMutex);

                MT_ERR_DEMUX("Channel delete failed 0x%x\n", ret);

                return ret;
            }
        }

        ret = close(g_s32DmxFd);
        if (ret != 0)
        {
            MT_FATAL_DEMUX("Cannot close '%s'\n", DmxDevName);
            (void)pthread_mutex_unlock(&g_stDmxMutex);
            return MT_FAILURE;
        }

        g_s32DmxFd = -1;

        if(g_dmxTrppRegristerBaseAddr  != 0)
        {
#if defined(CONFIG_MT_CHIP_SYMPHONY4)
		  if (MT_SUCCESS != mt_sys_unmap_register((mt_void*)g_dmxTrppRegristerBaseAddr))
#else
          if (MT_SUCCESS != mt_munmap((mt_void*)g_dmxTrppRegristerBaseAddr))
#endif
          {
              pthread_mutex_unlock(&g_stDmxMutex);

              MT_ERR_DEMUX("g_dmxTrppRegristerBaseAddr unmap failed\n");

              return MT_ERR_DMX_MUNMAP_FAILED;
          }
          g_dmxTrppRegristerBaseAddr = 0;
        }
        (void)pthread_mutex_unlock(&g_stDmxMutex);
        return MT_SUCCESS;
    }
    else
    {
        (void)pthread_mutex_unlock(&g_stDmxMutex);
        return MT_SUCCESS;
    }
}

mt_s32 MT_MPI_DMX_GetCapability(MT_UNF_DMX_CAPABILITY_S *pstCap)
{
    mt_s32                  ret;
    MT_UNF_DMX_CAPABILITY_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstCap);

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_GET_CAPABILITY, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        pstCap->u32IFPortNum 	= Param.u32IFPortNum;
        pstCap->u32TSIPortNum 	= Param.u32TSIPortNum;
        pstCap->u32TSOPortNum 	= Param.u32TSOPortNum;
        pstCap->u32RamPortNum   = Param.u32RamPortNum;
        pstCap->u32DmxNum       = Param.u32DmxNum;
        pstCap->u32ChannelNum   = Param.u32ChannelNum;
        pstCap->u32AVChannelNum = Param.u32AVChannelNum;
        pstCap->u32FilterNum    = Param.u32FilterNum;
        pstCap->u32KeyNum       = Param.u32KeyNum;
        pstCap->u32RecChnNum    = Param.u32RecChnNum;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetTSPortAttr(MT_UNF_DMX_PORT_E enPortId, MT_UNF_DMX_PORT_ATTR_S *pstAttr)
{
    mt_s32              ret;
    DMX_Port_GetAttr_S  Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstAttr);
    //printf("============================%s  %d\n",__FILE__,__LINE__);
    ret = DMX_MPI_PortGetTypeAndID(enPortId, &Param.PortMode, &Param.PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_PORT_GET_ATTR, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        memcpy(pstAttr, &Param.PortAttr, sizeof(MT_UNF_DMX_PORT_ATTR_S));
    }

    return ret;
}

mt_s32 MT_MPI_DMX_SetTSPortAttr(MT_UNF_DMX_PORT_E enPortId, const MT_UNF_DMX_PORT_ATTR_S *pstAttr)
{
    mt_s32              ret;
    DMX_Port_SetAttr_S  Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstAttr);
    MT_INFO_DEMUX("============================\n");
    ret = DMX_MPI_PortGetTypeAndID(enPortId, &Param.PortMode, &Param.PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    memcpy(&Param.PortAttr, pstAttr, sizeof(Param.PortAttr));

    return ioctl(g_s32DmxFd, CMD_DEMUX_PORT_SET_ATTR, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_GetTSOPortAttr(MT_UNF_DMX_TSO_PORT_E enPortId, MT_UNF_DMX_TSO_PORT_ATTR_S *pstAttr)
{
    mt_s32              ret;
    DMX_TSO_Port_Attr_S  Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstAttr);

    Param.PortId = (mt_u32)enPortId;


    ret = ioctl(g_s32DmxFd, CMD_DEMUX_TSO_PORT_GET_ATTR, (ulong)&Param);
	if(ret == MT_SUCCESS)
	{
		memcpy(pstAttr,&Param.PortAttr, sizeof(Param.PortAttr));
	}
    return ret;
}

mt_s32 MT_MPI_DMX_SetTSOPortAttr(MT_UNF_DMX_TSO_PORT_E enPortId, const MT_UNF_DMX_TSO_PORT_ATTR_S *pstAttr)
{
    DMX_TSO_Port_Attr_S  Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstAttr);

    Param.PortId = (mt_u32)enPortId;
    memcpy(&Param.PortAttr, pstAttr, sizeof(Param.PortAttr));

    return ioctl(g_s32DmxFd, CMD_DEMUX_TSO_PORT_SET_ATTR, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_GetDmxTagAttr(mt_u32 u32DmxId, MT_UNF_DMX_TAG_ATTR_S *pstAttr)
{
    mt_s32  ret;
    DMX_Tag_GetAttr_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstAttr);

    Param.DmxId = (mt_u32)u32DmxId;
    memcpy(&Param.TagAttr, pstAttr, sizeof(MT_UNF_DMX_TAG_ATTR_S));

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_DMX_GET_TAG_ATTR, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        memcpy(pstAttr, &Param.TagAttr, sizeof(MT_UNF_DMX_TAG_ATTR_S));
    }

    return ret;
}

mt_s32 MT_MPI_DMX_SetDmxTagAttr(mt_u32 u32DmxId, const MT_UNF_DMX_TAG_ATTR_S *pstAttr)
{
    DMX_Tag_GetAttr_S  Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstAttr);

    Param.DmxId = u32DmxId;
    memcpy(&Param.TagAttr, pstAttr, sizeof(MT_UNF_DMX_TAG_ATTR_S));

    return ioctl(g_s32DmxFd, CMD_DEMUX_DMX_SET_TAG_ATTR, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_AttachTSPort(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E enPortId)
{
    mt_s32              ret;
    DMX_Port_Attach_S   Param;
    mt_sys_version_s stSysChipInfo;

    memset(&stSysChipInfo, 0, sizeof(stSysChipInfo));
    mt_sys_get_version(&stSysChipInfo);

    MT_INFO_DEMUX("MT_MPI_DMX_AttachTSPort ====stSysChipInfo.enChipVersio=0x%x\n",stSysChipInfo.enChipVersion);

    MPIDmxCheckDeviceFd();

#ifdef CONFIG_MT_FPGA
    if((MT_CHIP_SYMPHONY4_A0  <= stSysChipInfo.enChipVersion)  &&  (MT_CHIP_SYMPHONY4_A1  >= stSysChipInfo.enChipVersion))
    {
      if(enPortId == MT_UNF_DMX_PORT_TSI_0)    enPortId = MT_UNF_DMX_PORT_TSI_1;
      if(enPortId == MT_UNF_DMX_PORT_TSI_1)    enPortId = MT_UNF_DMX_PORT_TSI_0;

      if(enPortId >= MT_UNF_DMX_PORT_TSI_0  && 	enPortId <= MT_UNF_DMX_PORT_TSI_3){
        Param.PortId = enPortId - MT_UNF_DMX_PORT_TSI_0;
        Param.PortMode = DMX_PORT_MODE_TUNER;
      }else{
        Param.PortId = enPortId - MT_UNF_DMX_PORT_RAM_0;
        Param.PortMode = DMX_PORT_MODE_RAM;
      }
      Param.DmxId = u32DmxId;
      ret =  ioctl(g_s32DmxFd, CMD_DEMUX_PORT_ATTACH, (ulong)&Param);
    }
   else
#endif
   {
	    ret = DMX_MPI_PortGetTypeAndID(enPortId, &Param.PortMode, &Param.PortId);
	    if (MT_SUCCESS != ret)
	    {
	        return ret;
	    }

	    Param.DmxId = u32DmxId;

	    ret =  ioctl(g_s32DmxFd, CMD_DEMUX_PORT_ATTACH, (ulong)&Param);
	    MT_INFO_DEMUX("MT_MPI_DMX_AttachTSPort -------------------ret=%d\n",ret);
   }
    return ret;
}

mt_s32 MT_MPI_DMX_DetachTSPort(mt_u32 u32DmxId)
{
    MPIDmxCheckDeviceFd();

    return ioctl(g_s32DmxFd, CMD_DEMUX_PORT_DETACH, (ulong)&u32DmxId);
}

mt_s32 MT_MPI_DMX_GetTSPortId(mt_u32 u32DmxId, MT_UNF_DMX_PORT_E *penPortId)
{
    mt_s32              ret;
    DMX_Port_GetId_S    Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(penPortId);

    Param.DmxId = u32DmxId;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_PORT_GETID, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *penPortId = (MT_UNF_DMX_PORT_E)Param.PortId;
        if ((DMX_PORT_MODE_TUNER  == Param.PortMode) && (*penPortId  >= DMX_IFPORT_CNT))
        {
            /*pay attention: if DMX_IFPORT_CNT == 0,then  (Param.PortId == 0)   corresponding with (MT_UNF_DMX_PORT_TSI_0==0x20)*/
            *penPortId = (MT_UNF_DMX_PORT_E)(MT_UNF_DMX_PORT_TSI_0 + Param.PortId - DMX_IFPORT_CNT);
        }
        if (DMX_PORT_MODE_RAM == Param.PortMode)
        {
            *penPortId = (MT_UNF_DMX_PORT_E)(MT_UNF_DMX_PORT_RAM_0 + Param.PortId);
        }
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetTSPortPacketNum(MT_UNF_DMX_PORT_E enPortId, MT_UNF_DMX_PORT_PACKETNUM_S *sPortStat)
{
    mt_s32              ret;
    DMX_PortPacketNum_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(sPortStat);
    MT_INFO_DEMUX("============================\n");
    ret = DMX_MPI_PortGetTypeAndID(enPortId, &Param.PortMode, &Param.PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_PORT_GETPACKETNUM, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        sPortStat->u32TsPackCnt     = Param.TsPackCnt;
        sPortStat->u32ErrTsPackCnt  = Param.ErrTsPackCnt;
    }

    return ret;
}

/* multi-times request in process, return success directly
    the first time use it in this process, need to map user mode address.
 */
mt_s32 MT_MPI_DMX_CreateTSBuffer(MT_UNF_DMX_PORT_E enPortId, mt_u32 u32TsBufSize, mt_handle *phTsBuffer)
{
    mt_s32          ret;
    mt_u32          PortId;
    DMX_PORT_MODE_E PortMode;
    DMX_TsBufInit_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(phTsBuffer);
    MT_INFO_DEMUX("============================\n");
    ret = DMX_MPI_PortGetTypeAndID(enPortId, &PortMode, &PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    if (DMX_PORT_MODE_TUNER == PortMode)
    {
        MT_ERR_DEMUX("Invalid port mode:%d!\n",PortMode);
        return MT_ERR_DMX_NOT_SUPPORT;
    }

    Param.PortId    = PortId;
    Param.BufSize   = u32TsBufSize;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_INIT, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        Param.TsBuf.u32BufUsrVirAddr = (ulong)mt_mmap(Param.TsBuf.u32BufPhyAddr, Param.BufSize);
        if (0 == Param.TsBuf.u32BufUsrVirAddr)
        {
            MT_ERR_DEMUX("Ts buffer mmap error: PortId=%d\n", enPortId);

            ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_DEINIT, (ulong)&PortId);

            return MT_ERR_DMX_MMAP_FAILED;
        }

        memcpy(&g_stTsBuf[PortId], &Param.TsBuf, sizeof(DMX_MMZ_BUF_S));

        *phTsBuffer = DMX_PORTID2BUFFERHANDLE(PortId);
    }

    return ret;
}

mt_s32 MT_MPI_DMX_DestroyTSBuffer(mt_handle hTsBuffer)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);

    return MPIDMXDestroyTSBuffer(DMX_BUFFERHANDLE2PORTID(hTsBuffer));
}

mt_s32 MT_MPI_DMX_GetTSBuffer(mt_handle hTsBuffer, mt_u32 u32ReqLen,
                              MT_UNF_STREAM_BUF_S *pstData, phys_addr_t *pu32PhyAddr, mt_u32 u32TimeOutMs)
{
    mt_s32          ret;
    DMX_TsBufGet_S  Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);
    MPIDmxCheckPointer(pstData);
    MPIDmxCheckPointer(pu32PhyAddr);

    Param.PortId    = DMX_BUFFERHANDLE2PORTID(hTsBuffer);
    Param.ReqLen    = u32ReqLen;
    Param.TimeoutMs = u32TimeOutMs;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_GET, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        pstData->u32Size = Param.Data.BufLen;
        pstData->pu8Data = (mt_u8*)(g_stTsBuf[Param.PortId].u32BufUsrVirAddr
                                + (ulong)(Param.Data.BufPhyAddr - g_stTsBuf[Param.PortId].u32BufPhyAddr));

        *pu32PhyAddr = Param.Data.BufPhyAddr;
    }

    return ret;
}

/*u32ValidDataLen is the valid length from u32StartPos */
mt_s32 MT_MPI_DMX_PutTSBuffer(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos)
{
    DMX_TsBufPut_S  Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);

    Param.PortId        = DMX_BUFFERHANDLE2PORTID(hTsBuffer);
    Param.ValidDataLen  = u32ValidDataLen;
    Param.StartPos      = u32StartPos;
    Param.DateType     = DMX_TS_188,
    Param.Pts           = 0;
    Param.Pid           = 0x1fff;
    Param.SpecifiedDataAddr = 0;
    return ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_PUT, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_PutTSBuffer_V1(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos, dmx_ts_data_t ts_type, mt_u32 pid)
{
    DMX_TsBufPut_S  Param;
    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);
    Param.PortId        = DMX_BUFFERHANDLE2PORTID(hTsBuffer);
    Param.ValidDataLen  = u32ValidDataLen;
    Param.StartPos      = u32StartPos;
    Param.DateType     = ts_type,
    Param.Pts           = 0;
    Param.Pid           = pid;
    Param.SpecifiedDataAddr = 0;
    return ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_PUT, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_PutTSBufferEx(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos, phys_addr_t u32SpecifiedDataPhyAddr)
{
    DMX_TsBufPut_S  Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);

    Param.PortId        = DMX_BUFFERHANDLE2PORTID(hTsBuffer);
    Param.ValidDataLen  = u32ValidDataLen;
    Param.StartPos      = u32StartPos;
    Param.DateType     = DMX_TS_188,
    Param.Pts           = 0;
    Param.Pid           = 0x1fff;
    Param.SpecifiedDataAddr = u32SpecifiedDataPhyAddr;
    return ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_PUT, (ulong)&Param);
}


mt_s32 MT_MPI_DMX_PutTSBufferForEsPes(mt_handle hTsBuffer, mt_u32 u32ValidDataLen, mt_u32 u32StartPos, mt_u32 videoPid, dmx_ts_data_t dateType, mt_u32 vpts)
{
    DMX_TsBufPut_S  Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);

    Param.PortId        = DMX_BUFFERHANDLE2PORTID(hTsBuffer);
    Param.ValidDataLen  = u32ValidDataLen;
    Param.StartPos      = u32StartPos;
    Param.DateType     = dateType,
    Param.Pts           = (vpts << 1);
    Param.Pid           = (videoPid & 0xffff);
    Param.SpecifiedDataAddr = 0;
    return ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_PUT, (ulong)&Param);
}


mt_s32 MT_MPI_DMX_ResetTSBuffer(mt_handle hTsBuffer)
{
    mt_u32 PortId = DMX_BUFFERHANDLE2PORTID(hTsBuffer);

    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);

    return ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_RESET, (ulong)&PortId);
}

/* get the status without limiting for muliti-process */
mt_s32 MT_MPI_DMX_GetTSBufferStatus(mt_handle hTsBuffer, MT_UNF_DMX_TSBUF_STATUS_S *pStatus)
{
    mt_s32              ret;
    DMX_TsBufStaGet_S   Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);
    MPIDmxCheckPointer(pStatus);

    Param.PortId = DMX_BUFFERHANDLE2PORTID(hTsBuffer);

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_GET_STATUS, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        memcpy(pStatus, &Param.Status, sizeof(MT_UNF_DMX_TSBUF_STATUS_S));
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetTSBufferPortId(mt_handle hTsBuffer, MT_UNF_DMX_PORT_E *penPortId)
{
    mt_u32 PortId = DMX_BUFFERHANDLE2PORTID(hTsBuffer);

    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);
    MPIDmxCheckPointer(penPortId);

    if (0 == g_stTsBuf[PortId].u32BufPhyAddr)
    {
        MT_ERR_DEMUX("TS buffer handle invalid, buffer not created:PortId=%d\n", PortId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    *penPortId = (MT_UNF_DMX_PORT_E)(MT_UNF_DMX_PORT_RAM_0 + PortId);

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DMX_GetTSBufferHandle(MT_UNF_DMX_PORT_E enPortId, mt_handle *phTsBuffer)
{
    mt_s32          ret;
    mt_u32          PortId;
    DMX_PORT_MODE_E PortMode;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(phTsBuffer);
    MT_INFO_DEMUX("============================\n");
    ret = DMX_MPI_PortGetTypeAndID(enPortId, &PortMode, &PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

    if (DMX_PORT_MODE_TUNER == PortMode)
    {
        MT_ERR_DEMUX("The Port %u not support TS buffer\n", enPortId);
        return MT_ERR_DMX_INVALID_PARA;
    }

    if (0 == g_stTsBuf[PortId].u32BufPhyAddr)
    {
        MT_ERR_DEMUX("TS Buffer not created\n");
        return MT_ERR_DMX_INVALID_PARA;
    }

    *phTsBuffer = DMX_PORTID2BUFFERHANDLE(PortId);

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DMX_GetTSBufferFullCare(mt_handle hTsBuffer, MT_UNF_DMX_AV_CHN_FULL_CARE_S *p_av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S *p_rec_care)
{
	mt_s32          ret;
    DMX_TsBufFullCare_S  Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);

    Param.PortId        = DMX_BUFFERHANDLE2PORTID(hTsBuffer);
    
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_GET_FULL_CARE, (ulong)&Param);
	if (MT_SUCCESS == ret)
	{
		p_av_care->all = Param.AvFullCare.all;
		p_rec_care->all = Param.RecFullCare.all;
		printf("[%s %d]port_id=%d, AvFullCare.all=0x%x, RecFullCare.all=0x%x\n", __FUNCTION__, __LINE__, Param.PortId, p_av_care->all, p_rec_care->all);
	}

	return ret;
}

mt_s32 MT_MPI_DMX_SetTSBufferFullCare(mt_handle hTsBuffer, MT_UNF_DMX_AV_CHN_FULL_CARE_S av_care, MT_UNF_DMX_REC_CHN_FULL_CARE_S rec_care)
{
    DMX_TsBufFullCare_S  Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_BUFFERHANDLE(hTsBuffer);

    Param.PortId        = DMX_BUFFERHANDLE2PORTID(hTsBuffer);
    Param.AvFullCare.all  = av_care.all;
    Param.RecFullCare.all = rec_care.all;
	printf("[%s %d]port_id=%d, AvFullCare.all=0x%x, RecFullCare.all=0x%x\n", __FUNCTION__, __LINE__, Param.PortId, Param.AvFullCare.all, Param.RecFullCare.all);
    return ioctl(g_s32DmxFd, CMD_DEMUX_TS_BUFFER_SET_FULL_CARE, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_GetPortMode(mt_u32 u32DmxId, MT_UNF_DMX_PORT_MODE_E *penPortMod)
{
    mt_s32                  ret;
    MT_UNF_DMX_PORT_E       PortId;
    MT_UNF_DMX_PORT_ATTR_S  PortAttr;

    if (-1 == g_s32DmxFd)
    {
        MT_WARN_DEMUX("Dmx not init!\n");
        return MT_ERR_DMX_NOT_INIT;
    }

    MPIDmxCheckPointer(penPortMod);

    ret = MT_MPI_DMX_GetTSPortId(u32DmxId, &PortId);
    if (MT_SUCCESS == ret)
    {
        ret = MT_MPI_DMX_GetTSPortAttr(PortId, &PortAttr);
        if (MT_SUCCESS == ret)
        {
            *penPortMod = PortAttr.enPortMod;
        }
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetChannelDefaultAttr(MT_UNF_DMX_CHAN_ATTR_S *pstChAttr)
{
    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstChAttr);

    pstChAttr->u32BufSize       = DMX_DEFAULT_CHANBUF_SIZE;
    pstChAttr->enCRCMode        = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    pstChAttr->enChannelType    = MT_UNF_DMX_CHAN_TYPE_SEC;
    pstChAttr->enOutputMode     = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;

    return MT_SUCCESS;
}

mt_s32 MT_MPI_DMX_CreateChannel(mt_u32 u32DmxId, const MT_UNF_DMX_CHAN_ATTR_S *pstChAttr,
                                mt_handle *phChannel)
{
    mt_s32 ret = MT_SUCCESS;
    DMX_ChanNew_S Param;
    mt_u32 u32ChId;
    mt_u32 size = 0;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstChAttr);
    MPIDmxCheckPointer(phChannel);

    memset(&Param, 0, sizeof(DMX_ChanNew_S));
    Param.u32DemuxId = u32DmxId;
    memcpy(&Param.stChAttr, pstChAttr, sizeof(MT_UNF_DMX_CHAN_ATTR_S));
    MT_INFO_DEMUX("size=%x\n",pstChAttr->u32BufSize);

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_NEW, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        *phChannel = Param.hChannel;
        u32ChId = DMX_CHANID(Param.hChannel);

        switch (pstChAttr->enChannelType)
        {
            case MT_UNF_DMX_CHAN_TYPE_SEC:
            case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
            {
                size = Param.stChBuf.u32BufSize + DMX_SYMPHONY_SEC_DESC_BUF_SIZE + DMX_SYMPHONY_SEC_ROLLBACK_BUF_SIZE;
                g_stChanBuf[u32ChId].u32BufUsrVirAddr = (ulong)mt_mmap(Param.stChBuf.u32BufPhyAddr, size);
    
                break;
            }
    
            case MT_UNF_DMX_CHAN_TYPE_HW_PES:
            {
                size = Param.stChBuf.u32BufSize + DMX_SYMPHONY_PES_ROLLBACK_BUF_SIZE;
                g_stChanBuf[u32ChId].u32BufUsrVirAddr = (ulong)mt_mmap(Param.stChBuf.u32BufPhyAddr, size); 
    
                break;
            }
    
            case MT_UNF_DMX_CHAN_TYPE_POST:
            case MT_UNF_DMX_CHAN_TYPE_PES:
            case MT_UNF_DMX_CHAN_TYPE_AUD:
            case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
            case MT_UNF_DMX_CHAN_TYPE_VID:
 	    case MT_UNF_DMX_CHAN_TYPE_DSS:
            {
                size = Param.stChBuf.u32BufSize;
                g_stChanBuf[u32ChId].u32BufUsrVirAddr = (ulong)mt_mmap(Param.stChBuf.u32BufPhyAddr, size);
    
                if (Param.stChDescBuf.u32BufPhyAddr != 0)
                {
                    size = Param.stChDescBuf.u32BufSize;
                    g_stChanDescBuf[u32ChId].u32BufUsrVirAddr = (ulong)mt_mmap(Param.stChDescBuf.u32BufPhyAddr, size);
                }
    
                break;
            }
    
            default:
            {
                MT_INFO_DEMUX("Create type:%d, but need not mmap buf\n", pstChAttr->enChannelType);
                return MT_ERR_DMX_INVALID_PARA;
            }
        }
    
        if (0 == g_stChanBuf[u32ChId].u32BufUsrVirAddr)
        {
            MT_FATAL_DEMUX("Channel buffer mmap error: ChanId=%d\n", u32ChId);
        
            if (MT_SUCCESS != ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_DEL, (ulong)phChannel))
            {
                MT_ERR_DEMUX("delete channel failed:ChId=%d.\n", u32ChId);
            }
            
            return MT_ERR_DMX_MMAP_FAILED;
        }
    
    
        g_stChanBuf[u32ChId].u32BufPhyAddr      = Param.stChBuf.u32BufPhyAddr;
        g_stChanBuf[u32ChId].u32BufKerVirAddr   = Param.stChBuf.u32BufKerVirAddr;
        g_stChanBuf[u32ChId].u32BufSize         = Param.stChBuf.u32BufSize;
    
        g_stChanDescBuf[u32ChId].u32BufPhyAddr      = Param.stChDescBuf.u32BufPhyAddr;
        g_stChanDescBuf[u32ChId].u32BufKerVirAddr   = Param.stChDescBuf.u32BufKerVirAddr;
        g_stChanDescBuf[u32ChId].u32BufSize         = Param.stChDescBuf.u32BufSize;
    
        MT_INFO_DEMUX("UsrVirAddr = %#x, PhyAddr = %#x , KerVirAddr = %#x, BufSize = %#x\n",
            g_stChanBuf[u32ChId].u32BufUsrVirAddr,u32ChId,g_stChanBuf[u32ChId].u32BufPhyAddr,
            g_stChanBuf[u32ChId].u32BufKerVirAddr,g_stChanBuf[u32ChId].u32BufSize);
        
    }

    return ret;
}

mt_s32 MT_MPI_DMX_DestroyChannel(mt_handle hChannel)
{
    mt_u32 u32ChId;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    u32ChId = DMX_CHANID(hChannel);

    return MPIDMXDestroyChannel(u32ChId);
}

mt_s32 MT_MPI_DMX_GetChannelAttr(mt_handle hChannel, MT_UNF_DMX_CHAN_ATTR_S *pstChAttr)
{
    mt_s32 ret;
    DMX_GetChan_Attr_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    MPIDmxCheckPointer(pstChAttr);

	memset(&Param, 0, sizeof(DMX_GetChan_Attr_S));
    Param.hChannel = hChannel;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_ATTR_GET, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        memcpy(pstChAttr, &Param.stChAttr, sizeof(MT_UNF_DMX_CHAN_ATTR_S));
    }

    return ret;
}

mt_s32 MT_MPI_DMX_SetChannelAttr(mt_handle hChannel, const MT_UNF_DMX_CHAN_ATTR_S *pstChAttr)
{
    DMX_SetChan_Attr_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    MPIDmxCheckPointer(pstChAttr);

    Param.hChannel = hChannel;
    memcpy(&Param.stChAttr, pstChAttr, sizeof(MT_UNF_DMX_CHAN_ATTR_S));

    /*set disable CRC in non-section mode */
    if ((MT_UNF_DMX_CHAN_TYPE_PES == Param.stChAttr.enChannelType)
        || (MT_UNF_DMX_CHAN_TYPE_POST == Param.stChAttr.enChannelType)
        || (MT_UNF_DMX_CHAN_TYPE_VID == Param.stChAttr.enChannelType)
        || (MT_UNF_DMX_CHAN_TYPE_DSS == Param.stChAttr.enChannelType)
        || (MT_UNF_DMX_CHAN_TYPE_AUD == Param.stChAttr.enChannelType)
        || (MT_UNF_DMX_CHAN_TYPE_AUD_AD == Param.stChAttr.enChannelType))
    {
        Param.stChAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
    }

    return ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_ATTR_SET, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_SetChannelPID(mt_handle hChannel, mt_u32 u32Pid)
{
    DMX_ChanPIDSet_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    Param.hChannel = hChannel;
    Param.u32Pid = u32Pid;
    return ioctl(g_s32DmxFd, CMD_DEMUX_PID_SET, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_GetChannelPID(mt_handle hChannel, mt_u32 *pu32Pid)
{
    mt_s32 ret;
    DMX_ChanPIDGet_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    MPIDmxCheckPointer(pu32Pid);

    Param.hChannel = hChannel;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_PID_GET, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        *pu32Pid = Param.u32Pid;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_OpenChannel(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    return ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_OPEN, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_CloseChannel(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    return ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_CLOSE, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_GetChannelStatus(mt_handle hChannel, MT_UNF_DMX_CHAN_STATUS_S *pstStatus)
{
    mt_s32 ret;
    DMX_ChanStatusGet_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    MPIDmxCheckPointer(pstStatus);

    Param.hChannel = hChannel;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_GET_CHAN_STATUS, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        memcpy(pstStatus, &Param.stStatus, sizeof(MT_UNF_DMX_CHAN_STATUS_S));
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetChannelHandle(mt_u32 u32DmxId, mt_u32 u32Pid, mt_handle *phChannel)
{
    mt_s32 ret;
    DMX_ChannelIdGet_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(phChannel);

    Param.u32DmxId  = u32DmxId;
    Param.u32Pid    = u32Pid;
    Param.enChannelType    = MT_UNF_DMX_CHAN_TYPE_BUTT;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_CHANID_GET, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *phChannel = Param.hChannel;
    }

    return ret;
}
mt_s32 MT_MPI_DMX_GetChannelHandleByPidType(mt_u32 u32DmxId , mt_u32 u32Pid, MT_UNF_DMX_CHAN_TYPE_E enChannelType, mt_handle *phChannel)
{
    mt_s32 ret;
    DMX_ChannelIdGet_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(phChannel);

    Param.u32DmxId  = u32DmxId;
    Param.u32Pid    = u32Pid;
    Param.enChannelType    = enChannelType;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_CHANID_GET, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *phChannel = Param.hChannel;
    }

    return ret;
}

/*
**  for Advca get audio & video channel handle
*/
mt_s32 MT_MPI_DMX_GetAVChannelHandle(mt_u32 u32DmxId, mt_handle *videoChannel, mt_handle *audioChannel)
{
    mt_s32 ret;
    DMX_AVChannelIdGet_S Param = {0};

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(videoChannel);
    MPIDmxCheckPointer(audioChannel);

    Param.u32DmxId  = u32DmxId;
    //Param.u32Pid    = u32Pid;
    //Param.vidChannel = videoChannel;
    //Param.audChannel = audioChannel;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_AVCHANID_GET, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *videoChannel = Param.vidChannel;
        *audioChannel = Param.audChannel;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetAVChaneId(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    return ioctl(g_s32DmxFd, CMD_DEMUX_GET_CHAN_AVID, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_GetFreeChannelCount(mt_u32 u32DmxId, mt_u32 *pu32FreeCount)
{
    mt_s32 ret;
    DMX_FreeChanGet_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pu32FreeCount);

    Param.u32DmxId = u32DmxId;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_FREECHAN_GET, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *pu32FreeCount = Param.u32FreeCount;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetScrambledFlag(mt_handle hChannel, MT_UNF_DMX_SCRAMBLED_FLAG_E *penScrambleFlag)
{
    mt_s32 ret;
    DMX_ScrambledFlagGet_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    MPIDmxCheckPointer(penScrambleFlag);

    Param.hChannel = hChannel;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_SCRAMBLEFLAG_GET, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        *penScrambleFlag = Param.enScrambleFlag;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_SetChannelEosFlag(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    return ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_SET_EOS_FLAG, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_GetChannelTsCount(mt_handle hChannel, mt_u32 *pu32TsCount)
{
    mt_s32 ret;
    DMX_ChanChanTsCnt_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    MPIDmxCheckPointer(pu32TsCount);

    Param.hChannel = hChannel;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_GET_CHAN_TSCNT, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        *pu32TsCount = Param.u32ChanTsCnt;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_CreateFilter(mt_u32 u32DmxId, const MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr, mt_handle *phFilter)
{
    mt_s32          ret;
    DMX_NewFilter_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstFilterAttr);
    MPIDmxCheckPointer(phFilter);

    Param.DmxId = u32DmxId;
    memcpy(&Param.FilterAttr, pstFilterAttr, sizeof(MT_UNF_DMX_FILTER_ATTR_S));

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_FLT_NEW, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *phFilter = Param.Filter;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_DestroyFilter(mt_handle hFilter)
{
    MPIDmxCheckDeviceFd();

    return ioctl(g_s32DmxFd, CMD_DEMUX_FLT_DEL, (ulong)&hFilter);
}

mt_s32 MT_MPI_DMX_DeleteAllFilter(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();

    return ioctl(g_s32DmxFd, CMD_DEMUX_FLT_DELALL, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_SetFilterAttr(mt_handle hFilter, const MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr)
{
    DMX_FilterSet_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstFilterAttr);

    Param.Filter = hFilter;
    memcpy(&Param.FilterAttr, pstFilterAttr, sizeof(MT_UNF_DMX_FILTER_ATTR_S));

    return ioctl(g_s32DmxFd, CMD_DEMUX_FLT_SET, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_GetFilterAttr(mt_handle hFilter, MT_UNF_DMX_FILTER_ATTR_S *pstFilterAttr)
{
    mt_s32          ret;
    DMX_FilterGet_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstFilterAttr);

    Param.Filter = hFilter;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_FLT_GET, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        memcpy(pstFilterAttr, &Param.FilterAttr, sizeof(MT_UNF_DMX_FILTER_ATTR_S));
    }

    return ret;
}

mt_s32 MT_MPI_DMX_AttachFilter(mt_handle hFilter, mt_handle hChannel)
{
    DMX_FilterAttach_S Param;

    MPIDmxCheckDeviceFd();

    Param.Filter    = hFilter;
    Param.Channel   = hChannel;

    return ioctl(g_s32DmxFd, CMD_DEMUX_FLT_ATTACH, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_DetachFilter(mt_handle hFilter, mt_handle hChannel)
{
    DMX_FilterDetach_S Param;

    MPIDmxCheckDeviceFd();

    Param.Filter    = hFilter;
    Param.Channel   = hChannel;

    return ioctl(g_s32DmxFd, CMD_DEMUX_FLT_DETACH, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_GetFilterChannelHandle(mt_handle hFilter, mt_handle *phChannel)
{
    mt_s32                      ret;
    DMX_FilterChannelIDGet_S    Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(phChannel);

    Param.Filter = hFilter;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_FLT_CHANID_GET, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *phChannel = Param.Channel;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetFreeFilterCount(mt_u32 u32DmxId, mt_u32 *pu32FreeCount)
{
    mt_s32              ret;
    DMX_FreeFilterGet_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pu32FreeCount);

    Param.DmxId = u32DmxId;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_FREEFLT_GET, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *pu32FreeCount = Param.FreeCount;
    }

    return ret;
}

mt_s32  MT_MPI_DMX_CheckDataHandle(mt_handle hChannel, mt_u32 u32TimeOutMs)
{
    mt_s32 ret = MT_SUCCESS;
    DMX_CheckDataFlag_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    Param.hChannel = hChannel;
    Param.u32TimeOutMs = u32TimeOutMs;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_CHECK_DATA_FLAG, (ulong)&Param);

    return ret;
}


/* return the data case of all the channel, excepted audio and video channel, the number of handle will not more than what user want */
mt_s32  MT_MPI_DMX_GetDataHandle(mt_handle *phChannel, mt_u32 *pu32ChNum,
                                 mt_u32 u32TimeOutMs)
{
    mt_s32 ret;
    DMX_GetDataFlag_S Param;
    mt_u32 ChNumGet = 0;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(phChannel);
    MPIDmxCheckPointer(pu32ChNum);

    if (0 == *pu32ChNum)
    {
        MT_ERR_DEMUX("Invalid channel number:%d\n", *pu32ChNum);
        return MT_ERR_DMX_INVALID_PARA;
    }

    Param.u32TimeOutMs = u32TimeOutMs;

    /* get whether the 96 channels data has flag or not*/
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_GET_DATA_FLAG, (ulong)&Param);
    //printf("Param.u32Flag: %d, %d, %d\n",Param.u32Flag[0],Param.u32Flag[1],Param.u32Flag[2]);
    if (MT_SUCCESS == ret)
    {
        mt_u32 i;

        /* in sequence, look up all the channel, and return the existent data channel */
        for (i = 0; i < DMX_CHANNEL_CNT; i++)
        {
            //printf("Param.u32Flag[%d] >> (%d % 32)) & 0x01=%d\n",i/32, i, (Param.u32Flag[i / 32] >> (i % 32)) & 0x01);
            if ((Param.u32Flag[i / 32] >> (i % 32)) & 0x01)
            {
                phChannel[ChNumGet] = DMX_CHANHANDLE(i);
                ChNumGet++;
                //printf("ChNumGet =%d *pu32ChNum=%d\n",ChNumGet,*pu32ChNum);
                if (ChNumGet >= *pu32ChNum)
                {
                    break;
                }
            }
        }
    }
    //printf("ChNumGet =%d *pu32ChNum=%d\n",ChNumGet,*pu32ChNum);

    /* return the number of existent data channel */
    *pu32ChNum = ChNumGet;

    return ret;
}

mt_s32  MT_MPI_DMX_SelectDataHandle(mt_handle *phWatchChannel, mt_u32 u32WatchNum,
                                    mt_handle *phDataChannel, mt_u32 *pu32ChNum, mt_u32 u32TimeOutMs)
{
    mt_s32 ret;
    DMX_SelectDataFlag_S Param;
    mt_u32 ChNumGet = 0;
    mt_u32 i;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(phDataChannel);
    MPIDmxCheckPointer(phWatchChannel);
    MPIDmxCheckPointer(pu32ChNum);
    for (i = 0; i < u32WatchNum; i++)
    {
        DMX_CHECK_CHANHANDLE(phWatchChannel[i]);
    }

    Param.u32Flag[0] = 0;
    Param.u32Flag[1] = 0;
    Param.u32Flag[2] = 0;
    Param.u32Flag[3] = 0;

    if (0 == u32WatchNum)
    {
        MT_ERR_DEMUX("Invalid channel number:%d\n", u32WatchNum);
        return MT_ERR_DMX_INVALID_PARA;
    }

    Param.channel = phWatchChannel;
    Param.channelnum   = u32WatchNum;
    Param.u32TimeOutMs = u32TimeOutMs;

    /* get whether the pointed channel data has flag or not */
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_SELECT_DATA_FLAG, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        /* in sequence, look up all the channel, and return the existent data channel */
        for (i = 0; i < DMX_CHANNEL_CNT; i++)
        {
            if ((Param.u32Flag[i / 32] >> (i % 32)) & 0x01)
            {
                phDataChannel[ChNumGet] = DMX_CHANHANDLE(i);

                ChNumGet++;
                if (ChNumGet >= u32WatchNum)
                {
                    break;
                }
            }
        }
    }

    /* return the number of existent data channel */
    *pu32ChNum = ChNumGet;
    return ret;
}

mt_s32  MT_MPI_DMX_AcquireBuf(mt_handle hChannel, mt_u32 u32AcquireNum,
                              mt_u32 *pu32AcquiredNum, MT_UNF_DMX_DATA_S *pstBuf,
                              mt_u32 u32TimeOutMs)
{
    mt_s32 ret = MT_SUCCESS;
    DMX_AcqMsg_S Param;
    mt_u32 i = 0;
    phys_addr_t u32PhyAddr = 0;
    mt_u32 u32ChId = DMX_INVALID_CHAN_ID;
    MT_UNF_DMX_DATA_S *pstBufTmp;
    MT_UNF_DMX_CHAN_TYPE_E chan_type = MT_UNF_DMX_CHAN_TYPE_BUTT;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    MPIDmxCheckPointer(pu32AcquiredNum);
    MPIDmxCheckPointer(pstBuf);
    pstBufTmp = pstBuf;

    if (0 == u32AcquireNum)
    {
        return MT_ERR_DMX_INVALID_PARA;
    }

    u32ChId = DMX_CHANID(hChannel);
    Param.hChannel = hChannel;
    Param.u32AcquireNum = u32AcquireNum;
    Param.pstBuf = pstBufTmp;
    Param.u32TimeOutMs = u32TimeOutMs;

    /* given the address and length of the buffer, the buffer address array will be copied out in kernel mode */
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_ACQUIRE_MSG, (ulong)&Param);

    if ((MT_SUCCESS != ret) || (Param.u32AcquiredNum == 0))
    {
        *pu32AcquiredNum = 0;
        return ret;
    }

    chan_type = MPIGetChnType(hChannel);

    switch (chan_type)
    {
        case MT_UNF_DMX_CHAN_TYPE_HW_PES:
        case MT_UNF_DMX_CHAN_TYPE_POST:
        {
            for (i = 0; i < Param.u32AcquiredNum; i++)
            {
                u32PhyAddr = pstBufTmp[i].data_phy_addr;

                if ((u32PhyAddr >= g_stChanBuf[u32ChId].u32BufPhyAddr) &&
                    ((u32PhyAddr - g_stChanBuf[u32ChId].u32BufPhyAddr) <= g_stChanBuf[u32ChId].u32BufSize))
                {
                    pstBufTmp[i].pu8Data = (mt_u8*)((ulong)(u32PhyAddr - g_stChanBuf[u32ChId].u32BufPhyAddr) + g_stChanBuf[u32ChId].u32BufUsrVirAddr);
                }
                else
                {
                    MT_ERR_DEMUX("Invalid HW PES/TS phy addr, Phyaddr:%#x, Curaddr:%#x, bufsize:%d, ChanId:%d\n", 
                        g_stChanBuf[u32ChId].u32BufPhyAddr, u32PhyAddr, g_stChanBuf[u32ChId].u32BufSize, u32ChId);
                    *pu32AcquiredNum = 0;
                    return MT_FAILURE;
                }
            }

            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_SEC:
        case MT_UNF_DMX_CHAN_TYPE_ECM_EMM:
        {
            mt_u32 reassamble_buf_phyaddr = 0;

            reassamble_buf_phyaddr = g_stChanBuf[u32ChId].u32BufPhyAddr + \
                DMX_SYMPHONY_SEC_DESC_BUF_SIZE +  g_stChanBuf[u32ChId].u32BufSize;

            for (i = 0; i < Param.u32AcquiredNum; i++)
            {
                u32PhyAddr = pstBufTmp[i].data_phy_addr;
        
                if ((u32PhyAddr >= g_stChanBuf[u32ChId].u32BufPhyAddr) &&
                    ((u32PhyAddr - g_stChanBuf[u32ChId].u32BufPhyAddr) < g_stChanBuf[u32ChId].u32BufSize))
                {
                    pstBufTmp[i].pu8Data = (mt_u8*)((ulong)(u32PhyAddr - g_stChanBuf[u32ChId].u32BufPhyAddr)  + g_stChanBuf[u32ChId].u32BufUsrVirAddr);
                }
                else if ((u32PhyAddr >= g_stChanBuf[u32ChId].u32BufPhyAddr) &&
                   (u32PhyAddr == reassamble_buf_phyaddr))
                {
                    pstBufTmp[i].pu8Data = (mt_u8*)(g_stChanBuf[u32ChId].u32BufUsrVirAddr + DMX_SYMPHONY_SEC_DESC_BUF_SIZE +  g_stChanBuf[u32ChId].u32BufSize);
                }
                else
                {
                    MT_ERR_DEMUX("Invalid SEC Phy addr, Phyaddr:%#x, Curaddr:%#x, bufsize:%d, ChanId:%d\n",
                        g_stChanBuf[u32ChId].u32BufPhyAddr, u32PhyAddr, 
                        (DMX_SYMPHONY_SEC_DESC_BUF_SIZE +  g_stChanBuf[u32ChId].u32BufSize), u32ChId);
                    *pu32AcquiredNum = 0;
                    return MT_FAILURE;
                }
            }

            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            for (i = 0; i < Param.u32AcquiredNum; i++)
            {
                u32PhyAddr = pstBufTmp[i].data_phy_addr;
            
                if ((u32PhyAddr >= g_stChanDescBuf[u32ChId].u32BufPhyAddr) &&
                    ((u32PhyAddr - g_stChanDescBuf[u32ChId].u32BufPhyAddr) < g_stChanDescBuf[u32ChId].u32BufSize))
                {
                    pstBufTmp[i].pu8Data = (mt_u8*)((ulong)(u32PhyAddr - g_stChanDescBuf[u32ChId].u32BufPhyAddr) + g_stChanDescBuf[u32ChId].u32BufUsrVirAddr);
                }
                else
                {
                    MT_ERR_DEMUX("Invalid PES Phy addr, Phyaddr:%#x, Curaddr:%#x, bufsize:%d, ChanId:%d\n",
                        g_stChanDescBuf[u32ChId].u32BufPhyAddr, u32PhyAddr, g_stChanDescBuf[u32ChId].u32BufSize, u32ChId);
                    *pu32AcquiredNum = 0;
                    return MT_FAILURE;
                }
            }

            break;
        }

        default:
            MT_ERR_DEMUX("AcqBuf Err chantype:%d\n", chan_type);
            return MT_ERR_DMX_INVALID_PARA;
    }

    *pu32AcquiredNum = Param.u32AcquiredNum;
    return MT_SUCCESS;
}


mt_s32  MT_MPI_DMX_ReleaseBuf(mt_handle hChannel, mt_u32 u32ReleaseNum,
                              MT_UNF_DMX_DATA_S *pstBuf)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    MPIDmxCheckPointer(pstBuf);

    if (0 == u32ReleaseNum)
    {
        return MT_SUCCESS;
    }

    return MPIReleaseBuf(hChannel, u32ReleaseNum, pstBuf);
}

mt_s32 MT_MPI_DMX_CreatePcrChannel(mt_u32 u32DmxId, ulong *pu32PcrChId)
{
    mt_s32 ret;
    DMX_NewPcr_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pu32PcrChId);

    Param.u32DmxId = u32DmxId;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_PCR_NEW, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *pu32PcrChId = Param.u32PcrId;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_DestroyPcrChannel(mt_u32 u32PcrChId)
{
    MPIDmxCheckDeviceFd();

    return ioctl(g_s32DmxFd, CMD_DEMUX_PCR_DEL, (ulong)&u32PcrChId);
}

mt_s32 MT_MPI_DMX_PcrPidSet(mt_u32 pu32PcrChId, mt_u32 u32Pid)
{
    DMX_PcrPidSet_S Param;

    MPIDmxCheckDeviceFd();

    Param.pu32PcrChId = pu32PcrChId;
    Param.u32Pid = u32Pid;

    return ioctl(g_s32DmxFd, CMD_DEMUX_PCRPID_SET, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_PcrPidGet(mt_u32 pu32PcrChId, mt_u32 *pu32Pid)
{
    mt_s32 ret;
    DMX_PcrPidGet_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pu32Pid);

    Param.pu32PcrChId = pu32PcrChId;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_PCRPID_GET, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        *pu32Pid = Param.u32Pid;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_PcrScrGet(mt_u32 pu32PcrChId, mt_u64 *pu64PcrMs, mt_u64 *pu64ScrMs)
{
    mt_s32 ret;
    DMX_PcrScrGet_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pu64PcrMs);
    MPIDmxCheckPointer(pu64ScrMs);

    Param.pu32PcrChId = pu32PcrChId;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_CURPCR_GET, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        *pu64PcrMs = Param.u64PcrValue;
        *pu64ScrMs = Param.u64ScrValue;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_PcrSyncAttach(mt_u32 u32PcrChId, mt_u32 u32SyncHandle)
{
    mt_s32 ret;
    DMX_PCRSYNC_S Param;

    MPIDmxCheckDeviceFd();

    Param.u32PcrChId = u32PcrChId;
    Param.u32SyncHandle = u32SyncHandle;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_PCRSYN_ATTACH, (ulong)&Param);

    return ret;
}

mt_s32 MT_MPI_DMX_PcrSyncDetach(mt_u32 u32PcrChId)
{
    mt_s32 ret;
    DMX_PCRSYNC_S Param;

    MPIDmxCheckDeviceFd();

    Param.u32PcrChId = u32PcrChId;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_PCRSYN_DETACH, (ulong)&Param);

    return ret;
}

mt_s32 MT_MPI_DMX_GetPESBufferStatus(mt_handle hChannel, MT_MPI_DMX_BUF_STATUS_S *pBufStat)
{
    mt_s32 ret;
    DMX_PesBufStaGet_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    MPIDmxCheckPointer(pBufStat);

    Param.hChannel = hChannel;
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_PES_BUFFER_GETSTAT, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        memcpy(pBufStat, &Param.stBufStat, sizeof(MT_MPI_DMX_BUF_STATUS_S));
    }

    return ret;
}

/* send stream for audio decoder in user mode */
mt_s32 MT_MPI_DMX_AcquireEs(mt_handle hChannel, MT_UNF_ES_BUF_S *pEsBuf)
{
    mt_s32 ret = MT_SUCCESS;
    DMX_PesBufGet_S Param;
    phys_addr_t u32PhyAddr = 0;;
    mt_u32 u32ChId = DMX_INVALID_CHAN_ID;
    MT_UNF_DMX_CHAN_TYPE_E chan_type = MT_UNF_DMX_CHAN_TYPE_BUTT;

    MPIDmxCheckPointer(pEsBuf);
    pEsBuf->pu8Buf = NULL;
    pEsBuf->u32BufLen = 0;
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);    

    Param.hChannel = hChannel;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_ES_BUFFER_GET, (ulong)&Param);

    if (MT_SUCCESS == ret)
    {
        u32ChId = DMX_CHANID(hChannel);
        chan_type = MPIGetChnType(hChannel);

        switch (chan_type)
        {
            case MT_UNF_DMX_CHAN_TYPE_PES:
            {
                u32PhyAddr = Param.stEsBuf.es_data_phy_addr;

                if ((u32PhyAddr >= g_stChanDescBuf[u32ChId].u32BufPhyAddr) &&
                    ((u32PhyAddr - g_stChanDescBuf[u32ChId].u32BufPhyAddr) < g_stChanDescBuf[u32ChId].u32BufSize))
                {                
                    pEsBuf->pu8Buf = (mt_u8*)(g_stChanDescBuf[u32ChId].u32BufUsrVirAddr + (ulong)(u32PhyAddr - g_stChanDescBuf[u32ChId].u32BufPhyAddr));
                }
                else
                {
                    MT_ERR_DEMUX("Invalid PES phy addr, Phyaddr:%#x, Curaddr:%#x, bufsize:%d, ChanId:%d\n", 
                        g_stChanDescBuf[u32ChId].u32BufPhyAddr, u32PhyAddr, g_stChanDescBuf[u32ChId].u32BufSize, u32ChId);
                    return MT_FAILURE;
                }

                break;
            }

            case MT_UNF_DMX_CHAN_TYPE_AUD:
            case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
            case MT_UNF_DMX_CHAN_TYPE_VID:
			case MT_UNF_DMX_CHAN_TYPE_DSS:
            {
                u32PhyAddr = Param.stEsBuf.es_data_phy_addr;

                if ((u32PhyAddr >= g_stChanBuf[u32ChId].u32BufPhyAddr) &&
                    ((u32PhyAddr - g_stChanBuf[u32ChId].u32BufPhyAddr) < g_stChanBuf[u32ChId].u32BufSize))
                {
                    pEsBuf->pu8Buf = (mt_u8*)(g_stChanBuf[u32ChId].u32BufUsrVirAddr + (ulong)(u32PhyAddr - g_stChanBuf[u32ChId].u32BufPhyAddr));
                }
                else
                {
                    MT_ERR_DEMUX("Invalid A/V ES phy addr, Phyaddr:%#x, Curaddr:%#x, bufsize:%d, ChanId:%d\n", 
                        g_stChanBuf[u32ChId].u32BufPhyAddr, u32PhyAddr, g_stChanBuf[u32ChId].u32BufSize, u32ChId);
                    return MT_FAILURE;
                }

                break;
            }

            default:
                MT_ERR_DEMUX("AcqEs Err chantype:%d\n", chan_type);
                return MT_ERR_DMX_INVALID_PARA;

        }

        pEsBuf->u32BufLen = Param.stEsBuf.u32BufLen;
        pEsBuf->u64PtsMs = Param.stEsBuf.u64PtsMs;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_ReleaseEs(mt_handle hChannel, const MT_UNF_ES_BUF_S *pEsBuf)
{
    DMX_PesBufGet_S Param;
    ulong u32UsrAddr = 0;
    mt_u32 u32ChId = DMX_INVALID_CHAN_ID;
    MT_UNF_DMX_CHAN_TYPE_E chan_type = MT_UNF_DMX_CHAN_TYPE_BUTT;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pEsBuf);
    DMX_CHECK_CHANHANDLE(hChannel);

    u32ChId = DMX_CHANID(hChannel);
    chan_type = MPIGetChnType(hChannel);

    switch (chan_type)
    {
        case MT_UNF_DMX_CHAN_TYPE_PES:
        {
            u32UsrAddr = (ulong)pEsBuf->pu8Buf;

            if ((u32UsrAddr >= g_stChanDescBuf[u32ChId].u32BufUsrVirAddr) &&
                ((u32UsrAddr - g_stChanDescBuf[u32ChId].u32BufUsrVirAddr) < g_stChanDescBuf[u32ChId].u32BufSize))
            {                
                Param.stEsBuf.es_data_phy_addr = g_stChanDescBuf[u32ChId].u32BufPhyAddr + (u32UsrAddr - g_stChanDescBuf[u32ChId].u32BufUsrVirAddr);
            }
            else
            {
                MT_ERR_DEMUX("Invalid PES Usr addr, Usraddr:%#x, Curaddr:%#x, bufsize:%d, ChanId:%d\n", 
                    g_stChanDescBuf[u32ChId].u32BufUsrVirAddr, u32UsrAddr, g_stChanDescBuf[u32ChId].u32BufSize, u32ChId);
                return MT_FAILURE;
            }

            break;
        }

        case MT_UNF_DMX_CHAN_TYPE_AUD:
        case MT_UNF_DMX_CHAN_TYPE_AUD_AD:
        case MT_UNF_DMX_CHAN_TYPE_VID:
		case MT_UNF_DMX_CHAN_TYPE_DSS:
        {
            u32UsrAddr = (ulong)pEsBuf->pu8Buf;

            if ((u32UsrAddr >= g_stChanBuf[u32ChId].u32BufUsrVirAddr) &&
                ((u32UsrAddr - g_stChanBuf[u32ChId].u32BufUsrVirAddr) < g_stChanBuf[u32ChId].u32BufSize))
            {
                Param.stEsBuf.es_data_phy_addr = g_stChanBuf[u32ChId].u32BufPhyAddr + (u32UsrAddr - g_stChanBuf[u32ChId].u32BufUsrVirAddr);
            }
            else
            {
                MT_ERR_DEMUX("Invalid A/V ES Usr addr, Usraddr:%#x, Curaddr:%#x, bufsize:%d, ChanId:%d\n", 
                    g_stChanBuf[u32ChId].u32BufUsrVirAddr, u32UsrAddr, g_stChanBuf[u32ChId].u32BufSize, u32ChId);
                return MT_FAILURE;
            }

            break;
        }

        default:
            MT_ERR_DEMUX("ReleaseEs Err chantype:%d\n", chan_type);
            return MT_ERR_DMX_INVALID_PARA;

    }

    Param.stEsBuf.u32BufLen = pEsBuf->u32BufLen;
    Param.stEsBuf.u64PtsMs = pEsBuf->u64PtsMs;
    Param.hChannel = hChannel;

    return ioctl(g_s32DmxFd, CMD_DEMUX_ES_BUFFER_PUT, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_CreateRecChn(MT_UNF_DMX_REC_ATTR_S *pstRecAttr, mt_handle *phRecChn)
{
    mt_s32                  ret;
    DMX_Rec_CreateChan_S    Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstRecAttr);
    MPIDmxCheckPointer(phRecChn);

    memcpy(&Param.RecAttr, pstRecAttr, sizeof(Param.RecAttr));
    MT_INFO_DEMUX("MT_MPI_DMX_CreateRecChn ==============CMD_DEMUX_REC_CHAN_CREATE=0x%x\n",CMD_DEMUX_REC_CHAN_CREATE);
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_CREATE, (ulong)&Param);
    MT_INFO_DEMUX("MT_MPI_DMX_CreateRecChn ========1======ret=%d,Param.RecBufPhyAddr=%x,Param.RecBufSize=0x%x\n",ret,Param.RecBufPhyAddr,Param.RecBufSize);
    if (MT_SUCCESS == ret)
    {
        ulong UsrAddr;

        UsrAddr = (ulong)mt_mmap(Param.RecBufPhyAddr, Param.RecBufSize);
        if (0 == UsrAddr)
        {
            if (MT_SUCCESS != ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_DESTROY, (ulong)&Param.RecHandle))
            {
                MT_ERR_DEMUX("destroy rec failed\n");
            }

            ret = MT_ERR_DMX_MMAP_FAILED;
        }
        else
        {
            mt_u32 RecId = DMX_RECID(Param.RecHandle);

            g_RecMmzBuf[RecId].u32BufPhyAddr    = Param.RecBufPhyAddr;
            g_RecMmzBuf[RecId].u32BufUsrVirAddr = UsrAddr;
            g_RecMmzBuf[RecId].u32BufSize       = Param.RecBufSize;
            MT_INFO_DEMUX("[%s %d]RecId=%d, g_RecMmzBuf[RecId].u32BufPhyAddr =0x%llx, g_RecMmzBuf[RecId].u32BufUsrVirAddr=0x%llx, g_RecMmzBuf[RecId].u32BufSize = 0x%llx\n",
				__FUNCTION__, __LINE__, RecId, g_RecMmzBuf[RecId].u32BufPhyAddr, g_RecMmzBuf[RecId].u32BufUsrVirAddr,g_RecMmzBuf[RecId].u32BufSize);
            *phRecChn = Param.RecHandle;
        }

        if(ret == 0 && Param.RecIdxBufPhyAddr){
            UsrAddr = (ulong)mt_mmap(Param.RecIdxBufPhyAddr, Param.RecIdxBufSize);
            if (0 == UsrAddr)
            {
                if (MT_SUCCESS != ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_DESTROY, (ulong)&Param.RecHandle))
                {
                    MT_ERR_DEMUX("destroy rec failed\n");
                }

                ret = MT_ERR_DMX_MMAP_FAILED;
            }
            else
            {
                mt_u32 RecId = DMX_RECID(Param.RecHandle);

                g_RecIdxMmzBuf[RecId].u32BufPhyAddr    = Param.RecIdxBufPhyAddr;
                g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr = UsrAddr;
                g_RecIdxMmzBuf[RecId].u32BufSize       = Param.RecIdxBufSize;
                MT_INFO_DEMUX("[%s %d]RecId=%d, g_RecIdxMmzBuf[RecId].u32BufPhyAddr =0x%x, g_RecMmzBuf[RecId].u32BufUsrVirAddr=0x%x, g_RecMmzBuf[RecId].u32BufSize = 0x%x\n",
    				__FUNCTION__, __LINE__, RecId, g_RecIdxMmzBuf[RecId].u32BufPhyAddr, g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr,g_RecIdxMmzBuf[RecId].u32BufSize);
                //*phRecChn = Param.RecHandle;
            }
        }
    }

    return ret;
}

mt_s32 MT_MPI_DMX_CreateLinkRecChn(MT_UNF_DMX_REC_ATTR_S *pstRecAttr, mt_handle *phRecChn)
{
    mt_s32                  	ret;
    DMX_LinkRec_CreateChan_S    Param;
	ulong UsrAddr;
	mt_u8 i;
	
    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstRecAttr);
    MPIDmxCheckPointer(phRecChn);

	memset(&Param, 0, sizeof(DMX_LinkRec_CreateChan_S));
    memcpy(&Param.RecAttr, pstRecAttr, sizeof(Param.RecAttr));
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_LINKREC_CHAN_CREATE, (ulong)&Param);	
	
    if (MT_SUCCESS == ret)
	{
		g_LinkRecNodeNum = Param.link_node_num;
		MT_INFO_DEMUX("[%s %d]Param.link_node_num=%d\n", __FUNCTION__, __LINE__, Param.link_node_num);		

		for (i=0; i<Param.link_node_num; i++)
		{
	    	MT_INFO_DEMUX("[%s %d]i=%d, Param.RecBufPhyAddr[i]=%x, Param.RecBufSize[i]=0x%x\n", __FUNCTION__, __LINE__, 
							i, Param.RecBufPhyAddr[i], Param.RecBufSize[i]);					        

	        UsrAddr = (ulong)mt_mmap(Param.RecBufPhyAddr[i], Param.RecBufSize[i]);
	        if (0 == UsrAddr)
	        {
	            if (MT_SUCCESS != ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_DESTROY, (ulong)&Param.RecHandle))
	            {
	                MT_ERR_DEMUX("destroy rec failed\n");
	            }

	            ret = MT_ERR_DMX_MMAP_FAILED;
	        }
	        else
	        {
	            mt_u32 RecId = DMX_RECID(Param.RecHandle);
				
	            g_LinkRecMmzBuf[RecId][i].u32BufPhyAddr    = Param.RecBufPhyAddr[i];
	            g_LinkRecMmzBuf[RecId][i].u32BufUsrVirAddr = UsrAddr;
	            g_LinkRecMmzBuf[RecId][i].u32BufSize       = Param.RecBufSize[i];
	            MT_INFO_DEMUX("[%s %d]RecId=%d, i=%d, g_LinkRecMmzBuf[RecId][i].u32BufPhyAddr =0x%x, g_LinkRecMmzBuf[RecId][i].u32BufUsrVirAddr=0x%x, g_LinkRecMmzBuf[RecId][i].u32BufSize = 0x%x\n",
					__FUNCTION__, __LINE__, RecId, i, g_LinkRecMmzBuf[RecId][i].u32BufPhyAddr, g_LinkRecMmzBuf[RecId][i].u32BufUsrVirAddr, g_LinkRecMmzBuf[RecId][i].u32BufSize);
	            *phRecChn = Param.RecHandle;
	        }
		}

		if(ret == 0 && Param.RecIdxBufPhyAddr)
		{
            UsrAddr = (ulong)mt_mmap(Param.RecIdxBufPhyAddr, Param.RecIdxBufSize);
            if (0 == UsrAddr)
            {
                if (MT_SUCCESS != ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_DESTROY, (ulong)&Param.RecHandle))
                {
                    MT_ERR_DEMUX("destroy rec failed\n");
                }

                ret = MT_ERR_DMX_MMAP_FAILED;
            }
            else
            {
                mt_u32 RecId = DMX_RECID(Param.RecHandle);

                g_RecIdxMmzBuf[RecId].u32BufPhyAddr    = Param.RecIdxBufPhyAddr;
                g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr = UsrAddr;
                g_RecIdxMmzBuf[RecId].u32BufSize       = Param.RecIdxBufSize;
                MT_INFO_DEMUX("Param.RecData.u32DataPhyAddr =0x%x, g_RecMmzBuf[RecId].u32BufUsrVirAddr=0x%x, g_RecMmzBuf[RecId].u32BufSize = 0x%x\n",
    				g_RecIdxMmzBuf[RecId].u32BufPhyAddr, g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr,g_RecIdxMmzBuf[RecId].u32BufSize);
                //*phRecChn = Param.RecHandle;
            }
        }		
    }

    return ret;
}

mt_s32 MT_MPI_DMX_DestroyRecChn(mt_handle hRecChn)
{
    mt_s32 ret = MT_SUCCESS;
	mt_u32 RecId = DMX_RECID(hRecChn);
	
    MPIDmxCheckDeviceFd();

	if (NULL != (mt_void*)g_RecMmzBuf[RecId].u32BufUsrVirAddr)
	{
		if (MT_SUCCESS != mt_munmap((mt_void*)g_RecMmzBuf[RecId].u32BufUsrVirAddr))
	    {
	    	MT_ERR_DEMUX("rec channel %u record buffer unmap failed\n", RecId);
            return MT_ERR_DMX_MUNMAP_FAILED;    
	    }
		
		g_RecMmzBuf[RecId].u32BufPhyAddr    = 0;
	    g_RecMmzBuf[RecId].u32BufUsrVirAddr = 0;
	    g_RecMmzBuf[RecId].u32BufSize       = 0;
	}

	if (NULL != (mt_void*)g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr)
	{
		if (MT_SUCCESS != mt_munmap((mt_void*)g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr))
	    {
	    	MT_ERR_DEMUX("rec channel %u index buffer unmap failed\n", RecId);
            return MT_ERR_DMX_MUNMAP_FAILED;    
	    }
		
		g_RecIdxMmzBuf[RecId].u32BufPhyAddr    = 0;
	    g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr = 0;
	    g_RecIdxMmzBuf[RecId].u32BufSize       = 0;
	}
	
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_DESTROY, (ulong)&hRecChn);
    if (MT_SUCCESS != ret)
    {
    	MT_ERR_DEMUX("destroy rec channel %u failed\n", RecId);
        return ret;        
    }

    return ret;
}

mt_s32 MT_MPI_DMX_DestroyLinkRecChn(mt_handle hRecChn)
{
    mt_s32 ret;
	mt_u8 i = 0;
	mt_u32 RecId = DMX_RECID(hRecChn);

    MPIDmxCheckDeviceFd();

	for (i=0; i<g_LinkRecNodeNum; i++)
	{
		if (NULL != (mt_void*)g_LinkRecMmzBuf[RecId][i].u32BufUsrVirAddr)
		{
			if (MT_SUCCESS != mt_munmap((mt_void*)g_LinkRecMmzBuf[RecId][i].u32BufUsrVirAddr))
	        {
	            MT_ERR_DEMUX("rec channel %u buffer unmap failed\n", RecId);
            	return MT_ERR_DMX_MUNMAP_FAILED;
	        }

			g_LinkRecMmzBuf[RecId][i].u32BufPhyAddr    = 0;
            g_LinkRecMmzBuf[RecId][i].u32BufUsrVirAddr = 0;
            g_LinkRecMmzBuf[RecId][i].u32BufSize       = 0;
		}
	}
	
	if (NULL != (mt_void*)g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr)
	{
		if (MT_SUCCESS != mt_munmap((mt_void*)g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr))
	    {
	    	MT_ERR_DEMUX("rec channel %u index buffer unmap failed\n", RecId);
            return MT_ERR_DMX_MUNMAP_FAILED;    
	    }
		
		g_RecIdxMmzBuf[RecId].u32BufPhyAddr    = 0;
	    g_RecIdxMmzBuf[RecId].u32BufUsrVirAddr = 0;
	    g_RecIdxMmzBuf[RecId].u32BufSize       = 0;
	}
	
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_LINKREC_CHAN_DESTROY, (ulong)&hRecChn);	
    if (ret != MT_SUCCESS)
    {
        MT_ERR_DEMUX("destroy rec channel %u failed\n", RecId);
        return ret;
    }

    return ret;
}


mt_s32 MT_MPI_DMX_AddRecPid(mt_handle hRecChn, mt_u32 u32Pid, mt_handle *phChannel)
{
    mt_s32              ret;
    DMX_Rec_AddPid_S    Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(phChannel);

    Param.RecHandle = hRecChn;
    Param.Pid       = u32Pid;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_ADD_PID, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        *phChannel = Param.ChanHandle;
    }

    return MT_SUCCESS;//ret;
}

mt_s32 MT_MPI_DMX_DelRecPid(mt_handle hRecChn, mt_handle hChannel)
{
    DMX_Rec_DelPid_S Param;

    MPIDmxCheckDeviceFd();

    Param.RecHandle     = hRecChn;
    Param.ChanHandle    = hChannel;

    return ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_DEL_PID, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_DelAllRecPid(mt_handle hRecChn)
{
    MPIDmxCheckDeviceFd();

    return ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_DEL_ALL_PID, (ulong)&hRecChn);
}

mt_s32 MT_MPI_DMX_AddExcludeRecPid(mt_handle hRecChn, mt_u32 u32Pid)
{
    DMX_Rec_ExcludePid_S Param;

    MPIDmxCheckDeviceFd();

    Param.RecHandle = hRecChn;
    Param.Pid       = u32Pid;

    return ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_ADD_EXCLUDE_PID, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_DelExcludeRecPid(mt_handle hRecChn, mt_u32 u32Pid)
{
    DMX_Rec_ExcludePid_S Param;

    MPIDmxCheckDeviceFd();

    Param.RecHandle = hRecChn;
    Param.Pid       = u32Pid;

    return ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_DEL_EXCLUDE_PID, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_DelAllExcludeRecPid(mt_handle hRecChn)
{
    MPIDmxCheckDeviceFd();

    return ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_CANCEL_EXCLUDE, (ulong)&hRecChn);
}

mt_s32 MT_MPI_DMX_StartRecChn(mt_handle hRecChn)
{
    MPIDmxCheckDeviceFd();

    return ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_START, (ulong)&hRecChn);
}

mt_s32 MT_MPI_DMX_StopRecChn(mt_handle hRecChn)
{
    MPIDmxCheckDeviceFd();

    return ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_STOP, (ulong)&hRecChn);
}

mt_s32 MT_MPI_DMX_AcquireRecData(mt_handle hRecChn, MT_UNF_DMX_REC_DATA_S *pstRecData, mt_u32 u32TimeoutMs)
{
    mt_s32                  ret;
    DMX_Rec_AcquireData_S   Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstRecData);

    Param.RecHandle = hRecChn;
    Param.TimeoutMs = u32TimeoutMs;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_ACQUIRE_DATA, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        mt_u32 RecId = DMX_RECID(hRecChn);
        mt_u32 Offset;

        Offset = Param.RecData.u32DataPhyAddr - g_RecMmzBuf[RecId].u32BufPhyAddr;
        //printf("Param.RecData.u32DataPhyAddr =0x%x, g_RecMmzBuf[RecId].u32BufPhyAddr=0x%x, Offset=0x%x\n",
	//		Param.RecData.u32DataPhyAddr, g_RecMmzBuf[RecId].u32BufPhyAddr,Offset);
        pstRecData->pDataAddr       = (mt_u8*)(g_RecMmzBuf[RecId].u32BufUsrVirAddr + Offset);
        //pstRecData->pDataAddr       = (mt_u8*)(Param.RecData.pDataAddr);
        pstRecData->u32Offset       = Offset;
        pstRecData->u32DataPhyAddr  = Param.RecData.u32DataPhyAddr;
        pstRecData->u32Len          = Param.RecData.u32Len;
        pstRecData->u32Size         = g_RecMmzBuf[RecId].u32BufSize;
	//printf("MT_MPI_DMX_AcquireRecData >>>>>>>>>>> pstRecData->pDataAddr=0x%x, len=%d\n",pstRecData->pDataAddr,pstRecData->u32Len);
    }

    return ret;
}

mt_s32 MT_MPI_DMX_AcquireLinkRecData(mt_handle hRecChn, MT_UNF_DMX_REC_DATA_S *pstRecData, mt_u32 u32TimeoutMs)
{
    mt_s32                  	ret;
    DMX_LinkRec_AcquireData_S   Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstRecData);

    Param.RecHandle = hRecChn;
    Param.TimeoutMs = u32TimeoutMs;

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_LINKREC_CHAN_ACQUIRE_DATA, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        mt_u32 RecId = DMX_RECID(hRecChn);
        mt_u32 Offset;
		mt_u8  node_index = Param.node_index;

		//printf("[%s %d]node_index=%d, Param.RecData.u32DataPhyAddr=0x%llx, Param.RecData.u32Len=%d\n", __FUNCTION__, __LINE__, node_index, Param.RecData.u32DataPhyAddr, Param.RecData.u32Len);
        Offset = Param.RecData.u32DataPhyAddr - g_LinkRecMmzBuf[RecId][node_index].u32BufPhyAddr;
        pstRecData->pDataAddr       = (mt_u8*)(g_LinkRecMmzBuf[RecId][node_index].u32BufUsrVirAddr + Offset);
        pstRecData->u32Offset       = Offset;
        pstRecData->u32DataPhyAddr  = Param.RecData.u32DataPhyAddr;
        pstRecData->u32Len          = Param.RecData.u32Len;
        pstRecData->u32Size         = g_LinkRecMmzBuf[RecId][node_index].u32BufSize;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_ReleaseRecData(mt_handle hRecChn, const MT_UNF_DMX_REC_DATA_S *pstRecData)
{
    DMX_Rec_ReleaseData_S   Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstRecData);

    Param.RecHandle = hRecChn;
    memcpy(&Param.RecData, pstRecData, sizeof(MT_UNF_DMX_REC_DATA_S));

    return ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_RELEASE_DATA, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_ReleaseLinkRecData(mt_handle hRecChn, const MT_UNF_DMX_REC_DATA_S *pstRecData)
{
    DMX_Rec_ReleaseData_S   Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstRecData);

    Param.RecHandle = hRecChn;
    memcpy(&Param.RecData, pstRecData, sizeof(MT_UNF_DMX_REC_DATA_S));

    return ioctl(g_s32DmxFd, CMD_DEMUX_LINKREC_CHAN_RELEASE_DATA, (ulong)&Param);
}


mt_s32 MT_MPI_DMX_AcquireRecIndex(mt_handle hRecChn, MT_UNF_DMX_REC_INDEX_S *pstRecIndex, mt_u32 u32TimeoutMs)
{
    mt_s32                  ret;
    DMX_Rec_AcquireIndex_S  Param;
    //static MT_UNF_DMX_REC_INDEX_S  preIndexData = {0};

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstRecIndex);

    memset(&Param, 0, sizeof(DMX_Rec_AcquireIndex_S));
    Param.RecHandle = hRecChn;
    Param.TimeoutMs = u32TimeoutMs;
    if(1==pstRecIndex->u32DataTimeMs){
        Param.IndexData.u32DataTimeMs = 1;
    }else{
        Param.IndexData.u32DataTimeMs = 0;
    }
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_ACQUIRE_INDEX, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        #if 0
        //printf("\n%s_%d:%x, 0  framesize=%x, offset=%x\n",__func__,__LINE__,&Param.IndexData.u32FrameSize, &Param.IndexData.u64GlobalOffset);
        if(preIndexData.u32FrameSize == 0 && preIndexData.u64GlobalOffset == 0)
        {
            memcpy(&preIndexData, &Param.IndexData, sizeof(MT_UNF_DMX_REC_INDEX_S));
            return MT_ERR_DMX_NOAVAILABLE_DATA;
        }
        else
        {
            preIndexData.u32FrameSize = Param.IndexData.u32FrameSize;
            memcpy(pstRecIndex, &preIndexData, sizeof(MT_UNF_DMX_REC_INDEX_S));
            memcpy(&preIndexData, &Param.IndexData, sizeof(MT_UNF_DMX_REC_INDEX_S));
        }
        //printf("\n%s_%d:%x, 1  framesize=%x, offset=%x\n",__func__,__LINE__,pstRecIndex->u32FrameSize, pstRecIndex->u64GlobalOffset);
        #else
        memcpy(pstRecIndex, &Param.IndexData, sizeof(MT_UNF_DMX_REC_INDEX_S));
        #endif
    }

    return ret;
}

mt_s32 MT_MPI_DMX_AcquireLinkRecIndex(mt_handle hRecChn, MT_UNF_DMX_REC_INDEX_S *pstRecIndex, mt_u32 u32TimeoutMs)
{
    mt_s32                  ret;
    DMX_Rec_AcquireIndex_S  Param;
    //static MT_UNF_DMX_REC_INDEX_S  preIndexData = {0};

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstRecIndex);

    memset(&Param, 0, sizeof(DMX_Rec_AcquireIndex_S));
    Param.RecHandle = hRecChn;
    Param.TimeoutMs = u32TimeoutMs;
    if(1==pstRecIndex->u32DataTimeMs){
        Param.IndexData.u32DataTimeMs = 1;
    }else{
        Param.IndexData.u32DataTimeMs = 0;
    }
    ret = ioctl(g_s32DmxFd, CMD_DEMUX_LINKREC_CHAN_ACQUIRE_INDEX, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {        
        memcpy(pstRecIndex, &Param.IndexData, sizeof(MT_UNF_DMX_REC_INDEX_S));
    }

    return ret;
}

mt_s32 MT_MPI_DMX_GetRecBufferStatus(mt_handle hRecChn, MT_UNF_DMX_RECBUF_STATUS_S *pstBufStatus)
{
    mt_s32              ret;
    DMX_Rec_BufStatus_S Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pstBufStatus);

    Param.RecHandle = hRecChn;
    mt_u32 RecId = DMX_RECID(hRecChn);

    ret = ioctl(g_s32DmxFd, CMD_DEMUX_REC_CHAN_GET_BUF_STATUS, (ulong)&Param);
    if (MT_SUCCESS == ret)
    {
        memcpy(pstBufStatus, &Param.BufStatus, sizeof(MT_UNF_DMX_RECBUF_STATUS_S));
        pstBufStatus->u32BufUsrAddr =  g_RecMmzBuf[RecId].u32BufUsrVirAddr;
        pstBufStatus->u32BufPhyAddr = g_RecMmzBuf[RecId].u32BufPhyAddr;
    }

    return ret;
}

mt_s32 MT_MPI_DMX_Invoke(MT_UNF_DMX_INVOKE_TYPE_E enCmd, const mt_void *pCmdPara)
{
    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pCmdPara);

    if (MT_UNF_DMX_INVOKE_TYPE_CHAN_CC_REPEAT_SET == enCmd)
    {
        DMX_SetChan_CC_REPEAT_S Param;
        //memcpy(&Param.stChCCRepeatSet, (MT_UNF_DMX_CHAN_CC_REPEAT_SET_S*)pCmdPara, sizeof(MT_UNF_DMX_CHAN_CC_REPEAT_SET_S));
        memcpy(&Param.stChCCRepeatSet, pCmdPara, sizeof(MT_UNF_DMX_CHAN_CC_REPEAT_SET_S));
        return ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_CC_REPEAT_SET, (ulong)&Param);
    }

    if (MT_UNF_DMX_INVOKE_TYPE_PUSI_SET == enCmd)
    {
        MT_UNF_DMX_PUSI_SET_S Param;
        //memcpy(&Param, (MT_UNF_DMX_PUSI_SET_S*)pCmdPara, sizeof(MT_UNF_DMX_PUSI_SET_S));
        memcpy(&Param, pCmdPara, sizeof(MT_UNF_DMX_PUSI_SET_S));
        return ioctl(g_s32DmxFd, CMD_DEMUX_SET_PUSI, (ulong)&Param);
    }

    if (MT_UNF_DMX_INVOKE_TYPE_TEI_SET == enCmd)
    {
        MT_UNF_DMX_TEI_SET_S Param;
        memcpy(&Param, pCmdPara, sizeof(MT_UNF_DMX_TEI_SET_S));

        return ioctl(g_s32DmxFd, CMD_DEMUX_SET_TEI, (ulong)&Param);
    }
    if (MT_UNF_DMX_INVOKE_TYPE_TSI_ATTACH_TSO == enCmd)
    {
        MT_UNF_DMX_TSI_ATTACH_TSO_S Param;
        //memcpy(&Param, (MT_UNF_DMX_TSI_ATTACH_TSO_S*)pCmdPara, sizeof(MT_UNF_DMX_TSI_ATTACH_TSO_S));
        memcpy(&Param, pCmdPara, sizeof(MT_UNF_DMX_TSI_ATTACH_TSO_S));
        return ioctl(g_s32DmxFd, CMD_DEMUX_TSI_ATTACH_TSO, (ulong)&Param);
    }

    MT_ERR_DEMUX("unknow cmd:%d.\n",enCmd);
    return MT_ERR_DMX_INVALID_PARA;

}


mt_void MT_MPI_DMX_DumpAllRegister(void)
{
    mt_u32 Param = 0;

    (void)ioctl(g_s32DmxFd, CMD_DEMUX_DUMP_ALL_REGISTER, (ulong)&Param);

    return;
}


mt_s32 MT_MPI_DMX_BufFullCareSet(mt_handle hChannel, const MT_UNF_DMX_BUF_FULL_CARE_S *pBufFullCare)
{
    DMX_BufFullCare_S   Param;

    MPIDmxCheckDeviceFd();
    MPIDmxCheckPointer(pBufFullCare);

    Param.hChannel = hChannel;
    memcpy(&Param.stBufFullCare, pBufFullCare, sizeof(MT_UNF_DMX_BUF_FULL_CARE_S));

    return ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_BUF_FULL_CARE_SET, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_DataPushStart(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    return ioctl(g_s32DmxFd, CMD_DEMUX_DATA_PUSH_START, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_DataPushStop(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    return ioctl(g_s32DmxFd, CMD_DEMUX_DATA_PUSH_STOP, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_ChannelIndexResume(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);
    return ioctl(g_s32DmxFd, CMD_DEMUX_CHAN_INDEX_RESUME, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_T2MI_Config(MT_UNF_DMX_T2MI_CONFIG_S mpi_t2mi_para)
{
	mt_s32              ret = MT_SUCCESS;
    DMX_Port_SetAttr_S  Param;
	DMX_T2MI_Para_S drv_t2mi_param = {0};
	
	ret = DMX_MPI_PortGetTypeAndID(mpi_t2mi_para.input, &Param.PortMode, &Param.PortId);
	if (MT_SUCCESS != ret)
    {
    	MT_ERR_DEMUX("mpi_t2mi_para.input=0x%x, get portid error\n", mpi_t2mi_para.input);
        return ret;
    }
	drv_t2mi_param.input = Param.PortId;

	ret = DMX_MPI_PortGetTypeAndID(mpi_t2mi_para.output, &Param.PortMode, &Param.PortId);
	if (MT_SUCCESS != ret)
    {
    	MT_ERR_DEMUX("mpi_t2mi_para.output=0x%x, get portid error\n", mpi_t2mi_para.output);
        return ret;
    }
	drv_t2mi_param.output = Param.PortId;

	drv_t2mi_param.enable = mpi_t2mi_para.enable;
	drv_t2mi_param.pid = mpi_t2mi_para.pid;
	drv_t2mi_param.plpid = mpi_t2mi_para.plpid;
	
    (void)ioctl(g_s32DmxFd, CMD_DEMUX_T2MI_CONFIG, (ulong)&drv_t2mi_param);

	return ret;
}

mt_s32 MT_MPI_DMX_T2MIEnable(mt_u32 enable)
{
	MPIDmxCheckDeviceFd();
    return ioctl(g_s32DmxFd, CMD_DEMUX_T2MI_ENABLE, (ulong)&enable);
}

mt_s32 MT_MPI_DMX_T2MISetInCh(MT_UNF_DMX_PORT_E enPortId)
{
	MPIDmxCheckDeviceFd();
    return ioctl(g_s32DmxFd, CMD_DEMUX_T2MI_SET_IN_CH, (ulong)&enPortId);
}

mt_s32 MT_MPI_DMX_T2MISetOutCh(MT_UNF_DMX_PORT_E enPortId)
{
	MPIDmxCheckDeviceFd();
    return ioctl(g_s32DmxFd, CMD_DEMUX_T2MI_SET_OUT_CH, (ulong)&enPortId);
}

mt_s32 MT_MPI_DMX_T2MISetPid(mt_u32 pid)
{
	MPIDmxCheckDeviceFd();
    return ioctl(g_s32DmxFd, CMD_DEMUX_T2MI_SET_PID, (ulong)&pid);
}

mt_s32 MT_MPI_DMX_T2MISetPlpid(mt_u32 plpid)
{
	MPIDmxCheckDeviceFd();
    return ioctl(g_s32DmxFd, CMD_DEMUX_T2MI_SET_PLPID, (ulong)&plpid);
}

mt_s32 MT_MPI_DMX_T2MISoftReset(void)
{
    mt_u32 Param = 0;

	MPIDmxCheckDeviceFd();
    return ioctl(g_s32DmxFd, CMD_DEMUX_T2MI_SOFTRESET, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_Soft_Reset(void)
{
    mt_u32 Param = 0;

	MPIDmxCheckDeviceFd();
    return ioctl(g_s32DmxFd, CMD_DEMUX_SOFT_RESET, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_HardwareInit(void)
{
    mt_u32 Param = 0;

	MPIDmxCheckDeviceFd();
    return ioctl(g_s32DmxFd, CMD_DEMUX_HAEDWARE_INIT, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_FastPlayStart(mt_u8 swtsi_num, mt_u8 rec_id, mt_u8 node_num)
{
	DMX_FastPlay_Param_S Param;

	MPIDmxCheckDeviceFd();
	
	Param.swtsi_num = swtsi_num;
	Param.rec_id    = rec_id;
	Param.node_num  = node_num;
    return ioctl(g_s32DmxFd, CMD_DEMUX_FASTPLAY_START, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_FastPlayStop(void)
{
	mt_u32 Param = 0;
	
    MPIDmxCheckDeviceFd();
    return ioctl(g_s32DmxFd, CMD_DEMUX_FASTPLAY_STOP, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_SetPortId(mt_handle hChannel, MT_UNF_DMX_PORT_E enPortId)
{
	DMX_Set_PortId_S	Param;

	DMX_MPI_PortGetTypeAndID(enPortId, &Param.PortMode, &Param.PortId);
	Param.hChannel = hChannel;
	
    return ioctl(g_s32DmxFd, CMD_DEMUX_DMX_SET_PORTID, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_TrickSeekIn(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    return ioctl(g_s32DmxFd, CMD_DEMUX_TRICK_SEEK_IN, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_TrickSeekOut(mt_handle hChannel)
{
    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    return ioctl(g_s32DmxFd, CMD_DEMUX_TRICK_SEEK_OUT, (ulong)&hChannel);
}

mt_s32 MT_MPI_DMX_Get_TSI_Clk(MT_UNF_DMX_TSI_CLK_E *sel_clk)
{	
	return ioctl(g_s32DmxFd, CMD_DEMUX_GET_CLK, (ulong)sel_clk);
}

mt_s32 MT_MPI_DMX_Set_TSI_Clk(MT_UNF_DMX_TSI_CLK_E sel_clk)
{
	return ioctl(g_s32DmxFd, CMD_DEMUX_SET_CLK, (ulong)&sel_clk);
}

mt_s32 MT_MPI_DMX_Get_TSPort_Ctrl(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CTRL_S *port_ctrl)
{
	mt_s32            ret = MT_FAILURE;
    DMX_Port_Ctrl_S   Param;

	memset(&Param, 0, sizeof(DMX_Port_Ctrl_S));
	ret = DMX_MPI_PortGetTypeAndID(port_id, &Param.PortMode, &Param.PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

	ret = ioctl(g_s32DmxFd, CMD_DEMUX_GET_PORT_CTRL, (ulong)&Param);	
	if (MT_SUCCESS == ret)
	{
		port_ctrl->all = Param.PortValue.all;
	}
	
	return ret;
}

mt_s32 MT_MPI_DMX_Set_TSPort_Ctrl(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CTRL_S port_ctrl)
{
	mt_s32            ret = MT_FAILURE;
    DMX_Port_Ctrl_S   Param;

	memset(&Param, 0, sizeof(DMX_Port_Ctrl_S));
	ret = DMX_MPI_PortGetTypeAndID(port_id, &Param.PortMode, &Param.PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

	Param.PortValue.all = port_ctrl.all;
	
	return ioctl(g_s32DmxFd, CMD_DEMUX_SET_PORT_CTRL, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_Get_TSPort_Clk(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CLK_SEL_E *port_clk)
{
	mt_s32			ret = MT_FAILURE;
    DMX_Port_Clk_S	Param;

	memset(&Param, 0, sizeof(DMX_Port_Clk_S));
	ret = DMX_MPI_PortGetTypeAndID(port_id, &Param.PortMode, &Param.PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

	ret = ioctl(g_s32DmxFd, CMD_DEMUX_GET_PORT_CLK, (ulong)&Param);
	if (MT_SUCCESS == ret)
	{
		*port_clk = Param.PortClk;
	}
	
	return ret;
}

mt_s32 MT_MPI_DMX_Set_TSPort_Clk(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_CLK_SEL_E port_clk)
{
	mt_s32			ret = MT_FAILURE;
    DMX_Port_Clk_S	Param;

	memset(&Param, 0, sizeof(DMX_Port_Clk_S));
	ret = DMX_MPI_PortGetTypeAndID(port_id, &Param.PortMode, &Param.PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

	Param.PortClk = port_clk;
	
	return ioctl(g_s32DmxFd, CMD_DEMUX_SET_PORT_CLK, (ulong)&Param);
}

mt_s32 MT_MPI_DMX_Get_TSPort_Src(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_SRC_SEL_E *port_src)
{
	mt_s32         ret = MT_FAILURE;
    DMX_Port_Src_S   Param;

	memset(&Param, 0, sizeof(DMX_Port_Src_S));
	ret = DMX_MPI_PortGetTypeAndID(port_id, &Param.PortMode, &Param.PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

	ret = ioctl(g_s32DmxFd, CMD_DEMUX_GET_PORT_SRC, (ulong)&Param);
	if (MT_SUCCESS == ret)
	{
		*port_src = Param.PortSrc;
	}
	
	return ret;
}

mt_s32 MT_MPI_DMX_Set_TSPort_Src(MT_UNF_DMX_PORT_E port_id, MT_UNF_DMX_PORT_SRC_SEL_E port_src)
{
	mt_s32         ret = MT_FAILURE;
    DMX_Port_Src_S   Param;

	memset(&Param, 0, sizeof(DMX_Port_Src_S));
	ret = DMX_MPI_PortGetTypeAndID(port_id, &Param.PortMode, &Param.PortId);
    if (MT_SUCCESS != ret)
    {
        return ret;
    }

	Param.PortSrc = port_src;
	
	return ioctl(g_s32DmxFd, CMD_DEMUX_SET_PORT_SRC, (ulong)&Param);
}


/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_s32 MT_MPI_DMX_CiplusEnable(mt_handle hChannel, mt_u8 isenable)
{
    TSI_CIPlus_Enable_S Param;

    MPIDmxCheckDeviceFd();
    DMX_CHECK_CHANHANDLE(hChannel);

    Param.hChannel = hChannel;
    Param.isenable= isenable;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_CHAN_EN, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_Ciplus_RecChanSet(mt_handle recchan)
{
    TSI_CIPlus_ChCfg_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);
    Param.chan = recchan;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_REC_CH_SET, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_Ciplus_SwtsiChanSet(mt_handle swtsichan)
{
    TSI_CIPlus_ChCfg_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);
   	mt_u32 id = (mt_u32)swtsichan - (mt_u32)MT_UNF_DMX_PORT_RAM_0;
    Param.chan = id;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_SWTSI_CH_SET, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_Ciplus_BufCfg(mt_u8 usecache)
{
    TSI_CIPlus_BufCfg_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);

	Param.use_cache = usecache;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_BUF_CFG, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_Ciplus_ClkCfg(mt_u32 clkdiv)
{
    TSI_CIPlus_CamClkCfg_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);

    Param.div = clkdiv;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_CAMCLK_CFG, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_Ciplus_TsIntervalCfg(mt_u32 interval)
{
    TSI_CIPlus_TsPktIntervalCfg_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);

    Param.interval = interval;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_TSINTERVAL_CFG, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_Tsi2SourceCfg(mt_u8 from_cam,mt_u8 serial)
{
    TSI_TSI2_SourceCfg_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);

    Param.from_cam = from_cam;
    Param.serial = serial;
    return ioctl(g_s32DmxFd, CMD_TSI_TSI2_SRC_CFG, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_LlnNumStartCfg(mt_u32 lln_num)
{
    TSI_CIPlus_LlnNumStart_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);

    Param.lln_num_start = lln_num;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_LLN_NUM_START_CFG, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_SwtsiByteorderCfg(mt_u32 islittle)
{
	 TSI_CIPlus_SwtsiByteorder_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);
	printf("MT_MPI_Tsi_SwtsiByteorderCfg,islittle:%d\n",islittle);
    Param.byteorder = islittle & 0x01;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_SWTSI_BYTEORDER_CFG, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_SwtsiBufFullCfg(mt_handle hChannel,mt_u32 fullcfg)
{
	 TSI_CIPlus_SwtsiFullCfg_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);
   	mt_u32 id = (mt_u32)hChannel - (mt_u32)MT_UNF_DMX_PORT_RAM_0;

    Param.hChannel = id;
    Param.care_full = fullcfg & 0x01;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_SWTSI_FULL_CFG, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_AhbRdDelayCfg(mt_handle hChannel,mt_u32 delay)
{
	 TSI_CIPlus_AhbRdDelay_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);

    Param.ahb_rd_delay= delay;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_AHBRD_DELAY_CFG, (ulong)&Param);
}

mt_s32 MT_MPI_Tsi_Ciplus_GetStatus(mt_u8 *bufstatus,mt_u8 *cistatus)
{
	TSI_CIPlus_Status_S Param;
	int ret = 0;
	ret = ioctl(g_s32DmxFd,CMD_TSI_CI_GETSTATUS,(ulong)&Param);
	*bufstatus = Param.bufstatus;
	*cistatus = Param.cistatus;
	return ret;
}

mt_s32 MT_MPI_Tsi_Ciplus_EnableCfg(mt_u8 enable)
{
    TSI_CIPlus_EnableCfg_S  Param;

    MPIDmxCheckDeviceFd();
   // DMX_CHECK_CHANHANDLE(hChannel);

	Param.enable = enable;
    return ioctl(g_s32DmxFd, CMD_TSI_CI_ENABLE_CFG, (ulong)&Param);
}
#endif/*add end*/

