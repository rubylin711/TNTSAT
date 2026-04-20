#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/in.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <errno.h>
#include "mt_type.h"
#include <pthread.h>


#ifdef MT_SAMPLE_NET_DUMP_DEBUG

#define MT_NET_DUMP_PRINT   printf
#else

#define MT_NET_DUMP_PRINT

#endif

#define SAMPLE_NET_DUMP_FUNCTION_ENTER()    MT_NET_DUMP_PRINT("[%s]: Enter ==>> \n", __FUNCTION__)
#define SAMPLE_NET_DUMP_FUNCTION_EXIT()     MT_NET_DUMP_PRINT("[%s]: Exit ==<< \n", __FUNCTION__)



#define SAMPLE_NET_DUMP_FATAL_PRINT(fmt...)         MT_NET_DUMP_PRINT(" [FATAL] " fmt)
#define SAMPLE_NET_DUMP_ERR_PRINT(fmt...)           MT_NET_DUMP_PRINT(" [ERROR] " fmt)
#define SAMPLE_NET_DUMP_WARN_PRINT(fmt...)          MT_NET_DUMP_PRINT(" [WARN] "  fmt)
#define SAMPLE_NET_DUMP_INFO_PRINT(fmt...)          MT_NET_DUMP_PRINT(" [INFO] "  fmt)
#define SAMPLE_NET_DUMP_DBG_PRINT(fmt...)           MT_NET_DUMP_PRINT(" [DEBUG] " fmt)

#define SAMPLE_NET_DUMP_PRINT printf

#define RCV_BUF_SIZE 1024 * 5

static int g_iRecvBufSize = RCV_BUF_SIZE;
static char g_acRecvBuf[RCV_BUF_SIZE] = {0};

static const char *g_szIfName = "eth0";
/* Protocol type of Ethernet frame encapsulation*/
static const int g_iEthProId[] = { ETHERTYPE_IP, ETHERTYPE_ARP, ETHERTYPE_IPV6};
static const char g_szProName[][24] = { "none", "ip", "arp", "ipv6" };
static MT_BOOL g_bTaskQuit = MT_TRUE;

#ifdef MT_SAMPLE_APP
mt_s32 MT_NetDumpMain(mt_s32 argc, mt_char *argv[]);
#else
mt_s32 main(mt_s32 argc, mt_char *argv[]);
#endif

/*
@brief outof MAC address
@param[in] iType, smac and dmac types
@param[in] acHWAddr, Hardware address
@return void
*/
static void MT_NetDumpshowMac(const int iType, const unsigned char* acHWAddr)
{
    int i = 0;

    if(0 == iType)
    {
        MT_NET_DUMP_PRINT("SMAC=[");
    }
    else
    {
        MT_NET_DUMP_PRINT("DMAC=[");
    }

    for(i = 0; i < ETHER_ADDR_LEN - 1; i++)
    {
        MT_NET_DUMP_PRINT("%02x:", *((unsigned char *)&(acHWAddr[i])));
    }

    MT_NET_DUMP_PRINT("%02x] ", *((unsigned char *)&(acHWAddr[i])));
}


/*
@ brief physical NIC hybrid mode
@param[in] pcIfName, Network port name
@param[in] fd, socket
@param[in] iFlags, Hybrid mode flag
@return MT_SUCCESS
@return MT_FAILURE
*/
static int MT_NetDumpsetPromisc(const char *pcIfName, int fd, int iFlags)
{
    int Ret = -1;
    struct ifreq stIfr = { 0 };

    /* Get the interface attribute flag bit */
    strcpy(stIfr.ifr_name, pcIfName);
    Ret = ioctl(fd, SIOCGIFFLAGS, &stIfr);
    if(MT_SUCCESS > Ret)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("[Error]Get Interface Flags");
        return MT_FAILURE;
    }

    if(0 == iFlags)
    {
        /*  Unpromiscuous mode*/
        stIfr.ifr_flags &= ~IFF_PROMISC;
    }
    else
    {
        /* set to promiscuous mode */
        stIfr.ifr_flags |= IFF_PROMISC;
    }

    Ret = ioctl(fd, SIOCSIFFLAGS, &stIfr);
    if(MT_SUCCESS > Ret)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("[Error]Set Interface Flags");
        return MT_FAILURE;
    }

    return MT_SUCCESS;
}


/*
@brief Get the protocol type encapsulated by the L2 frame
@param[in] iProNum, Obtained protocol type
@return (char *)(g_szProName[iIndex + 1])
*/
static char *MT_NetDumpgetProName(const int iProNum)
{
    int iIndex = 0;

    for(iIndex = 0; iIndex < sizeof(g_iEthProId) / sizeof(g_iEthProId[0]); iIndex++)
    {
        if(iProNum == g_iEthProId[iIndex])
        {
            break;
        }
    }
    return (char *)(g_szProName[iIndex + 1]);
}




/*
@brief Parse the Ethernet frame header
@param[in] pstEthHead, The header of the obtained packet
@return MT_SUCCESS
@return MT_FAILURE
*/
static int MT_NetDumpparseEthHead(const struct ether_header *pstEthHead)
{
    unsigned short usEthPktType = 0;

    if(NULL == pstEthHead)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("pstEthHead is NULL!\n");
        return MT_FAILURE;
    }
    /* protocol type,source MAC address,and destination MAC address */
    usEthPktType = ntohs(pstEthHead->ether_type);
    SAMPLE_NET_DUMP_INFO_PRINT(">>> Eth-Pkt-Type:0x%04x(%s) ", usEthPktType, MT_NetDumpgetProName(usEthPktType));
    (void)MT_NetDumpshowMac(0, pstEthHead->ether_shost);
    (void)MT_NetDumpshowMac(1, pstEthHead->ether_dhost);

    return MT_SUCCESS;
}


/*
@brief parse the IP data packet header
@param[in] pstIpHead, The IP header is obtained
@return MT_SUCCESS
@return MT_FAILURE
*/
static int MT_NetDumpparseIpHead(const struct ip *pstIpHead)
{
    struct protoent *pstIpProto = NULL;

    if(NULL == pstIpHead)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("pstIpHead is NULL!\n");
        return MT_FAILURE;
    }
    /* protocal type ,source IP address ,destination IP address  */
    pstIpProto = getprotobynumber(pstIpHead->ip_p);
    if(NULL != pstIpProto)
    {
        MT_NET_DUMP_PRINT(" IP-Pkt-Type:%d(%s)\n ", pstIpHead->ip_p, pstIpProto->p_name);
    }
    else
    {
        MT_NET_DUMP_PRINT(" IP-Pkt-Type:%d(%s)\n ", pstIpHead->ip_p, "None");
    }
    MT_NET_DUMP_PRINT("SAddr=[%s]\n", inet_ntoa(pstIpHead->ip_src));
    MT_NET_DUMP_PRINT(" DAddr=[%s]\n", inet_ntoa(pstIpHead->ip_dst));
    return MT_SUCCESS;
}


/*
@brief Data frame parsing function
@param[in] pcFrameData,The obtained frame
@return MT_SUCCESS
@rerurn MT_FAILURE
*/
static int MT_NetDumpparseFrame(const char *pcFrameData)
{
    int Ret = -1;
    struct ether_header *pstEthHead = NULL;
    struct ip *pstIpHead = NULL;

    /* Ethnet frame header parsing */
    pstEthHead = (struct ether_header*)g_acRecvBuf;
    Ret = MT_NetDumpparseEthHead(pstEthHead);
    if(MT_SUCCESS > Ret)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("MT_NetDumpparseEthHead failed!\n");
        return MT_FAILURE;
    }
    /* IP packet type  */
    pstIpHead = (struct ip *)(pstEthHead + 1);
    Ret = MT_NetDumpparseIpHead(pstIpHead);
    if(MT_SUCCESS > Ret)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("MT_NetDumpparseIpHead failed!\n");
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}


/*
@brief Capture the network card data frame
@param[in] fd, socket
@return void
*/
static void MT_NetDumpstartCapture(void *args)
{
    int Ret = -1;
    socklen_t stFromLen = 0;
    int fd = *(int *)args;

        /* loop monitoring */
        while(g_bTaskQuit == MT_FALSE)
        {
            /* clear the receive buffer */
            memset(g_acRecvBuf, 0, RCV_BUF_SIZE);
            /* Receive data frame */
            Ret = recvfrom(fd, g_acRecvBuf, g_iRecvBufSize, 0, NULL, &stFromLen);
            if(MT_SUCCESS > Ret)
            {
                continue;
            }

            /* parsed data frame*/
            (void)MT_NetDumpparseFrame(g_acRecvBuf);
        }

}
static MT_VOID MT_NetDumpPrintMenu(MT_VOID)
{
    SAMPLE_NET_DUMP_PRINT("     h : help \n");
    SAMPLE_NET_DUMP_PRINT("     q : quit \n");
    SAMPLE_NET_DUMP_PRINT("Dump>> ");

}

static void MT_NetDumpCmdTask(void)
{
    char *fgetret = NULL;
    MT_CHAR inputCmd[32] = { 0 };


    while (1)
    {
        (MT_VOID)MT_NetDumpPrintMenu();
        fgetret = fgets((char *)(inputCmd), (sizeof(inputCmd) - 1), stdin);
        fgetret = fgetret;


        if('q' == inputCmd[0])
        {
            SAMPLE_NET_DUMP_INFO_PRINT("now exit!\n");
            g_bTaskQuit = MT_TRUE;
            break;
        }
        else if('h' == inputCmd[0])
        {
            SAMPLE_NET_DUMP_INFO_PRINT("Print help info \n");
            continue;
        }

    }
}



#ifdef MT_SAMPLE_APP
mt_s32 MT_NetDumpMain(mt_s32 argc, mt_char *argv[])
#else
mt_s32 main(mt_s32 argc, mt_char *argv[])
#endif

{
    int Ret = -1;
    int fd = -1;
    struct ifreq stIf = { 0 };
    struct sockaddr_ll stLocal = { 0 };
    pthread_t recvThread = { 0 };
    struct timeval timeout={3,0};

    /* Create SOCKET */
    fd = socket(PF_PACKET, SOCK_RAW , htons(ETH_P_ALL));
    if(MT_SUCCESS > fd)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("[Error]Initinate L2 raw socket");
        return -1;
    }

    /*Set the promiscuous mode of nics */
    Ret = MT_NetDumpsetPromisc(g_szIfName, fd, 1);
    if(MT_SUCCESS != Ret)
    {
         SAMPLE_NET_DUMP_ERR_PRINT("MT_NetDumpsetPromisc err! Ret = 0x%x\n",Ret);
         close(fd);
         return Ret;
    }
    /* Set the recv timeout period*/
    Ret = setsockopt(fd,SOL_SOCKET,SO_RCVTIMEO,&timeout,sizeof(timeout));
    if(MT_SUCCESS > Ret)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("recv timeout\n");
        close(fd);
        return Ret;
    }
    /* set SOCKET options */
    Ret = setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &g_iRecvBufSize,sizeof(int));
    if(MT_SUCCESS > Ret)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("[Error]Set socket option");
        close(fd);
        return Ret;
    }

    /* obtain the physical NIC interface index */
    strcpy(stIf.ifr_name, g_szIfName);
    Ret = ioctl(fd, SIOCGIFINDEX, &stIf);
    if(MT_SUCCESS > Ret)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("[Error]Ioctl operation");
        close(fd);
        return Ret;
    }
    /* Bind a physical NIC */
    stLocal.sll_family = PF_PACKET;
    stLocal.sll_ifindex = stIf.ifr_ifindex;
    stLocal.sll_protocol = htons(ETH_P_ALL);
    Ret = bind(fd, (struct sockaddr *)&stLocal, sizeof(stLocal));
    if(MT_SUCCESS > Ret)
    {
        SAMPLE_NET_DUMP_ERR_PRINT("[Error]Bind the interface");
        close(fd);
        return Ret;
    }
    g_bTaskQuit = MT_FALSE;
    Ret = pthread_create(&recvThread, MT_NULL, (void * (*)(void *))MT_NetDumpstartCapture, &fd);
    if(MT_SUCCESS != Ret)
    {
        SAMPLE_NET_DUMP_ERR_PRINT(" pthread_create  timeThread failed.\n");
    }

    (MT_VOID)MT_NetDumpCmdTask();

    (MT_VOID)pthread_join(recvThread, NULL);
    /* Close SOCKET */
    close(fd);

    return MT_SUCCESS;
}


