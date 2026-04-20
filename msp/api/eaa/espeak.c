/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <getopt.h>
#include <time.h>
#include "StdAfx.h"

#include "speak_lib.h"
#include "speex_resampler.h"


static int g_default_samplerate = 22050;
static int  g_samplerate_out = 48000;
static  short g_wav_output[20480];
static int g_inited = 0;
void resample_audio(const  short *input, unsigned int input_frames,
                   short *output, unsigned int *output_frames)
{
	    int err;
	    SpeexResamplerState *resampler = speex_resampler_init(
	        1,
	        g_default_samplerate,
	        g_samplerate_out,
	        3,
	        &err
	    );

	    if (err != RESAMPLER_ERR_SUCCESS) {
	        OS_PRINTF( "init failed ret: %d\n", err);
	        return ;
	    }

	    unsigned int in_len = input_frames;
	    unsigned int out_len = input_frames * g_samplerate_out / g_default_samplerate;
	    err = speex_resampler_process_int(
	        resampler,
	        0,
	        input,
	        &in_len,
	        output,
	        &out_len
	    );
	*output_frames = out_len;
    speex_resampler_destroy(resampler);
}

typedef int (pcm_out_callback)(short*, int);
static pcm_out_callback *g_pcm_out_callback = NULL;

static int SynthCallback(short *wav, int numsamples, espeak_EVENT *events)
{//========================================================================
	int ret = 0;
	while(events->type != 0)
	{
		if(events->type == espeakEVENT_SAMPLERATE)
		{
			g_default_samplerate = events->id.number;
		}
		else
		if(events->type == espeakEVENT_SENTENCE)
		{
		}
		events++;
	}

	if(numsamples > 0)
	{
		unsigned int  output_frames = 0;
		resample_audio(wav, (unsigned int )numsamples*2, g_wav_output, &output_frames);
		output_frames &= (~ 0x1);
		if(NULL != g_pcm_out_callback)
		{
			ret = g_pcm_out_callback(g_wav_output, output_frames);
		}
	}
	return ret;
}

int txt_convert2pcm(const char* p_text, int sample_rate)
{
	g_samplerate_out = sample_rate;
	int synth_flags = espeakCHARS_AUTO | espeakPHONEMES | espeakENDPAUSE;
	if(p_text != NULL)
	{
		int size;
		size = strlen(p_text);
		espeak_Synth(p_text,size+1,0,POS_CHARACTER,0,synth_flags,NULL,NULL);
	}
	else
	{
		return -1;
	}

	if(espeak_Synchronize() != EE_OK)
	{
		OS_PRINTF( "espeak_Synchronize() failed, maybe error when opening output device\n");
		return -3;
	}
	return 0;
}

/*
voicename			  Use voice file of this name from espeak-data/voices
volume               Amplitude, 0 to 200, default is 100
pitch  			  Pitch adjustment, 0 to 99, default is 50
option_linelength	  Line length. If not zero (which is the default), consider
	  			  lines less than this length as end-of-clause
speed                Speed in words per minute, 80 to 450, default is 175
wordgap			  Word gap. Pause between words, units of 10mS at the default speed
*/
int espeak_init (char* data_path, char* voicename, int volume, int pitch, int option_linelength, int speed, int wordgap, pcm_out_callback * func)
{
	espeak_VOICE voice_select;

	if(g_inited == 0)
	{
		g_inited = 1 ;
	}
	else
	{
		return -1;
	}
	OS_PRINTF(" %s %d g_inited %d \n", __func__, __LINE__, g_inited);
	g_default_samplerate = espeak_Initialize(AUDIO_OUTPUT_SYNCHRONOUS,0,data_path,0);
	g_pcm_out_callback = func;
	espeak_SetSynthCallback(SynthCallback);

	if(voicename[0] == 0)
		strcpy(voicename,"default");
	if(espeak_SetVoiceByName(voicename) != EE_OK)
	{
		memset(&voice_select,0,sizeof(voice_select));
		voice_select.languages = voicename;
		if(espeak_SetVoiceByProperties(&voice_select) != EE_OK)
		{
			OS_PRINTF(" Failed to read svoice '%s'\n", voicename);
			return -1;
		}
	}

	// set any non-default values of parameters. This must be done after espeak_Initialize()
	if(speed > 0)
		espeak_SetParameter(espeakRATE,speed,0);
	if(volume >= 0)
		espeak_SetParameter(espeakVOLUME,volume,0);
	if(pitch >= 0)
		espeak_SetParameter(espeakPITCH,pitch,0);
	if(wordgap >= 0)
		espeak_SetParameter(espeakWORDGAP,wordgap,0);
	if(option_linelength > 0)
		espeak_SetParameter(espeakLINELENGTH,option_linelength,0);
	OS_PRINTF(" %s %d g_inited %d  Done\n", __func__, __LINE__, g_inited);
	return(0);
}


void espeak_deinit(void)
{
	OS_PRINTF(" %s %d g_inited %d \n", __func__, __LINE__, g_inited);
	if(g_inited)
	{
		espeak_Terminate();
		g_inited = 0;
		OS_PRINTF(" %s %d g_inited %d \n", __func__, __LINE__, g_inited);
	}
}

