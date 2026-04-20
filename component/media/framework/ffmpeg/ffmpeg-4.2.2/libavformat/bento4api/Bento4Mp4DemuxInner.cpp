#include "Bento4Mp4DemuxInner.h"

#define BENTO_PRINT printf





#define SHOW_INFO 1

#ifdef SHOW_INFO

static void
ShowMovieInfo(AP4_Movie& movie)
{
  BENTO_PRINT("Movie:\n");
  BENTO_PRINT("  duration:   %d ms\n", movie.GetDurationMs());
  BENTO_PRINT("  time scale: %d\n", movie.GetTimeScale());
  BENTO_PRINT("  fragments:  %s\n", movie.HasFragments() ? "yes" : "no");
  BENTO_PRINT("\n");
}

static void
ShowData(const AP4_DataBuffer& data)
{
  for (unsigned int i = 0; i < data.GetDataSize(); i++) {
    BENTO_PRINT("%02x", (unsigned char)data.GetData()[i]);
  }
}

/*----------------------------------------------------------------------
|   ShowPayload
+---------------------------------------------------------------------*/
static void
ShowPayload(AP4_Atom& atom, bool ascii = false)
{
  AP4_UI64 payload_size = atom.GetSize() - 8;
  if (payload_size <= 1024) {
    AP4_MemoryByteStream* payload = new AP4_MemoryByteStream();
    atom.Write(*payload);
    if (ascii) {
      // ascii
      payload->WriteUI08(0); // terminate with a NULL character
      BENTO_PRINT("%s", (const char*)payload->GetData() + atom.GetHeaderSize());
    }
    else {
      // hex
      for (unsigned int i = 0; i < payload_size; i++) {
        BENTO_PRINT("%02x", (unsigned char)payload->GetData()[atom.GetHeaderSize() + i]);
      }
    }
    payload->Release();
  }
}

/*----------------------------------------------------------------------
|   ShowSampleDescription_Text
+---------------------------------------------------------------------*/
static void
ShowSampleDescription(AP4_SampleDescription& description, bool verbose)
{
  AP4_SampleDescription* desc = &description;
  if (desc->GetType() == AP4_SampleDescription::TYPE_PROTECTED) {
    AP4_ProtectedSampleDescription* prot_desc = AP4_DYNAMIC_CAST(AP4_ProtectedSampleDescription, desc);
    if (prot_desc) {
      //ShowProtectedSampleDescription_Text(*prot_desc, verbose);
      desc = prot_desc->GetOriginalSampleDescription();
    }
  }

  if (verbose) {
    BENTO_PRINT("    Bytes: ");
    AP4_Atom* details = desc->ToAtom();
    ShowPayload(*details, false);
    BENTO_PRINT("\n");
    delete details;
  }

  char coding[5];
  AP4_FormatFourChars(coding, desc->GetFormat());
  BENTO_PRINT("    Coding:      %s", coding);
  const char* format_name = AP4_GetFormatName(desc->GetFormat());
  if (format_name) {
    BENTO_PRINT(" (%s)\n", format_name);
  }
  else {
    BENTO_PRINT("\n");
  }
  if (desc->GetType() == AP4_SampleDescription::TYPE_MPEG) {
    // MPEG sample description
    AP4_MpegSampleDescription* mpeg_desc = AP4_DYNAMIC_CAST(AP4_MpegSampleDescription, desc);

    BENTO_PRINT("    Stream Type: %s\n", mpeg_desc->GetStreamTypeString(mpeg_desc->GetStreamType()));
    BENTO_PRINT("    Object Type: %s\n", mpeg_desc->GetObjectTypeString(mpeg_desc->GetObjectTypeId()));
    BENTO_PRINT("    Max Bitrate: %d\n", mpeg_desc->GetMaxBitrate());
    BENTO_PRINT("    Avg Bitrate: %d\n", mpeg_desc->GetAvgBitrate());
    BENTO_PRINT("    Buffer Size: %d\n", mpeg_desc->GetBufferSize());

    if (mpeg_desc->GetObjectTypeId() == AP4_OTI_MPEG4_AUDIO ||
      mpeg_desc->GetObjectTypeId() == AP4_OTI_MPEG2_AAC_AUDIO_LC ||
      mpeg_desc->GetObjectTypeId() == AP4_OTI_MPEG2_AAC_AUDIO_MAIN) {
      AP4_MpegAudioSampleDescription* mpeg_audio_desc = AP4_DYNAMIC_CAST(AP4_MpegAudioSampleDescription, mpeg_desc);
      //if (mpeg_audio_desc) ShowMpegAudioSampleDescription(*mpeg_audio_desc);
    }
  }
  AP4_AudioSampleDescription* audio_desc =
    AP4_DYNAMIC_CAST(AP4_AudioSampleDescription, desc);
  if (audio_desc) {
    // Audio sample description
    BENTO_PRINT("    Sample Rate: %d\n", audio_desc->GetSampleRate());
    BENTO_PRINT("    Sample Size: %d\n", audio_desc->GetSampleSize());
    BENTO_PRINT("    Channels:    %d\n", audio_desc->GetChannelCount());
  }
  AP4_VideoSampleDescription* video_desc =
    AP4_DYNAMIC_CAST(AP4_VideoSampleDescription, desc);
  if (video_desc) {
    // Video sample description
    BENTO_PRINT("    Width:       %d\n", video_desc->GetWidth());
    BENTO_PRINT("    Height:      %d\n", video_desc->GetHeight());
    BENTO_PRINT("    Depth:       %d\n", video_desc->GetDepth());
  }

  // Dolby Digital specifics
  if (desc->GetFormat() == AP4_SAMPLE_FORMAT_EC_3) {
    AP4_Dec3Atom* dec3 = AP4_DYNAMIC_CAST(AP4_Dec3Atom, desc->GetDetails().GetChild(AP4_ATOM_TYPE('d', 'e', 'c', '3')));
    if (dec3) {
      BENTO_PRINT("    AC3 Data Rate: %d\n", dec3->GetDataRate());
      for (unsigned int i = 0; i < dec3->GetSubStreams().ItemCount(); i++) {
        BENTO_PRINT("    AC3 Substream %d:\n", i);
        BENTO_PRINT("        fscod       = %d\n", dec3->GetSubStreams()[i].fscod);
        BENTO_PRINT("        bsid        = %d\n", dec3->GetSubStreams()[i].bsid);
        BENTO_PRINT("        bsmod       = %d\n", dec3->GetSubStreams()[i].bsmod);
        BENTO_PRINT("        acmod       = %d\n", dec3->GetSubStreams()[i].acmod);
        BENTO_PRINT("        lfeon       = %d\n", dec3->GetSubStreams()[i].lfeon);
        BENTO_PRINT("        num_dep_sub = %d\n", dec3->GetSubStreams()[i].num_dep_sub);
        BENTO_PRINT("        chan_loc    = %d\n", dec3->GetSubStreams()[i].chan_loc);
      }
      BENTO_PRINT("    AC3 dec3 payload: [");
      ShowData(dec3->GetRawBytes());
      BENTO_PRINT("]\n");
    }
  }

  // AVC specifics
  if (desc->GetType() == AP4_SampleDescription::TYPE_AVC) {
    // AVC Sample Description
    AP4_AvcSampleDescription* avc_desc = AP4_DYNAMIC_CAST(AP4_AvcSampleDescription, desc);
    const char* profile_name = AP4_AvccAtom::GetProfileName(avc_desc->GetProfile());
    BENTO_PRINT("    AVC Profile:          %d", avc_desc->GetProfile());
    if (profile_name) {
      BENTO_PRINT(" (%s)\n", profile_name);
    }
    else {
      BENTO_PRINT("\n");
    }
    BENTO_PRINT("    AVC Profile Compat:   %x\n", avc_desc->GetProfileCompatibility());
    BENTO_PRINT("    AVC Level:            %d\n", avc_desc->GetLevel());
    BENTO_PRINT("    AVC NALU Length Size: %d\n", avc_desc->GetNaluLengthSize());
    BENTO_PRINT("    AVC SPS: [");
    const char* sep = "";
    for (unsigned int i = 0; i < avc_desc->GetSequenceParameters().ItemCount(); i++) {
      BENTO_PRINT("%s", sep);
      ShowData(avc_desc->GetSequenceParameters()[i]);
      sep = ", ";
    }
    BENTO_PRINT("]\n");
    BENTO_PRINT("    AVC PPS: [");
    sep = "";
    for (unsigned int i = 0; i < avc_desc->GetPictureParameters().ItemCount(); i++) {
      BENTO_PRINT("%s", sep);
      ShowData(avc_desc->GetPictureParameters()[i]);
      sep = ", ";
    }
    BENTO_PRINT("]\n");
    BENTO_PRINT("    Codecs String: ");
    AP4_String codecpar;
    avc_desc->GetCodecString(codecpar);
    BENTO_PRINT("%s", codecpar.GetChars());
    BENTO_PRINT("\n");
  }
  else if (desc->GetType() == AP4_SampleDescription::TYPE_HEVC) {
    // HEVC Sample Description
    AP4_HevcSampleDescription* hevc_desc = AP4_DYNAMIC_CAST(AP4_HevcSampleDescription, desc);
    const char* profile_name = AP4_HvccAtom::GetProfileName(hevc_desc->GetGeneralProfileSpace(), hevc_desc->GetGeneralProfile());
    BENTO_PRINT("    HEVC Profile Space:       %d\n", hevc_desc->GetGeneralProfileSpace());
    BENTO_PRINT("    HEVC Profile:             %d", hevc_desc->GetGeneralProfile());
    if (profile_name) BENTO_PRINT(" (%s)", profile_name);
    BENTO_PRINT("\n");
    BENTO_PRINT("    HEVC Profile Compat:      %x\n", hevc_desc->GetGeneralProfileCompatibilityFlags());
    BENTO_PRINT("    HEVC Level:               %d.%d\n", hevc_desc->GetGeneralLevel() / 30, (hevc_desc->GetGeneralLevel() % 30) / 3);
    BENTO_PRINT("    HEVC Tier:                %d\n", hevc_desc->GetGeneralTierFlag());
    BENTO_PRINT("    HEVC Chroma Format:       %d", hevc_desc->GetChromaFormat());
    const char* chroma_format_name = AP4_HvccAtom::GetChromaFormatName(hevc_desc->GetChromaFormat());
    if (chroma_format_name) BENTO_PRINT(" (%s)", chroma_format_name);
    BENTO_PRINT("\n");
    BENTO_PRINT("    HEVC Chroma Bit Depth:    %d\n", hevc_desc->GetChromaBitDepth());
    BENTO_PRINT("    HEVC Luma Bit Depth:      %d\n", hevc_desc->GetLumaBitDepth());
    BENTO_PRINT("    HEVC Average Frame Rate:  %d\n", hevc_desc->GetAverageFrameRate());
    BENTO_PRINT("    HEVC Constant Frame Rate: %d\n", hevc_desc->GetConstantFrameRate());
    BENTO_PRINT("    HEVC NALU Length Size:    %d\n", hevc_desc->GetNaluLengthSize());
    BENTO_PRINT("    HEVC Sequences:\n");
    for (unsigned int i = 0; i < hevc_desc->GetSequences().ItemCount(); i++) {
      const AP4_HvccAtom::Sequence& seq = hevc_desc->GetSequences()[i];
      BENTO_PRINT("      {\n");
      BENTO_PRINT("        Array Completeness=%d\n", seq.m_ArrayCompleteness);
      BENTO_PRINT("        Type=%d", seq.m_NaluType);
      const char* nalu_type_name = AP4_HevcNalParser::NaluTypeName(seq.m_NaluType);
      if (nalu_type_name) {
        BENTO_PRINT(" (%s)", nalu_type_name);
      }
      BENTO_PRINT("\n");
      for (unsigned int j = 0; j < seq.m_Nalus.ItemCount(); j++) {
        BENTO_PRINT("        ");
        ShowData(seq.m_Nalus[j]);
      }
      BENTO_PRINT("\n      }\n");
    }
    BENTO_PRINT("    Codecs String: ");
    AP4_String codecpar;
    hevc_desc->GetCodecString(codecpar);
    BENTO_PRINT("%s", codecpar.GetChars());
    BENTO_PRINT("\n");
  }

  // Dolby Vision specifics
  AP4_DvccAtom* dvcc = AP4_DYNAMIC_CAST(AP4_DvccAtom, desc->GetDetails().GetChild(AP4_ATOM_TYPE_DVCC));
  if (dvcc) {
    BENTO_PRINT("    Dolby Vision:\n");
    BENTO_PRINT("      Version:     %d.%d\n", dvcc->GetDvVersionMajor(), dvcc->GetDvVersionMinor());
    const char* profile_name = AP4_DvccAtom::GetProfileName(dvcc->GetDvProfile());
    if (profile_name) {
      BENTO_PRINT("      Profile:     %s\n", profile_name);
    }
    else {
      BENTO_PRINT("      Profile:     %d\n", dvcc->GetDvProfile());
    }
    BENTO_PRINT("      Level:       %d\n", dvcc->GetDvLevel());
    BENTO_PRINT("      RPU Present: %s\n", dvcc->GetRpuPresentFlag() ? "true" : "false");
    BENTO_PRINT("      EL Present:  %s\n", dvcc->GetElPresentFlag() ? "true" : "false");
    BENTO_PRINT("      BL Present:  %s\n", dvcc->GetBlPresentFlag() ? "true" : "false");
  }

  // Subtitles
  if (desc->GetType() == AP4_SampleDescription::TYPE_SUBTITLES) {
    AP4_SubtitleSampleDescription* subt_desc = AP4_DYNAMIC_CAST(AP4_SubtitleSampleDescription, desc);
    BENTO_PRINT("    Subtitles:\n");
    BENTO_PRINT("       Namespace:       %s\n", subt_desc->GetNamespace().GetChars());
    BENTO_PRINT("       Schema Location: %s\n", subt_desc->GetSchemaLocation().GetChars());
    BENTO_PRINT("       Image Mime Type: %s\n", subt_desc->GetImageMimeType().GetChars());
  }
}


/*----------------------------------------------------------------------
|   ReadGolomb
+---------------------------------------------------------------------*/
static unsigned int
ReadGolomb(AP4_BitStream& bits)
{
  unsigned int leading_zeros = 0;
  while (bits.ReadBit() == 0) {
    leading_zeros++;
  }
  if (leading_zeros) {
    return (1 << leading_zeros) - 1 + bits.ReadBits(leading_zeros);
  }
  else {
    return 0;
  }
}

/*----------------------------------------------------------------------
|   ShowAvcInfo
+---------------------------------------------------------------------*/
static void
ShowAvcInfo(const AP4_DataBuffer& sample_data, AP4_AvcSampleDescription* avc_desc)
{
  const unsigned char* data = sample_data.GetData();
  AP4_Size             size = sample_data.GetDataSize();

  while (size >= avc_desc->GetNaluLengthSize()) {
    unsigned int nalu_length = 0;
    if (avc_desc->GetNaluLengthSize() == 1) {
      nalu_length = *data++;
      --size;
    }
    else if (avc_desc->GetNaluLengthSize() == 2) {
      nalu_length = AP4_BytesToUInt16BE(data);
      data += 2;
      size -= 2;
    }
    else if (avc_desc->GetNaluLengthSize() == 4) {
      nalu_length = AP4_BytesToUInt32BE(data);
      data += 4;
      size -= 4;
    }
    else {
      return;
    }
    if (nalu_length <= size) {
      size -= nalu_length;
    }
    else {
      size = 0;
    }

    switch (*data & 0x1F) {
    case 1: {
      AP4_BitStream bits;
      bits.WriteBytes(data + 1, 8);
      ReadGolomb(bits);
      unsigned int slice_type = ReadGolomb(bits);
      switch (slice_type) {
      case 0: BENTO_PRINT("<P>");  break;
      case 1: BENTO_PRINT("<B>");  break;
      case 2: BENTO_PRINT("<I>");  break;
      case 3:	BENTO_PRINT("<SP>"); break;
      case 4: BENTO_PRINT("<SI>"); break;
      case 5: BENTO_PRINT("<P>");  break;
      case 6: BENTO_PRINT("<B>");  break;
      case 7: BENTO_PRINT("<I>");  break;
      case 8:	BENTO_PRINT("<SP>"); break;
      case 9: BENTO_PRINT("<SI>"); break;
      default: BENTO_PRINT("<S/%d>", slice_type); break;
      }
      return; // only show first slice type
    }

    case 5:
      BENTO_PRINT("<I>");
      return;
    }

    data += nalu_length;
  }
}


static void
ShowSample_Text(AP4_Track&      track,
  AP4_Sample&     sample,
  AP4_DataBuffer& sample_data,
  unsigned int    index,
  bool            verbose,
  bool            show_sample_data,
  AP4_AvcSampleDescription* avc_desc)
{
  BENTO_PRINT("[%06d] size=%6d duration=%6d",
    index + 1,
    (int)sample.GetSize(),
    (int)sample.GetDuration());
  if (verbose) {
    BENTO_PRINT(" (%6d ms) offset=%10lld dts=%10lld (%10lld ms) cts=%10lld (%10lld ms) [%d]",
      (int)AP4_ConvertTime(sample.GetDuration(), track.GetMediaTimeScale(), 1000),
      sample.GetOffset(),
      sample.GetDts(),
      AP4_ConvertTime(sample.GetDts(), track.GetMediaTimeScale(), 1000),
      sample.GetCts(),
      AP4_ConvertTime(sample.GetCts(), track.GetMediaTimeScale(), 1000),
      sample.GetDescriptionIndex());
  }
  if (sample.IsSync()) {
    BENTO_PRINT(" [S] ");
  }
  else {
    BENTO_PRINT("     ");
  }
  if (avc_desc || show_sample_data) {
    sample.ReadData(sample_data);
  }
  if (avc_desc) {
    ShowAvcInfo(sample_data, avc_desc);
  }
  if (show_sample_data) {
    unsigned int show = sample_data.GetDataSize();
    if (!verbose) {
      if (show > 12) show = 12; // max first 12 chars
    }

    for (unsigned int i = 0; i < show; i++) {
      if (verbose) {
        if (i % 16 == 0) {
          BENTO_PRINT("\n%06d: ", i);
        }
      }
      BENTO_PRINT("%02x", sample_data.GetData()[i]);
      if (verbose) BENTO_PRINT(" ");
    }
    if (show != sample_data.GetDataSize()) {
      BENTO_PRINT("...");
    }
  }
}


/*----------------------------------------------------------------------
|   ShowTrackInfo_Text
+---------------------------------------------------------------------*/
static void
ShowTrackInfo_Text(AP4_Movie& movie, AP4_Track& track, AP4_ByteStream& stream, bool show_samples, bool show_sample_data, bool verbose, bool fast)
{
  BENTO_PRINT("  flags:        %d", track.GetFlags());
  if (track.GetFlags() & AP4_TRACK_FLAG_ENABLED) {
    BENTO_PRINT(" ENABLED");
  }
  if (track.GetFlags() & AP4_TRACK_FLAG_IN_MOVIE) {
    BENTO_PRINT(" IN-MOVIE");
  }
  if (track.GetFlags() & AP4_TRACK_FLAG_IN_PREVIEW) {
    BENTO_PRINT(" IN-PREVIEW");
  }
  BENTO_PRINT("\n");
  BENTO_PRINT("  id:           %d\n", track.GetId());
  BENTO_PRINT("  type:         ");
  switch (track.GetType()) {//zx
  case AP4_Track::TYPE_AUDIO:     BENTO_PRINT("Audio\n");     break;
  case AP4_Track::TYPE_VIDEO:     BENTO_PRINT("Video\n");     break;
  case AP4_Track::TYPE_HINT:      BENTO_PRINT("Hint\n");      break;
  case AP4_Track::TYPE_SYSTEM:    BENTO_PRINT("System\n");    break;
  case AP4_Track::TYPE_TEXT:      BENTO_PRINT("Text\n");      break;
  case AP4_Track::TYPE_JPEG:      BENTO_PRINT("JPEG\n");      break;
  case AP4_Track::TYPE_SUBTITLES: BENTO_PRINT("Subtitles\n"); break;
  default: {
    char hdlr[5];
    AP4_FormatFourChars(hdlr, track.GetHandlerType());
    BENTO_PRINT("Unknown [");
    BENTO_PRINT("%s", hdlr);
    BENTO_PRINT("]\n");
    break;
  }
  }
  BENTO_PRINT("  duration: %d ms\n", track.GetDurationMs());
  BENTO_PRINT("  language: %s\n", track.GetTrackLanguage());
  BENTO_PRINT("  media:\n");
  BENTO_PRINT("    sample count: %d\n", track.GetSampleCount());
  BENTO_PRINT("    timescale:    %d\n", track.GetMediaTimeScale());
  BENTO_PRINT("    duration:     %lld (media timescale units)\n", track.GetMediaDuration());
  BENTO_PRINT("    duration:     %d (ms)\n", (AP4_UI32)AP4_ConvertTime(track.GetMediaDuration(), track.GetMediaTimeScale(), 1000));
  if (!fast) {
    MediaInfo media_info;
    ScanMedia(movie, track, stream, media_info);
    BENTO_PRINT("    bitrate (computed): %.3f Kbps\n", media_info.bitrate / 1000.0);
    if (movie.HasFragments()) {
      BENTO_PRINT("    sample count with fragments: %lld\n", media_info.sample_count);
      BENTO_PRINT("    duration with fragments:     %lld\n", media_info.duration);
      BENTO_PRINT("    duration with fragments:     %d (ms)\n", (AP4_UI32)AP4_ConvertTime(media_info.duration, track.GetMediaTimeScale(), 1000));
    }
  }
  if (track.GetWidth() || track.GetHeight()) {
    BENTO_PRINT("  display width:  %f\n", (float)track.GetWidth() / 65536.0);
    BENTO_PRINT("  display height: %f\n", (float)track.GetHeight() / 65536.0);
  }
  if (track.GetType() == AP4_Track::TYPE_VIDEO && track.GetSampleCount()) {
    BENTO_PRINT("  frame rate (computed): %.3f\n", (float)track.GetSampleCount() /
      ((float)track.GetMediaDuration() / (float)track.GetMediaTimeScale()));
  }

  // show all sample descriptions
  AP4_AvcSampleDescription* avc_desc = NULL;
  for (unsigned int desc_index = 0;
    AP4_SampleDescription* sample_desc = track.GetSampleDescription(desc_index);
    desc_index++) {
    BENTO_PRINT("  Sample Description %d\n", desc_index);
    ShowSampleDescription(*sample_desc, verbose);
    avc_desc = AP4_DYNAMIC_CAST(AP4_AvcSampleDescription, sample_desc);
  }

  // show samples if requested
  if (show_samples) {
    AP4_Sample     sample;
    AP4_DataBuffer sample_data;
    AP4_Ordinal    index = 0;
    while (AP4_SUCCEEDED(track.GetSample(index, sample))) {
      if (avc_desc || show_sample_data) {
        sample.ReadData(sample_data);
      }

      ShowSample_Text(track, sample, sample_data, index, verbose, show_sample_data, avc_desc);
      BENTO_PRINT("\n");
      index++;
    }
  }
}

static void
ShowTracks(AP4_Movie& movie, AP4_List<AP4_Track>& tracks, AP4_ByteStream& stream, bool show_samples, bool show_sample_data, bool verbose, bool fast)
{
  int index = 1;
  for (AP4_List<AP4_Track>::Item* track_item = tracks.FirstItem();
    track_item;
    track_item = track_item->GetNext(), ++index) {
    BENTO_PRINT("Track %d:\n", index);
    ShowTrackInfo_Text(movie, *track_item->GetData(), stream, show_samples, show_sample_data, verbose, fast);
  }
}

#endif