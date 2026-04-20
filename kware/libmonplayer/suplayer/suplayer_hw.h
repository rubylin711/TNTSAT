/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef __MON_PLAYER_HW_H__
#define __MON_PLAYER_HW_H__
int Push_Video_ES(void * hMonplay );
int Push_Audio_ES(void * hMonplay);
MO_S32 MT_Decoder_Init(void * hPlayer);
MO_S32 MT_Decoder_Deinit(void * hPlayer);
int Is_Decoder_End(void * hMonplay);

#endif
