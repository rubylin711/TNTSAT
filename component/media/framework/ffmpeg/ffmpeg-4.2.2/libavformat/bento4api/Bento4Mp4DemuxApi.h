#ifndef AP4_BENTO4MP4DEMUX_API_H
#define AP4_BENTO4MP4DEMUX_API_H

#define BENTO4_SEEK_BYTE 0
#define BENTO4_SEEK_TIME 1

typedef signed char s8;

/*!
  Unsigned integer, weight is 8 bits. In most case it equal to unsigned char.
  */
typedef unsigned char u8;

/*!
  Signed integer, weight is 16 bits. In most case it equal to short.
  */
typedef signed short s16;

/*!
  Unsigned integer, weight is 16 bits. In most case it equal to unsigned short.
  */
typedef unsigned short u16;

/*!
  Signed integer, weight is 32 bits. In most case it equal to long.
  */
typedef signed long s32;

/*!
  Unsigned integer, weight is 32 bits. In most case it equal to unsigned long.
  */
typedef unsigned long u32;
/*!
  Signed integer, weight is 64 bits
  */
typedef signed long long s64;
/*!
  Usigned integer, weight is 64 bits
  */
typedef unsigned long long u64;

/*!
  Boolean type, the value should be FALSE and TRUE. Recommand replace BOOL by
  RET_CODE as return value.
  */
typedef int BOOL;
#define RET_CODE s32
/*!
  Boolean false
  */
#ifndef FALSE
/*!
  Boolean false
  */
#define FALSE (0)
#endif

/*!
  Boolean true
  */
#ifndef TRUE
/*!
  Boolean true
  */
#define TRUE (1)
#endif
#ifndef NULL
#define NULL 0
#endif

#ifdef __cplusplus
extern "C" {
#endif



int    Bento4Mp4DemuxApi_Prob(char* buffer, int len);
void*  Bento4Mp4DemuxApi_ReadHeader(AVFormatContext *s);
int    Bento4Mp4DemuxApi_ResetHeader(void *p_instance, AVFormatContext *s);
int    Bento4Mp4DemuxApi_ReadPacket(void *p_instance, AVStream *st, AVIndexEntry *sample, AVPacket *pkt);
int    Bento4Mp4DemuxApi_BuildIndexEntry(void *p_instance, AVStream *st);
int    Bento4Mp4DemuxApi_BuildMulIndexEntry(void *p_instance, AVFormatContext *s);
int    Bento4Mp4DemuxApi_Seek(void *p_instance, s64 offset, s32 type);
int    Bento4Mp4DemuxApi_CloseStream(void *p_instance, AVStream *st);
int    Bento4Mp4DemuxApi_GetNextStForMultiTracks(void *p_instance);
int    Bento4Mp4DemuxApi_Check_Switch_Streams(void *p_instance, AVStream *st);
int    Bento4Mp4DemuxApi_Destroy(void *p_instance);

//see _GstSidxBoxEntry
typedef struct _SidxBoxEntry
{
  u32 size;
  s64 duration;
  BOOL starts_with_sap;
  u8 sap_type;
  u32 sap_delta_time;

  s64 offset;
  s64 pts;
} SidxBoxEntry;

//see _GstSidxBox
typedef struct _SidxBox
{
  u32 timescale;
  s64 earliest_pts;
  s64 first_offset;
  s64 sidx_base_offset;

  //int entry_index;
  int entries_count;

  //GstSidxBoxEntry *entries;
  SidxBoxEntry *entries;
} SidxBox;

typedef struct _TfraEntry
{
  s64 offset;
  s64 pts;
  //usless
  //int trafNumber;
  //int trunNumber;
  //int sampleNumber;
} TfraEntry;


typedef struct Bento4StreamContext {
    int current_sample;
    int time_scale;
    int track_id;
    SidxBox *sidx;
	int trfaEntries_Count;
    TfraEntry *p_trfaEntry;
    void*   p_bento4Inner;//Bento4InnerContext
} Bento4StreamContext;


#ifdef __cplusplus
}
#endif


#endif // AP4_NEPTUNE_ADAPTERS
