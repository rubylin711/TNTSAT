
#include "Bento4Mp4Demux.h"
#include "Bento4Mp4DemuxApi.h"
#include <sys/time.h>

extern const AVCodecTag ff_codec_movaudio_tags[];
extern const AVCodecTag ff_codec_movvideo_tags[];
#ifdef MT_DRM_SUPPORT
#include "MTDrmApi.h"
extern char *g_playready_cert;
#endif

#define BENTO_PRINT  printf
#define  BENTO_ERROR printf

#define MAX_PKT_SIZE (3145728)

#ifdef DRM_SMP_ENABLE
#define MAX_VIDEO_FILTER_SIZE 4096
#endif


#ifdef VMX_OTT_SVP
#define MAX_VIDEO_FILTER_SIZE 4096
#endif

#ifdef MT_DRM_SUPPORT

static char PLAYREADY_SYSTEM_ID[32] = "\x9a\x04\xf0\x79\x98\x40\x42\x86\xab\x92\xe6\x5b\xe0\x88\x5f\x95";

int g_encrypt_type = 1; // 1 is playread, 2 widevine, 0 use default bento key to decrypt
#endif

 void ScanMedia(AP4_Movie& movie, AP4_Track& track, AP4_ByteStream& stream, MediaInfo& info)
{
  AP4_UI64 total_size = 0;
  AP4_UI64 total_duration = 0;

  AP4_UI64 position;
  //stream.Tell(position);
  stream.Seek(0);
  AP4_LinearReader reader(movie, &stream);
  reader.EnableTrack(track.GetId());

  info.sample_count = 0;

  AP4_Sample sample;
  if (movie.HasFragments()) {
    AP4_DataBuffer sample_data;
    for (unsigned int i = 0; ; i++) {
      BENTO_PRINT("HasFragments info.sample_count %d\n", info.sample_count);
      AP4_UI32 track_id = 0;
      AP4_Result result = reader.ReadNextSample(sample, sample_data, track_id);
      if (AP4_SUCCEEDED(result)) {
        total_size += sample.GetSize();
        total_duration += sample.GetDuration();
        ++info.sample_count;
        BENTO_PRINT("HasFragments 2 info.sample_count %d\n", info.sample_count);
      }
      else {
        break;
      }
    }
  }
  else {
    BENTO_PRINT("noFragments track.GetSampleCount() %d\n", track.GetSampleCount());
    info.sample_count = track.GetSampleCount();
    for (unsigned int i = 0; i < track.GetSampleCount(); i++) {
      if (AP4_SUCCEEDED(track.GetSample(i, sample))) {
        total_size += sample.GetSize();
      }
    }
    total_duration = track.GetMediaDuration();
  }
  info.duration = total_duration;

  double duration_ms = (double)AP4_ConvertTime(total_duration, track.GetMediaTimeScale(), 1000);
  if (duration_ms) {
    info.bitrate = 8.0*1000.0*(double)total_size / duration_ms;
  }
  else {
    info.bitrate = 0.0;
  }
}

static int build_index_entry(AP4_Track& track, AVStream *st)
{
    AP4_Sample     sample;
    AP4_Ordinal    index = 0;

    if (av_reallocp_array(&st->index_entries,
                          st->nb_index_entries + track.GetSampleCount(),
                          sizeof(*st->index_entries)) < 0) {
        st->nb_index_entries = 0;
        return -1;
    }
    st->index_entries_allocated_size = (st->nb_index_entries + track.GetSampleCount()) * sizeof(*st->index_entries);

    unsigned int distance = 0;
    while (AP4_SUCCEEDED(track.GetSample(index, sample))) {
        int keyframe = 0;

        if (sample.IsSync())
            keyframe = 1;

        if (keyframe)
             distance = 0;

        AVIndexEntry *e = &st->index_entries[st->nb_index_entries++];
        e->pos = sample.GetOffset();
        e->timestamp = sample.GetDts();
        e->size = sample.GetSize();
        e->min_distance = distance;
        e->flags = keyframe ? AVINDEX_KEYFRAME : 0;
        /*BENTO_PRINT("st->nb_index_entries %d, AVIndex stream %d, offset %"PRIx64", dts %"PRId64", "
                "size %d, distance %d, keyframe %d\n", st->nb_index_entries, st->index,
                e->pos, e->timestamp, e->size, distance, keyframe);*/
        distance++;
        index++;
    }

    return 0;
}

static AP4_Result
MakeAvcFramePrefix(AP4_SampleDescription* sdesc, AP4_DataBuffer& prefix, unsigned int& nalu_length_size)
{
    AP4_AvcSampleDescription* avc_desc = AP4_DYNAMIC_CAST(AP4_AvcSampleDescription, sdesc);
    if (avc_desc == NULL) {
        BENTO_ERROR( "ERROR: track does not contain an AVC stream\n");
        return AP4_FAILURE;
    }

    AP4_UI32 s_format = sdesc->GetFormat();
    if (s_format == AP4_SAMPLE_FORMAT_AVC4 ||
        s_format == AP4_SAMPLE_FORMAT_DVAV) //s_format == AP4_SAMPLE_FORMAT_AVC3 ||
    {
        // no need for a prefix, SPS/PPS NALs should be in the elementary stream already
        BENTO_ERROR( "ERROR: sdesc->GetFormat() %.4s\n", (char *)&s_format );
        return AP4_SUCCESS;
    }

    // make the SPS/PPS prefix
    nalu_length_size = avc_desc->GetNaluLengthSize();
    for (unsigned int i=0; i<avc_desc->GetSequenceParameters().ItemCount(); i++) {
        AP4_DataBuffer& buffer = avc_desc->GetSequenceParameters()[i];
        unsigned int prefix_size = prefix.GetDataSize();
        prefix.SetDataSize(prefix_size+4+buffer.GetDataSize());
        unsigned char* p = prefix.UseData()+prefix_size;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 1;
        AP4_CopyMemory(p, buffer.GetData(), buffer.GetDataSize());
    }
    for (unsigned int i=0; i<avc_desc->GetPictureParameters().ItemCount(); i++) {
        AP4_DataBuffer& buffer = avc_desc->GetPictureParameters()[i];
        unsigned int prefix_size = prefix.GetDataSize();
        prefix.SetDataSize(prefix_size+4+buffer.GetDataSize());
        unsigned char* p = prefix.UseData()+prefix_size;
        *p++ = 0;
        *p++ = 0;
        *p++ = 0;
        *p++ = 1;
        AP4_CopyMemory(p, buffer.GetData(), buffer.GetDataSize());
    }

    return AP4_SUCCESS;
}

static AP4_Result
MakeHevcFramePrefix(AP4_SampleDescription* sdesc, AP4_DataBuffer& prefix, unsigned int& nalu_length_size)
{
    AP4_HevcSampleDescription* hevc_desc = AP4_DYNAMIC_CAST(AP4_HevcSampleDescription, sdesc);
    if (hevc_desc == NULL) {
        BENTO_ERROR( "ERROR: track does not contain an HEVC stream\n");
        return AP4_FAILURE;
    }

    // extract the nalu length size
    nalu_length_size = hevc_desc->GetNaluLengthSize();

    // make the VPS/SPS/PPS prefix
    for (unsigned int i=0; i<hevc_desc->GetSequences().ItemCount(); i++) {
        const AP4_HvccAtom::Sequence& seq = hevc_desc->GetSequences()[i];
        if (seq.m_NaluType == AP4_HEVC_NALU_TYPE_VPS_NUT) {
            for (unsigned int j=0; j<seq.m_Nalus.ItemCount(); j++) {
                const AP4_DataBuffer& buffer = seq.m_Nalus[j];
                unsigned int prefix_size = prefix.GetDataSize();
                prefix.SetDataSize(prefix_size+4+buffer.GetDataSize());
                unsigned char* p = prefix.UseData()+prefix_size;
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
                *p++ = 1;
                AP4_CopyMemory(p, buffer.GetData(), buffer.GetDataSize());
            }
        }
    }

    for (unsigned int i=0; i<hevc_desc->GetSequences().ItemCount(); i++) {
        const AP4_HvccAtom::Sequence& seq = hevc_desc->GetSequences()[i];
        if (seq.m_NaluType == AP4_HEVC_NALU_TYPE_SPS_NUT) {
            for (unsigned int j=0; j<seq.m_Nalus.ItemCount(); j++) {
                const AP4_DataBuffer& buffer = seq.m_Nalus[j];
                unsigned int prefix_size = prefix.GetDataSize();
                prefix.SetDataSize(prefix_size+4+buffer.GetDataSize());
                unsigned char* p = prefix.UseData()+prefix_size;
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
                *p++ = 1;
                AP4_CopyMemory(p, buffer.GetData(), buffer.GetDataSize());
            }
        }
    }

    for (unsigned int i=0; i<hevc_desc->GetSequences().ItemCount(); i++) {
        const AP4_HvccAtom::Sequence& seq = hevc_desc->GetSequences()[i];
        if (seq.m_NaluType == AP4_HEVC_NALU_TYPE_PPS_NUT) {
            for (unsigned int j=0; j<seq.m_Nalus.ItemCount(); j++) {
                const AP4_DataBuffer& buffer = seq.m_Nalus[j];
                unsigned int prefix_size = prefix.GetDataSize();
                prefix.SetDataSize(prefix_size+4+buffer.GetDataSize());
                unsigned char* p = prefix.UseData()+prefix_size;
                *p++ = 0;
                *p++ = 0;
                *p++ = 0;
                *p++ = 1;
                AP4_CopyMemory(p, buffer.GetData(), buffer.GetDataSize());
            }
        }
    }

    return AP4_SUCCESS;
}

#ifdef VMX_OTT_SVP
int filterAvcSample(unsigned char* in_raw_data,
            int             in_raw_data_size,
            unsigned char*  prefix_data,
            int             prefix_data_size,
            int             nalu_length_size,
            unsigned char*  out_data,
            int*            out_data_size
)
{
    int frame_data_size = 0;
    unsigned char* frame_buffer = NULL;
    unsigned char* data = in_raw_data;
    int data_size = in_raw_data_size;
    int nalu_size = 0;
    // add a delimiter if we don't already have one
    bool have_access_unit_delimiter = (in_raw_data_size >  nalu_length_size) && ((in_raw_data[nalu_length_size] & 0x1F) == 9);//AP4_AVC_NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER is 9
    if (!have_access_unit_delimiter) {
        frame_buffer = out_data+frame_data_size;

        // start of access unit
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 0;
        frame_buffer[3] = 1;
        frame_buffer[4] = 9;    // NAL type = Access Unit Delimiter;
        frame_buffer[5] = 0xE0; // Slice types = ANY
        frame_data_size += 6;
    }

    bool prefix_added = false;
    while (data_size) {
        // sanity check
        if (data_size < nalu_length_size) break;

        // get the next NAL unit
        if (nalu_length_size == 1) {
            nalu_size = *data++;
            data_size--;
        } else if (nalu_length_size == 2) {
            //( ((AP4_UI16)bytes[0])<<8  ) |( ((AP4_UI16)bytes[1])     );
            nalu_size = ( ((int)*(data+0))<<8 ) | ( ((int)*(data+1)) );
            data      += 2;
            data_size -= 2;
        } else if (nalu_length_size == 4) {
        //( ((AP4_UI32)bytes[0])<<24 ) |( ((AP4_UI32)bytes[1])<<16 ) |( ((AP4_UI32)bytes[2])<<8  ) |( ((AP4_UI32)bytes[3])     );
            nalu_size = ( ((int)*(data+0))<<24 ) | ( ((int)*(data+1))<<16 ) | ( ((int)*(data+2))<<8 ) | ( ((int)*(data+3)) );
            data      += 4;
            data_size -= 4;
        } else {
            break;
        }
        if (nalu_size > data_size) break;

        // add the prefix if needed
        if (prefix_data_size && !prefix_added && !have_access_unit_delimiter) {
            frame_buffer = out_data+frame_data_size;
            memcpy(frame_buffer, prefix_data, prefix_data_size);
            frame_data_size += prefix_data_size;
            prefix_added = true;
        }

        // add a start code before the NAL unit
        //AP4_Size frame_data_size = frame_data.GetDataSize();
        //printf("%s %d frame_data_size %d\n", __func__, __LINE__, frame_data_size);
        //frame_data.SetDataSize(frame_data_size+3+nalu_size);
        //frame_buffer = frame_data.UseData()+frame_data_size;
        frame_buffer = out_data+frame_data_size;
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 1;
        memcpy(frame_buffer+3, data, nalu_size);
        frame_data_size += (3 + nalu_size);

        // add the prefix if needed
        if (prefix_data_size && !prefix_added ) {
            frame_buffer = out_data+frame_data_size;
            memcpy(frame_buffer, prefix_data, prefix_data_size);
            frame_data_size += prefix_data_size;
            prefix_added = true;
        }

        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    }

    if((frame_data_size + 16) < *out_data_size)
        *out_data_size = frame_data_size;//FF_INPUT_BUFFER_PADDING_SIZE is 16;
    else
        BENTO_ERROR("%s ERROR! filter data %d is bigger than malloc buf %d!!!\n",
            __func__, (frame_data_size + 16), *out_data_size);
    //if(prefix_added)
    //    printf("in_data_size %d, prefix.GetDataSize() %d, pkt->size %d, nalu_length_size %d\n",
    //        in_raw_data_size, prefix_data_size, *out_data_size, nalu_length_size);
    return *out_data_size;
}
#endif

#ifdef DRM_SMP_ENABLE
static int filterAvcSample(unsigned char* in_raw_data,
            int             in_raw_data_size,
            unsigned char*  prefix_data,
            int             prefix_data_size,
            int             nalu_length_size,
            unsigned char*  out_data,
            int*            out_data_size
)
{
    int frame_data_size = 0;
    unsigned char* frame_buffer = NULL;
    unsigned char* data = in_raw_data;
    int data_size = in_raw_data_size;
    int nalu_size = 0;
    // add a delimiter if we don't already have one
    bool have_access_unit_delimiter = (in_raw_data_size >  nalu_length_size) && ((in_raw_data[nalu_length_size] & 0x1F) == 9);//AP4_AVC_NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER is 9
    if (!have_access_unit_delimiter) {
        frame_buffer = out_data+frame_data_size;

        // start of access unit
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 0;
        frame_buffer[3] = 1;
        frame_buffer[4] = 9;    // NAL type = Access Unit Delimiter;
        frame_buffer[5] = 0xE0; // Slice types = ANY
        frame_data_size += 6;
    }

    bool prefix_added = false;
    while (data_size) {
        // sanity check
        if (data_size < nalu_length_size) break;

        // get the next NAL unit
        if (nalu_length_size == 1) {
            nalu_size = *data++;
            data_size--;
        } else if (nalu_length_size == 2) {
            //( ((AP4_UI16)bytes[0])<<8  ) |( ((AP4_UI16)bytes[1])     );
            nalu_size = ( ((int)*(data+0))<<8 ) | ( ((int)*(data+1)) );
            data      += 2;
            data_size -= 2;
        } else if (nalu_length_size == 4) {
        //( ((AP4_UI32)bytes[0])<<24 ) |( ((AP4_UI32)bytes[1])<<16 ) |( ((AP4_UI32)bytes[2])<<8  ) |( ((AP4_UI32)bytes[3])     );
            nalu_size = ( ((int)*(data+0))<<24 ) | ( ((int)*(data+1))<<16 ) | ( ((int)*(data+2))<<8 ) | ( ((int)*(data+3)) );
            data      += 4;
            data_size -= 4;
        } else {
            break;
        }
        if (nalu_size > data_size) break;

        // add the prefix if needed
        if (prefix_data_size && !prefix_added && !have_access_unit_delimiter) {
            frame_buffer = out_data+frame_data_size;
            memcpy(frame_buffer, prefix_data, prefix_data_size);
            frame_data_size += prefix_data_size;
            prefix_added = true;
        }

        // add a start code before the NAL unit
        //AP4_Size frame_data_size = frame_data.GetDataSize();
        //printf("%s %d frame_data_size %d\n", __func__, __LINE__, frame_data_size);
        //frame_data.SetDataSize(frame_data_size+3+nalu_size);
        //frame_buffer = frame_data.UseData()+frame_data_size;
        frame_buffer = out_data+frame_data_size;
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 1;
        memcpy(frame_buffer+3, data, nalu_size);
        frame_data_size += (3 + nalu_size);

        // add the prefix if needed
        if (prefix_data_size && !prefix_added ) {
            frame_buffer = out_data+frame_data_size;
            memcpy(frame_buffer, prefix_data, prefix_data_size);
            frame_data_size += prefix_data_size;
            prefix_added = true;
        }

        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    }

    if((frame_data_size + 16) < *out_data_size)
        *out_data_size = frame_data_size;//FF_INPUT_BUFFER_PADDING_SIZE is 16;
    else
        BENTO_ERROR("%s ERROR! filter data %d is bigger than malloc buf %d!!!\n",
            __func__, (frame_data_size + 16), *out_data_size);
    //if(prefix_added)
    //    printf("in_data_size %d, prefix.GetDataSize() %d, pkt->size %d, nalu_length_size %d\n",
    //        in_raw_data_size, prefix_data_size, *out_data_size, nalu_length_size);
    return *out_data_size;
}

static int filterHevcSample(unsigned char* in_raw_data,
            int             in_raw_data_size,
            unsigned char*  prefix_data,
            int             prefix_data_size,
            int             nalu_length_size,
            unsigned char*  out_data,
            int*            out_data_size
)
{
    int frame_data_size = 0;
    unsigned char* frame_buffer = NULL;
    unsigned char* data = in_raw_data;
    int data_size = in_raw_data_size;
    int nalu_size = 0;

    // detect if we have VPS/SPS/PPS and/or AUD NAL units already
    bool have_param_sets = false;
    bool have_access_unit_delimiter = false;
    while (data_size) {
        // sanity check
        if (data_size < nalu_length_size) break;

        // get the next NAL unit
        AP4_UI32 nalu_size;
        if (nalu_length_size == 1) {
            nalu_size = *data++;
            data_size--;
        } else if (nalu_length_size == 2) {
            //nalu_size = AP4_BytesToInt16BE(data);
            //( ((AP4_UI16)bytes[0])<<8  ) |( ((AP4_UI16)bytes[1])     );
            nalu_size = ( ((int)*(data+0))<<8 ) | ( ((int)*(data+1)) );
            data      += 2;
            data_size -= 2;
        } else if (nalu_length_size == 4) {
            //nalu_size = AP4_BytesToInt32BE(data);
            //( ((AP4_UI32)bytes[0])<<24 ) |( ((AP4_UI32)bytes[1])<<16 ) |( ((AP4_UI32)bytes[2])<<8  ) |( ((AP4_UI32)bytes[3])     );
            nalu_size = ( ((int)*(data+0))<<24 ) | ( ((int)*(data+1))<<16 ) | ( ((int)*(data+2))<<8 ) | ( ((int)*(data+3)) );
            data      += 4;
            data_size -= 4;
        } else {
            break;
        }
        if (nalu_size > data_size) break;

        unsigned int nal_unit_type = (data[0]>>1)&0x3F;
        if (nal_unit_type == 35) {//AP4_HEVC_NALU_TYPE_AUD_NUT is 35
            have_access_unit_delimiter = true;
        }
        //AP4_HEVC_NALU_TYPE_VPS_NUT 32, AP4_HEVC_NALU_TYPE_SPS_NUT 33, AP4_HEVC_NALU_TYPE_PPS_NUT 34
        if (nal_unit_type == 32 ||
            nal_unit_type == 33 ||
            nal_unit_type == 34) {
            have_param_sets = true;
            break;
        }

        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    }
    data = in_raw_data;
    data_size = in_raw_data_size;

    // allocate a buffer for the frame data
    frame_buffer = out_data+frame_data_size;

    // add a delimiter if we don't already have one
    if (data_size && !have_access_unit_delimiter) {
        // start of access unit
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 0;
        frame_buffer[3] = 1;
        frame_buffer[4] = AP4_HEVC_NALU_TYPE_AUD_NUT<<1;
        frame_buffer[5] = 1;
        frame_buffer[6] = 0x40; // pic_type = 2 (B,P,I)
        frame_data_size += 7;
    }

    // write the NAL units
    bool prefix_added = false;
    while (data_size) {
        // sanity check
        if (data_size < nalu_length_size) break;

        // get the next NAL unit
        AP4_UI32 nalu_size;
        if (nalu_length_size == 1) {
            nalu_size = *data++;
            data_size--;
        } else if (nalu_length_size == 2) {
            //nalu_size = AP4_BytesToInt16BE(data);
            //( ((AP4_UI16)bytes[0])<<8  ) |( ((AP4_UI16)bytes[1])     );
            nalu_size = ( ((int)*(data+0))<<8 ) | ( ((int)*(data+1)) );
            data      += 2;
            data_size -= 2;
        } else if (nalu_length_size == 4) {
            //nalu_size = AP4_BytesToInt32BE(data);
            //( ((AP4_UI32)bytes[0])<<24 ) |( ((AP4_UI32)bytes[1])<<16 ) |( ((AP4_UI32)bytes[2])<<8  ) |( ((AP4_UI32)bytes[3])     );
            nalu_size = ( ((int)*(data+0))<<24 ) | ( ((int)*(data+1))<<16 ) | ( ((int)*(data+2))<<8 ) | ( ((int)*(data+3)) );
            data      += 4;
            data_size -= 4;
        } else {
            break;
        }
        if (nalu_size > data_size) break;

        // add the prefix if needed
        if (!have_param_sets && !prefix_added && !have_access_unit_delimiter) {
            frame_buffer = out_data+frame_data_size;
            memcpy(frame_buffer, prefix_data, prefix_data_size);
            frame_data_size += prefix_data_size;
            prefix_added = true;
        }

        // add a start code before the NAL unit
        frame_buffer = out_data+frame_data_size;
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 1;
        memcpy(frame_buffer+3, data, nalu_size);
        frame_data_size += (3 + nalu_size);

        // add the prefix if needed
        if (!have_param_sets && !prefix_added) {
            frame_buffer = out_data+frame_data_size;
            memcpy(frame_buffer, prefix_data, prefix_data_size);
            frame_data_size += prefix_data_size;
            prefix_added = true;
        }

        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    }

    if((frame_data_size + 16) < *out_data_size)
        *out_data_size = frame_data_size;//FF_INPUT_BUFFER_PADDING_SIZE is 16;
    else
        BENTO_ERROR("%s ERROR! filter data %d is bigger than malloc buf %d!!!\n",
            __func__, (frame_data_size + 16), *out_data_size);

    //if(prefix_added)
        //printf("hevc in_data_size %d, prefix.GetDataSize() %d, pkt->size %d, nalu_length_size %d\n",
            //in_raw_data_size, prefix_data_size, *out_data_size, nalu_length_size);
    return *out_data_size;
}

#endif

int DRM_AddAAHeader(unsigned char *in_buf, int in_size, int aud_sample_rate, int aud_channels, unsigned char * out_buf)
{
    uint8_t aac_buf[7] = { 0xff, 0xf1, 0x40, 0x00, 0x00, 0x1f, 0xfc };
    unsigned int num_data_block = (unsigned int)in_size / 1024;
    uint16_t frame_Length;
    int i = 0;
    const int aac_sample_rates[16] = {
    96000, 88200, 64000, 48000, 44100, 32000,
    24000, 22050, 16000, 12000, 11025, 8000, 7350
    };

    for (i = 0; i < 16; i++)
    if (aud_sample_rate == aac_sample_rates[i]) {
        break;
    }

    //printf("\ni%d\n",i);
    frame_Length = in_size + 7;
    /* frame size over last 2 bits */
    aac_buf[2] |= ((i & 0xf) << 2);
    aac_buf[3] |= (aud_channels << 6);

    if (aud_channels > 3) {
    aac_buf[2] |= (aud_channels >> 2);
    }

    aac_buf[3] |= (frame_Length & 0x1800) >> 11; // the upper 2 bit
    /* frame size continued over full byte */
    aac_buf[4] = (frame_Length & 0x1FF8) >> 3; // the middle 8 bit
    /* frame size continued first 3 bits */
    aac_buf[5] |= (frame_Length & 0x7) << 5; // < span style='font-size:12px;font-style:normal;font-weight:normal;font-family:'  Courier New monospacecolorrgb  >//the last 3 bit
    aac_buf[6] |= num_data_block & 0x03;     //Set raw Data blocks.
    if(out_buf)
    {
        memcpy(out_buf, aac_buf, 7);
        memcpy(out_buf+7, in_buf, in_size);
    }
    //printf("[%s] --------- i=%d, aud_sample_rate =%d, channels =%d, frame_len=%d\n",__func__,i,aud_sample_rate,aud_channels,frame_Length);

    return in_size+7;
}


static int
CopyAvcSample(const AP4_DataBuffer& sample_data,
            AP4_DataBuffer&       prefix,
            unsigned int          nalu_length_size,
            AVPacket              *pkt)
{
    int ret = -1;
    const unsigned char* data      = sample_data.GetData();
    unsigned int         data_size = sample_data.GetDataSize();

    // allocate a buffer for the PES packet
    AP4_DataBuffer frame_data;
    unsigned char* frame_buffer = NULL;
    //BENTO_PRINT("nalu_length_size %d\n", nalu_length_size);

    // add a delimiter if we don't already have one
    bool have_access_unit_delimiter = (data_size >  nalu_length_size) && ((data[nalu_length_size] & 0x1F) == AP4_AVC_NAL_UNIT_TYPE_ACCESS_UNIT_DELIMITER);
    if (!have_access_unit_delimiter) {
        AP4_Size frame_data_size = frame_data.GetDataSize();
        frame_data.SetDataSize(frame_data_size+6);
        frame_buffer = frame_data.UseData()+frame_data_size;

        // start of access unit
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 0;
        frame_buffer[3] = 1;
        frame_buffer[4] = 9;    // NAL type = Access Unit Delimiter;
        frame_buffer[5] = 0xE0; // Slice types = ANY
    }

    // write the NAL units
    bool prefix_added = false;
    while (data_size) {
        // sanity check
        if (data_size < nalu_length_size) break;

        // get the next NAL unit
        AP4_UI32 nalu_size;
        if (nalu_length_size == 1) {
            nalu_size = *data++;
            data_size--;
        } else if (nalu_length_size == 2) {
            nalu_size = AP4_BytesToInt16BE(data);
            data      += 2;
            data_size -= 2;
        } else if (nalu_length_size == 4) {
            nalu_size = AP4_BytesToInt32BE(data);
            data      += 4;
            data_size -= 4;
        } else {
            break;
        }
        if (nalu_size > data_size) break;

        // add the prefix if needed
        if (prefix.GetDataSize() && !prefix_added && !have_access_unit_delimiter) {
            AP4_Size frame_data_size = frame_data.GetDataSize();
            frame_data.SetDataSize(frame_data_size+prefix.GetDataSize());
            frame_buffer = frame_data.UseData()+frame_data_size;
            AP4_CopyMemory(frame_buffer, prefix.GetData(), prefix.GetDataSize());
            prefix_added = true;
        }

        // add a start code before the NAL unit
        AP4_Size frame_data_size = frame_data.GetDataSize();
        frame_data.SetDataSize(frame_data_size+3+nalu_size);
        frame_buffer = frame_data.UseData()+frame_data_size;
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 1;
        AP4_CopyMemory(frame_buffer+3, data, nalu_size);

        // add the prefix if needed
        if (prefix.GetDataSize() && !prefix_added) {
            AP4_Size frame_data_size = frame_data.GetDataSize();
            frame_data.SetDataSize(frame_data_size+prefix.GetDataSize());
            frame_buffer = frame_data.UseData()+frame_data_size;
            AP4_CopyMemory(frame_buffer, prefix.GetData(), prefix.GetDataSize());
            prefix_added = true;
        }

        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    }

    //output->Write(frame_data.GetData(), frame_data.GetDataSize());
    if ((ret = av_new_packet(pkt, (frame_data.GetDataSize()+AV_INPUT_BUFFER_PADDING_SIZE))) < 0)
        return ret;
    memcpy(pkt->data, frame_data.GetData(), frame_data.GetDataSize());
    pkt->size = frame_data.GetDataSize();
    return ret;
}

static int
WriteAvcSample(const AP4_DataBuffer& sample_data,
               AP4_DataBuffer&       prefix,
               unsigned int          nalu_length_size,
               AVPacket              *pkt)
{
    int ret = -1;
    // allocate a buffer for the PES packet
    AP4_DataBuffer frame_data;
    frame_data.SetDataSize(6+prefix.GetDataSize());
    unsigned char* frame_buffer = frame_data.UseData();

    // start of access unit
    frame_buffer[0] = 0;
    frame_buffer[1] = 0;
    frame_buffer[2] = 0;
    frame_buffer[3] = 1;
    frame_buffer[4] = 9;    // NAL type = Access Unit Delimiter;
    frame_buffer[5] = 0xE0; // Slice types = ANY

    // copy the prefix
    AP4_CopyMemory(frame_buffer+6, prefix.GetData(), prefix.GetDataSize());

    // write the NAL units
    const unsigned char* data      = sample_data.GetData();
    unsigned int         data_size = sample_data.GetDataSize();

    while (data_size) {
        // sanity check
        if (data_size < nalu_length_size) break;

        // get the next NAL unit
        AP4_UI32 nalu_size;
        if (nalu_length_size == 1) {
            nalu_size = *data++;
            data_size--;
        }
        else if (nalu_length_size == 2) {
            nalu_size = AP4_BytesToInt16BE(data);
            data      += 2;
            data_size -= 2;
        }
        else if (nalu_length_size == 4) {
            nalu_size = AP4_BytesToInt32BE(data);
            data      += 4;
            data_size -= 4;
        }
        else {
            break;
        }
        if (nalu_size > data_size) break;

        // add a start code before the NAL unit
        unsigned int offset = frame_data.GetDataSize();
        frame_data.SetDataSize(offset+3+nalu_size);
        frame_buffer = frame_data.UseData()+offset;
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 1;
        AP4_CopyMemory(frame_buffer+3, data, nalu_size);

        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    }

    //output->Write(frame_data.GetData(), frame_data.GetDataSize());
    if ((ret = av_new_packet(pkt, (frame_data.GetDataSize()+AV_INPUT_BUFFER_PADDING_SIZE))) < 0)
        return ret;
    memcpy(pkt->data, frame_data.GetData(), frame_data.GetDataSize());
    pkt->size = frame_data.GetDataSize();
    return ret;
}

static int
CopyHevcSample(const AP4_DataBuffer& sample_data,
            AP4_DataBuffer&       prefix,
            unsigned int          nalu_length_size,
            AVPacket              *pkt)
{
    int ret = -1;
    const unsigned char* data      = sample_data.GetData();
    unsigned int         data_size = sample_data.GetDataSize();

    // detect if we have VPS/SPS/PPS and/or AUD NAL units already
    bool have_param_sets = false;
    bool have_access_unit_delimiter = false;
    while (data_size) {
        // sanity check
        if (data_size < nalu_length_size) break;

        // get the next NAL unit
        AP4_UI32 nalu_size;
        if (nalu_length_size == 1) {
            nalu_size = *data++;
            data_size--;
        } else if (nalu_length_size == 2) {
            nalu_size = AP4_BytesToInt16BE(data);
            data      += 2;
            data_size -= 2;
        } else if (nalu_length_size == 4) {
            nalu_size = AP4_BytesToInt32BE(data);
            data      += 4;
            data_size -= 4;
        } else {
            break;
        }
        if (nalu_size > data_size) break;

        unsigned int nal_unit_type = (data[0]>>1)&0x3F;
        if (nal_unit_type == AP4_HEVC_NALU_TYPE_AUD_NUT) {
            have_access_unit_delimiter = true;
        }
        if (nal_unit_type == AP4_HEVC_NALU_TYPE_VPS_NUT ||
            nal_unit_type == AP4_HEVC_NALU_TYPE_SPS_NUT ||
            nal_unit_type == AP4_HEVC_NALU_TYPE_PPS_NUT) {
            have_param_sets = true;
            break;
        }

        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    }
    data      = sample_data.GetData();
    data_size = sample_data.GetDataSize();

    // allocate a buffer for the frame data
    AP4_DataBuffer frame_data;
    unsigned char* frame_buffer = NULL;

    // add a delimiter if we don't already have one
    if (data_size && !have_access_unit_delimiter) {
        AP4_Size frame_data_size = frame_data.GetDataSize();
        frame_data.SetDataSize(frame_data_size+7);
        frame_buffer = frame_data.UseData()+frame_data_size;

        // start of access unit
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 0;
        frame_buffer[3] = 1;
        frame_buffer[4] = AP4_HEVC_NALU_TYPE_AUD_NUT<<1;
        frame_buffer[5] = 1;
        frame_buffer[6] = 0x40; // pic_type = 2 (B,P,I)
    }

    // write the NAL units
    bool prefix_added = false;
    while (data_size) {
        // sanity check
        if (data_size < nalu_length_size) break;

        // get the next NAL unit
        AP4_UI32 nalu_size;
        if (nalu_length_size == 1) {
            nalu_size = *data++;
            data_size--;
        } else if (nalu_length_size == 2) {
            nalu_size = AP4_BytesToInt16BE(data);
            data      += 2;
            data_size -= 2;
        } else if (nalu_length_size == 4) {
            nalu_size = AP4_BytesToInt32BE(data);
            data      += 4;
            data_size -= 4;
        } else {
            break;
        }
        if (nalu_size > data_size) break;

        // add the prefix if needed
        if (!have_param_sets && !prefix_added && !have_access_unit_delimiter) {
            AP4_Size frame_data_size = frame_data.GetDataSize();
            frame_data.SetDataSize(frame_data_size+prefix.GetDataSize());
            frame_buffer = frame_data.UseData()+frame_data_size;
            AP4_CopyMemory(frame_buffer, prefix.GetData(), prefix.GetDataSize());
            prefix_added = true;
        }

        // add a start code before the NAL unit
        AP4_Size frame_data_size = frame_data.GetDataSize();
        frame_data.SetDataSize(frame_data_size+3+nalu_size);
        frame_buffer = frame_data.UseData()+frame_data_size;
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 1;
        AP4_CopyMemory(frame_buffer+3, data, nalu_size);

        // add the prefix if needed
        if (!have_param_sets && !prefix_added) {
            frame_data_size = frame_data.GetDataSize();
            frame_data.SetDataSize(frame_data_size+prefix.GetDataSize());
            frame_buffer = frame_data.UseData()+frame_data_size;
            AP4_CopyMemory(frame_buffer, prefix.GetData(), prefix.GetDataSize());
            prefix_added = true;
        }

        // move to the next NAL unit
        data      += nalu_size;
        data_size -= nalu_size;
    }

    //output->Write(frame_data.GetData(), frame_data.GetDataSize());
    if ((ret = av_new_packet(pkt, (frame_data.GetDataSize()+AV_INPUT_BUFFER_PADDING_SIZE))) < 0)
                return ret;
    memcpy(pkt->data, frame_data.GetData(), frame_data.GetDataSize());
    pkt->size = frame_data.GetDataSize();

    return ret;
}

static int
CopyDashAvcSample(const AP4_DataBuffer& sample_data,
            AP4_DataBuffer&       prefix,
            int                   flags,
            AVPacket              *pkt)
{
    int ret = -1;
    const unsigned char* data      = sample_data.GetData();
    unsigned int         data_size = sample_data.GetDataSize();

    // allocate a buffer for the PES packet
    AP4_DataBuffer frame_data;
    unsigned char* frame_buffer = NULL;


    AP4_Size frame_data_size = data_size;
    if(flags & AVINDEX_KEYFRAME)
    {
        frame_data_size += prefix.GetDataSize() + 4;
    }
    frame_data.SetDataSize(frame_data_size);
    frame_buffer = frame_data.UseData();
    if(flags & AVINDEX_KEYFRAME)
    {
        frame_buffer[0] = 0;
        frame_buffer[1] = 0;
        frame_buffer[2] = 0;
        frame_buffer[3] = 1;
        frame_buffer = frame_data.UseData() + 4;
        AP4_CopyMemory(frame_buffer, prefix.GetData(), prefix.GetDataSize());
        frame_buffer = frame_data.UseData() + prefix.GetDataSize() + 4;
    }
    AP4_CopyMemory(frame_buffer, data, data_size);

    //output->Write(frame_data.GetData(), frame_data.GetDataSize());
    if ((ret = av_new_packet(pkt, (frame_data.GetDataSize()+AV_INPUT_BUFFER_PADDING_SIZE))) < 0)
        return ret;
    memcpy(pkt->data, frame_data.GetData(), frame_data.GetDataSize());
    pkt->size = frame_data.GetDataSize();
    return ret;
}

AVCodecID
AP4_GetFFMPGE_Codec(AP4_UI32 format)
{
    switch (format) {
        case AP4_SAMPLE_FORMAT_MP4A: return AV_CODEC_ID_AAC; //return "MPEG-4 Audio";
        case AP4_ATOM_TYPE_ENCA    : return AV_CODEC_ID_AAC; //?? return aac ;
        case AP4_SAMPLE_FORMAT_MP4V: return AV_CODEC_ID_MPEG4; //return "MPEG-4 Video";
        case AP4_SAMPLE_FORMAT_ALAC: return AV_CODEC_ID_ALAC; //return "Apple Lossless Audio";
        case AP4_SAMPLE_FORMAT_AVC1: return AV_CODEC_ID_H264; //return "H.264";
        case AP4_SAMPLE_FORMAT_AVC2: return AV_CODEC_ID_H264; //return "H.264";
        case AP4_SAMPLE_FORMAT_AVC3: return AV_CODEC_ID_H264; //return "H.264";
        case AP4_SAMPLE_FORMAT_AVC4: return AV_CODEC_ID_H264; //return "H.264";
        case AP4_SAMPLE_FORMAT_DVAV: return AV_CODEC_ID_AC3; //return "Dolby Vision (H.264)";
        case AP4_SAMPLE_FORMAT_DVA1: return AV_CODEC_ID_AC3; // "Dolby Vision (H.264)";
        case AP4_SAMPLE_FORMAT_HEV1: return AV_CODEC_ID_HEVC; //return "H.265";
        case AP4_SAMPLE_FORMAT_HVC1: return AV_CODEC_ID_HEVC; //return "H.265";
        case AP4_SAMPLE_FORMAT_DVH1: return AV_CODEC_ID_EAC3; //"Dolby Vision (H.265)";
        case AP4_SAMPLE_FORMAT_DVHE: return AV_CODEC_ID_EAC3; //"Dolby Vision (H.265)";
        case AP4_SAMPLE_FORMAT_OVC1: return AV_CODEC_ID_VC1; //return "VC-1";
        case AP4_SAMPLE_FORMAT_OWMA: return AV_CODEC_ID_WMAV1; //return "WMA";
        case AP4_SAMPLE_FORMAT_AC_3: return AV_CODEC_ID_AC3; //return "Dolby Digital (AC-3)";
        case AP4_SAMPLE_FORMAT_EC_3: return AV_CODEC_ID_EAC3; //"Dolby Digital Plus (Enhanced AC-3)";
        case AP4_SAMPLE_FORMAT_DTSC: return AV_CODEC_ID_DTS; //return "DTS";
        case AP4_SAMPLE_FORMAT_DTSH: return AV_CODEC_ID_DTS; //return "DTS-HD";
        case AP4_SAMPLE_FORMAT_DTSL: return AV_CODEC_ID_DTS; //return "DTS-HD Lossless";
        case AP4_SAMPLE_FORMAT_DTSE: return AV_CODEC_ID_DTS; //return "DTS Low Bitrate";
        case AP4_SAMPLE_FORMAT_RAW_: return AV_CODEC_ID_PCM_S16LE; //???  return "Uncompressed Audio";
        case AP4_SAMPLE_FORMAT_S263: return AV_CODEC_ID_H263; //return "H.263";
        case AP4_SAMPLE_FORMAT_SAMR: return AV_CODEC_ID_AMR_NB; //return "Narrowband AMR";
        case AP4_SAMPLE_FORMAT_SAWB: return AV_CODEC_ID_AMR_WB; //return "Wideband AMR";
        case AP4_SAMPLE_FORMAT_SAWP: return AV_CODEC_ID_AMR_NB; //return "Extended AMR";
        case AP4_SAMPLE_FORMAT_VP8:  return AV_CODEC_ID_VP8;
        case AP4_SAMPLE_FORMAT_VP9:  return AV_CODEC_ID_VP9;

        case AP4_SAMPLE_FORMAT_AVCP:  //"Advanced Video Coding Parameters";
        case AP4_SAMPLE_FORMAT_DRAC:  //return "Dirac";
        case AP4_SAMPLE_FORMAT_DRA1:  //return "DRA Audio";
        case AP4_SAMPLE_FORMAT_G726:  //return "G726";
        case AP4_SAMPLE_FORMAT_MJP2:  //return "Motion JPEG 2000";
        case AP4_SAMPLE_FORMAT_OKSD:  //return "OMA Keys";
        case AP4_SAMPLE_FORMAT_TWOS:  //return "Uncompressed 16-bit Audio";
        case AP4_SAMPLE_FORMAT_TEXT:  //return "Textual Metadata";
        case AP4_SAMPLE_FORMAT_RTP_:  //return "RTP Hints";
        case AP4_SAMPLE_FORMAT_SEVC:  //return "EVRC Voice";
        case AP4_SAMPLE_FORMAT_SQCP:  //return "13K Voice";
        case AP4_SAMPLE_FORMAT_SRTP:  //return "SRTP Hints";
        case AP4_SAMPLE_FORMAT_SSMV:  //return "SMV Voice";
        case AP4_SAMPLE_FORMAT_VC_1:  //return "SMPTE VC-1";
        case AP4_SAMPLE_FORMAT_TX3G:  //return "Timed Text";
        case AP4_SAMPLE_FORMAT_XML_:  //return "XML Metadata";
        case AP4_SAMPLE_FORMAT_STPP:  //return "Timed Text";
        case AP4_SAMPLE_FORMAT_MP4S:  //return "MPEG-4 Systems";
        default:
            BENTO_ERROR("unsupport bento4 codec format %.4s\n", (char *)&format);
            return AV_CODEC_ID_NONE;
    }
}



unsigned int codec_get_tag(const AVCodecTag *tags, enum AVCodecID id)
{
    while (tags->id != AV_CODEC_ID_NONE) {
        if (tags->id == id)
            return tags->tag;
        tags++;
    }
    return 0;
}

static void get_tenc_kid(AP4_ProtectedSampleDescription* psdesc, Bento4InnerContext *p_InnerContext)
{
    AP4_ProtectionSchemeInfo* scheme_info = psdesc->GetSchemeInfo();
    p_InnerContext->cenc_base64_str = NULL;
    if(scheme_info)
    {
        AP4_ContainerAtom* schi = scheme_info->GetSchiAtom();
        if(schi)
        {
            AP4_TencAtom* tenc = AP4_DYNAMIC_CAST(AP4_TencAtom, schi->FindChild("tenc"));
            if(tenc)
            {
                const char* cenc_kid = (const char* )tenc->GetDefaultKid();
                volatile int len = 0;   //for bug 124938, avoid g++ bug
                if(cenc_kid)
                   len = strlen(cenc_kid);
                if(len < 16 || cenc_kid == NULL)        // for tsscan
                {
                    //no cenc_kid
                    return;
                }
                int ii = 0;
                /*if(0)
                {
                    for(ii = 0; ii < len; ii++)
                    {
                        printf("%.2x ", *(cenc_kid+ ii));
                    }
                    printf("\n");
                }*/

                char *endian_str = (char *)av_malloc(len + 1);
                //printf("len %d\n", len);
                endian_str[len] = '\0';
                if(len % 4 )
                    BENTO_ERROR("ERROR: cenc_kid lenght error, %d!", len);
                //<KID value= "base64-encoded guid"
                *(endian_str ) = (*(cenc_kid + 3))&0xFF;
                *(endian_str + 1) = (*(cenc_kid + 2))&0xFF;
                *(endian_str + 2) = (*(cenc_kid + 1))&0xFF;
                *(endian_str + 3) = (*(cenc_kid  ))&0xFF;
                *(endian_str + 4) = (*(cenc_kid + 5))&0xFF;
                *(endian_str + 5) = (*(cenc_kid + 4))&0xFF;
                *(endian_str + 6) = (*(cenc_kid + 7))&0xFF;
                *(endian_str + 7) = (*(cenc_kid + 6))&0xFF;
                memcpy(endian_str+8, cenc_kid+8, 8);

                #if 0
                /*for(ii = 0; ii < len; ii++)
                {
                    printf("%.2x ", *(endian_str+ ii) & 0xFF);
                }
                printf("\n");*/
                int cenc_base64_len = AV_BASE64_SIZE(strlen(endian_str));
                p_InnerContext->cenc_base64_str = (char *)av_malloc(cenc_base64_len);
                av_base64_encode(p_InnerContext->cenc_base64_str, cenc_base64_len,
                    (const uint8_t *)endian_str, strlen(endian_str));
                if(p_InnerContext->cenc_base64_str)
                    BENTO_PRINT("base64 str: %s\n", p_InnerContext->cenc_base64_str);
                av_free(endian_str);
                #endif
                p_InnerContext->cenc_base64_str = endian_str;
            }
        }
    }

    return;
}


static int get_desc_from_track(AP4_Track *track, AVStream *st)
{
    unsigned int desc_index=0;
    AP4_SampleDescription* sample_desc;
    Bento4StreamContext *sc = (Bento4StreamContext *)st->priv_data;
    Bento4InnerContext *p_InnerContext = (Bento4InnerContext *)sc->p_bento4Inner;
    //if(p_InnerContext->pSampleDescription);//track->GetSampleDescription, dont need delete!
    //    SAFE_DELETE(p_InnerContext->pSampleDescription);
    if(p_InnerContext->pPrefix);
        SAFE_DELETE(p_InnerContext->pPrefix);
    if(p_InnerContext->pSampleDecrypter);
        SAFE_DELETE(p_InnerContext->pSampleDecrypter);
    if(p_InnerContext->m_SampleTable);
        SAFE_DELETE(p_InnerContext->m_SampleTable);
    if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
    {
        for (unsigned int desc_index=0;
            AP4_SampleDescription* sample_desc = track->GetSampleDescription(desc_index);
            desc_index++) {
            if(desc_index >= 1)
            {
                BENTO_PRINT("  Sample Description more than 1 don support!!!\n");
                break;
            }
            st->codecpar->codec_id = AP4_GetFFMPGE_Codec(sample_desc->GetFormat());
            st->codecpar->codec_tag = codec_get_tag(ff_codec_movaudio_tags, st->codecpar->codec_id);
            BENTO_PRINT("audio format %d st->codecpar->codec_id 0x%x\n", sample_desc->GetFormat(), st->codecpar->codec_id);
            AP4_AudioSampleDescription* audio_desc =
            AP4_DYNAMIC_CAST(AP4_AudioSampleDescription, sample_desc);

            if (audio_desc == NULL)
            {
                AP4_ProtectedSampleDescription* psdesc = AP4_DYNAMIC_CAST(AP4_ProtectedSampleDescription, sample_desc);
                if (psdesc)
                {
                    sample_desc = psdesc->GetOriginalSampleDescription();
                    st->codecpar->codec_id = AP4_GetFFMPGE_Codec(psdesc->GetOriginalFormat());
                    audio_desc = AP4_DYNAMIC_CAST(AP4_AudioSampleDescription, sample_desc);

                    get_tenc_kid(psdesc, p_InnerContext);
                }
            }

            if (audio_desc) {
                // Audio sample description
                st->codecpar->channels    = audio_desc->GetChannelCount();
                st->codecpar->sample_rate = audio_desc->GetSampleRate();
                st->codecpar->bits_per_coded_sample = audio_desc->GetSampleSize();
                if(sample_desc->GetType() == AP4_SampleDescription::TYPE_MPEG)
                {
                    AP4_MpegSampleDescription* mpeg_desc = AP4_DYNAMIC_CAST(AP4_MpegSampleDescription, sample_desc);
                    if (mpeg_desc->GetObjectTypeId() == AP4_OTI_MPEG4_AUDIO          ||
                        mpeg_desc->GetObjectTypeId() == AP4_OTI_MPEG2_AAC_AUDIO_LC   ||
                        mpeg_desc->GetObjectTypeId() == AP4_OTI_MPEG2_AAC_AUDIO_MAIN)
                    {
                        AP4_MpegAudioSampleDescription* mpeg_audio_desc = AP4_DYNAMIC_CAST(AP4_MpegAudioSampleDescription, mpeg_desc);
                        if (mpeg_audio_desc)
                        {
                            const AP4_DataBuffer& dsi = mpeg_audio_desc->GetDecoderInfo();
                            AP4_Mp4AudioDecoderConfig dec_config;
                            dec_config.Parse(dsi.GetData(), dsi.GetDataSize());
                            st->codecpar->channels = dec_config.m_ChannelCount;
                            st->codecpar->sample_rate =  dec_config.m_SamplingFrequency;
                        }
                    }
                }
            }
            BENTO_PRINT("audio channels %d, sample_rate %d, bits_per_coded_sample %d\n",
                st->codecpar->channels, st->codecpar->sample_rate, st->codecpar->bits_per_coded_sample);
        }
    }
    else if(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
    {
        for (desc_index=0;
                sample_desc = track->GetSampleDescription(desc_index);
                desc_index++)
        {
            if(desc_index >= 1)
            {
                BENTO_PRINT("  Sample Description more than 1 don support!!!\n");
                break;
            }
            AP4_VideoSampleDescription* video_desc =
                    AP4_DYNAMIC_CAST(AP4_VideoSampleDescription, sample_desc);
            if (video_desc == NULL)
            {
                AP4_ProtectedSampleDescription* psdesc = AP4_DYNAMIC_CAST(AP4_ProtectedSampleDescription, sample_desc);
                if (psdesc)
                {
                    sample_desc = psdesc->GetOriginalSampleDescription();
                    st->codecpar->codec_id = AP4_GetFFMPGE_Codec(psdesc->GetOriginalFormat());
                    video_desc = AP4_DYNAMIC_CAST(AP4_VideoSampleDescription, sample_desc);

                    get_tenc_kid(psdesc, p_InnerContext);
                }
            }

            if (video_desc)
            {
                // Video sample description
                st->codecpar->codec_id = AP4_GetFFMPGE_Codec(sample_desc->GetFormat());
                p_InnerContext->pSampleDescription = sample_desc;
                st->codecpar->width = video_desc->GetWidth();
                st->codecpar->height = video_desc->GetHeight();
                st->codecpar->codec_tag = codec_get_tag(ff_codec_movvideo_tags, st->codecpar->codec_id);
                if (st->codecpar->codec_id == AV_CODEC_ID_H264)
                {
                    p_InnerContext->pPrefix = new AP4_DataBuffer;
                    if (AP4_FAILED(MakeAvcFramePrefix(p_InnerContext->pSampleDescription, *(p_InnerContext->pPrefix), p_InnerContext->nalu_length_size)))
                    {
                        BENTO_PRINT("%s: Error MakeAvcFramePrefix failed!\n", __func__);
                        return -1;
                    }
                    BENTO_PRINT("zx h264 prefix %d\n", p_InnerContext->pPrefix->GetDataSize());
                }
                else if (st->codecpar->codec_id == AV_CODEC_ID_HEVC)
                {
                    p_InnerContext->pPrefix = new AP4_DataBuffer;
                    if (AP4_FAILED(MakeHevcFramePrefix(p_InnerContext->pSampleDescription, *(p_InnerContext->pPrefix), p_InnerContext->nalu_length_size)))
                    {
                        BENTO_PRINT("%s: Error MakeAvcFramePrefix failed!\n", __func__);
                        return -1;
                    }
                    BENTO_PRINT("zx hevc prefix %d\n", p_InnerContext->pPrefix->GetDataSize());
                }
                int fps = 0;
                if(track->GetSampleCount()> 0 && track->GetMediaDuration()> 0)
                    fps = (int)( (float)track->GetSampleCount()/
                                                  ((float)track->GetMediaDuration()/(float)track->GetMediaTimeScale()) + 0.5);
                BENTO_PRINT("st 0x%x fps %d, width %d height %d\n", st, fps, st->codecpar->width, st->codecpar->height);
                st->avg_frame_rate.num = fps;
                st->avg_frame_rate.den = 1;
                st->r_frame_rate = st->avg_frame_rate;
            }
        }
    }

    return 0;
}

static void avplay_get_drm_type(int *type)
{
    char *env = getenv ("MT_DRM_TYPE");
    if(env)
    {
        *type = atoi(env);
    }
#ifdef MT_DRM_SUPPORT
    else
    {
        *type = DRM_TYPE_PLAYREADY;
    }
#endif
}


int Bento4Mp4Demux::create_mediasession_by_content(AVFormatContext *s, AVStream *st)
{
#ifdef MT_DRM_SUPPORT
    char *p_content_protection = s->p_content_protection;
    MTDRM_CONFIG  cfg = {0};
    int drm_type = 0;
    MTDRM_PROTECT_INFO pro = {0};
    mt_char * path = getenv ("MT_PLAYREADY_PATH");
    if(path)
    {
        cfg.cert_path = path;
    }
    else
    {
        cfg.cert_path = "./";
    }

    avplay_get_drm_type(&drm_type);
    MTDrm_GetDrmInstance((MTDRM_TYPE)drm_type, &cfg, &st->eDrmIndex);

    //printf("%s %d zxtest s->eDrmIndex 0x%x\n", __func__, __LINE__, st->eDrmIndex);

    pro.init_data = (mt_u8 *)p_content_protection;
    pro.init_data_len = strlen(p_content_protection);

    if(MTDrm_SetProtectInfo(&pro, st->eDrmIndex) == MTDRM_SUCCESS)
    {
        m_drmStart = TRUE;
    }
    else
        return -1;
#endif

    return 0;

}

int Bento4Mp4Demux::create_mediasession_for_track(AVFormatContext *s, AVStream *st, AP4_MemoryByteStream* mbs)
{
#ifdef MT_DRM_SUPPORT
    int i=0;
    int drm_type = 0;
    MTDRM_CONFIG  cfg = {0};
    MTDRM_PROTECT_INFO pro = {0};

    mt_char * path = getenv ("MT_PLAYREADY_PATH");
    if(path)
    {
        cfg.cert_path = path;
    }
    else
    {
        cfg.cert_path = "./";
    }

    avplay_get_drm_type(&drm_type);
    MTDrm_GetDrmInstance((MTDRM_TYPE)drm_type, &cfg, &st->eDrmIndex);

    //printf("%s %d zxtest st->eDrmIndex 0x%x\n", __func__, __LINE__, st->eDrmIndex);

    for(i=0; i< s->nb_streams; i++)
    {
        AVStream *st = (AVStream *)s->streams[i];
        Bento4StreamContext *sc = (Bento4StreamContext *)st->priv_data;
        Bento4InnerContext *p_InnerContext = (Bento4InnerContext *)sc->p_bento4Inner;
        if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO || st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            if(p_InnerContext->cenc_base64_str)
            {
                p_InnerContext->decrypt_index = i;
                pro.kids[pro.kid_count++] = p_InnerContext->cenc_base64_str;
                if(pro.kid_count >= MT_DRM_MAX_KEYS) break;
            }
        }
    }
    /*if(cenc0)
        BENTO_PRINT("cenc0 %s\n", cenc0);
    if(cenc1)
        BENTO_PRINT("cenc1 %s\n", cenc1);*/

    pro.init_data = (mt_u8 *)mbs->GetData();
    pro.init_data_len = mbs->GetDataSize();
    pro.is_pssh = TRUE;
#ifdef VMX_OTT_SVP

    int tmp_len = 0;
    char *p_start = (char *)pro.init_data;
    while(tmp_len<mbs->GetDataSize() && *p_start != '<'){
        tmp_len++;
        p_start++;
    }

    printf("%s pro.init_data_len len %d tmp_len %d\n",__func__, pro.init_data_len, tmp_len);
    int vmx_len = 0;
    if(p_start) {
        p_start = strstr(p_start, "CDATA[");
        p_start = p_start + 6;
        if(p_start) {
            char *p_end = strstr(p_start, "]");
            vmx_len = p_end - p_start;
        vmx_str = (char *)av_mallocz(vmx_len + 1);
        memcpy(vmx_str, p_start, vmx_len);
        printf("zx vmx_str %s\n", vmx_str);  //for vmx
        }
    }
    pro.is_pssh = TRUE;
    m_drmStart = TRUE;
#else

    //printf("zx MTDrm_SetProtectInfo pro.init_data_len %d\n", pro.init_data_len);
    if(MTDrm_SetProtectInfo(&pro, st->eDrmIndex) == MTDRM_SUCCESS)
    {
        m_drmStart = TRUE;
    }
    else
    {

        return -1;
    }

#if 0
    if(is_one_drm_session)
    {
        MTDrm_SetProtectInfo((unsigned char *)mbs->GetData(), mbs->GetDataSize(), NULL, NULL, 0,1);
        Drm_StartMediaSession((char*)NULL);
    }
    else
    {
        //create two session and two pssh
        char *p_ascii_xml = NULL;
        int xml_start = get_xml_acsii_str((char *)mbs->GetData(), mbs->GetDataSize(), &p_ascii_xml);
        int ascii_xml_len = strlen(p_ascii_xml);
        char *p_one_kid_str = (char *)av_mallocz(ascii_xml_len);
        printf("len %d, %s\n",strlen(p_ascii_xml),p_ascii_xml);
        char *p_kids_start = strstr(p_ascii_xml, "<KIDS");
        char *p_kids_end   = strstr(p_ascii_xml, "</KIDS");
        p_kids_end += 7;
        char *p_cur_kid_start = NULL;
        char *p_cur_kid_end = NULL;
        if(p_kids_start && p_kids_end)
        {
            for(i=0; i< s->nb_streams; i++)
            {
                AVStream *st = (AVStream *)s->streams[i];
                Bento4StreamContext *sc = (Bento4StreamContext *)st->priv_data;
                Bento4InnerContext *p_InnerContext = (Bento4InnerContext *)sc->p_bento4Inner;
                p_cur_kid_start = strstr(p_kids_start, p_InnerContext->cenc_base64_str);
                if(p_cur_kid_start)
                {
                    p_cur_kid_end = p_cur_kid_start + strlen(p_InnerContext->cenc_base64_str);
                    while(strncmp(p_cur_kid_start, "<KID", 4) != 0)
                        p_cur_kid_start--;
                    while(strncmp(p_cur_kid_end, "</KID", 5) != 0)
                        p_cur_kid_end++;
                    p_cur_kid_end += 6;

                    char cur_kid[128] = {0};
                    memcpy(cur_kid, p_cur_kid_start, p_cur_kid_end-p_cur_kid_start);
                    BENTO_PRINT("cur kid str:%s\n", cur_kid);

                    memset(p_one_kid_str, 0 , ascii_xml_len);
                    memcpy(p_one_kid_str, p_ascii_xml, p_kids_start-p_ascii_xml);
                    memcpy(p_one_kid_str+strlen(p_one_kid_str), cur_kid, strlen(cur_kid));
                    memcpy(p_one_kid_str+strlen(p_one_kid_str), p_kids_end, p_kids_start+strlen(p_ascii_xml)-p_kids_end);
                    BENTO_PRINT("cur kid str:%s\n", p_one_kid_str);

                    int one_kid_pssh_len = xml_start + strlen(p_one_kid_str) * 2;
                    char *p_new_pssh_str = (char *)av_mallocz(one_kid_pssh_len);
                    char *p_mbs = (char *)mbs->GetData();
                    //write 4 byte len
                    //AV_WB32  AV_WL32
                    int diff_len = (strlen(p_ascii_xml) - strlen(p_one_kid_str)) * 2;
                    BENTO_PRINT("ascii_xml_len %d, p_one_kid_str %d, diff_len %d\n",
                        strlen(p_ascii_xml), strlen(p_one_kid_str), diff_len);
                    char * p = p_new_pssh_str;
                    AV_WB32(p, one_kid_pssh_len);
                    p += 4;
                    memcpy(p, p_mbs+4, 24); //pssh 4 byte, version 4 byte, system id 16 byte
                    p += 24;
                    p_mbs += 28;
                    u32 playready_len = AV_RB32(p_mbs);
                    //printf("playready_len 11 0x%x\n", playready_len);
                    playready_len -= diff_len;
                    //printf("new playready_len 0x%x\n", playready_len);
                    AV_WB32(p, playready_len);
                    p += 4;
                    p_mbs += 4;
                    AV_WL32(p, playready_len);
                    p += 4;
                    p_mbs += 4;
                    memcpy(p, p_mbs, xml_start-36 - 2);
                    p = p_new_pssh_str + xml_start - 2;
                    AV_WL16(p, strlen(p_one_kid_str)*2);
                    p += 2;
                    *(p++) = *p_one_kid_str;

                    //final unicode xml
                    for(int stri=1; stri< strlen(p_one_kid_str); stri++)
                    {
                        *(p++) = '\0';
                        *(p++) = *(p_one_kid_str + stri);
                    }

                    for(int aa=0; aa< one_kid_pssh_len; aa++)
                        BENTO_PRINT("%x %c \n",*(p_new_pssh_str+ aa)&0xff, *(p_new_pssh_str+ aa) &0xff);
                    //MTDrm_SetProtectInfo((unsigned char*)p_new_pssh_str, one_kid_pssh_len, 0,1);
                    //Drm_StartMediaSession((char*)NULL);
                }
            }
        }

        if(p_ascii_xml)
            av_free(p_ascii_xml);
    }
#endif

#endif

#endif
    return 0;
}

int Bento4Mp4Demux::get_tfra_add_stream(AVFormatContext *s)
{
    AP4_ContainerAtom*       m_Mfra = NULL;
    AP4_Position             start_offset = 0;
    AP4_LargeSize            stream_size  = 0;
    int result = -1;
    int s_i = 0;
    Bento4StreamContext *sc = NULL;
    AVStream *st = NULL;
    _stream.Tell(start_offset);
    _stream.GetSize(stream_size);
    //BENTO_PRINT("%s: start_offset %lld, stream_size %lld", __func__, start_offset, stream_size);
    if(start_offset >= (stream_size - 4))
    {
        if (AP4_FAILED(_stream.Next()))
        {
            BENTO_ERROR("zx11 _stream.Next() failed, maybe eof!\n");
            result = -1;
            goto cleanup;
        }
        _stream.Tell(start_offset);
    }

    unsigned char mfro[12];
    result = _stream.Seek(stream_size-12);
    if (AP4_SUCCEEDED(result)) {
        result = _stream.Read(mfro, 12);
    }
    if (AP4_SUCCEEDED(result) && mfro[0] == 'm' && mfro[1] == 'f' && mfro[2] == 'r' && mfro[3] == 'o') {
        AP4_UI32 mfra_size = AP4_BytesToUInt32BE(&mfro[8]);
        if ((AP4_LargeSize)mfra_size < stream_size) {
            result = _stream.Seek(stream_size-mfra_size);
            if (AP4_SUCCEEDED(result)) {
                AP4_Atom* mfra = NULL;
                //AP4_LargeSize available = mfra_size;
                AP4_DefaultAtomFactory::Instance_.CreateAtomFromStream(_stream, mfra);
                m_Mfra = AP4_DYNAMIC_CAST(AP4_ContainerAtom, mfra);
            }
        }
    }

    //printf("zx tmp %s\n", mfro);
    if (m_Mfra == NULL) {
        BENTO_PRINT("no m_Mfra");
        result = -1;
        goto cleanup;
    }

    if(m_Mfra)
    {
        for (s_i = 0; s_i < s->nb_streams; s_i++) {
            st = s->streams[s_i];
            sc = (Bento4StreamContext *)st->priv_data;
            // find the tfra index for this track
            AP4_TfraAtom* tfra = NULL;
            for (AP4_List<AP4_Atom>::Item* item = m_Mfra->GetChildren().FirstItem();
                                           item;
                                           item = item->GetNext()) {
                if (item->GetData()->GetType() == AP4_ATOM_TYPE_TFRA) {
                    AP4_TfraAtom* tfra_ = (AP4_TfraAtom*)item->GetData();
                    if (tfra_->GetTrackId() == sc->track_id) {
                        tfra = tfra_;
                        break;
                    }
                }
            }
            if (tfra == NULL) {
                return AP4_ERROR_NOT_SUPPORTED;
            }

            AP4_Array<AP4_TfraAtom::Entry>& entries = tfra->GetEntries();
            sc->trfaEntries_Count = (int)entries.ItemCount();
            BENTO_PRINT("avstream %d, trfaEntries_Count %d\n",
                    s_i,  sc->trfaEntries_Count);
            sc->p_trfaEntry = (TfraEntry *)av_mallocz (sizeof (TfraEntry) * sc->trfaEntries_Count);
            for (int i=0; i<sc->trfaEntries_Count; i++) {
                sc->p_trfaEntry[i].offset = entries[i].m_MoofOffset;
                sc->p_trfaEntry[i].pts = entries[i].m_Time*AV_TIME_BASE/sc->time_scale;
                //BENTO_PRINT("offset %lld, pst %lld\n",sc->p_trfaEntry[i].offset, sc->p_trfaEntry[i].pts);
            }
            if(st->duration == 0)
                st->duration = sc->p_trfaEntry[sc->trfaEntries_Count-1].pts;
            BENTO_PRINT("st %d st->duration %lld\n",st->codecpar->codec_type, st->duration);
        }

        m_isDash = FALSE;
        delete m_Mfra;
    }


cleanup:

    _stream.Seek(start_offset);
    return result;

}

int Bento4Mp4Demux::get_sidx_add_buffer(AVStream *st)
{
    AP4_Atom*                atom = NULL;
    AP4_SidxAtom*            sidx = NULL;
    AP4_Position             midx_offset = 0;
    AP4_Position             start_offset = 0;
    AP4_LargeSize            stream_size  = 0;
    Bento4StreamContext      *sc = (Bento4StreamContext *)st->priv_data;
    int result = -1;
    _stream.Tell(start_offset);
    _stream.GetSize(stream_size);
    //BENTO_PRINT("start_offset %lld, stream_size %lld", start_offset, stream_size);
    if(start_offset >= (stream_size - 4))
    {
        if (AP4_FAILED(_stream.Next()))
        {
            BENTO_ERROR("zx11 _stream.Next() failed, maybe eof!\n");
            result = -1;
            goto cleanup;
        }
        _stream.Tell(start_offset);
    }
    for (;;) {
        result = _stream.Tell(midx_offset);
        if (AP4_FAILED(result)) {
            BENTO_PRINT("tell failed (%d)", result);
            result = -1;
            goto cleanup;
        }
        result = AP4_DefaultAtomFactory::Instance_.CreateAtomFromStream(_stream,
                                                                        atom);
        if (AP4_FAILED(result)) {
            if (result == AP4_ERROR_EOS) {
                 BENTO_ERROR("zx11 AP4_ERROR_EOS\n");
                result = AP4_ERROR_EOS;
            } else {
                sc->sidx = NULL;
                BENTO_PRINT("no sidx atom \n");
            }
            goto cleanup;
        }

        //BENTO_PRINT("zx11 atmo %d, AP4_ATOM_TYPE_MOOF %d\n", atom->GetType(), AP4_ATOM_TYPE_MOOF);
        if (atom->GetType() == AP4_ATOM_TYPE_SIDX)
        {
            sidx = AP4_DYNAMIC_CAST(AP4_SidxAtom, atom);
            break;
        }

        if (atom->GetType() == AP4_ATOM_TYPE_MOOF)
        {
            sc->sidx = NULL;
            BENTO_PRINT("no sidx atom \n");
            goto cleanup;
        }

        delete atom;
        atom = NULL;
    }

    if(sidx)
    {
        _stream.Tell(midx_offset);
        int i = 0;
        SidxBox *p_sidx = (SidxBox *)av_mallocz(sizeof(SidxBox));
        p_sidx->timescale = sidx->GetTimeScale();
        p_sidx->earliest_pts = sidx->GetEarliestPresentationTime();
        p_sidx->first_offset= sidx->GetFirstOffset();
        p_sidx->entries_count = sidx->GetReferences().ItemCount();
        p_sidx->sidx_base_offset = midx_offset;
        p_sidx->entries = (SidxBoxEntry *)av_mallocz (sizeof (SidxBoxEntry) * p_sidx->entries_count);

        BENTO_PRINT("midx_offset %lld, timescale %d, earliest_pts %lld, first_offset %lld\n",
            midx_offset, p_sidx->timescale, p_sidx->earliest_pts, p_sidx->first_offset);
        BENTO_PRINT("p_sidx 0x%x, p_sidx->entries 0x%x\n", p_sidx, p_sidx->entries);
        int cumulative_entry_size = p_sidx->first_offset;
        int cumulative_pts = p_sidx->earliest_pts*AV_TIME_BASE/p_sidx->timescale;
        for (i=0; i < sidx->GetReferences().ItemCount(); i++) {
            AP4_SidxAtom::Reference& sidx_ref = sidx->UseReferences()[i];
            /*BENTO_PRINT("entry %04d reference_type=%d, referenced_size=%u, subsegment_duration=%u, starts_with_SAP=%d, SAP_type=%d, SAP_delta_time=%d\n",
                i, sidx_ref.m_ReferenceType,
                 sidx_ref.m_ReferencedSize,
                 sidx_ref.m_SubsegmentDuration,
                 sidx_ref.m_StartsWithSap,
                 sidx_ref.m_SapType,
                 sidx_ref.m_SapDeltaTime);*/
            SidxBoxEntry *entry = &p_sidx->entries[i];
            entry->offset = (s64)cumulative_entry_size;
            entry->pts = (s64)cumulative_pts;
            entry->size = sidx_ref.m_ReferencedSize;
            entry->duration = ((float)sidx_ref.m_SubsegmentDuration/(float)p_sidx->timescale)*AV_TIME_BASE;
            cumulative_entry_size += sidx_ref.m_ReferencedSize;
            cumulative_pts += entry->duration;
            //BENTO_PRINT("entry %04d offset=%lld, pts=%lld, size=%u, duration=%lld\n",
            //    i, entry->offset, entry->pts, entry->size, entry->duration);
        }

        result = 0;
        sc->sidx = p_sidx;
    }


cleanup:
    if (atom != NULL) {
        delete atom;
    }

    _stream.Seek(start_offset);
    return result;

}

int Bento4Mp4Demux::CheckSwitchStreams(AVStream *st)
{
    //for dash profile_isoff_ondemand
    return _stream.CheckSwitchStreams();
}

extern int g_dash_playmode;
int Bento4Mp4Demux::ReadHeader(AVFormatContext *s)
{
  BENTO_PRINT("%s %d enter\n", __func__, __LINE__);
  _stream._pb = s->pb;
  _pAP4File = new AP4_File(_stream, AP4_DefaultAtomFactory::Instance_, true);
  bool show_samples = false;
  bool show_sample_data = false;
  bool verbose = true;
  bool fast = false;
  bool has_sidx = false;
  int  ret = 0;

  s->duration = 0;
  AP4_Movie* movie = _pAP4File->GetMovie();
  //AP4_Track *cur_track = movie->GetTrack(AP4_Track::TYPE_VIDEO);
  //BENTO_PRINT("zx test 11 video track 0x%x\n", cur_track);
  //if (movie) {
  //    ShowMovieInfo(*movie);
  //}
#ifdef MT_DRM_SUPPORT
  av_drm_set_index_info(-1, -1);//reset drm eDrmIndex

  AP4_MoovAtom* moov = movie->GetMoovAtom();
  AP4_PsshAtom* psshAtom = AP4_DYNAMIC_CAST(AP4_PsshAtom, moov->FindChild("pssh"));
  AP4_UuidAtom* uuidAtom = NULL;
  AP4_MemoryByteStream* mbs = NULL;
  if (psshAtom)
  {
    BENTO_PRINT("Found psshAtom by MoovAtom\n");
    mbs = new AP4_MemoryByteStream();
    psshAtom->Write(*mbs);//will release in create_mediasession_for_track
  }
  else
  {
    //try uuid
      uuidAtom = AP4_DYNAMIC_CAST(AP4_UuidAtom, moov->FindChild("uuid"));
      if (uuidAtom)
      {
        BENTO_PRINT("Found uuidAtom by MoovAtom\n");
        mbs = new AP4_MemoryByteStream();
        uuidAtom->Write(*mbs);//will release in create_mediasession_for_track
        char system_id[17]= {0};
        char * p_uuid = (char *)mbs->GetData();
        memcpy(system_id, p_uuid+28, 16);

        if(strncmp(PLAYREADY_SYSTEM_ID, system_id,16) != 0)
        {
            BENTO_PRINT("system_id is not playread!\n");
            mbs->Release();
            uuidAtom = NULL;

            for(int stri=1; stri< 16; stri++)
            {
                BENTO_PRINT("0x%x \n", system_id[stri]);
            }
        }
      }
  }
#endif

  AP4_List<AP4_Track>& tracks = movie->GetTracks();
  BENTO_PRINT("s 0x%x Found %d Tracks\n", s, tracks.ItemCount());
  //ShowTracks(*movie, tracks, _stream, show_samples, show_sample_data, verbose, fast);
  int index = 0;
  for (AP4_List<AP4_Track>::Item* track_item = tracks.FirstItem();
         track_item;
         track_item = track_item->GetNext(), ++index) {
        AP4_Track *track = track_item->GetData();
        AVStream *st = avformat_new_stream(s, NULL);
        if (!st) return AVERROR(ENOMEM);
        st->index = index;

        Bento4StreamContext *sc = (Bento4StreamContext *)av_mallocz(sizeof(Bento4StreamContext));
        if (!sc) return AVERROR(ENOMEM);
        st->priv_data = sc;
        sc->time_scale = track->GetMediaTimeScale();
        sc->track_id = track->GetId();
        st->sample_aspect_ratio.den = sc->time_scale; /* number of frame */
        st->sample_aspect_ratio.num = 1;
        st->avg_frame_rate.den = sc->time_scale; /* number of frame */
        st->avg_frame_rate.num = 1;
        st->time_base.den = sc->time_scale; /* number of frame */
        st->time_base.num = 1;
        Bento4InnerContext *p_tmp_innerContex = (Bento4InnerContext *)av_mallocz(sizeof(Bento4InnerContext));
        sc->p_bento4Inner = (void *)p_tmp_innerContex;
        //BENTO_PRINT("zx track id %d sc current_sample %d st->codecpar->time_base.den %d\n", track->GetId(), sc->current_sample, st->codecpar->time_base.den);
        switch (track->GetType()) {
            case AP4_Track::TYPE_AUDIO:     st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;     break;
            case AP4_Track::TYPE_VIDEO:     st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;     break;
            case AP4_Track::TYPE_SUBTITLES: st->codecpar->codec_type = AVMEDIA_TYPE_SUBTITLE;  break;
            case AP4_Track::TYPE_HINT:      BENTO_PRINT("Hint, NO AV TYPE\n");      break;
            case AP4_Track::TYPE_SYSTEM:    BENTO_PRINT("System, NO AV TYPE\n");    break;
            case AP4_Track::TYPE_JPEG:      BENTO_PRINT("JPEG, NO AV TYPE\n");      break;
            case AP4_Track::TYPE_TEXT:      BENTO_PRINT("Text, NO AV TYPE\n");      break;
            default: {
                BENTO_PRINT("NO AV TYPE!");
                break;
            }
        }

        MediaInfo media_info;
        if(!movie->HasFragments())
        {
            ScanMedia(*movie, *track, _stream, media_info);
            st->nb_frames = media_info.sample_count;
            st->duration = media_info.duration;//dont do change (AP4_UI32)AP4_ConvertTime(media_info.duration, track->GetMediaTimeScale(), 1000000);//us
        }
        else
        {
            st->nb_frames = 0;
            st->duration = 0;
            m_isDash = TRUE;
        }


        //BENTO_PRINT("st->nb_frames %lld, st->duration %lld\n", st->nb_frames, st->duration);
        BENTO_PRINT("codec_type %d, movie 0x%x, cur_track 0x%x, SampleDescriptionCount %d,  st->index %d\n",
            st->codecpar->codec_type, movie, track, track->GetSampleDescriptionCount(), st->index);
        if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            int ret = get_desc_from_track(track, st);

            if(ret != 0)
            {
                BENTO_ERROR("get desc from bento4 track failed!\n");
                return ret;
            }
        }
        else if(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            int ret = get_desc_from_track(track, st);

            if(ret != 0)
            {
                BENTO_ERROR("get desc from bento4 track failed!\n");
                return ret;
            }
            st->codecpar->extradata_size = 0;
            st->codecpar->extradata = NULL;

        }

        if(g_dash_playmode != 2)
            get_sidx_add_buffer(st);

        if(!movie->HasFragments())
            build_index_entry(*track, st);

        if(sc->sidx)
            has_sidx = true;

#ifdef MT_DRM_SUPPORT
        if(g_encrypt_type)
        {
            if ((psshAtom || uuidAtom) && mbs)
            {
                ret = create_mediasession_for_track(s, st, mbs);
            }
            else if (s->p_content_protection)
            {
                ret = create_mediasession_by_content(s, st);
            }

            if(ret != 0)
            {
                BENTO_ERROR("%s %d MTDrm_SetProtectInfo failed\n",__func__, __LINE__);
                return AVERROR(EINVAL);
            }
        }
#endif

    }

#ifdef MT_DRM_SUPPORT
    if (s->p_content_protection == NULL)
    {
        int i = 0;
        int a_index = -1;
        int v_index = -1;
        for(i=0; i< s->nb_streams; i++)
        {
            AVStream *st = (AVStream *)s->streams[i];
            if(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
                v_index = st->eDrmIndex;
            else if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
                a_index = st->eDrmIndex;
        }
        BENTO_PRINT("%s v_index %d a_index %d\n",__func__, v_index, a_index);
        av_drm_set_index_info(v_index, a_index);
    }
    if(mbs)
        mbs->Release();
#endif

    BENTO_PRINT("%s %d\n",__func__, __LINE__);
    if(has_sidx == false && g_dash_playmode != 2)
    {
        get_tfra_add_stream(s);
    }

    if(m_isDash == FALSE && s->duration == 0) {
        int s_i = 0;
        for (s_i = 0; s_i < s->nb_streams; s_i++) {
            AVStream *st = s->streams[s_i];
            if(s->duration < st->duration) {
                s->duration = st->duration;
            }
        }
    }

    BENTO_PRINT("%s %d end end s->duration %lld\n",__func__, __LINE__, s->duration);
  return 0;
}

int Bento4Mp4Demux::ResetHeader(AVFormatContext *s)
{
    AVStream*             st;
    Bento4StreamContext*  sc;
    AP4_Track*            cur_track;
    int                   s_i = 0;
    bool                  has_sidx = false;

    if(_pAP4File)
    {
        delete _pAP4File;
        BENTO_PRINT("%s %d delete _pAP4File , _stream._pb 0x%x opaque 0x%x\n",
            __func__, __LINE__, _stream._pb, _stream._pb->opaque);
    }
    BENTO_PRINT("%s %d new AP4_File \n", __func__, __LINE__);
    _stream.Seek(0);
    _pAP4File = new AP4_File(_stream, AP4_DefaultAtomFactory::Instance_, true);
    //BENTO_PRINT("%s %d new AP4_File end thread %u\n", __func__, __LINE__, (unsigned int)pthread_self());
    AP4_Movie* movie = _pAP4File->GetMovie();//maybe anonther init.mp4, parse it again

    for (s_i = 0; s_i < s->nb_streams; s_i++)
    {
        st = s->streams[s_i];
        sc = (Bento4StreamContext *)st->priv_data;
        if(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            cur_track = movie->GetTrack(AP4_Track::TYPE_VIDEO);
            int ret = get_desc_from_track(cur_track, st);

            if(ret != 0)
            {
                BENTO_ERROR("get desc from bento4 track failed!\n");
                return ret;
            }
        }
        else if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            cur_track = movie->GetTrack(AP4_Track::TYPE_AUDIO);
            int ret = get_desc_from_track(cur_track, st);

            if(ret != 0)
            {
                BENTO_ERROR("get desc from bento4 track failed!\n");
                return ret;
            }
        }
        else if(st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
            cur_track = movie->GetTrack(AP4_Track::TYPE_SUBTITLES);

        sc->time_scale = cur_track->GetMediaTimeScale();
        sc->track_id = cur_track->GetId();
        st->sample_aspect_ratio.den = sc->time_scale; /* number of frame */
        st->sample_aspect_ratio.num = 1;
        st->avg_frame_rate.den = sc->time_scale; /* number of frame */
        st->avg_frame_rate.num = 1;
        st->time_base.den = sc->time_scale; /* number of frame */
        st->time_base.num = 1;

        if(st->index_entries)
            av_freep(&st->index_entries);
        st->index_entries = NULL;
        st->nb_index_entries = 0;
        sc->current_sample = 0;
        //sidx and tfar still in Bento4StreamContext sc, dont changed

        if(sc->sidx)
        {
            if(sc->sidx->entries)
            {
                av_free(sc->sidx->entries);
                sc->sidx->entries = NULL;
            }
            av_free(sc->sidx);
            sc->sidx = NULL;
            get_sidx_add_buffer(st);
            has_sidx = true;
        }
    }

    if(has_sidx == false && sc->p_trfaEntry)
    {
        av_free(sc->p_trfaEntry);
        sc->p_trfaEntry = NULL;
        get_tfra_add_stream(s);
    }
    s->dash_select_track = -1;

    return 0;
}

int Bento4Mp4Demux::GetNextStForMultiTracks()
{
    AP4_Atom*                atom = NULL;
    AP4_ContainerAtom*       moof = NULL;
    AP4_Position             moof_payload_offset = 0;
    AP4_Position             mdat_payload_offset = 0;
    AP4_Position             start_offset = 0;
    AP4_LargeSize            stream_size  = 0;
    AP4_MovieFragment*       fragment = NULL;
    AP4_Array<AP4_UI32>      track_ids;
    AP4_UI32                 track_id = 0xFFFF;
    int result = 0;
    _stream.Tell(start_offset);
    _stream.GetSize(stream_size);
    //BENTO_PRINT("GetNextStForMultiTracks: start_offset %lld, stream_size %lld\n", start_offset, stream_size);
    if(start_offset >= (stream_size - 4))
    {
        if (AP4_FAILED(_stream.Next()))
        {
            BENTO_ERROR("zx11 _stream.Next() failed, maybe eof!\n");
            result = -1;
            goto cleanup;
        }
        _stream.Tell(start_offset);
    }
    for (;;) {
        result = _stream.Tell(moof_payload_offset);
        if (AP4_FAILED(result)) {
            BENTO_ERROR("tell failed (%d)", result);
            result = -1;
            goto cleanup;
        }
        result = AP4_DefaultAtomFactory::Instance_.CreateAtomFromStream(_stream,
                                                                        atom);
        if (AP4_FAILED(result)) {
            if (result == AP4_ERROR_EOS) {
                 BENTO_ERROR("zx11 AP4_ERROR_EOS\n");
                result = AP4_ERROR_EOS;
            } else {
                BENTO_ERROR("failed to parse atom while looking for moof "\
                                  "(%d)\n", result);
            }
            goto cleanup;
        }

        if (atom->GetType() == AP4_ATOM_TYPE_MOOF)
            break;

        delete atom;
        atom = NULL;
    }


    // record the stream position:
    // we'll assume that the mdat starts just after and the header is 8 bytes
    result = _stream.Tell(mdat_payload_offset);
    if (AP4_FAILED(result)) {
        BENTO_ERROR("tell failed (%d)\n", result);
        goto cleanup;
    }
    mdat_payload_offset += 8; // header
    moof = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);

    // now create a movie fragment
    fragment = new AP4_MovieFragment(moof);
    atom = NULL; // ownership has been transferred

    // get the ID of the first track
    // (we assume there is only one track in each fragment, since we don't
    // support multiplexed fragments)
    fragment->GetTrackIds(track_ids);
    if (track_ids.ItemCount() < 1) {
        BENTO_ERROR("no track found in fragment\n");
        goto cleanup;
    }
    if (track_ids.ItemCount() > 1) {
        BENTO_PRINT("more than one track IDs found in fragment, selecting the first one only\n");
    }
    track_id = track_ids[0];
    //BENTO_PRINT("zx11 sampletable selected track ID = %d\n", track_id);

cleanup:
    _stream.Seek(start_offset);

    if (atom != NULL) {
        delete atom;
    }
    if(fragment)
        delete fragment;

    if(result == -1 )
        return result;

    return track_id;
}

int Bento4Mp4Demux::CreateFragmentSampleTable(
    AP4_Movie*                        movie,
    AP4_CencSampleInfoTable*&         cenc_info,
    AP4_UI32&                         cenc_algorithm_id,
    const AP4_UI08*&                  cenc_kid,
    AP4_Array<DashExtensionInfo>&     extensions,
    AP4_UI32&                         track_id,
    Bento4InnerContext*&              p_InnerContext,
    int                               select_track_id)
{
    AP4_Atom*                atom = NULL;
    AP4_ContainerAtom*       moof = NULL;
    AP4_Position             moof_payload_offset = 0;
    AP4_Position             mdat_payload_offset = 0;
    AP4_Position             start_offset = 0;
    AP4_LargeSize            stream_size  = 0;
    AP4_MovieFragment*       fragment = NULL;
    AP4_Array<AP4_UI32>      track_ids;
    unsigned int sample_description_index = 0;
    AP4_SampleDescription*   sdesc = NULL;
    AP4_Track*               track = NULL;
    AP4_FragmentSampleTable* sample_table = NULL;
    int result = -1;
    AP4_LargeSize chunk_next_moof_size = -1;
    AP4_UI32      type = 0;

    // parse until we get to the moof
    _stream.Tell(start_offset);
    _stream.GetSize(stream_size);
    //BENTO_PRINT("start_offset %lld, stream_size %lld, _stream._pb->is_chunked %d\n",
    //    start_offset, stream_size, _stream._pb->is_chunked);
    if((_stream._pb->is_chunked == 0) && start_offset >= (stream_size - 4)) {
        if (AP4_FAILED(_stream.Next()))
        {
            BENTO_ERROR("zx11 _stream.Next() failed, maybe eof!\n");
            result = -1;
            goto cleanup;
        }
        _stream.Tell(start_offset);
    }
    else
    {
        AP4_UI08 input_buffer[4] = {0};
        AP4_Size to_feed = 4;
        result = _stream.ReadPartial(input_buffer, 4, to_feed);
        if(result == AP4_FAILURE)
            BENTO_ERROR("%s ReadPartial failed!\n",__func__);
        //BENTO_PRINT("%s _stream._pb 0x%x, is_chunked_reset %d\n",
        //    __func__, _stream._pb, _stream._pb->is_chunked_reset);
        if(_stream._pb->is_chunked_reset) {
            avio_seek(_stream._pb, 0, SEEK_SET);
            _stream._pb->is_chunked_reset = 0;
            chunk_next_moof_size = -1;
        } else {
            for (;;) {
                if(av_player_is_exit()) {
                    BENTO_PRINT("%s %d exit\n", __func__, __LINE__);
                    return -1;
                }
                chunk_next_moof_size = AP4_BytesToUInt32BE(input_buffer);
                result = _stream.ReadPartial(input_buffer, 4, to_feed);
                if(result == AP4_FAILURE) {
                    BENTO_PRINT("%s %d exit\n", __func__, __LINE__);
                    return -1;
                }
                type = AP4_BytesToUInt32BE(input_buffer);
                AP4_Position cur;
                _stream.Tell(cur);
                //BENTO_PRINT("%s zx tmp %lld, cur ps %lld, stream_size %lld,  %.4s\n",
                //    __func__, chunk_next_moof_size, cur, stream_size, (char*)&type);
                if(type == AP4_ATOM_TYPE_MOOF){
                    _stream.Seek(start_offset);
                    chunk_next_moof_size += 4;
                    break;
                } else {
                    _stream.Seek(start_offset + chunk_next_moof_size );
                    _stream.Tell(start_offset);
                    result = _stream.ReadPartial(input_buffer, 4, to_feed);
                    if(result == AP4_FAILURE) {
                        BENTO_PRINT("%s %d exit\n", __func__, __LINE__);
                        return -1;
                    }
                }
            }
            /* //size 4 byte, moof 4 byte
            printf("zx11 %x %x %x %x %x %x %x %x\n", input_buffer[0], input_buffer[1],
                input_buffer[2], input_buffer[3], input_buffer[4], input_buffer[5],
                input_buffer[6], input_buffer[7]);*/
        }
    }

    for (;;) {
        result = _stream.Tell(moof_payload_offset);
        if (AP4_FAILED(result)) {
            BENTO_ERROR("tell failed (%d)", result);
            result = -1;
            goto cleanup;
        }
        if(chunk_next_moof_size != -1)
            result = AP4_DefaultAtomFactory::Instance_.CreateAtomFromStream(_stream,
                                                                        chunk_next_moof_size, atom);
        else
            result = AP4_DefaultAtomFactory::Instance_.CreateAtomFromStream(_stream,
                                                                        atom);
        if (AP4_FAILED(result)) {
            if (result == AP4_ERROR_EOS) {
                 BENTO_ERROR("zx11 AP4_ERROR_EOS\n");
                result = AP4_ERROR_EOS;
            } else {
                BENTO_ERROR("failed to parse atom while looking for moof "\
                                  "(%d)\n", result);
            }
            goto cleanup;
        }

        AP4_UI32 type = atom->GetType();
        //BENTO_PRINT("atom->GetType() %.4s\n", (char *)&type);
        if (atom->GetType() == AP4_ATOM_TYPE_MOOF)
        {
            start_offset = moof_payload_offset;
            break;
        }

        if (atom->GetType() == AP4_ATOM_TYPE_PRFT ||
            atom->GetType() == AP4_ATOM_TYPE_PSSH) {
            AP4_MemoryByteStream* mbs = new AP4_MemoryByteStream();
            atom->Write(*mbs);
            DashExtensionInfo tmp;
            tmp.type = atom->GetType();
            tmp.data.SetData(mbs->GetData(),mbs->GetDataSize());
            extensions.Append(tmp);
            mbs->Release();
        }
        delete atom;
        atom = NULL;
    }


    // record the stream position:
    // we'll assume that the mdat starts just after and the header is 8 bytes
    result = _stream.Tell(mdat_payload_offset);
    if (AP4_FAILED(result)) {
        BENTO_ERROR("tell failed (%d)\n", result);
        goto cleanup;
    }
    mdat_payload_offset += 8; // header
    moof = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);

    // now create a movie fragment
    fragment = new AP4_MovieFragment(moof);
    atom = NULL; // ownership has been transferred

    // get the ID of the first track
    // (we assume there is only one track in each fragment, since we don't
    // support multiplexed fragments)
    fragment->GetTrackIds(track_ids);
    if (track_ids.ItemCount() < 1) {
        BENTO_ERROR("no track found in fragment\n");
        goto cleanup;
    }
    if (track_ids.ItemCount() > 1) {
        BENTO_PRINT("more than one track IDs found in fragment, selecting the first one only\n");
    }
    track_id = track_ids[0];
    //BENTO_PRINT("zx11 sampletable selected track ID = %d\n", track_id);
    if(track_id != select_track_id)
    {
        BENTO_ERROR("Error: file next track_id %d dont match select_track_id %d!\n", track_id, select_track_id);
        _stream.Seek(start_offset);
        result = -2;
        goto cleanup;
    }


    track = movie->GetTrack(track_id);
    if (!track) {
        BENTO_ERROR("Cannot get fragment's track id '%u' in the init segment\n", track_id);
        goto cleanup;
    }

    // create the sample table
    result = fragment->CreateSampleTable(movie,
                                         track_id,
                                         &_stream,
                                         moof_payload_offset,
                                         mdat_payload_offset,
                                         m_HaveMediaDtsOrigin ?
                                         m_MediaDtsOrigin : 0,
                                         sample_table);
    if (AP4_FAILED(result) || (sample_table == NULL)) {
        BENTO_ERROR("Could not create fragment sample table\n");
        goto cleanup;
    }

    p_InnerContext->m_SampleTable = sample_table;
    //BENTO_PRINT("zx p_SampleTable->GetSampleCount %d\n", p_SampleTable->GetSampleCount());
    if (p_InnerContext->m_SampleTable->GetSampleCount()) {
        AP4_Sample first_sample;
        if (AP4_SUCCEEDED(p_InnerContext->m_SampleTable->GetSample(0, first_sample))) {
            m_MediaDtsOrigin = first_sample.GetDts();
            m_HaveMediaDtsOrigin = TRUE;
            //m_DtsBase = (NPT_Int64)AP4_ConvertTime(m_MediaDtsOrigin, m_MediaTimeScale, NANO);
            //m_NextDts = m_DtsBase;
            //m_HaveDtsBase = true;
            //NPT_LOG_FINE_2("New %s DTS Base: %.3f seconds", MediaTypeName(), double(m_DtsBase));
            sample_description_index = first_sample.GetDescriptionIndex();
        }
    }
    else
    {
        BENTO_ERROR("Error: track %d GetSampleCount 0!!!\n",
            track_id);
        goto cleanup;
    }

    // check the sample description for this track (assume a single sample
    // description)
    sdesc = track->GetSampleDescription(sample_description_index);
    cenc_info = NULL;
    cenc_algorithm_id = 0;
    if ( sdesc) {
        AP4_ProtectedSampleDescription* psdesc =
            AP4_DYNAMIC_CAST(AP4_ProtectedSampleDescription, sdesc);
        if (m_drmStart && psdesc)
        {
          AP4_ContainerAtom* traf = NULL;
          if (AP4_SUCCEEDED(fragment->GetTrafAtom(track_id, traf)))
          {
            AP4_SaioAtom* saio = NULL;
            AP4_SaizAtom* saiz = NULL;
            AP4_CencSampleEncryption* sample_encryption_atom = NULL;


            unsigned char *key = (unsigned char*)"[}[}[}[}[}[}[}[}";
            //unsigned char *key = (unsigned char*)"\xb4\x2c\xa3\x17\x2e\xe4\xe6\x9b\xf5\x18\x48\xa5\x9d\xb9\xcd\x13";
            /*int len1;
            printf("--zp-- key base64:%s\n", "AmfjCTOPbEOl3WD/5mcecA==");
            unsigned char *key = (unsigned char *)Encryt_base64_decode("W31bfVt9W31bfVt9W31bfQ==", &len1);*/

            if (p_InnerContext->pSampleDecrypter)
            {
                SAFE_DELETE(p_InnerContext->pSampleDecrypter);
            }
            AP4_CencSampleDecrypter* aa= NULL;

            result = AP4_CencSampleDecrypter::Create(
                      psdesc,
                      traf,
                      _stream,
                      AP4_Position(moof_payload_offset),
                      (AP4_UI08*)key,
                      AP4_Size(16),
                      &AP4_DefaultBlockCipherFactory::Instance,
                      saio,
                      saiz,
                      sample_encryption_atom,aa);
            p_InnerContext->pSampleDecrypter = aa;

            if (AP4_FAILED(result)) {
              BENTO_PRINT("unable to create sample info table "\
                "(%d)", result);
            }
          }
        }
    }
    else
        BENTO_ERROR("Error: track %d GetSampleDescription failed!!!\n",
            track_id);

cleanup:
    if (atom != NULL) {
        delete atom;
    }
    if(fragment)
        delete fragment;
    //AP4_RELEASE(stream);
    return result;
}

int Bento4Mp4Demux::CreateFragmentMultiTrackSampleTable( AP4_Movie* movie, AVFormatContext *s)
{
    AP4_Atom*                atom = NULL;
    AP4_ContainerAtom*       moof = NULL;
    AP4_Position             moof_payload_offset = 0;
    AP4_Position             mdat_payload_offset = 0;
    AP4_Position             start_offset = 0;
    AP4_LargeSize            stream_size  = 0;
    AP4_MovieFragment*       fragment = NULL;
    AP4_Array<AP4_UI32>      track_ids;
    unsigned int sample_description_index = 0;
    AP4_SampleDescription*   sdesc = NULL;
    AP4_Track*               track = NULL;
    AP4_FragmentSampleTable* sample_table = NULL;
    int result = -1;
    int i = 0;
    AVStream *st = NULL;
    Bento4StreamContext *sc = NULL;

    // parse until we get to the moof
    _stream.Tell(start_offset);
    _stream.GetSize(stream_size);
    BENTO_PRINT("%s start_offset %lld, stream_size %lld\n", __func__, start_offset, stream_size);
    if(start_offset >= (stream_size - 4))
    {
        if (AP4_FAILED(_stream.Next()))
        {
            BENTO_ERROR("zx11 _stream.Next() failed, maybe eof!\n");
            result = -1;
            goto cleanup;
        }
        _stream.Tell(start_offset);
    }
    for (;;) {
        result = _stream.Tell(moof_payload_offset);
        if (AP4_FAILED(result)) {
            BENTO_ERROR("tell failed (%d)", result);
            result = -1;
            goto cleanup;
        }

        BENTO_PRINT("%s moof_payload_offset %lld\n", __func__, moof_payload_offset);
        result = AP4_DefaultAtomFactory::Instance_.CreateAtomFromStream(_stream,
                                                                        atom);
        if (AP4_FAILED(result)) {
            if (result == AP4_ERROR_EOS) {
                 BENTO_ERROR("zx11 AP4_ERROR_EOS\n");
                result = AP4_ERROR_EOS;
            } else {
                BENTO_ERROR("failed to parse atom while looking for moof "\
                                  "(%d)\n", result);
            }
            goto cleanup;
        }

        AP4_UI32 type = atom->GetType();
        BENTO_PRINT("atom->GetType() %.4s\n", (char *)&type);
        if (atom->GetType() == AP4_ATOM_TYPE_MOOF)
        {
            start_offset = moof_payload_offset;
            break;
        }

        /*if (atom->GetType() == AP4_ATOM_TYPE_PRFT ||
            atom->GetType() == AP4_ATOM_TYPE_PSSH) {
            AP4_MemoryByteStream* mbs = new AP4_MemoryByteStream();
            atom->Write(*mbs);
            DashExtensionInfo tmp;
            tmp.type = atom->GetType();
            tmp.data.SetData(mbs->GetData(),mbs->GetDataSize());
            extensions.Append(tmp);
            mbs->Release();
        }*/
        delete atom;
        atom = NULL;
    }


    // record the stream position:
    // we'll assume that the mdat starts just after and the header is 8 bytes
    result = _stream.Tell(mdat_payload_offset);
    if (AP4_FAILED(result)) {
        BENTO_ERROR("tell failed (%d)\n", result);
        goto cleanup;
    }
    mdat_payload_offset += 8; // header
    moof = AP4_DYNAMIC_CAST(AP4_ContainerAtom, atom);

    // now create a movie fragment
    fragment = new AP4_MovieFragment(moof);
    atom = NULL; // ownership has been transferred

    // get the ID of the first track
    // (we assume there is only one track in each fragment, since we don't
    // support multiplexed fragments)
    fragment->GetTrackIds(track_ids);
    if (track_ids.ItemCount() < 1) {
        BENTO_ERROR("no track found in fragment\n");
        goto cleanup;
    }

    BENTO_PRINT("zx11 sampletable track_ids.ItemCount() %d\n",track_ids.ItemCount());

    if(s->nb_streams != track_ids.ItemCount())
    {
        BENTO_PRINT("Error, s->nb_streams %d is diff to track_ids.ItemCount() %d\n"
            , s->nb_streams, track_ids.ItemCount());
    }
    for (i = 0; i < s->nb_streams; i++) {
        st = s->streams[i];
        sc = (Bento4StreamContext *)st->priv_data;
        Bento4InnerContext *p_InnerContext = (Bento4InnerContext *)sc->p_bento4Inner;
        if(p_InnerContext)
        {
            //BENTO_PRINT("zx delete p_InnerContext->m_SampleTable 0x%x\n", p_InnerContext->m_SampleTable);
            if(p_InnerContext->m_SampleTable)
                SAFE_DELETE(p_InnerContext->m_SampleTable);
        }
        else
        {
            BENTO_ERROR("Bento4InnerContext is NULL, return !\n");
            return -1;
        }
        int track_id = sc->track_id;
        track = movie->GetTrack(track_id);
        if (!track) {
            BENTO_ERROR("Cannot get fragment's track id '%u' in the init segment\n", track_id);
            goto cleanup;
        }

        // create the sample table
        result = fragment->CreateSampleTable(movie,
                                             track_id,
                                             &_stream,
                                             moof_payload_offset,
                                             mdat_payload_offset,
                                             m_HaveMediaDtsOrigin ?
                                             m_MediaDtsOrigin : 0,
                                             sample_table);
        if (AP4_FAILED(result) || (sample_table == NULL)) {
            BENTO_ERROR("Could not create fragment sample table\n");
            goto cleanup;
        }

        p_InnerContext->m_SampleTable = sample_table;
        BENTO_PRINT("zx p_SampleTable->GetSampleCount %d\n", sample_table->GetSampleCount());
        if (p_InnerContext->m_SampleTable->GetSampleCount()) {
            AP4_Sample first_sample;
            if (AP4_SUCCEEDED(p_InnerContext->m_SampleTable->GetSample(0, first_sample))) {
                m_MediaDtsOrigin = first_sample.GetDts();
                m_HaveMediaDtsOrigin = TRUE;
                //m_DtsBase = (NPT_Int64)AP4_ConvertTime(m_MediaDtsOrigin, m_MediaTimeScale, NANO);
                //m_NextDts = m_DtsBase;
                //m_HaveDtsBase = true;
                //NPT_LOG_FINE_2("New %s DTS Base: %.3f seconds", MediaTypeName(), double(m_DtsBase));
                sample_description_index = first_sample.GetDescriptionIndex();
            }
        }
        else
        {
            BENTO_ERROR("Error: track %d GetSampleCount 0!!!\n",
                track_id);
            goto cleanup;
        }

        // check the sample description for this track (assume a single sample
        // description)
        sdesc = track->GetSampleDescription(sample_description_index);
        if ( sdesc) {
            BENTO_PRINT("%s %d sample_description_index %d\n", __func__, __LINE__, sample_description_index);
            AP4_ProtectedSampleDescription* psdesc =
                AP4_DYNAMIC_CAST(AP4_ProtectedSampleDescription, sdesc);
            //if (m_drmStart && psdesc)
            if (psdesc)
            {
              AP4_ContainerAtom* traf = NULL;
              if (AP4_SUCCEEDED(fragment->GetTrafAtom(track_id, traf)))
              {
                AP4_SaioAtom* saio = NULL;
                AP4_SaizAtom* saiz = NULL;
                AP4_CencSampleEncryption* sample_encryption_atom = NULL;

                BENTO_PRINT("%s %d\n", __func__, __LINE__);

                //unsigned char *key = (unsigned char*)"[}[}[}[}[}[}[}[}";
                //unsigned char *key = (unsigned char*)"\xb4\x2c\xa3\x17\x2e\xe4\xe6\x9b\xf5\x18\x48\xa5\x9d\xb9\xcd\x13";
                /*int len1;
                BENTO_PRINT("--zp-- key base64:%s\n", "AmfjCTOPbEOl3WD/5mcecA==");
                unsigned char *key = (unsigned char *)Encryt_base64_decode("W31bfVt9W31bfVt9W31bfQ==", &len1);*/
                //unsigned char *key = (unsigned char*)"\xa1\x4c\xa3\x61\xed\x58\x9f\x7b\xfa\x6b\x49\xcd\xdd\xe7\x6d\xec";
                unsigned char *key = (unsigned char*)"0123456789123456";

                if (p_InnerContext->pSampleDecrypter)
                {
                    SAFE_DELETE(p_InnerContext->pSampleDecrypter);
                }
                AP4_CencSampleDecrypter* aa= NULL;

                result = AP4_CencSampleDecrypter::Create(
                          psdesc,
                          traf,
                          _stream,
                          AP4_Position(moof_payload_offset),
                          (AP4_UI08*)key,
                          AP4_Size(16),
                          &AP4_DefaultBlockCipherFactory::Instance,
                          saio,
                          saiz,
                          sample_encryption_atom,aa);
                p_InnerContext->pSampleDecrypter = aa;

                if (AP4_FAILED(result)) {
                  BENTO_PRINT("unable to create sample info table "\
                    "(%d)", result);
                }
              }
            }
        }
        else
            BENTO_ERROR("Error: track %d GetSampleDescription failed!!!\n",
                track_id);
    }

cleanup:
    if (atom != NULL) {
        delete atom;
    }
    if(fragment)
        delete fragment;
    //AP4_RELEASE(stream);
    return result;
}

//for dash
int Bento4Mp4Demux::Seek(AP4_SI64 offset, AP4_SI32 type)
{
    BENTO_PRINT("Bento4Mp4Demux::Seek %lld, type %d\n", offset, type);
    int result = -1;
    if(type == BENTO4_SEEK_BYTE)
    {
        AP4_Position    seek_pos = offset;
        _stream.Seek(seek_pos);
        result = 0;
    }
    else
        BENTO_ERROR("Error: Bento4 only support byte seek!!!\n");

    return result;
}

//for dash multi track mp4
int Bento4Mp4Demux::buildMulIndexEntry(AVFormatContext *s)
{
    int result = -1;
    AP4_Sample               sample;
    unsigned int             distance = 0;
    Bento4StreamContext*     sc = NULL;
    AVStream                 *st = NULL;
    AP4_Movie*               movie = _pAP4File->GetMovie();
    int                      i = 0;
    int                      stream = 0;

    result = CreateFragmentMultiTrackSampleTable(movie, s);
    if(result != 0)
    {
        BENTO_ERROR("Error: CreateFragmentMultiTrackSampleTable for tracks failed!!!\n");
        return result;
    }
    m_isDash = TRUE;

    for (stream = 0; stream < s->nb_streams; stream++) {
        st = s->streams[stream];
        sc = (Bento4StreamContext *)st->priv_data;
        Bento4InnerContext *p_InnerContext = (Bento4InnerContext *)sc->p_bento4Inner;
        if (av_reallocp_array(&st->index_entries,
                              st->nb_index_entries + p_InnerContext->m_SampleTable->GetSampleCount(),
                              sizeof(*st->index_entries)) < 0) {
            st->nb_index_entries = 0;
            return -1;
        }
        st->index_entries_allocated_size = (st->nb_index_entries + p_InnerContext->m_SampleTable->GetSampleCount()) * sizeof(*st->index_entries);

        for (i = 0; i < p_InnerContext->m_SampleTable->GetSampleCount(); i++)
        {
            p_InnerContext->m_SampleTable->GetSample(i, sample);
            int keyframe = 0;

            if (sample.IsSync())
                keyframe = 1;

            if (keyframe)
                 distance = 0;

            if(sample.GetSize() > MAX_PKT_SIZE || sample.GetSize() <= 0)
            {
                BENTO_ERROR("cur size %d error, try next fragment!\n", sample.GetSize());
                break;
            }

            AVIndexEntry *e = &st->index_entries[st->nb_index_entries++];
            e->pos = sample.GetOffset();
            e->timestamp = sample.GetDts();
            e->size = sample.GetSize();
            e->min_distance = distance;
            e->flags = keyframe ? AVINDEX_KEYFRAME : 0;
            /*if(i< 3)
                BENTO_PRINT("SampleCount %d, st->nb_index %d, offset %lld, dts %lld, "
                    "size %d, distance %d, keyframe %d\n",
                    p_InnerContext->m_SampleTable->GetSampleCount(), st->index,
                    e->pos, e->timestamp, e->size, distance, keyframe);*/
            distance++;
        }
    }
    return result;
}

//for dash
int Bento4Mp4Demux::buildIndexEntry(AVStream *st)
{
    int result = -1;
    AP4_CencSampleInfoTable* cenc_info = NULL;
    AP4_UI32                 cenc_algorithm_id = 0;
    const AP4_UI08*          cenc_kid = NULL;
    AP4_Sample               sample;
    unsigned int             distance = 0;
    AP4_UI32                 track_id = 0;
    int i = 0;
    AP4_Array<DashExtensionInfo> extensions;
    Bento4StreamContext*  sc;

    sc = (Bento4StreamContext *)st->priv_data;
    Bento4InnerContext *p_InnerContext = (Bento4InnerContext *)sc->p_bento4Inner;
    if(p_InnerContext)
    {
        //BENTO_PRINT("zx delete p_InnerContext->m_SampleTable 0x%x\n", p_InnerContext->m_SampleTable);
        if(p_InnerContext->m_SampleTable)
            SAFE_DELETE(p_InnerContext->m_SampleTable);
    }
    else
    {
        BENTO_ERROR("Bento4InnerContext is NULL, return !\n");
        return -1;
    }
    result = CreateFragmentSampleTable(_pAP4File->GetMovie(),
                                       cenc_info,
                                       cenc_algorithm_id,
                                       cenc_kid,
                                       extensions,
                                       track_id,
                                       p_InnerContext,
                                       sc->track_id);
    if(result != 0)
    {
        BENTO_ERROR("Error: CreateFragmentSampleTable for track %d failed!!!\n", sc->track_id);
        return result;
    }
    m_isDash = TRUE;

    //BENTO_PRINT("build_index_entry for track %d, p_InnerContext->m_SampleTable 0x%x\n",
    //    track_id, p_InnerContext->m_SampleTable);
    AP4_Movie* movie = _pAP4File->GetMovie();
    if (av_reallocp_array(&st->index_entries,
                          st->nb_index_entries + p_InnerContext->m_SampleTable->GetSampleCount(),
                          sizeof(*st->index_entries)) < 0) {
        st->nb_index_entries = 0;
        return -1;
    }
    st->index_entries_allocated_size = (st->nb_index_entries + p_InnerContext->m_SampleTable->GetSampleCount()) * sizeof(*st->index_entries);

    for (i = 0; i < p_InnerContext->m_SampleTable->GetSampleCount(); i++)
    {
        p_InnerContext->m_SampleTable->GetSample(i, sample);
        int keyframe = 0;

        if (sample.IsSync())
            keyframe = 1;

        if (keyframe)
             distance = 0;

        if(sample.GetSize() > MAX_PKT_SIZE || sample.GetSize() <= 0)
        {
            BENTO_ERROR("cur size %d error, try next fragment!\n", sample.GetSize());
            break;
        }

        AVIndexEntry *e = &st->index_entries[st->nb_index_entries++];
        e->pos = sample.GetOffset();
        e->timestamp = sample.GetDts();
        e->size = sample.GetSize();
        e->min_distance = distance;
        e->flags = keyframe ? AVINDEX_KEYFRAME : 0;
        /*if(i< 3)
            BENTO_PRINT("SampleCount %d, st->nb_index %d, offset %lld, dts %lld, "
                "size %d, distance %d, keyframe %d\n",
                p_InnerContext->m_SampleTable->GetSampleCount(), st->index,
                e->pos, e->timestamp, e->size, distance, keyframe);*/
        distance++;
    }

    return result;
}

#ifdef DRM_SMP_ENABLE
static void drm_smp_pkt_free(void *opaque, unsigned char *data)
{
    AVPacket *pkt = (AVPacket *)opaque;

    (void) pkt;
    DMTRM_BUFFER_OUT tmpbuf;
    tmpbuf.data = (unsigned char *) data;
    tmpbuf.is_secure = 1;
    MTDrm_FreeSecureMemory(&tmpbuf);
}

static void drm_smp_pkt_new(AVPacket *pkt, unsigned char *data, int size)
{
    av_packet_unref(pkt);

    pkt->buf = av_buffer_create(data,
        size, drm_smp_pkt_free, pkt, 1); /* BUFFER_FLAG_READONLY 1 */

    if (!pkt->buf) {
        return;
    }

    pkt->data = data;
    pkt->size = size;
    pkt->is_secure = 1;
}

static int drm_smp_filter_h264(AVStream *st, AVPacket *pkt,
    AP4_DataBuffer *pData, Bento4InnerContext *p_InnerContext)
{
    int ret = -1;
    DMTRM_BUFFER_OUT newbuf =  {0};

    if (MTDrm_GetDecryptType(st->eDrmIndex)) {
        int pkt_size = pData->GetDataSize() + MAX_VIDEO_FILTER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE;
        if (MTDrm_MallocSecureMemory(&newbuf, pkt_size) != MTDRM_SUCCESS) {
            printf("%s malloc secure memory failed!\n", __func__);
            drm_smp_pkt_free(NULL, (unsigned char *) pData->GetData());
            return AVERROR(ENOMEM);
        }

        drm_smp_pkt_new(pkt, newbuf.data, pkt_size);
        ret = MTDrm_FilterAvcSample((unsigned char *)pData->GetData(), pData->GetDataSize(),
                (unsigned char *)p_InnerContext->pPrefix->GetData(), p_InnerContext->pPrefix->GetDataSize(),
                p_InnerContext->nalu_length_size, newbuf.data, &(pkt->size));
        drm_smp_pkt_free(NULL, (unsigned char *) pData->GetData());
        if (0 != ret) {
            return -1;
        }
    } else {
        //clear stream!!!
        CopyAvcSample(*pData, *(p_InnerContext->pPrefix), p_InnerContext->nalu_length_size, pkt);
    }
    return 0;
}

static int drm_smp_filter_h265(AVStream *st, AVPacket *pkt,
    AP4_DataBuffer *pData, Bento4InnerContext *p_InnerContext)
{
    int ret = -1;
    DMTRM_BUFFER_OUT newbuf =  {0};

    if (MTDrm_GetDecryptType(st->eDrmIndex)) {
        int pkt_size = pData->GetDataSize() + MAX_VIDEO_FILTER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE;
        if (MTDrm_MallocSecureMemory(&newbuf, pkt_size) != MTDRM_SUCCESS) {
            printf("%s malloc secure memory failed!\n", __func__);
            drm_smp_pkt_free(NULL, (unsigned char *) pData->GetData());
            return AVERROR(ENOMEM);
        }

        drm_smp_pkt_new(pkt, newbuf.data, pkt_size);
        ret = MTDrm_FilterHevcSample((unsigned char *)pData->GetData(), pData->GetDataSize(),
                 (unsigned char *)p_InnerContext->pPrefix->GetData(), p_InnerContext->pPrefix->GetDataSize(),
                 p_InnerContext->nalu_length_size, newbuf.data, &(pkt->size));
        drm_smp_pkt_free(NULL, (unsigned char *) pData->GetData());
        if (0 != ret) {
            return -1;
        }
    } else {
        //clear stream!!!
        CopyHevcSample(*pData, *(p_InnerContext->pPrefix), p_InnerContext->nalu_length_size, pkt);
    }
    return 0;
}

static int drm_smp_filter_aac(AVStream *st, AVPacket *pkt,
    AP4_DataBuffer *pData, Bento4InnerContext *p_InnerContext)
{
    int result = -1;
    int ret = -1;
    DMTRM_BUFFER_OUT newbuf =  {0};

    if (MTDrm_GetDecryptType(st->eDrmIndex)) {
        int pkt_size = pData->GetDataSize() + 7 + AV_INPUT_BUFFER_PADDING_SIZE;
        if (MTDrm_MallocSecureMemory(&newbuf, pkt_size) != MTDRM_SUCCESS) {
            printf("%s malloc secure memory failed!\n", __func__);
            drm_smp_pkt_free(NULL, (unsigned char *) pData->GetData());
            return AVERROR(ENOMEM);
        }
        drm_smp_pkt_new(pkt, newbuf.data, pkt_size);
        ret = MTDrm_AddAAHeader((unsigned char *)pData->GetData(), pData->GetDataSize(),
                st->codec->sample_rate, st->codec->channels, newbuf.data);
        drm_smp_pkt_free(NULL, (unsigned char *) pData->GetData());
        if (0 != ret) {
            return -1;
        }
    } else {
        //clear stream!!!
        if ((result = av_new_packet(pkt, (pData->GetDataSize() + 7 + AV_INPUT_BUFFER_PADDING_SIZE))) < 0) {
            BENTO_ERROR("%s: Error av_new_packet fail!!! result = %d\n", __func__, result);
            return result;
        }
        DRM_AddAAHeader((unsigned char *)pData->GetData(), pData->GetDataSize(),
            st->codec->sample_rate, st->codec->channels, pkt->data);
    }
    pkt->size = pData->GetDataSize() + 7;
    return 0;
}

static int drm_smp_filter_general(AVStream *st,
    AVPacket *pkt, AP4_DataBuffer *pData, AVIndexEntry *avsample)
{
    if (MTDrm_GetDecryptType(st->eDrmIndex)) {
        drm_smp_pkt_new(pkt, (unsigned char *) pData->GetData(), pData->GetDataSize());
    } else {
        //clear stream!!!
        int result = -1;
        if ((result = av_new_packet(pkt, (avsample->size + AV_INPUT_BUFFER_PADDING_SIZE))) < 0) {
            BENTO_ERROR("%s: Error av_new_packet fail!!! result = %d\n", __func__, result);
            return result;
        }
        memcpy(pkt->data, pData->GetData(), pData->GetDataSize());
        pkt->size = pData->GetDataSize();
    }
    return 0;
}
#endif

int Bento4Mp4Demux::ReadPacketFromPipe(AVStream *st, AVIndexEntry *avsample, AVPacket *pkt)
{
    Bento4StreamContext  *sc;
    AP4_Sample            sample;
    AP4_ByteStream       *sample_stream  = NULL;
    AP4_MemoryByteStream *mem_stream     = NULL;
    int                   result = -1;
#ifdef DRM_SMP_ENABLE
    DMTRM_BUFFER_OUT newbuf;
    int ret = -1;
    MTDrm_SetDecryptType(st->eDrmIndex, 0);
#endif

    sc = (Bento4StreamContext *)st->priv_data;
    Bento4InnerContext *p_InnerContext = (Bento4InnerContext *)sc->p_bento4Inner;
    if (p_InnerContext == NULL) {
        BENTO_ERROR("Error, p_InnerContext is NULL!!!\n");
        return -1;
    }

    AP4_FragmentSampleTable *p_SampleTable = (AP4_FragmentSampleTable *)p_InnerContext->m_SampleTable;
    int error_sample = 0;
get_sample:
    p_SampleTable->GetSample(sc->current_sample, sample);
    if (sample.GetSize() != avsample->size && sample.GetOffset() != avsample->pos) {
        BENTO_PRINT("%s: Error sample %d dont match av index_entries\n",
                    __func__, sc->current_sample);
        BENTO_PRINT("%d %lld, av %d %lld\n",
                    sample.GetSize(), sample.GetOffset(), avsample->size, avsample->pos);
        return -1;
    }

    if (sample.GetSize() > MAX_PKT_SIZE || sample.GetSize() <= 0) {
        BENTO_PRINT("zx sample size error!!!  %d get next!\n", sample.GetSize());
        error_sample++;
        if (error_sample > 3) {
            return -1;
        }
        goto get_sample;
    }

    while (1) {
        int sample_size = sample.GetSize();
        sample_stream = sample.GetDataStream();
        if (sample_stream == NULL) {
            BENTO_ERROR("%s: Error sample_stream is NULL!!!, sample_size %d\n", __func__, sample_size);
            return -1;
        }

        mem_stream = new AP4_MemoryByteStream(sample_size);
        result = sample_stream->Seek(sample.GetOffset());
        if (AP4_FAILED(result)) {
            BENTO_ERROR("seek in sample stream failed with error %d\n", result);
            continue;
        }
        result = sample_stream->Read(mem_stream->UseData(), sample_size);
        if (AP4_FAILED(result)) {
            BENTO_ERROR("read in sample stream failed with error %d \n", result);
            return -1;
        }

        AP4_DataBuffer data;
        AP4_DataBuffer data2;
        AP4_DataBuffer *pData = &data;

        data.SetData(mem_stream->GetData(), mem_stream->GetDataSize());

#ifdef MT_DRM_SUPPORT
        Bento4InnerContext *p_InnerContext = (Bento4InnerContext *)sc->p_bento4Inner;
        if (p_InnerContext->pSampleDecrypter) {
#ifdef VMX_OTT_SVP
            if (p_InnerContext->pPrefix) {
                p_InnerContext->pSampleDecrypter->DecryptSampleData(data, data2, NULL, 0, (AP4_UI32)p_InnerContext->decrypt_index, vmx_str,
                    st->codecpar->codec_type, (unsigned char *)p_InnerContext->pPrefix->GetData(), p_InnerContext->pPrefix->GetDataSize(),
                    p_InnerContext->nalu_length_size, st->codec->sample_rate, st->codec->channels, st->eDrmIndex);
            } else {
                p_InnerContext->pSampleDecrypter->DecryptSampleData(data, data2, NULL, 0, (AP4_UI32)p_InnerContext->decrypt_index, vmx_str,
                    st->codecpar->codec_type, NULL, 0,
                    p_InnerContext->nalu_length_size, st->codec->sample_rate, st->codec->channels, st->eDrmIndex);
            }
#else
            if (p_InnerContext->pPrefix) {
                p_InnerContext->pSampleDecrypter->DecryptSampleData(data, data2, NULL, g_encrypt_type, (AP4_UI32)p_InnerContext->decrypt_index, vmx_str,
                    st->codecpar->codec_type, (unsigned char *)p_InnerContext->pPrefix->GetData(), p_InnerContext->pPrefix->GetDataSize(),
                    p_InnerContext->nalu_length_size, st->codec->sample_rate, st->codec->channels, st->eDrmIndex);
            } else {
                p_InnerContext->pSampleDecrypter->DecryptSampleData(data, data2, NULL, g_encrypt_type, (AP4_UI32)p_InnerContext->decrypt_index, vmx_str,
                    st->codecpar->codec_type, NULL, 0,
                    p_InnerContext->nalu_length_size, st->codec->sample_rate, st->codec->channels, st->eDrmIndex);
            }
#endif
            pData = &data2;
#ifdef DRM_SMP_ENABLE
            pData->SetDataSize(data.GetDataSize());
            MTDrm_SetDecryptType(st->eDrmIndex, 1);
#endif
        }
#endif
        if (st->codecpar->codec_id == AV_CODEC_ID_H264) {
#if defined(DRM_SMP_ENABLE)
            ret = drm_smp_filter_h264(st, pkt, pData, p_InnerContext);
            if (0 != ret) {
                return ret;
            }
#elif defined(VMX_OTT_SVP)
            int pkt_size = pData->GetDataSize() + MAX_VIDEO_FILTER_SIZE + AV_INPUT_BUFFER_PADDING_SIZE;
            int ret = -1;

            av_new_packet(pkt, 8);//fake pkt
            av_free(pkt->buf->data);//free fakt pkt store buf, will set real secure buf
            pkt->data = pkt->buf->data = (uint8_t *)pData->GetData();
            pkt->size = pData->GetDataSize();
#else
            CopyAvcSample(*pData, *(p_InnerContext->pPrefix), p_InnerContext->nalu_length_size, pkt);
#endif
        } else if (st->codecpar->codec_id == AV_CODEC_ID_HEVC) {
#ifdef DRM_SMP_ENABLE
            ret = drm_smp_filter_h265(st, pkt, pData, p_InnerContext);
            if (0 != ret) {
                return ret;
            }
#else
            CopyHevcSample(*pData, *(p_InnerContext->pPrefix), p_InnerContext->nalu_length_size, pkt);
#endif
        } else if (st->codec->codec_id == AV_CODEC_ID_AAC) { //
#if defined(DRM_SMP_ENABLE)
            ret = drm_smp_filter_aac(st, pkt, pData, p_InnerContext);
            if (0 != ret) {
                return ret;
            }
#elif defined(VMX_OTT_SVP)
            int pkt_size = pData->GetDataSize() + 7 + AV_INPUT_BUFFER_PADDING_SIZE;
            int ret = -1;
            av_new_packet(pkt, 8);//fake pkt
            av_free(pkt->buf->data);//free fakt pkt store buf, will set real secure buf
            pkt->data = pkt->buf->data = (uint8_t *)pData->GetData();
            pkt->size = pData->GetDataSize();
#else
            if ((result = av_new_packet(pkt, (pData->GetDataSize() + 7 + AV_INPUT_BUFFER_PADDING_SIZE))) < 0) {
                BENTO_ERROR("%s: Error av_new_packet fail!!! result = %d\n", __func__, result);
                return result;
            }
            DRM_AddAAHeader((unsigned char *)pData->GetData(), pData->GetDataSize(),
                            st->codec->sample_rate, st->codec->channels, pkt->data);
            pkt->size = pData->GetDataSize() + 7;
#endif
        } else {
#ifdef DRM_SMP_ENABLE
            ret = drm_smp_filter_general(st, pkt, pData, avsample);
            if (0 != ret) {
                return ret;
            }
#else
            if ((result = av_new_packet(pkt, (avsample->size + AV_INPUT_BUFFER_PADDING_SIZE))) < 0) {
                BENTO_ERROR("%s: Error av_new_packet fail!!! result = %d\n", __func__, result);
                return result;
            }
            memcpy(pkt->data, pData->GetData(), pData->GetDataSize());
            pkt->size = pData->GetDataSize();
#endif
        }

        pkt->stream_index = st->index;
        pkt->dts = avsample->timestamp;
        pkt->pts = sample.GetCts();//dont need sample.GetCts()*AV_TIME_BASE/sc->time_scale
        pkt->flags |= avsample->flags & AVINDEX_KEYFRAME ? AV_PKT_FLAG_KEY : 0;

        // release the sample stream
        AP4_RELEASE(sample_stream);
        sample_stream = NULL;

        // swap the sample stream
        AP4_RELEASE(mem_stream);
        mem_stream = NULL;
        break;
    }

    if (pkt->size == 0) {
        BENTO_ERROR("CopyAvcSample error pkt->size == 0\n");
        return -1;
    }
    return 0;
}


int Bento4Mp4Demux::ReadPacket(AVStream *st, AVIndexEntry *avsample, AVPacket *pkt)
{
    //Bento4StreamContext *sc;
    //AP4_Track* cur_track;
    //int ret = -1;
    //AP4_CencSampleInfoTable* cenc_info = NULL;
    //AP4_UI32                 cenc_algorithm_id = 0;
    //const AP4_UI08*          cenc_kid = NULL;
    //AP4_UI32   track_id = 0;
    //AP4_Array<DashExtensionInfo> extensions;
    // get the video track

    if(m_isDash)
    {
        return ReadPacketFromPipe(st, avsample, pkt);
    }

    BENTO_PRINT("dont support no fragment mp4\n");
#if 0
    AP4_Movie* movie = _pAP4File->GetMovie();
    if(st->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        cur_track = movie->GetTrack(AP4_Track::TYPE_VIDEO);
    else if(st->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        cur_track = movie->GetTrack(AP4_Track::TYPE_AUDIO);
    else if(st->codecpar->codec_type == AVMEDIA_TYPE_SUBTITLE)
        cur_track = movie->GetTrack(AP4_Track::TYPE_SUBTITLES);

    sc = (Bento4StreamContext *)st->priv_data;
    AP4_DataBuffer data;
    AP4_DataBuffer data2;
    AP4_DataBuffer *pData = &data;
    AP4_Sample     ap4_sample;
    BENTO_PRINT("%s: codec_type %d, sc->current_sample %d st->index %d\n",
        __func__, st->codecpar->codec_type, sc->current_sample, st->index);
    if(AP4_SUCCEEDED(cur_track->ReadSample(sc->current_sample, ap4_sample, data)) )
    {
        if (sc->pSampleDecrypter)
        {
          sc->pSampleDecrypter->DecryptSampleData(data, data2, NULL);
          pData = &data2;
        }

        if(st->codecpar->codec_id == AV_CODEC_ID_H264)
        {
            //BENTO_PRINT("h264 prefix %d\n", prefix.GetDataSize());
            CopyAvcSample(*pData, *(sc->pPrefix), sc->nalu_length_size, pkt);
        }
        else if(st->codecpar->codec_id == AV_CODEC_ID_HEVC)
        {
            CopyHevcSample(*pData, *(sc->pPrefix), sc->nalu_length_size, pkt);
        }
        else
        {
            if ((ret = av_new_packet(pkt, (avsample->size+AV_INPUT_BUFFER_PADDING_SIZE))) < 0)
                return ret;
            memcpy(pkt->data, pData->GetData(), pData->GetDataSize());
            pkt->size = pData->GetDataSize();
        }

        pkt->stream_index = st->index;
        pkt->dts = avsample->timestamp;
        pkt->pts = ap4_sample.GetCts();
        pkt->flags |= avsample->flags & AVINDEX_KEYFRAME ? AV_PKT_FLAG_KEY : 0;
    }
    else
        return -1;
#endif
    return 0;
}

void Bento4Mp4Demux::close(AVStream *st)
{
    Bento4StreamContext*  sc;

    sc = (Bento4StreamContext *)st->priv_data;

    BENTO_PRINT("sc->sidx  0x%x\n", sc->sidx);
    if(sc->sidx)
    {
        BENTO_PRINT("sc->sidx->entries 0x%x\n", sc->sidx->entries);
        if(sc->sidx->entries)
        {
            av_free(sc->sidx->entries);
            sc->sidx->entries = NULL;
        }
        av_free(sc->sidx);
        sc->sidx = NULL;
    }

    if(sc->p_trfaEntry)
    {
        av_free(sc->p_trfaEntry);
        sc->p_trfaEntry = NULL;
    }
    sc->trfaEntries_Count = 0;

    Bento4InnerContext *p_InnerContext = (Bento4InnerContext *)sc->p_bento4Inner;
    if(p_InnerContext)
    {
        //if(p_InnerContext->pSampleDescription);////track->GetSampleDescription, dont need delete!
        //    SAFE_DELETE(p_InnerContext->pSampleDescription);
        if(p_InnerContext->pPrefix);
            SAFE_DELETE(p_InnerContext->pPrefix);
        if(p_InnerContext->pSampleDecrypter)
            SAFE_DELETE(p_InnerContext->pSampleDecrypter);
        if(p_InnerContext->m_SampleTable)
            SAFE_DELETE(p_InnerContext->m_SampleTable);
        if(p_InnerContext->cenc_base64_str)
            av_free(p_InnerContext->cenc_base64_str);

        av_free(p_InnerContext);
        p_InnerContext = NULL;
    }

    m_drmStart = FALSE;
}

Bento4Mp4Demux::~Bento4Mp4Demux()
{
  SAFE_DELETE(_pAP4File);

  if(vmx_str)
    av_freep(&vmx_str);
#ifdef MT_DRM_SUPPORT
  //if(g_encrypt_type)
  //  MTDrm_ReleaseDrmInstance(DRM_TYPE_WIDEVINE);

#endif

}



int Bento4Mp4Demux::Probe(char* buffer, int len)
{
#if 0
  int score = 0;
  FFmpegByteStream bufStream;
  bufStream.InitFromBuffer(buffer,len);
  BENTO_PRINT("%s %d enter\n", __func__, __LINE__);

  AP4_Processor* processor = NULL;
  AP4_File* input_file = new AP4_File(bufStream,TRUE);
  AP4_FtypAtom* ftyp = input_file->GetFileType();
  if (ftyp) {
    if (ftyp->GetMajorBrand() == AP4_OMA_DCF_BRAND_ODCF || ftyp->HasCompatibleBrand(AP4_OMA_DCF_BRAND_ODCF))
    {
      BENTO_PRINT("AP4_OmaDcfDecryptingProcessor\n");
      score = 100;
    }
    else if (ftyp->GetMajorBrand() == AP4_MARLIN_BRAND_MGSV || ftyp->HasCompatibleBrand(AP4_MARLIN_BRAND_MGSV))
    {
      BENTO_PRINT("AP4_MarlinIpmpDecryptingProcessor\n");
      score = 100;
    }
    else if (ftyp->GetMajorBrand() == AP4_PIFF_BRAND || ftyp->HasCompatibleBrand(AP4_PIFF_BRAND))
    {
      BENTO_PRINT("AP4_CencDecryptingProcessor\n");
      score = 100;
    }
  }
  if (score == 0)
  {
    AP4_Movie* movie = input_file->GetMovie();
    if (movie) {
      AP4_List<AP4_Track>& tracks = movie->GetTracks();
      for (unsigned int i=0; i<tracks.ItemCount(); i++)
      {
        AP4_Track* track = NULL;
        tracks.Get(i, track);
        if (track)
        {
          AP4_SampleDescription* sdesc = track->GetSampleDescription(0);
          if (sdesc && sdesc->GetType() == AP4_SampleDescription::TYPE_PROTECTED)
          {
            AP4_ProtectedSampleDescription* psdesc = AP4_DYNAMIC_CAST(AP4_ProtectedSampleDescription, sdesc);
            if (psdesc)
            {
              if (psdesc->GetSchemeType() == AP4_PROTECTION_SCHEME_TYPE_CENC ||
                psdesc->GetSchemeType() == AP4_PROTECTION_SCHEME_TYPE_CBC1 ||
                psdesc->GetSchemeType() == AP4_PROTECTION_SCHEME_TYPE_CENS ||
                psdesc->GetSchemeType() == AP4_PROTECTION_SCHEME_TYPE_CBCS)
              {
                 BENTO_PRINT("hahaha...\n");
                 score = 100;
                 break;
              }
            }
          }
        }
      }
    }
  }
  delete input_file;

  return score;
#endif
  return 0;
}

Bento4Mp4Demux::Bento4Mp4Demux()
{
    fragmented = 1;
    m_HaveMediaDtsOrigin = FALSE;
    m_isDash = FALSE;
    m_drmStart = FALSE;
    vmx_str = NULL;
}

/*int main3(int argc, char** argv)
{
  //Bento4Mp4Demux mp4Demux;
  Bento4Mp4Demux::Probe(NULL,0);
  return 0;
}*/
