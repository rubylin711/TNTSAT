#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include "mt_unf_flash.h"
#include "mt_adp_config.h"
#include "mt_adp_mpi.h"

#include <stdlib.h>
#include <string.h>


#ifdef MTADP_CONFIG_DEBUG
#define MT_CONFIG_PRINT   printf 
#else
#define MT_CONFIG_PRINT
#endif

#define SAMPLE_CONFIG_FUNCTION_ENTER()             MT_CONFIG_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_CONFIG_FUNCTION_EXIT()              MT_CONFIG_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)
 
#define SAMPLE_CONFIG_FATAL_PRINT(fmt...)          MT_CONFIG_PRINT(" [FATAL] " fmt)
#define SAMPLE_CONFIG_ERR_PRINT(fmt...)            MT_CONFIG_PRINT(" [ERROR] " fmt)
#define SAMPLE_CONFIG_WARN_PRINT(fmt...)           MT_CONFIG_PRINT(" [WARN] "  fmt)
#define SAMPLE_CONFIG_INFO_PRINT(fmt...)           MT_CONFIG_PRINT(" [INFO] "  fmt)
#define SAMPLE_CONFIG_DBG_PRINT(fmt...)            MT_CONFIG_PRINT(" [DEBUG] " fmt)

#define MAX_LINE_LENGTH 256


#ifdef CONFIG_MT_CHIP_SYMPHONY4
#define MTDDEV "/dev/mtd9"
#endif

mt_s32 MTADP_Find_Device(mt_char *mtdedv) 
{
    mt_char targetDevice[] = "sampledb"; 
    mt_char line[MAX_LINE_LENGTH];
    mt_s32 mtdNumber = -1; 
    
    FILE *file = fopen("/proc/mtd", "r");

    if (file == NULL) 
    {
        SAMPLE_CONFIG_ERR_PRINT("Error opening /proc/mtd");
        return MT_FAILURE;
    }

  
    while (fgets(line, sizeof(line), file)) 
    {
        char *name_start = strstr(line, "\"");  
        char *name_end = NULL;

        if (name_start != NULL) 
        {
            name_start++;  
            name_end = strstr(name_start, "\""); 

            if (name_end != NULL) 
            {
                *name_end = '\0'; 

                if (strcmp(name_start, targetDevice) == 0)
                {             
                    sscanf(line, "mtd%d:", &mtdNumber);
                    break;
                }
            }
        }
    }

    fclose(file);

    if (mtdNumber != -1) 
    {
        SAMPLE_CONFIG_INFO_PRINT("MTD Device %s is associated with mtd%d\n", targetDevice, mtdNumber);
        snprintf(mtdedv, 256, "/dev/mtd%d", mtdNumber);
    
        return MT_SUCCESS;
    }
    else 
    {
        SAMPLE_CONFIG_ERR_PRINT("MTD Device %s not found\n", targetDevice);
        return MT_FAILURE;
    }

   
    
}








mt_s32 MTADP_Config_Set_Default(config_t *config)
{
#ifdef CONFIG_MT_CHIP_SYMPHONY6 
    config->ratio.enable = MT_TRUE;
    config->ratio.enDispAspectRatio = MT_UNF_DISP_ASPECT_RATIO_AUTO;
    config->ratio.enAspectCvrs = MT_UNF_VO_ASPECT_CVRS_IGNORE;
    config->hdcp = MT_TRUE;
#endif
    config->disp_fmt.format = MT_UNF_ENC_FMT_1080i_50;
    return MT_SUCCESS;
}


mt_s32 MTADP_Read_All_Config(config_t *config)
{
    mt_s32 s32Ret = 0;
    MT_HANDLE hmtdblock = MT_INVALID_HANDLE;
    MT_U8   readBuffer[sizeof(config_t)];
    mt_char mtdedv[256];
    
#ifdef CONFIG_MT_CHIP_SYMPHONY6    
    s32Ret = MTADP_Find_Device(mtdedv);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("MTADP_Find_Device failed.ret = %#x\n",s32Ret);
        return MT_FAILURE;
    }
#endif    
    s32Ret = mt_unf_flash_init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("mt_unf_flash_init failed.ret = %#x\n",s32Ret);
        return MT_FAILURE;
    }
      
#ifdef CONFIG_MT_CHIP_SYMPHONY4    
    s32Ret = mt_unf_flash_open(MTDDEV, &hmtdblock);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("mt_unf_flash_open failed.ret = %#x\n",s32Ret);
        (MT_VOID)mt_unf_flash_deinit();
        return MT_FAILURE;
    }
#else
    s32Ret = mt_unf_flash_open(mtdedv, &hmtdblock);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("mt_unf_flash_open failed.ret = %#x\n",s32Ret);
        (MT_VOID)mt_unf_flash_deinit();
        return MT_FAILURE;
    }
#endif
    memset(readBuffer, 0, sizeof(config_t));
    s32Ret = mt_unf_flash_read(hmtdblock, 0, readBuffer, sizeof(config_t));
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("mt_unf_flash_read failed.  ret = %#x\n", s32Ret);
        (MT_VOID)mt_unf_flash_close(hmtdblock);
        (MT_VOID)mt_unf_flash_deinit();
        return MT_FAILURE;
    }
    
    memcpy(config, readBuffer, sizeof(config_t));
    
    
    (MT_VOID)mt_unf_flash_close(hmtdblock);

    (MT_VOID)mt_unf_flash_deinit();

    return MT_SUCCESS;
}


mt_s32 MTADP_Write_All_Config(config_t *config)
{
    mt_s32 s32Ret = 0;
    MT_HANDLE hmtdblock = MT_INVALID_HANDLE;
    MT_U8   writeBuffer[sizeof(config_t)];
        mt_char mtdedv[256];
    
#ifdef CONFIG_MT_CHIP_SYMPHONY6    
    s32Ret = MTADP_Find_Device(mtdedv);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("MTADP_Find_Device failed.ret = %#x\n",s32Ret);
        return MT_FAILURE;
    }
#endif  

    
    s32Ret = mt_unf_flash_init();
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("mt_unf_flash_init failed.ret = %#x\n",s32Ret);
        return MT_FAILURE;
    }
      
#ifdef CONFIG_MT_CHIP_SYMPHONY4 
    s32Ret = mt_unf_flash_open(MTDDEV, &hmtdblock);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("mt_unf_flash_open failed.ret = %#x\n",s32Ret);
        (MT_VOID)mt_unf_flash_deinit();
        return MT_FAILURE;
    }
#else
    s32Ret = mt_unf_flash_open(mtdedv, &hmtdblock);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("mt_unf_flash_open failed.ret = %#x\n",s32Ret);
        (MT_VOID)mt_unf_flash_deinit();
        return MT_FAILURE;
    }
#endif

    memcpy(writeBuffer, config, sizeof(config_t));

    
    s32Ret = mt_unf_flash_write(hmtdblock, 0, writeBuffer, sizeof(config_t));
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_CONFIG_ERR_PRINT("mt_unf_flash_write failed. ret = %#x\n", s32Ret);
        return MT_FAILURE;
    }

    (MT_VOID)mt_unf_flash_close(hmtdblock);

    (MT_VOID)mt_unf_flash_deinit();
    
    return MT_SUCCESS;
}




