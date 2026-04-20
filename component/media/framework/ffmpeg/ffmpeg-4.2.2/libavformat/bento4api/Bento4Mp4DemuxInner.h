#ifndef AP4_BENTO4MP4DEMUX_INNER_H
#define AP4_BENTO4MP4DEMUX_INNER_H

#ifndef SAFE_DELETE
#define SAFE_DELETE(p) do{ if(p){delete(p);  (p)=NULL;} }while(0)
#endif

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
//#include "libswscale/swscale.h"
}

#include "Ap4.h"
#include "FFmpegByteStreamAdapter.h"
//#include "Bento4SampleDecrypter.h"
//#include "Bento4Mp4Track.h"

typedef struct Bento4InnerContext {
    AP4_SampleDescription *pSampleDescription;
	AP4_DataBuffer   *pPrefix;
	unsigned int     nalu_length_size;
    AP4_CencSampleDecrypter  *pSampleDecrypter;
    AP4_FragmentSampleTable  *m_SampleTable;
    char *cenc_base64_str;
    int  decrypt_index;//for multiple kids, choose m_oDecryptContext in playready
} Bento4InnerContext;

typedef struct {
  AP4_UI64 sample_count;
  AP4_UI64 duration;
  double   bitrate;
} MediaInfo;
void ScanMedia(AP4_Movie& movie, AP4_Track& track, AP4_ByteStream& stream, MediaInfo& info);
#endif // AP4_NEPTUNE_ADAPTERS
