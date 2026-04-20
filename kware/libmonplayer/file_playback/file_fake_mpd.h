/********************************************************************************************/
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif 

typedef struct {
  char* url;
  char* mpd_str;
}DASH_FAKE_MPD;


char *set_dash_playurl(long long duration, char *audio_url, char *video_url);

#ifdef __cplusplus
}
#endif //__cplusplus



