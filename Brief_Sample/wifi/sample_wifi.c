/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>
#include "pthread.h"
#include "mt_wlan_sta.h"
#include "mt_unf_common.h"
#include "mt_adp_mpi.h"
#include <spawn.h>
#include <sys/wait.h>
#include <fcntl.h>

/***************************** Macro Definition ******************************/
#ifdef  MT_SAMPLE_WIFI_DEBUG
#define MT_WIFI_PRINT   printf
#else
#define MT_WIFI_PRINT
#endif

#define SAMPLE_WIFI_FUNCTION_ENTER()         MT_WIFI_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_WIFI_FUNCTION_EXIT()          MT_WIFI_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)

#define SAMPLE_WIFI_FATAL_PRINT(fmt...)      MT_WIFI_PRINT(" [FATAL] " fmt)
#define SAMPLE_WIFI_ERR_PRINT(fmt...)        MT_WIFI_PRINT(" [ERROR] " fmt)
#define SAMPLE_WIFI_WARN_PRINT(fmt...)       MT_WIFI_PRINT(" [WARN] "  fmt)
#define SAMPLE_WIFI_INFO_PRINT(fmt...)       MT_WIFI_PRINT(" [INFO] "  fmt)
#define SAMPLE_WIFI_DBG_PRINT(fmt...)        MT_WIFI_PRINT(" [DEBUG] " fmt)

#define SAMPLE_WIFI_PRINT   printf

#define MAX_IP_ADDR_STR             16
#define MAX_MAC_ADDR_STR            18
#define MAX_IFNAME_STR              20
#define MAX_WIFI_INTERFACE_LEN      12
#define MT_WIFI_CONNECT_SUCCESS      2
#define MT_WIFI_ENCRYPT_TYPE_MAX     4
#define MT_TASK_RUN                  1
#define MT_TASK_EXIT                 2

#ifndef CONFIG_MT_SANITIZE_NONE
#define MT_TIMES_NULL                60
#else
#define MT_TIMES_NULL                10
#endif

/*************************** Structure Definition ****************************/
typedef enum
{
    MT_WIFI_STATUS_INIT = 0,
    MT_WIFI_STATUS_SCANNING,
    MT_WIFI_STATUS_SCANNED,
    MT_WIFI_STATUS_CONNECTING,
    MT_WIFI_STATUS_CONNECTED,
    MT_WIFI_STATUS_DISCONNECTED,
    MT_WIFI_STATUS_STARTING,
    MT_WIFI_STATUS_STARTEND,
    MT_WIFI_STATUS_BUTT
}MT_WIFI_CONNECT_STATUS;

typedef struct
{
    MT_CHAR mac[MAX_MAC_ADDR_STR];
    MT_CHAR ipaddr[MAX_IP_ADDR_STR];
    MT_CHAR netmask[MAX_IP_ADDR_STR];
    MT_CHAR getway[MAX_IP_ADDR_STR];
    MT_CHAR dns1[MAX_IP_ADDR_STR];
    MT_CHAR dns2[MAX_IP_ADDR_STR];
}mt_wifi_network_t;

typedef struct
{
    MT_WIFI_CONNECT_STATUS stause;
    MT_S32     apNum;
    ap_info_t *apList;
    ap_info_t connectApInfo;
    mt_wifi_network_t  netInfo;
    MT_CHAR wlanName[MAX_IFNAME_STR];
}mt_wifi_sta_info_t;

/********************** Global Variable declaration **************************/
static MT_CHAR            key[32] = { 0 };
static mt_wifi_sta_info_t g_wifiInfo;
static MT_BOOL            g_bTaskQuit = MT_TRUE;
static ap_info_t          *pConnectAp = NULL;
static MT_BOOL            g_isConnect = MT_FALSE;
const MT_CHAR encrypType[4][12] =
{
    "NONE",
    "WEP",
    "WPA1",
    "WPA2"
};
/******************************* API declaration *****************************/
#ifdef MT_SAMPLE_APP
MT_S32 MT_WiFiMain(MT_S32 argc, MT_CHAR *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

static MT_CHAR* get_word(MT_CHAR* line, MT_CHAR* value)
{
    MT_CHAR * p = line;

    while((*p == ' ') || (*p == '\t'))
    {
        p++;
    }

    while ((*p != '\t') && (*p != ' ') && (*p != '\n') && (*p != 0))
    {
        *value++ = *p++;
    }

    *value = 0;

    return p;
}


static MT_S32 MT_WiFiGetIPAddress(MT_CHAR *name, MT_CHAR *ipAdd)
{
    MT_S32 sockfd = 0;
    struct ifreq ifr = { 0 };
    struct sockaddr_in *s_in = { 0 };

    if(NULL == ipAdd)
    {
        SAMPLE_WIFI_ERR_PRINT("null pointer\n");
        return MT_FAILURE;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(sockfd < 0)
    {
        SAMPLE_WIFI_ERR_PRINT("socket create fail!\n");
        return MT_FAILURE;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, name);
    if(ioctl(sockfd, SIOCGIFADDR, &ifr) < 0)
    {
        SAMPLE_WIFI_ERR_PRINT("ioctl fail!\n");
        close(sockfd);
        return MT_FAILURE;
    }

    s_in = (struct sockaddr_in *)(&ifr.ifr_addr);
    memcpy((void *)ipAdd, inet_ntoa(s_in->sin_addr), 15);

    close(sockfd);
    return MT_SUCCESS;
}


static MT_S32 MT_WiFiGetSubNetmask(MT_CHAR *name, MT_CHAR* subNetmask)
{
    MT_S32 skfd = 0;
    struct ifreq ifr = { 0 };
    struct sockaddr_in *s_in = { 0 };

    if(MT_NULL == subNetmask)
    {
        SAMPLE_WIFI_ERR_PRINT("null pointer\n");
        return MT_FAILURE;
    }

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        SAMPLE_WIFI_ERR_PRINT("socket create fail!\n");
        return MT_FAILURE;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, name);
    if(ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0)
    {
        SAMPLE_WIFI_ERR_PRINT("ioctl fail!\n");
        close(skfd);
        return MT_FAILURE;
    }

    s_in = (struct sockaddr_in *)(&ifr.ifr_netmask);

    memcpy((void *)subNetmask, inet_ntoa(s_in->sin_addr), 15);

    close(skfd);
    return MT_SUCCESS;
}


static MT_S32 MT_WiFiGetGateway(MT_CHAR *name, MT_CHAR* gateway)
{
    MT_CHAR line[512] = { 0 };
    MT_CHAR str[16] = { 0 };
    FILE    *fp = MT_NULL;

    if(MT_NULL == gateway)
    {
        SAMPLE_WIFI_ERR_PRINT("null pointer\n");
        return MT_FAILURE;
    }

    fp = fopen("/proc/net/route", "r");
    if(fp)
    {
        memset(str, 0, sizeof(str));
        while(fgets(line, sizeof(line), fp) != 0)
        {
            MT_CHAR* p = line;
            p = strstr(p, name);
            if(p)
            {
                p = get_word(p, str);
                p = get_word(p, str);
                if(strcmp(str, "00000000") == 0)
                {
                    p = get_word(p, str);
                    if(strcmp(str, "00000000") != 0)
                    {
                        int a, b, c, d;
                        sscanf(str, "%02X%02X%02X%02X", &a, &b, &c, &d);
                        sprintf(gateway, "%d.%d.%d.%d", d, c, b, a);
                        fclose(fp);
                        return MT_SUCCESS;
                    }
                }
            }
        }

        fclose(fp);
    }

    strcpy(gateway, "0.0.0.0");
    return MT_FAILURE;
}


static MT_S32 MT_WiFiGetDNS(MT_CHAR *dns)
{
    MT_CHAR line[100];
    MT_CHAR str[16];
    FILE* fp;

    if(MT_NULL == dns)
    {
        SAMPLE_WIFI_ERR_PRINT("null pointer\n");
        return MT_FAILURE;
    }

    memset(str, 0, 16);
    fp = fopen("/etc/resolv.conf", "r");
    if(fp)
    {
        while(fgets(line, 100, fp) != 0)
        {
            MT_CHAR* p;
            p = strstr(line, "nameserver");
            if (p)
            {
                while((*p != '\t') && (*p != ' ') && (*p != '\n') && (*p != 0))
                {
                    p++;
                }
                get_word(p, str);
                break;
            }
        }

        fclose(fp);
    }

    if (strlen(str) == 0)
    {
        strcpy(dns, "0.0.0.0");
        return MT_FAILURE;
    }
    else
    {
        strcpy(dns, str);
        return MT_SUCCESS;
    }
}


static MT_S32 MT_WiFiGetMac(MT_CHAR *name, MT_CHAR *mac)
{
    MT_S32  skfd;
    MT_CHAR buf[18];
    struct  ifreq ifr;

    if(MT_NULL == mac)

    {
        SAMPLE_WIFI_ERR_PRINT("null pointer\n");
        return MT_FAILURE;
    }

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        SAMPLE_WIFI_ERR_PRINT("socket create error!\n");
        return MT_FAILURE;
    }

    memset(&ifr, 0, sizeof(ifr));
    strcpy(ifr.ifr_name, name);
    if(ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        SAMPLE_WIFI_ERR_PRINT("ioctl fail!\n");
        close(skfd);
        return MT_FAILURE;
    }

    memset(buf, 0, 18);
    memcpy(buf, ifr.ifr_hwaddr.sa_data, 8);
    snprintf(buf, sizeof(buf), "%02X:%02X:%02X:%02X:%02X:%02X",
             (buf[0] & 0377), (buf[1] & 0377), (buf[2] & 0377),
             (buf[3] & 0377), (buf[4] & 0377), (buf[5] & 0377)
            );
    memcpy(mac, buf, strlen(buf));

    close(skfd);
    return MT_SUCCESS;
}


static MT_S32 MT_WiFiNetGetInfo(mt_wifi_network_t *netInfo, MT_CHAR *wlanName)
{
    MT_S32 ret = MT_FAILURE;

    ret = MT_WiFiGetIPAddress(wlanName, netInfo->ipaddr);
    ret |= MT_WiFiGetSubNetmask(wlanName, netInfo->netmask);
    ret |= MT_WiFiGetGateway(wlanName, netInfo->getway);
    ret |= MT_WiFiGetDNS(netInfo->dns1);
    ret |= MT_WiFiGetMac(wlanName, netInfo->mac);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_WIFI_ERR_PRINT("Failed to obtain net info\n");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}

static MT_VOID MT_WiFiNetPrintInfo(mt_wifi_network_t *pNetInfo)
{
    SAMPLE_WIFI_PRINT("Network configuration:\n");
    SAMPLE_WIFI_PRINT("Mac:        %s \n", pNetInfo->mac);
    SAMPLE_WIFI_PRINT("IPaddr:     %s \n", pNetInfo->ipaddr);
    SAMPLE_WIFI_PRINT("Subnetmask: %s \n", pNetInfo->netmask);
    SAMPLE_WIFI_PRINT("Getway:     %s \n", pNetInfo->getway);
    SAMPLE_WIFI_PRINT("DNS1:       %s \n", pNetInfo->dns1);
}


static MT_S32 MT_WifiConvertSignal(mt_s32 quality)
{
    mt_s32 signal_level = 0;

    if(quality >= -40)
    {
        signal_level = 99;
    }
    else if(quality < -40 && quality >= -55)
    {
        signal_level = 90;
    }
    else if(quality < -55 && quality >= -66)
    {
        signal_level = 70;
    }
    else if(quality < -66 && quality >= -77)
    {
        signal_level = 50;
    }
    else if(quality < -77 && quality >= -88)
    {
        signal_level = 30;
    }
    else if(quality < -88 && quality >= -100)
    {
        signal_level = 10;
    }
    else
    {
        signal_level = 0;
    }

    return signal_level;
}

static MT_S32 MT_WifiAddListAPs(MT_VOID)
{
    MT_S32    i = 0;
    MT_S32    aps_count = 0;

    g_wifiInfo.apList = MT_NULL;

    aps_count = MT_WLAN_STA_GetScanResult(MT_NULL, &g_wifiInfo.apList);
    if(aps_count <= 0)
    {
        SAMPLE_WIFI_ERR_PRINT("Scan failed.\n");
        return MT_FAILURE;
    }

    g_wifiInfo.apNum = aps_count;

    for(i = 0; i < aps_count; ++i)
    {
        g_wifiInfo.apList[i].signal_level = MT_WifiConvertSignal(g_wifiInfo.apList[i].signal_level);

    }


    return MT_SUCCESS;
}


static MT_VOID MT_WiFiWaitingScanning(MT_U32 timeout)
{
    while(1)
    {
        if(g_wifiInfo.stause == MT_WIFI_STATUS_SCANNED)
        {
            break;
        }
        sleep(1);

        if(timeout == 0)
        {
            SAMPLE_WIFI_ERR_PRINT("STA Scan failed!\n");
            break;
        }
        timeout--;
    }

    SAMPLE_WIFI_INFO_PRINT("STA Scan done!\n");
}

static MT_S32 MT_WiFiReScanAP(MT_VOID)
{

    MT_S32  ret = MT_SUCCESS;
    MT_S32  connect = 0;

    ret = MT_WLAN_STA_GetConnectionStatus(MT_NULL, MT_NULL, &connect);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_WIFI_ERR_PRINT("Wifi did not start\n");
        return MT_FAILURE;
    }

    free(g_wifiInfo.apList);
    g_wifiInfo.apNum = 0;

    g_wifiInfo.stause = MT_WIFI_STATUS_SCANNING;
    ret = MT_WLAN_STA_Scan(MT_NULL);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_WIFI_ERR_PRINT("failed to MT_WLAN_STA_Scan\n");
        return MT_FAILURE;
    }

    (MT_VOID)MT_WiFiWaitingScanning(MT_TIMES_NULL);

    return MT_SUCCESS;
}

static MT_VOID MT_WiFiStopConnectWifi(MT_VOID)
{
    SAMPLE_WIFI_FUNCTION_ENTER();

    (MT_VOID)MT_WLAN_STA_Disconnect(MT_NULL);

    SAMPLE_WIFI_FUNCTION_EXIT();
}

static MT_S32 MT_WiFiWaitingConnecting(MT_U32 timeout)
{
    while(1)
    {
        if(g_wifiInfo.stause == MT_WIFI_STATUS_CONNECTED)
        {
            SAMPLE_WIFI_INFO_PRINT("Connection success\n");
            break;
        }

        if(timeout == 0)
        {
            SAMPLE_WIFI_ERR_PRINT("Connection failed\n");
            (MT_VOID)MT_WiFiStopConnectWifi();
            return MT_FAILURE;
        }
        timeout--;
        SAMPLE_WIFI_INFO_PRINT("Wifi Connecting...\n");
        sleep(1);
    }

    system("ifconfig eth0 0.0.0.0");
    return MT_SUCCESS;
}



static void MT_WiFiPrintApList(ap_info_t *ap_list, mt_u32 ap_num)
{

    SAMPLE_WIFI_PRINT("<Index>\t  <SSID>\t\t    <Strength/dBm>   <EncType>\t      <MAC>\n");
    for(MT_S32 i = 0; i < ap_num; ++i)
    {
        if(0 == strcmp(ap_list[i].ssid, ""))
        {
            continue;
        }
        SAMPLE_WIFI_PRINT("  [%d]\t%-32s  %d%%\t\t%s\t%s\n", i,
                                                  ap_list[i].ssid,
                                                  ap_list[i].signal_level,
                                                  encrypType[ap_list[i].encryp_type - 1],
                                                  ap_list[i].ap_mac);
    }

}

static MT_S32 MT_WiFiConnectNetwork(MT_S8 *macaddr)
{
    MT_S32  ret;
    MT_S32  time = 0;
    MT_CHAR ipaddr[32];
    MT_CHAR cmdbuf[128];

    for(MT_S32 i = 0; i < g_wifiInfo.apNum; ++i)
    {
        if(0 == strcmp((MT_CHAR*)g_wifiInfo.apList[i].ap_mac, (MT_CHAR*)macaddr))
        {
            memcpy(&g_wifiInfo.connectApInfo, &g_wifiInfo.apList[i], sizeof(ap_info_t));
            break;
        }
    }

    sprintf(cmdbuf, "udhcpc -t 4 -n -i %s 2>&1 >/dev/null", g_wifiInfo.wlanName);
    system(cmdbuf);

    while(1)
    {
        ret = MT_WiFiGetIPAddress(g_wifiInfo.wlanName, ipaddr);
        if(MT_SUCCESS != ret)
        {
            SAMPLE_WIFI_INFO_PRINT("Re-DHCP\n");
        }
        else
        {
            SAMPLE_WIFI_INFO_PRINT("DHCP success\n");
            ret = MT_WiFiNetGetInfo(&g_wifiInfo.netInfo, g_wifiInfo.wlanName);
            if(MT_SUCCESS != ret)
            {
                return ret;
            }
            return MT_SUCCESS;
        }
        time++;
        if(4 == time)
        {
            SAMPLE_WIFI_ERR_PRINT("DHCP error\n");
            break;
        }
        sleep(1);
        SAMPLE_WIFI_INFO_PRINT("Retry. times[%d]\n", time);
    }

    return MT_FAILURE;
}


static MT_VOID MT_WiFiDisconnectNetwork(MT_VOID)
{
    char cmdbuf[128];

    memset(&g_wifiInfo.connectApInfo, 0, sizeof(g_wifiInfo.connectApInfo));

    sprintf(cmdbuf, "kill `ps | awk '{if (!/awk/ && /udhcpc/) print $1}'` 2>&1 >/dev/null");
    system(cmdbuf);

    sprintf(cmdbuf, "ip addr flush dev %s 2>&1 >/dev/null", g_wifiInfo.wlanName);
    system(cmdbuf);

    memset(&g_wifiInfo.netInfo, 0 ,sizeof(g_wifiInfo.netInfo));

}

static MT_S32 MT_WiFiCheckNetwork(mt_wifi_network_t *pNetInfo)
{
    char cmdbuf[128];

    (MT_VOID)MT_WiFiNetPrintInfo(pNetInfo);

    SAMPLE_WIFI_INFO_PRINT("Check network connecting.....\n");

    sprintf(cmdbuf, "ping %s -c 4", pNetInfo->getway);
    if(system(cmdbuf) < 0)
    {
        SAMPLE_WIFI_ERR_PRINT("error occured\n");
    }

    return MT_SUCCESS;
}

static MT_VOID MT_WiFiBoot(MT_VOID)
{
    MT_S32 s32Ret = MT_FAILURE;
    MT_S32 connect = 0;
    s32Ret = MT_WLAN_STA_GetConnectionStatus(MT_NULL, MT_NULL, &connect);
    if(MT_SUCCESS == s32Ret)
    {
        SAMPLE_WIFI_ERR_PRINT("Wifi has been turned on\n");
        return;
    }

    s32Ret = MT_WLAN_STA_Open(g_wifiInfo.wlanName);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_WIFI_ERR_PRINT("failed to MT_WLAN_STA_Open, ret = %d\n", s32Ret);
        return;
    }

    s32Ret = MT_WLAN_STA_Start(MT_NULL);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_WIFI_ERR_PRINT("failed to MT_WLAN_STA_Start\n");
        (MT_VOID)MT_WLAN_STA_Close(MT_NULL);
        return;
    }

    if(MT_TRUE == g_isConnect)
    {
        (MT_VOID)MT_WLAN_STA_Disconnect(MT_NULL);
        g_wifiInfo.stause = MT_WIFI_STATUS_CONNECTING;
        s32Ret = MT_WLAN_STA_Connect(MT_NULL, pConnectAp);

        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_WIFI_ERR_PRINT("failed to MT_WLAN_STA_Connect\n");
            return;
        }

        s32Ret = MT_WiFiWaitingConnecting(MT_TIMES_NULL);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_WIFI_ERR_PRINT("Connection timed out\n");
            return;
        }
    }
}

static void MT_WiFiCallback(MT_S32 event, void* arg)
{
    switch(event)
    {
        case MT_WLAN_STA_EVENT_DEV_INSERT:
            SAMPLE_WIFI_INFO_PRINT("Dev Insert <%lu>\n", (ulong)arg);
            g_wifiInfo.stause = MT_WIFI_STATUS_STARTING;
            (MT_VOID)MT_WiFiBoot();
            g_wifiInfo.stause = MT_WIFI_STATUS_STARTEND;
            break;
        case MT_WLAN_STA_EVENT_DEV_REMOVE:
            SAMPLE_WIFI_INFO_PRINT("Device Removed <%lu>\n", (ulong)arg);
            (MT_VOID)MT_WLAN_STA_Stop(MT_NULL);
            (MT_VOID)MT_WLAN_STA_Close(MT_NULL);
            break;
        case MT_WLAN_STA_EVENT_DISCONNECTED:
            SAMPLE_WIFI_INFO_PRINT("Link Down!\n");
            g_wifiInfo.stause = MT_WIFI_STATUS_DISCONNECTED;
            MT_WiFiDisconnectNetwork();
            break;
        case MT_WLAN_STA_EVENT_SCAN_RESULT_AVAILABLE:
            SAMPLE_WIFI_INFO_PRINT("Scan done!\n");
            MT_WifiAddListAPs();
            g_wifiInfo.stause = MT_WIFI_STATUS_SCANNED;
            break;
        case MT_WLAN_STA_EVENT_CONNECTED:
            SAMPLE_WIFI_INFO_PRINT("Link Up<apmac = %s>!\n", (MT_S8*)arg);
            g_wifiInfo.stause = MT_WIFI_STATUS_CONNECTED;
            MT_WiFiConnectNetwork((MT_S8*)arg);
            break;
    }

}

static MT_S32 MT_WiFiStart(MT_CHAR *name)
{
    MT_S32 s32Ret = MT_FAILURE;


    s32Ret = MT_WLAN_STA_Init("/usr/local/stb/wifi/wdrv.cfg", MT_WiFiCallback);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_WIFI_ERR_PRINT("failed to MT_WLAN_STA_Init, ret = %d\n", s32Ret);
        return s32Ret;
    }

    sleep(2);

    s32Ret = MT_WLAN_STA_Open(name);
    if(MT_SUCCESS != s32Ret)
    {
        SAMPLE_WIFI_ERR_PRINT("failed to MT_WLAN_STA_Open, ret = %d\n", s32Ret);
        (MT_VOID)MT_WLAN_STA_DeInit();
        return s32Ret;
    }

    return MT_SUCCESS;
}


static MT_VOID MT_WiFiStop(MT_VOID)
{
    SAMPLE_WIFI_FUNCTION_ENTER();

    g_wifiInfo.stause = MT_WIFI_STATUS_DISCONNECTED;

    (MT_VOID)MT_WLAN_STA_Disconnect(MT_NULL);
    sleep(1);
    (MT_VOID)MT_WLAN_STA_Stop(MT_NULL);
    (MT_VOID)MT_WLAN_STA_Close(MT_NULL);
    (MT_VOID)MT_WLAN_STA_DeInit();

    free(g_wifiInfo.apList);
    memset(&g_wifiInfo, 0, sizeof(mt_wifi_sta_info_t));
    g_isConnect = MT_FALSE;

    SAMPLE_WIFI_FUNCTION_EXIT();
}





static MT_VOID MT_WiFiGetConnectionStatus(MT_VOID)
{
    MT_S32 connect = 0;
    MT_S32 ret = MT_FAILURE;

    ret = MT_WLAN_STA_GetConnectionStatus(MT_NULL, MT_NULL, &connect);
    if(MT_SUCCESS != ret)
    {
        SAMPLE_WIFI_ERR_PRINT("failed to MTCommand_Wifi_Sta_Connect\n");
        return;
    }

    if(MT_WIFI_CONNECT_SUCCESS != connect)
    {
        SAMPLE_WIFI_PRINT("wifi is disconnect.\n");
        return;
    }


    SAMPLE_WIFI_PRINT("<SSID>\t\t             <Strength>\t<EncType> <Status>\n");
    SAMPLE_WIFI_PRINT("%-32s%d%%\t   %s   %s\n",
                                              g_wifiInfo.connectApInfo.ssid,
                                              g_wifiInfo.connectApInfo.signal_level,
                                              encrypType[g_wifiInfo.connectApInfo.encryp_type - 1],
                                              "connected");

    return;
}


static MT_VOID MT_WiFiExit(MT_VOID)
{
    g_bTaskQuit = MT_TRUE;

    (MT_VOID)MT_WiFiStop();
}


static MT_VOID MT_WiFiPrintMenu(MT_VOID)
{
    SAMPLE_WIFI_PRINT("\ncommond: \n");
    SAMPLE_WIFI_PRINT("     s: scan nearby networks \n");
    SAMPLE_WIFI_PRINT("     c: select WiFi connection \n");
    SAMPLE_WIFI_PRINT("     m: manual add a wifi connection \n");
    SAMPLE_WIFI_PRINT("     v: review the network configuration \n");
    SAMPLE_WIFI_PRINT("     p: check network connection \n");
    SAMPLE_WIFI_PRINT("     d: disconnect \n");
    SAMPLE_WIFI_PRINT("     t: check the wifi status \n");
    SAMPLE_WIFI_PRINT("     k: start wifi \n");
#ifdef MT_SAMPLE_APP
    SAMPLE_WIFI_PRINT("     b :background run \n");
#endif
    SAMPLE_WIFI_PRINT("     q: quit \n");
    SAMPLE_WIFI_PRINT("     h: help \n");
    SAMPLE_WIFI_PRINT("WIFI>> ");
}

static MT_VOID MT_WiFiCmdTask(MT_VOID)
{
    MT_S32               s32Ret = MT_FAILURE;
    MT_S32               num = 0;
    MT_CHAR              inputCmd[32] = { 0 };
    MT_CHAR              ssid[33] = { 0 };

    if(g_bTaskQuit == MT_FALSE)
    {
        (MT_VOID)MT_WiFiWaitingScanning(MT_TIMES_NULL);
    }

    while(1)
    {
        (MT_VOID)MT_WiFiPrintMenu();

        fgets((MT_CHAR *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        if('s' == inputCmd[0])
        {
            SAMPLE_WIFI_PRINT("Scanning.....\n");
            s32Ret = MT_WiFiReScanAP();
            if(MT_SUCCESS == s32Ret)
            {
                (MT_VOID)MT_WiFiPrintApList(g_wifiInfo.apList, g_wifiInfo.apNum);
            }
        }
        else if('c' == inputCmd[0])
        {
            SAMPLE_WIFI_PRINT("Please select the following wifi to connect:\n");

            (MT_VOID)MT_WiFiPrintApList(g_wifiInfo.apList, g_wifiInfo.apNum);
            SAMPLE_WIFI_PRINT("Please enter the serial number: \n>> ");

            scanf("%d", &num);
            getchar();

            if(num > g_wifiInfo.apNum - 1)
            {
                SAMPLE_WIFI_PRINT("Out of range\n");
                continue;
            }
            pConnectAp = g_wifiInfo.apList + num;

            if(IW_ENC_NONE != pConnectAp->encryp_type)
            {
                SAMPLE_WIFI_PRINT("Please enter Key\n>> ");
                fgets((MT_CHAR *)(key), (sizeof(key) - 1), stdin);
                key[strlen(key) - 1] = '\0'; //eat enter key

                pConnectAp->key = (unsigned char*)key;
                SAMPLE_WIFI_DBG_PRINT("Connect Key[%lu]: [%c%c%c%c....]\n", strlen(key),
                                                                       pConnectAp->key[0],
                                                                       pConnectAp->key[1],
                                                                       pConnectAp->key[2],
                                                                       pConnectAp->key[3]);
            }
            //send disconnect message
            (MT_VOID)MT_WLAN_STA_Disconnect(MT_NULL);
            g_wifiInfo.stause = MT_WIFI_STATUS_CONNECTING;
            s32Ret = MT_WLAN_STA_Connect(MT_NULL, pConnectAp);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_WIFI_ERR_PRINT("failed to MT_WLAN_STA_Connect\n");
                continue;
            }

            s32Ret = MT_WiFiWaitingConnecting(MT_TIMES_NULL);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_WIFI_ERR_PRINT("Connection timed out\n");
                continue;
            }

            g_isConnect = MT_TRUE;
        }
        else if('m' == inputCmd[0])
        {
            pConnectAp = (ap_info_t *)malloc(sizeof(ap_info_t));
            SAMPLE_WIFI_PRINT("Please enter SSID\n>> ");
            fgets((MT_CHAR *)(ssid), (sizeof(ssid) - 1), stdin);
            strncpy(pConnectAp->ssid, ssid, strlen(ssid) - 1);
            SAMPLE_WIFI_INFO_PRINT("[%s]\n", pConnectAp->ssid);

            SAMPLE_WIFI_PRINT("Please enter the security type(NONE-1, WEP-2, WPA1-3, WPA2-4)\n>> ");
            scanf("%d", (mt_u32 *)&pConnectAp->encryp_type);
            getchar();

            if( pConnectAp->encryp_type > MT_WIFI_ENCRYPT_TYPE_MAX)
            {
                SAMPLE_WIFI_ERR_PRINT("Input error. must input less [%d] \n", MT_WIFI_ENCRYPT_TYPE_MAX);
                free(pConnectAp);
                continue;
            }

            if(IW_ENC_NONE != pConnectAp->encryp_type)
            {
                SAMPLE_WIFI_PRINT("Please enter Key\n>> ");
                fgets((MT_CHAR *)(key), (sizeof(key) - 1), stdin);

                key[strlen(key) - 1] = '\0'; //eat enter key

                pConnectAp->key = (unsigned char*)key;
            }

            pConnectAp->hidden = 1;

            //send disconnect message
            (MT_VOID)MT_WLAN_STA_Disconnect(MT_NULL);

            g_wifiInfo.stause = MT_WIFI_STATUS_CONNECTING;
            s32Ret = MT_WLAN_STA_Connect(MT_NULL, pConnectAp);
            if(MT_SUCCESS != s32Ret)
            {
                SAMPLE_WIFI_ERR_PRINT("failed to MT_WLAN_STA_Connect\n");
                free(pConnectAp);
                continue;
            }

            (MT_VOID)MT_WiFiWaitingConnecting(MT_TIMES_NULL);

            g_isConnect = MT_TRUE;
        }
        else if('v' == inputCmd[0])
        {
            (MT_VOID)MT_WiFiNetPrintInfo(&g_wifiInfo.netInfo);
        }
        else if('p' == inputCmd[0])
        {
            (MT_VOID)MT_WiFiCheckNetwork(&g_wifiInfo.netInfo);
        }
        else if('d' == inputCmd[0])
        {
            (MT_VOID)MT_WLAN_STA_Disconnect(MT_NULL);
        }
        else if('t' == inputCmd[0])
        {
            (MT_VOID)MT_WiFiGetConnectionStatus();
        }
        else if('k' == inputCmd[0])
        {
            (MT_VOID)MT_WiFiBoot();
        }
    #ifdef MT_SAMPLE_APP
        else if ('b' == inputCmd[0])
        {
            SAMPLE_WIFI_INFO_PRINT("WiFi play in back!\n");
            break;
        }
    #endif
        else if('q' == inputCmd[0])
        {
            SAMPLE_WIFI_INFO_PRINT("<Exit>!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_WIFI_INFO_PRINT("Print help info \n");
        }
    }
}

static MT_VOID MT_WiFiPrint_Help(MT_CHAR *name)
{
    SAMPLE_WIFI_PRINT("Lack of parameters\n");
    SAMPLE_WIFI_PRINT("\nUsage:\n");
    SAMPLE_WIFI_PRINT("%s\n", name);
#ifdef MT_SAMPLE_APP
    SAMPLE_WIFI_PRINT("    -q: Exit the background\n");
#endif
}


static MT_S32 MT_WiFiParase_args(MT_S32 argc, MT_CHAR *argv[])
{
    MT_S32 opt = 0;

    while((opt = MTADP_Getopt(argc, argv, ":?hH:q")) != -1)
    {
        switch(opt)
        {
            case 'h':
            case '?':
            case 'H':
                (MT_VOID)MT_WiFiPrint_Help(argv[0]);
                return MT_FAILURE;
            case 'q':
                if(g_bTaskQuit == MT_FALSE)
                {
                    (MT_VOID)MT_WiFiExit();
                }
                return MT_TASK_EXIT;
            default:
                return MT_SUCCESS;
        }
    }

    return MT_SUCCESS;
}


#ifdef MT_SAMPLE_APP
MT_S32 MT_WiFiMain(MT_S32 argc, MT_CHAR *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif
{
    MT_S32     s32Ret = MT_SUCCESS;
    s32Ret = MT_WiFiParase_args(argc, argv);
    if (MT_FAILURE == s32Ret)
    {
        SAMPLE_WIFI_ERR_PRINT("Parase args err. stop window.\n");

        return MT_FAILURE;
    }
    else if(MT_TASK_EXIT == s32Ret)
    {
        SAMPLE_WIFI_ERR_PRINT("Recv stop command. stop window.\n");

        return MT_SUCCESS;
    }

    if(g_bTaskQuit == MT_TRUE)
    {
    #ifndef MT_SAMPLE_APP
        /** System initialization */
        s32Ret = mt_sys_init();
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_WIFI_ERR_PRINT("failed to mt_sys_init\n");
            return s32Ret;
        }
    #endif
        g_bTaskQuit = MT_FALSE;
        memset(&g_wifiInfo, 0, sizeof(mt_wifi_sta_info_t));

        s32Ret = MT_WiFiStart(g_wifiInfo.wlanName);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_WIFI_ERR_PRINT("failed to MT_WiFiStart\n");
            goto ERR0;
        }

        while(g_wifiInfo.stause == MT_WIFI_STATUS_STARTING)
        {
            usleep(500);
            continue;
        }

        g_wifiInfo.stause = MT_WIFI_STATUS_SCANNING;
        s32Ret = MT_WLAN_STA_Scan(MT_NULL);
        if(MT_SUCCESS != s32Ret)
        {
            SAMPLE_WIFI_ERR_PRINT("failed to MT_WLAN_STA_Scan\n");
            goto ERR1;
        }
    }
    (MT_VOID)MT_WiFiCmdTask();

    if(g_bTaskQuit != MT_TRUE)
    {
        return MT_TASK_RUN;
    }

ERR1:
    (MT_VOID)MT_WiFiStop();
ERR0:
#ifndef MT_SAMPLE_APP
    (MT_VOID)mt_sys_deinit();
#endif
    g_bTaskQuit = MT_TRUE;

    return MT_SUCCESS;
}
