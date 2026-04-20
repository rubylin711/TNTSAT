#include <sys/types.h>
#include <sys/stat.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <string.h>   /* for NULL */
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "mt_type.h"

#include "mt_unf_demux.h"
#include "mt_unf_avplay.h"
#include "mt_adp_debug.h"
#include "mt_adp_demux.h"
#include "mt_adp_search.h"


#ifdef MTADP_DEMUX_DEBUG

#define MTADP_DEMUX_PRINT   MTADP_PRINT

#else

#define MTADP_DEMUX_PRINT

#endif


#define MTADP_DEMUX_FUNCTION_ENTER()  MTADP_DEMUX_PRINT(" [MTADP_DEMUX][%s]: Enter ==>> \n", __FUNCTION__)
#define MTADP_DEMUX_FUNCTION_EXIT()   MTADP_DEMUX_PRINT(" [MTADP_DEMUX][%s]: Exit ==<< \n", __FUNCTION__)

#define MTADP_DEMUX_FATAL_PRINT(fmt...)       MTADP_DEMUX_PRINT(" [MTADP_DEMUX][FATAL] " fmt)
#define MTADP_DEMUX_ERR_PRINT(fmt...)         MTADP_DEMUX_PRINT(" [MTADP_DEMUX][ERROR] " fmt)
#define MTADP_DEMUX_WARN_PRINT(fmt...)        MTADP_DEMUX_PRINT(" [MTADP_DEMUX][WARN] "  fmt)
#define MTADP_DEMUX_INFO_PRINT(fmt...)        MTADP_DEMUX_PRINT(" [MTADP_DEMUX][INFO] "  fmt)
#define MTADP_DEMUX_MT_DBG_PRINT(fmt...)      MTADP_DEMUX_PRINT(" [MTADP_DEMUX][DEBUG] " fmt)


static MT_BOOL g_DmxInitFlag = MT_FALSE;
static MT_BOOL bIPInjectFlag[DMX_PORT_COUNT] = { 0 };
static MT_BOOL bFileInjectFlag[DMX_PORT_COUNT] = { 0 };

mt_handle g_PortTsBuf[DMX_PORT_COUNT];

IP_PLAY_HANDLE_S IP_Play_Handles[MAX_IP_THREAD_CNT];
FILE_PLAY_HANDLE_S FILE_Play_Handles[MAX_FILE_THREAD_CNT];
ES_FILE_PLAY_HANDLE_S Es_FILE_Play_Handles[MAX_FILE_THREAD_CNT];

mt_s32 DMX_Init()
{
    mt_u32 i;

    if (0 == g_DmxInitFlag)
    {

        for (i = 0; i < DMX_PORT_COUNT; i++)
        {
            g_PortTsBuf[i]     = MT_INVALID_HANDLE;
            bIPInjectFlag[i]   = MT_FALSE;
            bFileInjectFlag[i] = MT_FALSE;
        }

        for (i = 0; i < MAX_IP_THREAD_CNT; i++)
        {
            IP_Play_Handles[i].bUsed = MT_FALSE;
        }

        for (i = 0; i < MAX_FILE_THREAD_CNT; i++)
        {
            FILE_Play_Handles[i].bUsed = MT_FALSE;
        }

        g_DmxInitFlag = 1;
    }

    return MT_SUCCESS;
}

mt_s32 DMX_DeInit()
{
    mt_s32 ret = MT_SUCCESS;

    if (g_DmxInitFlag == 1)
    {

        g_DmxInitFlag = 0;
    }

    return ret;
}

mt_s32 DMX_Prepare(mt_u32 u32DmxID, mt_u32 u32PortID, mt_u32 u32BufSize)
{
    MT_UNF_DMX_TSBUF_STATUS_S Status;

    if ((u32DmxID >= DMX_COUNT) || (u32PortID >= DMX_PORT_COUNT))
    {
        return MT_FAILURE;
    }


    /*for IP play*/
    if (u32PortID >= DMX_DVBPORT_COUNT)
    {
        if (MT_INVALID_HANDLE != g_PortTsBuf[u32PortID])
        {
            MT_UNF_DMX_GetTSBufferStatus(g_PortTsBuf[u32PortID],&Status);
            if(Status.u32BufSize == u32BufSize)
                  return MT_SUCCESS;

            MT_UNF_DMX_DestroyTSBuffer(g_PortTsBuf[u32PortID]);
        }
        g_PortTsBuf[u32PortID] = MT_INVALID_HANDLE;
        if(u32BufSize != 0)
        {
             MT_UNF_DMX_CreateTSBuffer(u32PortID, u32BufSize, &g_PortTsBuf[u32PortID]);
        }
        printf(" DMX_Prepare  port %d handle = 0x%x \n",u32PortID, (mt_u32)g_PortTsBuf[u32PortID]);
    }

    return MT_SUCCESS;
}



mt_s32 DMX_SectionStartDataFilter(mt_u32 u32DmxId, DMX_DATA_FILTER_S * pstDataFilter)
{
    mt_u32 i;
    mt_u8 *p;
    MT_UNF_DMX_CHAN_ATTR_S stChanAttr;
    mt_handle hChan, hFilter = MT_NULL;
    mt_s32 s32Ret;
    MT_UNF_DMX_FILTER_ATTR_S stFilterAttr;
    mt_u32 u32AquiredNum = 0;
    mt_u8 *u8DataBuf=NULL;
    mt_u32 u32BufSize[MAX_SECTION_NUM];

    u8DataBuf=malloc(MAX_SECTION_LEN * MAX_SECTION_NUM);
    if(NULL==u8DataBuf){
        return MT_FAILURE;
    }
    stChanAttr.u32BufSize = 16 * 1024;
    stChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    stChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_BY_SYNTAX_AND_DISCARD;
    stChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    s32Ret=MT_UNF_DMX_CreateChannel(u32DmxId, &stChanAttr, &hChan);
    if(MT_SUCCESS!=s32Ret){
        printf("create ch err\n");
        goto FREE_BIGMEM;
    }
    s32Ret=MT_UNF_DMX_SetChannelPID(hChan, pstDataFilter->u32TSPID);
    if(MT_SUCCESS!=s32Ret){
        printf("set pid err\n");
        goto FREE_CHANNEL;
    }
    stFilterAttr.u32FilterDepth = pstDataFilter->u16FilterDepth;
    memcpy(stFilterAttr.au8Match, pstDataFilter->u8Match, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Mask, pstDataFilter->u8Mask, DMX_FILTER_MAX_DEPTH);
    memcpy(stFilterAttr.au8Negate, pstDataFilter->u8Negate, DMX_FILTER_MAX_DEPTH);

    s32Ret=MT_UNF_DMX_CreateFilter(u32DmxId, &stFilterAttr, &hFilter);
    if(MT_SUCCESS!=s32Ret){
        printf("create flt err\n");
        goto FREE_CHANNEL;
    }
    s32Ret=MT_UNF_DMX_SetFilterAttr(hFilter, &stFilterAttr);
    s32Ret|=MT_UNF_DMX_AttachFilter(hFilter, hChan);
    if(MT_SUCCESS!=s32Ret){
        printf("attach flt err\n");
        goto FREE_FILTER;
    }
    s32Ret=MT_UNF_DMX_OpenChannel(hChan);
    if(MT_SUCCESS!=s32Ret){
        printf("open chnl err\n");
        goto DETCH_FILTER;
    }

    memset(u8DataBuf, 0, MAX_SECTION_LEN * MAX_SECTION_NUM);
    memset(u32BufSize, 0, sizeof(u32BufSize));
    s32Ret = MTADP_DEMUX_DataRead(hChan, pstDataFilter->u32TimeOut, u8DataBuf, &u32AquiredNum , u32BufSize);
    if (MT_SUCCESS == s32Ret)
    {
        p = u8DataBuf;
        for(i=0;i<u32AquiredNum;i++)
        {
            pstDataFilter->funSectionFunCallback(p, u32BufSize[i], pstDataFilter->pSectionStruct);
            p = p+MAX_SECTION_LEN;
        }
    }
    else
    {
         printf("MTADP_DEMUX_DataRead return MT_FAILURE\n");
    }
    s32Ret = MT_UNF_DMX_CloseChannel(hChan);
    if(MT_SUCCESS!=s32Ret){
        printf("close ch err\n");
    }
    DETCH_FILTER:
    s32Ret = MT_UNF_DMX_DetachFilter(hFilter, hChan);
    if(MT_SUCCESS!=s32Ret){
        printf("Det flt err\n");
    }
    FREE_FILTER:
    s32Ret = MT_UNF_DMX_DestroyFilter(hFilter);
    if(MT_SUCCESS!=s32Ret){
        printf("Del flt err\n");
    }
    FREE_CHANNEL:
    s32Ret = MT_UNF_DMX_DestroyChannel(hChan);
    if(MT_SUCCESS!=s32Ret){
        printf("Del ch err\n");
    }
    FREE_BIGMEM:
    free(u8DataBuf);
    return s32Ret;
}

/*
    Timeout:in millisecond
 */

mt_s32 MTADP_DEMUX_DataRead(mt_handle hChannel, mt_u32 u32TimeOutms, mt_u8 *pBuf, mt_u32* pAcquiredNum , mt_u32 * pBuffSize)
{
    mt_u8  u8tableid = 0;
    MT_UNF_DMX_DATA_S sSection[32];
    mt_u32 num, i = 0;
    mt_u32 count = 0 ;
    mt_u32 u32Times = 0;
    mt_u32 u32SecTotalNum=0, u32SecNum = 0;
    mt_u32 RequestNum = 32;
    mt_u8 u8SecGotFlag[MAX_SECTION_NUM];

    u32Times = u32TimeOutms / 10;

    memset(u8SecGotFlag, 0, sizeof(u8SecGotFlag[MAX_SECTION_NUM]));

    while (--u32Times)
    {
        num = 0;
        if ((MT_SUCCESS == MT_UNF_DMX_AcquireBuf(hChannel, RequestNum, &num, sSection, u32TimeOutms)) && (num > 0))
        {
            for (i = 0; i < num; i++)
            {
                if (sSection[i].enDataType == MT_UNF_DMX_DATA_TYPE_WHOLE)
                {
                    u8tableid = sSection[i].pu8Data[0];
                    if( ((EIT_TABLE_ID_SCHEDULE_ACTUAL_LOW <= u8tableid) && ( u8tableid <= EIT_TABLE_ID_SCHEDULE_ACTUAL_HIGH))
                        ||((EIT_TABLE_ID_SCHEDULE_OTHER_LOW <= u8tableid) && ( u8tableid <= EIT_TABLE_ID_SCHEDULE_OTHER_HIGH)))
                    {
                        u32SecNum = sSection[i].pu8Data[6]>>3;
                        u32SecTotalNum = (sSection[i].pu8Data[7]>>3) +1;
                    }
                    else if ( TDT_TABLE_ID == u8tableid ||TOT_TABLE_ID == u8tableid)
                    {
                        u32SecNum = 0;
                        u32SecTotalNum = 1;
                    }
                    else
                    {
                        u32SecNum = sSection[i].pu8Data[6];
                        u32SecTotalNum = sSection[i].pu8Data[7] + 1;
                    }

              if(u8SecGotFlag[u32SecNum] == 0)
                    {
                        memcpy((void *)(pBuf + u32SecNum * MAX_SECTION_LEN), sSection[i].pu8Data, sSection[i].u32Size);
                        u8SecGotFlag[u32SecNum] = 1;
                        pBuffSize[  u32SecNum] = sSection[i].u32Size;
                        count++;
                    }
                }
            }
            MT_UNF_DMX_ReleaseBuf(hChannel, num, sSection);

            /* to check if all sections are received*/
            if(u32SecTotalNum == count)
                break;
        }
        else
        {
            printf("MT_UNF_DMX_AcquireBuf time out\n");
            return MT_FAILURE;
        }
    }

    if (u32Times == 0)
    {
        printf("MT_UNF_DMX_AcquireBuf time out\n");
        return MT_FAILURE;
    }

    if(pAcquiredNum != MT_NULL)
        *pAcquiredNum = u32SecTotalNum;

    return MT_SUCCESS;
}



MT_S32 MTADP_DEMUX_OpenChannel(mt_u32 u32DmxId, MT_U16  pid, MT_HANDLE *pChannelHandle)
{
    
    mt_s32 ret;
    MT_HANDLE channelId = MT_INVALID_HANDLE;    
    MT_UNF_DMX_CHAN_ATTR_S   stChanAttr= { 0 };

    MTADP_DEMUX_FUNCTION_ENTER();

    MTADP_DEMUX_INFO_PRINT("open Channel set pid : 0x%x \n", pid);
    
    memset(&stChanAttr, 0x0, sizeof(MT_UNF_DMX_CHAN_ATTR_S));
    stChanAttr.enChannelType = MT_UNF_DMX_CHAN_TYPE_SEC;
    stChanAttr.enCRCMode = MT_UNF_DMX_CHAN_CRC_MODE_FORBID;
    stChanAttr.enOutputMode = MT_UNF_DMX_CHAN_OUTPUT_MODE_PLAY;
    stChanAttr.u32BufSize = DEMUX_SECITON_BUF_SIZE;
    ret = MT_UNF_DMX_CreateChannel(u32DmxId, &stChanAttr, &channelId);
    if(ret != SUCCESS)
    {
      MTADP_DEMUX_ERR_PRINT("MT_UNF_DMX_CreateChannel err \n");
      return ret;
    }
    ret = MT_UNF_DMX_SetChannelPID(channelId, pid);
    if(ret != SUCCESS)
    {
      MTADP_DEMUX_ERR_PRINT("MT_UNF_DMX_SetChannelPID err \n");
      return ret;
    }

    *pChannelHandle = channelId;


    MTADP_DEMUX_INFO_PRINT("ChannelID: 0x%x \n", *pChannelHandle);
    
    MTADP_DEMUX_FUNCTION_EXIT();
    return MT_SUCCESS;
    
}


MT_S32 MTADP_DEMUX_CloseChannel(MT_HANDLE channelHandle)
{
    mt_s32 ret;
    
    ret = MT_UNF_DMX_CloseChannel(channelHandle);
    if(ret != 0)
    {
        MTADP_DEMUX_ERR_PRINT("err");
         return ret;
    }

    ret = MT_UNF_DMX_DestroyChannel(channelHandle);
    if(ret != 0)
    {
        MTADP_DEMUX_ERR_PRINT("err");
        return ret;
    }

    return MT_SUCCESS;
}

MT_S32 MTADP_DEMUX_StartChannel(MT_HANDLE channelHandle)
{
  mt_s32 ret;  
  MTADP_DEMUX_FUNCTION_ENTER();
  
  if(channelHandle == MT_INVALID_HANDLE)
    return MT_FAILURE;

  ret = MT_UNF_DMX_OpenChannel(channelHandle);
    if(ret != 0)
    {
      MTADP_DEMUX_ERR_PRINT("err");
      return ret;
    }
    MTADP_DEMUX_FUNCTION_EXIT();

    return MT_SUCCESS;
}

MT_S32 MTADP_DEMUX_StopChannel(MT_HANDLE channelHandle)
{
   mt_s32 ret;

   MTADP_DEMUX_FUNCTION_ENTER();
   
   if(channelHandle == MT_INVALID_HANDLE)
   return MT_FAILURE;


   ret = MT_UNF_DMX_CloseChannel(channelHandle);

   if(ret != 0)
    {
      MTADP_DEMUX_ERR_PRINT("err");
      return ret;
    }
   MTADP_DEMUX_FUNCTION_EXIT();

    return MT_SUCCESS;
}


MT_S32 MTADP_DEMUX_SetFilter(mt_u32 u32DmxId, MT_HANDLE channelHandle, DEMUX_FILTER_S *pFilterInfo, MT_HANDLE *pFilterHandle)
{
    MT_HANDLE filter_handle = MT_INVALID_HANDLE;
    mt_u8 index;
    mt_s32 ret;
    MT_UNF_DMX_FILTER_ATTR_S fiterAttr;

    MTADP_DEMUX_FUNCTION_ENTER();

    memset(&fiterAttr, 0x0, sizeof(MT_UNF_DMX_FILTER_ATTR_S));
    memset(fiterAttr.au8Mask, 0xff, DMX_FILTER_MAX_DEPTH);
	
    fiterAttr.u32FilterDepth = pFilterInfo->u16FilterDepth;
    memcpy(fiterAttr.au8Match, pFilterInfo->u8Match, fiterAttr.u32FilterDepth);
    for(index = 0; index < pFilterInfo->u16FilterDepth; index++)
    {
        fiterAttr.au8Mask[index] = ~(pFilterInfo->u8Mask[index]);
    }
    

     ret = MT_UNF_DMX_CreateFilter(u32DmxId, &fiterAttr, &filter_handle);
     if(ret != 0)
     {
         MTADP_DEMUX_ERR_PRINT("MT_UNF_DMX_CreateFilter err \n");
        return ret;
     }
     ret = MT_UNF_DMX_AttachFilter(filter_handle, channelHandle);
     if(ret != 0)
     {
        MTADP_DEMUX_ERR_PRINT("MT_UNF_DMX_AttachFilter err \n"); 
		MT_UNF_DMX_DestroyFilter(filter_handle);
        return ret;
     }
    
    MTADP_DEMUX_INFO_PRINT("channelHandle: 0x%x fileterHandle: 0x%x\n",channelHandle, filter_handle);    
    
    ret = MT_UNF_DMX_SetFilterAttr(filter_handle, &fiterAttr);
    if(ret != 0)
    {
        MTADP_DEMUX_ERR_PRINT("MT_UNF_DMX_SetFilterAttr err \n");
        return ret;
    }    
    *pFilterHandle = filter_handle;

    MTADP_DEMUX_FUNCTION_EXIT();

    return MT_SUCCESS;
}

MT_S32 MTADP_DEMUX_CloseFilter(MT_HANDLE channelHandle, MT_HANDLE filterHandle)
{
    mt_s32 ret;
    MTADP_DEMUX_FUNCTION_ENTER();
    
    ret = MT_UNF_DMX_DetachFilter(filterHandle, channelHandle);
    if(ret != MT_SUCCESS)
    {
       MTADP_DEMUX_ERR_PRINT("MT_UNF_DMX_DetachFilter err \n");
       return ret;
    }
    
    ret = MT_UNF_DMX_DestroyFilter(filterHandle);
    if(ret != 0)
      {
      MTADP_DEMUX_ERR_PRINT("MT_UNF_DMX_DestroyFilter err \n");
      return ret;
    }
    MTADP_DEMUX_FUNCTION_EXIT();

    return MT_SUCCESS;
}



static mt_void SocketThread(mt_void *args)
{
    mt_s32 SocketFd;
    struct sockaddr_in ServerAddr;
    in_addr_t IpAddr;
    struct ip_mreq Mreq;
    mt_u32 AddrLen;
    ulong u32Handle;

    MT_UNF_STREAM_BUF_S StreamBuf;
    mt_u32 ReadLen;
    mt_u32 GetBufCount  = 0;
    mt_u32 ReceiveCount = 0;
    mt_s32 Ret;

    u32Handle = (ulong) args;
    if (MAX_IP_THREAD_CNT <= u32Handle)
    {
        printf("\n Socket Thread: invalid args");
        return;
    }

    SocketFd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (SocketFd < 0)
    {
        printf("create socket error [%d].\n", errno);
        return;
    }

    ServerAddr.sin_family = AF_INET;
    ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    ServerAddr.sin_port = htons(IP_Play_Handles[u32Handle].u32UdpPort);

    if (bind(SocketFd, (struct sockaddr *)(&ServerAddr), sizeof(struct sockaddr_in)) < 0)
    {
        printf("socket bind error [%d].\n", errno);
        close(SocketFd);
        return;
    }

    IpAddr = inet_addr(IP_Play_Handles[u32Handle].cMultiIPAddr);

    printf("\n SocketThread %s 0x%x 0x%x 0x%x\n", IP_Play_Handles[u32Handle].cMultiIPAddr,
           IP_Play_Handles[u32Handle].u32UdpPort, IpAddr, ServerAddr.sin_port);

    if (IpAddr)
    {
        Mreq.imr_multiaddr.s_addr = IpAddr;
        Mreq.imr_interface.s_addr = htonl(INADDR_ANY);
        if (setsockopt(SocketFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, &Mreq, sizeof(struct ip_mreq)))
        {
            printf("Socket setsockopt ADD_MEMBERSHIP error [%d].\n", errno);
            close(SocketFd);
            return;
        }
    }

    AddrLen = sizeof(ServerAddr);

    while (!IP_Play_Handles[u32Handle].bStopSocketThread)
    {
        Ret = MT_UNF_DMX_GetTSBuffer(g_PortTsBuf[IP_Play_Handles[u32Handle].u32PortID], 188 * 50, &StreamBuf,0);
        if (Ret != MT_SUCCESS)
        {
            GetBufCount++;
            if (GetBufCount >= 10)
            {
                printf("########## TS come too fast! #########, Ret=%d\n", Ret);
                GetBufCount = 0;
            }

            MT_USLEEP(10000);
            continue;
        }

        GetBufCount = 0;

        ReadLen = recvfrom(SocketFd, StreamBuf.pu8Data, 1316, 0,
                           (struct sockaddr *)&ServerAddr, &AddrLen);
        if (ReadLen <= 0)
        {
            ReceiveCount++;
            if (ReceiveCount >= 50)
            {
                printf("########## TS come too slow or net error! #########\n");
                ReceiveCount = 0;
            }
        }
        else
        {
            ReceiveCount = 0;
            Ret = MT_UNF_DMX_PutTSBuffer(g_PortTsBuf[IP_Play_Handles[u32Handle].u32PortID], ReadLen);
            if (Ret != MT_SUCCESS)
            {
                printf("call MT_UNF_DMX_PutTSBuffer failed.\n");
            }
        }
    }

    close(SocketFd);
    return;
}

static mt_void FileTsTthread(mt_void *args)
{
    MT_UNF_STREAM_BUF_S StreamBuf;
    mt_u32 Readlen;
    mt_s32 Ret;
    ulong u32Handle = 0;

    u32Handle = (ulong)args;
    if (MAX_FILE_THREAD_CNT <= u32Handle)
    {
        printf("\n File TS Thread: invalid args");
        return;
    }

    while (!FILE_Play_Handles[u32Handle].bStopFileThread)
    {
        Ret = MT_UNF_DMX_GetTSBuffer(g_PortTsBuf[FILE_Play_Handles[u32Handle].u32PortID], 188 * 50, &StreamBuf,0);
        if (Ret != MT_SUCCESS)
        {
            MT_USLEEP(10*1000);
            continue;
        }

        Readlen = fread((void *)StreamBuf.pu8Data, sizeof(mt_s8), 0x1000, FILE_Play_Handles[u32Handle].g_pTsFile);
        if (Readlen <= 0)
        {
            printf("read ts file error!\n");
            rewind(FILE_Play_Handles[u32Handle].g_pTsFile);
            continue;
        }

        Ret = MT_UNF_DMX_PutTSBuffer(g_PortTsBuf[FILE_Play_Handles[u32Handle].u32PortID], Readlen);
        if (Ret != MT_SUCCESS)
        {
            printf("call MT_UNF_DMX_PutTSBuffer failed.\n");
        }
    }

    return;
}


static mt_void FileEsTthread(mt_void *args)
{
    MT_UNF_STREAM_BUF_S StreamBuf;
    mt_u32 Readlen;
    mt_s32 Ret;
    ulong u32Handle = 0;
    MT_UNF_AVPLAY_BUFID_E bufid = MT_UNF_AVPLAY_BUF_ID_ES_AUD;

    u32Handle = (ulong)args;
    if (MAX_FILE_THREAD_CNT <= u32Handle)
    {
        printf("\n File TS Thread: invalid args");
        return;
    }

    if(Es_FILE_Play_Handles[u32Handle].type == 0)
        bufid = MT_UNF_AVPLAY_BUF_ID_ES_AUD;
    else if(Es_FILE_Play_Handles[u32Handle].type == 1)
        bufid = MT_UNF_AVPLAY_BUF_ID_ES_VID;

    while (!Es_FILE_Play_Handles[u32Handle].bStopFileThread)
    {
        Ret = MT_UNF_AVPLAY_GetBuf(Es_FILE_Play_Handles[u32Handle].havplay , bufid , 0x4000, &StreamBuf,0);
        if (Ret != MT_SUCCESS)
        {
            MT_USLEEP(10*1000);
            continue;
        }

        Readlen = fread((void *)StreamBuf.pu8Data, sizeof(mt_s8), 0x4000, Es_FILE_Play_Handles[u32Handle].g_pEsFile);
        if (Readlen <= 0)
        {
            printf("read ts file error!\n");
            rewind(Es_FILE_Play_Handles[u32Handle].g_pEsFile);
            continue;
        }

        Ret = MT_UNF_AVPLAY_PutBuf(Es_FILE_Play_Handles[u32Handle].havplay , bufid , Readlen , 0);
        if (Ret != MT_SUCCESS)
        {
            printf("call MT_UNF_DMX_PutTSBuffer failed.\n");
        }
    }

    return;
}

/*****************************************************************************
* Function:      AvStartAvPlay
* Description:   start AV play
* Data Accessed:
* Data Updated:
* Input:             mt_u16 videopid, mt_u16 audiopid, mt_u16 pcrpid, mt_u16 telpid, mt_u16 subpid
* Output:           None
* Return:          None
* Others: None
*****************************************************************************/
mt_s32  DMX_IPStartInject(mt_char *pMultiAddr, mt_u32 u32UdpPort, mt_u32 u32PortID)
{
    ulong i;

    if (MT_TRUE == bIPInjectFlag[u32PortID])
    {
        DMX_IPStopInject(u32PortID);
    }

    for (i = 0; i < MAX_IP_THREAD_CNT; i++)
    {
        if (MT_FALSE == IP_Play_Handles[i].bUsed)
        {
            break;
        }
    }

    if (MAX_IP_THREAD_CNT == i)
    {
        return MT_FAILURE;
    }

    strcpy(IP_Play_Handles[i].cMultiIPAddr, pMultiAddr);
    IP_Play_Handles[i].u32UdpPort = u32UdpPort;
    IP_Play_Handles[i].u32PortID = u32PortID;
    IP_Play_Handles[i].bStopSocketThread = MT_FALSE;
    pthread_create(&IP_Play_Handles[i].g_SocketThd, MT_NULL, (mt_void *)SocketThread, (mt_void *)i);
    IP_Play_Handles[i].bUsed = MT_TRUE;

    bIPInjectFlag[u32PortID] = MT_TRUE;

    return MT_SUCCESS;
}

/*****************************************************************************
* Function:      AvStartAvPlay
* Description:   stop AV play
* Data Accessed:
* Data Updated:
* Input:             mt_u16 videopid, mt_u16 audiopid, mt_u16 pcrpid, mt_u16 telpid, mt_u16 subpid
* Output:           None
* Return:          None
* Others: None
*****************************************************************************/
mt_s32  DMX_IPStopInject(mt_u32 u32PortID)
{
    ulong i;

    if ((u32PortID < DMX_DVBPORT_COUNT) || (u32PortID >= DMX_PORT_COUNT))
    {
        return MT_FAILURE;
    }

    if (MT_FALSE == bIPInjectFlag[u32PortID])
    {
        return MT_SUCCESS;
    }

    for (i = 0; i < MAX_IP_THREAD_CNT; i++)
    {
        if ((u32PortID == IP_Play_Handles[i].u32PortID) && (MT_TRUE == IP_Play_Handles[i].bUsed))
        {
            break;
        }
    }

    if (MAX_IP_THREAD_CNT == i)
    {
        return MT_FAILURE;
    }

    IP_Play_Handles[i].bStopSocketThread = MT_TRUE;
    pthread_join(IP_Play_Handles[i].g_SocketThd, MT_NULL);

    IP_Play_Handles[i].bUsed = MT_FALSE;

    bIPInjectFlag[u32PortID] = MT_FALSE;

    return MT_SUCCESS;
}

mt_s32  DMX_FileStartInject(mt_char *path, mt_u32 u32PortID) //flag = 0 TS:flag =1 :ES
{
    ulong i, s32Ret;
    FILE *pTsFile;

    if (MT_TRUE == bFileInjectFlag[u32PortID])
    {
        DMX_FileStopInject(u32PortID);
    }

    for (i = 0; i < MAX_FILE_THREAD_CNT; i++)
    {
        if (MT_FALSE == FILE_Play_Handles[i].bUsed)
        {
            break;
        }
    }

    if (MAX_IP_THREAD_CNT == i)
    {
        return MT_FAILURE;
    }

    pTsFile = fopen(path, "rb");
    if (!pTsFile)
    {
        printf("open file %s error!\n", path);
        return MT_FAILURE;
    }

    FILE_Play_Handles[i].g_pTsFile = pTsFile;
    FILE_Play_Handles[i].u32PortID = u32PortID;
    FILE_Play_Handles[i].bStopFileThread = MT_FALSE;
    s32Ret = pthread_create(&FILE_Play_Handles[i].g_FileThd, MT_NULL, (mt_void *)FileTsTthread, (mt_void *)i);
    if (s32Ret != MT_SUCCESS)
    {
        fclose(pTsFile);
        return MT_FAILURE;
    }

    FILE_Play_Handles[i].bUsed = MT_TRUE;
    bFileInjectFlag[u32PortID] = MT_TRUE;

    return MT_SUCCESS;
}


mt_s32  DMX_FileStopInject(mt_u32 u32PortID)
{
    //MT_UNF_AVPLAY_STOP_OPT_S stAVStop;
    mt_s32 i;

    if (MT_FALSE == bFileInjectFlag[u32PortID])
    {
        return MT_SUCCESS;
    }

    for (i = 0; i < MAX_FILE_THREAD_CNT; i++)
    {
        if ((MT_TRUE == FILE_Play_Handles[i].bUsed) && (u32PortID == FILE_Play_Handles[i].u32PortID))
        {
            break;
        }
    }

    FILE_Play_Handles[i].bStopFileThread = MT_TRUE;
    pthread_join(FILE_Play_Handles[i].g_FileThd, MT_NULL);

    fclose(FILE_Play_Handles[i].g_pTsFile);
    FILE_Play_Handles[i].bUsed = MT_FALSE;

    //stAVStop.enMode = MT_UNF_AVPLAY_STOP_MODE_BLACK;
    //stAVStop.u32TimeoutMs = 0;

    bFileInjectFlag[u32PortID] = MT_FALSE;

    return MT_SUCCESS;
}


mt_s32  DMX_EsFileStartInject(mt_char *path, mt_handle avplay, mt_u32 type) //flag = 0 TS:flag =1 :ES
{
    ulong i, s32Ret;
    FILE *pEsFile;

    for (i = 0; i < MAX_FILE_THREAD_CNT; i++)
    {
        if (MT_TRUE == Es_FILE_Play_Handles[i].bUsed && Es_FILE_Play_Handles[i].havplay == avplay)
        {
            DMX_EsFileStopInject(i);
            break;
        }
    }

    for (i = 0; i < MAX_FILE_THREAD_CNT; i++)
    {
        if (MT_FALSE == Es_FILE_Play_Handles[i].bUsed)
        {
            break;
        }
    }

    if (MAX_IP_THREAD_CNT == i)
    {
        return MT_FAILURE;
    }

    pEsFile = fopen(path, "rb");
    if (!pEsFile)
    {
        printf("open file %s error!\n", path);
        return MT_FAILURE;
    }

    Es_FILE_Play_Handles[i].g_pEsFile = pEsFile;
    Es_FILE_Play_Handles[i].havplay = avplay;
    Es_FILE_Play_Handles[i].bStopFileThread = MT_FALSE;
    Es_FILE_Play_Handles[i].type = type;
    s32Ret = pthread_create(&Es_FILE_Play_Handles[i].g_FileThd, MT_NULL, (mt_void *)FileEsTthread, (mt_void *)i);
    if (s32Ret != MT_SUCCESS)
    {
        fclose(pEsFile);
        return MT_FAILURE;
    }

    Es_FILE_Play_Handles[i].bUsed = MT_TRUE;
    return MT_SUCCESS;
}


mt_s32  DMX_EsFileStopInject(mt_u32 index)
{

    Es_FILE_Play_Handles[index].bStopFileThread = MT_TRUE;
    pthread_join(Es_FILE_Play_Handles[index].g_FileThd, MT_NULL);

    fclose(Es_FILE_Play_Handles[index].g_pEsFile);
    Es_FILE_Play_Handles[index].bUsed = MT_FALSE;

    return MT_SUCCESS;
}
#if 0
mt_s32 MTADP_Demux_Init(mt_u32 DmxPortID,mt_u32 TsPortID)
{
    mt_s32 Ret;
    Ret = MT_UNF_DMX_Init();
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_DMX_Init failed.\n");
        return Ret;
    }

    Ret = MT_UNF_DMX_AttachTSPort(DmxPortID, TsPortID);
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        MT_UNF_DMX_DeInit();
        return Ret;
    }

    return MT_SUCCESS;
}

mt_s32 MTADP_Demux_DeInit(mt_u32 DmxPortID)
{
    mt_s32 Ret;

    Ret = MT_UNF_DMX_DetachTSPort(DmxPortID);
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_DMX_DetachTSPort failed.\n");
        return Ret;
    }

    Ret = MT_UNF_DMX_DeInit();
    if (Ret != MT_SUCCESS)
    {
        printf("call MT_UNF_DMX_AttachTSPort failed.\n");
        return Ret;
    }

    return MT_SUCCESS;
}
#endif
