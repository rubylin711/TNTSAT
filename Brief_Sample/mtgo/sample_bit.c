#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include "mt_go.h"
#include "mt_go_decoder.h"
#include "mt_unf_video.h"
#include "mt_unf_disp.h"
#include "mt_common.h"
#include "sample_mtgo_common.h"
#include "fcntl.h"
#include <pthread.h>
#include "mt_adp_mpi.h"

#define MTGO_BIT_DECPIC_PRINT        printf
#define MAX_FILENAME_LEN 256
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

typedef enum
{
    BIT_TYPE_16 = 1,
    BIT_TYPE_24,
    BIT_TYPE_32,
    BIT_TYPE_UYVY,
    BIT_TYPE_YUV8888,
}BIT_TYPE;

typedef struct
{
    mt_handle BkSurface;  //the tmp bksurface for Render;
    mt_handle DecHandle;  //Handle to the decoding
    MT_RECT   BKRect;    //The last frame
}animation_info_t;

typedef struct
{
    mt_char picname[1024];
    pthread_t Cmdtask;
    mt_handle hLayer;
    mt_handle hDecSurface;
    mt_handle hLayerSurface;
    mt_handle hRender;
    MT_BOOL   type;
}picture_info_t;

static MT_BOOL g_bTaskQuit = MT_TRUE;
static MT_BOOL g_decPic = MT_FALSE;
static MTGO_DEC_IMGATTR_S g_stAttr = { 0 };
static picture_info_t g_pic_info;

#ifdef MT_SAMPLE_APP
MT_S32 MT_MtgoBitDecpicMain(MT_S32 argc, mt_char *argv[]);
#else
MT_S32 main(MT_S32 argc, mt_char *argv[]);
#endif

static MT_S32 MT_MtgoBitDecpicFilesize(int file)
{
    MT_S32 curpos = 0;
    MT_S32 length = 0;
    curpos = lseek(file, 0, SEEK_CUR);
    lseek(file, 0L, SEEK_SET);
    length = lseek(file, 0L, SEEK_END);
    lseek(file, curpos, SEEK_SET);
    return length;
}


/*
@brief Decoding picture
@param[in] pp_FileName,The file name to decode
@param[in] p_Surface,decode surface
@return ::MT_SUCCESS
*/
static MT_S32 MT_MtgoBitDecpic(mt_char *pp_FileName, mt_handle *p_Surface, MTGO_PF_E format)
{
    MT_S32 ret = 0;
    mt_handle hDecoder = 0;//Handle to the decoding
    MTGO_DEC_ATTR_S stSrcDesc = { 0 };//Input stream object
    MTGO_DEC_IMGINFO_S stImgInfo = { 0 };//Image format

    mt_u32 file_size = 0;
    MT_S32 file = 0;
    mt_u32 read_size = 0;
    mt_char *p_buf = NULL;

    MT_ASSERT(pp_FileName != NULL);
    MT_ASSERT(p_Surface != NULL);

    file = open(pp_FileName, O_RDONLY);
    if (file < 0)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, fail to read file!\n", __FUNCTION__);
        return MT_FAILURE;
    }

    file_size = MT_MtgoBitDecpicFilesize(file);
    if (file_size <= 0)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_MtgoBitDecpicFilesize err!\n");
        return MT_FAILURE;
    }
    p_buf = (mt_char *)malloc(file_size);
    if (p_buf == NULL)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, err: no buffer \n", __FUNCTION__);
        return MT_FAILURE;
    }

    read_size = read(file, p_buf, file_size);
    if (read_size != file_size)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, read size error: %d!\n", __FUNCTION__, read_size);
        free(p_buf);
    }


    /*Create the decoding handle*/
    stSrcDesc.SrcType = MTGO_DEC_SRCTYPE_FILE;
    stSrcDesc.SrcInfo.pFileName = pp_FileName;
    stSrcDesc.SrcInfo.MemInfo.pAddr = p_buf;
    stSrcDesc.SrcInfo.MemInfo.Length = file_size;
    ret = MT_GO_CreateDecoder(&stSrcDesc, &hDecoder);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateDecoder error. ret=0x%x \n", ret);
        return ret;
    }

    ret = MT_GO_DecImgInfo(hDecoder, 0, &stImgInfo);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgInfo error. ret=0x%x \n", ret);
        MT_GO_DestroyDecoder(hDecoder);
        return ret;
    }

    /** Decoding to the surface The maximum hardware size is 4095, beyond which the display will not be complete, so scale to this value*/
    if (stImgInfo.Width > 4095)
    {
        g_stAttr.Width = 4095;
    }
    else
    {
        g_stAttr.Width = stImgInfo.Width;
    }

    if (stImgInfo.Height > 4095)
    {
        g_stAttr.Height = 4095;
    }
    else
    {
        g_stAttr.Height = stImgInfo.Height;
    }

    g_stAttr.Format = format;
    if (stImgInfo.Width > 4095 || stImgInfo.Height > 4095)
    {
        ret = MT_GO_DecImgData(hDecoder, 0, &g_stAttr, p_Surface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgData1 error. ret=0x%x \n", ret);
            return ret;
        }
    }
    else
    {
        ret = MT_GO_DecImgData(hDecoder, 0, &g_stAttr, p_Surface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgData2 error. ret=0x%x \n", ret);
            return ret;
        }
    }

    ret = MT_GO_DestroyDecoder(hDecoder);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DestroyDecoder error. ret=0x%x \n", ret);
        return ret;
    }
    free(p_buf);
    return ret;
}


static MT_S32 MT_MtgoBitDecpicTask(void *arg)
{
    MT_S32 ret = 0;
    MT_RECT stRect = { 0 };
    MTGO_BLTOPT_S stBltOpt = { 0 };
    MTGO_PF_E *format = (MTGO_PF_E*)arg;

    g_pic_info.type = 0;
    ret = MT_MtgoBitDecpic(g_pic_info.picname, &g_pic_info.hDecSurface, *format);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("dec fail ret (%d)\n", ret);
        g_bTaskQuit = MT_TRUE;
        g_decPic = MT_FALSE;
        return MT_FAILURE;
    }


    if(g_stAttr.Width > 1280)
    {
        g_stAttr.Width = 1280;
    }
    if(g_stAttr.Height > 720)
    {
        g_stAttr.Height = 720;
    }
    stRect.x = (1280 - g_stAttr.Width)/2;
    stRect.y = (720 - g_stAttr.Height)/2;
    stRect.w = g_stAttr.Width;
    stRect.h = g_stAttr.Height;
    /** Blit it to graphic layer Surface */
    ret = MT_GO_Blit(g_pic_info.hDecSurface, NULL, g_pic_info.hLayerSurface, &stRect, &stBltOpt);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", ret);
    }
    /** fresh display*/
    ret = MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", ret);
    }

    while(g_bTaskQuit != MT_TRUE && g_decPic != MT_FALSE)
    {
        usleep(500);
    }
    ret = MT_GO_FreeSurface(g_pic_info.hDecSurface);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_FreeSurface error. ret=0x%x \n", ret);
    }

    return MT_SUCCESS;
}



static MT_S32 MT_MtgoBitCreateDecpic(MTGO_PF_E format)
{
    MT_S32 ret = 0;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    MTGO_LAYER_E eLayerID = MTGO_LAYER_OSD0;

    ret = MT_GO_GetLayerDefaultParam(eLayerID, &stLayerInfo);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerDefaultParam error. ret=0x%x \n", ret);
        return MT_FAILURE;
    }

    stLayerInfo.PixelFormat = format;
    stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;


    ret = MT_GO_CreateLayer(&stLayerInfo, &g_pic_info.hLayer);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateLayer error. ret=0x%x \n", ret);
        return MT_FAILURE;
    }

    ret = MT_GO_GetLayerSurface(g_pic_info.hLayer, &g_pic_info.hLayerSurface);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerSurface error. ret=0x%x \n", ret);
        goto ERR0;
    }

    ret = MT_GO_SetLayerAlpha(g_pic_info.hLayer, 0xff);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_SetLayerAlpha error. ret=0x%x \n", ret);
        goto ERR0;
    }

    ret = MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", ret);
        goto ERR0;
    }
    SAMPLE_MTGO_INFO_PRINT("Start to decoding  file: %s\n", g_pic_info.picname);

    g_decPic = MT_TRUE;

    ret = pthread_create(&g_pic_info.Cmdtask, NULL, (void * (*)(void *))MT_MtgoBitDecpicTask, &format);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("pthread_create error. ret=0x%x \n", ret);
        goto ERR0;
    }

    return MT_SUCCESS;
ERR0:
    (MT_VOID)MT_GO_DestroyLayer(g_pic_info.hLayer);
    return MT_FAILURE;
}

static MT_VOID MT_MtgoBitDestroyDecpic(MT_VOID)
{
    g_decPic = MT_FALSE;

    (MT_VOID)pthread_join(g_pic_info.Cmdtask, NULL);

    (MT_VOID)MT_GO_DestroyLayer(g_pic_info.hLayer);
}


static MT_VOID MT_MtgoBitDecpicPrintMenu(MT_VOID)
{
    MTGO_PRINT("     a: select the image format\n");
#ifdef MT_SAMPLE_APP
    MTGO_PRINT("     b: background run\n");
#endif
    MTGO_PRINT("     h: help\n");
    MTGO_PRINT("     q: quit\n");
    MTGO_PRINT("Bit>> ");

}

static MTGO_PF_E MT_MtgoBitDecpicType(MT_S32 type)
{
    switch(type)
    {
        case BIT_TYPE_16:
            return MTGO_PF_4444;
        case BIT_TYPE_24:
            return MTGO_PF_8565;
        case BIT_TYPE_32:
            return MTGO_PF_8888;
        case BIT_TYPE_UYVY:
            return MTGO_PF_UYVY;
        case BIT_TYPE_YUV8888:
            return MTGO_PF_YUV8888;
        default:
            SAMPLE_MTGO_ERR_PRINT("The input format is incorrect\n");
            return MTGO_PF_BUTT;
    }

    return MTGO_PF_BUTT;
}

static MT_S32 MT_MtgoBitDecpicCmdTask(MT_VOID)
{
    MT_S32 s32Ret = 0;
    MT_S32 set = 0;
    char *fgetret = NULL;
    mt_s8 inputCmd[32];
    MTGO_PF_E format = 0;

    while(g_bTaskQuit == MT_FALSE)
    {
        (MT_VOID)MT_MtgoBitDecpicPrintMenu();
        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            g_decPic = MT_FALSE;
            break;
        }
        else if ('a' == inputCmd[0])
        {
            MTGO_PRINT("Select the image format(1: 16bit, 2: 24bit, 3: 32bit, 4: UYVY, 5: YUV8888): ");
            scanf("%d", &set);
            getchar();

            format = MT_MtgoBitDecpicType(set);
            if(MTGO_PF_BUTT == format)
            {
                continue;
            }

            (MT_VOID)MT_MtgoBitDestroyDecpic();
            s32Ret = MT_MtgoBitCreateDecpic(format);
            if(MT_FAILURE == s32Ret)
            {
                SAMPLE_MTGO_ERR_PRINT("Image decoding failed\n");
            }
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("MTGO decpic play in back!\n");
            break;
        }
#endif
        else if('h' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("print help menu \n");
        }
    }

    return 0;
}


static MT_VOID MT_MtgoBitDecpicExit(MT_VOID)
{
    int ret = 0;
    if(g_bTaskQuit == MT_FALSE)
    {
        ret = MT_GO_FreeSurface(g_pic_info.hDecSurface);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_FreeSurface error. ret=0x%x \n", ret);
        }
    }
    g_bTaskQuit = MT_TRUE;
    (MT_VOID)pthread_join(g_pic_info.Cmdtask, NULL);
    (MT_VOID)MT_GO_DestroyLayer(g_pic_info.hLayer);
    (MT_VOID)MT_GO_Deinit();
    memset(&g_pic_info, 0, sizeof(picture_info_t));
}

static mt_void MT_MtgoBitDecpicHelp(char *name)
{
    MTGO_PRINT("sample usage: \n");
    MTGO_PRINT(" %s -f 1.jpg\n", name);
    MTGO_PRINT(" %s -q  <exit> \n", name);
    return;
}

static MT_S32 MT_MtgoBitDecpicParase_args(MT_S32 argc, MT_CHAR *argv[], picture_info_t *ppic_info)
{
    MT_S32 opt = 0;
    SAMPLE_MTGO_FUNCTION_ENTER();

    while((opt = MTADP_Getopt(argc, argv, ":?hHf:q")) != -1)
    {
        switch(opt)
        {
            case '?':
            case 'h':
            case 'H':
                SAMPLE_MTGO_INFO_PRINT("Get opt[%c], run help info.\n", opt);
                (MT_VOID)MT_MtgoBitDecpicHelp(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MtgoBitDecpicExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                memcpy(ppic_info->picname, mt_optarg, strlen(mt_optarg));
                break;
            default:
                SAMPLE_MTGO_ERR_PRINT("ERR opt[%c], print help info.\n", opt);
                (MT_VOID)MT_MtgoBitDecpicHelp(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_MtgoBitDecpicMain(MT_S32 argc, mt_char *argv[])
#else
MT_S32 main(MT_S32 argc, mt_char *argv[])
#endif

{
    MT_S32 ret = 0;

    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        MT_MtgoBitDecpicHelp(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_MtgoBitDecpicParase_args(argc, argv, &g_pic_info);
    if (MT_FAILURE == ret)
    {
        SAMPLE_MTGO_INFO_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == ret)
    {
        SAMPLE_MTGO_INFO_PRINT("Recv stop command. stop window.\n");

        memset(&g_pic_info, 0, sizeof(g_pic_info));

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
#ifndef MT_SAMPLE_APP
        ret = Sample_MTGO_Display_Init(MT_UNF_ENC_FMT_720P_60);
        if (MT_FAILURE == ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MTGO_Display_Init err. ret= 0x%x.\n", ret);
            return MT_FAILURE;
        }
#endif
        ret = MT_GO_Init();
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_Init error. ret=0x%x \n", ret);
            goto ERR1;
        }

        g_bTaskQuit = MT_FALSE;

        ret = MT_MtgoBitCreateDecpic(MTGO_PF_8888);
        if(MT_FAILURE == ret)
        {
            SAMPLE_MTGO_ERR_PRINT("Image decoding failed\n");
        }
    }
    (MT_VOID)MT_MtgoBitDecpicCmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

    (MT_VOID)MT_MtgoBitDecpicExit();

ERR1:
#ifndef MT_SAMPLE_APP
        Sample_MTGO_Display_DeInit();
#endif

        SAMPLE_MTGO_FUNCTION_EXIT();
        return MT_SUCCESS;
}

