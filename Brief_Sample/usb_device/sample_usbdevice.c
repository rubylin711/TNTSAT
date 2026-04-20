#include <fcntl.h>
#include <unistd.h>
#include "mt_unf_common.h"
#include "mt_unf_ecs.h"
#include "mt_type.h"
#include "mt_debug.h"
#include "mt_unf_demux.h"
#include "mt_unf_descrambler.h"
#include <pthread.h>

#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <string.h>
#include "mt_unf_disp.h"
#include "mt_unf_common.h"
#include "mt_unf_demux.h"
#include "mt_unf_ecs.h"
#include "mt_unf_vo.h"
#include "mt_unf_avplay.h"
#include "mt_unf_sound.h"
#include "mt_mpi_demux.h"

#include "mt_adp_hdmi.h"
#include "mt_adp_boardcfg.h"
#include "mt_adp_mpi.h"

/***************************** Macro Definition ******************************/
#ifdef MT_SAMPLE_USB_DEBUG

#define MT_USB_PRINT   printf
#else

#define MT_USB_PRINT

#endif

#define SAMPLE_USB_FUNCTION_ENTER() MT_USB_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_USB_FUNCTION_EXIT()      MT_USB_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_USB_FATAL_PRINT(fmt...)  MT_USB_PRINT(" [FATAL] " fmt)
#define SAMPLE_USB_ERR_PRINT(fmt...)        MT_USB_PRINT(" [ERROR] " fmt)
#define SAMPLE_USB_WARN_PRINT(fmt...)       MT_USB_PRINT(" [WARN] "  fmt)
#define SAMPLE_USB_INFO_PRINT(fmt...)       MT_USB_PRINT(" [INFO] "  fmt)
#define SAMPLE_USB_DBG_PRINT(fmt...)        MT_USB_PRINT(" [DEBUG] " fmt)

#define MT_TASK_RUN            1
#define MT_TASK_EXIT           2

static MT_BOOL g_bTaskQuit = MT_TRUE;

#ifdef MT_SAMPLE_APP
MT_S32 MT_UsbdeviceMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static MT_VOID MT_UsbdevicePrintMenu(MT_VOID)
{
    MT_USB_PRINT("\n");
    MT_USB_PRINT("     e : device mode \n");
    MT_USB_PRINT("     d : host mode \n");
#ifdef MT_SAMPLE_APP
    MT_USB_PRINT("     b : background run \n");
#endif
    MT_USB_PRINT("     h : help \n");
    MT_USB_PRINT("     q : quit \n");
    MT_USB_PRINT("usb_device>> ");

}

static MT_VOID MT_UsbdeviceExit(void)
{
    system("usbmodeswitch.sh 0");
    g_bTaskQuit = MT_TRUE;
}
static void MT_UsbdeviceCmdTask(void)
{
    MT_CHAR    inputCmd[32] = { 0 };
    MT_CHAR    *pfgetret = NULL;
    MT_S32     usb_mode = 0;;

    while(1)
    {
        (MT_VOID)MT_UsbdevicePrintMenu();
        pfgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        pfgetret = pfgetret;

        if('q' == inputCmd[0])
        {
            SAMPLE_USB_INFO_PRINT("<Exit!>\n");
            if(usb_mode == 1)
            {
                system("usbmodeswitch.sh 0");
            }
            g_bTaskQuit = MT_TRUE;
            break;
        }
#ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_USB_INFO_PRINT("usb_device in back!\n");
            break;
        }
#endif
        else if('e' == inputCmd[0])
        {
            system("usbmodeswitch.sh 1");
            usb_mode = 1;
            continue;
        }
        else if('d' == inputCmd[0])
        {
            system("usbmodeswitch.sh 0");
            usb_mode = 0;
            continue;
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_USB_INFO_PRINT("Print help info \n");
            continue;
        }
    }
}

static MT_S32 MT_UsbdeviceParase_args(MT_S32 argc, MT_CHAR *argv[])
{
    int opt = 0;


    while((opt = MTADP_Getopt(argc, argv, ":?hH:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_UsbdeviceExit();
                }
                return MT_TASK_EXIT;


            default:
                return MT_FAILURE;
            break;
        }
    }


    return MT_SUCCESS;
}



#ifdef MT_SAMPLE_APP
MT_S32 MT_UsbdeviceMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    mt_s32 s32Ret = 0;

    if(argc != 1 && g_bTaskQuit == MT_TRUE)
    {
        return MT_SUCCESS;
    }

    /** Get the parameters */
    s32Ret = MT_UsbdeviceParase_args(argc, argv);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_USB_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_USB_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }
    if(g_bTaskQuit == MT_TRUE)
    {
        g_bTaskQuit = MT_FALSE;
    }
    (MT_VOID)MT_UsbdeviceCmdTask();
    SAMPLE_USB_INFO_PRINT( " done message in program\n " );
    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }


    return MT_SUCCESS;

}


