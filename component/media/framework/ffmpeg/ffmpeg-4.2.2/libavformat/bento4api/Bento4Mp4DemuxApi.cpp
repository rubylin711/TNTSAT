#include "Bento4Mp4Demux.h"
#include "Bento4Mp4DemuxApi.h"
#include "Bento4Mp4DemuxInner.h"


/****
gstreamer multithread need lock, mplayer dont need!!!!
****/
//static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
//#define MUTEX_LOCK()				    pthread_mutex_lock(&g_mutex)
//#define MUTEX_UNLOCK()					pthread_mutex_unlock(&g_mutex)

#define MUTEX_LOCK()
#define MUTEX_UNLOCK()


int Bento4Mp4DemuxApi_Prob(char* buffer, int len)
{
  return Bento4Mp4Demux::Probe(buffer,  len);
}

void* Bento4Mp4DemuxApi_ReadHeader(AVFormatContext *s)
{
  int ret = 0;
  Bento4Mp4Demux *cur_demux = new Bento4Mp4Demux();
  MUTEX_LOCK();
  ret = cur_demux->ReadHeader(s);
  MUTEX_UNLOCK();
  if(ret != 0)
  {
    delete cur_demux;
    return NULL;
  }
  return (void *)cur_demux;
}


int Bento4Mp4DemuxApi_ResetHeader(void *p_instance, AVFormatContext *s)
{
  int ret = 0;
  Bento4Mp4Demux *cur_demux = (Bento4Mp4Demux *)p_instance;
  MUTEX_LOCK();
  //printf("ResetHeader lock thread %u\n", (unsigned int)pthread_self());
  ret = cur_demux->ResetHeader(s);
  MUTEX_UNLOCK();
  //printf("ResetHeader unlock thread %u\n", (unsigned int)pthread_self());
  return ret;
}

int Bento4Mp4DemuxApi_Check_Switch_Streams(void *p_instance, AVStream *st)
{
    Bento4Mp4Demux *cur_demux = (Bento4Mp4Demux *)p_instance;
    MUTEX_LOCK();
    int ret = cur_demux->CheckSwitchStreams(st);
    MUTEX_UNLOCK();
    return ret;
}


int Bento4Mp4DemuxApi_ReadPacket(void *p_instance, AVStream *st, AVIndexEntry *sample, AVPacket *pkt)
{
    Bento4Mp4Demux *cur_demux = (Bento4Mp4Demux *)p_instance;
    MUTEX_LOCK();
    int ret = cur_demux->ReadPacket(st, sample, pkt);
    MUTEX_UNLOCK();
    return ret;
}

int Bento4Mp4DemuxApi_BuildIndexEntry(void *p_instance, AVStream *st)
{
    Bento4Mp4Demux *cur_demux = (Bento4Mp4Demux *)p_instance;
    MUTEX_LOCK();
    int ret = cur_demux->buildIndexEntry(st);
    MUTEX_UNLOCK();
    return ret;
}

int Bento4Mp4DemuxApi_BuildMulIndexEntry(void *p_instance, AVFormatContext *s)
{
    Bento4Mp4Demux *cur_demux = (Bento4Mp4Demux *)p_instance;
    MUTEX_LOCK();
    int ret = cur_demux->buildMulIndexEntry(s);
    MUTEX_UNLOCK();
    return ret;
}

int Bento4Mp4DemuxApi_GetNextStForMultiTracks(void *p_instance)
{
    Bento4Mp4Demux *cur_demux = (Bento4Mp4Demux *)p_instance;
    MUTEX_LOCK();
    int ret = cur_demux->GetNextStForMultiTracks();
    MUTEX_UNLOCK();
    return ret;
}

int Bento4Mp4DemuxApi_Seek(void *p_instance, s64 offset, s32 type)
{
    Bento4Mp4Demux *cur_demux = (Bento4Mp4Demux *)p_instance;
    MUTEX_LOCK();
    int ret = cur_demux->Seek(offset, type);
    MUTEX_UNLOCK();
    return ret;
}

int Bento4Mp4DemuxApi_CloseStream(void *p_instance, AVStream *st)
{
    Bento4Mp4Demux *cur_demux = (Bento4Mp4Demux *)p_instance;
    MUTEX_LOCK();
    cur_demux->close(st);
    MUTEX_UNLOCK();
    return 0;
}

int Bento4Mp4DemuxApi_Destroy(void *p_instance)
{
    Bento4Mp4Demux *cur_demux = (Bento4Mp4Demux *)p_instance;
    MUTEX_LOCK();
    if(cur_demux)
    {
        delete cur_demux;
        cur_demux = NULL;
    }
    MUTEX_UNLOCK();
    return 0;
}


