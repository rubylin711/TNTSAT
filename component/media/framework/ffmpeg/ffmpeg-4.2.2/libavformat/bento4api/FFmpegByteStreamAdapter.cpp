#include <stdio.h>
#include "FFmpegByteStreamAdapter.h"
#include <pthread.h>


FFmpegByteStreamAdapter::FFmpegByteStreamAdapter(AVIOContext *pb)
{
  _pb = pb;
}

FFmpegByteStreamAdapter::~FFmpegByteStreamAdapter()
{

}

AP4_Result
FFmpegByteStreamAdapter::ReadPartial(void*     buffer,
                                                       AP4_Size  bytes_to_read,
                                                       AP4_Size& bytes_read)
{
    AP4_Result ret = AP4_SUCCESS;
    bytes_read = avio_read(_pb, (unsigned char*)buffer, bytes_to_read);
    if (bytes_read == 0 || bytes_read  == AVERROR_EOF)
    {
        printf("ReadPartial failed\n");
        ret = AP4_FAILURE;
    }

    return ret;
}

/*----------------------------------------------------------------------
|   FFmpegByteStream::WritePartial
+---------------------------------------------------------------------*/
AP4_Result
FFmpegByteStreamAdapter::WritePartial(const void* ,
                                                        AP4_Size    ,
                                                        AP4_Size&   )
{
    return AP4_ERROR_NOT_SUPPORTED;
}

/*----------------------------------------------------------------------
|   FFmpegByteStream::Seek
+---------------------------------------------------------------------*/
AP4_Result
FFmpegByteStreamAdapter::Seek(AP4_Position position)
{
    AP4_Position cur_position = avio_seek(_pb, 0, SEEK_CUR);
    //printf("zx cur_position %lld, position %lld\n", cur_position, position);
    if(cur_position == position)
        return AP4_SUCCESS;
		
    avio_seek(_pb, position, SEEK_SET);
    return AP4_SUCCESS;
}



AP4_Result
FFmpegByteStreamAdapter::Next()
{
    //will switch to next segment by avio_read
    //must
    unsigned char* buffer[16] = {0};
    AP4_Size bytes_to_read = 4;
    AP4_Size bytes_read = avio_read(_pb, (unsigned char*)buffer, bytes_to_read);
    avio_seek(_pb, 0, SEEK_SET);
    if (bytes_read == 0 || bytes_read  == AVERROR_EOF)
    {
        printf("avio_read failed bytes_read %d\n", bytes_read);
        return  AP4_FAILURE;
    }
    //printf("FFmpegByteStreamAdapter::Next()\n");

    return AP4_SUCCESS;
}

int FFmpegByteStreamAdapter::CheckSwitchStreams()
{
    unsigned char* buffer[16] = {0};
    int ret = avio_read(_pb, (unsigned char*)buffer, -1);//notice, avio also add code to -1
    return ret;
}


/*----------------------------------------------------------------------
|   FFmpegByteStream::
+---------------------------------------------------------------------*/
AP4_Result
FFmpegByteStreamAdapter::Tell(AP4_Position& position)
{
    //printf("FFmpegByteStreamAdapter::Tell whence %d\n", SEEK_CUR);
    position = avio_seek(_pb, 0, SEEK_CUR);
    return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   FFmpegByteStream::GetSize
+---------------------------------------------------------------------*/
AP4_Result
FFmpegByteStreamAdapter::GetSize(AP4_LargeSize& size)
{
  size = avio_size(_pb);
  return AP4_SUCCESS;
}

/*----------------------------------------------------------------------
|   FFmpegByteStream::AddReference
+---------------------------------------------------------------------*/
void
FFmpegByteStreamAdapter::AddReference()
{

}

/*----------------------------------------------------------------------
|   FFmpegByteStream::Release
+---------------------------------------------------------------------*/
void
FFmpegByteStreamAdapter::Release()
{

}