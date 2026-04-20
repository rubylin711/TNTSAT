#ifndef AP4_BENTO4MP4DEMUX_H
#define AP4_BENTO4MP4DEMUX_H

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavformat/internal.h"
//#include "libavformat/isom.h"
#include "libavutil/base64.h"
#include <pthread.h>
}


#include "Ap4.h"
#include "Ap4Mp4AudioInfo.h"
#include "Bento4Mp4DemuxInner.h"
#include "FFmpegByteStreamAdapter.h"



struct DashExtensionInfo {
    int     type;
    AP4_DataBuffer data;
};



class Bento4Mp4Demux
{
 public:
   static int Probe(char* buffer, int len);

   Bento4Mp4Demux();
   virtual ~Bento4Mp4Demux();
   int ReadHeader(AVFormatContext *s);
   int ResetHeader(AVFormatContext *s);
   int ReadPacket(AVStream *st, AVIndexEntry *avsample, AVPacket *pkt);
   int buildIndexEntry(AVStream *st);
   int buildMulIndexEntry(AVFormatContext *s);
   int Seek(AP4_SI64 offset, AP4_SI32 type);
   int CreateFragmentSampleTable(
    AP4_Movie*                        movie,
    AP4_CencSampleInfoTable*&         cenc_info,
    AP4_UI32&                         cenc_algorithm_id,
    const AP4_UI08*&                  cenc_kid,
    AP4_Array<DashExtensionInfo>&     extensions,
    AP4_UI32&                         track_id,
    Bento4InnerContext*&              p_InnerContext,
    int                               select_track_id);
   int CreateFragmentMultiTrackSampleTable( AP4_Movie* movie, AVFormatContext *s);
   int ReadPacketFromPipe(AVStream *st, AVIndexEntry *avsample, AVPacket *pkt);
   int create_mediasession_for_track(AVFormatContext *s, AVStream *st, AP4_MemoryByteStream* mbs);
   int create_mediasession_by_content(AVFormatContext *s, AVStream *st);
   int get_sidx_add_buffer(AVStream *st);
   int get_tfra_add_stream(AVFormatContext *s);
   int GetNextStForMultiTracks();
   int CheckSwitchStreams(AVStream *st);
   void close(AVStream *st);
private:




private:

    FFmpegByteStreamAdapter _stream;
    AP4_File         *_pAP4File = NULL;
    int              m_isDash;
    int              m_drmStart;
    int              fragmented;//1 //1 is dash, 0 in normal mp4 file
    int              m_HaveMediaDtsOrigin;
    AP4_UI64         m_MediaDtsOrigin;
	char             *vmx_str;
    //AP4_UI32         m_MediaTimeScale;
    //Bento4Mp4Track   *_pMediaTrack;
};

#endif // AP4_NEPTUNE_ADAPTERS
