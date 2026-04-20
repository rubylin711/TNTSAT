/*
 * routines (with C-linkage) that interface between MPlayer
 * and the "LIVE555 Streaming Media" libraries
 *
 * This file is part of MPlayer.
 *
 * MPlayer is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * MPlayer is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with MPlayer; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#define RTSPCLIENT_SYNCHRONOUS_INTERFACE 1

extern "C" {
	// on MinGW, we must include windows.h before the things it conflicts
#ifdef __MINGW32__    // with.  they are each protected from
#include <windows.h>  // windows.h, but not the other way around.
#endif
#include "mp_msg.h"
#include "demuxer.h"
#include "demux_rtp.h"
#include "stheader.h"
	//extern int fprintf( FILE *stream, const char *format, ... );
	//extern FILE *stderr;
	//extern int printf(const char *format,...);
}
#include "demux_rtp_internal.h"

#include "BasicUsageEnvironment.hh"
#include "liveMedia.hh"
#include "GroupsockHelper.hh"
#include <unistd.h>

#ifndef __LINUX__
//#include "string.h"
// system
#include "mt_type.h"
#include "sys_define.h"
#include "mtos_printk.h"
#include "mtos_mem.h"
#else
#define OS_PRINTF printf
#endif
#include "mtos_misc.h"
#ifdef __LINUX__
#else
#include "mp_func_trans.h"
#endif
#include "file_playback_sequence.h"

//#define CONFIG_FFMPEG
#define DEBUG_PRINT_DISCARDED_PACKETS

#ifndef __LINUX__
int verbose = 0;
#endif

#ifdef RTSP_CLT_QUIT_DEBUG //debug client quit
int event_loop_cnt = 0;
int m2ts_ag_cnt=0;
int ticks_array[1024] = {0};
int ticks_frame_end[2] = {0};
int rtp_seq_no_bak = 0;
int recv_len_debug = 0;
int recv_cnt_g=0;
int tick_start = 0;
int tick_start_flag = 0;
extern int flag_rtsp_fill_start, while_cnt;

#endif

// A data structure representing input data for each stream:
class ReadBufferQueue {
	public:
		ReadBufferQueue(MediaSubsession* subsession, demuxer_t* demuxer,
				char const* tag);
		virtual ~ReadBufferQueue();

		FramedSource* readSource() const { return fReadSource; }
		RTPSource* rtpSource() const { return fRTPSource; }
		demuxer_t* ourDemuxer() const { return fOurDemuxer; }
		char const* tag() const { return fTag; }

		char blockingFlag = 0; // used to implement synchronous reads

		// For A/V synchronization:
		Boolean prevPacketWasSynchronized;
		float prevPacketPTS;
		ReadBufferQueue** otherQueue;

		// The 'queue' actually consists of just a single "demux_packet_t"
		// (because the underlying OS does the actual queueing/buffering):
		demux_packet_t* dp;

		// However, we sometimes inspect buffers before delivering them.
		// For this, we maintain a queue of pending buffers:
		void savePendingBuffer(demux_packet_t* dp);
		demux_packet_t* getPendingBuffer();

		// For H264 over rtsp using AVParser, the next packet has to be saved
		demux_packet_t* nextpacket;

	private:
		demux_packet_t* pendingDPHead;
		demux_packet_t* pendingDPTail;

		FramedSource* fReadSource;
		RTPSource* fRTPSource;
		demuxer_t* fOurDemuxer;
		char const* fTag; // used for debugging
};

// A structure of RTP-specific state, kept so that we can cleanly
// reclaim it:
struct RTPState {
	char const* sdpDescription;
	RTSPClient* rtspClient;
	SIPClient* sipClient;
	MediaSession* mediaSession;
	ReadBufferQueue* audioBufferQueue;
	ReadBufferQueue* videoBufferQueue;
	unsigned flags;
	struct timeval firstSyncTime;
};

extern "C" char* network_username;
extern "C" char* network_password;
static char* openURL_rtsp(RTSPClient* client, char const* url) {
	// If we were given a user name (and optional password), then use them:
	//OS_PRINTF("[%s]------------start!\n",__func__);
	if (network_username != NULL) {
		OS_PRINTF("[%s]------------network_username != NULL\n",__func__);
		char const* password = network_password == NULL ? "" : network_password;
		return client->describeWithPassword(url, network_username, password);
	} else {
		OS_PRINTF("[%s]------------network_username == NULL\n",__func__);
		return client->describeURL(url);
	}
}

static char* openURL_sip(SIPClient* client, char const* url) {
	// If we were given a user name (and optional password), then use them:
	//OS_PRINTF("[%s]------------start!\n",__func__);
	if (network_username != NULL) {
		OS_PRINTF("[%s]------------network_username != NULL\n",__func__);
		char const* password = network_password == NULL ? "" : network_password;
		return client->inviteWithPassword(url, network_username, password);
	} else {
		OS_PRINTF("[%s]------------network_username == NULL\n",__func__);
		return client->invite(url);
	}
}

#ifdef CONFIG_LIBNEMESI
extern int rtsp_transport_tcp;
extern int rtsp_transport_http;
#else
int rtsp_transport_tcp = 0;
int rtsp_transport_http = 0;
#endif

#ifdef CONFIG_FFMPEG_MP
extern AVCodecContext *avcctx;
#endif

extern "C" int audio_id, video_id, dvdsub_id;
extern "C" demuxer_t* demux_open_rtp(demuxer_t* demuxer) {
	Boolean success = False;
       FILE_SEQ_T * p_file_seq =  file_seq_get_instance();
       rtsp_transport_tcp = p_file_seq->rtsp_by_tcp_udp;

	//OS_PRINTF("[%s]------------start!\n",__func__);

    

	do {
		//OS_PRINTF("[%s]------------come in do while !\n",__func__);
		TaskScheduler* scheduler = BasicTaskScheduler::createNew();
		OS_PRINTF("[%s]------------call BasicTaskScheduler::createNew(), return: %x\n",__func__,scheduler);
		if (scheduler == NULL) break;
		UsageEnvironment* env = BasicUsageEnvironment::createNew(*scheduler);
		OS_PRINTF("[%s]------------call BasicUsageEnvironment::createNew, return: %x\n",__func__,env);
		if (env == NULL) break;

		RTSPClient* rtspClient = NULL;
		SIPClient* sipClient = NULL;

		if (demuxer == NULL || demuxer->stream == NULL) {
			OS_PRINTF("[%s]------------This shouldn't happen! demuxer=%x, demuxer->stream =%x\n",__func__,demuxer,demuxer->stream);
			break;  // shouldn't happen
		}
		demuxer->stream->eof = 0; // just in case

		// Look at the stream's 'priv' field to see if we were initiated
		// via a SDP description:
		char* sdpDescription = (char*)(demuxer->stream->priv);
		if (sdpDescription == NULL) {
			// We weren't given a SDP description directly, so assume that
			// we were given a RTSP or SIP URL:
			char const* protocol = demuxer->stream->streaming_ctrl->url->protocol;
			char const* url = demuxer->stream->streaming_ctrl->url->url;
			if (strcmp(protocol, "rtsp") == 0) {
				if (rtsp_transport_http == 1) {
					rtsp_transport_http = demuxer->stream->streaming_ctrl->url->port;
					rtsp_transport_tcp = 1;
				}
				rtspClient = RTSPClient::createNew(*env, verbose, "MPlayer", rtsp_transport_http);
				if (rtspClient == NULL) {
#ifdef __LINUX__		
					fprintf(stderr, "Failed to create RTSP client: %s\n",
							env->getResultMsg());
#else
					OS_PRINTF( "Failed to create RTSP client: %s\n",
							env->getResultMsg());
#endif
					break;
				}
				sdpDescription = openURL_rtsp(rtspClient, url);
			} else { // SIP
				unsigned char desiredAudioType = 0; // PCMU (use 3 for GSM)
				sipClient = SIPClient::createNew(*env, desiredAudioType, NULL,
						verbose, "MPlayer");
				if (sipClient == NULL) {
#ifdef __LINUX__		
					fprintf(stderr, "Failed to create SIP client: %s\n",
							env->getResultMsg());
#else
					OS_PRINTF( "Failed to create SIP client: %s\n",
							env->getResultMsg());
#endif
					break;
				}
				sipClient->setClientStartPortNum(8000);
				sdpDescription = openURL_sip(sipClient, url);
			}

			if (sdpDescription == NULL) {
#ifdef __LINUX__	  	
				fprintf(stderr, "Failed to get a SDP description from URL \"%s\": %s\n",
						url, env->getResultMsg());
#else
				OS_PRINTF("Failed to get a SDP description from URL \"%s\": %s\n",
						url, env->getResultMsg());
#endif

				break;
			}
		}

		OS_PRINTF("[%s]------------ call MediaSession::createNew(*env, sdpDescription)!\n",__func__);
		// Now that we have a SDP description, create a MediaSession from it:
		MediaSession* mediaSession = MediaSession::createNew(*env, sdpDescription);
		if (mediaSession == NULL) {
			OS_PRINTF("[%s]------------ mediaSession == NULL!\n",__func__);
			break;
		}


		// Create a 'RTPState' structure containing the state that we just created,
		// and store it in the demuxer's 'priv' field, for future reference:
		RTPState* rtpState = new RTPState;
		rtpState->sdpDescription = sdpDescription;
		rtpState->rtspClient = rtspClient;
		rtpState->sipClient = sipClient;
		rtpState->mediaSession = mediaSession;
		rtpState->audioBufferQueue = rtpState->videoBufferQueue = NULL;
		rtpState->flags = 0;
		rtpState->firstSyncTime.tv_sec = rtpState->firstSyncTime.tv_usec = 0;
		demuxer->priv = rtpState;

		int audiofound = 0, videofound = 0;
		// Create RTP receivers (sources) for each subsession:
		MediaSubsessionIterator iter(*mediaSession);
		MediaSubsession* subsession;
		unsigned desiredReceiveBufferSize;
		while ((subsession = iter.next()) != NULL) {
			//OS_PRINTF("[%s]------------ come in while()\n",__func__);
			// Ignore any subsession that's not audio or video:
			if (strcmp(subsession->mediumName(), "audio") == 0) {
				if (audiofound) {
#ifdef __LINUX__	  			
					fprintf(stderr, "Additional subsession \"audio/%s\" skipped\n", subsession->codecName());
#else
					OS_PRINTF("Additional subsession \"audio/%s\" skipped\n", subsession->codecName());
#endif
					continue;
				}
				desiredReceiveBufferSize = 100000;
			} else if (strcmp(subsession->mediumName(), "video") == 0) {
				if (videofound) {
#ifdef __LINUX__	  			
					fprintf(stderr, "Additional subsession \"video/%s\" skipped\n", subsession->codecName());
#else
					OS_PRINTF("Additional subsession \"video/%s\" skipped\n", subsession->codecName());
#endif
					continue;
				}
				desiredReceiveBufferSize = 2000000;
			} else {
				continue;
			}

			if (rtsp_port)
				subsession->setClientPortNum (rtsp_port);

			if (!subsession->initiate()) {
#ifdef __LINUX__	  	
				fprintf(stderr, "Failed to initiate \"%s/%s\" RTP subsession: %s\n", subsession->mediumName(), subsession->codecName(), env->getResultMsg());
#else
				OS_PRINTF("Failed to initiate \"%s/%s\" RTP subsession: %s\n", subsession->mediumName(), subsession->codecName(), env->getResultMsg());
#endif
			} else {
#ifdef __LINUX__	  	
				fprintf(stderr, "Initiated \"%s/%s\" RTP subsession on port %d\n", subsession->mediumName(), subsession->codecName(), subsession->clientPortNum());
#else
				OS_PRINTF("Initiated \"%s/%s\" RTP subsession on port %d\n", subsession->mediumName(), subsession->codecName(), subsession->clientPortNum());
#endif

				// Set the OS's socket receive buffer sufficiently large to avoid
				// incoming packets getting dropped between successive reads from this
				// subsession's demuxer.  Depending on the bitrate(s) that you expect,
				// you may wish to tweak the "desiredReceiveBufferSize" values above.
				int rtpSocketNum = subsession->rtpSource()->RTPgs()->socketNum();
				int receiveBufferSize
					= increaseReceiveBufferTo(*env, rtpSocketNum,
							desiredReceiveBufferSize);
				if (verbose > 0) {
#ifdef __LINUX__	  			
					fprintf(stderr, "Increased %s socket receive buffer to %d bytes \n",
							subsession->mediumName(), receiveBufferSize);
#else
					OS_PRINTF("Increased %s socket receive buffer to %d bytes \n",
							subsession->mediumName(), receiveBufferSize);
#endif
				}

				if (rtspClient != NULL) {
					// Issue a RTSP "SETUP" command on the chosen subsession:
					if (!rtspClient->setupMediaSubsession(*subsession, False,
								rtsp_transport_tcp)) break;
					if (!strcmp(subsession->mediumName(), "audio"))
						audiofound = 1;
					if (!strcmp(subsession->mediumName(), "video"))
						videofound = 1;
				}
			}
		}
		//OS_PRINTF("[%s]--------------------out of while ((subsession = iter.next()) != NULL) \n", __func__);


		if (rtspClient != NULL) {
			// Issue a RTSP aggregate "PLAY" command on the whole session:
			if (!rtspClient->playMediaSession(*mediaSession)) break;
		} else if (sipClient != NULL) {
			sipClient->sendACK(); // to start the stream flowing
		}
		//OS_PRINTF("[%s]--------------------after  if (rtspClient != NULL) \n", __func__);
		// Now that the session is ready to be read, do additional
		// MPlayer codec-specific initialization on each subsession:
		iter.reset();
		while ((subsession = iter.next()) != NULL) {
			if (subsession->readSource() == NULL) continue; // not reading this

			unsigned flags = 0;
			if (strcmp(subsession->mediumName(), "audio") == 0) {
				rtpState->audioBufferQueue
					= new ReadBufferQueue(subsession, demuxer, "audio");
				rtpState->audioBufferQueue->otherQueue = &(rtpState->videoBufferQueue);
				rtpCodecInitialize_audio(demuxer, subsession, flags);
			} else if (strcmp(subsession->mediumName(), "video") == 0) {
				rtpState->videoBufferQueue
					= new ReadBufferQueue(subsession, demuxer, "video");
				rtpState->videoBufferQueue->otherQueue = &(rtpState->audioBufferQueue);
				rtpCodecInitialize_video(demuxer, subsession, flags);
			}
			rtpState->flags |= flags;
		}
		success = True;
	} while (0);

	//printf("[%s]------------out of while, success=%d!",__func__,success);
	if (!success) return NULL; // an error occurred

	// Hack: If audio and video are demuxed together on a single RTP stream,
	// then create a new "demuxer_t" structure to allow the higher-level
	// code to recognize this:
	if (demux_is_multiplexed_rtp_stream(demuxer)) {
		stream_t* s = new_ds_stream(demuxer->video);
		OS_PRINTF("[%s] --demux_is_multiplexed_rtp_stream\n", __func__);
		demuxer_t* od = demux_open(s, DEMUXER_TYPE_UNKNOWN,
				audio_id, video_id, dvdsub_id, NULL);
		demuxer = new_demuxers_demuxer(od, od, od);

	}
	OS_PRINTF("[%s] --demuxer: %p, return\n", __func__,demuxer);
	return demuxer;
}

extern "C" int demux_is_mpeg_rtp_stream(demuxer_t* demuxer) {
	// Get the RTP state that was stored in the demuxer's 'priv' field:
	RTPState* rtpState = (RTPState*)(demuxer->priv);

	return (rtpState->flags&RTPSTATE_IS_MPEG12_VIDEO) != 0;
}

extern "C" int demux_is_multiplexed_rtp_stream(demuxer_t* demuxer) {
	// Get the RTP state that was stored in the demuxer's 'priv' field:
	RTPState* rtpState = (RTPState*)(demuxer->priv);

	return (rtpState->flags&RTPSTATE_IS_MULTIPLEXED) != 0;
}

static demux_packet_t* getBuffer(demuxer_t* demuxer, demux_stream_t* ds,
		Boolean mustGetNewData,
		float& ptsBehind); // forward

extern "C" int demux_rtp_fill_buffer(demuxer_t* demuxer, demux_stream_t* ds) {
	// Get a filled-in "demux_packet" from the RTP source, and deliver it.
	// Note that this is called as a synchronous read operation, so it needs
	// to block in the (hopefully infrequent) case where no packet is
	// immediately available.

	//OS_PRINTF("\n=============demux_rtp_fill_buffer   start!\n");
	int repeat_read_cnt = 0;
	while (1) {
		
		float ptsBehind;
		demux_packet_t* dp = getBuffer(demuxer, ds, False, ptsBehind); // blocking
		
		//OS_PRINTF("[%s]-----------out of getBuffer, dp->len = %d, dp->pts: %d\n",__func__,dp->len,(int)(dp->pts*1000));

		if (demuxer->stream->eof) {
			OS_PRINTF("[%s]-----------source stream has closed down \n",__func__);
			return 0; // source stream has closed down

		}		
		if (dp == NULL) {
			repeat_read_cnt ++;
			OS_PRINTF("[%s]-----------dp == NULL, continue\n",__func__);
			if(repeat_read_cnt > 5){
#if 0//ndef __LINUX__//add for debug memory use
				mem_user_dbg_info_t dbg_info;
				mtos_mem_user_debug(&dbg_info);
				OS_PRINTF("[%s]-1-- alloced[%d], alloced_peak[%d], rest_size[%d]\n",__func__,dbg_info.alloced, dbg_info.alloced_peak,dbg_info.rest_size);
				//OS_PRINTF("[%s]---total_malloc_size[%d]\n",__func__,total_malloc_size);
#endif
				return 0;
			}
			continue;
			//return 0;
		}

#if 0//del by doreen at 2013-07-09. Always add the packet.
		// Before using this packet, check to make sure that its presentation
		// time is not far behind the other stream (if any).  If it is,
		// then we discard this packet, and get another instead.  (The rest of
		// MPlayer doesn't always do a good job of synchronizing when the
		// audio and video streams get this far apart.)
		// (We don't do this when streaming over TCP, because then the audio and
		// video streams are interleaved.)
		// (Also, if the stream is *excessively* far behind, then we allow
		// the packet, because in this case it probably means that there was
		// an error in the source's timestamp synchronization.)
		const float ptsBehindThreshold = 1.0; // seconds
		const float ptsBehindLimit = 60.0; // seconds
		if (ptsBehind < ptsBehindThreshold ||
				ptsBehind > ptsBehindLimit ||
				rtsp_transport_tcp) 
#endif
		{ // packet's OK
			ds_add_packet(ds, dp);
			//OS_PRINTF("[%s]=============ds_add_packet!\n",__func__);
			break;
		}


#ifdef DEBUG_PRINT_DISCARDED_PACKETS

		RTPState* rtpState = (RTPState*)(demuxer->priv);
		ReadBufferQueue* bufferQueue = ds == demuxer->video ? rtpState->videoBufferQueue : rtpState->audioBufferQueue;
#ifdef __LINUX__	  	
		fprintf(stderr, "Discarding %s packet (%fs behind)\n", bufferQueue->tag(), ptsBehind);
#else
		OS_PRINTF( "Discarding %s packet (%fs behind)\n", bufferQueue->tag(), ptsBehind);
#endif

#endif
		free_demux_packet(dp); // give back this packet, and get another one
		
	}
	
	//OS_PRINTF("=============demux_rtp_fill_buffer   end!\n");
	return 1;
	
}

Boolean awaitRTPPacket(demuxer_t* demuxer, demux_stream_t* ds,
		unsigned char*& packetData, unsigned& packetDataLen,
		float& pts) {
	// Similar to "demux_rtp_fill_buffer()", except that the "demux_packet"
	// is not delivered to the "demux_stream".
	float ptsBehind;
	demux_packet_t* dp = getBuffer(demuxer, ds, True, ptsBehind); // blocking
	if (dp == NULL) return False;

	packetData = dp->buffer;
	packetDataLen = dp->len;
	pts = dp->pts;

	return True;
}

static void teardownRTSPorSIPSession(RTPState* rtpState); // forward

extern "C" void demux_close_rtp(demuxer_t* demuxer) {
	// Reclaim all RTP-related state:

	// Get the RTP state that was stored in the demuxer's 'priv' field:
	RTPState* rtpState = (RTPState*)(demuxer->priv);
	if (rtpState == NULL) return;

	teardownRTSPorSIPSession(rtpState);

	UsageEnvironment* env = NULL;
	TaskScheduler* scheduler = NULL;
	if (rtpState->mediaSession != NULL) {
		env = &(rtpState->mediaSession->envir());
		scheduler = &(env->taskScheduler());
	}
	Medium::closeMedium(rtpState->mediaSession);
	Medium::closeMedium(rtpState->rtspClient);
	Medium::closeMedium(rtpState->sipClient);
	delete rtpState->audioBufferQueue;
	delete rtpState->videoBufferQueue;
	delete[] rtpState->sdpDescription;
	delete rtpState;
#ifdef CONFIG_FFMPEG_MP
	av_freep(&avcctx);
#endif

	env->reclaim(); delete scheduler;
}

////////// Extra routines that help implement the above interface functions:

#define MAX_RTP_FRAME_SIZE 500000//5000000
// >= the largest conceivable frame composed from one or more RTP packets

static void afterReading(void* clientData, unsigned frameSize,
		unsigned /*numTruncatedBytes*/,
		struct timeval presentationTime,
		unsigned /*durationInMicroseconds*/) {
		
	int headersize = 0;
	
	if (frameSize >= MAX_RTP_FRAME_SIZE) {
#ifdef __LINUX__	  	  	
		fprintf(stderr, "Saw an input frame too large (>=%d).  Increase MAX_RTP_FRAME_SIZE in \"demux_rtp.cpp\".\n",
				MAX_RTP_FRAME_SIZE);
#else
		OS_PRINTF("Saw an input frame too large (>=%d).  Increase MAX_RTP_FRAME_SIZE in \"demux_rtp.cpp\".\n",
				MAX_RTP_FRAME_SIZE);
#endif
	}
	
	ReadBufferQueue* bufferQueue = (ReadBufferQueue*)clientData;
	demuxer_t* demuxer = bufferQueue->ourDemuxer();
	RTPState* rtpState = (RTPState*)(demuxer->priv);

	if (frameSize > 0) 
		demuxer->stream->eof = 0;


	demux_packet_t* dp = bufferQueue->dp;


	if (bufferQueue->readSource()->isAMRAudioSource())
		headersize = 1;
	else if (bufferQueue == rtpState->videoBufferQueue &&
			((sh_video_t*)demuxer->video->sh)->format == mmioFOURCC('H','2','6','4')) {
			
		dp->buffer[0]=0x00;
		dp->buffer[1]=0x00;
		dp->buffer[2]=0x01;
		headersize = 3;
	}

	resize_demux_packet(dp, frameSize + headersize);

	// Set the packet's presentation time stamp, depending on whether or
	// not our RTP source's timestamps have been synchronized yet:
	Boolean hasBeenSynchronized
		= bufferQueue->rtpSource()->hasBeenSynchronizedUsingRTCP();
	//OS_PRINTF("[%s]-- hasBeenSynchronized: %d\n",__func__, hasBeenSynchronized);
	hasBeenSynchronized = 1;
	if (hasBeenSynchronized) {
		
		if (verbose > 0 && !bufferQueue->prevPacketWasSynchronized) {
#ifdef __LINUX__	  			
			fprintf(stderr, "%s stream has been synchronized using RTCP \n",
					bufferQueue->tag());
#else
			OS_PRINTF("%s stream has been synchronized using RTCP \n",
					bufferQueue->tag());
#endif
		}

		struct timeval* fst = &(rtpState->firstSyncTime); // abbrev
		if (fst->tv_sec == 0 && fst->tv_usec == 0) {
			*fst = presentationTime;
			OS_PRINTF("[%s] -------- update fst with present time [%d %d]\n",__func__,\
	(int)(fst->tv_sec),(int)(fst->tv_usec));
		}

		// For the "pts" field, use the time differential from the first
		// synchronized time, rather than absolute time, in order to avoid
		// round-off errors when converting to a float:
		dp->pts = presentationTime.tv_sec - fst->tv_sec
			+ (presentationTime.tv_usec - fst->tv_usec)/1000000.0;
		//if(dp->pts < 0)
			//OS_PRINTF("[%s] -------- dp->pts: %d prst[%d %d] fst[%d %d]\n",__func__,\
	//(int)(dp->pts*1000),(int)(presentationTime.tv_sec),(int)(presentationTime.tv_usec),(int)(fst->tv_sec),(int)(fst->tv_usec));
		
		bufferQueue->prevPacketPTS = dp->pts;
		
	} else {
	
		if (verbose > 0 && bufferQueue->prevPacketWasSynchronized) {
#ifdef __LINUX__	  			
			fprintf(stderr, "%s stream is no longer RTCP-synchronized \n",
					bufferQueue->tag());
#else
			OS_PRINTF("%s stream is no longer RTCP-synchronized \n",
					bufferQueue->tag());
#endif
		}

		// use the previous packet's "pts" once again:
		dp->pts = bufferQueue->prevPacketPTS;
		//OS_PRINTF("[%s]-- use pre pts, dp->pts: %d\n",__func__, (int)(dp->pts*1000));
	}
	
	bufferQueue->prevPacketWasSynchronized = hasBeenSynchronized;

	dp->pos = demuxer->filepos;
	demuxer->filepos += frameSize + headersize;

	// Signal any pending 'doEventLoop()' call on this queue:
	bufferQueue->blockingFlag = ~0;
#ifdef RTSP_CLT_QUIT_DEBUG
{
        int ticks = mtos_ticks_get();
        ticks_frame_end[0] = ticks_frame_end[1];
        ticks_frame_end[1] = ticks;
        int delta = (ticks_frame_end[1]-ticks_frame_end[0]);
        if(delta > 100)
        OS_PRINTF("[%s]-- frame ticks: %d, dp->len:%d\n",__func__,delta, dp->len);
       /* if(while_cnt>50 && event_loop_cnt>30){
                bufferQueue = NULL;
                bufferQueue->blockingFlag = ~0;
            }*/
}
#endif
	//OS_PRINTF("[%s]-- demuxer->filepos: %lld, frameSize: %d, headersize: %d\n",__func__,demuxer->filepos, frameSize, headersize);
}

static void onSourceClosure(void* clientData) {
	ReadBufferQueue* bufferQueue = (ReadBufferQueue*)clientData;
	demuxer_t* demuxer = bufferQueue->ourDemuxer();

	demuxer->stream->eof = 1;
	OS_PRINTF("[%s]----------------demuxer->stream->eof = 1!\n",__func__);	
#if 0// doreen debug    
       demuxer->stream = NULL;
       demuxer->stream->eof = 1;
#endif       
	// Signal any pending 'doEventLoop()' call on this queue:
	bufferQueue->blockingFlag = ~0;
}

int flag_has_add_h264_header = 0;

static demux_packet_t* getBuffer(demuxer_t* demuxer, demux_stream_t* ds,
		Boolean mustGetNewData,
		float& ptsBehind) {

	//OS_PRINTF("[%s]-----------------------start!\n",__func__);				 
	// Begin by finding the buffer queue that we want to read from:
	// (Get this from the RTP state, which we stored in
	//  the demuxer's 'priv' field)
	RTPState* rtpState = (RTPState*)(demuxer->priv);
	ReadBufferQueue* bufferQueue = NULL;
	int headersize = 0;
	int waitboth = 0;
	TaskToken task, task2;
        int 	ticks_pre=0;
        double play_end_time = 0;
        FILE_SEQ_T * p_file_seq =  file_seq_get_instance();

	if (demuxer->stream->eof) {
		OS_PRINTF("[%s]-----------------------demuxer->stream->eof!\n",__func__);	
		return NULL;
	}

	if (ds == demuxer->video) {
		
		bufferQueue = rtpState->audioBufferQueue;
		
		// HACK: for the latest versions we must also receive audio
		// when probing for video FPS, otherwise the stream just hangs
		// and times out
		if (mustGetNewData &&
				bufferQueue &&
				bufferQueue->readSource() &&
				!bufferQueue->nextpacket) {
			headersize = bufferQueue->readSource()->isAMRAudioSource() ? 1 : 0;
			demux_packet_t *dp = new_demux_packet(MAX_RTP_FRAME_SIZE);
			bufferQueue->dp = dp;
			bufferQueue->blockingFlag = 0;
			bufferQueue->readSource()->getNextFrame(
					&dp->buffer[headersize], MAX_RTP_FRAME_SIZE - headersize,
					afterReading, bufferQueue,
					onSourceClosure, bufferQueue);
			task2 = bufferQueue->readSource()->envir().taskScheduler().
				scheduleDelayedTask(10000000, onSourceClosure, bufferQueue);
			waitboth = 1;
		}
		
		bufferQueue = rtpState->videoBufferQueue;

		//OS_PRINTF("[%s]=============format: 0x%x\n",__func__,((sh_video_t*)ds->sh)->format);
		if (((sh_video_t*)ds->sh)->format == mmioFOURCC('H','2','6','4'))
			headersize = 3;
		
		//OS_PRINTF("[%s]=============headersize=%d!\n",__func__,headersize);
		
	} else if (ds == demuxer->audio) {
	
		bufferQueue = rtpState->audioBufferQueue;
		if (bufferQueue->readSource()->isAMRAudioSource())
			headersize = 1;
		
	} else {
#ifdef __LINUX__	  	
		fprintf(stderr, "(demux_rtp)getBuffer: internal error: unknown stream\n");
#else
		OS_PRINTF("(demux_rtp)getBuffer: internal error: unknown stream\n");
#endif
		return NULL;
	}
	

	if (bufferQueue == NULL || bufferQueue->readSource() == NULL) {
#ifdef __LINUX__	  	  	
		fprintf(stderr, "(demux_rtp)getBuffer failed: no appropriate RTP subsession has been set up\n");
#else
		OS_PRINTF("(demux_rtp)getBuffer failed: no appropriate RTP subsession has been set up\n");
#endif
		return NULL;
	}
	

	demux_packet_t* dp = NULL;
	if (!mustGetNewData) {
		// Check whether we have a previously-saved buffer that we can use:
		dp = bufferQueue->getPendingBuffer();
		if (dp != NULL) {
			ptsBehind = 0.0; // so that we always accept this data
			//OS_PRINTF("[%s]-1--dp->len = %d, dp->pts: %d\n",__func__,dp->len,(int)(dp->pts*1000));
			return dp;
		}
	}


	// Allocate a new packet buffer, and arrange to read into it:
	if (!bufferQueue->nextpacket) {
		//OS_PRINTF("[%s]-----new_demux_packet \n",__func__);
		dp = new_demux_packet(MAX_RTP_FRAME_SIZE);
		bufferQueue->dp = dp;
		if (dp == NULL){
			OS_PRINTF("[%s]----------------------dp==NULL\n",__func__);
			return NULL;
		}
	}

	//OS_PRINTF("[%s]-2--dp->len = %d, dp->pts: %d\n",__func__,dp->len,(int)(dp->pts*1000));

#if   0//def CONFIG_FFMPEG   peacer del 20130821
	extern AVCodecParserContext * h264parserctx;
	int consumed, poutbuf_size = 1;
	const uint8_t *poutbuf = NULL;
	float lastpts = 0.0;

	do {
		if (!bufferQueue->nextpacket) {
#endif
			// Schedule the read operation:
			//OS_PRINTF("[%s]--1---start to  getNextFrame\n",__func__);
			bufferQueue->blockingFlag = 0;
			bufferQueue->readSource()->getNextFrame(&dp->buffer[headersize], MAX_RTP_FRAME_SIZE - headersize,
					afterReading, bufferQueue,
					onSourceClosure, bufferQueue);
			
			//OS_PRINTF("[%s]--2---Block ourselves until data becomes available\n",__func__);
			// Block ourselves until data becomes available:
			TaskScheduler& scheduler
				= bufferQueue->readSource()->envir().taskScheduler();
			
			int delay = p_file_seq->rtsp_clt_quit_delay;      //10000000;
                     play_end_time = rtpState->mediaSession->playEndTime();
			if ((play_end_time > 0) && (bufferQueue->prevPacketPTS * 1.05 > play_end_time)){
                            //OS_PRINTF("[%s] prevPacketPTS: %d, playEndTime: %d\n",__func__, (int)(bufferQueue->prevPacketPTS*1000), (int)(rtpState->mediaSession->playEndTime()*1000));
				delay /= 10;
				//delay /= 2;
                    }
			//OS_PRINTF("[%s] delay:%d, prevPacketPTS: %d, playEndTime: %d\n",__func__,delay, (int)(bufferQueue->prevPacketPTS*1000), (int)(rtpState->mediaSession->playEndTime()*1000));
			task = scheduler.scheduleDelayedTask(delay, onSourceClosure, bufferQueue);
#ifdef RTSP_CLT_QUIT_DEBUG            
                     ticks_pre = mtos_ticks_get();
                     if(tick_start_flag == 0){
                             tick_start = ticks_pre;
                             tick_start_flag = 1;
                     }
#endif
			
			//OS_PRINTF("[%s]--3---call doEventLoop, blockingFlag: %d\n",__func__, bufferQueue->blockingFlag);
			scheduler.doEventLoop(&bufferQueue->blockingFlag);
#ifdef RTSP_CLT_QUIT_DEBUG//debug quit
                    if (demuxer->stream->eof) {
                		OS_PRINTF("[%s] @1@ %d\n",__func__,__LINE__);	
#if 1	//debug client quit
                            {
                            		int ticks = mtos_ticks_get();
                            		int delta_ticks = (ticks - ticks_pre)*10;
                            		OS_PRINTF("[%s] delta_ticks: %d ms, %d,%d\n",__func__, delta_ticks, ticks, ticks_pre);
                                          OS_PRINTF("[%s] event_loop_cnt %d\n",__func__, event_loop_cnt);
                                          /*for(int i=0; i<event_loop_cnt; i++)
                                                OS_PRINTF("[%s] i=%d: %d \n",__func__, i, ticks_array[i]);*/
                                          
                                          //OS_PRINTF("[%s] receive last frame consume ticks:  %d,%d\n",__func__, ticks_frame_end[0], ticks_frame_end[1]);
                                          OS_PRINTF("[%s] rtp_seq_no_bak: %d \n",__func__, rtp_seq_no_bak);
                                          OS_PRINTF("[%s] file playback run %d ms\n",__func__, (ticks-tick_start_flag)*10);
                                          tick_start_flag = 0;
                            		
                            }
#endif

                	}
#endif
			scheduler.unscheduleDelayedTask(task);
#ifdef RTSP_CLT_QUIT_DEBUG//debug quit
                    if (demuxer->stream->eof) {
                		OS_PRINTF("[%s] @2@ %d\n",__func__,__LINE__);	
                	}
#endif
			if (waitboth) {
				OS_PRINTF("[%s]-------------waitboth:%d,  doEventLoop\n",__func__, waitboth);
				scheduler.doEventLoop(&rtpState->audioBufferQueue->blockingFlag);
#ifdef RTSP_CLT_QUIT_DEBUG//debug quit
                            if (demuxer->stream->eof) {
                        		OS_PRINTF("[%s] @3@ %d\n",__func__,__LINE__);	
                        	}
#endif
				scheduler.unscheduleDelayedTask(task2);
#ifdef RTSP_CLT_QUIT_DEBUG//debug quit
                    if (demuxer->stream->eof) {
                		OS_PRINTF("[%s] @4@ %d\n",__func__,__LINE__);	
                	}
#endif
			}
			
			//OS_PRINTF("[%s]-3--dp->len = %d, dp->pts: %d\n",__func__,dp->len,(int)(dp->pts*1000));
			
			if (demuxer->stream->eof) {
				OS_PRINTF("[%s]-------------if (demuxer->stream->eof) free_demux_packet \n",__func__);
				free_demux_packet(dp);
				return NULL;
			}

			if (headersize == 1) // amr
				dp->buffer[0] = ((AMRAudioSource*)bufferQueue->readSource())->lastFrameHeader();


#if   0//def CONFIG_FFMPEG    peacer del 20130821
		} else {
			bufferQueue->dp = dp = bufferQueue->nextpacket;
			bufferQueue->nextpacket = NULL;
		}
		if (headersize == 3 && h264parserctx) { // h264
			consumed = h264parserctx->parser->parser_parse(h264parserctx,
					avcctx,
					&poutbuf, &poutbuf_size,
					dp->buffer, dp->len);

			if (!consumed && !poutbuf_size)
				return NULL;

			if (!poutbuf_size) {
				lastpts=dp->pts;
				free_demux_packet(dp);
				bufferQueue->dp = dp = new_demux_packet(MAX_RTP_FRAME_SIZE);
			} else {
				bufferQueue->nextpacket = dp;
				bufferQueue->dp = dp = new_demux_packet(poutbuf_size);
				memcpy(dp->buffer, poutbuf, poutbuf_size);
				dp->pts=lastpts;
			}
		}
	} while (!poutbuf_size);
#endif

	// Set the "ptsBehind" result parameter:
	if (bufferQueue->prevPacketPTS != 0.0
			&& bufferQueue->prevPacketWasSynchronized
			&& *(bufferQueue->otherQueue) != NULL
			&& (*(bufferQueue->otherQueue))->prevPacketPTS != 0.0
			&& (*(bufferQueue->otherQueue))->prevPacketWasSynchronized) {
		ptsBehind = (*(bufferQueue->otherQueue))->prevPacketPTS
			- bufferQueue->prevPacketPTS;
	} else {
		ptsBehind = 0.0;
	}

	if (mustGetNewData) {
		// Save this buffer for future reads:
		bufferQueue->savePendingBuffer(dp);
	}

#if 1//add header of h.264 by doreen
	sh_video_t* sh_video = (sh_video_t*)(ds->sh);
	unsigned char* extraData = (unsigned char *)(sh_video->bih+1);
	int extraSize = 0;
	int buf_size = 0;
	unsigned char * new_buffer;

	if (headersize == 3 )//h.264   && flag_has_add_h264_header==0
	{
		extraSize = sh_video->bih->biSize - sizeof(BITMAPINFOHEADER);

		if(extraSize>0)
		{

			buf_size = dp->len;
#if 0			
			OS_PRINTF("[%s]---------extradata_size=%d, extradata: %x %x %x %x %x %x %x %x\n",\
					__func__,extraSize,extraData[0],extraData[1],extraData[2],\
					extraData[3],extraData[4],extraData[5],extraData[6],extraData[7]);

			OS_PRINTF("[%s]---------ori buffer size=%d, ori buffer data: %x %x %x %x %x %x %x %x\n",\
					__func__,buf_size,dp->buffer[0],dp->buffer[1],dp->buffer[2],\
					dp->buffer[3],dp->buffer[4],dp->buffer[5],dp->buffer[6],dp->buffer[7]);

#endif
			//OS_PRINTF("[%s]-----------dp->len = %d, buf_size: %d, new buffer size: %d \n",__func__,dp->len,buf_size,buf_size+extraSize);

			new_buffer = (unsigned char *)malloc33(buf_size+extraSize);
			memcpy(new_buffer, extraData, extraSize);
			memcpy(new_buffer+extraSize, dp->buffer, buf_size);

			free33(dp->buffer);
			dp->buffer = new_buffer;
			dp->len = buf_size+extraSize;

			/*OS_PRINTF("[%s]---------new buffer size=%d, new buffer data: %x %x %x %x %x %x %x %x\n",\
			  __func__,dp->len,dp->buffer[0],dp->buffer[1],dp->buffer[2],\
			  dp->buffer[3],dp->buffer[4],dp->buffer[5],dp->buffer[6],dp->buffer[7]);*/
			flag_has_add_h264_header =1;
		}

	}

#endif
	//OS_PRINTF("[%s]--end\n",__func__);
	return dp;
}

static void teardownRTSPorSIPSession(RTPState* rtpState) {
	MediaSession* mediaSession = rtpState->mediaSession;
	if (mediaSession == NULL) return;
	if (rtpState->rtspClient != NULL) {
		rtpState->rtspClient->teardownMediaSession(*mediaSession);
	} else if (rtpState->sipClient != NULL) {
		rtpState->sipClient->sendBYE();
	}
}

////////// "ReadBuffer" and "ReadBufferQueue" implementation:

ReadBufferQueue::ReadBufferQueue(MediaSubsession* subsession,
		demuxer_t* demuxer, char const* tag)
: prevPacketWasSynchronized(False), prevPacketPTS(0.0), otherQueue(NULL),
	dp(NULL), nextpacket(NULL),
	pendingDPHead(NULL), pendingDPTail(NULL),
	fReadSource(subsession == NULL ? NULL : subsession->readSource()),
	fRTPSource(subsession == NULL ? NULL : subsession->rtpSource()),
	fOurDemuxer(demuxer), fTag(strdup33(tag)) {
	}

ReadBufferQueue::~ReadBufferQueue() {
	free33((void *)fTag);

	// Free any pending buffers (that never got delivered):
	demux_packet_t* dp = pendingDPHead;
	while (dp != NULL) {
		demux_packet_t* dpNext = dp->next;
		dp->next = NULL;
		free_demux_packet(dp);
		dp = dpNext;
	}
}

void ReadBufferQueue::savePendingBuffer(demux_packet_t* dp) {
	// Keep this buffer around, until MPlayer asks for it later:
	if (pendingDPTail == NULL) {
		pendingDPHead = pendingDPTail = dp;
	} else {
		pendingDPTail->next = dp;
		pendingDPTail = dp;
	}
	dp->next = NULL;
}

demux_packet_t* ReadBufferQueue::getPendingBuffer() {
	
	demux_packet_t* dp = pendingDPHead;
	
	if (dp != NULL) {
		
		pendingDPHead = dp->next;
		if (pendingDPHead == NULL) pendingDPTail = NULL;

		dp->next = NULL;
	}

	return dp;
}

static int demux_rtp_control(struct demuxer *demuxer, int cmd, void *arg) {
	double endpts = ((RTPState*)demuxer->priv)->mediaSession->playEndTime();

	switch(cmd) {
		case DEMUXER_CTRL_GET_TIME_LENGTH:
			if (endpts <= 0)
				return DEMUXER_CTRL_DONTKNOW;
			*((double *)arg) = endpts;
			return DEMUXER_CTRL_OK;

		case DEMUXER_CTRL_GET_PERCENT_POS:
			if (endpts <= 0)
				return DEMUXER_CTRL_DONTKNOW;
			*((int *)arg) = (int)(((RTPState*)demuxer->priv)->videoBufferQueue->prevPacketPTS*100/endpts);
			return DEMUXER_CTRL_OK;

		default:
			return DEMUXER_CTRL_NOTIMPL;
	}
}

demuxer_desc_t demuxer_desc_rtp = {
	"LIVE555 RTP demuxer",
	"live555",
	"",
	"Ross Finlayson",
	"requires LIVE555 Streaming Media library",
	DEMUXER_TYPE_RTP,
	0, // no autodetect
	NULL,
	demux_rtp_fill_buffer,
	demux_open_rtp,
	demux_close_rtp,
	NULL,
	demux_rtp_control
};
