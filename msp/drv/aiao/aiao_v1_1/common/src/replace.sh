#!/bin/sh
echo "start"
sed -i "s/HI/MT/g" *.*
sed -i "s/hi_u32/mt_u32/g" *.*
sed -i "s/HI_U32/mt_u32/g" *.*
sed -i "s/HI_S32/mt_s32/g" *.*
sed -i "s/HI_VOID/mt_void/g" *.*
sed -i "s/HI_CHAR/mt_char/g" *.*
sed -i "s/HI_U8/mt_u8/g" *.*
sed -i "s/HI_U8/mt_s8/g" *.*
sed -i "s/HI_U16/mt_u16/g" *.*
sed -i "s/HI_S16/mt_s16/g" *.*
sed -i "s/hi_handle/mt_handle/g" *.*
sed -i "s/mmz_buffer_s/mmz_buffer_s/g" *.*
sed -i "s/basedev_s/basedev_s/g" *.*
sed -i "s/mt_drv_module_getfunction/mt_drv_module_getfunction/g" *.*
sed -i "s/mt_drv_mmz_alloc/mt_drv_mmz_alloc/g" *.*
sed -i "s/mt_drv_mmz_map_cache/mt_drv_mmz_map_cache/g" *.*
sed -i "s/mt_drv_mmz_allocAndMap/mt_drv_mmz_alloc_and_map/g" *.*
sed -i "s/mt_drv_mmz_unmap/mt_drv_mmz_unmap/g" *.*
sed -i "s/mt_drv_mmz_release/mt_drv_mmz_release/g" *.*
sed -i "s/mt_drv_mmz_unmapAndRelease/mt_drv_mmz_unmap_and_release/g" *.*
sed -i "s/mt_drv_file_write/mt_drv_file_write/g" *.*
sed -i "s/mt_drv_file_close/mt_drv_file_close/g" *.*
sed -i "s/mt_drv_file_get_storepath/mt_drv_file_get_storepath/g" *.*
sed -i "s/mt_drv_file_open/mt_drv_file_open/g" *.*
sed -i "s/mt_drv_proc_rm_module/mt_drv_proc_rm_module/g" *.*
sed -i "s/mt_drv_proc_echohelp/mt_drv_proc_echohelp/g" *.*
sed -i "s/mt_drv_proc_add_module/mt_drv_proc_add_module/g" *.*
sed -i "s/mt_drv_usercopy/mt_drv_usercopy/g" *.*
sed -i "s/THIS_MODULE/THIS_MODULE/g" *.*
sed -i "s/baseops_s/baseops_s/g" *.*
sed -i "s/baseops_s/baseops_s/g" *.*
sed -i "s/mt_device_s/mt_device_s/g" *.*
sed -i "s/MT_UNF_SND_SPDIF_SCMSMODE_COPYPROHIBITED/MT_UNF_SND_SPDIF_SCMSMODE_COPYPROHIBITED/g" *.*
echo "finish"

