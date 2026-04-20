/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mount.h>
#include <fcntl.h>

#include "mt_type.h"
#include "mt_module_debug.h"
#include "mt_pvr_fifo.h"
#include "mt_adpt_thread.h"
#include "mt_unf_pvr.h"
#include "mt_pvr_rec_ctrl.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

static MT_U32    loop_event_id = 0xffffffff;
#ifndef MT_S64
#define MT_S64 mt_s64
#endif

#define PACKET_IDENTIFIER(a, b)         ((MT_U32)((((a) & 0x1f) << 8) | ((b) & 0xff)))

#define PVR_INVALID_DEFAULT_FILE_EVENT_SIZE          (0xffffffffffffffffLLU)
#define PVR_DEFAULT_FILE_NODE_SIZE                           (2000LLU*1024*1024)

#define PVR_FILE_MAX_EVENT                   128
#define PVR_FILE_MAX_EVENT_NODE        512
#define ONE_M_BYTES (1024*1024)
/** */
typedef enum mtPVR_FILE_NAME_FMT_E
{
    PVR_FILE_NAME_FMT_HISI,
    PVR_FILE_NAME_FMT_XX_DOT_TS,
    PVR_FILE_NAME_FMT_XX_DOT_ENC_DOT_TS,
    PVR_FILE_NAME_FMT_XX_DOT_AD_DOT_TS,

    PVR_FILE_NAME_FMT_BUTT
} PVR_FILE_NAME_FMT_E;


typedef struct mtPVR_EventDataNode_S
{
    MT_U32    u32NodeCreateTime;
    MT_U32    u32NodeStartFrame;
}PVR_EventDataNode_S;

typedef struct mtPVR_CELLMAP_S
{
    MT_U64    u64head;      //cell start.u64.offset, from 0
    MT_U64    u64Tail;
    MT_U64    u64size;      //cell size
    MT_CHAR cellname[PVR_MAX_FILENAME_LEN]; //name without path. forexample: event1
    MT_U32    u32StartTime;
    MT_BOOL  bSaved;
    MT_BOOL  bShare;
    PVR_EventDataNode_S    u32EventNode[PVR_FILE_MAX_EVENT_NODE];
    MT_U32    firstNode;
    MT_U32    coverLastEventId;
}PVR_CELLMAP_S;
typedef struct mtPVR_EVENTIDX_S
{
    MT_U32  flag_wrewind;       //if write-rewind,reopen fs-handle
    MT_U32  flag_shared;        //shared flag
    MT_U32  normalnum;          //normal number for tsfile'name
    MT_U32  ridpos;             //cellmap rid pos
    MT_U32  widpos;             //cellmap wid pos
    MT_U32  celllen;            //cellmap mem-len
    PVR_CELLMAP_S   *cell;      //cellmap mem
    MT_U32  u32StartTime;
    PVR_CELLMAP_S  currentRunningEventCell;
}PVR_EVENTIDX_S;

typedef struct mtPVR_FILE_S
{
    MT_BOOL bOpened;
    int     systemFd;           //true file-fd
    int     openMode;

    MT_U64  u64CycMaxSize;      //max-file-storage-size

    MT_U64  u64FileNodeSize;    //current file size    -->current subevent size==PVR_DEFAULT_FILE_NODE_SIZE
    MT_U64  u64StartOfCurFd;    //current file startpos-->current subevent startpos
    MT_U64  u64SeekOffset;      //current all-writed size
    MT_U64  u64MaxOffset;       //max read pos,for pagecache clear
    MT_U64  u64LastWPtrEnd;
    MT_CHAR szFilePath[PVR_MAX_FILENAME_LEN];
    MT_CHAR szFileName[PVR_MAX_FILENAME_LEN];
    PVR_FILE_NAME_FMT_E enFileNameFmt;

    PVR_EVENTIDX_S *eventidx;   //event idx, be shared to rw
} PVR_FILE_S;

static PVR_FILE_S g_stPvrFiles[PVR_FILE_MAX_FILE];
static MT_U32  g_timeShiftEventLoopTimeInMs = 2*60*60*1000;
static MT_U32  g_timeShiftSingleEventLoopStartFrame = 0;
static MT_U32  g_FileSizeInMbytes = PVR_DEFAULT_FILE_NODE_SIZE/ONE_M_BYTES;

#define PVRFileGetNodeIdx(offset, nodeSize)  ((offset)/nodeSize)
#define PVRFileGetNodeOffset(offset, nodeSize)  ((offset)%nodeSize)
#define PVRFileOffsetMatchNode(offset, nodeStartOffset, nodeSize) ((offset >= nodeStartOffset) && (offset < (nodeStartOffset + nodeSize)) )

#define PVRFileGetRealFNameHisi(fileName, realName, nodeIdx) do {\
        if (0 == nodeIdx)\
        {\
            snprintf(realName, PVR_MAX_FILENAME_LEN,"%s", fileName); \
        }\
        else\
        {\
            snprintf(realName, PVR_MAX_FILENAME_LEN,"%s.%04d", fileName, nodeIdx); \
        }\
    }while(0)

#define PVRFileGetRealFNameXxTs(fileName, realName, nodeIdx) do {\
        snprintf(realName,PVR_MAX_FILENAME_LEN, "%s%02d.ts", fileName, nodeIdx); \
    }while(0)

#define PVRFileGetRealFNameXxEncTs(fileName, realName, nodeIdx) do {\
        snprintf(realName, PVR_MAX_FILENAME_LEN,"%s%02d.enc.ts", fileName, nodeIdx); \
    }while(0)

#define PVRFileGetRealFNameXxAdTs(fileName, realName, nodeIdx) do {\
        snprintf(realName, PVR_MAX_FILENAME_LEN,"%s%02d.ad.ts", fileName, nodeIdx); \
    }while(0)


static MT_U32 PVR_Get_File_Name_Pos(MT_CHAR *filename)
{
    MT_U32 lens=0;
    if(NULL==filename){
        return 0;
    }
    lens=strlen(filename);
    while(lens){
        if(filename[lens]=='/'){
            lens++;
            break;
        }
        lens--;
    }
    return lens;
}

static PVR_CELLMAP_S* PVR_BigEx_Cell_Space(PVR_CELLMAP_S *cell,MT_U32 oldlen,MT_U32 newlen)
{
    PVR_CELLMAP_S *oldc=cell;
    PVR_CELLMAP_S *newc=NULL;

    if(newlen<=oldlen){
        return NULL;
    }
    newc=(PVR_CELLMAP_S*)malloc(newlen);
    if(NULL==newc){
        return NULL;
    }
    memset(newc,0x00,newlen);
    memcpy(newc,oldc,oldlen);
    free(oldc);
    return newc;
}


static MT_S32 PVR_Remove_Timeout_Event(PVR_FILE_S *pPvrFile)
{
    int i=0;
    MT_U32  current_time=0;
    (void)MT_PVR_SysGetTimeStampMs(&current_time);

    if(pPvrFile->eventidx)     //free mapfile
    {          
        if(pPvrFile->eventidx->cell)
        {
            PVR_CELLMAP_S   *tmp_cell = pPvrFile->eventidx->cell;
            PVR_EVENT_D("pvrfiles->eventidx->cell->cellname=%s, pvrfiles->eventidx->widpos=%d\n",tmp_cell->cellname,pPvrFile->eventidx->widpos);
            for(i=0; i<pPvrFile->eventidx->widpos;i++)
            {
                PVR_EVENT_D("pvrfiles->eventidx->cell->cellname=%s, tmp_cell->bSaved=%d,  startTime=%d  current_time = %d\n",
                        tmp_cell->cellname,tmp_cell->bSaved,tmp_cell->u32StartTime,  current_time );
                if((tmp_cell->bSaved == MT_FALSE)  
                    && (tmp_cell->u32StartTime > 0)
                    && (current_time > ((MT_U32)g_timeShiftEventLoopTimeInMs))
                    && ((current_time - tmp_cell->u32StartTime)  >  (MT_U32)g_timeShiftEventLoopTimeInMs))
                {
                    MT_CHAR  fNameEvent[PVR_MAX_FILENAME_LEN];
                    memset(fNameEvent, 0, PVR_MAX_FILENAME_LEN);
                    strcpy(fNameEvent, pPvrFile->szFilePath);
                    strcat(fNameEvent, tmp_cell->cellname);
                    PVR_EVENT_D("remove >>>  tmp_cell->cellname=%s\n",fNameEvent);
                    if(/*tmp_cell->cellname && */PVR_CHECK_FILE_EXIST(fNameEvent)) // cellname always not null, not need to check
                    {
                          MT_CHAR  tmp[PVR_MAX_FILENAME_LEN*2];

                          memset(tmp, 0, sizeof(tmp));
                          sprintf(tmp, "%s%s", fNameEvent,".attr");
                          if(PVR_CHECK_FILE_EXIST(tmp))     (MT_VOID)remove(tmp);

                          memset(tmp, 0, sizeof(tmp));
                          sprintf(tmp, "%s%s", fNameEvent, ".ca");
                          if(PVR_CHECK_FILE_EXIST(tmp))     (MT_VOID)remove(tmp);

                          memset(tmp, 0, sizeof(tmp));
                          sprintf(tmp, "%s%s", fNameEvent, ".idx");
                          if(PVR_CHECK_FILE_EXIST(tmp))     (MT_VOID)remove(tmp);

                          (MT_VOID)remove(fNameEvent);
                          PVR_EVENT_D("remove <<<  tmp_cell->cellname=%s\n",fNameEvent);
                    }
                    tmp_cell->u32StartTime = 0;
                }
                tmp_cell ++;
            }
        }
    }
    return 0;
}


static PVR_FILE_NAME_FMT_E PVR_Check_FileType(const MT_CHAR *filename)
{
    MT_CHAR  fNameReal[PVR_MAX_FILENAME_LEN + 5];

    //lint -e506 -e774
    PVRFileGetRealFNameHisi(filename, fNameReal, 0);
    //lint +e506 +e774
    if (PVR_CHECK_FILE_EXIST(fNameReal))
    {
        return PVR_FILE_NAME_FMT_HISI;
    }

    PVRFileGetRealFNameXxTs(filename, fNameReal, 0);
    if (PVR_CHECK_FILE_EXIST(fNameReal))
    {
        return PVR_FILE_NAME_FMT_XX_DOT_TS;
    }

    PVRFileGetRealFNameXxEncTs(filename, fNameReal, 0);
    if (PVR_CHECK_FILE_EXIST(fNameReal))
    {
        return PVR_FILE_NAME_FMT_XX_DOT_ENC_DOT_TS;
    }

    PVRFileGetRealFNameXxAdTs(filename, fNameReal, 0);
    if (PVR_CHECK_FILE_EXIST(fNameReal))
    {
        return PVR_FILE_NAME_FMT_XX_DOT_AD_DOT_TS;
    }

    return PVR_FILE_NAME_FMT_BUTT;
}


/*
gongfu00.ts
gongfu01.ts
gongfu02.ts......
-------------
cctv00.enc.ts
cctv01.enc.ts
cctv02.enc.ts
*/
static MT_VOID PVRFileGetRealFName(const MT_CHAR *pFileName,  MT_CHAR *pRealName, MT_S32 nodeIdx)
{
    PVR_FILE_NAME_FMT_E fileFmt;

    fileFmt = PVR_Check_FileType(pFileName);

    switch (fileFmt)
    {
        case PVR_FILE_NAME_FMT_HISI:
            PVRFileGetRealFNameHisi(pFileName, pRealName, nodeIdx);
            break;
        case PVR_FILE_NAME_FMT_XX_DOT_TS:
            PVRFileGetRealFNameXxTs(pFileName, pRealName, nodeIdx);
            break;
        case PVR_FILE_NAME_FMT_XX_DOT_ENC_DOT_TS:
            PVRFileGetRealFNameXxEncTs(pFileName, pRealName, nodeIdx);
            break;
        case PVR_FILE_NAME_FMT_XX_DOT_AD_DOT_TS:
            PVRFileGetRealFNameXxAdTs(pFileName, pRealName, nodeIdx);
            break;
        default:
            PVRFileGetRealFNameHisi(pFileName, pRealName, nodeIdx);
    }

    return;
}


ssize_t PVR_READALL(void *buf, size_t size, PVR_FILE fd,  off_t offset)
{
    ssize_t nread = 0, n;

    if (0 == size)
    {
        return 0;
    }

    do {
        if ((n = (ssize_t)PVR_READ((void*)(&((char *)buf)[nread]), ((size) - (size_t)nread), fd, offset + (off_t)nread)) == (-1))
        {
            //lint -e774
            if (NULL != &errno)
            {
                if (EINTR == errno)
                {
                    MT_WARN_PVR("read err @EINTR\n");
                    continue;
                }
                else
                {
                    return -1;
                }
            }
            //lint +e774
        }

        if (0 == n) /* EOF */
        {
            return (ssize_t)nread;
        }
        nread += n;
        if (nread < (ssize_t)size)
        {
            MT_WARN_PVR("read ok @ less\n");
        }
    }while(nread < (ssize_t)size);

    return (ssize_t)nread;
}

ssize_t PVR_WRITEALL(const void *buf, size_t len, PVR_FILE fd, off_t offset)
{
    ssize_t n, sizeWriten = 0;

    do {

#ifdef PVR_CHECK_USB_FILE
        if (MT_SUCCESS != PVR_CheckUsbStillPlugIn())
        {
            return -1;
        }
#endif
        n = PVR_WRITE( &((const char *)buf)[sizeWriten],
                len - (size_t)sizeWriten, fd, offset + (off_t)sizeWriten);
        if (-1 == n)
        {
            //lint -e774
            if (NULL != &errno)
            {
                if (EINTR == errno)
                {
                    continue;
                }
                else
                {
                    return -1;
                }
            }
            //lint +e774
        }

        sizeWriten += n;
    }while((size_t)sizeWriten < len);

    return sizeWriten;
}

/*****************************************************************************
 Prototype       : PVR_CHECK_FILE_EXIST
 Description     : check file exist
 Input           : pszFileName  **
 Output          : None
 Return Value    :
 Global Variable
    Read Only    :
    Read & Write :
  History
  1.Date         : 2008/4/26
    Author       : q46153
    Modification : Created function

*****************************************************************************/
MT_BOOL PVR_CHECK_FILE_EXIST(const MT_CHAR *pszFileName)
{
    MT_S32 ret;
    ret = pvr_access(pszFileName, F_OK);
    if (ret == 0)
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
}

MT_BOOL PVR_CHECK_FILE_EXIST64(const MT_CHAR *pszFileName)
{
    MT_CHAR  fNameReal[PVR_MAX_FILENAME_LEN + 5];

    MT_S32 ret;

    PVRFileGetRealFName(pszFileName, fNameReal, 0);

    ret = pvr_access(fNameReal, F_OK);
    if (ret == 0)
    {
        return MT_TRUE;
    }
    else
    {
        return MT_FALSE;
    }
}


MT_VOID PVR_REMOVE_FILE64(const MT_CHAR *pszFileName)
{
    int nodeIdx = 0;

    MT_CHAR  fNameReal[PVR_MAX_FILENAME_LEN + 5];

    PVRFileGetRealFName(pszFileName, fNameReal, nodeIdx);
    while(PVR_CHECK_FILE_EXIST(fNameReal))
    {
        (MT_VOID)pvr_remove(fNameReal);

        nodeIdx++;
        PVRFileGetRealFName(pszFileName, fNameReal, nodeIdx);
    }
}

MT_U64 PVR_FILE_GetFileSize(const MT_CHAR *pszFileName)
{
    MT_S64 size = 0;
    PVR_FILE fd;

    fd = pvr_open(pszFileName, PVR_FOPEN_MODE_DATA_READ, 0777);
    if (PVR_FILE_INVALID_FILE != fd)
    {
        size = (MT_S64)pvr_lseek(fd, 0, SEEK_END);
		pvr_close(fd);
    }

    return (MT_U64)size;
}

MT_U32 PVR_Get_File_BlockNum(const MT_CHAR *pszFileName)
{
    int     nodeIdx = 0;
    MT_CHAR fNameReal[PVR_MAX_FILENAME_LEN + 5];

    PVRFileGetRealFName(pszFileName, fNameReal, nodeIdx);
    while(PVR_CHECK_FILE_EXIST(fNameReal))
    {
        nodeIdx++;
        PVRFileGetRealFName(pszFileName, fNameReal, nodeIdx);
    }

    return nodeIdx;
}

MT_U64 PVR_FILE_GetFileSize64(const MT_CHAR *pszFileName)
{
    MT_S64      size = 0;
    PVR_FILE    fd;
    int         nodeIdx = 0;
    MT_U64      nodeSize = 0;
    MT_CHAR  fNameReal[PVR_MAX_FILENAME_LEN + 5];

    PVRFileGetRealFName(pszFileName, fNameReal, nodeIdx);
    while(PVR_CHECK_FILE_EXIST(fNameReal))
    {
        nodeSize = PVR_FILE_GetFileSize(fNameReal);
        size += (MT_S64)nodeSize;
        nodeIdx++;
        PVRFileGetRealFName(pszFileName, fNameReal, nodeIdx);
    };

    nodeIdx--;
    PVRFileGetRealFName(pszFileName, fNameReal, nodeIdx);
    fd = pvr_open(fNameReal, PVR_FOPEN_MODE_DATA_READ, 0777);
    if (PVR_FILE_INVALID_FILE != fd)
    {
        size += (MT_S64)pvr_lseek(fd, 0, (int)SEEK_END);
        size -= (MT_S64)nodeSize;
        pvr_close(fd);
    }


    return (MT_U64)size;
}


static void pvr_free_eventidx(PVR_FILE_S *pvrfiles, MT_BOOL bTimeShiftEvent)
{
    int i=0;
    
    if(NULL==pvrfiles){
        return;
    }
    if(pvrfiles->eventidx){             //free mapfile
        if(pvrfiles->eventidx->cell){
            PVR_CELLMAP_S   *tmp_cell = &(pvrfiles->eventidx->cell[0]);
            PVR_EVENT_D("pvrfiles->eventidx->cell->cellname=%s, pvrfiles->eventidx->widpos=%d\n",tmp_cell->cellname,pvrfiles->eventidx->widpos);
            for(i=0; i<=pvrfiles->eventidx->widpos;i++)
            {
                PVR_EVENT_D("i=%d >>> pvrfiles->eventidx->cell->cellname=%s, tmp_cell->bSaved=%d, bTimeShiftEvent=%d\n",
                            i,tmp_cell->cellname,tmp_cell->bSaved,bTimeShiftEvent);
	         if((bTimeShiftEvent == MT_FALSE) && (i == 0))
	         {
                   tmp_cell ++;
	            continue;
	         }   
	         if(bTimeShiftEvent && (i==0) &&  tmp_cell->bSaved)
	         {
	               MT_CHAR  fNameEvent[PVR_MAX_FILENAME_LEN];
                      memset(fNameEvent, 0, PVR_MAX_FILENAME_LEN);
                      strcpy(fNameEvent, pvrfiles->szFilePath);
                      strcat(fNameEvent, tmp_cell->cellname);
                      //if(tmp_cell->cellname) // cellname always not null, not need to check
                      {
                          MT_CHAR  tmp[PVR_MAX_FILENAME_LEN*2];
                          MT_CHAR  tmp1[PVR_MAX_FILENAME_LEN*2];
                          memset(tmp1, 0, sizeof(tmp1));
                          snprintf(tmp1,sizeof(tmp1)-1, "%s%s", fNameEvent,".idx_event0");
                          if(PVR_CHECK_FILE_EXIST(tmp1))
                          {
                               memset(tmp, 0, sizeof(tmp));
                               snprintf(tmp,sizeof(tmp1)-1, "%s%s", fNameEvent,".idx");
                               if(PVR_CHECK_FILE_EXIST(tmp))     (MT_VOID)remove(tmp);        
                               (MT_VOID)rename(tmp1, tmp);
                          }    
                     }     
	         }
                if(tmp_cell->bSaved == MT_FALSE)
                {
                    MT_CHAR  fNameEvent[PVR_MAX_FILENAME_LEN];
                    memset(fNameEvent, 0, PVR_MAX_FILENAME_LEN);
                    strcpy(fNameEvent, pvrfiles->szFilePath);
                    strcat(fNameEvent, tmp_cell->cellname);
                    PVR_EVENT_D("remove >>>  tmp_cell->cellname=%s\n",fNameEvent);
                    //if(tmp_cell->cellname /*&& PVR_CHECK_FILE_EXIST(fNameEvent)*/) // cellname always not null, not need to check
                    {
                          MT_CHAR  tmp[PVR_MAX_FILENAME_LEN*2];
			              int j=0;
                          memset(tmp, 0, sizeof(tmp));
                          snprintf(tmp, sizeof(tmp)-1,"%s%s", fNameEvent,".attr");
                          if(PVR_CHECK_FILE_EXIST(tmp))
                          {
                                (MT_VOID)remove(tmp);         //delete  timeshift.ts.attr
                           }
                           else
                           {
                               if((i==0)  &&  (strstr(fNameEvent, ".ts")  != NULL))    //delete timeshift.attr  file
                               {
                                     memset(tmp, 0, sizeof(tmp));
                                     memcpy(tmp,  fNameEvent, (strlen(fNameEvent) - strlen(".ts")));
                                     strcat(tmp, ".attr");
                                     PVR_EVENT_D("remove timeshift.attr >>>  tmp=%s\n",tmp);
                                     if(PVR_CHECK_FILE_EXIST(tmp))     (MT_VOID)remove(tmp);
                               }
                          }
                          memset(tmp, 0, sizeof(tmp));
                          sprintf(tmp, "%s%s", fNameEvent, ".ca");
                          if(PVR_CHECK_FILE_EXIST(tmp))     (MT_VOID)remove(tmp);
                          
                          memset(tmp, 0, sizeof(tmp));
                          sprintf(tmp, "%s%s", fNameEvent, ".idx");
                          if(PVR_CHECK_FILE_EXIST(tmp))     (MT_VOID)remove(tmp);

                          (MT_VOID)remove(fNameEvent);
                          
                          if(tmp_cell->firstNode > 0)   j = tmp_cell->firstNode;
                          else j = 1;
                          
                          for(; j<PVR_REC_MAX_NODE; j++)
                          {
                              memset(tmp, 0 , sizeof(tmp));
                              snprintf(tmp, sizeof(tmp)-1,"%s.%04d", fNameEvent, j); 
                              PVR_EVENT_D("j = %d  >>> tmp=%s\n",j,tmp);
                              if(PVR_CHECK_FILE_EXIST(tmp)) 
                              {    
                               	       (MT_VOID)remove(tmp);
                              	       PVR_EVENT_D("remove <<<  0 nodeFile=%s\n",tmp);
                              }
      		                else
     		                {
     		                     break;
     		                }
                          }			  
                          PVR_EVENT_D("remove <<<  tmp_cell->cellname=%s\n",fNameEvent);
                    }
                }
                else
                {
                
                }
                tmp_cell ++;
            }
            free(pvrfiles->eventidx->cell);
            pvrfiles->eventidx->cell=NULL;
        }
        free(pvrfiles->eventidx);
     }
}



static PVR_EVENTIDX_S * pvr_init_eventidx(MT_S32 pvrFd, const MT_CHAR *filename)
{
    PVR_EVENT_D("pvr_init_eventidx >>>  fd = %d,  filename = %s\n",pvrFd, filename);
    PVR_EVENTIDX_S *eventidx=malloc(sizeof(PVR_EVENTIDX_S));
    memset(eventidx,0x00,sizeof(PVR_EVENTIDX_S));
    g_stPvrFiles[pvrFd].eventidx=eventidx;
    MT_U32 namepos=PVR_Get_File_Name_Pos((MT_CHAR *)filename);
    memcpy(g_stPvrFiles[pvrFd].szFilePath,filename,namepos);
    g_stPvrFiles[pvrFd].szFilePath[namepos]=0;
    g_stPvrFiles[pvrFd].eventidx->celllen = PVR_FILE_MAX_EVENT*sizeof(PVR_CELLMAP_S);
    g_stPvrFiles[pvrFd].eventidx->cell    = (PVR_CELLMAP_S*)malloc(g_stPvrFiles[pvrFd].eventidx->celllen);
    if(g_stPvrFiles[pvrFd].eventidx->cell == NULL)
    {
    	  MT_ERR_PVR("pvr_init_eventidx, Malloc failed !\n");
	  return NULL;
    }
    memset(g_stPvrFiles[pvrFd].eventidx->cell, 0, g_stPvrFiles[pvrFd].eventidx->celllen);
    g_stPvrFiles[pvrFd].eventidx->cell[0].u64head=0;
    g_stPvrFiles[pvrFd].eventidx->cell[0].u64size=PVR_INVALID_DEFAULT_FILE_EVENT_SIZE;     //default==max
    strcpy(g_stPvrFiles[pvrFd].eventidx->cell[0].cellname,&filename[namepos]);
    PVR_EVENT_D("+++pvr.first=%d,%s,%s\n",namepos,g_stPvrFiles[pvrFd].szFilePath,g_stPvrFiles[pvrFd].eventidx->cell[0].cellname);
    g_stPvrFiles[pvrFd].eventidx->ridpos   = 0;
    g_stPvrFiles[pvrFd].eventidx->widpos   = 0;
    g_stPvrFiles[pvrFd].eventidx->normalnum= 0;
    g_stPvrFiles[pvrFd].eventidx->flag_shared = 0;
    
    //g_stPvrFiles[pvrFd].eventidx->cell[0].bSaved = MT_TRUE;

    memcpy(&(g_stPvrFiles[pvrFd].eventidx->currentRunningEventCell), &(g_stPvrFiles[pvrFd].eventidx->cell[0]), sizeof(PVR_CELLMAP_S));
    return eventidx;
}


PVR_FILE64 PVR_OPEN64(const MT_CHAR *filename, int mode,  
                                        MT_BOOL bSupportTimeShiftEvent,
                                        MT_U32 u32TimeShiftEventDataUnitSize,
                                        MT_U32 u32TimeShiftEventLoopTimeInMs,
                                        MT_U32 nodeIdx)
{
    MT_S32 pvrFd;
    int sysFd;
    MT_U64   fileNodeSize;
    MT_CHAR  fNameOpen[PVR_MAX_FILENAME_LEN + 5];
    PVR_FILE_NAME_FMT_E fileFmt;

    if(u32TimeShiftEventLoopTimeInMs >= PVR_MIN_LOOP_REC_TIME_FOR_EVENT)
    {
        g_timeShiftEventLoopTimeInMs = u32TimeShiftEventLoopTimeInMs;
    }
    g_timeShiftSingleEventLoopStartFrame = 0;

    if (PVR_FOPEN_MODE_DATA_READ == mode)
    {
        if(nodeIdx > 0)
        {
            MT_CHAR  tmp[PVR_MAX_FILENAME_LEN];
            memset(tmp, 0 , PVR_MAX_FILENAME_LEN);
            snprintf(tmp, PVR_MAX_FILENAME_LEN,"%s.%04d", filename, nodeIdx); 
            fileFmt = PVR_Check_FileType(tmp);
            PVR_EVENT_D("PVR_OPEN64 >>> tmp=%s\n",tmp);

        }
        else
        {
            fileFmt = PVR_Check_FileType(filename);
            PVR_EVENT_D("PVR_OPEN64 >>> filename=%s\n",filename);           
        }
        if (PVR_FILE_NAME_FMT_BUTT == fileFmt)
        {
            MT_ERR_PVR("can NOT tell File '%s' format, open failed.\n", filename);
            return -1;
        }
    }
    else
    {
        fileFmt = PVR_FILE_NAME_FMT_HISI;
    }
    PVR_EVENT_D("open64 >>> mode=%d, fileFmt=%d, bSupportTimeShiftEvent=%d\n",mode,fileFmt, bSupportTimeShiftEvent);
    for (pvrFd = 0; pvrFd < PVR_FILE_MAX_FILE; pvrFd++)
    {
        if (MT_FALSE == g_stPvrFiles[pvrFd].bOpened)
        {
            memset(&g_stPvrFiles[pvrFd],0x00,sizeof(PVR_FILE_S));
            g_stPvrFiles[pvrFd].bOpened = MT_TRUE;
            break;
        }
    }

    if (pvrFd >= PVR_FILE_MAX_FILE)  /* NO fd */
    {
        MT_ERR_PVR("PVR file: can NOT open more file.\n");
        return -1;
    }

    PVRFileGetRealFName(filename, fNameOpen, nodeIdx);

    g_stPvrFiles[pvrFd].enFileNameFmt = fileFmt;

    if(PVR_FOPEN_MODE_DATA_WRITE == (mode & PVR_FOPEN_MODE_DATA_WRITE))
    {  //create a event-idx
        //first event=default_name
        if(pvr_init_eventidx(pvrFd, filename) == NULL)
        {
	        MT_ERR_PVR("PVR file: pvr_init_eventidx Error !\n");
	        return -1;
        }
    }
    else
    {
        int i=0;
        g_stPvrFiles[pvrFd].eventidx=NULL;
        for(i=0;i<PVR_FILE_MAX_FILE;i++)
        {
            if((0==strcmp(filename,g_stPvrFiles[i].szFileName)) 
                && (PVR_FOPEN_MODE_DATA_WRITE == (g_stPvrFiles[i].openMode & PVR_FOPEN_MODE_DATA_WRITE)))
            {
                g_stPvrFiles[pvrFd].eventidx=g_stPvrFiles[i].eventidx;
                strcpy(g_stPvrFiles[pvrFd].szFilePath,g_stPvrFiles[i].szFilePath);
                g_stPvrFiles[pvrFd].eventidx->flag_shared = 1;
                break;
            }
        }
    }

    if (PVR_FILE_NAME_FMT_HISI == fileFmt)
    {
        if(bSupportTimeShiftEvent == MT_TRUE)
        {
             if(u32TimeShiftEventDataUnitSize >= PVR_MIN_FILE_NODE_SIZE_FOR_EVENT)
             {
                   fileNodeSize = u32TimeShiftEventDataUnitSize;
             }
             else
             {
                fileNodeSize = PVR_DEFAULT_FILE_NODE_SIZE_FOR_EVENT;
             }
        }
        else
        {
            fileNodeSize = g_FileSizeInMbytes*ONE_M_BYTES;//PVR_DEFAULT_FILE_NODE_SIZE;
        }
    }
    else
    {
        fileNodeSize = PVR_FILE_GetFileSize(fNameOpen);
    }

    g_stPvrFiles[pvrFd].u64FileNodeSize = fileNodeSize;
    PVR_EVENT_D("PVR file: fmt:%d, NodeSize:%llu.\n", fileFmt, fileNodeSize);

    //MT_INFO_PVR("+++pvr.open=%s, mode=%x,%x\n",fNameOpen,mode,O_DIRECT);
    if(O_APPEND&mode){                                                      //to get the last block
        MT_U32 nodeIdx_1=PVR_Get_File_BlockNum(fNameOpen);
        if(1 < nodeIdx_1){
            MT_CHAR fNameReal[PVR_MAX_FILENAME_LEN + 5];
            nodeIdx_1--;
            PVRFileGetRealFName(fNameOpen, fNameReal, nodeIdx_1);
            strcpy(fNameOpen,fNameReal);
        }
    }
    sysFd = PVR_OPEN(fNameOpen, mode);

    if (-1 == sysFd)
    {
        pvr_free_eventidx(&g_stPvrFiles[pvrFd], bSupportTimeShiftEvent);    //free event idex
        g_stPvrFiles[pvrFd].bOpened = MT_FALSE;
        g_stPvrFiles[pvrFd].systemFd = PVR_FILE_INVALID_FILE;
        MT_ERR_PVR("PVR file: can NOT open '%s', mode:%#x.\n", fNameOpen, mode);
        return -1;
    }

    memset(g_stPvrFiles[pvrFd].szFileName, '\0', sizeof(g_stPvrFiles[pvrFd].szFileName));
    strncpy(g_stPvrFiles[pvrFd].szFileName, filename,strlen(filename));

    g_stPvrFiles[pvrFd].systemFd = sysFd;
    g_stPvrFiles[pvrFd].openMode = mode&(~O_APPEND);    //remove append;
    g_stPvrFiles[pvrFd].u64StartOfCurFd = 0;
    g_stPvrFiles[pvrFd].u64MaxOffset=0;
    g_stPvrFiles[pvrFd].u64SeekOffset = 0;
    g_stPvrFiles[pvrFd].u64CycMaxSize = 0;
    g_stPvrFiles[pvrFd].u64LastWPtrEnd = 0;
    MT_INFO_PVR("open64 g_stPvrFiles[%d]:%s success %x\n",pvrFd,g_stPvrFiles[pvrFd].szFileName,sysFd);
    return pvrFd + PVR_FILE_FD_BASE;
}

int PVR_CLOSE64(PVR_FILE64 file, MT_BOOL bTimeShiftEvent)
{
    int pvrFd = PVRFileGetPVRFd(file);

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);
    MT_INFO_PVR("PVR_CLOSE64 %x,%x\n",file,g_stPvrFiles[pvrFd].openMode);

    g_stPvrFiles[pvrFd].bOpened = MT_FALSE;
    { //free shared event infor!
        MT_U32 flag_shared;
        PVR_EVENTIDX_S *eventidx=NULL;
        if(g_stPvrFiles[pvrFd].eventidx)
        {
            eventidx=g_stPvrFiles[pvrFd].eventidx;
            PVR_EVENT_D("%s=====eventidx->flag_shared=%d\n",__FUNCTION__,eventidx->flag_shared);

            flag_shared=eventidx->flag_shared;
            eventidx->flag_shared=0;
            if(0==flag_shared)
            {
                PVR_EVENT_D("%s======%d\n",__FUNCTION__,__LINE__);
                pvr_free_eventidx(&g_stPvrFiles[pvrFd], bTimeShiftEvent);
                g_stPvrFiles[pvrFd].eventidx=NULL;
            }
        }
    }

    g_stPvrFiles[pvrFd].openMode = 0;

    return PVR_CLOSE(g_stPvrFiles[pvrFd].systemFd);
}

MT_VOID PVR_SET_MAXFILE_SIZE(PVR_FILE64 file, MT_U64 maxSize)
{
    int pvrFd = PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    if (pvrFd >= PVR_FILE_MAX_FILE)
    {
        return ;
    }

    if (g_stPvrFiles[pvrFd].bOpened != MT_TRUE)
    {
        return;
    }

    pPvrFile = &g_stPvrFiles[pvrFd];
    pPvrFile->u64CycMaxSize = maxSize;

    return;
}

MT_U32 PVR_SetRecFileSize( MT_U32 u32NewFileSizeInMBytes)
{
    MT_U32 tmp = g_FileSizeInMbytes;
    g_FileSizeInMbytes = u32NewFileSizeInMBytes;
    //MT_ERR_PVR("u32NewFileSizeInMBytes = %d\n",u32NewFileSizeInMBytes);
    return tmp;
}

int PVR_FSYNC64(PVR_FILE64 file)
{
    int pvrFd = PVRFileGetPVRFd(file);

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);

    return PVR_FSYNC(g_stPvrFiles[pvrFd].systemFd);
}

MT_S32 PVR_GetTsFileFd(PVR_FILE64 file)
{
    MT_U32      pvrFd = (MT_U32)PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);

    pPvrFile = &g_stPvrFiles[pvrFd];

    return pPvrFile->systemFd;
}

ssize_t PVR_PREAD64(MT_U8 *pMem, MT_U32 size, PVR_FILE64 file, MT_U64 offset)
{
    MT_U32 nodeIdx;
    int sysFd;
    int pvrFd = PVRFileGetPVRFd(file);
    MT_U32      readLen1 = 0;
    MT_U32      readLen2 = 0;
    MT_U32      readed;
    PVR_FILE_S *pPvrFile;
    MT_U64      truenodesize=0,eventendpos=0;
    //MT_U64      tempall=offset+size;

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);
    PVR_EVENT_D("FUNC >>>  %s    %d,  offset=%llx\n",__FUNCTION__,__LINE__,offset);

    pPvrFile = &g_stPvrFiles[pvrFd];
    if (0 == size)
    {
        MT_ERR_PVR("%s_%d,size==0\n",__FUNCTION__,__LINE__);
        return 0;
    }

    if((NULL != pPvrFile->eventidx) && (pPvrFile->eventidx->flag_wrewind)){ //if w-rewind. r need reopen!
        pPvrFile->eventidx->flag_wrewind=0;
        if(pPvrFile->u64MaxOffset > pPvrFile->u64FileNodeSize){ //protect!
            pPvrFile->u64MaxOffset = pPvrFile->u64FileNodeSize;
        }
        if(-1 != pPvrFile->systemFd){ //recycle cache!
            PVR_FADVISE(pPvrFile->systemFd, 0, pPvrFile->u64MaxOffset, POSIX_FADV_DONTNEED);
        }
    }

    if(PVR_FILE_INVALID_FILE != pPvrFile->systemFd)
    {
        //sub-event may be close.
        truenodesize=pPvrFile->u64FileNodeSize;
        //support tf-evet
        if(NULL == pPvrFile->eventidx)
        {
            MT_CHAR indexFileName[PVR_MAX_FILENAME_LEN]={0};
            sprintf(indexFileName,"%s.idx",pPvrFile->szFileName);
            if(PVR_CHECK_FILE_EXIST(indexFileName)) 
            {
                PVR_IDX_HEADER_INFO_S  pHeadInfo={0,};
                int indexFd = PVR_OPEN(indexFileName, PVR_FOPEN_MODE_DATA_READ);
                if (MT_SUCCESS != PVRIndexGetHeaderInfo(indexFd, &pHeadInfo))
                {
                    MT_ERR_PVR("No Header Info in index File: %s\n", indexFileName);
                    PVR_CLOSE(indexFd);
                    return MT_ERR_PVR_INDEX_FORMAT_ERR; 
                }
                PVR_CLOSE(indexFd);
                PVR_EVENT_D("%s >>> pHeadInfo.shareFile == %s    %d\n",__FUNCTION__,
                        pHeadInfo.shareEvent[0].shareFileName, pHeadInfo.shareEvent[0].u64ShareFileOffset);
                if((pPvrFile->eventidx == NULL) && (pHeadInfo.shareEvent[0].u64ShareFileOffset > 0) && (pHeadInfo.u8ShareEventCount > 0))
                {
                    int i=0;
                    pPvrFile->eventidx = pvr_init_eventidx(pvrFd, pPvrFile->szFileName);
                    if(pPvrFile->eventidx == NULL)
                    {
		                MT_ERR_PVR("%s_%d,  pvr_init_eventidx fail\n",__FUNCTION__,__LINE__);
		                return -1;
                    }
                    pPvrFile->eventidx->widpos = pHeadInfo.u8ShareEventCount + 1;
                    pPvrFile->eventidx->ridpos = 0;
                    pPvrFile->eventidx->cell[0].u64head = 0;
                    pPvrFile->eventidx->cell[0].u64size = pHeadInfo.shareEvent[0].u64ShareFileOffset;
                    pPvrFile->eventidx->cell[0].bSaved = MT_TRUE;
                    if((pHeadInfo.shareEvent[0].u64ShareFileOffset > 0) && (pHeadInfo.u8ShareEventCount > 0))
                    {
                        for(i=0; i<pHeadInfo.u8ShareEventCount; i++)
                        {
                            pPvrFile->eventidx->cell[i+1].u64head = pHeadInfo.shareEvent[i].u64ShareFileOffset;
                            memset(indexFileName, 0, PVR_MAX_FILENAME_LEN);
                            strcpy(indexFileName, pPvrFile->szFilePath);
                            strcat(indexFileName, pHeadInfo.shareEvent[i].shareFileName);
                            if(pHeadInfo.shareEvent[i+1].u64ShareFileOffset > pHeadInfo.shareEvent[i].u64ShareFileOffset)  
                                pPvrFile->eventidx->cell[i+1].u64size = pHeadInfo.shareEvent[i+1].u64ShareFileOffset - pHeadInfo.shareEvent[i].u64ShareFileOffset;
                            else
                                pPvrFile->eventidx->cell[i+1].u64size = PVR_INVALID_DEFAULT_FILE_EVENT_SIZE;//PVR_FILE_GetFileSize(indexFileName);
                            strcpy(pPvrFile->eventidx->cell[i+1].cellname, pHeadInfo.shareEvent[i].shareFileName);
                            pPvrFile->eventidx->cell[i+1].bSaved = MT_TRUE;
                        }
                        if(1)
                        {
                            for(i=0;i<=pHeadInfo.u8ShareEventCount;i++)
                            {
                        
                                PVR_EVENT_D("%d >>> pPvrFile->eventidx->cell[i].cellname=%s, cell[i].size=0x%llx\n",
                                            i,pPvrFile->eventidx->cell[i].cellname,pPvrFile->eventidx->cell[i].u64size);
                             }
                        }
                    }
                    else
                    {
                         pPvrFile->eventidx->cell[0].u64size = PVR_INVALID_DEFAULT_FILE_EVENT_SIZE;
                    }
                }
            }
            PVR_EVENT_D("++truenodesize=%llx,%llx,%llx, pPvrFile->szFileName=%s\n",offset,pPvrFile->u64StartOfCurFd,truenodesize, pPvrFile->szFileName);
        }

        if((NULL != pPvrFile->eventidx) && (0xffffffff != pPvrFile->eventidx->ridpos) &&
           (PVR_INVALID_DEFAULT_FILE_EVENT_SIZE != pPvrFile->eventidx->cell[pPvrFile->eventidx->ridpos].u64size)){ //this event recording over
            PVR_EVENT_D("%s >>> ++r=%x,w=%x,%llx,%llx,   %llx,%llx,%llx\n",__FUNCTION__,pPvrFile->eventidx->ridpos,pPvrFile->eventidx->widpos,
              pPvrFile->eventidx->cell[pPvrFile->eventidx->ridpos].u64head,pPvrFile->eventidx->cell[pPvrFile->eventidx->ridpos].u64size,
              pPvrFile->u64StartOfCurFd,pPvrFile->u64FileNodeSize,eventendpos);
            eventendpos=pPvrFile->eventidx->cell[pPvrFile->eventidx->ridpos].u64head + pPvrFile->eventidx->cell[pPvrFile->eventidx->ridpos].u64size;
            if(pPvrFile->u64StartOfCurFd + pPvrFile->u64FileNodeSize > eventendpos){
                truenodesize=eventendpos-pPvrFile->u64StartOfCurFd;
            }
        }

        if (PVRFileOffsetMatchNode(offset, pPvrFile->u64StartOfCurFd, truenodesize))
        {
            sysFd = pPvrFile->systemFd;
            readLen1 = (offset + size >= pPvrFile->u64StartOfCurFd + truenodesize)
                        ?  (MT_U32)(pPvrFile->u64StartOfCurFd + truenodesize - offset)
                        : size;
            PVR_EVENT_D("%s >>> size =%x, readLen1=%x, %llx=%llx   %x==%x\n",__FUNCTION__,size,readLen1,
                        offset + size,
                        pPvrFile->u64StartOfCurFd + truenodesize,
                         (MT_U32)(pPvrFile->u64StartOfCurFd + truenodesize - offset),
                        size);
            if(pPvrFile->openMode & O_DIRECT){
                pvr_lseek(sysFd,(off_t)offset, 0);
                readed = pvr_read(sysFd,pMem, readLen1);
                PVR_EVENT_D("PVR_PREAD64 >>> +++di.r0.fd=%d,%x,%x,fl=%llx,fg=%llx\n",sysFd,readed,readLen1,(offset - pPvrFile->u64StartOfCurFd),offset);
            }else{
                PVR_EVENT_D("PVR_PREAD64 >>> readed =%x, readLen1=%x\n",size,readLen1);
                readed = (MT_U32)PVR_READ((void *)pMem, (size_t)readLen1, sysFd, (off64_t)(offset - pPvrFile->u64StartOfCurFd));
                PVR_EVENT_D("PVR_PREAD64 >>> +++fs.r0.fd=%d,%x,%x,fl=%llx,fg=%llx\n",sysFd,readed,readLen1,(offset - pPvrFile->u64StartOfCurFd),offset);
            }
            if (readed != readLen1)
            {
                if(readed > 0)
                {
                    pPvrFile->u64SeekOffset = offset +  (MT_U64)readed;
                }
                if(pPvrFile->u64MaxOffset<pPvrFile->u64SeekOffset){ //get max read-pos
                    pPvrFile->u64MaxOffset=pPvrFile->u64SeekOffset;
                }
                PVR_EVENT_D("PVR_PREAD64 >>> readLen1 =%d,    ==========    readed=%d\n",readLen1,readed);
                return (ssize_t)readed;
            }
            offset += (MT_U64)readLen1;
        }
    }
    PVR_EVENT_D("readLen1 =%x,    ==========    size=%x\n",readLen1,size);
    if (readLen1 != size) /* need another file */
    {
        MT_CHAR  fNameOpen[PVR_MAX_FILENAME_LEN + 5];
        MT_CHAR  fNameEvent[PVR_MAX_FILENAME_LEN + 5];
        MT_U32 i=0;
        if(PVR_FILE_INVALID_FILE != pPvrFile->systemFd){
            PVR_CLOSE(pPvrFile->systemFd);
            pPvrFile->systemFd=PVR_FILE_INVALID_FILE;
        }
        PVR_EVENT_D("FUNC == %s    %d\n",__FUNCTION__,__LINE__);

        if(NULL != pPvrFile->eventidx)    //read by event mode
        {      
            pPvrFile->eventidx->ridpos=0xffffffff;
            for(i=0; i<=pPvrFile->eventidx->widpos; i++)   //find by offset [start , end]
            {
                PVR_EVENT_D("PVR_PREAD64 >>> Event i =%d,%llx[%llx,%llx, %llx]\n",i,offset,pPvrFile->eventidx->cell[i].u64head,
                            pPvrFile->eventidx->cell[i].u64head+pPvrFile->eventidx->cell[i].u64size,
                            pPvrFile->eventidx->cell[i].u64Tail);
                if((offset >= pPvrFile->eventidx->cell[i].u64head)
                   && ((offset < pPvrFile->eventidx->cell[i].u64head+pPvrFile->eventidx->cell[i].u64size)
                    || (PVR_INVALID_DEFAULT_FILE_EVENT_SIZE == pPvrFile->eventidx->cell[i].u64size)
                    ||((0 == pPvrFile->eventidx->cell[i].u64size) && (PVR_INVALID_DEFAULT_FILE_EVENT_SIZE == pPvrFile->eventidx->cell[i].u64Tail))))  //PVR_INVALID_DEFAULT_FILE_EVENT_SIZE means end
                {
                    pPvrFile->eventidx->ridpos=i;
                    break;
                }
            }
            if(0xffffffff==pPvrFile->eventidx->ridpos){
                PVR_EVENT_D("error. no more events %x,%x!\n",pPvrFile->eventidx->ridpos,pPvrFile->eventidx->widpos);
                return (ssize_t)readLen1;
            }
            if(loop_event_id  != 0xffffffff)   pPvrFile->eventidx->ridpos=  loop_event_id;
		 
            //PVR_EVENT_D("PVR_PREAD64 >>> pPvrFile->eventidx->ridpos=%d\n",pPvrFile->eventidx->ridpos);
            nodeIdx = (MT_U32)PVRFileGetNodeIdx((offset-pPvrFile->eventidx->cell[pPvrFile->eventidx->ridpos].u64head), pPvrFile->u64FileNodeSize);
           // PVR_EVENT_D("PVR_PREAD64 >>> Event ++++nodeIdx=%x,%llx,%llx,%llx, ridpos = %x\n",nodeIdx,offset,
           //         pPvrFile->eventidx->cell[pPvrFile->eventidx->ridpos].u64head,pPvrFile->u64FileNodeSize,pPvrFile->eventidx->ridpos);
            strcpy(fNameEvent,pPvrFile->szFilePath);
            strcat(fNameEvent,pPvrFile->eventidx->cell[pPvrFile->eventidx->ridpos].cellname);
            //PVR_EVENT_D("PVR_PREAD64 >>> fNameEvent=%s\n",fNameEvent);
            PVRFileGetRealFName(fNameEvent, fNameOpen, (MT_S32)nodeIdx);
            sysFd = PVR_OPEN(fNameOpen, pPvrFile->openMode);
            pPvrFile->u64StartOfCurFd = pPvrFile->eventidx->cell[pPvrFile->eventidx->ridpos].u64head+((MT_U64)nodeIdx)*pPvrFile->u64FileNodeSize;
            PVR_EVENT_D("Event +++rd64.read %s,%s,%d,%d,%d,%llx\n",fNameOpen,fNameEvent,sysFd,nodeIdx,pPvrFile->eventidx->ridpos,pPvrFile->u64StartOfCurFd);
        }
        else    //read by normal mode
        {       
            nodeIdx = (MT_U32)PVRFileGetNodeIdx(offset, pPvrFile->u64FileNodeSize);
            PVR_EVENT_D("PVR_PREAD64 >>> nodeIdx = 0x%x, offset=%llx, pPvrFile->u64FileNodeSize0x%llx\n",nodeIdx,offset,pPvrFile->u64FileNodeSize);            
            PVRFileGetRealFName(pPvrFile->szFileName, fNameOpen, (MT_S32)nodeIdx);
            PVR_EVENT_D("PVR_PREAD64 >>> +++r1.open anoter file=%s,%x\n",fNameOpen,pPvrFile->openMode);
            sysFd = PVR_OPEN(fNameOpen, pPvrFile->openMode);
            pPvrFile->u64StartOfCurFd = ((MT_U64)nodeIdx)*pPvrFile->u64FileNodeSize;
            PVR_EVENT_D("PVR_PREAD64 >>> +++rd64.open %s,fNameEvent=%s,sysFd=%d,nodeIdx=%d,u64StartOfCurFd=%llx\n",fNameOpen,fNameEvent,sysFd,nodeIdx,pPvrFile->u64StartOfCurFd);
        }
        if (-1 == sysFd)
        {     //open event_next.ts fail
            MT_INFO_PVR("PVR_PREAD64 >>> PVR_PREAD64 >>> %s_%d,size==0\n",__FUNCTION__,__LINE__);
            PVR_EVENT_D("PVR_PREAD64 >>> PVR_PREAD64 >>> pPvrFile->szFileName ======= %s\n",pPvrFile->szFileName);
            return (ssize_t)readLen1;
        }
        pPvrFile->systemFd = sysFd;
        pPvrFile->u64MaxOffset=0;
        readLen2 = size - readLen1;
        if(pPvrFile->openMode & O_DIRECT){
            pvr_lseek(sysFd,(off_t)offset, 0);
            readed = pvr_read(sysFd,(pMem+readLen1), readLen2);
            PVR_EVENT_D("+++di.r1.fd=%d,%x,%x,fl=%llx,fg=%llx\n",sysFd,readed,readLen2,(offset - pPvrFile->u64StartOfCurFd),offset);
        }else{
            readed = (MT_U32)PVR_READ((void*)(pMem+readLen1), (size_t)readLen2, sysFd,(off64_t)(offset - pPvrFile->u64StartOfCurFd));
            PVR_EVENT_D("+++fs.r1.fd=%d,%x,%x,fl=%llx,fg=%llx\n",sysFd,readed,readLen2,(offset - pPvrFile->u64StartOfCurFd),offset);
        }
        if (-1 == (MT_S32)readed)
        {
            MT_INFO_PVR("%s_%d,readed==-1\n",__FUNCTION__,__LINE__);
            return -1;
        }
        else //if (readed != readLen2)
        {
            pPvrFile->u64SeekOffset = offset + (MT_U64)readed;    //offset include readlen1
            if(pPvrFile->u64MaxOffset<pPvrFile->u64SeekOffset){ //get max read-pos
                pPvrFile->u64MaxOffset=pPvrFile->u64SeekOffset;
            }
            return (ssize_t)(readed + readLen1);
        }
    }

    pPvrFile->u64SeekOffset = offset +  readLen2;
    if(pPvrFile->u64MaxOffset<pPvrFile->u64SeekOffset){ //get max read-pos
        pPvrFile->u64MaxOffset=pPvrFile->u64SeekOffset;
    }

    return (ssize_t)size;
}


/*
 * read ts without index file
 */
ssize_t PVR_TS_PREAD64(MT_U8 *pMem, MT_U32 size, PVR_FILE64 file, MT_U64 offset)
{
    MT_S32      pvrFd = PVRFileGetPVRFd(file);
    MT_S64      readed;
    PVR_FILE_S *pPvrFile;
    MT_U64      u64Tmp = 0;

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);

    pPvrFile = &g_stPvrFiles[pvrFd];

    readed = (MT_S64)PVR_READ((void *)pMem, (size_t)size, pPvrFile->systemFd, (off64_t)offset);
    if (readed < 0)
    {
        MT_ERR_PVR("[PVR_TS_PREAD64] PVR_READ error");
    }
    else if (readed > 0)
    {
        u64Tmp = (MT_U64)readed;
        pPvrFile->u64SeekOffset = offset + u64Tmp;
    }

    return (ssize_t)readed;
}


static MT_S32 PVR_Add_New_Event(PVR_FILE_S *pPvrFile,MT_CHAR *filename, MT_BOOL isStop, MT_U32  u32EventStartTime, MT_BOOL eventNameIsNull)
{
    MT_U32 truelen=0;
    MT_U32 wid=0;
    MT_U32 next=0;
    PVR_CELLMAP_S *tmp=NULL;
//    int i=0;
    
    if((NULL==pPvrFile)||(NULL==pPvrFile->eventidx) || (NULL==pPvrFile->eventidx->cell))
    {
        return -1;
    }
    wid = pPvrFile->eventidx->widpos;
    next = 1+wid;
    truelen = (next+1)*sizeof(PVR_CELLMAP_S);

    if(truelen >= pPvrFile->eventidx->celllen)   //extend memory.num+PVR_FILE_MAX_EVENT
    {
        tmp=PVR_BigEx_Cell_Space(pPvrFile->eventidx->cell,pPvrFile->eventidx->celllen,(pPvrFile->eventidx->celllen+PVR_FILE_MAX_EVENT*sizeof(PVR_CELLMAP_S)));
        if(NULL != tmp)
        {
            pPvrFile->eventidx->cell=tmp;
            pPvrFile->eventidx->celllen=(pPvrFile->eventidx->celllen+PVR_FILE_MAX_EVENT*sizeof(PVR_CELLMAP_S));
        }
        else
        {
            return -2;
        }
    }
    // free timeout event data
    PVR_Remove_Timeout_Event(pPvrFile);
    
    pPvrFile->eventidx->cell[wid].u64size = pPvrFile->u64SeekOffset-pPvrFile->eventidx->cell[wid].u64head;    //record the ture size
    
    if(isStop == MT_TRUE)
    {
        pPvrFile->eventidx->cell[wid].u64Tail = pPvrFile->u64SeekOffset;
    }
    else
    {
        pPvrFile->eventidx->cell[wid].u64Tail = 0;
    }
    strcpy(pPvrFile->eventidx->cell[next].cellname,filename);
    pPvrFile->eventidx->cell[next].u64head=pPvrFile->u64SeekOffset;
    pPvrFile->eventidx->cell[next].u64size=0;   //default==max
    pPvrFile->eventidx->cell[next].u64Tail = PVR_INVALID_DEFAULT_FILE_EVENT_SIZE; 
    pPvrFile->eventidx->cell[next].bSaved = MT_FALSE;
    (void)MT_PVR_SysGetTimeStampMs(&(pPvrFile->eventidx->cell[next].u32StartTime));
    pPvrFile->eventidx->cell[next].u32EventNode[0].u32NodeStartFrame = 0;
    if(!eventNameIsNull)
    {
        memcpy(&(pPvrFile->eventidx->currentRunningEventCell), &(pPvrFile->eventidx->cell[next]), sizeof(PVR_CELLMAP_S));
    }
    PVR_EVENT_D("pPvrFile->eventidx->cell[%d].u64Tail = %llx,  u64size = %llx, next=%d, cell[next].cellname=%s\n",
                                                    wid, 
                                                    pPvrFile->eventidx->cell[wid].u64Tail, 
                                                    pPvrFile->eventidx->cell[wid].u64size,
                                                    next, pPvrFile->eventidx->cell[next].cellname);
                                                    
    pPvrFile->eventidx->widpos=next;    

    return 0;
}


MT_S32 PVR_RecNewEvent(PVR_FILE64 file,MT_CHAR *fname, 
                                                   PVR_TIMESHIFT_EVENT_TYPE  type,  
                                                   MT_U32  u32EventStartTime,
                                                   MT_U8 *preEventName,
                                                   MT_BOOL  preEventIsSaved)
{
    MT_S32 ret;
    int sysFd;
    int pvrFd = 0xff;
    MT_BOOL  isStop=MT_FALSE;
    MT_BOOL  newEventNameIsNull  = MT_FALSE;

    PVR_FILE_S *pPvrFile;
    MT_CHAR  fPATHOpen[PVR_MAX_FILENAME_LEN*2]={0};
    MT_CHAR  fPATHEvent[PVR_MAX_FILENAME_LEN + 32]={0};
    MT_CHAR  fFILEEvent[PVR_MAX_FILENAME_LEN + 32]={0};
    MT_CHAR *filepath=PVR_Get_EventPath(file);

    pvrFd = PVRFileGetPVRFd(file);
    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);
    pPvrFile = &g_stPvrFiles[pvrFd];

    PVR_EVENT_D("PVR_RecNewEvent \n");
    if(pPvrFile->eventidx == NULL)          return -1;
    if(preEventName)    PVR_EVENT_D("preEventName=%s, preEventIsSaved=%d, wid=%d\n",preEventName, preEventIsSaved,pPvrFile->eventidx->widpos);
    if(preEventName && preEventIsSaved)
    {
        int i=0;
        int mastEventId = -1;
        MT_BOOL allCellSave = MT_FALSE;
        PVR_IDX_HEADER_INFO_S  pHeadInfo={0,};
        
        memset(fFILEEvent, 0, PVR_MAX_FILENAME_LEN);
        strcpy(fFILEEvent, (char*)preEventName);
   	    if(strstr(fFILEEvent, ".ts") == NULL)
            strcat(fFILEEvent,".ts");

        for(i=0; i<=pPvrFile->eventidx->widpos; i++)  //for save stop, save stop,save stop ......
        {
            PVR_EVENT_D("preEventName=%s, pPvrFile->eventidx->cell[%d].cellname=%s\n",fFILEEvent, i,pPvrFile->eventidx->cell[i].cellname);
            if(!strcmp(fFILEEvent, pPvrFile->eventidx->cell[i].cellname))
            {
                allCellSave = MT_TRUE;
                mastEventId = i;
            }
 
            if(allCellSave)  pPvrFile->eventidx->cell[i].bSaved = MT_TRUE;
            PVR_EVENT_D("preEventName=%s, i = %d  %d, %d\n",fFILEEvent, i,mastEventId,pPvrFile->eventidx->cell[i].bSaved);
            if(filepath)   PVR_EVENT_D("filepath=%s\n",filepath);
            if((mastEventId >= 0) && (i > mastEventId) && (pPvrFile->eventidx->cell[i].bSaved == MT_TRUE) && filepath)
            {
                  memset(fPATHOpen, 0, sizeof(fPATHOpen));
                  sprintf(fPATHOpen, "%s%s.idx",filepath,fFILEEvent);
                  if(mastEventId == 0)      strcat(fPATHOpen, "_event0");
                  PVR_EVENT_D("fPATHOpen=%s\n",fPATHOpen);
                  if (MT_SUCCESS  == PVRIndexGetHeaderInfoByName((MT_U8*)fPATHOpen, &pHeadInfo,sizeof(PVR_IDX_HEADER_INFO_S)))
                  {
                         PVR_EVENT_D("allCellSave=%d, mastEventId=%d, cell[%d].cellname=%s, size=%llx, (cell[i-1].cellname=%s, head=%llx size=%llx)  shareEvent[%d].u64ShareFileOffset=%x \n",
                                    allCellSave, mastEventId,i,pPvrFile->eventidx->cell[i].cellname,pPvrFile->eventidx->cell[i].u64size,
                                    pPvrFile->eventidx->cell[i-1].cellname, pPvrFile->eventidx->cell[i-1].u64head,pPvrFile->eventidx->cell[i-1].u64size,
                                    i-(mastEventId+1),pHeadInfo.shareEvent[i-(mastEventId+1)].u64ShareFileOffset);
                         if(pHeadInfo.u8ShareEventCount  >= PVR_TIMESHIFT_EVENT_SAVE_TIMES)   
                         {
                                MT_ERR_PVR("PVR save event > PVR_TIMESHIFT_EVENT_SAVE_TIMES(%x > %x)\n", 
                                                    pHeadInfo.u8ShareEventCount,PVR_TIMESHIFT_EVENT_SAVE_TIMES);
                                break;
                         }
                         if(pHeadInfo.shareEvent[i-(mastEventId+1)].u64ShareFileOffset == 0)
                         {
                                MT_U64 offset = pPvrFile->eventidx->cell[i-1].u64head + pPvrFile->eventidx->cell[i-1].u64size - pPvrFile->eventidx->cell[mastEventId].u64head;
                                pHeadInfo.shareEvent[i-(mastEventId+1)].u64ShareFileOffset = offset;
                                strcpy(pHeadInfo.shareEvent[i-(mastEventId+1)].shareFileName, pPvrFile->eventidx->cell[i].cellname);
                                pPvrFile->eventidx->cell[i].bShare = MT_TRUE;
                                pHeadInfo.u8ShareEventCount ++;
                                PVR_EVENT_D("i=%d, %d/%d ----- shareFileName=%s,  u64ShareFileOffset=%d\n",i, (i-mastEventId-1), pHeadInfo.u8ShareEventCount,
                                        pHeadInfo.shareEvent[i-(mastEventId+1)].shareFileName,pHeadInfo.shareEvent[i].u64ShareFileOffset);
                          }
                          ret = PVRIndexSaveHeaderInfo((MT_U8*)fPATHOpen, &pHeadInfo);
                          if(ret != MT_SUCCESS)   MT_ERR_PVR("Save Header Info in index File: %s\n", fPATHOpen);
                   }
                   else
                   {
                          pPvrFile->eventidx->cell[mastEventId].coverLastEventId = i;
                  }
            }
            //PVR_EVENT_D("allCellSave=%d,  pPvrFile->eventidx->cell[%d].bSaved=%d\n",allCellSave,i,pPvrFile->eventidx->cell[i].bSaved);
        }       
    }
    memset(fFILEEvent, 0, PVR_MAX_FILENAME_LEN);
    memset(fPATHOpen, 0, PVR_MAX_FILENAME_LEN);
    if(NULL==fname){
        //sprintf(fFILEEvent,"%s_%d",pPvrFile->eventidx->cell[0].cellname,pPvrFile->eventidx->normalnum); //first event has default name
        if((pPvrFile->eventidx->currentRunningEventCell.u64head > 0) &&  (strlen(pPvrFile->eventidx->currentRunningEventCell.cellname) > 1))
        {
            sprintf(fFILEEvent,"%s_%d",pPvrFile->eventidx->currentRunningEventCell.cellname, pPvrFile->eventidx->normalnum); //first event has default name
        }
        else
        {
            sprintf(fFILEEvent,"%s_%d",pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].cellname, 0); //first event has default name
        }
        pPvrFile->eventidx->normalnum++;
        isStop = MT_TRUE;
        newEventNameIsNull = MT_TRUE;
    }
    else
    {
        strcat(fFILEEvent,fname);
        if(strstr(fFILEEvent, ".ts") == NULL)    strcat(fFILEEvent,".ts");
        if(type == TIMESHIFT_EVENT_STOP_START)    isStop = MT_TRUE;
        else  isStop=MT_FALSE;
        newEventNameIsNull = MT_FALSE;
    }
    PVR_EVENT_D("+++newevent=%s,%s\n",fname,pPvrFile->szFilePath);
    strcpy(fPATHEvent,pPvrFile->szFilePath);
    strcat(fPATHEvent,fFILEEvent);
    PVRFileGetRealFName(fPATHEvent, fPATHOpen, (MT_S32)0);
    sysFd = PVR_OPEN(fPATHOpen, (pPvrFile->openMode | O_TRUNC ));	// (O_CREAT | O_RDWR | O_LARGEFILE  | O_TRUNC )
    PVR_EVENT_D("+++event.open=%s,%s,%s\n",fPATHOpen,fPATHEvent,fFILEEvent);
    if (-1 == sysFd)
    {
        MT_ERR_PVR("PVR can't open file:'%s' for write.\n", fPATHOpen);
        return -1;
    }else{
        ret=PVR_Add_New_Event(pPvrFile,fFILEEvent, isStop, u32EventStartTime, newEventNameIsNull);
        if(0==ret){
            PVR_CLOSE(pPvrFile->systemFd);
            pPvrFile->systemFd = sysFd;
            pPvrFile->u64StartOfCurFd = pPvrFile->u64SeekOffset;
        }else{
          MT_ERR_PVR("PVR can't add event map:%x\n", ret);
          return -1;
        }
    }
    MT_INFO_PVR("++++new event success\n");

    return 0;
}


MT_S32 PVR_Rec_SaveTimeShiftEvent0Data(PVR_FILE64 file,  PVR_EVENT_STATUS_E eventStatus)
{
//    MT_U32 i=0;
//    MT_U32 j=0;
//    MT_CHAR cellname[PVR_MAX_FILENAME_LEN]={0};
    int pvrFd = PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);
    pPvrFile = &g_stPvrFiles[pvrFd];
    if(pPvrFile->eventidx)
    {
        pPvrFile->eventidx->cell[0].bSaved = MT_TRUE;
        PVR_EVENT_D("PVR_Rec_SaveTimeShiftEvent0Data  >>> 2\n");
    }
    return 0;

}

MT_S32 PVR_Rec_GetEventIndexDataEntry(PVR_FILE64 file,MT_CHAR *pEeventName,
                                    PVR_IDX_HEADER_INFO_S *stIdxHeaderInfo, MT_U64 *s, MT_U64 *e,  PVR_EVENT_STATUS_E eventStatus)
{
    MT_U32 i=0;
    MT_U32 j=0;
    MT_CHAR cellname[PVR_MAX_FILENAME_LEN]={0};
    int pvrFd = PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);

    pPvrFile = &g_stPvrFiles[pvrFd];
    strcpy(cellname,pEeventName);
    if(strstr(cellname, ".ts") == NULL)    strcat(cellname,".ts");
    for(i=0; i<=pPvrFile->eventidx->widpos; i++)
    {
        PVR_EVENT_D("cellname=%s,pPvrFile->eventidx->cell[%d].cellname=%s, head=%llx, starttime=%d, size=%llx, tail=%llx, eventStatus=%d\n",cellname,i,pPvrFile->eventidx->cell[i].cellname,
            pPvrFile->eventidx->cell[i].u64head, pPvrFile->eventidx->cell[i].u32StartTime,
            pPvrFile->eventidx->cell[i].u64size,pPvrFile->eventidx->cell[i].u64Tail, eventStatus);
        if(0==strcmp(cellname, pPvrFile->eventidx->cell[i].cellname)){
            break;
        }
    }
    if((i > pPvrFile->eventidx->widpos) && (pPvrFile->eventidx->cell[i].u64head > 0)){  //recording
        MT_ERR_PVR("++find no it1\n");
        return -1;
    }
    if(i > pPvrFile->eventidx->widpos){
        MT_ERR_PVR("++find no it2\n");
        return -2;
    }
    *s=pPvrFile->eventidx->cell[i].u64head;
    pPvrFile->eventidx->cell[i].bSaved = MT_TRUE;
    if(pPvrFile->eventidx->cell[i].bShare)
    {
        stIdxHeaderInfo->shareFlagTsFile = 1;
    }
    //stIdxHeaderInfo->shareEvent[0].u64ShareFileOffset = 0;
    //memset(stIdxHeaderInfo->shareEvent[0].shareFileName, 0, PVR_MAX_FILENAME_LEN);
#if 1
    
    if(PVR_EVENT_STOP_SAVE == eventStatus)
    {
         if(pPvrFile->eventidx->cell[i].coverLastEventId > i)
         {
                *e=pPvrFile->eventidx->cell[i].u64head;
                for(j=i; j<= pPvrFile->eventidx->cell[i].coverLastEventId; j++)
                {
                    *e += pPvrFile->eventidx->cell[j].u64size;
                }
         }
         else
         {
               *e=pPvrFile->eventidx->cell[i].u64head + pPvrFile->eventidx->cell[i].u64size;
         }
    }
    else if((PVR_EVENT_RECING_SAVE == eventStatus ) || (PVR_EVENT_APPEND_SAVE == eventStatus))
    {
        *e= pPvrFile->u64SeekOffset;
    }
    else
    {
        MT_ERR_PVR("eventStatus = %d,  Error !!!\n",eventStatus);
        return -3;
    }
#else
  //  if(continue_rec == MT_TRUE)
    {
     //   pPvrFile->eventidx->cell[i].u64Tail  = 0;
    }
   for(j=i; j<=pPvrFile->eventidx->widpos; j++)
   { 
        if(pPvrFile->eventidx->cell[j].bSaved)
        {
            *e=pPvrFile->eventidx->cell[j].u64head+pPvrFile->eventidx->cell[j].u64size;
            PVR_EVENT_D("cccc >>> i=%d, *s=%llx, *e=%llx, u64MaxOffset=%llx, j=%d\n",
                                i,*s,*e, pPvrFile->u64MaxOffset, j); 
        }
   }

#endif
    return 0;
}

MT_CHAR * PVR_Get_EventPath(PVR_FILE64 file)
{
    int pvrFd = PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    if (pvrFd >= PVR_FILE_MAX_FILE){
        return NULL;
    }
    if (g_stPvrFiles[pvrFd].bOpened != MT_TRUE){
        return NULL;
    }
    pPvrFile = &g_stPvrFiles[pvrFd];

    return pPvrFile->szFilePath;
}

MT_CHAR * PVR_Get_EventTimeshiftName(PVR_FILE64 file)
{
    int pvrFd = PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    if (pvrFd >= PVR_FILE_MAX_FILE){
        return NULL;
    }
    if (g_stPvrFiles[pvrFd].bOpened != MT_TRUE){
        return NULL;
    }
    pPvrFile = &g_stPvrFiles[pvrFd];

    return pPvrFile->szFileName;
}

void PVR_Set_Rewinded(PVR_FILE64 file)
{
    int pvrFd = PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    if (pvrFd >= PVR_FILE_MAX_FILE){
        return ;
    }
    if (g_stPvrFiles[pvrFd].bOpened != MT_TRUE){
        return ;
    }
    pPvrFile = &g_stPvrFiles[pvrFd];

    if(pPvrFile->eventidx){
        pPvrFile->eventidx->flag_wrewind=1;
        //printf("+++rec.set flag-rewind=1\n");
    }
}

ssize_t PVR_PWRITE64(PVR_REC_CHN_S *pRecChn, const void *pMem,  MT_U32 size, PVR_FILE64 file, MT_U64 offset)
{
    MT_U32 nodeIdx=0;
    int sysFd;
    int pvrFd = PVRFileGetPVRFd(file);
    size_t      readLen1 = 0;
    size_t      readLen2 = 0;
    MT_S64     writen;
    PVR_FILE_S *pPvrFile;

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);

    pPvrFile = &g_stPvrFiles[pvrFd];
    if (0 == size)
    {
        return 0;
    }
    if(pRecChn == NULL)
    {
	 return -1;
    }
    if(PVR_FILE_INVALID_FILE == pPvrFile->systemFd){
        return 0;
    }
#ifdef PVR_CHECK_USB_FILE
    if (MT_SUCCESS != PVR_CheckUsbStillPlugIn())
    {
        return -1;
    }
#endif
    //PVR_EVENT_D("%s >>> file:%x,%llx,%x,%llx,%llx, %llx\n",__FUNCTION__,file,offset,size,pPvrFile->u64StartOfCurFd, pPvrFile->u64FileNodeSize,pPvrFile->u64SeekOffset);
    if (PVRFileOffsetMatchNode(offset, pPvrFile->u64StartOfCurFd, pPvrFile->u64FileNodeSize))
    {
        sysFd = pPvrFile->systemFd;
        readLen1 = (offset + size >= pPvrFile->u64StartOfCurFd + pPvrFile->u64FileNodeSize)
                    ? (size_t)(pPvrFile->u64StartOfCurFd + pPvrFile->u64FileNodeSize - offset)
                    : (size_t)size;
        //PVR_EVENT_D("PVR_PWRITE64 >>> ++w64.check.%llx,%llx\n",offset + size,pPvrFile->u64StartOfCurFd + pPvrFile->u64FileNodeSize);
        PVR_EVENT_D("PVR_PWRITE64 >>> +++w0.fd=%d,%d,r1=%x,r=%x,fl=%llx,fg=%llx\n",pRecChn->u32ChnID ,sysFd,readLen1,size,(offset - pPvrFile->u64StartOfCurFd),offset);
        writen = (MT_S64)PVR_WRITE(pMem, readLen1, sysFd, (off_t)(offset - pPvrFile->u64StartOfCurFd));
       //writen = pvr_write(sysFd,pMem, readLen1);
        if(writen < 0)
        {
            MT_ERR_PVR("pwrite1 error.writen<0 %x,%x,%x,%x\n",errno,pMem,readLen1,size);
            return -1;
        }
        else if(writen == 0)
        {
            PVR_EVENT_D("PVR_PWRITE64 >>> writen=0\n");
            return 0;
        }
        else
        {
            if ((size_t)writen != readLen1)
            {
                pPvrFile->u64SeekOffset = offset + (MT_U64)writen;
                PVR_EVENT_D("PVR_PWRITE64 >>> writen=%llx\n",writen);
 
                return (ssize_t)writen;
            }
            offset += readLen1;
        }
    }

    if((readLen1 != size) && (pPvrFile->eventidx))  /* need another file */
    {
        MT_CHAR  fNameOpen[PVR_MAX_FILENAME_LEN *2] = {0,};
        MT_CHAR  fNameEvent[PVR_MAX_FILENAME_LEN]= {0,};
        MT_CHAR  fNameNode[PVR_MAX_FILENAME_LEN *2] = {0,};
        MT_U32 u32CurrentTimeMs=0;
        int i=0;
        PVR_CLOSE(pPvrFile->systemFd);
	 //PVR_EVENT_D("PVR_PWRITE64 >>> line=%d\n",__LINE__);	
        pPvrFile->systemFd=PVR_FILE_INVALID_FILE;	 
	    if(pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].bSaved == MT_TRUE)
	    {
		    pRecChn->stUserCfg.bRewind = MT_FALSE;
		    pRecChn->stUserCfg.u64MaxTimeInMs = 0;
	    }
        if((offset == 0) && (pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u64head > 0))
        {
    		pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u64head = 0;
    		PVR_EVENT_D("PVR_PWRITE64 >>>offset = %llx, u64size = %llx\n",offset,pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u64size);
        }
        nodeIdx = (MT_U32)PVRFileGetNodeIdx((offset - pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u64head), pPvrFile->u64FileNodeSize);
    	 if(nodeIdx > PVR_REC_MAX_NODE && offset == 0)
    	 {
    		MT_ERR_PVR("PVRFileGetNodeIdx error :' nodeIdx = 0x%x'\n", nodeIdx);
    		nodeIdx = 0;
    		loop_event_id = pPvrFile->eventidx->widpos;
    	 }
        strcpy(fNameEvent,pPvrFile->szFilePath);
        strcat(fNameEvent,pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].cellname);
        PVRFileGetRealFName(fNameEvent, fNameOpen, (MT_S32)nodeIdx);
        //PVR_EVENT_D("%s >>> offset=%llx, head[%d]=%llx, pPvrFile->u64FileNodeSize=%llx, nodeIdx=%d\n",__FUNCTION__,offset, pPvrFile->eventidx->widpos, 
        //            pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u64head, pPvrFile->u64FileNodeSize, nodeIdx);
        (void)MT_PVR_SysGetTimeStampMs(&u32CurrentTimeMs);
        if(PVR_FILE_MAX_EVENT_NODE <= nodeIdx)
        {
            MT_ERR_PVR("PVR event Too many nodes ! nodeIdx=0x%x \n", nodeIdx);
            return -1;
        }
        pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u32EventNode[nodeIdx].u32NodeCreateTime  = u32CurrentTimeMs;
        if(pRecChn && pRecChn->stUserCfg.bSupportTimeShiftEvent)
        {
            pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u32EventNode[nodeIdx].u32NodeStartFrame = pRecChn->IndexHandle->stCycMgr.u32LastFrame;  
            PVR_EVENT_D("%s >>> fNameOpen ====%s, nodeIdx=%d, pu32TimeMs=%d, lastFrame=%d\n",__FUNCTION__,fNameOpen, nodeIdx, u32CurrentTimeMs,
                    pRecChn->IndexHandle->stCycMgr.u32LastFrame);
            if(nodeIdx > 2 && pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].bSaved == MT_FALSE)
            {
                for(i=1; i < nodeIdx-2; i++)   //node[0] is reserved, because of node_name
                {
                    MT_U32 nodeTime = (pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u32EventNode[i].u32NodeCreateTime);
                    if((nodeTime > 0) && (((u32CurrentTimeMs) - (nodeTime)) > g_timeShiftEventLoopTimeInMs))
                    {
                         snprintf(fNameNode, sizeof(fNameNode)-2,"%s.%04d", fNameEvent, i); 
                         if(PVR_CHECK_FILE_EXIST(fNameNode))
                         {
                            (MT_VOID)remove(fNameNode);
                            pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u32EventNode[i].u32NodeCreateTime= 0;
                            pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].firstNode = i + 1;
                            if(pRecChn)
                            {
                                 pRecChn->real_file_start_frame = pPvrFile->eventidx->cell[pPvrFile->eventidx->widpos].u32EventNode[i+1].u32NodeStartFrame;
                                 //PVR_UpdateSingleEventLoopStartFrame(pRecChn->real_file_start_frame);
                                 pRecChn->timeShiftEventTsFileFirstNode = i + 1;
                                 PVR_EVENT_D("pRecChn->real_file_start_frame =============%x\n",pRecChn->real_file_start_frame);
                             }
                         }
                         if(PVR_CHECK_FILE_EXIST(fNameEvent))
                         {
                            (MT_VOID)remove(fNameEvent);
                         }
                    }
                }
            }
        }
  
        sysFd = PVR_OPEN(fNameOpen, pPvrFile->openMode);
        if (-1 == sysFd)
        {
            MT_ERR_PVR("PVR can't open file:'%s' for write.\n", fNameOpen);
            return -1;
        }
        pPvrFile->systemFd = sysFd;
        pPvrFile->u64StartOfCurFd = offset;
        //MT_INFO_PVR("+++startoffset=%x\n",pPvrFile->u64StartOfCurFd);
        readLen2 = size - readLen1;
        //PVR_EVENT_D("%s >>> +++w1.fd=%d,r1=%x,r=%x,fl=%llx,fg=%llx\n",__FUNCTION__,sysFd,readLen1,size,(offset - pPvrFile->u64StartOfCurFd),offset);
        if(0)
        {
        	MT_U8 *rec_data = (MT_U8 *)((size_t)pMem+readLen1);
        	if(rec_data[0] != 0x47)
        	{
        		PVR_EVENT_D("Error Data : %x,%x,%x,%x,%x\n",rec_data[0],rec_data[1],rec_data[2],rec_data[3],rec_data[4]);
            	//return 0;
        	}
        }
        writen = (MT_S64)PVR_WRITE((void*)((size_t)pMem+readLen1), readLen2, sysFd, (off64_t)(offset - pPvrFile->u64StartOfCurFd));
        //writen = pvr_write(sysFd,((size_t)pMem+readLen1), readLen2);
        if ( writen < 0)
        {
            MT_ERR_PVR("pwrite2 error.writen<0 %x,%x,%x\n",errno,pMem,readLen1);
            return -1;
        }
        else if(writen == 0)
        {
            return 0;
        }
        else //if (writen != readLen2)
        {
            pPvrFile->u64SeekOffset = offset + (MT_U64)writen;    //offset include readlen1
            return (ssize_t)writen + (ssize_t)readLen1;
        }
    }

    pPvrFile->u64SeekOffset = offset +  readLen2;
    //PVR_EVENT_D("PVR_PWRITE64 >>> line=%d, pPvrFile->u64SeekOffset=0x%llx\n",__LINE__,pPvrFile->u64SeekOffset);	

    return (ssize_t)size;
}

ssize_t PVR_READ64(MT_U8 *pMem, MT_U32 size, PVR_FILE64 file)
{
    ssize_t ret;
    int pvrFd = PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);
    pPvrFile = &g_stPvrFiles[pvrFd];
    ret = PVR_PREAD64(pMem, size, file, pPvrFile->u64SeekOffset);

    return ret;
}

ssize_t PVR_WRITE64(const void *pMem,  MT_U32 size, PVR_FILE64 file)
{
    ssize_t ret;
    int pvrFd = PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);
    pPvrFile = &g_stPvrFiles[pvrFd];

    ret = PVR_PWRITE64(NULL, pMem, size, file, pPvrFile->u64SeekOffset);

    return ret;
}

ssize_t PVR_SEEK64(PVR_FILE64 file, MT_S64 offset, int whence)
{
    int pvrFd = PVRFileGetPVRFd(file);
    PVR_FILE_S *pPvrFile;

    PVRFileCheckPVRFd(pvrFd);
    PVRFileCheckPVRFdOpen(pvrFd);

    pPvrFile = &g_stPvrFiles[pvrFd];

    switch (whence)
    {
        case SEEK_SET :
            if (offset < 0)
            {
                pPvrFile->u64SeekOffset = 0;
            }
            else
            {
                pPvrFile->u64SeekOffset = (MT_U64)offset;
            }
            break;
        case SEEK_CUR :
            pPvrFile->u64SeekOffset = (pPvrFile->u64SeekOffset + (MT_U64)offset);
            break;
        case SEEK_END:
            return -1;
            //break;
        default:
            return -1;
    }

    return (ssize_t)(pPvrFile->u64SeekOffset);
}




int PVR_Mount(const char *source, const char *target,
                 const char *filesystemtype, unsigned long mountflags,
                 const void *data)
{
    int ret = -1;
    MT_BOOL bVFAT = MT_TRUE;
    /*MT_BOOL bNTFS = MT_FALSE;*/
    MT_BOOL bHIFAT = MT_FALSE;
#ifdef SUPPORT_HIMOUNT
    if (MT_NULL != strcasestr(filesystemtype, "HIFAT"))
    {
        bHIFAT = MT_TRUE;
    }
#endif
    //lint -e774
	/* not supported */
	/*
    if (MT_TRUE == bNTFS && MT_TRUE == bHIFAT)
    {
        bVFAT = MT_TRUE;
        bNTFS = MT_FALSE;
    }*/

    if (MT_TRUE == bVFAT)
    {
#ifdef SUPPORT_HIMOUNT
    if (MT_TRUE == bHIFAT)
    {
        ret = himount(source, target);
        MT_ERR_PVR("himount(%s,%s) return %d.\n", source, target, ret);
    }
    else
    {
        char system_mount_command[512];
        snprintf(system_mount_command, sizeof(system_mount_command)-1, "mount %s %s", source, target);
        system_mount_command[sizeof(system_mount_command)-1] = 0;
        ret = system((const char*)system_mount_command);
        MT_ERR_PVR("\"%s\" return %d.\n", system_mount_command, ret);
    }
#else
    ret = mount(source, target, "vfat", 0, NULL);
    MT_INFO_PVR("mount(%s,%s, \"vfat\") return %d.\n", source, target, ret);
#endif
    }
    mountflags = 0;
    data = NULL;
    //lint +e774

    MT_INFO_PVR("filesystemtype =%s\n",filesystemtype);
    UNUSED(bHIFAT);
    UNUSED(data);
    UNUSED(mountflags);
    return ret;
}
/*
int PVR_Umount(const char *target)
{
    return pvr_umount(target);
}
*/
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

