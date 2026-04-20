/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include "string.h"
#include <sys/time.h>
#include "mt_type.h"
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include "sys_define.h"

//#include "mt_drv_mmz.h"
//#include "mt_drv_mem.h"

//#include "av_misc.h"
#include "mt_cc_parse.h"
#include "mt_cc_fifo.h"

//#define CC_INSERTER_SPIN_LOCK_ID           4
//#define DEBUG_CC
#define CC_DEBUG_LEVEL  0

#ifdef DEBUG_CC
//#define CC_PRINTF(level, format, args...) if(level <= DEBUG_LEVEL) {char msg[100]; sprintf(msg,format,##args);printf("[CC: %dms] %s\n", time_ms(), msg); fflush(stdout);}
#define CC_PRINTF(level, format, args...) if(((MT_S32)level) < DEBUG_LEVEL) {char msg[100]; snprintf(msg,100,format,##args);printf("\r\n [CC: ] %s\n", msg);}
#else
#define CC_PRINTF(...)
#endif

extern mt_u8 *CcMalloc(mt_u32 size, mt_u32 align_v);
extern void CcFree(void* addr);
cc_fifo_inserter_struct *g_p_cc_data = MT_NULL;

//static mt_u32 ccPhyAddr=0;
static pthread_mutex_t g_cc_fifo_mutex = PTHREAD_MUTEX_INITIALIZER;

mt_s32 cc_fifo_init(void)
{
    pthread_mutex_lock(&g_cc_fifo_mutex);
    g_p_cc_data = (cc_fifo_inserter_struct *)CcMalloc(sizeof(cc_fifo_inserter_struct), 32);
    g_p_cc_data->fifo_read_p = g_p_cc_data->fifo_write_p = 0;
    pthread_mutex_unlock(&g_cc_fifo_mutex);
    return MT_SUCCESS;
}

mt_s32 cc_fifo_deinit(void)
{
    pthread_mutex_lock(&g_cc_fifo_mutex);
    if(g_p_cc_data != NULL)
    {
        CcFree((void*)g_p_cc_data);
        g_p_cc_data = NULL;
    }
    pthread_mutex_unlock(&g_cc_fifo_mutex);
    return MT_SUCCESS;
}

mt_s32 cc_fifo_put(mt_u8 *cc_data, mt_u32 cc_len)
{
    if(getCcDisplayOnOff() == MT_FALSE)   return   MT_SUCCESS;

    if(cc_len >  CC_FIFO_TOTAL_LENGTH)
    {
        CC_PRINTF(1,"\r\n ERROR ......cc_len=%d\n",cc_len);
        return  MT_FAILURE;
    }

    if(cc_len <= 0)
    {
	   return  MT_FAILURE;
    }
    pthread_mutex_lock(&g_cc_fifo_mutex);
    if(g_p_cc_data == NULL)    
    {
        pthread_mutex_unlock(&g_cc_fifo_mutex);
        return   MT_FAILURE;
    }
    if(g_p_cc_data->fifo_write_p + cc_len <= CC_FIFO_TOTAL_LENGTH)
    {
        memcpy(&(g_p_cc_data->fifo_buff[g_p_cc_data->fifo_write_p]), cc_data, cc_len);
    }
    else
    {
        int len1 = CC_FIFO_TOTAL_LENGTH - g_p_cc_data->fifo_write_p;
        int len2 = cc_len - len1;
        memcpy(&(g_p_cc_data->fifo_buff[g_p_cc_data->fifo_write_p]), cc_data, len1);
        memcpy(&(g_p_cc_data->fifo_buff[0]), cc_data+len1, len2);
        //g_p_cc_data->fifo_write_p = len2;
    }
    g_p_cc_data->fifo_write_p += cc_len;
    g_p_cc_data->fifo_write_p %= CC_FIFO_TOTAL_LENGTH;
    pthread_mutex_unlock(&g_cc_fifo_mutex);
    return MT_SUCCESS;
}

mt_s32 cc_fifo_get(mt_u8 *data, mt_u32* length)
{
    int len = 0;
    int wp = 0;


    if(data == NULL)
    {
        CC_PRINTF(1, "\r\n data = null \n");
        return  MT_FAILURE;
    }
    pthread_mutex_lock(&g_cc_fifo_mutex);
    if(g_p_cc_data == NULL)
    {
        pthread_mutex_unlock(&g_cc_fifo_mutex);
        *length = 0;
        return   MT_FAILURE;
    }
    
    wp = g_p_cc_data->fifo_write_p;
    *length = 0;
    

    if(g_p_cc_data->fifo_read_p  == wp)
    {
        CC_PRINTF(1, "\r\n no data 2\n");
        pthread_mutex_unlock(&g_cc_fifo_mutex);
        return MT_SUCCESS;
    }
    

    if(wp > g_p_cc_data->fifo_read_p)
    {
        len = wp - g_p_cc_data->fifo_read_p;
        if(len >= 6)
        {
	        memcpy(data, &(g_p_cc_data->fifo_buff[g_p_cc_data->fifo_read_p]), len);
            *length = len;
	        //g_p_cc_data->fifo_read_p = g_p_cc_data->fifo_write_p;

            g_p_cc_data->fifo_read_p += len;


            if( (len % 3) || (len > 1024))
            {
              //while(1)
                printf("\n\n\n\n\n\n  not 3 alginnnnnnnnnnnnnnnnn  %d  \n\n\n\n\n\n", len);
            }              
       }
    }
    else
    {
        int len1 = CC_FIFO_TOTAL_LENGTH - g_p_cc_data->fifo_read_p;
        int len2 = wp;
        CC_PRINTF(1, "\r\n len1=%d, \n",len1);

        len = len1 + len2;
        
    	 if(len >= 6)
    	 {
            if( (len % 3) || (len > 1024))
            {
                //while(1)
                printf("\n\n\n\n\n\n  not 3 alginnnnnnnnnnnnnnnnn  1111111  %d  \n\n\n\n\n\n", len);
            }
        
            memcpy(data, &(g_p_cc_data->fifo_buff[g_p_cc_data->fifo_read_p]), len1);
            memcpy(data + len1, &(g_p_cc_data->fifo_buff[0]), len2);
            *length = len1 + wp;
            g_p_cc_data->fifo_read_p = wp;
    	 }
    }
    pthread_mutex_unlock(&g_cc_fifo_mutex);
    return MT_SUCCESS;
}

mt_s32 cc_fifo_clear(void)
{
    pthread_mutex_lock(&g_cc_fifo_mutex);
    if(NULL == g_p_cc_data)
    {
        pthread_mutex_unlock(&g_cc_fifo_mutex);
    	return MT_FAILURE;
    }
    g_p_cc_data->fifo_read_p = g_p_cc_data->fifo_write_p;
    pthread_mutex_unlock(&g_cc_fifo_mutex);
   //g_p_cc_data->fifo_write_p_pre = g_p_cc_data->fifo_write_p;
    return MT_SUCCESS;
}

