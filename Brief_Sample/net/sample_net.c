/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include "mt_unf_common.h"
#include "mt_api_eth.h"
#include "mt_adp_mpi.h"
/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_NET_DEBUG
#define MT_NET_PRINT   printf
#else
#define MT_NET_PRINT
#endif

#define SAMPLE_NET_FUNCTION_ENTER() MT_NET_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_NET_FUNCTION_EXIT()      MT_NET_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_NET_FATAL_PRINT(fmt...)      MT_NET_PRINT(" [FATAL] " fmt)
#define SAMPLE_NET_ERR_PRINT(fmt...)            MT_NET_PRINT(" [ERROR] " fmt)
#define SAMPLE_NET_WARN_PRINT(fmt...)           MT_NET_PRINT(" [WARN] "  fmt)
#define SAMPLE_NET_INFO_PRINT(fmt...)           MT_NET_PRINT(" [INFO] "  fmt)
#define SAMPLE_NET_DBG_PRINT(fmt...)            MT_NET_PRINT(" [DEBUG] " fmt)

#define SAMPLE_NET_PRINT   printf

#define MAX_IP_ADDR_STR      16
#define MAX_ADDR_STR         18
#define MAX_RETRY_COUNT      20

/*************************** Structure Definition ****************************/
typedef enum
{
    MT_NET_MODE_STATIC,     /**<STATIC*/ /**<CNcomment:静态IP模式*/
    MT_NET_MODE_DHCP,       /**<DHCP*/ /**<CNcomment:DHCP模式*/
    MT_NET_MODE_PPPOE,      /**<PPPOE*/ /**<CNcomment:DHCP模式*/
} mt_net_mode_t;

typedef enum
{
    MT_NET_OPERATE_SET_MAC  = 0x1,       /**<SET MAC*/ /**<CNcomment:设置Mac地址*/
    MT_NET_OPERATE_SET_IP   = 0x2,       /**<SET IP*/ /**<CNcomment:设置IP地址*/
    MT_NET_OPERATE_SET_MASK = 0x4,       /**<SET MASK*/ /**<CNcomment:设置子网掩码*/
    MT_NET_OPERATE_SET_GW   = 0x8,       /**<SET GATEWAY*/ /**<CNcomment:设置路由*/
    MT_NET_OPERATE_SET_DNS  = 0x10,      /**<SET DNS*/ /**<CNcomment:设置DNS*/
    MT_NET_OPERATE_SET_AUTO = 0xFE,      /**<SET ALL>*/ /**<CNcomment:DHCP/PPPOE模式下设置所有信息*/
    MT_NET_OPERATE_SET_ALL  = 0xFF,
}mt_net_operate_t;

typedef struct
{
    MT_CHAR mac[MAX_ADDR_STR];
    MT_CHAR ipaddr[MAX_IP_ADDR_STR];
    MT_CHAR netmask[MAX_IP_ADDR_STR];
    MT_CHAR getway[MAX_IP_ADDR_STR];
    MT_CHAR dns1[MAX_IP_ADDR_STR];
    MT_CHAR dns2[MAX_IP_ADDR_STR];
}mt_net_interface_t;

typedef struct
{
    Eth_Port_E eth_port;
    mt_net_mode_t net_mode;
    mt_net_operate_t operate;
    mt_net_interface_t ipinfo;

}mt_net_input_para_t;
/********************** Global Variable declaration **************************/
static const MT_CHAR eth_name[2][5] =
{
    "eth0",
    "eth1"
};

#ifdef MT_SAMPLE_APP
MT_S32 MT_NetMain(MT_S32 argc, MT_CHAR *argv[]);
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[]);
#endif

static MT_S32 MT_NetChackIPv4(MT_CHAR *ipaddr)
{
    MT_CHAR *p = MT_NULL;
    MT_U16   ip = 0;
    MT_U8   index = 0;
    MT_U8   count = 0;

    SAMPLE_NET_INFO_PRINT("Check ipaddr: %s \n", ipaddr);

    p = ipaddr;
    while( *p != '\0')
    {
        if(count == 0)
        {
            ip = strtol(p, NULL, 10);
            if(ip > 255)
            {
                SAMPLE_NET_ERR_PRINT("Check IPaddr [%d] error.\n", ip);
                return MT_FAILURE;
            }
            else if(index == 0 && ip == 255)
            {
                SAMPLE_NET_ERR_PRINT("Check IPaddr [%d] error.First ip is err.\n", ip);
                return MT_FAILURE;
            }
            index++;
        }
        if(*p >= '0' && *p <= '9')
        {
            count++;
        }
        else if(*p == '.')
        {
            count = 0;
        }
        else
        {
            SAMPLE_NET_ERR_PRINT("Check IPaddr [%s] error. ERR character.\n", ipaddr);
            return MT_FAILURE;
        }

        p++;
    }
    if(index != 4)
    {
        SAMPLE_NET_ERR_PRINT("Check IPaddr [%s] error.\n", ipaddr);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


static MT_S32 MT_NetChackMask(MT_CHAR *maskStr)
{
    MT_CHAR *p = MT_NULL;
    MT_U16   currentIP = 0;
    MT_U16   beforeIP = 255;
    MT_U8    index = 0;
    MT_U8    count = 0;
    MT_S32   k = 0;
    MT_BOOL  identify = MT_FALSE;
    SAMPLE_NET_INFO_PRINT("Check mask: %s \n", maskStr);

    p = maskStr;
    while( *p != '\0')
    {
        if(count == 0)
        {
            currentIP = strtol(p, NULL, 10);
            if(255 != beforeIP)
            {
                if(0 != currentIP)
                {
                    SAMPLE_NET_ERR_PRINT("Check mask [%d] error.\n", currentIP);
                    return MT_FAILURE;
                }
                index++;
            }
            else
            {
                for(int i = 7; i >= 0; i--)
                {
                    k |= 1 << i;
                    if(0 == currentIP)
                    {
                        identify = MT_TRUE;
                        break;
                    }
                    else if(k == currentIP)
                    {
                        identify = MT_TRUE;
                        break;
                    }
                }

                if(MT_FALSE == identify)
                {
                    SAMPLE_NET_ERR_PRINT("Check mask [%d] error.\n", currentIP);
                    return MT_FAILURE;
                }
                k = 0;
                identify = MT_FALSE;
                index++;
            }
            beforeIP = currentIP;
        }
        if(*p >= '0' && *p <= '9')
        {
            count++;
        }
        else if(*p == '.')
        {
            count = 0;
        }
        else
        {
            SAMPLE_NET_ERR_PRINT("Check mask [%s] error. ERR character.\n", maskStr);
            return MT_FAILURE;
        }

        p++;
    }
    if(4 != index)
    {
        SAMPLE_NET_ERR_PRINT("Check mask [%s] error.\n", maskStr);
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


static MT_S32 MT_NetChackMAC(const MT_CHAR *src)
{
    MT_CHAR *p = MT_NULL;
    MT_U8   index = 1;
    MT_U8   count = 0;

    SAMPLE_NET_INFO_PRINT("Check MAC: %s \n", src);

    p = src;
    while(*p != '\0')
    {
        if((*p >= '0' && *p <= '9') ||
            (*p >= 'a' && *p <= 'f') ||
            (*p >= 'A' && *p <= 'F'))
        {
            count++;
        }
        else if(*p == ':')
        {
            if('\0' == *(p + 1) || ':' == *(p + 1))
            {
                SAMPLE_NET_ERR_PRINT("Check MAC [%s] error. ERR character[%c].\n", src, *(p + 1));
                return MT_FAILURE;
            }

            count = 0;
            index++;
        }
        else
        {
            SAMPLE_NET_ERR_PRINT("Check MAC [%s] error. ERR character[%c].\n", src, *p);
            return MT_FAILURE;
        }

        if(count > 2)
        {
            SAMPLE_NET_ERR_PRINT("Check MAC [%s] error. ERR MAC bit.\n", src);
            return MT_FAILURE;
        }

        p++;
    }
    if(index != 6)
    {
        SAMPLE_NET_ERR_PRINT("Check MAC [%s] error. more mac bit. \n", src);
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}
static MT_S32 MT_NetChackParam(mt_net_input_para_t *pInputParam)
{
    MT_S32 ret = MT_FAILURE;

    SAMPLE_NET_INFO_PRINT("operate: %x \n", pInputParam->operate);
    if(MT_NET_OPERATE_SET_MAC == (MT_NET_OPERATE_SET_MAC & pInputParam->operate))
    {
        ret = MT_NetChackMAC(pInputParam->ipinfo.mac);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NET_ERR_PRINT("The MAC was entered incorrectly\n");
            return ret;
        }
    }

    if(MT_NET_OPERATE_SET_AUTO == (MT_NET_OPERATE_SET_AUTO & pInputParam->operate))
    {
        SAMPLE_NET_INFO_PRINT("DHCP mode not check IP.\n");
        return MT_SUCCESS;
    }

    if(MT_NET_OPERATE_SET_IP == (MT_NET_OPERATE_SET_IP & pInputParam->operate))
    {
        ret = MT_NetChackIPv4(pInputParam->ipinfo.ipaddr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NET_ERR_PRINT("The IP address was entered incorrectly\n");
            return ret;
        }
    }

    if(MT_NET_OPERATE_SET_MASK == (MT_NET_OPERATE_SET_MASK & pInputParam->operate))
    {
        ret = MT_NetChackMask(pInputParam->ipinfo.netmask);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NET_ERR_PRINT("The Mask was entered incorrectly\n");
            return ret;
        }
    }

    if(MT_NET_OPERATE_SET_GW == (MT_NET_OPERATE_SET_GW & pInputParam->operate))
    {
        ret = MT_NetChackIPv4(pInputParam->ipinfo.getway);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NET_ERR_PRINT("The GW was entered incorrectly\n");
            return ret;
        }
    }

    if(MT_NET_OPERATE_SET_DNS == (MT_NET_OPERATE_SET_DNS & pInputParam->operate))
    {
        ret = MT_NetChackIPv4(pInputParam->ipinfo.dns1);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NET_ERR_PRINT("The DNS was entered incorrectly\n");
            return ret;
        }
    }


    return MT_SUCCESS;
}


static MT_S32 MT_NetGWIterate(MT_CHAR getway[32])
{
    MT_S32  count = 0;
    MT_CHAR gwList[32][32] = { 0 };
    MT_CHAR buffer[128] = { 0 };
    FILE    *fp = MT_NULL;
    MT_CHAR readline[100] = { 0 };

    fp = popen("route |grep default|awk \'{print $2}\'", "r" );
    if(NULL == fp)
    {
        return MT_FAILURE;
    }

    memset(readline, 0, sizeof(readline));
    while(NULL != fgets(readline, sizeof(readline),fp))
    {
        if(readline[strlen(readline)-1] == '\n')
        {
           readline[strlen(readline)-1] = 0;
        }
        strcpy(gwList[count], readline);

        count = count + 1;
    }

    for(int j = 0; j < count; j++)
    {
        if(0 == strcmp(gwList[j], getway))
        {
            sprintf(buffer, "route del default gw %s", getway);
            if(system(buffer) < 0)
            {
            SAMPLE_NET_ERR_PRINT("error occured\n");
            }
            break;
        }
    }

    pclose(fp);
    return MT_SUCCESS;
}


static MT_VOID MT_NetGetCurNetInfo(Eth_Port_E ethPort, mt_net_interface_t *getInfo)
{
    (MT_VOID)MT_ETH_GatewayGet(ethPort, getInfo->getway);

    (MT_VOID)MT_ETH_IPAddressGet(ethPort, getInfo->ipaddr);

    (MT_VOID)MT_ETH_SubNetmaskGet(ethPort, getInfo->netmask);

    (MT_VOID)MT_ETH_DNSGet(ethPort, getInfo->dns1);

    (MT_VOID)MT_ETH_GetMac(ethPort, getInfo->mac);
}
static MT_VOID MT_NetSetDhcp(mt_net_input_para_t *pInputParam)
{
    MT_S32             ret = 0;
    MT_CHAR            buffer[128] = { 0 };
    mt_net_interface_t getInfo = { 0 };

    if(MT_NET_OPERATE_SET_MAC & pInputParam->operate)
    {
        ret = MT_ETH_Close(pInputParam->eth_port);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NET_ERR_PRINT(" MT_ETH_Open failed.ret = %#x\n",ret);
            return;
        }

        (MT_VOID)MT_ETH_SetMac(pInputParam->eth_port, pInputParam->ipinfo.mac);

        ret = MT_ETH_Open(pInputParam->eth_port);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NET_ERR_PRINT(" MT_ETH_Open failed.ret = %#x\n",ret);
            return;
        }
    }

    sprintf(buffer, "udhcpc -t 4 -n -i %s 2>&1 >/dev/null", eth_name[pInputParam->eth_port]);
    if(system(buffer) < 0)
    {
        SAMPLE_NET_ERR_PRINT("error occured\n");
    }

    (MT_VOID)MT_NetGetCurNetInfo(pInputParam->eth_port, &getInfo);

    SAMPLE_NET_PRINT("\n");
    SAMPLE_NET_PRINT("The network configuration obtained after setting\n");
    SAMPLE_NET_INFO_PRINT("Interface:  %s \n", eth_name[pInputParam->eth_port]);
    SAMPLE_NET_INFO_PRINT("Mac:        %s \n", getInfo.mac);
    SAMPLE_NET_INFO_PRINT("IPaddr:     %s \n", getInfo.ipaddr);
    SAMPLE_NET_INFO_PRINT("Subnetmask: %s \n", getInfo.netmask);
    SAMPLE_NET_INFO_PRINT("Getway:     %s \n", getInfo.getway);
    SAMPLE_NET_INFO_PRINT("DNS1:       %s \n", getInfo.dns1);

    sprintf(buffer, "ping %s -c 4", getInfo.getway);
    if(system(buffer) < 0)
    {
        SAMPLE_NET_ERR_PRINT("error occured\n");
    }
}

static MT_S32 MT_NetSetStatic(mt_net_input_para_t *pInputParam)
{
    MT_S32             ret = 0;
    MT_CHAR            buffer[128] = { 0 };
    mt_net_interface_t getInfo = { 0 };
    mt_net_interface_t curInfo = { 0 };

    SAMPLE_NET_PRINT("\n");
    SAMPLE_NET_PRINT("The network configuration obtained after setting\n");

    (MT_VOID)MT_NetGetCurNetInfo(pInputParam->eth_port, &curInfo);

    if(MT_NET_OPERATE_SET_MAC & pInputParam->operate)
    {
        ret = MT_ETH_Close(pInputParam->eth_port);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NET_ERR_PRINT(" MT_ETH_Close failed.ret = %#x\n", ret);
            return MT_FAILURE;
        }

        if(0 != strcmp(curInfo.mac, pInputParam->ipinfo.mac))
        {
            ret = MT_ETH_SetMac(pInputParam->eth_port, pInputParam->ipinfo.mac);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_NET_ERR_PRINT(" MT_ETH_SetMac failed.ret = %#x\n", ret);
                return MT_FAILURE;
            }

            (MT_VOID)MT_ETH_GetMac(pInputParam->eth_port, getInfo.mac);
            SAMPLE_NET_INFO_PRINT("Mac:        %s-->%s\n", curInfo.mac, getInfo.mac);
        }
        else
        {
            SAMPLE_NET_WARN_PRINT("Mac:        %s[Unchanged]\n", curInfo.mac);
        }

        ret = MT_ETH_Open(pInputParam->eth_port);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_NET_ERR_PRINT(" MT_ETH_Open failed.ret = %#x\n", ret);
            return MT_FAILURE;
        }
    }

    if(MT_NET_OPERATE_SET_IP & pInputParam->operate)
    {
        if(0 != strcmp(curInfo.ipaddr, pInputParam->ipinfo.ipaddr))
        {
            ret = MT_ETH_IPAddressSet(pInputParam->eth_port, pInputParam->ipinfo.ipaddr);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_NET_ERR_PRINT(" MT_ETH_IPAddressSet failed.ret = %#x\n", ret);
                return MT_FAILURE;
            }

            (MT_VOID)MT_ETH_IPAddressGet(pInputParam->eth_port, getInfo.ipaddr);
            SAMPLE_NET_INFO_PRINT("IPaddr:     %s-->%s\n", curInfo.ipaddr, getInfo.ipaddr);
        }
        else
        {
            SAMPLE_NET_WARN_PRINT("IPaddr:     %s[Unchanged]\n", curInfo.ipaddr);
        }
    }

    if(MT_NET_OPERATE_SET_MASK & pInputParam->operate)
    {
        if(0 != strcmp(curInfo.netmask, pInputParam->ipinfo.netmask))
        {
            ret = MT_ETH_SubNetmaskSet(pInputParam->eth_port, pInputParam->ipinfo.netmask);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_NET_ERR_PRINT(" MT_ETH_SubNetmaskSet failed.ret = %#x\n", ret);
                return MT_FAILURE;
            }

            (MT_VOID)MT_ETH_SubNetmaskGet(pInputParam->eth_port, getInfo.netmask);
            SAMPLE_NET_INFO_PRINT("Subnetmask: %s-->%s\n", curInfo.netmask, getInfo.netmask);
        }
        else
        {
            SAMPLE_NET_WARN_PRINT("Subnetmask: %s[Unchanged]\n", curInfo.netmask);
        }
    }

    if(MT_NET_OPERATE_SET_GW & pInputParam->operate)
    {
        if(0 != strcmp(curInfo.getway, pInputParam->ipinfo.getway))
        {
            ret = MT_NetGWIterate(pInputParam->ipinfo.getway);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_NET_ERR_PRINT("MT_NetGWIterate failed.ret = %#x\n", ret);
                return MT_FAILURE;
            }

            ret = MT_ETH_GatewaySet(pInputParam->eth_port, pInputParam->ipinfo.getway);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_NET_ERR_PRINT("MT_ETH_GatewaySet failed.ret = %#x\n", ret);
                return MT_FAILURE;
            }

            (MT_VOID)MT_ETH_GatewayGet(pInputParam->eth_port, getInfo.getway);
            SAMPLE_NET_INFO_PRINT("Getway:     %s-->%s\n", curInfo.getway, getInfo.getway);
        }
        else
        {
            SAMPLE_NET_WARN_PRINT("Getway:     %s[Unchanged]\n", curInfo.getway);
        }
    }

    if(MT_NET_OPERATE_SET_DNS & pInputParam->operate)
    {
        if(0 != strcmp(curInfo.dns1, pInputParam->ipinfo.dns1))
        {
            ret = MT_ETH_DNSSet(pInputParam->eth_port, MT_TRUE, pInputParam->ipinfo.dns1);
            if(MT_SUCCESS != ret)
            {
                SAMPLE_NET_ERR_PRINT(" MT_ETH_DNSSet failed.ret = %#x\n", ret);
                return MT_FAILURE;
            }

            (MT_VOID)MT_ETH_DNSGet(pInputParam->eth_port, getInfo.dns1);
            SAMPLE_NET_INFO_PRINT("DNS1:       %s-->%s\n", curInfo.dns1, getInfo.dns1);
        }
        else
        {
            SAMPLE_NET_WARN_PRINT("DNS1:       %s[Unchanged]\n", curInfo.dns1);
        }
    }

    SAMPLE_NET_PRINT("\n");

    if(system(buffer) < 0)
    {
        SAMPLE_NET_ERR_PRINT("error occured\n");
    }

    return MT_SUCCESS;
}


static void MT_NetPrint_Help(MT_CHAR *name)
{
    SAMPLE_NET_PRINT("Lack of parameters\n");
    SAMPLE_NET_PRINT("\nUsage:\n");
    SAMPLE_NET_PRINT("%s\n", name);
    SAMPLE_NET_PRINT("  -i Select the network port(default is eth0), param<eth0/eth1>\n");
    SAMPLE_NET_PRINT("  -t Select DHCP and Static modes(default is Static mode), parma<dhcp/static>\n");
    SAMPLE_NET_PRINT("  -s Set the IP address\n");
    SAMPLE_NET_PRINT("  -m Set the subnet mask\n");
    SAMPLE_NET_PRINT("  -g Set up the gateway\n");
    SAMPLE_NET_PRINT("  -d Set up DNS\n");
    SAMPLE_NET_PRINT("  -M Set the MAC address\n");
    SAMPLE_NET_PRINT("You can split the commands according to your usage needs to set the network settings you want to change\n");
    SAMPLE_NET_PRINT("  Set the Static mode incomplete command:\n");
    SAMPLE_NET_PRINT("     %s -s 10.48.146.17 -m 255.255.224.0 -g 10.48.128.1 -d 10.48.128.12 -M 00:1B:11:17:00:16\n", name);
    SAMPLE_NET_PRINT("  Set the Static mode full command:\n");
    SAMPLE_NET_PRINT("     %s -i eth0 -t static -s 10.48.146.17 -m 255.255.224.0 -g 10.48.128.1 -d 10.48.128.12 -M 00:1B:11:17:00:16\n", name);
    SAMPLE_NET_PRINT("  Set the DHCP mode(MAC is not set):\n");
    SAMPLE_NET_PRINT("     %s -i eth0 -t dhcp\n", name);
    SAMPLE_NET_PRINT("  Set the DHCP mode(Set up MAC):\n");
    SAMPLE_NET_PRINT("     %s -i eth0 -t dhcp -M 00:1B:11:17:00:16\n", name);
}


static MT_S32 MT_NetParase_args(MT_S32 argc, MT_CHAR *argv[], mt_net_input_para_t *pInputParam)
{
    MT_S32 opt = 0;

    if(argc < 2)
    {
        (MT_VOID)MT_NetPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    while((opt = MTADP_Getopt(argc, argv, ":?hHt:s:m:g:d:i:M:")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_NetPrint_Help(argv[0]);
                return MT_FAILURE;
            case 't':

                if(!strcmp("dhcp", mt_optarg))
                {
                    pInputParam->net_mode = MT_NET_MODE_DHCP;
                    pInputParam->operate |= MT_NET_OPERATE_SET_AUTO;
                }
                else if(!strcmp("pppoe", mt_optarg))
                {
                    pInputParam->net_mode = MT_NET_MODE_PPPOE;
                    pInputParam->operate |= MT_NET_OPERATE_SET_AUTO;
                }
                else if(!strcmp("static", mt_optarg))
                {
                    pInputParam->net_mode = MT_NET_MODE_STATIC;
                }
                else
                {
                    (MT_VOID)MT_NetPrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                break;
            case 'M':
                pInputParam->operate |= MT_NET_OPERATE_SET_MAC;
                MTADP_Strncpy(pInputParam->ipinfo.mac, mt_optarg, sizeof(pInputParam->ipinfo.mac));
                break;

            case 's':
                pInputParam->net_mode = MT_NET_MODE_STATIC;
                pInputParam->operate |= MT_NET_OPERATE_SET_IP;
                MTADP_Strncpy(pInputParam->ipinfo.ipaddr, mt_optarg, sizeof(pInputParam->ipinfo.ipaddr));
                break;
            case 'm':
                pInputParam->net_mode = MT_NET_MODE_STATIC;
                pInputParam->operate |= MT_NET_OPERATE_SET_MASK;
                MTADP_Strncpy(pInputParam->ipinfo.netmask, mt_optarg, sizeof(pInputParam->ipinfo.netmask));
                break;
            case 'g':
                pInputParam->net_mode = MT_NET_MODE_STATIC;
                pInputParam->operate |= MT_NET_OPERATE_SET_GW;
                MTADP_Strncpy(pInputParam->ipinfo.getway, mt_optarg, sizeof(pInputParam->ipinfo.getway));
                break;
            case 'd':
                pInputParam->net_mode = MT_NET_MODE_STATIC;
                pInputParam->operate |= MT_NET_OPERATE_SET_DNS;
                MTADP_Strncpy(pInputParam->ipinfo.dns1, mt_optarg, sizeof(pInputParam->ipinfo.dns1));
                break;
            case 'i':
                if(!strcmp("eth0", mt_optarg))
                {
                    pInputParam->eth_port = ETH_PORT_ETH0;
                }
                else if(!strcmp("eth1", mt_optarg))
                {
                    pInputParam->eth_port = ETH_PORT_ETH1;
                }
                else
                {
                    (MT_VOID)MT_NetPrint_Help(argv[0]);
                    return MT_FAILURE;
                }
                break;
            default:
                (MT_VOID)MT_NetPrint_Help(argv[0]);
                return MT_FAILURE;
        }
    }

    return MT_SUCCESS;
}

#ifdef MT_SAMPLE_APP
MT_S32 MT_NetMain(MT_S32 argc, MT_CHAR *argv[])
#else
MT_S32 main(MT_S32 argc, MT_CHAR *argv[])
#endif
{
    MT_S32              ret = MT_SUCCESS;
    Eth_LinkStatus_E    getState = 0;
    mt_net_input_para_t sInputParam ={ 0 };
    MT_S32              reTry = 0;

    ret = MT_NetParase_args(argc, argv, &sInputParam);
    if(MT_SUCCESS != ret)
    {
        return MT_FAILURE;
    }

#ifndef MT_SAMPLE_APP
    ret = mt_sys_init();
    if(MT_SUCCESS != ret)
    {
        SAMPLE_NET_ERR_PRINT("failed to mt_sys_init\n");
        return ret;
    }
#endif

    ret = MT_ETH_Open(sInputParam.eth_port);
    if (MT_SUCCESS != ret)
    {
        SAMPLE_NET_ERR_PRINT("MT_ETH_Open failed.ret = %#x\n",ret);
        return ret;
    }

    if(0 == (MT_NET_OPERATE_SET_ALL & sInputParam.operate))
    {
        (MT_VOID)MT_NetPrint_Help(argv[0]);
        return MT_FAILURE;
    }

    ret = MT_NetChackParam(&sInputParam);
    if(MT_SUCCESS != ret)
    {
        return ret;
    }
    do{

        ret = MT_ETH_GetLinkStatus(sInputParam.eth_port, &getState);
        if((MT_SUCCESS != ret) || (ETH_LINK_STATUS_OFF == getState))
        {
            SAMPLE_NET_ERR_PRINT("Check ETH%d link status: DOWN . reTry count: %d \n", sInputParam.eth_port, reTry);
            reTry++;
            sleep(1);
            if(reTry >= MAX_RETRY_COUNT)
            {
                break;
            }
            continue;
        }

        if(ETH_LINK_STATUS_ON == getState)
        {
            break;
        }
    }while(1);


    if(ETH_LINK_STATUS_OFF == getState)
    {
        SAMPLE_NET_ERR_PRINT("The network port is not connected. Please check Network Link...\n");
        return MT_FAILURE;
    }

    if(MT_NET_MODE_DHCP == sInputParam.net_mode)
    {
        (MT_VOID)MT_NetSetDhcp(&sInputParam);
    }
    else if(MT_NET_MODE_STATIC == sInputParam.net_mode)
    {
        (MT_VOID)MT_NetSetStatic(&sInputParam);
    }

#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif

    return MT_SUCCESS;
}
