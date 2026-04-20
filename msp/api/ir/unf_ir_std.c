/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <malloc.h>
#include <string.h>
#include <semaphore.h>
#include <linux/input.h>
#include <linux/lirc.h>
#include <poll.h>

#include "mt_drv_struct.h"
#include "drv_ir_ioctl.h"
#include "mt_drv_ir.h"
#include "mt_unf_ir.h"
#include "mt_module_debug.h"

#define COMMON_LONGPRESS_INTERVAL  150  //ms common repeat interval [110~150],if < this,we think long press
#define SECONDKEY_MIX_INTERVAL  200     //ms the mix interval between first key and second key
#define REPEATKEY_DFT_INTERVAL  200     //ms the default interval between n key and n+1 key
#define SCANCODE_FIFO_MAX       128     //Max num of key to cache .must be 2^n
#define SCANCODE_FIFO_MAX_MSK   (SCANCODE_FIFO_MAX-1)   //SCANCODE_FIFO_MAX's Mask

typedef struct scancode_fifo_s {
    struct lirc_scancode ircode;
    MT_UNF_KEY_STATUS_E status;
} SCANCODE_FIFO_S;

typedef struct scancode_info_s {
    SCANCODE_FIFO_S fifo[SCANCODE_FIFO_MAX];
    mt_u32 rp;
    mt_u32 wp;
    MT_UNF_KEY_STATUS_E lststatus;
} SCANCODE_INFO_S;

#define LINUX_LIRC_DEV          "/dev/lirc0"
#define LINUX_INPUT_EVENT_DEV   "/dev/input/event0"
#define LINUX_LIRC_CFG_FILE     "/sys/class/rc/rc0/protocols"
#define LINUX_IR_PROC           "/proc/mtir"

static mt_s32 g_Input_Event_Fd = -1;
static mt_s32 g_Lirc_Fd = -1;
static MT_S32 s_i32FetchMode = 0;	//default=scancode
static MT_BOOL s_bEnableKeyUp = 0;
static MT_BOOL s_bEnableRepKey = 0;
static MT_U32 s_u32RepKeyMixInterval = REPEATKEY_DFT_INTERVAL;
static MT_U32 s_u32Key1_2_MixInterval = SECONDKEY_MIX_INTERVAL;
static pthread_t g_irTaskid;
static volatile MT_U32 s_s32TaskRunning = 0;
static volatile SCANCODE_INFO_S scode_info={0};

static pthread_mutex_t      g_IrMutex = PTHREAD_MUTEX_INITIALIZER;
#define MT_IR_LOCK()        (void)pthread_mutex_lock(&g_IrMutex);
#define MT_IR_UNLOCK()      (void)pthread_mutex_unlock(&g_IrMutex);

//protect rp
static pthread_mutex_t      g_read_mutex = PTHREAD_MUTEX_INITIALIZER;
#define MT_IR_Read_LOCK()   (void)pthread_mutex_lock(&g_read_mutex);
#define MT_IR_Read_UNLOCK() (void)pthread_mutex_unlock(&g_read_mutex);

//#define MT_ERR_IR(fmt ...) do{}while(0)
//#define MT_FATAL_IR(fmt ...) do{}while(0)
//#define MT_INFO_IR(fmt ...) do{}while(0)

#define CHECK_IR_OPEN()\
do{\
    MT_IR_LOCK();\
    if (g_Lirc_Fd < 0)\
    {\
        MT_ERR_IR("IR is not open.\n");\
        MT_IR_UNLOCK();\
        return MT_ERR_IR_NOT_INIT;\
    }\
    MT_IR_UNLOCK();\
}while(0)

static int hextod_(char *s, int n)
{
  int v=0, i=0;
  for (i=0; i<n; i++) {
    if ((s[i] >= '0') && (s[i] <= '9')) {
      v= v*16 + (s[i] - '0');
    } else if ((s[i] >= 'a') && (s[i] <= 'f')) {
      v= v*16 + (10 + s[i] - 'a');
    } else if((s[i] >= 'A') && (s[i] <= 'F')) {
      v= v*16 + (10 + s[i] - 'A');
    }
  }
  return v;
}

static mt_s32 EnumProtocal2CharProtocal(unsigned short rc_proto,mt_char *pszProtocolName, mt_s32 s32NameSize)
{
    if(pszProtocolName) {
        pszProtocolName[0]=0;
    } else {
        return MT_FAILURE;
    }

    switch(rc_proto) {
    case RC_PROTO_RC5_:
    case RC_PROTO_RC5X_20_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"rc-5");
        break;
    case RC_PROTO_RC5_SZ_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"rc-5-sz");
        break;
    case RC_PROTO_JVC_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"jvc");
        break;
    case RC_PROTO_SONY12_:
    case RC_PROTO_SONY15_:
    case RC_PROTO_SONY20_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"sony");
        break;
    case RC_PROTO_NEC_:
    case RC_PROTO_NECX_:
    case RC_PROTO_NEC32_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"nec");
        break;
    case RC_PROTO_SANYO_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"sanyo");
        break;
    case RC_PROTO_MCIR2_KBD_:
    case RC_PROTO_MCIR2_MSE_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"mce_kbd");
        break;
    case RC_PROTO_RC6_0_:
    case RC_PROTO_RC6_6A_20_:
    case RC_PROTO_RC6_6A_24_:
    case RC_PROTO_RC6_6A_32_:
    case RC_PROTO_RC6_MCE_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"rc-6");
        break;
    case RC_PROTO_SHARP_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"sharp");
        break;
    case RC_PROTO_XMP_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"xmp");
        break;
    case RC_PROTO_IMON_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"imon");
        break;
    case RC_PROTO_RCMM12_:
    case RC_PROTO_RCMM24_:
    case RC_PROTO_RCMM32_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"rc-mm");
        break;
    case RC_PROTO_PANSNC_7051_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"pnsnc7051");
        break;
    case RC_PROTO_CEC_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"ece");
        break;
    case RC_PROTO_XBOX_DVD_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"xbox");
        break;
    case RC_PROTO_HD5_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"hd-5");
        break;
    case RC_PROTO_OCN_:
        snprintf(pszProtocolName, (size_t)s32NameSize,"ocn");
        break;
    default:
        return MT_FAILURE;
        break;
    }
    return 0;
}

/*************************************************************
Function:       MT_LIRC_IR_Init
Description:    open ir device,and do the basical initialization
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         MT_SUCCESS
                MT_ERR_IR_OPEN_ERR
Others:         NA
*************************************************************/
mt_s32 MT_INPUT_EVENT_Init(mt_void)
{
    mt_s32 tmp_fd=-1;

    MT_IR_LOCK();
    if (g_Input_Event_Fd > 0) {
        MT_IR_UNLOCK();
        return MT_SUCCESS;
    }

    tmp_fd = open(LINUX_INPUT_EVENT_DEV, O_RDWR | O_CLOEXEC, 0);
    g_Input_Event_Fd = tmp_fd;
    MT_IR_UNLOCK();

    if (tmp_fd < 0) {
        MT_FATAL_IR("open IR err.\n");
        return MT_ERR_IR_OPEN_ERR;
    }

    return MT_SUCCESS;
}

/*************************************************************
Function:       MT_UNF_IR_DeInit
Description:    close ir device
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         MT_SUCCESS
Others:         NA
*************************************************************/
mt_s32 MT_INPUT_EVENT_DeInit(mt_void)
{
    MT_IR_LOCK();

    if (g_Input_Event_Fd < 0) {
        MT_IR_UNLOCK();
        return MT_SUCCESS;
    }

    close(g_Input_Event_Fd);

    g_Input_Event_Fd = -1;

    MT_IR_UNLOCK();

    return MT_SUCCESS;
}

/*************************************************************
Function:       MT_LIRC_IR_Add_KeyMap
Description:    map scancode to vkey.then drv can get scancode and user can get vkey
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         MT_SUCCESS
                MT_FAILURE
Others:         NA
*************************************************************/
mt_s32 MT_INPUT_EVENT_Add_KeyMap(mt_u64 scancode,mt_u32 vkey)
{
    struct input_keymap_entry tmp= {0};
    int ret=0;

    tmp.len = 8;    //if len=4,32bits scancode. if len=8,64bits scancode.
    tmp.keycode = vkey;
    *((u64 *)&tmp.scancode) = scancode;

    MT_IR_LOCK();
    if (g_Input_Event_Fd > 0) {
        ret=ioctl(g_Input_Event_Fd, EVIOCSKEYCODE_V2, &tmp);
    } else {
        ret=MT_FAILURE;
    }
    MT_IR_UNLOCK();

    if (ret) {
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

/*************************************************************
Function:       MT_LIRC_IR_Del_KeyMap
Description:    unmap scancode to vkey.then drv can get scancode but user can't get vkey
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         MT_SUCCESS
                MT_FAILURE
Others:         NA
*************************************************************/
mt_s32 MT_INPUT_EVENT_Del_KeyMap(mt_u64 scancode)
{
    struct input_keymap_entry tmp= {0};
    int ret=0;

    tmp.len = 8;    //if len=4,32bits scancode. if len=8,64bits scancode.
    tmp.keycode = 0;
    *((u64 *)&tmp.scancode) = scancode;

    MT_IR_LOCK();
    if (g_Input_Event_Fd > 0) {
        ret=ioctl(g_Input_Event_Fd, EVIOCSKEYCODE_V2, &tmp);
    } else {
        ret=MT_FAILURE;
    }
    MT_IR_UNLOCK();

    if (ret) {
        return MT_FAILURE;
    }
    return MT_SUCCESS;
}

/*************************************************************
Function:       MT_LIRC_IR_Read_Key
Description:    read vkey from drv
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         MT_SUCCESS
                MT_FAILURE
Others:         NA
*************************************************************/
mt_s32 MT_INPUT_EVENT_Read_Vkey(struct input_event *ev,mt_s32 timeout_ms)
{
    int rd=0,ret=0;
    struct input_event ev_tmp= {0};
    struct pollfd fds[1];

    if(NULL == ev) {
        return MT_FAILURE;
    }

    MT_IR_LOCK();
    if(g_Input_Event_Fd > 0) {
        fds[0].fd = g_Input_Event_Fd;
        fds[0].events = POLLIN;

        int rc = (int)poll(fds, 1, timeout_ms);
        if(0 < rc) {
            rd = (int)read(g_Input_Event_Fd, &ev_tmp, sizeof(struct input_event));//read event
            MT_IR_UNLOCK();

            if (rd != sizeof(struct input_event)) {
                MT_ERR_IR("Failed to read input event");
                return MT_FAILURE;
            }

            if (ev_tmp.type == EV_KEY) {    //check event
                *ev = ev_tmp;
                return MT_SUCCESS;
            }
            return MT_FAILURE;
        } else if (rc == -1) {   //printf("Failed to poll device\n");
            ret=MT_FAILURE;
        } else {                //printf("Timeout occurred\n");
            ret=MT_FAILURE;
        }
    } else {
        ret=MT_FAILURE;
    }
    MT_IR_UNLOCK();

    return ret;
}

/*************************************************************
Function:       MT_LIRC_IR_Get_Protocols
Description:    get all of enabled Protocols and disabled Protocols
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         MT_SUCCESS
                MT_FAILURE
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_Get_Protocols(char disprot[][16], mt_u32 *count_dis, char enprot[][16], mt_u32 *count_en)
{
    char *pt_last=NULL,*pt_now=NULL,buf[256]= {0};
    mt_u32 count_dis_tmp=0,count_en_tmp=0;

    if(NULL==disprot || NULL==enprot || NULL==count_dis || NULL==count_en) {
        return MT_FAILURE;
    }

    *count_dis=0;
    *count_en=0;
    FILE *fp = fopen(LINUX_LIRC_CFG_FILE, "r");
    if (NULL == fp) {
        MT_ERR_IR("Failed to open protocols file");
        return MT_FAILURE;
    }
    fgets(buf, 256, fp);  //read one line
    fclose(fp);

    pt_now=buf;
    pt_last=buf;
    for( ; ; ) {
        while((' ' != *pt_now) && (0 != *pt_now)) {
            pt_now++;
        }
        if(*pt_last == '[') {   //get enbale-prot
            memcpy(enprot[count_en_tmp],pt_last+1,(size_t)(pt_now-pt_last-2));
            enprot[count_en_tmp][pt_now-pt_last-2]=0;
            count_en_tmp++;
        } else {                 //get diabled-prot
            memcpy(disprot[count_dis_tmp],pt_last,(size_t)(pt_now-pt_last));
            disprot[count_dis_tmp][pt_now-pt_last]=0;
            count_dis_tmp++;
        }
        if(0 == *pt_now) {
            break;
        }
        pt_now++;           //jump ' '
        pt_last=pt_now;
    }

    if(count_en_tmp) {  //remove "[lirc]"
        count_en_tmp--;
    }
    *count_dis = count_dis_tmp;
    *count_en = count_en_tmp;

    return MT_SUCCESS;
}

/*************************************************************
Function:       MT_LIRC_IR_Config_Protocols
Description:    to enable Protocols or disable Protocols.
                if enable a protocol, then drv can get all scancodes of this protocol
                if disable a protocol, then drv can't get any scancode of this protocol
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         MT_SUCCESS
                MT_FAILURE
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_Config_Protocols_ByName(mt_char prot[][16], mt_u32 count,mt_s32 flag_enable)
{
    mt_s32 i=0,ret=MT_FAILURE;
    char tmpuse[64]= {0};
    FILE *fp=NULL;
    char *ep=NULL;

    if((NULL == prot) || (0==count)) {
        return MT_FAILURE;
    }

    ep=malloc(16*count);
    if(ep) {
        ep[0]=0;
        for(i=0; i<count; i++) {
            if(flag_enable) {
                snprintf(tmpuse, sizeof(tmpuse),"+%s ",prot[i]);
            } else {
                snprintf(tmpuse, sizeof(tmpuse),"-%s ",prot[i]);
            }
            strcat(ep,tmpuse);
        }
        fp = fopen(LINUX_LIRC_CFG_FILE, "w");
        if (NULL != fp) {
            fwrite(ep,strlen(ep),1,fp);
            fclose(fp);
            ret=MT_SUCCESS;
        }
        free(ep);
    }
    return ret;
}
/*************************************************************
Function:       MT_LIRC_IR_Config_Protocols
Description:    to enable Protocols or disable Protocols.
                if enable a protocol, then drv can get all scancodes of this protocol
                if disable a protocol, then drv can't get any scancode of this protocol
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         MT_SUCCESS
                MT_FAILURE
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_Config_Protocols_ByType(irda_protocol_t *prot, mt_u32 count,mt_s32 flag_enable)
{
    mt_s32 i=0,ret=MT_FAILURE;
    char tmpuse[68]= {0};
    FILE *fp=NULL;
    char *ep=NULL;
    char pszProtocolName[64];

    if((NULL == prot) || (0==count)) {
        return MT_FAILURE;
    }

    ep=malloc(16*count);
    if(ep) {
        ep[0]=0;
        for(i=0; i<count; i++) {
            ret=EnumProtocal2CharProtocal(prot[i],pszProtocolName,64);
            if(0 == ret) {
                if(flag_enable) {
                    snprintf(tmpuse, sizeof(tmpuse),"+%s ",pszProtocolName);
                } else {
                    snprintf(tmpuse, sizeof(tmpuse),"-%s ",pszProtocolName);
                }
                strcat(ep,tmpuse);
            }
        }
        fp = fopen(LINUX_LIRC_CFG_FILE, "w");
        if (NULL != fp) {
            //printf("++cfg.ep=%s\n",ep);
            fwrite(ep,strlen(ep),1,fp);
            fclose(fp);
            ret=MT_SUCCESS;
        }
        free(ep);
    }
    return ret;
}
/*************************************************************
Function:       MT_LIRC_IR_Config_Protocols
Description:    to enable Protocols or disable Protocols.
                if enable a protocol, then drv can get all scancodes of this protocol
                if disable a protocol, then drv can't get any scancode of this protocol
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         MT_SUCCESS
                MT_FAILURE
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_Protocal_Type2Name(irda_protocol_t prot, mt_char *pszProtocolName, mt_s32 s32NameSize)
{
    return EnumProtocal2CharProtocal(prot,pszProtocolName,s32NameSize);
}
static void ir_push_fifo(struct lirc_scancode *code, MT_UNF_KEY_STATUS_E status)
{
    mt_u32 tmpuse = ((scode_info.wp+1)&SCANCODE_FIFO_MAX_MSK);
    if(tmpuse != scode_info.rp) {
        scode_info.fifo[scode_info.wp].ircode=*code;
        scode_info.fifo[scode_info.wp].status=status;
        scode_info.wp = tmpuse;
        scode_info.lststatus = status;
    }
}

static int ir_pop_fifo(struct lirc_scancode *code, MT_UNF_KEY_STATUS_E *status)
{
    if(scode_info.wp != scode_info.rp) {
        *code = scode_info.fifo[scode_info.rp].ircode;
        *status = scode_info.fifo[scode_info.rp].status;
        scode_info.rp = ((scode_info.rp+1)&SCANCODE_FIFO_MAX_MSK);
        return MT_SUCCESS;
    }
    return MT_FAILURE;
}
/*
the thread that recieve key info from /dev/lirc0.
key:manual press must be bigger than long press(110ms).
*/
static void * IR_Receive_(void *args)
{
    mt_u32 u32TimeoutMs=20;             //20ms
    struct pollfd fds[1]= {0};
    struct lirc_scancode nowcode={0};   //current key info
    struct lirc_scancode lstcode={0};   //last key info
    unsigned int cellsz=sizeof(struct lirc_scancode);
    u32 sw_second_key_time_plus=0;      //totol time between first key and second key
    u32 sw_repeat_key_time_plus=0;      //totol time between n key and n+1 key, n>=2
    mt_u64 lstkey_time=0;               //the created time of a key
    int rc=0,rd=0;

    lstcode.scancode = 0;
    while(s_s32TaskRunning) {
        fds[0].fd = g_Lirc_Fd;
        fds[0].events = POLLIN;
        rc = (int)poll(fds, 1, (int)u32TimeoutMs);
        if(0 < rc) {
            rd = (int)read(g_Lirc_Fd, &nowcode, cellsz);
            if (rd == cellsz) {                                      //get key
                MT_DBG_IR("ir.uapi get key:%llx,%lld\n",nowcode.scancode,nowcode.timestamp);
                if(lstcode.scancode != nowcode.scancode){            //different key
                    if(s_bEnableKeyUp && (0 != lstcode.scancode)) {
                        ir_push_fifo(&lstcode,MT_UNF_KEY_STATUS_UP);
                        lstcode.scancode = 0;
                    }
                    ir_push_fifo(&nowcode,MT_UNF_KEY_STATUS_DOWN);
                    sw_second_key_time_plus = 0;
                    lstcode=nowcode;
                }else{                                                  //same key
                    mt_u64 nowtime=(nowcode.timestamp/1000000);
                    if((nowtime-lstkey_time) > COMMON_LONGPRESS_INTERVAL){ //it's a short press, think a newkey
                        if(s_bEnableKeyUp) {
                            ir_push_fifo(&lstcode,MT_UNF_KEY_STATUS_UP);
                            lstcode.scancode = 0;
                        }
                        ir_push_fifo(&nowcode,MT_UNF_KEY_STATUS_DOWN);
                        sw_second_key_time_plus = 0;
                        lstcode=nowcode;
                    }else{                                              //it's a long press
                        if(MT_UNF_KEY_STATUS_DOWN == scode_info.lststatus){ //lastkey=down,deal second key
                            sw_second_key_time_plus += (nowtime-lstkey_time);
                            if(sw_second_key_time_plus >= s_u32Key1_2_MixInterval){
                                if(s_bEnableRepKey) {
                                    ir_push_fifo(&nowcode,MT_UNF_KEY_STATUS_HOLD);
                                    sw_second_key_time_plus = 0;
                                    sw_repeat_key_time_plus = 0;
                                    lstcode=nowcode;
                                }else{
                                    if(s_bEnableKeyUp) {
                                        ir_push_fifo(&lstcode,MT_UNF_KEY_STATUS_UP);
                                        lstcode.scancode = 0;
                                    }
                                    ir_push_fifo(&nowcode,MT_UNF_KEY_STATUS_DOWN);
                                    sw_second_key_time_plus = 0;
                                    sw_repeat_key_time_plus = 0;
                                    lstcode=nowcode;
                                }
                            }
                        }else{
                            sw_repeat_key_time_plus += (nowtime-lstkey_time);
                            if(sw_repeat_key_time_plus >= s_u32RepKeyMixInterval){
                                if(s_bEnableRepKey) {
                                    ir_push_fifo(&nowcode,MT_UNF_KEY_STATUS_HOLD);
                                    sw_repeat_key_time_plus = 0;
                                    lstcode=nowcode;
                                }else{
                                    if(s_bEnableKeyUp) {
                                        ir_push_fifo(&lstcode,MT_UNF_KEY_STATUS_UP);
                                        lstcode.scancode = 0;
                                    }
                                    ir_push_fifo(&nowcode,MT_UNF_KEY_STATUS_DOWN);
                                    sw_repeat_key_time_plus = 0;
                                    lstcode=nowcode;
                                }
                            }
                        }
                    }
                }
                lstkey_time=(nowcode.timestamp/1000000);
            }
        }else if (rc == -1) {
            ;//printf("Failed to poll device\n");
        } else {
            if(s_bEnableKeyUp && (0 != lstcode.scancode)) {
                struct timespec end;
                ulong diffms=0;
                clock_gettime(CLOCK_MONOTONIC, &end);
                diffms = (end.tv_sec*1000+end.tv_nsec/1000000);
                if(diffms > (20+lstkey_time+COMMON_LONGPRESS_INTERVAL)){
                    ir_push_fifo(&lstcode,MT_UNF_KEY_STATUS_UP);
                    lstcode.scancode=0;
                }
            }
        }
    }
    return 0;
}

/*************************************************************
Function:       HI_UNF_IR_Init
Description:    open ir device,and do the basical initialization
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         HI_SUCCESS
                HI_ERR_IR_OPEN_ERR
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_Init(mt_void)
{
    /* if had opened will return success*/
    int ret = 0;
    MT_IR_LOCK();
    if (g_Lirc_Fd > 0) {
        MT_IR_UNLOCK();
        return MT_SUCCESS;
    }

    g_Lirc_Fd = open(LINUX_LIRC_DEV, O_RDWR, 0);
    if (g_Lirc_Fd < 0) {
        MT_IR_UNLOCK();
        MT_FATAL_IR("open IR err.\n");
        return MT_ERR_IR_OPEN_ERR;
    }
    s_u32RepKeyMixInterval = REPEATKEY_DFT_INTERVAL;
    scode_info.rp=0;
    scode_info.wp=0;
    scode_info.lststatus=MT_UNF_KEY_STATUS_BUTT;
    s_s32TaskRunning = 1;
    ret = pthread_create(&g_irTaskid, NULL, IR_Receive_, NULL);
    if(0 != ret){
        close(g_Lirc_Fd);
        g_Lirc_Fd = -1;
        MT_IR_UNLOCK();
        MT_FATAL_IR("open IR err1.\n");
        return MT_ERR_IR_OPEN_ERR;
    }
    MT_IR_UNLOCK();

    return MT_SUCCESS;
}

/*************************************************************
Function:       HI_UNF_IR_DeInit
Description:    close ir device
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         HI_SUCCESS
                HI_ERR_IR_CLOSE_ERR
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_DeInit(mt_void)
{
    mt_s32 Ret;

    MT_IR_LOCK();
    if (g_Lirc_Fd < 0) {
        MT_IR_UNLOCK();
        return MT_SUCCESS;
    }
    s_s32TaskRunning = 0;
    pthread_join(g_irTaskid, 0);
    Ret = close(g_Lirc_Fd);
    if(MT_SUCCESS != Ret) {
        MT_IR_UNLOCK();
        MT_FATAL_IR("Close IR err.\n");
        return MT_ERR_IR_CLOSE_ERR;
    }

    g_Lirc_Fd = -1;

    MT_IR_UNLOCK();

    return MT_SUCCESS;
}

/*************************************************************
Function:       HI_UNF_IR_Enable
Description:    Enable ir device
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         HI_SUCCESS
                HI_ERR_IR_INVALID_PARA
                HI_ERR_IR_ENABLE_FAILED
                HI_ERR_IR_NOT_INIT
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_Enable(MT_BOOL bEnable, irda_protocol_t u8Protocol)
{
    irda_protocol_t tmp=u8Protocol;
    return MT_UNF_IR_Config_Protocols_ByType(&tmp,1,bEnable?1:0);
}

/*************************************************************
Function:       MT_UNF_IR_GetUserAndKey
Description:    Get usrcode and keycode
Calls:
Data Accessed:
Data Updated:   NA
Input:          pszProtocolName pu64KeyId
Output:         Usercode Keycode
Return:         HI_SUCCESS
                HI_ERR_IR_INVALID_PARA
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_GetUserAndKey(mt_char *pszProtocolName, mt_u64 *pu64KeyId, mt_u32 *Usercode, mt_u32 *Keycode)
{
    mt_u32 keytype = 0;

    if ((NULL == pszProtocolName) ||
       (NULL == pu64KeyId) ||
       (NULL == Usercode) ||
       (NULL == Keycode)) {
        return MT_ERR_IR_INVALID_PARA;
    }
    keytype = hextod_(pszProtocolName, 2);
    if (RC_PROTO_NEC_ == keytype) {//8(usr)+8(key)
        *Usercode= ((*pu64KeyId>>8)&0xff);
        *Keycode = ((*pu64KeyId)&0xff);
    } else if (RC_PROTO_NECX_ == keytype) {//16+8
        *Usercode= ((*pu64KeyId>>8)&0xffff);
        *Keycode = ((*pu64KeyId)&0xff);
    } else if (RC_PROTO_NEC32_ == keytype) {//16+16
        *Usercode= ((*pu64KeyId>>16)&0xffff);
        *Keycode = ((*pu64KeyId)&0xffff);
    } else if (RC_PROTO_RC5_ == keytype) {//5+(6+1)
        *Usercode= ((*pu64KeyId>>8)&0x1f);
        *Keycode = ((*pu64KeyId)&0x7f);
    } else if (RC_PROTO_RC5X_20_ == keytype) {//5+16
        *Usercode= ((*pu64KeyId>>16)&0x1f);
        *Keycode = ((*pu64KeyId)&0xffff);
    } else if (RC_PROTO_RC5_SZ_ == keytype) {//8+6
        *Usercode= ((*pu64KeyId>>6)&0xff);
        *Keycode = ((*pu64KeyId)&0x3f);
    } else if (RC_PROTO_RCMM32_ == keytype) {//7+8
        *Usercode= ((*pu64KeyId>>8)&0x7f);
        *Keycode = ((*pu64KeyId)&0xff);
    } else if (RC_PROTO_PANSNC_7051_ == keytype) {//32+16
        *Usercode= ((*pu64KeyId>>16)&0xffffffff);
        *Keycode = ((*pu64KeyId)&0xffff);
    } else if (RC_PROTO_HD5_ == keytype) {//5+6
        *Usercode= ((*pu64KeyId>>8)&0x1f);
        *Keycode = ((*pu64KeyId)&0x3f);
    } else if (RC_PROTO_OCN_ == keytype) {//24+16
        *Usercode= ((*pu64KeyId>>16)&0xffffff);
        *Keycode = ((*pu64KeyId)&0xffff);
    } else {
        *Usercode= 0;
        *Keycode = (u32)*pu64KeyId;
    }
    //printf("tp=%d, all=%llx,u=%x,k=%x\n", keytype,*pu64KeyId,*Usercode,*Keycode);
    return MT_SUCCESS;
}

/*************************************************************
Function:       MT_UNF_IR_ClearkeyWithProtocl
Description:    clear irda key
Calls:
Data Accessed:
Data Updated:   NA
Input:          Na
Output:         Na
Return:        HI_ERR_IR_NULL_PTR
                HI_ERR_IR_INVALID_PARA
                HI_ERR_IR_SET_BLOCKTIME_FAILED
                HI_ERR_IR_READ_FAILED
                HI_ERR_IR_NOT_INIT
                HI_SUCCESS
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_ClearkeyWithProtocol(void)
{
    unsigned int mode2_scankey;
    struct lirc_scancode tmpcode;
    unsigned int cellsz=sizeof(struct lirc_scancode);
    int readed=0;
    struct pollfd fds[1]= {0};
    int rc;

    if (g_Lirc_Fd < 0) {
        return MT_ERR_IR_NOT_INIT;
    }

    if(0==s_i32FetchMode) {
        while(1) {
            fds[0].fd = g_Lirc_Fd;
            fds[0].events = POLLIN;
            rc = (int)poll(fds, 1, 1);
            if(0 < rc) {
                read(g_Lirc_Fd,&tmpcode,cellsz);
            } else {
                break;
            }
        }
    } else {
        while(1) {
            readed=(int)read(g_Lirc_Fd,&mode2_scankey,4);
            if(readed != 4) {
                break;
            }
        }
    }
    return MT_SUCCESS;
}
/*************************************************************
Function:       HI_UNF_IR_GetValueWithProtocol
Description:    get the value and status of key
Calls:
Data Accessed:
Data Updated:   NA
Input:          u32TimeoutMs: overtime value with unit of ms : 0 means no block while 0xFFFFFFFF means block forever
                s32NameSize: buffer size of protocol_name.
Output:         pu32PressStatus:    status of the key
                   0:  means press
                   1:  means hold
                   2:  means release
                pu64KeyId:          value of the key

                pszProtocolName: ir protocol name.

Return:         HI_ERR_IR_NULL_PTR
                HI_ERR_IR_INVALID_PARA
                HI_ERR_IR_SET_BLOCKTIME_FAILED
                HI_ERR_IR_READ_FAILED
                HI_ERR_IR_NOT_INIT
                HI_SUCCESS
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_GetValueWithProtocol(MT_UNF_KEY_STATUS_E *penPressStatus, mt_u64 *pu64KeyId,
                                      mt_char *pszProtocolName, mt_s32 s32NameSize, mt_u32 u32TimeoutMs)
{
    mt_s32 ret = -1;
    struct lirc_scancode tmpcode;

    if (!penPressStatus) {
        MT_ERR_IR("para penPressStatus is null.\n");
        return MT_ERR_IR_NULL_PTR;
    }

    if (!pu64KeyId) {
        MT_ERR_IR("para pu64KeyId is null.\n");
        return MT_ERR_IR_NULL_PTR;
    }

    if (pszProtocolName && (s32NameSize < PROTOCOL_NAME_SZ)) {
        MT_ERR_IR("Invalid protocol buffer!\n");
        return MT_ERR_IR_INVALID_PARA;
    }
    CHECK_IR_OPEN();
    if (0 != s_i32FetchMode) {
        return MT_ERR_IR_NOT_INIT;
    }
    u32TimeoutMs = (u32TimeoutMs/10*10);
    do{
        MT_IR_Read_LOCK();
        ret = ir_pop_fifo(&tmpcode,penPressStatus);
        MT_IR_Read_UNLOCK();
        if(MT_SUCCESS == ret){
            break;
        }
        MT_USLEEP(10*1000);
        u32TimeoutMs -= 10;
    }while(u32TimeoutMs);

    if(MT_SUCCESS == ret){
        *pu64KeyId = tmpcode.scancode;
        if (pszProtocolName && (s32NameSize >= PROTOCOL_NAME_SZ)) {
            snprintf(pszProtocolName, (size_t)s32NameSize,"%02x: ",tmpcode.rc_proto);
            EnumProtocal2CharProtocal(tmpcode.rc_proto, pszProtocolName+4, s32NameSize-4);
        }
        return MT_SUCCESS;
    }
    return MT_FAILURE;
}

/*************************************************************
Function:       HI_UNF_IR_SetFetchMode
Description:    set key fetch mode or symbol mode. Don't support in IR_STD.
Calls:
Data Accessed:
Data Updated:   NA
Input:          mode: 0-> key mode. 1-> raw symbol mode.
Output:
Return:         HI_SUCCESS
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_SetFetchMode(mt_s32 s32Mode)
{
    int flags=0;
    unsigned int mode=0;
    s_i32FetchMode = s32Mode;

    if (g_Lirc_Fd < 0) {
        return MT_ERR_IR_NOT_INIT;
    }

    flags = fcntl(g_Lirc_Fd, F_GETFL);      //get cur flag
    if (flags < 0) {
        ;//perror("Failed to get file flags");
    }

    if(0==s32Mode) {                        //LIRC_MODE_SCANCODE
        mode=LIRC_MODE_SCANCODE;
        ioctl(g_Lirc_Fd,LIRC_SET_REC_MODE,&mode);
        flags &= ~O_NONBLOCK;               //block
    } else {                                 //pulse space
        mode=LIRC_MODE_MODE2;
        ioctl(g_Lirc_Fd,LIRC_SET_REC_MODE,&mode);
        flags |= O_NONBLOCK;        //non block
    }

    if (fcntl(g_Lirc_Fd, F_SETFL, flags) < 0) { //set new flag
        ;//perror("Failed to set file flags");
    }

    return MT_SUCCESS;
}

/*************************************************************
Function:       HI_UNF_IR_GetSymbol
Description:    get one raw symbols from ir module.Don't support in IR_STD.
Calls:
Data Accessed:
Data Updated:   NA
Input:          u32TimeoutMs: read timeout in ms.
Output:         pu64lower, pu64upper.
Return:         HI_ERR_IR_UNSUPPORT
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_GetSymbol(ir_pluse_data_s *plusedata)
{
    mt_u32 i=0,mode2_scankey;
    int readed=0;

    if (g_Lirc_Fd < 0) {
        return MT_ERR_IR_NOT_INIT;
    }
    if (0==s_i32FetchMode) {
        return MT_ERR_IR_NOT_INIT;
    }
    if (NULL == plusedata) {
        return MT_ERR_IR_NULL_PTR;
    }

    while(i<IR_MAX_PLUSE_NUM) {
        readed=(int)read(g_Lirc_Fd,&mode2_scankey,4);
        if(4 != readed) {
            break;
        }
        if(LIRC_IS_SPACE(mode2_scankey)) {
            plusedata->period[i]=plusedata->ontime[i]+(MT_U16)LIRC_VALUE(mode2_scankey);
            i++;
            plusedata->vaild=1;
        } else if(LIRC_IS_PULSE(mode2_scankey)) {
            plusedata->ontime[i]=(MT_U16)LIRC_VALUE(mode2_scankey);
        }
    }
    if (0 == i) {
        return MT_ERR_IR_READ_FAILED;
    }
    plusedata->cur = (MT_U16)i;
    return MT_SUCCESS;
}

/*************************************************************
Function:       HI_UNF_IR_EnableKeyUp
Description:    config whether report the state of key release
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         HI_SUCCESS
                HI_ERR_IR_INVALID_PARA
                HI_ERR_IR_SET_KEYUP_FAILED
                HI_ERR_IR_NOT_INIT
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_EnableKeyUp(MT_BOOL bEnable)
{
    s_bEnableKeyUp = bEnable;
    return MT_SUCCESS;
}

/*************************************************************
Function:       HI_UNF_IR_IsRepKey
Description:    config whether report repeat key
Calls:
Data Accessed:
Data Updated:   NA
Input:
Output:
Return:         HI_SUCCESS
                HI_ERR_IR_INVALID_PARA
                HI_ERR_IR_SET_REPEAT_FAILED
                HI_ERR_IR_NOT_INIT
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_EnableRepKey(MT_BOOL bEnable)
{
    s_bEnableRepKey = bEnable;
    return MT_SUCCESS;
}

/*************************************************************
Function:       HI_UNF_IR_RepKeyTimeoutVal
Description:    Set the reporting interval when you keep pressing button.
Calls:
Data Accessed:
Data Updated:   NA
Input:          u32TimeoutMs  The minimum interval to report repeat key
Output:
Return:         HI_SUCCESS
                HI_ERR_IR_NOT_INIT
                HI_ERR_IR_SET_REPKEYTIMEOUT_FAILED
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_SetRepKeyTimeoutAttr(mt_u32 u32TimeoutMs)
{
    s_u32RepKeyMixInterval = u32TimeoutMs;
    return MT_SUCCESS;
}

/*************************************************************
Function:       MT_UNF_IR_SetKey1and2TimeoutAttr
Description:    Set the reporting interval when first key and second key.
Calls:
Data Accessed:
Data Updated:   NA
Input:          u32TimeoutMs  The minimum interval to report repeat key
Output:
Return:         HI_SUCCESS
                HI_ERR_IR_NOT_INIT
                HI_ERR_IR_SET_REPKEYTIMEOUT_FAILED
Others:         NA
*************************************************************/
mt_s32 MT_UNF_IR_SetKey1and2TimeoutAttr(mt_u32 u32TimeoutMs)
{
    s_u32Key1_2_MixInterval = u32TimeoutMs;
    if (s_u32Key1_2_MixInterval > 10000) {
        s_u32Key1_2_MixInterval = 10000;
    }
    return MT_SUCCESS;
}

mt_s32 MT_UNF_IR_SetWaveFilter(ir_wavefilter_config_s *irconfig)
{
    //"echo \"pm 2 1 24 0 0xf21a14 2 24 0 0xf21a14\" >/proc/mt_ir
    //pm num {prot wavelen wavemask wavecode}.loop
    mt_s32 i=0,ret=MT_FAILURE;
    char tmpuse[128]= {0};
    FILE *fp=NULL;
    char *ep=NULL;
    irda_wfilt_cfg_t *tmppt=NULL;

    ep=malloc(256);
    if(ep) {
        snprintf(ep, 256,"pm %d",irconfig->irda_wfilt_channel);
        if(4 < irconfig->irda_wfilt_channel) {
            irconfig->irda_wfilt_channel = 4;
        }
        for(i=0; i<irconfig->irda_wfilt_channel; i++) {
            tmppt=&irconfig->irda_wfilt_channel_cfg[i];
            snprintf(tmpuse, sizeof(tmpuse)," %d %d 0x%llx 0x%llx",
                     tmppt->protocol,tmppt->addr_len,tmppt->wfilt_mask,tmppt->wfilt_code);
            strcat(ep,tmpuse);
        }
        fp = fopen(LINUX_IR_PROC, "w");
        if (NULL != fp) {
            //printf("++ep:%s\n",ep);
            fwrite(ep,strlen(ep),1,fp);
            fclose(fp);
            ret=MT_SUCCESS;
        }
        free(ep);
    }
    return ret;
}

