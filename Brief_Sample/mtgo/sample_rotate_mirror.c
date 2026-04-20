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



#define MTGO_DECPIC_PRINT        printf


#define MAX_FILENAME_LEN 256
static MT_BOOL g_bTaskQuit = MT_TRUE;
#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2




static mt_s32 date[5][4] = {
{400,50,200,300},
{650,50,300,200},
{1000,50,200,300},
{50,400,300,200},
{400,400,300,200}
};



typedef struct
{
    mt_char picname[256];
    pthread_t Cmdtask;
    mt_handle hLayer;
    mt_handle hDecSurface;
    mt_handle hLayerSurface;
    mt_handle memSurface;
}picture_info_t;

static picture_info_t g_pic_info;

#ifdef MT_SAMPLE_APP
    mt_s32 MT_RotateMirrorMain(mt_s32 argc, mt_char *argv[]);
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_s32 MT_MtgoRoMiFilesize(int file)
{
    mt_s32 curpos = 0;
    mt_s32 length = 0;
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
static mt_s32 MT_MtgoRoMi(mt_char *pp_FileName, mt_handle *p_Surface)
{
    mt_s32 ret = 0;
    mt_handle hDecoder = 0;//Handle to the decoding
    MTGO_DEC_ATTR_S stSrcDesc = { 0 };//Input stream object
    MTGO_DEC_IMGINFO_S stImgInfo = { 0 };//Image format
    MTGO_DEC_IMGATTR_S stAttr = { 0 };

    mt_u32 file_size = 0;
    mt_s32 file = 0;
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

    file_size = MT_MtgoRoMiFilesize(file);
    if (file_size <= 0)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_MtgoRoMiFilesize err!\n");
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
        stAttr.Width = 4095;
    }
    else
    {
        stAttr.Width = stImgInfo.Width;
    }

    if (stImgInfo.Height > 4095)
    {
        stAttr.Height = 4095;
    }
    else
    {
        stAttr.Height = stImgInfo.Height;
    }

    stAttr.Format = MTGO_PF_8888;
    if (stImgInfo.Width > 4095 || stImgInfo.Height > 4095)
    {
        ret = MT_GO_DecImgData(hDecoder, 0, &stAttr, p_Surface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgData1 error. ret=0x%x \n", ret);
            return ret;
        }
    }
    else
    {
        ret = MT_GO_DecImgData(hDecoder, 0, NULL, p_Surface);
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

//Use the help
static mt_void MT_MtgoRoMiHelp(char *name)
{
    MTGO_PRINT("sample usage: \n");
    MTGO_PRINT(" %s -f 1.jpg\n", name);
    MTGO_PRINT(" %s -q  <exit> \n", name);
    return;
}
static MT_VOID MT_MtgoRoMiPrintMenu(MT_VOID)
{
    MTGO_PRINT("     x : Rotate \n");
    MTGO_PRINT("     z : Mirror \n");
    MTGO_PRINT("     c : Mirror and Rotate\n");
#ifdef MT_SAMPLE_APP
    MTGO_PRINT("     b : background run \n");
#endif
    MTGO_PRINT("     h : help \n");
    MTGO_PRINT("     q : quit \n");
    MTGO_PRINT("Romi>> ");

}
static mt_s32 MT_MtgoRoMiTask(void *arg)
{
    mt_s32 ret = 0;
    MT_RECT stRect = { 0 };
    MTGO_BLTOPT_S stBltOpt = { 0 };
    mt_char *p_pos = NULL;
    MT_RECT dtRect = {0};




    /**decoding*/
    p_pos = strrchr(g_pic_info.picname, '.');
    if (NULL == p_pos)
    {
        return 0;
    }
    p_pos++;
    if(strcasecmp(p_pos, "gif") == 0)
    {
        printf("is gif to err \n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }
    else
    {

        ret = MT_MtgoRoMi(g_pic_info.picname, &g_pic_info.hDecSurface);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("dec fail ret (%d)\n", ret);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }



        ret = MT_GO_Blit(g_pic_info.hDecSurface, NULL, g_pic_info.memSurface, NULL, &stBltOpt);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", ret);
            return MT_FAILURE;
        }
        stRect.x = 0;
        stRect.y = 0;
        stRect.w = 300;
        stRect.h = 200;
        dtRect.x = 50;
        dtRect.y = 50;
        dtRect.w = 300;
        dtRect.h = 200;
        /** Blit it to graphic layer Surface */
        ret = MT_GO_Blit(g_pic_info.memSurface, &stRect, g_pic_info.hLayerSurface, &dtRect, &stBltOpt);
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

        while(g_bTaskQuit != MT_TRUE)
        {
            usleep(500);
        }



    }
    return MT_SUCCESS;
}


static mt_s32 MT_MtgoRoMiCmdTask(MT_VOID)
{

    mt_s32 ret = 0;
    char *fgetret=NULL;
    mt_s8 inputCmd[32];
    MT_RECT stRect = { 0 };
    MT_RECT dtRect = { 0 };
    MTGO_BLTOPT_S stBltOpt = { 0 };
    mt_s32 Rotate = 0;
    mt_s32 Mirror = 0;

    stBltOpt.PixelAlphaComp = MTGO_COMPOPT_NONE;

    stRect.x = 0;
    stRect.y = 0;
    stRect.w = 300;
    stRect.h = 200;

    int i = -1;
    int j = 2;
    while (g_bTaskQuit == MT_FALSE)
    {
        (MT_VOID)MT_MtgoRoMiPrintMenu();
        fgetret=fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret=fgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else if('c' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("The image is mirrored left and right, and then rotated 90 degrees clockwise to display the image \n");
            dtRect.x = 750;
            dtRect.y = 400;
            dtRect.w = 200;
            dtRect.h = 300;
            stBltOpt.RotateType = 1;
            stBltOpt.MirrorType = 1;
            ret = MT_GO_Blit(g_pic_info.memSurface, &stRect, g_pic_info.hLayerSurface, &dtRect, &stBltOpt);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", ret);
            }
            (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
        }
        else if('z' == inputCmd[0])
        {
            j++;
            if(j == 3)
            {
                SAMPLE_MTGO_INFO_PRINT("The left and right mirrors show the picture \n");
            }
            else if(j == 4)
            {
                SAMPLE_MTGO_INFO_PRINT("The image is mirrored up and down \n");
            }
            else
            {
                SAMPLE_MTGO_ERR_PRINT("Mirror is over!! \n");
                continue;
            }
            dtRect.x = date[j][0];
            dtRect.y = date[j][1];
            dtRect.w = date[j][2];
            dtRect.h = date[j][3];
            Mirror += 1;
            stBltOpt.RotateType = 0;
            stBltOpt.MirrorType = Mirror;
            ret = MT_GO_Blit(g_pic_info.memSurface, &stRect, g_pic_info.hLayerSurface, &dtRect, &stBltOpt);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", ret);
            }
            (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
        }
        else if('x' == inputCmd[0])
        {
            i++;
            if(i == 0)
            {
                SAMPLE_MTGO_INFO_PRINT("Flip the image 90 degrees \n");
            }
            else if(i == 1)
            {
                SAMPLE_MTGO_INFO_PRINT("Flip the image 180 degrees \n");
            }
            else if(i == 2)
            {
                SAMPLE_MTGO_INFO_PRINT("Flip the image 270 degrees \n");
            }
            else
            {
                SAMPLE_MTGO_ERR_PRINT("Rotate is over!! \n");
                continue;
            }
            dtRect.x = date[i][0];
            dtRect.y = date[i][1];
            dtRect.w = date[i][2];
            dtRect.h = date[i][3];
            Rotate += 1;
            stBltOpt.RotateType = Rotate;
            stBltOpt.MirrorType = 0;
            ret = MT_GO_Blit(g_pic_info.memSurface, &stRect, g_pic_info.hLayerSurface, &dtRect, &stBltOpt);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", ret);
            }

            (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
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

static MT_VOID MT_MtgoRoMiExit(MT_VOID)
{
    int ret = 0;

     g_bTaskQuit = MT_TRUE;



//FIX BUG: free(): double free detected in tcache 2
//         already called MT_GO_DestroyLayer()
//    ret = MT_GO_FreeSurface(g_pic_info.hLayerSurface);
    ret = MT_GO_FreeSurface(g_pic_info.memSurface);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_FreeSurface error. ret=0x%x \n", ret);
    }
        ret = MT_GO_FreeSurface(g_pic_info.hDecSurface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_FreeSurface error. ret=0x%x \n", ret);
        }

    (MT_VOID)MT_GO_DestroyLayer(g_pic_info.hLayer);
    (MT_VOID)pthread_join(g_pic_info.Cmdtask, NULL);
    (MT_VOID)MT_GO_Deinit();
    memset(&g_pic_info, 0, sizeof(picture_info_t));
}

static MT_S32 MT_MtgoRoMiParase_args(MT_S32 argc, MT_CHAR *argv[], picture_info_t *ppic_info)
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
                (MT_VOID)MT_MtgoRoMiHelp(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MtgoRoMiExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                memcpy(ppic_info->picname, mt_optarg, strlen(mt_optarg));
                break;
            default:
                SAMPLE_MTGO_ERR_PRINT("ERR opt[%c], print help info.\n", opt);
                (MT_VOID)MT_MtgoRoMiHelp(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
    mt_s32 MT_RotateMirrorMain(mt_s32 argc, mt_char *argv[])
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32 ret = 0;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    MTGO_LAYER_E eLayerID = MTGO_LAYER_OSD0;



    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        MT_MtgoRoMiHelp(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_MtgoRoMiParase_args(argc, argv, &g_pic_info);
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

        ret = MT_GO_GetLayerDefaultParam(eLayerID, &stLayerInfo);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerDefaultParam error. ret=0x%x \n", ret);
            goto ERR1;
        }

        stLayerInfo.PixelFormat = MTGO_PF_8888;
        stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;
        stLayerInfo.CanvasWidth = 1280;
        stLayerInfo.CanvasHeight = 720;
        stLayerInfo.DisplayWidth = 1280;
        stLayerInfo.DisplayHeight = 720;
        ret = MT_GO_CreateLayer(&stLayerInfo, &g_pic_info.hLayer);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateLayer error. ret=0x%x \n", ret);
            goto ERR1;
        }

        ret = MT_GO_GetLayerSurface(g_pic_info.hLayer, &g_pic_info.hLayerSurface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerSurface error. ret=0x%x \n", ret);
            goto ERR2;
        }

        ret = MT_GO_CreateSurface(300, 200, 9, &g_pic_info.memSurface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_GetLayerSurface error. ret=0x%x \n", ret);
            goto ERR2;
        }

        SAMPLE_MTGO_INFO_PRINT("Start to decoding  file: %s\n", g_pic_info.picname);

        g_bTaskQuit = MT_FALSE;
        ret = pthread_create(&g_pic_info.Cmdtask, NULL, (void * (*)(void *))MT_MtgoRoMiTask, NULL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("pthread_create error. ret=0x%x \n", ret);
            goto ERR2;
        }




    }
    (MT_VOID)MT_MtgoRoMiCmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

ERR2:

    (MT_VOID)MT_MtgoRoMiExit();

ERR1:
#ifndef MT_SAMPLE_APP
        Sample_MTGO_Display_DeInit();
#endif

        SAMPLE_MTGO_FUNCTION_EXIT();
        return MT_SUCCESS;
}

