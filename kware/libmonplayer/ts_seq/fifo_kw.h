/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __FIFO_KW_H__
#define __FIFO_KW_H__

/*!
 xxx
  */
void *init_fifo_kw(char *p_fifo, int fifoSize);
/*!
  xxx
  */
void deinit_fifo_kw(void *p_handle);
/*!
 xxxx
  */
void clear_sub_fifo_kw(void *p_handle);
/*!
 xxxx
  */
int tell_fifo_kw(void *p_handle);

int read_sub_fifo_kw(void *p_handle,char *p_data);
int write_sub_fifo_kw(void *p_handle,unsigned char *p_data, long size);

#endif
