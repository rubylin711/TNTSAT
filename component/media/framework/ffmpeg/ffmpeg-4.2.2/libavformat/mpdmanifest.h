/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#include "internal.h"

#define MAX_SEGMENT_NUM (256)

enum {
    DATA_TYPE_NONE,
    DATA_TYPE_MPD,
    DATA_TYPE_MPD_PERIOD,
    DATA_TYPE_MPD_ADAPTATIONSET,
    DATA_TYPE_MPD_CONTENTPROTECTION_MSPR,
    DATA_TYPE_MPD_PERIOD_REPRESENTATION,
    DATA_TYPE_MPD_BASEURL,
    DATA_TYPE_MPD_PERIOD_REPRESENTATION_SEGMEMGBASE,
    DATA_TYPE_MPD_PERIOD_SEGMENTTEMPLATE,
};

typedef struct MPD_SNode {
    long long t;
    long d;
    int r;
} MPD_SNode;

typedef struct MPD_SegmentTimeline {
    MPD_SNode * snode;
    unsigned int len;
} MPD_SegmentTimeline;

typedef struct MPD_SegmentURL {
    char media_url[128];
    struct MPD_SegmentURL * next_media_url;
} MPD_SegmentURL;

typedef struct MPD_SegmentList {
    MPD_SegmentURL * media_url_list;
    MPD_SegmentURL * cur_media_url;
    unsigned int len;
} MPD_SegmentList;

typedef struct MPD_Representation {
    int bandwidth;
    int width;
    int height;
    int frameRate;
    int startNumber;
    char base_url[MAX_URL_SIZE];
    int  segment_num;
    char * segmentList[MAX_SEGMENT_NUM];
    char * mimeType;
    char * codecs;
    float timescale;
    float duration;
    char * media;
    char * init_url;
    char * id;
    MPD_SegmentList segment_list;
    MPD_SegmentTimeline segmentTimeLine;
} MPD_Representation;

typedef struct MPD_ContentProtection{
    char *system_id;
    char *mspr;
}MPD_ContentProtection;

typedef struct MPD_AdaptationSet {
    int maxWidth;
    int maxHeight;
    int startNumber;
    char * init_url;
    char * lang;
    float timescale;
    float duration;
    char * media;
    int nb_representation;
    MPD_Representation ** representation;
    char * base_url;
    char * codecs;
    char * mimeType;
    int audioSamplingRate;
    MPD_SegmentList segment_list;
    MPD_ContentProtection contentProtection;
    MPD_SegmentTimeline segmentTimeLine;
} MPD_AdaptationSet;

typedef struct MPD_Period {
    int duration;//maybe 0

    int nb_adaptationset;
    MPD_AdaptationSet ** adaptationset;
    char * base_url;
    int64_t start_time;
} MPD_Period;

typedef struct MPDManifest {
    unsigned long maxSubsegmentDuration;
    unsigned long period_duration;
    int g_dash_playmode;
    int nb_period;
    MPD_Period ** period;
    int profile_isoff_ondemand;

    /* xml parsing */
    int parse_ret;                      ///< an error occurred while parsing XML
    int data_type;                      ///< next data chunk will be of this style
    char * profiles;
    int mpd_parse_flag;
    int period_parse_flag;
    int adaptationset_parse_flag;
    int representation_parse_flag;
    int segmentlist_parse_flag;
    int64_t availabilityStartTime;
    int64_t publishTime;
    int64_t suggested_presentation_delay;
    int64_t timeShiftBufferDepth;
    unsigned long minimumUpdatePeriod;
    unsigned long minBufferTime;
    unsigned long maxSegmentDuration;
    char * base_url;
} MPDManifest;

int ff_parse_mpd_manifest(uint8_t * buffer, int size, MPDManifest * manifest);
int ff_free_mpd_manifest(MPDManifest * manifest);
