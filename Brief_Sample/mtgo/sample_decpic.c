#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/mman.h>
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


//Image properties after decoding
static MTGO_DEC_IMGATTR_S g_stAttr = { 0 };



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
    mt_char *buf;
}picture_info_t;

static char* g_mem_type_str[] = {
    "MEM_NOM",
    "MEM_64M",
    "MEM_128M",
    "MEM_256M",
    "MEM_512M",
    "MEM_1024M",
    "MEM_2048M",
    "MEM_4096M"
};

static picture_info_t g_pic_info;

#ifdef MT_SAMPLE_APP
    mt_s32 MT_MtgoDecPicMain(mt_s32 argc, mt_char *argv[]);
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_s32 MT_MtgoDecpicFilesize(int file)
{
    mt_s32 curpos = 0;
    mt_s32 length = 0;
    curpos = lseek(file, 0, SEEK_CUR);
    lseek(file, 0L, SEEK_SET);
    length = lseek(file, 0L, SEEK_END);
    lseek(file, curpos, SEEK_SET);
    return length;
}

// read the register value by the absolute address
// and this code is refer from the devmem.c
static int read_reg_devmem(unsigned int reg_addr, unsigned int sizes, unsigned int *value)
{
    unsigned int i = 0;
    unsigned int tmp = 0;
    void *virt_addr;
    void *map_base;
    unsigned int target;
    unsigned page_size, mapped_size, offset_in_page;
    int fd;
    unsigned width = 8 * sizes;

    if (0 == sizes || 0 != (sizes % 4) || 0 != (reg_addr % 4)) {
        SAMPLE_MTGO_ERR_PRINT("Invalid parameters!\n");
        return -1;
    }

    fd = open("/dev/mem", O_RDONLY | O_SYNC);
    if (fd < 0) {
        SAMPLE_MTGO_ERR_PRINT("[Error]: open /dev/mem error\n");
        return -1;
    }

    target = reg_addr;
    mapped_size = page_size = getpagesize();
    offset_in_page = (unsigned)target & (page_size - 1);

    if (offset_in_page + width > page_size) {
        /* This access spans pages.
         * Must map two pages to make it possible: */
        mapped_size *= 2;
    }

    map_base = mmap(NULL, mapped_size, PROT_READ, MAP_SHARED, fd,
                    target & ~(unsigned int)(page_size - 1));
    if (map_base == MAP_FAILED) {
        SAMPLE_MTGO_ERR_PRINT("mmap");
        return -1;
    }

    virt_addr = (char *)map_base + offset_in_page;
    do {
        tmp = *(volatile unsigned int *)((volatile unsigned char *)virt_addr + i);
        //printf("value at address 0x%X (%p): 0x%X\n", target, virt_addr + i, tmp);
        i = i + 4;
    } while (i < sizes);

    if (munmap(map_base, mapped_size) == -1)
        SAMPLE_MTGO_ERR_PRINT("munmap");

    close(fd);
    fd = -1;

    *value = tmp;

    return 0;
}

static int GetMemType(unsigned int *mem_type)
{
    int ret = -1;
    unsigned int reg_addr = 0xBF090000;
    unsigned int sizes = 4;
    unsigned int reg_value;

    ret = read_reg_devmem(reg_addr, sizes, &reg_value);
    if (ret != 0)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, fail to read reg devmem!\n", __FUNCTION__);
        return -1;
    }

    /* reg_addr:    0xBF090000[15:13]
     * 0: 0MB   * 1: 64MB   * 2: 128MB  * 3: 256MB  * 4: 512MB  * 5: 1024MB     * 6: 2048MB     * 7: 4096MB
     */
    *mem_type = (reg_value >> 13) & 7;

    return 0;
}

static int GetAvailableMem(unsigned int *MemAvailable)
{
    int n = 0;
    int val_f;
    FILE *fp = NULL;
    char buf[128];

    fp = fopen("/proc/meminfo", "r");
    if (NULL == fp) {
        printf("fopen failed %s \n", strerror(errno));
        return -1;
    }

    while (fgets(buf, 128, fp)) {
            //printf("sync line %s\n", buf);
            val_f = 0;
            n = sscanf(buf, "MemAvailable:      %d kB", &val_f);
            if (n == 1) {
                *MemAvailable = (unsigned int)val_f;
                break;
            }
    }

    fclose(fp);
    fp = NULL;

    return 0;
}

/*
@brief Decoding picture
@param[in] pp_FileName,The file name to decode
@param[in] p_Surface,decode surface
@return ::MT_SUCCESS
*/
static mt_s32 MT_MtgoDecpicDecImgData(mt_handle hDecoder, mt_handle *p_Surface)
{
    mt_s32 ret = 0;
    MTGO_DEC_IMGINFO_S stImgInfo = { 0 };//Image format
    mt_handle Surface = MT_INVALID_HANDLE;


    if (p_Surface == NULL)
    {
        SAMPLE_MTGO_ERR_PRINT("Input the parameter p_Surface is NULL. \n");
        return MT_FAILURE;
    }

    ret = MT_GO_DecImgInfo(hDecoder, 0, &stImgInfo);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgInfo error. ret=0x%x \n", ret);
        return ret;
    }

    g_stAttr.Width = stImgInfo.Width;
    g_stAttr.Height = stImgInfo.Height;
    g_stAttr.Format = MTGO_PF_8888;
    ret = MT_GO_DecImgData(hDecoder, 0, &g_stAttr, &Surface);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgData error. ret=0x%x \n", ret);
        return MT_FAILURE;
    }

    *p_Surface = Surface;

    return MT_SUCCESS;
err1:
    MT_GO_FreeSurface(Surface);

    return ret;
}

/*
@brief decode GIF
@param[in] p_FileName,The file name to decode
@param[out] p_hRender,Pointer to the outgoing handle
@param[in] p_PrimaryInfo,Major information after decoding
@return ::MT_SUCCESS
*/
static mt_s32 MT_MtgoDecpicCreateBKsurface_GIF(mt_handle *p_hRender, MTGO_DEC_PRIMARYINFO_S PrimaryInfo)
{
    mt_s32 ret = 0;
    animation_info_t *p_Render = NULL;
    mt_handle Surface = MT_INVALID_HANDLE;


    if (p_hRender == NULL)
    {
        SAMPLE_MTGO_ERR_PRINT("Input the parameter phRender is NULL. \n");
        return MT_FAILURE;
    }

    ret = MT_GO_CreateSurface(PrimaryInfo.ScrWidth, PrimaryInfo.ScrHeight, MTGO_PF_8888, &Surface);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateSurface error. ret=0x%x \n", ret);
        return ret;
    }

    p_Render = (animation_info_t *)malloc (sizeof(animation_info_t));
    if (p_Render == NULL)
    {
        SAMPLE_MTGO_ERR_PRINT("p_Render is NULL!\n");
        goto err1;
    }

    memset(p_Render, 0, sizeof(animation_info_t));
    p_Render->BkSurface = Surface;
    *p_hRender = (mt_handle)p_Render;

    return MT_SUCCESS;
err1:
    (MT_VOID)MT_GO_FreeSurface(Surface);

    return ret;
}

/*
@brief Destroy the decode handle
@param[in] hRender,handle
@return ::MT_SUCCESS
*/
static mt_s32 MT_MtgoDecpicDestroyBKsurface_GIF(mt_handle hRender)
{
    mt_s32 ret;
    animation_info_t *p_Render;

    p_Render = (animation_info_t *)hRender;
    if (p_Render == NULL)
    {
        return MT_FAILURE;
    }

    ret = MT_GO_FreeSurface(p_Render->BkSurface);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_FreeSurface error. ret=0x%x \n", ret);
        return ret;
    }

    free(p_Render);
    p_Render = NULL;

    return ret;
}

/*
@brief Displays the resulting information to the set fixed location surface
@paran[in] hRender,Handle to the decoding
@param[in] Index,DstSurface,According to surface
@param[in] GIF Picture number, starting from 0
@param[in] x0,y0,The starting position coordinate of the decoded image
@return ::MT_SUCCESS
*/
static mt_s32 MT_MtgoDecpicRenderFrame_GIF(mt_handle hRender, mt_handle DstSurface, mt_u32 Index, mt_s32 x0, mt_s32 y0)
{
    mt_s32 Disposal = 0;
    mt_s32 Ret = 0;
    MTGO_DEC_IMGINFO_S ImgInfo = { 0 };
    MTGO_DEC_IMGINFO_S LastImgInfo = { 0 };
    MTGO_DEC_PRIMARYINFO_S PrimaryInfo = { 0 };
    MT_COLOR BKColor = 0;
    MTGO_BLTOPT_S BltOpt = {0};
    mt_handle ImgSurface = 0;
    MTGO_BLTOPT_S BltOptKey = {0};
    MT_RECT Rect, DstRect, LastRect;
    animation_info_t *p_Render = NULL;

    p_Render = (animation_info_t *)hRender;

    Ret = MT_GO_DecCommInfo(p_Render->DecHandle, &PrimaryInfo);
    if (Ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecCommInfo error. Ret=0x%x \n", Ret);
        return Ret;
    }

    if (PrimaryInfo.IsHaveBGColor)
    {
        BKColor = PrimaryInfo.BGColor;
    }
    else
    {
        BKColor = 0xffffffff; //color is white
    }

    Ret = MT_GO_DecImgInfo(p_Render->DecHandle, Index, &ImgInfo);
    if (Ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgInfo error. Ret=0x%x \n", Ret);
        return Ret;
    }

    //get the screen rect in dstsurface;
    DstRect.x = x0;
    DstRect.y = y0;
    DstRect.w = PrimaryInfo.ScrWidth;
    DstRect.h = PrimaryInfo.ScrHeight;

    if (0 == Index)
    {

        Ret = MT_GO_FillRect(DstSurface, &DstRect, BKColor, MTGO_COMPOPT_NONE);
        if (Ret != MT_SUCCESS)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_FillRect error. Ret=0x%x \n", Ret);
            return MT_FAILURE;
        }
    }
    else
    {
        mt_s32 LastDisposal;
        // get the last frame disposal method;

        Ret = MT_GO_DecImgInfo(p_Render->DecHandle, (Index - 1), &LastImgInfo);
        if (Ret != MT_SUCCESS)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgInfo error. Ret=0x%x \n", Ret);
            return MT_FAILURE;
        }

        LastDisposal = LastImgInfo.DisposalMethod;

        //get the last rect
        LastRect.x = x0 + LastImgInfo.OffSetX;
        LastRect.y = y0 + LastImgInfo.OffSetY;
        LastRect.w = LastImgInfo.Width;
        LastRect.h = LastImgInfo.Height;
        /*The decoder cleans the canvas with the background color and then renders the next image. The background color is set in the logical screen descriptor.*/
        if (2 == LastDisposal)
        {
            //fill background;
            Ret = MT_GO_FillRect(DstSurface, &LastRect, BKColor, MTGO_COMPOPT_NONE);
            if (Ret != MT_SUCCESS)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_FillRect error. Ret=0x%x \n", Ret);
                return MT_FAILURE;
            }
        }

    }

    Disposal = ImgInfo.DisposalMethod;
    if (3 == Disposal)    //copy DstSurface to back surface, back the whole frame info
    {
        Ret = MT_GO_Blit(DstSurface, &DstRect, p_Render->BkSurface, NULL, &BltOpt);
        if (Ret != MT_SUCCESS)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. Ret=0x%x \n", Ret);
            return MT_FAILURE;
        }
    }

    //fill the new surface
    Ret = MT_GO_DecImgData(p_Render->DecHandle, Index, NULL, &ImgSurface);    // the imgatt must be null
    if (Ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgData error. Ret=0x%x \n", Ret);
        return MT_FAILURE;
    }

    Rect.x = ImgInfo.OffSetX + x0;
    Rect.y = ImgInfo.OffSetY + y0;
    Rect.w = ImgInfo.Width;
    Rect.h = ImgInfo.Height;
    if (ImgInfo.IsHaveKey)
    {
        BltOptKey.ColorKeyFrom = MTGO_CKEY_SRC;
    }

    Ret = MT_GO_Blit(ImgSurface, MT_NULL, DstSurface, &Rect, &BltOptKey );
    if (Ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("Display_Init error. Ret=0x%x \n", Ret);
        goto err;
    }

    Ret = MT_GO_FreeSurface(ImgSurface);
    if (Ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgData error. Ret=0x%x \n", Ret);
        return MT_FAILURE;
    }
    return MT_SUCCESS;
err:
    (MT_VOID)MT_GO_FreeSurface(ImgSurface);

    return Ret;
}

//Use the help
static mt_void MT_MtgoDecpicHelp(char *name)
{
    MTGO_PRINT("sample usage: \n");
    MTGO_PRINT(" %s -f 1.jpg\n", name);
    MTGO_PRINT(" %s -q  <exit> \n", name);
    return;
}
static MT_VOID MT_MtgoDecpicPrintMenu(MT_VOID)
{
    MTGO_PRINT("Supports only 2 zooming in and out \n");
    MTGO_PRINT("     s : shrink \n");
    MTGO_PRINT("     z : enlarge \n");
    MTGO_PRINT("     r : restore \n");
#ifdef MT_SAMPLE_APP
    MTGO_PRINT("     b : background run \n");
#endif
    MTGO_PRINT("     h : help \n");
    MTGO_PRINT("     q : quit \n");
    MTGO_PRINT("Decpic>> ");

}

static mt_s32 MT_MtgoCreateDecoder(mt_char *pp_FileName, mt_handle *p_DecHanle, mt_char **pbuf)
{
    mt_s32 ret = 0;
    mt_handle hDecoder = 0;//Handle to the decoding
    MTGO_DEC_ATTR_S stSrcDesc = { 0 };//Input stream object
    mt_u32 file_size = 0;
    mt_s32 file = 0;
    mt_u32 read_size = 0;
    mt_char *p_buf = NULL;
    unsigned int mem_type;
    unsigned int mem_available;


    if (pp_FileName == NULL)
    {
        SAMPLE_MTGO_ERR_PRINT("[%s] %s is NULL !!!\n", __FUNCTION__, pp_FileName);
        return MT_FAILURE;
    }

    file = open(pp_FileName, O_RDONLY);
    if (file < 0)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, fail to read file!\n", __FUNCTION__);
        return MT_FAILURE;
    }

    ret = GetMemType(&mem_type);
    if (ret != 0)
    {
        SAMPLE_MTGO_ERR_PRINT("GetMemType err!\n");
        goto err1;
    }

    ret = GetAvailableMem(&mem_available);
    if (ret != 0)
    {
        SAMPLE_MTGO_ERR_PRINT("GetAvailableMem err!\n");
        goto err1;
    }

    file_size = MT_MtgoDecpicFilesize(file);
    if (file_size <= 0)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_MtgoDecpicFilesize err!\n");
        goto err1;
    }

    /* The image file size cannot exceed the actual available memory, with a margin of 4M */
    if (file_size > (mem_available - 4*1024*1024) * 1024) {
        SAMPLE_MTGO_ERR_PRINT("%s, [%s]available memorry is not enough, please change an smaller image file\n",
            g_mem_type_str[mem_type], __FUNCTION__);
        ret = MT_FAILURE;
        goto err1;
    }

    p_buf = (mt_char *)malloc(file_size);
    if (p_buf == NULL)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, err: no buffer \n", __FUNCTION__);
        ret = MT_FAILURE;
        goto err1;
    }

    read_size = read(file, p_buf, file_size);
    if (read_size != file_size)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, read size error: %d!\n", __FUNCTION__, read_size);
        ret = MT_FAILURE;
        goto err2;
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
        goto err2;
    }

    *p_DecHanle = hDecoder;
    *pbuf = p_buf;

    close(file);
    file = 0;

    return MT_SUCCESS;
err2:
    free(p_buf);
    p_buf = NULL;

err1:
    close(file);
    file = 0;

    return ret;
}

static mt_s32 MT_MtgoDestoryDecoder(mt_handle DecHanle)
{
    mt_s32 ret = 0;

    ret = MT_GO_DestroyDecoder(DecHanle);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DestroyDecoder error. ret=0x%x \n", ret);
    }

    free(g_pic_info.buf);
    g_pic_info.buf = NULL;

    return ret;
}

static mt_s32 MT_MtgoDecpicTask(void *arg)
{
    mt_s32 ret = 0;
    MT_RECT stRect = { 0 };
    MTGO_BLTOPT_S stBltOpt = { 0 };
    MTGO_DEC_PRIMARYINFO_S PrimaryInfo = { 0 };
    mt_u32 Index = 0;
    mt_s32 x0, y0;
    mt_handle DecHanle;
    animation_info_t *p_Render = NULL;


    ret = MT_MtgoCreateDecoder(g_pic_info.picname, &DecHanle, &g_pic_info.buf);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_MtgoCreateDecoderAndGetDecCommInfo error. ret=0x%x \n", ret);
        return MT_FAILURE;
    }

    ret = MT_GO_DecCommInfo(DecHanle, &PrimaryInfo);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecCommInfo error. Ret=0x%x \n", ret);
        goto err1;
    }

    if(PrimaryInfo.ImgType == MTGO_DEC_IMGTYPE_GIF)
    {
        g_pic_info.type = 1;

        ret = MT_MtgoDecpicCreateBKsurface_GIF(&g_pic_info.hRender,PrimaryInfo);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_MtgoDecpicCreateBKsurface_GIF error. ret=0x%x \n", ret);
            g_pic_info.hRender  = MT_INVALID_HANDLE;
            goto err1;
        }

        p_Render = (animation_info_t *)g_pic_info.hRender;
        p_Render->DecHandle = DecHanle;

        g_bTaskQuit = MT_FALSE;
        sleep(1);
        //render all the frame
        Index = 0;

        if(PrimaryInfo.ScrWidth > 1280)
        {
            PrimaryInfo.ScrWidth = 1280;
        }
        if(PrimaryInfo.ScrHeight > 720)
        {
            PrimaryInfo.ScrHeight = 720;
        }

        x0 = (1280 - PrimaryInfo.ScrWidth)/2;
        y0 = (720 - PrimaryInfo.ScrHeight)/2;
        while(g_bTaskQuit != MT_TRUE)
        {
            ret = MT_MtgoDecpicRenderFrame_GIF(g_pic_info.hRender, g_pic_info.hLayerSurface, Index, x0, y0);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("sample_RenderFrame_GIF error. ret=0x%x \n", ret);
                break;
            }
            usleep(100000);

            Index ++;

            if (Index == (PrimaryInfo.Count))
            {
                Index = 0;
            }
            ret = MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", ret);
                break;
            }
        }

        (MT_VOID)MT_MtgoDecpicDestroyBKsurface_GIF(g_pic_info.hRender);

        (MT_VOID)MT_GO_FillRect(g_pic_info.hLayerSurface, NULL, 0, MTGO_COMPOPT_NONE);
        (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
    }
    else
    {
        g_pic_info.type = 0;
        ret = MT_MtgoDecpicDecImgData(DecHanle, &g_pic_info.hDecSurface);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_MtgoDecpicDecImgData fail ret (%d)\n", ret);
            goto err1;
        }
        g_bTaskQuit = MT_FALSE;

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

        while(g_bTaskQuit != MT_TRUE)
        {
            usleep(500);
        }
        (MT_VOID)MT_GO_FreeSurface(g_pic_info.hDecSurface);
    }

err1:
    MT_MtgoDestoryDecoder(DecHanle);

    return ret;
}


static mt_s32 MT_MtgoDecpicCmdTask(MT_VOID)
{

    mt_s32 ret = 0;
    mt_s32 flag = 0;
    mt_s32 bflag = 0;
    char *fgetret=NULL;
    mt_s8 inputCmd[32];
    MT_RECT stRect = { 0 };
    MTGO_BLTOPT_S stBltOpt = { 0 };
    stBltOpt.EnableScale = MT_TRUE;
    stBltOpt.PixelAlphaComp = MTGO_COMPOPT_NONE;

    stRect.x = (1280 - g_stAttr.Width)/2;
    stRect.y = (720 - g_stAttr.Height)/2;
    stRect.w = g_stAttr.Width ;
    stRect.h = g_stAttr.Height;

    while (g_bTaskQuit == MT_FALSE)
    {
        (MT_VOID)MT_MtgoDecpicPrintMenu();
        fgetret=fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret=fgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else if('s' == inputCmd[0])
        {
            if(g_pic_info.type == 1)
            {
                SAMPLE_MTGO_ERR_PRINT("GIF images are not supported \n");
                continue;
            }
            if(flag > 1)
            {
                SAMPLE_MTGO_INFO_PRINT("It's already the smallest \n");
                continue;
            }
            if(bflag != 0)
            {
                bflag--;
            }
            flag++;
            (MT_VOID)MT_GO_FillRect(g_pic_info.hLayerSurface, NULL, 0xffffffff, MTGO_COMPOPT_NONE);
            (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);

            stRect.w /= 2;
            stRect.h /= 2;
            if(stRect.w <= 50)
            {
                stRect.w = 50;
            }
            if(stRect.h <= 50)
            {
                stRect.h = 50;
            }
            stRect.x = (1280 - stRect.w)/2;
            stRect.y = (720 - stRect.h)/2;

            /** Blit it to graphic layer Surface */
            ret = MT_GO_Blit(g_pic_info.hDecSurface, NULL, g_pic_info.hLayerSurface, &stRect, &stBltOpt);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", ret);
            }
            ret = MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", ret);
            }

        }
        else if('r' == inputCmd[0])
        {
            if(g_pic_info.type == 1)
            {
                SAMPLE_MTGO_ERR_PRINT("GIF images are not supported \n");
                continue;
            }
            flag = 0;
            bflag = 0;
            stRect.x = (1280 - g_stAttr.Width)/2;
            stRect.y = (720 - g_stAttr.Height)/2;
            stRect.w = g_stAttr.Width ;
            stRect.h = g_stAttr.Height;
            (MT_VOID)MT_GO_FillRect(g_pic_info.hLayerSurface, NULL, 0xffffffff, MTGO_COMPOPT_NONE);
            (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
            /** Blit it to graphic layer Surface */
            ret = MT_GO_Blit(g_pic_info.hDecSurface, NULL, g_pic_info.hLayerSurface, &stRect, &stBltOpt);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", ret);
            }
            ret = MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", ret);
            }
        }
        else if('z' == inputCmd[0])
        {
            if(g_pic_info.type == 1)
            {
                SAMPLE_MTGO_ERR_PRINT("GIF images are not supported \n");
                continue;
            }
            if(bflag > 1)
            {
                SAMPLE_MTGO_INFO_PRINT("It's already the largest \n");
                continue;
            }
            if(flag != 0)
            {
                flag--;
            }
            bflag++;
            (MT_VOID)MT_GO_FillRect(g_pic_info.hLayerSurface, NULL, 0xffffffff, MTGO_COMPOPT_NONE);
            (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);

            stRect.w *= 2;
            stRect.h *= 2;
            if(stRect.w >= 1280)
            {
                stRect.w = 1280;
            }
            if(stRect.h >= 720)
            {
                stRect.h = 720;
            }
            stRect.x = (1280 - stRect.w)/2;
            stRect.y = (720 - stRect.h)/2;
            /** Blit it to graphic layer Surface */
            ret = MT_GO_Blit(g_pic_info.hDecSurface, NULL, g_pic_info.hLayerSurface, &stRect, &stBltOpt);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_Blit error. ret=0x%x \n", ret);
            }
            ret = MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", ret);
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

static MT_VOID MT_MtgoDecpicExit(MT_VOID)
{

    g_bTaskQuit = MT_TRUE;
    sleep(1);
    (MT_VOID)MT_GO_DestroyLayer(g_pic_info.hLayer);

    (MT_VOID)pthread_join(g_pic_info.Cmdtask, NULL);

    (MT_VOID)MT_GO_Deinit();

    memset(&g_pic_info, 0, sizeof(picture_info_t));
}

static MT_S32 MT_MtgoDecpicParase_args(MT_S32 argc, MT_CHAR *argv[], picture_info_t *ppic_info)
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
                (MT_VOID)MT_MtgoDecpicHelp(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MtgoDecpicExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                memcpy(ppic_info->picname, mt_optarg, strlen(mt_optarg));
                break;
            default:
                SAMPLE_MTGO_ERR_PRINT("ERR opt[%c], print help info.\n", opt);
                (MT_VOID)MT_MtgoDecpicHelp(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}

#ifdef MT_SAMPLE_APP
    mt_s32 MT_MtgoDecPicMain(mt_s32 argc, mt_char *argv[])
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32 ret = 0;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    MTGO_LAYER_E eLayerID = MTGO_LAYER_OSD0;


    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        MT_MtgoDecpicHelp(argv[0]);
        return MT_SUCCESS;
    }

    ret = MT_MtgoDecpicParase_args(argc, argv, &g_pic_info);
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
            goto ERR2;
        }

        stLayerInfo.PixelFormat = MTGO_PF_8888;
        stLayerInfo.LayerFlushType = MTGO_LAYER_FLUSH_NORMAL;

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
            goto ERR3;
        }

        ret = MT_GO_SetLayerAlpha(g_pic_info.hLayer, 0xff);    //Transparency is full transparency(0~0xff)
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_SetLayerAlpha error. ret=0x%x \n", ret);
            goto ERR3;
        }
        SAMPLE_MTGO_INFO_PRINT("Start to decoding  file: %s\n", g_pic_info.picname);

        ret = pthread_create(&g_pic_info.Cmdtask, NULL, (void * (*)(void *))MT_MtgoDecpicTask, NULL);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("pthread_create error. ret=0x%x \n", ret);
            goto ERR3;
        }
        sleep(8);
        if(g_bTaskQuit == MT_TRUE)
        {
            goto ERR4;
        }

    }
    (MT_VOID)MT_MtgoDecpicCmdTask();
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }


ERR4:
    (MT_VOID)pthread_join(g_pic_info.Cmdtask, NULL);

ERR3:

    (MT_VOID)MT_GO_DestroyLayer(g_pic_info.hLayer);

ERR2:
    (MT_VOID)MT_GO_Deinit();

ERR1:
#ifndef MT_SAMPLE_APP
        Sample_MTGO_Display_DeInit();
#endif
    memset(&g_pic_info, 0, sizeof(picture_info_t));
    SAMPLE_MTGO_FUNCTION_EXIT();
    return MT_SUCCESS;
}

