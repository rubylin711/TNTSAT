
/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "mt_type.h"
#include "sys_define.h"
#include "string.h"

#include "mt_osal.h"
#include "mt_pvr_fs_def.h"
//#include "mt_pvr_print.h"
//#include "mt_pvr_api.h"
#include "mt_pvr_addon_common.h"
//#include "mt_pvr_addon_priv.h"
#include "mt_pvr_types.h"
//#include "mpvr_api.h"
#include "mt_unf_pvr.h"
#include "mt_pvr_addon_api.h"
#include "mt_pvr_intf.h"




#define CMX_FILE_UNIT_SIZE 32
#define CMX_FILE_UNIT_NUM 20
#define CMX_FILE_HEADER_SIZE 512
#define CMX_MAX_REC_FILE_NUM 2
#define CMX_POS_MAX_SIZE (94*1024)

#define PVR_ADDON_PRINT_DEBUG
#define P_PRINT_LEVEL 1
#define P_PRINT_INFO 2
#define P_PRINT_ERROR 3
#ifdef PVR_ADDON_PRINT_DEBUG
#define PVR_PRINTF(level, fmt, ...) \
    do { \
        if(level > P_PRINT_LEVEL) {\
        printf("[L%d|%s,%d] tick=%d ",level, __FUNCTION__, __LINE__, mt_ticks_get());\
        printf(fmt "\n", ##__VA_ARGS__);} \
    } while (0)

#define PVR_ADDON_PRINTF(level, fmt, ...) \
  do { \
    PVR_PRINTF(level, fmt, ##__VA_ARGS__); \
  } while(0)
#else
#define PVR_ADDON_PRINTF(...)     do{}while(0)
#endif

typedef struct
{
    u8 encrypt_flag;
    u8 file_name[MAX_FILE_PATH];
} mt_pvr_addon_cmx_header_t;

typedef struct
{
    u8 age_limit;
    u64 pos;
} mt_pvr_addon_cmx_file_t;

typedef struct
{
    //hfile_t priv_file;
    int ufs_file;
    u8 age_limit;
    u8 encrypt_flag;
    char data[CMX_FILE_HEADER_SIZE];
    u16 store_cnt;
    u8 file_idx;
} mt_pvr_addon_common_rec_priv_t;


typedef struct
{
    u64 file_size;
    u8 file_name[MAX_FILE_PATH];
    ulong handle;
    u8 bValid;
} mt_pvr_addon_common_rec_file_t;

typedef struct
{
    //u64 file_size;
    u8 file_name[MAX_FILE_PATH];
    u32 handle;
    u8 bValid;
} mt_pvr_addon_common_play_file_t;

typedef struct
{
    //hfile_t priv_file;
    int ufs_file;
    u8 age_limit;
    u64 start_pos;
    u64 end_pos;
    u64 file_size;
    s32 cmx_idx;
    u64 max_cnt;
    char data[CMX_FILE_UNIT_SIZE * CMX_FILE_UNIT_NUM];
    s32 cmx_idx_num;
    u8 encrypt_flag;
    u8 file_idx;
    u8 rec_file_valid;
    u8 new_file_name[MAX_FILE_PATH];
} mt_pvr_addon_common_play_priv_t;


typedef struct
{
    mt_pvr_addon_t  *addon;
    mt_pvr_addon_common_rec_file_t rec_file[CMX_MAX_REC_FILE_NUM];
    mt_pvr_addon_common_play_file_t play_file;
} mt_pvr_addon_common_priv_t;

static mt_pvr_addon_common_priv_t  *g_pst_common_addon = NULL;
static mt_pvr_addon_t * g_pvr_ca_addon = NULL;

static void mt_pvr_common_addon_file_by_suffix(const u8 *p_file_name,const char *suffix,u8 *p_new_file_name, u32 new_name_len)
{
    char *p_str = NULL;

    if (new_name_len < strlen((const char *)p_file_name))
    {
        return ;
    }

    strcpy((char *)p_new_file_name, (const char *)p_file_name);

    p_str = strrchr((const char *)p_new_file_name, 0x2e/*'.'*/);
    if (p_str)
    {
        p_str[0] = 0;
    }
    strncat((char *)p_new_file_name, suffix, new_name_len);
}



static mt_pvr_addon_init_common_t *get_pvr_common_addon_priv(void)
{
    mt_pvr_addon_init_common_t *p_priv = g_pst_common_addon->addon->p_args;

    if(p_priv == NULL)
    {
        return NULL;
    }

    return (mt_pvr_addon_init_common_t *)p_priv;
}

static mt_pvr_addon_common_priv_t *get_pvr_common_addon_local_priv(void)
{
    mt_pvr_addon_common_priv_t *p_priv = g_pst_common_addon;

    if(p_priv == NULL)
    {
        return NULL;
    }

    return (mt_pvr_addon_common_priv_t *)p_priv;
}


static MPVR_ADDON_STATUS mt_pvr_common_addon_delete_file(u8 *p_ts_file_name)
{
    mt_pvr_addon_init_common_t *priv =get_pvr_common_addon_priv();
    if(!priv)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }

    if(priv->get_age_limit!=NULL)
    {
        u8 new_file_name[MAX_FILE_PATH] = {0};
        mt_pvr_common_addon_file_by_suffix( p_ts_file_name,".cmx",new_file_name,MAX_FILE_PATH);
        PVR_REMOVE((char *)new_file_name);
    }
    return MPVR_ADDON_STATUS_OK;
}

static MPVR_ADDON_STATUS mt_pvr_common_addon_rename_file(u8 *p_old_ts_name, u8 *p_new_ts_name)
{
    mt_pvr_addon_init_common_t *priv =get_pvr_common_addon_priv();
    if(!priv)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }

    if(priv->get_age_limit!=NULL)
    {
        u8 new_file_name[MAX_FILE_PATH] = {0};
        u8 new_file_name1[MAX_FILE_PATH] = {0};
        mt_pvr_common_addon_file_by_suffix( p_old_ts_name,".cmx",new_file_name,MAX_FILE_PATH);
        mt_pvr_common_addon_file_by_suffix( p_new_ts_name,".cmx",new_file_name1,MAX_FILE_PATH);
        PVR_RENAME((char *)new_file_name,(char *)new_file_name1);
    }
    return MPVR_ADDON_STATUS_OK;
}

static MPVR_ADDON_STATUS mt_pvr_common_addon_rec_create(mt_handle_t *p_handle, mt_pvr_addon_rec_attr_t *p_rec_attr)
{
    mt_pvr_addon_init_common_t *priv =get_pvr_common_addon_priv();
    mt_pvr_addon_common_rec_priv_t *p_rec_priv = NULL;

    if(priv == NULL)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }

    p_rec_priv = malloc(sizeof(mt_pvr_addon_common_rec_priv_t));
    if(p_rec_priv == NULL)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }
    memset(p_rec_priv,0,sizeof(mt_pvr_addon_common_rec_priv_t));

    *p_handle = (mt_handle_t)p_rec_priv;
    p_rec_priv->encrypt_flag = p_rec_attr->pvr_encrypt_flag;

    if(priv->get_age_limit!=NULL)
    {
        u8 new_file_name[MAX_FILE_PATH] = {0};
        u32 write_size = 0;
        u8 i = 0;
        mt_pvr_addon_cmx_header_t header = {0};
        mt_pvr_addon_common_priv_t *local_priv = get_pvr_common_addon_local_priv();

        if(!local_priv)
        {
            return MPVR_ADDON_STATUS_FAILED;
        }

        mt_pvr_common_addon_file_by_suffix( p_rec_attr->file_name,".cmx",new_file_name,MAX_FILE_PATH);
        p_rec_priv->ufs_file = PVR_OPEN((const char *)new_file_name, PVR_FOPEN_MODE_INDEX_WRITE);

        if(p_rec_priv->ufs_file == -1)
        {
            free(p_rec_priv);
            return MPVR_ADDON_STATUS_CREATE_FAIL;
        }

        p_rec_priv->age_limit = 0xFF;
        header.encrypt_flag = p_rec_attr->pvr_encrypt_flag;
        strcpy((char *)header.file_name, (const char *)p_rec_attr->file_name);

        memcpy(p_rec_priv->data,&header,sizeof(mt_pvr_addon_cmx_header_t));
        write_size = PVR_WRITE(p_rec_priv->data, CMX_FILE_HEADER_SIZE, p_rec_priv->ufs_file, 0);
        if(write_size!=CMX_FILE_HEADER_SIZE)
            return MPVR_ADDON_STATUS_FAILED;
        PVR_FSYNC(p_rec_priv->ufs_file);
        for(i = 0 ; i < CMX_MAX_REC_FILE_NUM; i++)
        {
            if(local_priv->rec_file[i].bValid == 0)
            {
                local_priv->rec_file[i].bValid = 1;
                local_priv->rec_file[i].file_size = write_size;
                local_priv->rec_file[i].handle = (ulong)*p_handle;
                strcpy((char *)local_priv->rec_file[i].file_name, (const char *)new_file_name);
                p_rec_priv->file_idx = i;
                //PVR_ADDON_PRINTF(P_PRINT_INFO, "p_rec_priv->file_idx ---------=%d\n",p_rec_priv->file_idx );
                break;
            }
        }
        if(i == CMX_MAX_REC_FILE_NUM)
        {
            PVR_ADDON_PRINTF(P_PRINT_ERROR, "rec file num greater than %d --------- =%d\n",CMX_MAX_REC_FILE_NUM, i);
            return MPVR_ADDON_STATUS_FAILED;
        }
    }
    if(priv->rec_handle_cbk)
    {
        //priv->rec_handle_cbk(priv->p_args, p_rec_attr->file_name,p_rec_attr->name_len,(u32)*p_handle);
        priv->rec_handle_cbk(priv->p_args, p_rec_attr->file_name,p_rec_attr->name_len, p_rec_priv->file_idx);
    }

    return MPVR_ADDON_STATUS_OK;
}

static MPVR_ADDON_STATUS mt_pvr_common_addon_rec_destroy(mt_handle_t handle)
{
    mt_pvr_addon_init_common_t *priv = get_pvr_common_addon_priv();
    mt_pvr_addon_common_priv_t *local_priv = get_pvr_common_addon_local_priv();
    mt_pvr_addon_common_rec_priv_t *p_rec_priv = (mt_pvr_addon_common_rec_priv_t *)handle;

    PVR_ADDON_PRINTF(P_PRINT_INFO, "entry, handle=%p", handle);

    if(!priv || !p_rec_priv || !local_priv)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }

    if(priv->get_age_limit!=NULL)
    {
        if (p_rec_priv->ufs_file)
            PVR_CLOSE(p_rec_priv->ufs_file);
    }

    local_priv->rec_file[p_rec_priv->file_idx].file_size= 0;
    local_priv->rec_file[p_rec_priv->file_idx].bValid = 0;
    p_rec_priv->file_idx =0;

    free((void *)handle);

    PVR_ADDON_PRINTF(P_PRINT_INFO, "return");

    return MPVR_ADDON_STATUS_OK;
}


static MPVR_ADDON_STATUS mt_pvr_common_addon_rec_dataprocess(mt_handle_t handle, mt_u32 rechandle,
		MT_U32 channel_id,
        mt_pvr_addon_rec_param_input_t* p_param_in,
        mt_pvr_addon_rec_param_output_t* p_param_out)
{
    mt_pvr_addon_init_common_t *priv =get_pvr_common_addon_priv();
    mt_pvr_data_process_cb_param_t param = {0};
    mt_pvr_addon_common_rec_priv_t *p_rec_priv;
    mt_u32 time_now,time_last;
    MT_BOOL flag_aligned_pos = TRUE;
    RET_CODE ret = 0;

    if(!priv)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }
    
    p_rec_priv = (mt_pvr_addon_common_rec_priv_t *)handle;

    MT_PVR_SysGetTimeStampMs(&time_last);
    if(priv->get_age_limit!=NULL)
    {
        u8 age_limit=0;
        flag_aligned_pos = (p_param_in->pos % (p_param_in->len * 4) == 0)? TRUE:FALSE;
        if (flag_aligned_pos)
        {
            priv->get_age_limit(p_rec_priv->file_idx,priv->p_args, &age_limit);
        }
        else
        {
            age_limit = p_rec_priv->age_limit;
        }

        if(p_rec_priv->age_limit!=age_limit)
        {                       
            mt_pvr_addon_common_priv_t *local_priv = get_pvr_common_addon_local_priv();
            mt_pvr_addon_cmx_file_t file_data = {0};
            u32 write_size = 0;

            PVR_ADDON_PRINTF(P_PRINT_INFO, " age_limit  %d\n",age_limit);

            if (!local_priv)
            {
                return MPVR_ADDON_STATUS_FAILED;
            }
            p_rec_priv->age_limit = age_limit;
            file_data.age_limit = age_limit;
            file_data.pos = p_param_in->pos;// - p_param_in->len;
            MT_ASSERT(sizeof(mt_pvr_addon_cmx_file_t)<CMX_FILE_UNIT_SIZE);
            memset(p_rec_priv->data, 0, CMX_FILE_UNIT_SIZE);
            memcpy(p_rec_priv->data,&file_data,sizeof(mt_pvr_addon_cmx_file_t));

            write_size = PVR_WRITE(p_rec_priv->data, CMX_FILE_UNIT_SIZE, p_rec_priv->ufs_file, 0);
            if(write_size!=CMX_FILE_UNIT_SIZE)
                return MPVR_ADDON_STATUS_FAILED;

            PVR_FSYNC (p_rec_priv->ufs_file);
            local_priv->rec_file[p_rec_priv->file_idx].file_size+= write_size;
            //OS_PRINTF("size ++ = %lld\n",local_priv->rec_file[p_rec_priv->file_idx].file_size);
            //OS_PRINTF("save  %lld",file_data.pos);
            PVR_ADDON_PRINTF(P_PRINT_INFO, " save age_limit  %d size=%ld\n",file_data.age_limit, sizeof(mt_pvr_addon_cmx_file_t));
            p_param_out->type = MPVR_ADDON_PROCESS_RET_TYPE_STORE_24BITS;
            memcpy(&(p_param_out->value),&(p_rec_priv->store_cnt),2);
            PVR_ADDON_PRINTF(P_PRINT_INFO, " pos=%llu, idx=%d file_idx=%d.", p_param_in->pos, p_rec_priv->store_cnt, p_rec_priv->file_idx);
            p_rec_priv->store_cnt++;
        }
        else
        {
            u16 store_cnt;

            store_cnt =p_rec_priv->store_cnt-1;
            PVR_ADDON_PRINTF(P_PRINT_INFO, " pos=%llu, idx=%d file_idx=%d.", p_param_in->pos, store_cnt, p_rec_priv->file_idx);

            p_param_out->type = MPVR_ADDON_PROCESS_RET_TYPE_STORE_24BITS;
            memcpy(&(p_param_out->value),&store_cnt,2);
        }
    }

    if(!p_rec_priv->encrypt_flag)
        return MPVR_ADDON_STATUS_OK;

    param.handle = (void *)(ulong)handle;
    param.type = MPVR_PROCESS_TYPE_ENCRYPT;
    param.p_dest = p_param_in->p_dest_data;
    param.p_src= p_param_in->p_src_data;
    param.p_dest_phy = p_param_in->p_dest_data_phy;
    param.p_src_phy= p_param_in->p_src_data_phy;

    param.length= p_param_in->len;
    param.p_args= priv->p_args;

    if(priv->data_process_cb)
        ret =priv->data_process_cb(&param);
    if(ret ==0)
    {
        p_param_in->len = param.length;
        MT_PVR_SysGetTimeStampMs(&time_now);
        if((time_now > time_last) && 30<=(time_now-time_last)){
            MT_USLEEP(1000*4);
            time_last=time_now;
        }
        return MPVR_ADDON_STATUS_OK;
    }
    else
        return MPVR_ADDON_STATUS_FAILED;

}

/*
static void print_uni(u8 *p_path, u8 *p_content)
{
    PVR_ADDON_PRINTF(P_PRINT_INFO, "##%s:[%s]", p_content, p_path);
}

static void  mt_pvr_common_addon_update_cmx_size(mt_pvr_addon_common_play_priv_t *p_play_priv)
{
    mt_pvr_addon_common_priv_t *local_priv = get_pvr_common_addon_local_priv();
    if (!local_priv || !p_play_priv)
    {
        return ;
    }
    if(local_priv->rec_file[p_play_priv->file_idx].bValid && p_play_priv->rec_file_valid)
    {
        p_play_priv->file_size = local_priv->rec_file[p_play_priv->file_idx].file_size;
        if(p_play_priv->file_size>CMX_FILE_HEADER_SIZE)
            p_play_priv->max_cnt =( p_play_priv->file_size-CMX_FILE_HEADER_SIZE)/CMX_FILE_UNIT_SIZE;
    }
    else
    {
        p_play_priv->rec_file_valid = 0;
    }
}

static u64 _fs_get_fsize(u8* path)
{
    return PVR_FILE_GetFileSize((const char *)path);
}

static u8 mt_pvr_common_addon_get_age_limit(mt_pvr_addon_common_play_priv_t *p_play_priv,u16 cur_cnt, u64 pos)
{
    //u32 cmx_idx_num = 0;
    //u32 i = 0;
    u8 fs_ret  =0;
    u32 ret_size =0;
    //mt_pvr_addon_cmx_file_t *file_data =NULL;
    //mt_pvr_addon_cmx_file_t *file_data_next =NULL;
    s32 list_cnt = 0;
//	s32 need_cnt = 0;
    u8 age_limit = 0;
    //OS_PRINTF(" orig: %llu count=%d \n",pos, cur_cnt);
    //OS_PRINTF("  %llu  ",p_play_priv->start_pos);
    //OS_PRINTF("  %llu\n", p_play_priv->end_pos);

    mt_pvr_common_addon_update_cmx_size(p_play_priv);
    if(pos==0)
        cur_cnt =0;

    if(cur_cnt >= p_play_priv->max_cnt)
        return 0;

    if(p_play_priv->max_cnt -cur_cnt>CMX_FILE_UNIT_NUM)
    {
        list_cnt = CMX_FILE_UNIT_NUM;
        //need_cnt = MAX_TS_DECRYPT_UNIT;
    }
    else
    {
        list_cnt = p_play_priv->max_cnt -cur_cnt;
        //if(list_cnt>=MAX_TS_DECRYPT_UNIT)
        //	need_cnt = MAX_TS_DECRYPT_UNIT;
        //else
        //	need_cnt =list_cnt;
    }
    PVR_ADDON_PRINTF(P_PRINT_INFO, "max_cnt=%d, cur_cnt=%d", p_play_priv->max_cnt, cur_cnt);
    PVR_ADDON_PRINTF(P_PRINT_INFO, "list_cnt=%d", list_cnt);

    if(cur_cnt<p_play_priv->cmx_idx||cur_cnt>=p_play_priv->cmx_idx+p_play_priv->cmx_idx_num)
    {
        PVR_ADDON_PRINTF(P_PRINT_INFO, "");
        PVR_CLOSE(p_play_priv->ufs_file);
        print_uni(p_play_priv->new_file_name," new_file_name:");
        p_play_priv->ufs_file = PVR_OPEN((const char *)p_play_priv->new_file_name, PVR_FOPEN_MODE_INDEX_WRITE);

        if(p_play_priv->ufs_file == -1)
        {
            PVR_ADDON_PRINTF(P_PRINT_ERROR, "");
            return 0;
        }
        s64 seek_pos = cur_cnt * CMX_FILE_UNIT_SIZE + CMX_FILE_HEADER_SIZE;
        fs_ret = PVR_SEEK(p_play_priv->ufs_file, seek_pos, 0);
        if(fs_ret)
        {
            PVR_ADDON_PRINTF(P_PRINT_ERROR, "ufs_lseek fail   p_play_priv->cmx_idx = %d\n",p_play_priv->cmx_idx);
            return 0;
        }
        u32 read_size = CMX_FILE_UNIT_SIZE * list_cnt;
        memset(p_play_priv->data, 0, CMX_FILE_UNIT_SIZE * CMX_FILE_UNIT_NUM);
        fs_ret = PVR_READ(p_play_priv->data, read_size, p_play_priv->ufs_file, 0);
        if(fs_ret != read_size)
        {
            PVR_ADDON_PRINTF(P_PRINT_ERROR, "ufs_read fail p_play_priv->cmx_idx = %d  %d\n",ret_size,fs_ret);
            return 0;
        }
        PVR_ADDON_PRINTF(P_PRINT_INFO, "seek_pos=%lld cur_cnt=%d read_size=%lu ret_size=%lu", seek_pos, cur_cnt, read_size, ret_size);
        p_play_priv->cmx_idx =cur_cnt;
        p_play_priv->cmx_idx_num =list_cnt;
        PVR_ADDON_PRINTF(P_PRINT_INFO, "cmx_idx=%d cmx_idx_num=%d", p_play_priv->cmx_idx, p_play_priv->cmx_idx_num);
    }

    if(cur_cnt>=p_play_priv->cmx_idx && cur_cnt< (p_play_priv->cmx_idx+p_play_priv->cmx_idx_num))
    {
        mt_pvr_addon_cmx_file_t *p_file_data;
        int start_cnt = cur_cnt-p_play_priv->cmx_idx;
        PVR_ADDON_PRINTF(P_PRINT_INFO, "start_cnt=%d", start_cnt);
        //for(i=0;i<need_cnt;i++)
        {
            p_file_data= (mt_pvr_addon_cmx_file_t*)(&p_play_priv->data[(start_cnt)*CMX_FILE_UNIT_SIZE]);
            age_limit = p_file_data->age_limit;
            PVR_ADDON_PRINTF(P_PRINT_INFO, "age_limit=%d", age_limit);
        }
    }
    PVR_ADDON_PRINTF(P_PRINT_INFO, "age_limit=%d", age_limit);
    return age_limit;
}
*/
static MPVR_ADDON_STATUS mt_pvr_common_addon_play_create(mt_handle_t *p_handle, mt_pvr_addon_play_attr_t *p_play_attr)
{
    mt_pvr_addon_init_common_t *priv =get_pvr_common_addon_priv();
    mt_pvr_addon_common_play_priv_t *p_play_priv = NULL;
    PVR_ADDON_PRINTF(P_PRINT_INFO, "entry");
    if(!priv)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }
    p_play_priv =malloc(sizeof(mt_pvr_addon_common_play_priv_t));
    if(!p_play_priv)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }

    *p_handle = (mt_handle_t)(ulong)p_play_priv;
    memset(p_play_priv,0,sizeof(mt_pvr_addon_common_play_priv_t));
    p_play_priv->cmx_idx = -1;

    p_play_priv->encrypt_flag = p_play_attr->pvr_encrypt_flag;
/*
    if(priv->age_limit_check!=NULL)
    {
        u8 new_file_name[MAX_FILE_PATH] = {0};
        u8 age_limit = 0;
        u8 i = 0;
        u32 read_size = 0;
        u8 fs_ret  =0;
        mt_pvr_addon_cmx_header_t header = {0};
        mt_pvr_addon_common_priv_t *local_priv = get_pvr_common_addon_local_priv();
        if (!local_priv)
        {
            return MPVR_ADDON_STATUS_FAILED;
        }
        mt_pvr_common_addon_file_by_suffix( p_play_attr->file_name,".cmx",new_file_name,MAX_FILE_PATH);
        print_uni(new_file_name," new_file_name:");
        p_play_priv->file_size = _fs_get_fsize(new_file_name);

        p_play_priv->ufs_file = PVR_OPEN((const char *)new_file_name, PVR_FOPEN_MODE_INDEX_WRITE);

        if(p_play_priv->ufs_file == -1)
        {
            free(p_play_priv);
            PVR_ADDON_PRINTF(P_PRINT_ERROR, "fs_ret=%d", fs_ret);
            *p_handle = NULL;
            return MPVR_ADDON_STATUS_CREATE_FAIL;
        }
        memcpy(p_play_priv->new_file_name,new_file_name,MAX_FILE_PATH);
        if(p_play_priv->file_size>CMX_FILE_HEADER_SIZE)
            p_play_priv->max_cnt =( p_play_priv->file_size-CMX_FILE_HEADER_SIZE)/CMX_FILE_UNIT_SIZE;

        fs_ret = PVR_SEEK(p_play_priv->ufs_file, 0, 0);

        read_size = PVR_READ(&header, sizeof(mt_pvr_addon_cmx_header_t), p_play_priv->ufs_file, 0);
        if(read_size !=sizeof(mt_pvr_addon_cmx_header_t))
        {
            free(p_play_priv);
            PVR_ADDON_PRINTF(P_PRINT_ERROR, "fs_ret=%d", fs_ret);
            return MPVR_ADDON_STATUS_CREATE_FAIL;
        }

        p_play_priv->encrypt_flag = header.encrypt_flag;

        for(i =0; i<CMX_MAX_REC_FILE_NUM; i++)
        {
            if(local_priv->rec_file[i].bValid  && strcmp(new_file_name, local_priv->rec_file[i].file_name) == 0)
            {
                p_play_priv->file_idx= i;
                p_play_priv->rec_file_valid = 1;
                OS_PRINTF("p_play_priv->file_idx ---------=%d\n",p_play_priv->file_idx );
                break;
            }
        }
        local_priv->play_file.handle = (u32)*p_handle;
        strcpy(local_priv->play_file.file_name, new_file_name);
        local_priv->play_file.bValid = 1;

        age_limit = mt_pvr_common_addon_get_age_limit(p_play_priv,0,0);
        PVR_ADDON_PRINTF(P_PRINT_INFO, "age_limit=%d", age_limit);
    }
*/
    if(priv->play_handle_cbk)
        priv->play_handle_cbk(priv->p_args,p_play_attr->file_name,p_play_attr->name_len, 0);
    PVR_ADDON_PRINTF(P_PRINT_INFO, "return");
    return MPVR_ADDON_STATUS_OK;
}

static MPVR_ADDON_STATUS mt_pvr_common_addon_play_destroy(mt_handle_t handle)
{
    mt_pvr_addon_init_common_t *priv =get_pvr_common_addon_priv();
    mt_pvr_addon_common_play_priv_t *p_play_priv;

    PVR_ADDON_PRINTF(P_PRINT_INFO, "entry, handle=%p", handle);
    if(!priv)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }

    p_play_priv = (mt_pvr_addon_common_play_priv_t *)handle;
    if (!p_play_priv)
    {
        return MPVR_ADDON_STATUS_OK;
    }

    if(priv->age_limit_check!=NULL)
    {
        if (p_play_priv->ufs_file)
            PVR_CLOSE(p_play_priv->ufs_file);
    }

    free((void *)handle);
    PVR_ADDON_PRINTF(P_PRINT_INFO, "return");
    return MPVR_ADDON_STATUS_OK;
}

static MPVR_ADDON_STATUS mt_pvr_common_addon_play_dataprocess(mt_handle_t handle, u32 dmx_id, mt_pvr_addon_play_input_param_t* p_param_in,
        mt_pvr_addon_play_output_param_t * p_param_out)
{
    mt_pvr_addon_init_common_t *priv = get_pvr_common_addon_priv();
    mt_pvr_data_process_cb_param_t param = {0};
    mt_pvr_addon_common_play_priv_t *p_play_priv;    
    RET_CODE ret = 0;

    p_play_priv = (mt_pvr_addon_common_play_priv_t *)handle;
    if(!priv || !p_play_priv)
    {
        return MPVR_ADDON_STATUS_FAILED;
    }

    /*
    if(priv->age_limit_check)
    {
        u8 age_limit = 8;
        u16 cur_cnt = 0;
        memcpy(&cur_cnt,&(p_param_in->addon_priv),2);
        age_limit = mt_pvr_common_addon_get_age_limit(p_play_priv,cur_cnt, p_param_in->pos);
        while(1)
        {
            int ret = ERR_FAILURE;

            ret = priv->age_limit_check(0,priv->p_args, age_limit);
            if(ret == MPVR_ADDON_COMMON_AGE_LIMIT_EXIT)
            {
                return MPVR_ADDON_STATUS_EXIT_PLAY;
            }
            if(ret != SUCCESS)
            {
                continue;
            }
            else
            {
                break;
            }
        }
    }
*/
    if(!p_play_priv->encrypt_flag)
        return MPVR_ADDON_STATUS_OK;

    param.handle = (void *)handle;
    param.type = MPVR_PROCESS_TYPE_DECRYPT;
    param.p_dest =p_param_in->p_dest_data;
    param.p_src = p_param_in->p_src_data;
    param.p_dest_phy =p_param_in->p_dest_data_phy;
    param.p_src_phy = p_param_in->p_src_data_phy;

    param.length= p_param_in->len;
    param.p_args= priv->p_args;

    if(priv->data_process_cb)
        ret = priv->data_process_cb(&param);
    if(ret ==0)
    {
        return MPVR_ADDON_STATUS_OK;
    }
    else
        return MPVR_ADDON_STATUS_FAILED;

    return MPVR_ADDON_STATUS_OK;
}


void* mt_pvr_common_addon_create(mt_pvr_addon_init_common_t* args)
{
    mt_pvr_addon_t * p_addon = NULL;

    if (!g_pst_common_addon)
    {
        g_pst_common_addon = malloc(sizeof(mt_pvr_addon_common_priv_t));
        if (!g_pst_common_addon)
        {
            return NULL;
        }
        memset(g_pst_common_addon, 0, sizeof(mt_pvr_addon_common_priv_t));

        p_addon = malloc(sizeof(mt_pvr_addon_t));
        if (!p_addon)
        {
            return NULL;
        }
        memset(p_addon, 0, sizeof(mt_pvr_addon_t));
        g_pst_common_addon->addon = p_addon;
    }


    p_addon = g_pst_common_addon->addon ;
    memset(p_addon->handlesrec_pvr,0xff,sizeof(p_addon->handlesrec_pvr));
    if(p_addon->p_args == NULL)
        p_addon->p_args = malloc(sizeof(mt_pvr_addon_init_common_t));

    if(p_addon->p_args == NULL)
        return  NULL;

    memcpy(p_addon->p_args,args,sizeof(mt_pvr_addon_init_common_t));

    p_addon->version.major = 1;
    p_addon->version.minor = 0;

    p_addon->on_delete = mt_pvr_common_addon_delete_file;
    p_addon->on_rename = mt_pvr_common_addon_rename_file;

    p_addon->rec_inter.create = mt_pvr_common_addon_rec_create;
    p_addon->rec_inter.on_destroy = mt_pvr_common_addon_rec_destroy;
    p_addon->rec_inter.data_process = mt_pvr_common_addon_rec_dataprocess;

    p_addon->play_inter.create = mt_pvr_common_addon_play_create;
    p_addon->play_inter.on_destroy = mt_pvr_common_addon_play_destroy;
    p_addon->play_inter.data_process = mt_pvr_common_addon_play_dataprocess;

    return p_addon;
}

void mt_pvr_set_ca_addon(void      *p_addon)
{
    g_pvr_ca_addon = (mt_pvr_addon_t *)p_addon;
    if(g_pvr_ca_addon && g_pvr_ca_addon->init)
    {
        MPVR_ADDON_STATUS ret = MPVR_ADDON_STATUS_OK;
        if(MPVR_ADDON_STATUS_OK != (ret = g_pvr_ca_addon->init(g_pvr_ca_addon->p_args)))
        {
            PVR_ADDON_PRINTF(P_PRINT_ERROR, "init failed:%d",ret);
        }
    }
}

mt_pvr_addon_t *mt_pvr_get_ca_addon()
{
    return g_pvr_ca_addon;
}

void mt_pvr_addon_on_delete(u8 *p_ts_file_name)
{
    if(g_pvr_ca_addon && g_pvr_ca_addon->on_delete)
    {
        MPVR_ADDON_STATUS ret = MPVR_ADDON_STATUS_OK;
        if(MPVR_ADDON_STATUS_OK != (ret = g_pvr_ca_addon->on_delete(p_ts_file_name)))
        {
            PVR_ADDON_PRINTF(P_PRINT_ERROR, "delete failed:%d",ret);
        }
    }
}
