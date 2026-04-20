#include <dirent.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <sys/stat.h>
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
    mt_char pic_dir[100];
    mt_char picname[1024];
    pthread_t Cmdtask;
    mt_handle hLayer;
    mt_handle hDecSurface;
    mt_handle hLayerSurface;
    mt_handle hRender;
    MT_BOOL   type;
    mt_char *buf;
}picture_info_t;

static picture_info_t g_pic_info;

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

#ifdef MT_SAMPLE_APP
    mt_s32 MT_MtgoLoopPicMain(mt_s32 argc, mt_char *argv[]);
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static mt_s32 MT_MtgoLoopDecpicFilesize(int file)
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
static mt_s32 MT_MtgoLoopDecpic(mt_char *pp_FileName, mt_handle *p_Surface)
{
    mt_s32 ret = 0;
    mt_handle hDecoder = 0;//Handle to the decoding
    MTGO_DEC_ATTR_S stSrcDesc = { 0 };//Input stream object
    MTGO_DEC_IMGINFO_S stImgInfo = { 0 };//Image format

    mt_u32 file_size = 0;
    mt_s32 file = 0;
    mt_u32 read_size = 0;
    mt_char *p_buf = NULL;

    unsigned int mem_type;
    unsigned int mem_available;

    MT_ASSERT(pp_FileName != NULL);
    MT_ASSERT(p_Surface != NULL);

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
        return MT_FAILURE;
    }

    ret = GetAvailableMem(&mem_available);
    if (ret != 0)
    {
        SAMPLE_MTGO_ERR_PRINT("GetAvailableMem err!\n");
        return MT_FAILURE;
    }

    file_size = MT_MtgoLoopDecpicFilesize(file);
    if (file_size <= 0)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_MtgoLoopDecpicFilesize err!\n");
        close(file);
        return MT_FAILURE;
    }

    /* The image file size cannot exceed the actual available memory, with a margin of 4M */
    if (file_size > (mem_available - 4*1024*1024) * 1024) {
        SAMPLE_MTGO_ERR_PRINT("%s, [%s]available memorry is not enough, please change an smaller image file\n",
            g_mem_type_str[mem_type], __FUNCTION__);
        return MT_FAILURE;
    }

    p_buf = (mt_char *)malloc(file_size);
    if (p_buf == NULL)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, err: no buffer \n", __FUNCTION__);
        close(file);
        return MT_FAILURE;
    }

    read_size = read(file, p_buf, file_size);
    if (read_size != file_size)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, read size error: %d!\n", __FUNCTION__, read_size);
        free(p_buf);
        close(file);
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
        free(p_buf);
        return ret;
    }


    ret = MT_GO_DecImgInfo(hDecoder, 0, &stImgInfo);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgInfo error. ret=0x%x \n", ret);
        MT_GO_DestroyDecoder(hDecoder);
        free(p_buf);
        close(file);
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

    g_stAttr.Format = MTGO_PF_8888;
    if (stImgInfo.Width > 4095 || stImgInfo.Height > 4095)
    {
        ret = MT_GO_DecImgData(hDecoder, 0, &g_stAttr, p_Surface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgData1 error. ret=0x%x \n", ret);
            MT_GO_DestroyDecoder(hDecoder);
            free(p_buf);
            close(file);
            return ret;
        }
    }
    else
    {
        ret = MT_GO_DecImgData(hDecoder, 0, &g_stAttr, p_Surface);
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_DecImgData2 error. ret=0x%x \n", ret);
            MT_GO_DestroyDecoder(hDecoder);
            free(p_buf);
            close(file);
            return ret;
        }
    }

    ret = MT_GO_DestroyDecoder(hDecoder);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DestroyDecoder error. ret=0x%x \n", ret);
        free(p_buf);
        close(file);
        return ret;
    }
    free(p_buf);
    close(file);
    return ret;
}

/*
@brief decode GIF
@param[in] p_FileName,The file name to decode
@param[out] p_hRender,Pointer to the outgoing handle
@param[in] p_PrimaryInfo,Major information after decoding
@return ::MT_SUCCESS
*/
static mt_s32 MT_MtgoLoopDecpicCreate_GIF(mt_char *p_FileName, mt_handle *p_hRender, MTGO_DEC_PRIMARYINFO_S *p_PrimaryInfo, mt_char **pbuf)
{
    mt_s32 ret = 0;
    MTGO_DEC_ATTR_S SrcDesc = { 0 };
    MTGO_DEC_PRIMARYINFO_S PrimaryInfo = { 0 };
    animation_info_t *p_Render = NULL;
    mt_handle DecHanle = 0;
    mt_handle Surface = 0;
    mt_u32 file_size = 0;
    mt_u32 read_size = 0;
    mt_char *buf;

    mt_s32 file = 0;

    if ((p_FileName == NULL)|| (p_hRender == NULL))
    {
        SAMPLE_MTGO_ERR_PRINT(" p_FileName or phRender error. \n");
        return MT_FAILURE;
    }

    file = open(p_FileName, O_RDONLY);
    if (file < 0)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, fail to read file!\n", __FUNCTION__);
        return MT_FAILURE;
    }

    file_size = MT_MtgoLoopDecpicFilesize(file);
    if (file_size <= 0)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, read file size fail!\n", __FUNCTION__);
        goto err1;
    }

    buf = (mt_char *)malloc(file_size);
    if (buf == NULL)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, err: no buffer \n", __FUNCTION__);
        goto err1;
    }

    read_size = read(file, buf, file_size);
    if (read_size != file_size)
    {
        SAMPLE_MTGO_ERR_PRINT("%s, read size error: %d!\n", __FUNCTION__, read_size);
        goto err2;
    }

    SrcDesc.SrcType = MTGO_DEC_SRCTYPE_FILE;
    SrcDesc.SrcInfo.pFileName = p_FileName;
    SrcDesc.SrcInfo.MemInfo.pAddr = buf;
    SrcDesc.SrcInfo.MemInfo.Length = file_size;
    ret = MT_GO_CreateDecoder(&SrcDesc, &DecHanle);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateDecoder error. ret=0x%x \n", ret);
        goto err2;
    }

    ret = MT_GO_DecCommInfo(DecHanle, &PrimaryInfo);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DecCommInfo error. ret=0x%x \n", ret);
        goto err3;
    }

    ret = MT_GO_CreateSurface(PrimaryInfo.ScrWidth, PrimaryInfo.ScrHeight, MTGO_PF_1555, &Surface);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_CreateSurface error. ret=0x%x \n", ret);
        goto err3;
    }

    p_Render = (animation_info_t *)malloc (sizeof(animation_info_t));
    if (p_Render == NULL)
    {
        SAMPLE_MTGO_ERR_PRINT("p_Render is NULL!\n");
        goto err4;
    }

    memset(p_Render, 0, sizeof(animation_info_t));
    p_Render->BkSurface = Surface;
    p_Render->DecHandle = DecHanle;
    *p_PrimaryInfo = PrimaryInfo;
    *p_hRender = (mt_handle)p_Render;
    *pbuf = buf;
    close(file);
    file = -1;

    return MT_SUCCESS;

err4:
    (MT_VOID)MT_GO_FreeSurface(Surface);
err3:
    (MT_VOID)MT_GO_DestroyDecoder(DecHanle);
err2:
    free(buf);
    buf = NULL;
err1:
    close(file);
    file = -1;

    return ret;
}

/*
@brief Destroy the decode handle
@param[in] hRender,handle
@return ::MT_SUCCESS
*/
static mt_s32 MT_MtgoLoopDecpicDestroy_GIF(mt_handle hRender)
{
    mt_s32 ret;
    animation_info_t *p_Render;

    p_Render = (animation_info_t *)hRender;
    if (p_Render == NULL)
    {
        return MT_FAILURE;
    }

    ret = MT_GO_DestroyDecoder(p_Render->DecHandle);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_DestroyDecoder error. ret=0x%x \n", ret);
        return ret;
    }

    ret = MT_GO_FreeSurface(p_Render->BkSurface);
    if (ret != MT_SUCCESS)
    {
        SAMPLE_MTGO_ERR_PRINT("MT_GO_FreeSurface error. ret=0x%x \n", ret);
        return ret;
    }

    free(p_Render);
    p_Render = NULL;
    free(g_pic_info.buf);
    g_pic_info.buf = NULL;
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
static mt_s32 MT_MtgoLoopDecpicRenderFrame_GIF(mt_handle hRender, mt_handle DstSurface, mt_u32 Index, mt_s32 x0, mt_s32 y0)
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
        LastRect.x = x0 + p_Render->BKRect.x;
        LastRect.y = y0 + p_Render->BKRect.y;
        LastRect.w = p_Render->BKRect.w;
        LastRect.h = p_Render->BKRect.h;
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
static mt_void MT_MtgoLoopDecpicHelp(char *name)
{
    MTGO_PRINT("sample usage: \n");
    MTGO_PRINT(" %s -f ./res\n", name);
    MTGO_PRINT(" %s -q  <exit> \n", name);
    return;
}
static MT_VOID MT_MtgoLoopDecpicPrintMenu(MT_VOID)
{
#ifdef MT_SAMPLE_APP
    MTGO_PRINT("     b : background run \n");
#endif
    MTGO_PRINT("     h : help \n");
    MTGO_PRINT("     q : quit \n");
    MTGO_PRINT("Loop_pic>> ");

}
static mt_s32 MT_MtgoLoopDecpic_ends_with(const char *str, const char *suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);
    return (str_len >= suffix_len) && (strcmp(str + str_len - suffix_len, suffix) == 0);
}

static mt_s32 MT_MtgoLoopDecpicTask(void *arg)
{
    mt_s32 ret = 0;
    MT_RECT stRect = { 0 };
    MTGO_BLTOPT_S stBltOpt = { 0 };
    struct dirent **filelist = NULL;
    mt_s32 s32FileNum = 0;
    mt_char aszFileName[512] = {0};
    mt_char *jpg_suffix = ".jpg";
    mt_char *png_suffix = ".png";
    mt_char *bmp_suffix = ".bmp";
    mt_char *gif_suffix = ".gif";
    struct dirent **namelist = NULL;
    mt_u32 total_file_cnt = 0;
    mt_s32 i = 0;
    mt_s32 j = 0;
    mt_char *p_pos = NULL;
    MTGO_DEC_PRIMARYINFO_S PrimaryInfo = { 0 };
    mt_u32 Index = 0;
    struct stat path_stat;
    mt_s32 x0, y0;

    /*Check whether the path is a file or a directory.*/
    if(stat(g_pic_info.pic_dir, &path_stat) == 0) {
        if(S_ISDIR(path_stat.st_mode)) {
            s32FileNum = scandir(g_pic_info.pic_dir, &namelist, NULL, alphasort);
        } else if(S_ISREG(path_stat.st_mode)) {
            SAMPLE_MTGO_INFO_PRINT("%s is a file, need a dir.\n", g_pic_info.pic_dir);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        } else {
            SAMPLE_MTGO_INFO_PRINT("%s is neither a file nor a directory, need a dir.\n", g_pic_info.pic_dir);
            g_bTaskQuit = MT_TRUE;
            return MT_FAILURE;
        }
    } else {
        SAMPLE_MTGO_INFO_PRINT("Error getting file status.\n");
        g_bTaskQuit = MT_TRUE;
        return MT_FAILURE;
    }

    filelist = (struct dirent **)malloc(s32FileNum * sizeof(struct dirent *));

    for(i = 0; i < s32FileNum; ++i)
    {
        if (MT_MtgoLoopDecpic_ends_with(namelist[i]->d_name, jpg_suffix) || MT_MtgoLoopDecpic_ends_with(namelist[i]->d_name, png_suffix) || MT_MtgoLoopDecpic_ends_with(namelist[i]->d_name, bmp_suffix) || MT_MtgoLoopDecpic_ends_with(namelist[i]->d_name, gif_suffix))
        {
             SAMPLE_MTGO_INFO_PRINT("%s\n", namelist[i]->d_name);
             j += 1;
             filelist[j-1] = namelist[i];

         }

    }
    s32FileNum = j;

    SAMPLE_MTGO_INFO_PRINT("scandir [%d] files in the dir [%s]: \n", s32FileNum, g_pic_info.pic_dir);

    /*Check that there are no image files in this directory*/
    if (s32FileNum == 0) {
        SAMPLE_MTGO_INFO_PRINT("Do not find picture file in the dir [%s], check the dir again please. \n", g_pic_info.pic_dir);
        g_bTaskQuit = MT_TRUE;
    }

    total_file_cnt  = s32FileNum;

    while(g_bTaskQuit != MT_TRUE)
    {
        mt_s32 time = 3;

        s32FileNum--;
        snprintf(aszFileName, sizeof(aszFileName), "%s/%s", g_pic_info.pic_dir, filelist[s32FileNum]->d_name);
        SAMPLE_MTGO_INFO_PRINT("Start to decoding  file: %s: [%d]\n", aszFileName, s32FileNum);

        p_pos = strrchr(aszFileName, '.');
        if (NULL == p_pos)
        {
            return 0;
        }
        p_pos++;
        if(strcasecmp(p_pos, "gif") == 0)
        {
            ret = MT_MtgoLoopDecpicCreate_GIF((mt_char*)aszFileName, &g_pic_info.hRender, &PrimaryInfo, &g_pic_info.buf);
            if (MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("sample_Create_GIF error. ret=0x%x \n", ret);
                g_pic_info.hRender  = MT_INVALID_HANDLE;
                memset(aszFileName, 0, sizeof(aszFileName));
                if(s32FileNum == 0)
                {
                    s32FileNum = total_file_cnt;
                }
                continue;
            }

            sleep(3);
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
            while(1)
            {
                ret = MT_MtgoLoopDecpicRenderFrame_GIF(g_pic_info.hRender, g_pic_info.hLayerSurface, Index,  x0, y0);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_MTGO_ERR_PRINT("sample_RenderFrame_GIF error. ret=0x%x \n", ret);
                    (MT_VOID)MT_MtgoLoopDecpicDestroy_GIF(g_pic_info.hRender);
                    (MT_VOID)MT_GO_FillRect(g_pic_info.hLayerSurface, NULL, 0, MTGO_COMPOPT_NONE);
                    (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
                    break;
                }
                usleep(100000);

                ret = MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
                if (MT_SUCCESS != ret)
                {
                    SAMPLE_MTGO_ERR_PRINT("MT_GO_RefreshLayer error. ret=0x%x \n", ret);
                    (MT_VOID)MT_MtgoLoopDecpicDestroy_GIF(g_pic_info.hRender);
                    break;
                }

                Index ++;

                if (Index == (PrimaryInfo.Count))
                {
                    time--;
                    Index = 0;
                    if(time == 0)
                    {
                        (MT_VOID)MT_MtgoLoopDecpicDestroy_GIF(g_pic_info.hRender);
                        (MT_VOID)MT_GO_FillRect(g_pic_info.hLayerSurface, NULL, 0, MTGO_COMPOPT_NONE);
                        (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
                        sleep(2);
                        break;
                    }

                }

            }


        }

        else
        {

            ret = MT_MtgoLoopDecpic(aszFileName, &g_pic_info.hDecSurface);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_MTGO_ERR_PRINT("dec fail ret (%d)\n", ret);
                g_pic_info.hDecSurface  = MT_INVALID_HANDLE;
                memset(aszFileName, 0, sizeof(aszFileName));
                MT_GO_FillRect(g_pic_info.hLayerSurface, NULL, 0, MTGO_COMPOPT_NONE);
                MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
                if(s32FileNum == 0)
                {
                    s32FileNum = total_file_cnt;
                }
                continue;
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
            sleep(3);
            (MT_VOID)MT_GO_FillRect(g_pic_info.hLayerSurface, NULL, 0, MTGO_COMPOPT_NONE);
            (MT_VOID)MT_GO_RefreshLayer(g_pic_info.hLayer, NULL);
            (MT_VOID)MT_GO_FreeSurface(g_pic_info.hDecSurface);


        }
        memset(aszFileName, 0, sizeof(aszFileName));
        if(s32FileNum == 0)
        {
            s32FileNum = total_file_cnt;
        }
    }
    free(filelist);
    free(namelist);


    return MT_SUCCESS;
}


static mt_s32 MT_MtgoLoopDecpicCmdTask(MT_VOID)
{
    char *fgetret=NULL;
    mt_s8 inputCmd[32];


    while (g_bTaskQuit == MT_FALSE)
    {
        (MT_VOID)MT_MtgoLoopDecpicPrintMenu();
        fgetret=fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret=fgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_MTGO_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
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

static MT_VOID MT_MtgoLoopDecpicExit(MT_VOID)
{
    (MT_VOID)MT_GO_FreeSurface(g_pic_info.hLayerSurface);

    if(g_bTaskQuit == MT_FALSE)
    {
        if(g_pic_info.type == 1)
        {
            (MT_VOID)MT_MtgoLoopDecpicDestroy_GIF(g_pic_info.hRender);

        }
        else
        {
            (MT_VOID)MT_GO_FreeSurface(g_pic_info.hDecSurface);

        }
    }
    g_bTaskQuit = MT_TRUE;


    (MT_VOID)MT_GO_DestroyLayer(g_pic_info.hLayer);

    (MT_VOID)pthread_join(g_pic_info.Cmdtask, NULL);

    (MT_VOID)MT_GO_Deinit();

    memset(&g_pic_info, 0, sizeof(picture_info_t));
}

static MT_S32 MT_MtgoLoopDecpicParase_args(MT_S32 argc, MT_CHAR *argv[], picture_info_t *ppic_info)
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
                (MT_VOID)MT_MtgoLoopDecpicHelp(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_MtgoLoopDecpicExit();
                }
                return MT_TASK_EXIT;
            case 'f':
                memcpy(ppic_info->pic_dir, mt_optarg, strlen(mt_optarg));
                break;
            default:
                SAMPLE_MTGO_ERR_PRINT("ERR opt[%c], print help info.\n", opt);
                (MT_VOID)MT_MtgoLoopDecpicHelp(argv[0]);
                return MT_FAILURE;
        }
    }

    SAMPLE_MTGO_FUNCTION_EXIT();

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
    mt_s32 MT_MtgoLoopPicMain(mt_s32 argc, mt_char *argv[])
#else
    mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32 ret = 0;
    MTGO_LAYER_INFO_S stLayerInfo = { 0 };
    MTGO_LAYER_E eLayerID = MTGO_LAYER_OSD0;



    if(argc != 3 && g_bTaskQuit == MT_TRUE)
    {
        MT_MtgoLoopDecpicHelp(argv[0]);
        return MT_FAILURE;
    }

    ret = MT_MtgoLoopDecpicParase_args(argc, argv, &g_pic_info);
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
        g_bTaskQuit = MT_FALSE;

        ret = MT_GO_SetLayerAlpha(g_pic_info.hLayer, 0xff);    //Transparency is full transparency(0~0xff)
        if (MT_SUCCESS != ret)
        {
            SAMPLE_MTGO_ERR_PRINT("MT_GO_SetLayerAlpha error. ret=0x%x \n", ret);
            goto ERR3;
        }

        ret = pthread_create(&g_pic_info.Cmdtask, NULL, (void * (*)(void *))MT_MtgoLoopDecpicTask, NULL);
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
    (MT_VOID)MT_MtgoLoopDecpicCmdTask();
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

    memset(&g_pic_info, 0, sizeof(picture_info_t));

ERR1:
#ifndef MT_SAMPLE_APP
        Sample_MTGO_Display_DeInit();
#endif

        SAMPLE_MTGO_FUNCTION_EXIT();
        return MT_SUCCESS;
}


