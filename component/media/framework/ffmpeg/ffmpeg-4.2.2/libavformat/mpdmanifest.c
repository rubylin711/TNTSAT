/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#include "mpdmanifest.h"
#include "libavutil/avstring.h"
#include "libavutil/base64.h"
#include "expat/lib/expat.h"
#ifdef XML_LARGE_SIZE
#if defined(XML_USE_MSC_EXTENSIONS) && _MSC_VER < 1400
#define XML_FMT_INT_MOD "I64"
#else
#define XML_FMT_INT_MOD "ll"
#endif
#else
#define XML_FMT_INT_MOD "l"
#endif

#define XML_PARSE_DONE 99999

MPD_SNode snode[1024];
unsigned int snode_len = 0;

MPD_Period * cur_period = NULL;
MPD_AdaptationSet * cur_adaptationset = NULL;
MPD_Representation * cur_representation = NULL;

#define MAXINT 0x7FFFFFFF

#define SEC_PER_DAY          (86400LL)

/*
  Duration Data Type

  The duration data type is used to specify a time interval.

  The time interval is specified in the following form "-PnYnMnDTnHnMnS" where:

    * - indicates the negative sign (optional)
    * P indicates the period (required)
    * nY indicates the number of years
    * nM indicates the number of months
    * nD indicates the number of days
    * T indicates the start of a time section (required if you are going to specify hours, minutes, or seconds)
    * nH indicates the number of hours
    * nM indicates the number of minutes
    * nS indicates the number of seconds
*/

/* this function computes decimals * 10 ^ (3 - pos) */
static int
convert_to_millisecs (int decimals, int pos)
{
  int num = 1, den = 1;
  int i = 3 - pos;

  while (i < 0) {
    den *= 10;
    i++;
  }
  while (i > 0) {
    num *= 10;
    i--;
  }
  /* if i == 0 we have exactly 3 decimals and nothing to do */
  return decimals * num / den;
}


static int
accumulate (unsigned long * v, int mul, int add)
{
  unsigned long tmp;

  //if (*v > G_MAXUINT / mul)
  //  return FALSE;
  tmp = *v * mul;
  //if (tmp > G_MAXUINT - add)
  //  return FALSE;
  *v = tmp + add;
  return 1;
}

static int get_utc_time (char *str) {
    int ret = 0;
    int year, month, day, hour, minute, second, pos;
    struct tm tm;
    //printf ("dateTime: %s, len %d\n", str, strlen (str));
    /* parse year */
    ret = sscanf (str, "%d", &year);
    if (ret != 1 || year <= 0)
      goto error;
    pos = strcspn (str, "-");
    str += (pos + 1);
    /* parse month */
    ret = sscanf (str, "%d", &month);
    if (ret != 1 || month <= 0)
      goto error;
    pos = strcspn (str, "-");
    str += (pos + 1);
    /* parse day */
    ret = sscanf (str, "%d", &day);
    if (ret != 1 || day <= 0)
      goto error;
    pos = strcspn (str, "T");
    str += (pos + 1);
    /* parse hour */
    ret = sscanf (str, "%d", &hour);
    if (ret != 1 || hour < 0)
      goto error;
    pos = strcspn (str, ":");
    str += (pos + 1);
    /* parse minute */
    ret = sscanf (str, "%d", &minute);
    if (ret != 1 || minute < 0)
      goto error;
    pos = strcspn (str, ":");
    str += (pos + 1);
    /* parse second */
    ret = sscanf (str, "%dZ", &second);
    if (ret != 1 || second < 0)
      goto error;


    tm.tm_year = year - 1900;
	tm.tm_mon  = month - 1;
	tm.tm_mday = day;
	tm.tm_hour = hour;
	tm.tm_min  = minute;
	tm.tm_sec  = second;
	tm.tm_isdst = 0;

    ret = mktime(&tm);
    //printf ("time -: %4d/%02d/%02d %02d:%02d:%02d, utc time %d\n",
    //    year, month, day, hour, minute, second, ret);
	if (ret >= 0)
		return ret;

error:
    return -1;
}


static int mpdparser_parse_duration(const char *str, unsigned long * value)
{
  int ret, len, pos, posT;
  int years = -1, months = -1, days = -1, hours = -1, minutes = -1, seconds =
      -1, decimals = -1, read;
  int have_ms = 0;
  unsigned long tmp_value;

  len = strlen (str);
  //printf ("zx duration: %s, len %d\n", str, len);
  if (strspn (str, "PT0123456789., \tHMDSY") < len) {
    printf ("Error: Invalid character found: '%s'\n", str);
    goto error;
  }
  /* skip leading/trailing whitespace */
  while (strchr (" \t", str[0])) {
    str++;
    len--;
  }
  while (len > 0 && strchr (" \t", str[len - 1]))
    --len;

  /* read "P" for period */
  if (str[0] != 'P') {
    printf ("Error: P not found at the beginning of the string!\n");
    goto error;
  }
  str++;
  len--;

  /* read "T" for time (if present) */
  posT = strcspn (str, "T");
  len -= posT;
  if (posT > 0) {
    /* there is some room between P and T, so there must be a period section */
    /* read years, months, days */
    do {
      //printf ("parsing substring %s", str);
      pos = strcspn (str, "YMD");
      ret = sscanf (str, "%u", &read);
      if (ret != 1) {
        printf ("Error: can not read integer value from string %s!\n", str);
        goto error;
      }
      switch (str[pos]) {
        case 'Y':
          if (years != -1 || months != -1 || days != -1) {
            printf ("Error: year, month or day was already set\n");
            goto error;
          }
          years = read;
          break;
        case 'M':
          if (months != -1 || days != -1) {
            printf ("Error: month or day was already set\n");
            goto error;
          }
          months = read;
          if (months >= 12) {
            printf ("Error: Month out of range\n");
            goto error;
          }
          break;
        case 'D':
          if (days != -1) {
            printf ("Error: day was already set\n");
            goto error;
          }
          days = read;
          if (days >= 31) {
            printf ("Error: Day out of range\n");
            goto error;
          }
          break;
        default:
          printf ("Error: unexpected char %c!\n", str[pos]);
          goto error;
          break;
      }
      //printf ("read number %u type %c\n", read, str[pos]);
      str += (pos + 1);
      posT -= (pos + 1);
    } while (posT > 0);
  }

  if (years == -1)
    years = 0;
  if (months == -1)
    months = 0;
  if (days == -1)
    days = 0;

  //printf ("Y:M:D=%d:%d:%d\n", years, months, days);

  /* read "T" for time (if present) */
  /* here T is at pos == 0 */
  str++;
  len--;
  pos = 0;
  if (pos < len) {
    /* T found, there is a time section */
    /* read hours, minutes, seconds, hundredths of second */
    do {
      //printf ("parsing substring %s\n", str);
      pos = strcspn (str, "HMS,.");
      ret = sscanf (str, "%u", &read);
      if (ret != 1) {
        printf ("Error: can not read integer value from string %s!", str);
        goto error;
      }
      switch (str[pos]) {
        case 'H':
          if (hours != -1 || minutes != -1 || seconds != -1) {
            printf ("Error: hour, minute or second was already set");
            goto error;
          }
          hours = read;
          if (hours >= 24) {
            printf ("Error: Hour out of range");
            goto error;
          }
          break;
        case 'M':
          if (minutes != -1 || seconds != -1) {
            printf ("Error: minute or second was already set");
            goto error;
          }
          minutes = read;
          if (minutes >= 60) {
            printf ("Error: Minute out of range");
            goto error;
          }
          break;
        case 'S':
          if (have_ms) {
            /* we have read the decimal part of the seconds */
            decimals = convert_to_millisecs (read, pos);
            //printf ("decimal number %u (%d digits) -> %d ms\n", read, pos,
            //    decimals);
          } else {
            if (seconds != -1) {
              printf ("Error: second was already set\n");
              goto error;
            }
            /* no decimals */
            seconds = read;
          }
          break;
        case '.':
        case ',':
          /* we have read the integer part of a decimal number in seconds */
          if (seconds != -1) {
            printf ("Error: second was already set\n");
            goto error;
          }
          seconds = read;
          have_ms = 1;
          break;
        default:
          printf ("Error: unexpected char %c!\n", str[pos]);
          goto error;
          break;
      }
      //printf ("read number %u type %c\n", read, str[pos]);
      str += pos + 1;
      len -= (pos + 1);
    } while (len > 0);
  }

  if (hours == -1)
    hours = 0;
  if (minutes == -1)
    minutes = 0;
  if (seconds == -1)
    seconds = 0;
  if (decimals == -1)
    decimals = 0;

  tmp_value = 0;
  if (!accumulate (&tmp_value, 1, years)
      || !accumulate (&tmp_value, 365, months * 30)
      || !accumulate (&tmp_value, 1, days)
      || !accumulate (&tmp_value, 24, hours)
      || !accumulate (&tmp_value, 60, minutes)
      || !accumulate (&tmp_value, 60, seconds))
    goto error;

  if (tmp_value > MAXINT )
    goto error;

  *value = tmp_value;
  //printf ("duration H:M:S.MS=%d:%d:%d.%03d, sec %u\n", hours, minutes, seconds, decimals, tmp_value);
  return 0;

error:
  printf ("%s: error duration: %s\n", __func__, str);
  return -1;
}

static char * xml_strdup(const char * s)
{
    char * new_string = NULL;

    if (s == NULL) {
        return NULL;
    }

    new_string = (char *)av_mallocz(strlen(s) + 1);

    if (new_string == NULL) {
        return NULL ;
    }

    memset(new_string, 0, strlen(s) + 1);
    strcpy(new_string, s);
    return new_string;
}

static void XMLCALL xml_start(void * data, const char * el, const char ** attr)
{
    int i;
    MPDManifest * manifest = data;
    manifest->data_type = DATA_TYPE_NONE;
    float hour, min, sec = 0.0;
    unsigned long tmp_value = 0;
    //   OS_PRINTF("%s %d el=%s\n", __func__, __LINE__, el);

    if (!strcmp(el, "MPD"))  {
        manifest->mpd_parse_flag = 1;
        manifest->data_type = DATA_TYPE_MPD;
        manifest->g_dash_playmode = 0;

        for (i = 0; attr[i]; i += 2) {
            if (!strcmp(attr[i], "mediaPresentationDuration")) {
                mpdparser_parse_duration(attr[i + 1], &(manifest->maxSubsegmentDuration));
            } else if (!strcmp(attr[i], "profiles")) {
                if (strstr(attr[i + 1], "isoff-live")) {
                    manifest->g_dash_playmode = 2;
                } else if (strstr(attr[i + 1], "isoff-on-demand:2011")) {//profiles="urn:mpeg:dash:profile:isoff-on-demand:2011"
                    manifest->g_dash_playmode = 1;
                    manifest->profile_isoff_ondemand = 1;
                } else if (strstr(attr[i + 1], "full:2011")) {
                    manifest->g_dash_playmode = 2;
                } else if (strstr(attr[i + 1], "isoff-main")) {
                    manifest->g_dash_playmode = 1;
                }
                //printf("%s %d profiles g_dash_playmode %d, %s\n", __func__, __LINE__, manifest->g_dash_playmode, attr[i + 1]);
            /*} else if (!strcmp(attr[i], "profiles")) {            // for tsscan,  condition matches previous condition at line 384
                manifest->profiles = xml_strdup(attr[i + 1]);
            } else if (!strcmp(attr[i], "mediaPresentationDuration")) {     // condition matches previous condition at line 382
                manifest->profiles = xml_strdup(attr[i + 1]);*/
            } else if (!strcmp(attr[i], "availabilityStartTime")) {
                manifest->availabilityStartTime = get_utc_time(attr[i + 1]);
            } else if (!strcmp(attr[i], "publishTime")) {
                manifest->publishTime = get_utc_time(attr[i + 1]);
            } else if (!strcmp(attr[i], "minimumUpdatePeriod")) {
                mpdparser_parse_duration(attr[i + 1], &(manifest->minimumUpdatePeriod));
            } else if (!strcmp(attr[i], "minBufferTime")) {
                mpdparser_parse_duration(attr[i + 1], &(manifest->minBufferTime));
            } else if (!strcmp(attr[i], "maxSegmentDuration")) {
                mpdparser_parse_duration(attr[i + 1], &(manifest->maxSegmentDuration));
            } else if (!strcmp(attr[i], "suggestedPresentationDelay")) {
                mpdparser_parse_duration(attr[i + 1], &tmp_value);
                manifest->suggested_presentation_delay = tmp_value;
            } else if (!strcmp(attr[i], "timeShiftBufferDepth")) {
                mpdparser_parse_duration(attr[i + 1], &tmp_value);
                manifest->timeShiftBufferDepth = tmp_value;
            }


        }
    } else if (!strcmp(el, "Period")) {
        manifest->period_parse_flag = 1;
        manifest->data_type = DATA_TYPE_MPD_PERIOD;
        cur_period = NULL;
        cur_period = av_mallocz(sizeof(*cur_period));

        if (!cur_period) {
            manifest->parse_ret = AVERROR(ENOMEM);
            return;
        }

        dynarray_add(&manifest->period, &manifest->nb_period, cur_period);

        for (i = 0; attr[i]; i += 2) {
            if (!strcmp(attr[i], "duration")) {
                mpdparser_parse_duration(attr[i + 1], &(manifest->period_duration));
            }
            if (!strcmp(attr[i], "start")) {
                mpdparser_parse_duration(attr[i + 1], &tmp_value);
                cur_period->start_time = tmp_value;
            }
        }
    } else if (!strcmp(el, "AdaptationSet")) {
        manifest->adaptationset_parse_flag = 1;
        manifest->data_type = DATA_TYPE_MPD_ADAPTATIONSET;

        if (cur_period) {
            cur_adaptationset = av_mallocz(sizeof(*cur_adaptationset));
            cur_representation = NULL; //reset, otherwise is point to last cur_representation!
            cur_adaptationset->startNumber = -1;

            if (!cur_adaptationset) {
                manifest->parse_ret = AVERROR(ENOMEM);
                return;
            }

            dynarray_add(&cur_period->adaptationset, &cur_period->nb_adaptationset, cur_adaptationset);

            for (i = 0; attr[i]; i += 2) {
                //av_log(NULL,AV_LOG_DEBUG,"Representation number[%d] attr[%s] data[%s].\n", cur_period->nb_representation, attr[i], attr[i + 1]);
                if (!strcmp(attr[i], "lang")) {
                    if (cur_adaptationset) {
                        cur_adaptationset->lang = xml_strdup(attr[i + 1]);
                    }
                } else if (!strcmp(attr[i], "maxWidth")) {
                    if (cur_adaptationset) {
                        cur_adaptationset->maxWidth = strtol(attr[i + 1], NULL, 10);
                    }
                }   else if (!strcmp(attr[i], "maxHeight")) {
                    if (cur_adaptationset) {
                        cur_adaptationset->maxHeight = strtol(attr[i + 1], NULL, 10);
                    }
                }   else if (!strcmp(attr[i], "codecs")) {
                    if (cur_adaptationset) {
                        cur_adaptationset->codecs = xml_strdup(attr[i + 1]);
                    }
                } else if (!strcmp(attr[i], "mimeType")) {
                    if (cur_adaptationset) {
                        cur_adaptationset->mimeType = xml_strdup(attr[i + 1]);
                    }
                } else if (!strcmp(attr[i], "audioSamplingRate")) {
                    if (cur_adaptationset) {
                        cur_adaptationset->audioSamplingRate = strtol(attr[i + 1], NULL, 10);
                    }
                }
            }
        }
    } else if (!strcmp(el, "ContentProtection")) {
        if (cur_adaptationset && cur_period) {
            cur_adaptationset = cur_period->adaptationset[cur_period->nb_adaptationset - 1];
        }

        for (i = 0; attr[i]; i += 2) {
            //printf("ContentProtection attr[%s] data[%s].\n", attr[i], attr[i + 1]);
            if (!strcmp(attr[i], "schemeIdUri")) {
                if (cur_representation) {
                    cur_adaptationset->contentProtection.system_id = xml_strdup(attr[i + 1]);
                }
            }
        }
    } else if (!strcmp(el, "mspr:pro") || !strcmp(el, "pro") || !strcmp(el, "cenc:pssh")) {
        manifest->data_type = DATA_TYPE_MPD_CONTENTPROTECTION_MSPR;

    } else if (!strcmp(el, "Representation")) {
        manifest->representation_parse_flag = 1;
        manifest->data_type = DATA_TYPE_MPD_PERIOD_REPRESENTATION;

        if (cur_period && cur_adaptationset) {
            cur_adaptationset = cur_period->adaptationset[cur_period->nb_adaptationset - 1];
            cur_representation = av_mallocz(sizeof(*cur_representation));
            cur_representation->startNumber = -1;

            if (!cur_representation) {
                manifest->parse_ret = AVERROR(ENOMEM);
                return;
            }

            if(cur_adaptationset) dynarray_add(&cur_adaptationset->representation, &cur_adaptationset->nb_representation, cur_representation);
        }

        for (i = 0; attr[i]; i += 2) {
            //av_log(NULL,AV_LOG_DEBUG,"Representation number[%d] attr[%s] data[%s].\n", cur_period->nb_representation, attr[i], attr[i + 1]);
            if (!strcmp(attr[i], "width")) {
                if (cur_representation) {
                    cur_representation->width = strtol(attr[i + 1], NULL, 10);
                }
            } else if (!strcmp(attr[i], "height")) {
                if (cur_representation) {
                    cur_representation->height = strtol(attr[i + 1], NULL, 10);
                }
            } else if (!strcmp(attr[i], "bandwidth")) {
                if (cur_representation) {
                    cur_representation->bandwidth = strtol(attr[i + 1], NULL, 10);
                }
            } else if (!strcmp(attr[i], "frameRate")) {
                if (cur_representation) {
                    cur_representation->frameRate = strtol(attr[i + 1], NULL, 10);
                }
            } else if (!strcmp(attr[i], "mimeType")) {
                if (cur_representation) {
                    cur_representation->mimeType = xml_strdup(attr[i + 1]);
                }
            } else if (!strcmp(attr[i], "codecs")) {
                if (cur_representation) {
                    cur_representation->codecs = xml_strdup(attr[i + 1]);
                }
            } else if (!strcmp(attr[i], "id")) {
                if (cur_representation) {
                    cur_representation->id = xml_strdup(attr[i + 1]);
                }
            }
        }
    } else if (!strcmp(el, "SegmentTemplate"))  {
        if (manifest->adaptationset_parse_flag || manifest->representation_parse_flag) {
            for (i = 0; attr[i]; i += 2) {
                if (!strcmp(attr[i], "timescale")) {
                    if (manifest->representation_parse_flag && cur_representation) {
                        cur_representation->timescale = strtol(attr[i + 1], NULL, 10);
                    } else if (manifest->adaptationset_parse_flag && cur_adaptationset) {
                        cur_adaptationset->timescale = strtol(attr[i + 1], NULL, 10);
                    }
                } else if (!strcmp(attr[i], "duration")) {
                    if (manifest->representation_parse_flag && cur_representation) {
                        cur_representation->duration = strtol(attr[i + 1], NULL, 10);
                    } else if (manifest->adaptationset_parse_flag && cur_adaptationset) {
                        cur_adaptationset->duration = strtol(attr[i + 1], NULL, 10);
                    }
                } else if (!strcmp(attr[i], "media")) {
                    if (manifest->representation_parse_flag && cur_representation) {
                        cur_representation->media = xml_strdup(attr[i + 1]);
                    } else if (manifest->adaptationset_parse_flag && cur_adaptationset) {
                        cur_adaptationset->media = xml_strdup(attr[i + 1]);
                    }
                } else if (!strcmp(attr[i], "initialization")) {
                    if (manifest->representation_parse_flag && cur_representation) {
                        cur_representation->init_url = xml_strdup(attr[i + 1]);
                    } else if (manifest->adaptationset_parse_flag && cur_adaptationset) {
                        cur_adaptationset->init_url = xml_strdup(attr[i + 1]);
                    }
                } else if (!strcmp(attr[i], "startNumber")) {
                    if (manifest->representation_parse_flag && cur_representation) {
                        cur_representation->startNumber = strtol(attr[i + 1], NULL, 10);
                    } else if (manifest->adaptationset_parse_flag && cur_adaptationset) {
                        cur_adaptationset->startNumber = strtol(attr[i + 1], NULL, 10);
                    }
                }
            }
        }
    } else if (!strcmp(el, "BaseURL")) {
        manifest->data_type = DATA_TYPE_MPD_BASEURL;
    } else if (!strcmp(el, "S")) {
        for (i = 0; attr[i]; i += 2) {
            if (!strcmp(attr[i], "t")) {
                snode[snode_len].t = strtoll(attr[i + 1], NULL, 10);
            } else if (!strcmp(attr[i], "r")) {
                snode[snode_len].r = strtol(attr[i + 1], NULL, 10);
            } else if (!strcmp(attr[i], "d")) {
                snode[snode_len].d = strtol(attr[i + 1], NULL, 10);
            }
        }

        snode_len ++;
    } else if (!strcmp(el, "SegmentList")) {
        manifest->segmentlist_parse_flag = 1;

        if (manifest->adaptationset_parse_flag || manifest->representation_parse_flag) {
            for (i = 0; attr[i]; i += 2) {
                if (!strcmp(attr[i], "timescale")) {
                    if (manifest->representation_parse_flag && cur_representation) {
                        cur_representation->timescale = strtol(attr[i + 1], NULL, 10);
                    } else if (manifest->adaptationset_parse_flag && cur_adaptationset) {
                        cur_adaptationset->timescale = strtol(attr[i + 1], NULL, 10);
                    }
                } else if (!strcmp(attr[i], "duration")) {
                    if (manifest->representation_parse_flag && cur_representation) {
                        cur_representation->duration = strtol(attr[i + 1], NULL, 10);
                    } else if (manifest->adaptationset_parse_flag && cur_adaptationset) {
                        cur_adaptationset->duration = strtol(attr[i + 1], NULL, 10);
                    }
                }
            }
        }
    } else if (!strcmp(el, "Initialization")) {
        if (manifest->segmentlist_parse_flag == 1) {
            for (i = 0; attr[i]; i += 2) {
                if (!strcmp(attr[i], "sourceURL")) {
                    if (manifest->adaptationset_parse_flag || manifest->representation_parse_flag) {
                        if (manifest->representation_parse_flag && cur_representation) {
                            cur_representation->init_url = xml_strdup(attr[i + 1]);
                        } else if (manifest->adaptationset_parse_flag && cur_adaptationset) {
                            cur_adaptationset->init_url = xml_strdup(attr[i + 1]);
                        }
                    }
                }
            }
        }
    } else if (!strcmp(el, "SegmentURL")) {
        if (manifest->segmentlist_parse_flag == 1) {
            for (i = 0; attr[i]; i += 2) {
                if (!strcmp(attr[i], "media")) {
                    if (manifest->adaptationset_parse_flag || manifest->representation_parse_flag) {
                        if (manifest->representation_parse_flag && cur_representation) {
                            if (cur_representation->segment_list.media_url_list == NULL) {
                                cur_representation->segment_list.media_url_list = av_mallocz(sizeof(MPD_SegmentURL));

                                if (cur_representation->segment_list.media_url_list && strlen(attr[i + 1])) {
                                    strcpy(cur_representation->segment_list.media_url_list->media_url, attr[i + 1]);
                                    cur_representation->segment_list.cur_media_url = cur_representation->segment_list.media_url_list;
                                    cur_representation->segment_list.len++;
                                }
                            } else if (cur_representation->segment_list.cur_media_url) {
                                cur_representation->segment_list.cur_media_url->next_media_url = av_mallocz(sizeof(MPD_SegmentURL));

                                if (cur_representation->segment_list.cur_media_url->next_media_url && strlen(attr[i + 1])) {
                                    strcpy(cur_representation->segment_list.cur_media_url->next_media_url->media_url, attr[i + 1]);
                                    cur_representation->segment_list.cur_media_url = cur_representation->segment_list.cur_media_url->next_media_url;
                                    cur_representation->segment_list.len++;
                                }
                            }
                        } else if (manifest->adaptationset_parse_flag && cur_adaptationset) {
                            if (cur_adaptationset->segment_list.media_url_list == NULL) {
                                cur_adaptationset->segment_list.media_url_list = av_mallocz(sizeof(MPD_SegmentURL));

                                if (cur_adaptationset->segment_list.media_url_list && strlen(attr[i + 1])) {
                                    strcpy(cur_adaptationset->segment_list.media_url_list->media_url, attr[i + 1]);
                                    cur_adaptationset->segment_list.cur_media_url = cur_adaptationset->segment_list.media_url_list;
                                    cur_adaptationset->segment_list.len++;
                                }
                            } else if (cur_adaptationset->segment_list.cur_media_url) {
                                cur_adaptationset->segment_list.cur_media_url->next_media_url = av_mallocz(sizeof(MPD_SegmentURL));

                                if (cur_adaptationset->segment_list.cur_media_url->next_media_url && strlen(attr[i + 1])) {
                                    strcpy(cur_adaptationset->segment_list.cur_media_url->next_media_url->media_url, attr[i + 1]);
                                    cur_adaptationset->segment_list.cur_media_url = cur_adaptationset->segment_list.cur_media_url->next_media_url;
                                    cur_adaptationset->segment_list.len++;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}
static void XMLCALL xml_data(void * data, const char * s, int len)
{
    MPDManifest * manifest = data;
    uint8_t * src;
    uint8_t * dst;
    int   ret = 0;

    if (!len) {
        return;
    }

    switch (manifest->data_type) {
        case DATA_TYPE_NONE:
        case DATA_TYPE_MPD:
        case DATA_TYPE_MPD_PERIOD:
        case DATA_TYPE_MPD_PERIOD_REPRESENTATION:
        case DATA_TYPE_MPD_PERIOD_REPRESENTATION_SEGMEMGBASE: {
            break;
        }

        case DATA_TYPE_MPD_BASEURL: {
            if (manifest->mpd_parse_flag == 1) {
                if (manifest->period_parse_flag == 1) {
                    //mtos_printk("s :%.10s, len %d\n", s, len);
                    if(*(s+len) == '&')//for youtube
                    {
                        char *p_end = strstr(s,"</Base");
                        char tmp[2048] = {0};
                        memset(tmp, 0, 2048);
                        memcpy(tmp, s, (p_end-s) );
                        if (cur_representation)
                        {
                            if(cur_representation->base_url && strlen(cur_representation->base_url) > 1)
                                strcat(cur_representation->base_url, tmp);
                            else
                                strcpy(cur_representation->base_url, tmp);
                        }
                        else
                        {
                            cur_adaptationset->base_url = xml_strdup(tmp);
                        }
                        break;
                    }

                    if (manifest->adaptationset_parse_flag == 1) {
                        if (manifest->representation_parse_flag == 1) {
                            if (*s != 0xA && *s != 0x20) { //\n and space
                                char tmp[2048];
                                memset(tmp, 0, 2048);
                                memcpy(tmp, s, len);

                                if (cur_representation) {
                                    if(cur_representation->base_url && strlen(cur_representation->base_url) > 1)
                                        strcat(cur_representation->base_url, tmp);
                                    else
                                        strcpy(cur_representation->base_url, tmp);
                                }
                                else
                                {
                                    if(cur_adaptationset) {
                                        cur_adaptationset->base_url = xml_strdup(tmp);
                                    }
                                }
                            }
                        } else {
                            if (*s != 0xA && *s != 0x20) { //\n and space
                                char tmp[2048];
                                memset(tmp, 0, 2048);
                                memcpy(tmp, s, len);
                                if (cur_adaptationset) {
                                    cur_adaptationset->base_url = xml_strdup(tmp);
                                }
                            }
                        }
                    } else {
                        if (*s != 0xA && *s != 0x20) { //\n and space
                            char tmp[2048];
                            memset(tmp, 0, 2048);
                            memcpy(tmp, s, len);

                            if (cur_period) {
                                cur_period->base_url = xml_strdup(tmp);
                            }
                        }
                    }
                } else {
                    if (manifest->base_url == NULL) {
                        if (*s != 0xA && *s != 0x20) { //\n and space
                            char tmp[2048];
                            memset(tmp, 0, 2048);
                            memcpy(tmp, s, len);
                            manifest->base_url = xml_strdup(tmp);
                        }
                    }
                }
            }

            break;
        }

        case DATA_TYPE_MPD_CONTENTPROTECTION_MSPR:{
            cur_adaptationset->contentProtection.mspr = av_mallocz(len + 1);
            memcpy(cur_adaptationset->contentProtection.mspr, s, len);
            //printf("len %d, %s\n", len, cur_adaptationset->contentProtection.mspr);
            break;
        }

        case DATA_TYPE_MPD_ADAPTATIONSET: {
            break;
        }

        default: {
            av_log(NULL, AV_LOG_WARNING, " Invalid manifest data_type[%d] \n", manifest->data_type);
            break;
        }
    }

    manifest->data_type = DATA_TYPE_NONE;
    return;
}
static void XMLCALL xml_end(void * data, const char * el)
{
    MPDManifest * manifest = data;

    if (!strcmp(el, "MPD")) { //end
        manifest->parse_ret = XML_PARSE_DONE;
        manifest->mpd_parse_flag = 0;
    } else if (!strcmp(el, "AdaptationSet")) {
        manifest->adaptationset_parse_flag = 0;
    } else if (!strcmp(el, "Representation")) {
        manifest->representation_parse_flag = 0;
    /*} else if (!strcmp(el, "AdaptationSet")) {        // for tsscan   , condition matches previous condition at line 779
        manifest->period_parse_flag = 0;*/
    } else if (!strcmp(el, "SegmentList")) {
        manifest->segmentlist_parse_flag = 0;
    } else if (!strcmp(el, "SegmentTemplate")) {
        //printf("SegmentTemplate snode_len %d\n",snode_len);
        if (snode_len > 0) {
            if (cur_representation) {
                cur_representation->segmentTimeLine.snode = av_mallocz(snode_len * sizeof(MPD_SNode) + 1);

                if (cur_representation->segmentTimeLine.snode == NULL) {
                    return;
                }

                memcpy(cur_representation->segmentTimeLine.snode, snode, (snode_len * sizeof(MPD_SNode)));
                cur_representation->segmentTimeLine.len = snode_len;
            } else if (cur_adaptationset) {
                cur_adaptationset->segmentTimeLine.snode = av_mallocz(snode_len * sizeof(MPD_SNode) + 1);

                if (cur_adaptationset->segmentTimeLine.snode == NULL) {
                    return;
                }

                memcpy(cur_adaptationset->segmentTimeLine.snode, snode, (snode_len * sizeof(MPD_SNode)));
                cur_adaptationset->segmentTimeLine.len = snode_len;
            }
        }

        memset(snode, 0, sizeof(snode));
        snode_len = 0;
    }

    return;
}
int ff_parse_mpd_manifest(uint8_t * buffer, int size, MPDManifest * manifest)
{
    int done = 0;
    //char buf[4096];
    XML_Parser xmlp;
    int i, j;
    av_log(NULL, AV_LOG_DEBUG, "In parse_manifest \n");
    manifest->availabilityStartTime = -1LL;
    xmlp = XML_ParserCreate(NULL);

    if (!xmlp) {
        av_log(NULL, AV_LOG_ERROR, "Unable to allocate memory for the libexpat XML parser\n");
        return AVERROR(ENOMEM);
    }

    XML_SetUserData(xmlp, manifest);
    XML_SetElementHandler(xmlp, xml_start, xml_end);
    XML_SetCharacterDataHandler(xmlp, xml_data);
    memset(snode, 0, sizeof(snode));
    snode_len = 0;

    while (!done && !manifest->parse_ret) {
        if (XML_Parse(xmlp, buffer, size, done) == XML_STATUS_ERROR) {
            av_log(NULL, AV_LOG_ERROR, "Parse error at line %" XML_FMT_INT_MOD "u:\n%s\n",
                   XML_GetCurrentLineNumber(xmlp),
                   XML_ErrorString(XML_GetErrorCode(xmlp)));
            return AVERROR_INVALIDDATA;
        }
    }

    if (manifest->parse_ret == XML_PARSE_DONE) {
        manifest->parse_ret = 0;
    }

    XML_ParserFree(xmlp);
    return manifest->parse_ret;
}
int ff_free_mpd_manifest(MPDManifest * manifest)
{
    int i, j, k, l;
    MPD_Period * current_period = NULL;
    MPD_AdaptationSet * current_adaptationset = NULL;
    MPD_Representation * current_representation = NULL;
    MPD_SegmentURL * current_media_url = NULL;
    MPD_SegmentURL * current_media_url_tmp = NULL;

    if (manifest->profiles) {
        av_freep(&manifest->profiles);
    }

    if (manifest->base_url) {
        av_freep(&manifest->base_url);
    }

    for (i = 0; i < manifest->nb_period; i++) {
        current_period = manifest->period[i];

        if (current_period->base_url) {
            av_freep(&current_period->base_url);
        }

        for (l = 0; l < current_period->nb_adaptationset; l++) {
            current_adaptationset = current_period->adaptationset[l];

            if (current_adaptationset->init_url) {
                av_freep(&current_adaptationset->init_url);
            }

            if (current_adaptationset->lang) {
                av_freep(&current_adaptationset->lang);
            }

            if (current_adaptationset->media) {
                av_freep(&current_adaptationset->media);
            }

            if (current_adaptationset->base_url) {
                av_freep(&current_adaptationset->base_url);
            }

            if (current_adaptationset->codecs) {
                av_freep(&current_adaptationset->codecs);
            }

            if (current_adaptationset->mimeType) {
                av_freep(&current_adaptationset->mimeType);
            }

            if (current_adaptationset->segmentTimeLine.snode) {
                av_freep(&current_adaptationset->segmentTimeLine.snode);
            }

            if(current_adaptationset->contentProtection.mspr){
                av_freep(&current_adaptationset->contentProtection.mspr);
            }

            if(current_adaptationset->contentProtection.system_id){
                av_freep(&current_adaptationset->contentProtection.system_id);
            }

            current_media_url = current_adaptationset->segment_list.media_url_list;

            for (k = 0; k < current_adaptationset->segment_list.len; k++) {
                if (current_media_url) {
                    current_media_url_tmp = current_media_url->next_media_url;
                    av_freep(&current_media_url);
                    current_media_url = current_media_url_tmp;
                }
            }

            for (j = 0; j < current_adaptationset->nb_representation; j++) {
                current_representation = current_adaptationset->representation[j];

                for (k = 0; k < current_representation->segment_num; k++) {
                    if (current_representation->segmentList[k]) {
                        av_freep(&current_representation->segmentList[k]);
                    }
                }

                if (current_representation->segmentTimeLine.snode) {
                    av_freep(&current_representation->segmentTimeLine.snode);
                }

                if (current_representation->mimeType) {
                    av_freep(&current_representation->mimeType);
                }

                if (current_representation->codecs) {
                    av_freep(&current_representation->codecs);
                }

                if (current_representation->media) {
                    av_freep(&current_representation->media);
                }

                if (current_representation->init_url) {
                    av_freep(&current_representation->init_url);
                }

                if (current_representation->id) {
                    av_freep(&current_representation->id);
                }

                current_media_url = current_representation->segment_list.media_url_list;

                for (k = 0; k < current_representation->segment_list.len; k++) {
                    if (current_media_url) {
                        current_media_url_tmp = current_media_url->next_media_url;
                        av_freep(&current_media_url);
                        current_media_url = current_media_url_tmp;
                    }
                }

                av_freep(&current_representation);
            }

            av_freep(&current_adaptationset);
        }

        av_freep(&current_period);
    }

    memset(manifest, 0x00, sizeof(MPDManifest));
    return 0;
}
