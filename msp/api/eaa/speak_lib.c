/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#include "StdAfx.h"

#include "stdio.h"
#include "ctype.h"
#include "string.h"
#include "stdlib.h"
#include "wchar.h"
#include "locale.h"
#include <assert.h>
#include <time.h>

#include "speech.h"

#include <sys/stat.h>
#include <unistd.h>

#include "speak_lib.h"
#include "phoneme.h"
#include "synthesize.h"
#include "voice.h"
#include "translate.h"


void sync_espeak_Char(wchar_t character);

unsigned char *outbuf=NULL;

espeak_EVENT *event_list=NULL;
int event_list_ix=0;
int n_event_list;
long count_samples;
void* my_audio=NULL;

static unsigned int my_unique_identifier=0;
static void* my_user_data=NULL;
static espeak_AUDIO_OUTPUT my_mode=AUDIO_OUTPUT_SYNCHRONOUS;
static int synchronous_mode = 1;
static int out_samplerate = 0;
static int voice_samplerate = 22050;
static espeak_ERROR err = EE_OK;

t_espeak_callback* synth_callback = NULL;
int (* uri_callback)(int, const char *, const char *) = NULL;
int (* phoneme_callback)(const char *) = NULL;

char path_home[N_PATH_HOME];   // this is the espeak-data directory
extern int saved_parameters[N_SPEECH_PARAM]; //Parameters saved on synthesis start


void WVoiceChanged(voice_t *wvoice)
{//=================================
// Voice change in wavegen
	voice_samplerate = wvoice->samplerate;
}

static void select_output(espeak_AUDIO_OUTPUT output_type)
{//=======================================================
	my_mode = output_type;
	my_audio = NULL;
	synchronous_mode = 1;
 	option_waveout = 1;   // inhibit portaudio callback from wavegen.cpp
	out_samplerate = 0;

	switch(my_mode)
	{
	case AUDIO_OUTPUT_PLAYBACK:
		// wave_init() is now called just before the first wave_write()
		synchronous_mode = 0;
		break;

	case AUDIO_OUTPUT_RETRIEVAL:
		synchronous_mode = 0;
		break;

	case AUDIO_OUTPUT_SYNCHRONOUS:
		break;

	case AUDIO_OUTPUT_SYNCH_PLAYBACK:
		option_waveout = 0;
		WavegenInitSound();
		break;
	}
}   // end of select_output




int GetFileLength(const char *filename)
{//====================================
	struct stat statbuf;

	if(stat(filename,&statbuf) != 0)
		return(0);

	if((statbuf.st_mode & S_IFMT) == S_IFDIR)
		//	if(S_ISDIR(statbuf.st_mode))
		return(-2);  // a directory

	return(statbuf.st_size);
}  // end of GetFileLength


char *Alloc(int size)
{//==================
	char *p;
	if((p = (char *)malloc(size)) == NULL)
		OS_PRINTF("Can't allocate memory\n");  // I was told that size+1 fixes a crash on 64-bit systems
	return(p);
}

void Free(void *ptr)
{//=================
	if(ptr != NULL)
		free(ptr);
}
static void init_path(const char *path)
{//====================================
	if(path != NULL)
	{
		snprintf(path_home,sizeof(path_home),"%s/espeak-data",path);
		return;
	}
	strcpy(path_home,PATH_ESPEAK_DATA);
}

static int initialise(int control)
{//===============================
	int param;
	int result;
	int srate = 22050;  // default sample rate 22050 Hz

	err = EE_OK;
#ifdef _LZ_LINUX_
	LoadConfig();
#endif
	if((result = LoadPhData(&srate)) != 1)  // reads sample rate from espeak-data/phontab
	{
		if(result == -1)
		{
			OS_PRINTF("Failed to load espeak-data\n");
			return -1;
		}
	}
	WavegenInit(srate,0);

	memset(&current_voice_selected,0,sizeof(current_voice_selected));
	SetVoiceStack(NULL, "");
	SynthesizeInit();
	InitNamedata();

	for(param=0; param<N_SPEECH_PARAM; param++)
		param_stack[0].parameter[param] = param_defaults[param];

	return(0);
}


static espeak_ERROR Synthesize(unsigned int unique_identifier, const void *text, int flags)
{//========================================================================================
	// Fill the buffer with output sound
	int length;
	int finished = 0;
	int count_buffers = 0;
	if((outbuf==NULL) || (event_list==NULL))
		return(EE_INTERNAL_ERROR);  // espeak_Initialize()  has not been called

	option_multibyte = flags & 7;
	option_ssml = flags & espeakSSML;
	option_phoneme_input = flags & espeakPHONEMES;
	option_endpause = flags & espeakENDPAUSE;

	count_samples = 0;


	if(translator == NULL)
	{
		SetVoiceByName("default");
	}

	SpeakNextClause(NULL,text,0);

	if(my_mode == AUDIO_OUTPUT_SYNCH_PLAYBACK)
	{
		for(;;)
		{
			mtos_task_sleep(300);
			if(SynthOnTimer() != 0)
				break;
		}
		return(EE_OK);
	}

	for(;;)
	{
		out_ptr = outbuf;
		out_end = &outbuf[outbuf_size];
		event_list_ix = 0;
		WavegenFill(0);

		length = (out_ptr - outbuf)/2;
		count_samples += length;
		event_list[event_list_ix].type = espeakEVENT_LIST_TERMINATED; // indicates end of event list
		event_list[event_list_ix].unique_identifier = my_unique_identifier;
		event_list[event_list_ix].user_data = my_user_data;

		count_buffers++;
		if (my_mode==AUDIO_OUTPUT_PLAYBACK)
		{
		}
		else
		{
			finished = synth_callback((short *)outbuf, length, event_list);
		}
		if(finished)
		{
			SpeakNextClause(NULL,0,2);  // stop
			break;
		}

		if(Generate(phoneme_list,&n_phoneme_list,1)==0)
		{
			if(WcmdqUsed() == 0)
			{
				// don't process the next clause until the previous clause has finished generating speech.
				// This ensures that <audio> tag (which causes end-of-clause) is at a sound buffer boundary

				event_list[0].type = espeakEVENT_LIST_TERMINATED;
				event_list[0].unique_identifier = my_unique_identifier;
				event_list[0].user_data = my_user_data;

				if(SpeakNextClause(NULL,NULL,1)==0)
				{
					synth_callback(NULL, 0, event_list);  // NULL buffer ptr indicates end of data
					break;
				}
			}
		}
	}
	return(EE_OK);
}  //  end of Synthesize

#ifdef DEBUG_ENABLED
static const char* label[] = {
  "END_OF_EVENT_LIST",
  "WORD",
  "SENTENCE",
  "MARK",
  "PLAY",
  "END",
  "MSG_TERMINATED",
  "PHONEME",
  "SAMPLERATE",
  "??" };
#endif


void MarkerEvent(int type, unsigned int char_position, int value, int value2, unsigned char *out_ptr)
{//==================================================================================================
	// type: 1=word, 2=sentence, 3=named mark, 4=play audio, 5=end, 7=phoneme
	espeak_EVENT *ep;
	double time;

	if((event_list == NULL) || (event_list_ix >= (n_event_list-2)))
		return;

	ep = &event_list[event_list_ix++];
	ep->type = (espeak_EVENT_TYPE)type;
	ep->unique_identifier = my_unique_identifier;
	ep->user_data = my_user_data;
	ep->text_position = char_position & 0xffffff;
	ep->length = char_position >> 24;

	time = ((double)(count_samples + mbrola_delay + (out_ptr - out_start)/2)*1000.0)/samplerate;
	ep->audio_position = (int)(time);
	ep->sample = (count_samples + mbrola_delay + (out_ptr - out_start)/2);

#ifdef DEBUG_ENABLED
	SHOW("MarkerEvent > count_samples=%d, out_ptr=%x, out_start=0x%x\n",count_samples, out_ptr, out_start);
	SHOW("*** MarkerEvent > type=%s, uid=%d, text_pos=%d, length=%d, audio_position=%d, sample=%d\n",
			label[ep->type], ep->unique_identifier, ep->text_position, ep->length,
			ep->audio_position, ep->sample);
#endif

	if((type == espeakEVENT_MARK) || (type == espeakEVENT_PLAY))
		ep->id.name = &namedata[value];
	else
//#ifdef deleted
// temporarily removed, don't introduce until after eSpeak version 1.46.02
	if(type == espeakEVENT_PHONEME)
	{
		int *p;
		p = (int *)(ep->id.string);
		p[0] = value;
		p[1] = value2;
	}
	else
//#endif
	{
		ep->id.number = value;
	}
}  //  end of MarkerEvent




espeak_ERROR sync_espeak_Synth(unsigned int unique_identifier, const void *text, size_t size,
		      unsigned int position, espeak_POSITION_TYPE position_type,
		      unsigned int end_position, unsigned int flags, void* user_data)
{//===========================================================================
	espeak_ERROR aStatus;
    int i = 0;
	InitText(flags);
	my_unique_identifier = unique_identifier;
	my_user_data = user_data;

	for (i=0; i < N_SPEECH_PARAM; i++)
		saved_parameters[i] = param_stack[0].parameter[i];

	switch(position_type)
		{
		case POS_CHARACTER:
			skip_characters = position;
			break;

		case POS_WORD:
			skip_words = position;
			break;

		case POS_SENTENCE:
			skip_sentences = position;
			break;

		}
	if(skip_characters || skip_words || skip_sentences)
		skipping_text = 1;

	end_character_position = end_position;

	aStatus = Synthesize(unique_identifier, text, flags);

	return aStatus;
}  //  end of sync_espeak_Synth




espeak_ERROR sync_espeak_Synth_Mark(unsigned int unique_identifier, const void *text, size_t size,
			   const char *index_mark, unsigned int end_position,
			   unsigned int flags, void* user_data)
{//=========================================================================
	espeak_ERROR aStatus;

	InitText(flags);

	my_unique_identifier = unique_identifier;
	my_user_data = user_data;

	if(index_mark != NULL)
		{
		strncpy0(skip_marker, index_mark, sizeof(skip_marker));
		skipping_text = 1;
		}

	end_character_position = end_position;


	aStatus = Synthesize(unique_identifier, text, flags | espeakSSML);

	return (aStatus);
}  //  end of sync_espeak_Synth_Mark



void sync_espeak_Key(const char *key)
{//==================================
	// symbolic name, symbolicname_character  - is there a system resource of symbolic names per language?
	int letter;
	int ix;

	ix = utf8_in(&letter,key);
	if(key[ix] == 0)
	{
		// a single character
		sync_espeak_Char(letter);
		return;
	}

	my_unique_identifier = 0;
	my_user_data = NULL;
	Synthesize(0, key,0);   // speak key as a text string
}


void sync_espeak_Char(wchar_t character)
{//=====================================
	// is there a system resource of character names per language?
	char buf[80];
	my_unique_identifier = 0;
	my_user_data = NULL;

	sprintf(buf,"<say-as interpret-as=\"tts:char\">&#%d;</say-as>",character);
	Synthesize(0, buf,espeakSSML);
}



void sync_espeak_SetPunctuationList(const wchar_t *punctlist)
{//==========================================================
	// Set the list of punctuation which are spoken for "some".
	my_unique_identifier = 0;
	my_user_data = NULL;

	option_punctlist[0] = 0;
	if(punctlist != NULL)
	{
		wcsncpy(option_punctlist, punctlist, N_PUNCTLIST);
		option_punctlist[N_PUNCTLIST-1] = 0;
	}
}  //  end of sync_espeak_SetPunctuationList




#pragma GCC visibility push(default)


ESPEAK_API void espeak_SetSynthCallback(t_espeak_callback* SynthCallback)
{//======================================================================
	synth_callback = SynthCallback;
}

ESPEAK_API void espeak_SetUriCallback(int (* UriCallback)(int, const char*, const char *))
{//=======================================================================================
	uri_callback = UriCallback;
}


ESPEAK_API void espeak_SetPhonemeCallback(int (* PhonemeCallback)(const char*))
{//===========================================================================
	phoneme_callback = PhonemeCallback;
}

ESPEAK_API int espeak_Initialize(espeak_AUDIO_OUTPUT output_type, int buf_length, const char *path, int options)
{//=============================================================================================================
	int param;

	init_path(path);
	initialise(options);
	select_output(output_type);

	// buflength is in mS, allocate 2 bytes per sample
	if((buf_length == 0) || (output_type == AUDIO_OUTPUT_PLAYBACK) || (output_type == AUDIO_OUTPUT_SYNCH_PLAYBACK))
		buf_length = 200;

	outbuf_size = (buf_length * samplerate)/500;
	outbuf = (unsigned char*)malloc(outbuf_size*2);
	if((out_start = outbuf) == NULL)
		return(EE_INTERNAL_ERROR);

	// allocate space for event list.  Allow 200 events per second.
	// Add a constant to allow for very small buf_length
	n_event_list = (buf_length*200)/1000 + 20;
	if((event_list = (espeak_EVENT *)malloc(sizeof(espeak_EVENT) * n_event_list)) == NULL)
		return(EE_INTERNAL_ERROR);

	option_phonemes = 0;

	VoiceReset(0);
//	SetVoiceByName("default");

	for(param=0; param<N_SPEECH_PARAM; param++)
		param_stack[0].parameter[param] = saved_parameters[param] = param_defaults[param];

	SetParameter(espeakRATE,175,0);
	SetParameter(espeakVOLUME,100,0);
	SetParameter(espeakCAPITALS,option_capitals,0);
	SetParameter(espeakPUNCTUATION,option_punctuation,0);
	SetParameter(espeakWORDGAP,0,0);
//	DoVoiceChange(voice);
  return(samplerate);
}



ESPEAK_API espeak_ERROR espeak_Synth(const void *text, size_t size,
				     unsigned int position,
				     espeak_POSITION_TYPE position_type,
				     unsigned int end_position, unsigned int flags,
				     unsigned int* unique_identifier, void* user_data)
{//=====================================================================================
	espeak_ERROR a_error=EE_INTERNAL_ERROR;
	static unsigned int temp_identifier;

	if (unique_identifier == NULL)
	{
		unique_identifier = &temp_identifier;
	}
	*unique_identifier = 0;

	if(synchronous_mode)
	{
		return(sync_espeak_Synth(0,text,size,position,position_type,end_position,flags,user_data));
	}
	return a_error;
}  //  end of espeak_Synth



ESPEAK_API espeak_ERROR espeak_Synth_Mark(const void *text, size_t size,
					  const char *index_mark,
					  unsigned int end_position,
					  unsigned int flags,
					  unsigned int* unique_identifier,
					  void* user_data)
{//=========================================================================

	espeak_ERROR a_error=EE_OK;
	static unsigned int temp_identifier;
	if (unique_identifier == NULL)
	{
		unique_identifier = &temp_identifier;
	}
	*unique_identifier = 0;

	if(synchronous_mode)
	{
		return(sync_espeak_Synth_Mark(0,text,size,index_mark,end_position,flags,user_data));
	}

	return a_error;
}  //  end of espeak_Synth_Mark



ESPEAK_API espeak_ERROR espeak_Key(const char *key)
{//================================================
	// symbolic name, symbolicname_character  - is there a system resource of symbolicnames per language
	espeak_ERROR a_error = EE_OK;

	if(synchronous_mode)
	{
		sync_espeak_Key(key);
		return(EE_OK);
	}

  return a_error;
}


ESPEAK_API espeak_ERROR espeak_Char(wchar_t character)
{//===========================================
	sync_espeak_Char(character);
	return(EE_OK);
}


ESPEAK_API espeak_ERROR espeak_SetVoiceByName(const char *name)
{//============================================================
	return(SetVoiceByName(name));
}  // end of espeak_SetVoiceByName



ESPEAK_API espeak_ERROR espeak_SetVoiceByProperties(espeak_VOICE *voice_selector)
{//==============================================================================
	return(SetVoiceByProperties(voice_selector));
}  // end of espeak_SetVoiceByProperties


ESPEAK_API int espeak_GetParameter(espeak_PARAMETER parameter, int current)
{//========================================================================
	// current: 0=default value, 1=current value
	if(current)
	{
		return(param_stack[0].parameter[parameter]);
	}
	else
	{
		return(param_defaults[parameter]);
	}
}  //  end of espeak_GetParameter


ESPEAK_API espeak_ERROR espeak_SetParameter(espeak_PARAMETER parameter, int value, int relative)
{//=============================================================================================
	SetParameter(parameter,value,relative);
	return(EE_OK);
}


ESPEAK_API espeak_ERROR espeak_SetPunctuationList(const wchar_t *punctlist)
{//================================================================
  // Set the list of punctuation which are spoken for "some".
	sync_espeak_SetPunctuationList(punctlist);
	return(EE_OK);
}  //  end of espeak_SetPunctuationList


ESPEAK_API void espeak_SetPhonemeTrace(int value, FILE *stream)
{//============================================================
	/* Controls the output of phoneme symbols for the text
		bits 0-3:
		 value=0  No phoneme output (default)
		 value=1  Output the translated phoneme symbols for the text
		 value=2  as (1), but also output a trace of how the translation was done (matching rules and list entries)
		 value=3  as (1), but produces IPA phoneme names rather than ascii
		bit 4:   produce mbrola pho data
	*/
	option_phonemes = value & 7;
	f_trans = stream;
	if(stream == NULL)
		f_trans = stderr;

}   //  end of espeak_SetPhonemes

ESPEAK_API espeak_ERROR espeak_Cancel(void)
{//===============================
	embedded_value[EMBED_T] = 0;    // reset echo for pronunciation announcements
    int i = 0;
	for (i=0; i < N_SPEECH_PARAM; i++)
		SetParameter(i, saved_parameters[i], 0);

	return EE_OK;
}   //  end of espeak_Cancel

ESPEAK_API espeak_ERROR espeak_Synchronize(void)
{//=============================================
	espeak_ERROR berr = err;
	err = EE_OK;
	return berr;
}   //  end of espeak_Synchronize


extern void FreePhData(void);
extern void FreeVoiceList(void);
void api_DeleteTranslator(void);

ESPEAK_API espeak_ERROR espeak_Terminate(void)
{//===========================================
	Free(event_list);
	event_list = NULL;
	Free(outbuf);
	outbuf = NULL;
	FreePhData();
	FreeVoiceList();
	api_DeleteTranslator();
	return EE_OK;
}   //  end of espeak_Terminate

ESPEAK_API const char *espeak_Info(const char **ptr)
{//=================================================
	if(ptr != NULL)
	{
		*ptr = path_home;
	}
	return(version_string);
}

#pragma GCC visibility pop


