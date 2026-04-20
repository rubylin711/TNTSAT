#ifndef AP4_FFMPEG_BYTE_STREAM_ADAPTER_H_
#define AP4_FFMPEG_BYTE_STREAM_ADAPTER_H_

extern "C" {
//#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
//#include "libswscale/swscale.h"
}

/*----------------------------------------------------------------------
|   includes
+---------------------------------------------------------------------*/
#include "Ap4ByteStream.h"

/*----------------------------------------------------------------------
|   NPT_InputStream_To_AP4_ByteStream_Adapter
+---------------------------------------------------------------------*/
class FFmpegByteStreamAdapter : public AP4_ByteStream
{
public:
  FFmpegByteStreamAdapter(){}
  FFmpegByteStreamAdapter(AVIOContext *pb);
  virtual ~FFmpegByteStreamAdapter();

  // AP4_ByteStream methods
  virtual AP4_Result ReadPartial(void*     buffer,
    AP4_Size  bytes_to_read,
    AP4_Size& bytes_read);
  virtual AP4_Result WritePartial(const void* buffer,
    AP4_Size    bytes_to_write,
    AP4_Size&   bytes_written);
  virtual AP4_Result Seek(AP4_Position position);
  virtual AP4_Result Tell(AP4_Position& position);
  virtual AP4_Result GetSize(AP4_LargeSize& size);
  virtual AP4_Result Next();

  // AP4_Referenceable methods
  virtual void AddReference();
  virtual void Release();

  int CheckSwitchStreams();

public:
  AVIOContext    *_pb;
private:

  int             _ReferenceCount;
};

#endif // AP4_NEPTUNE_ADAPTERS
