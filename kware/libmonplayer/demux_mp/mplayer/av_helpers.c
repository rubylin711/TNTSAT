/*
 * Generic libav* helpers
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

#include "mutil.h"
#include "mlog.h"
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "mp_msg.h"
#include "av_helpers.h"

int avformat_initialized;
AVDictionary *ff_avformat_opts = NULL;

static void show_av_version(int type, const char *name,
                            int header_ver, int ver, const char *conf)
{
#ifdef CONFIG_FFMPEG_SO
#define FFMPEG_TYPE "external"
#else
#define FFMPEG_TYPE "internal"
#endif
    mp_msg(type, MSGL_INFO, "%s version %d.%d.%d (" FFMPEG_TYPE ")\n",
           name, ver >> 16, (ver >> 8) & 0xFF, ver & 0xFF);
    if (header_ver != ver)
        mp_msg(type, MSGL_INFO, "Mismatching header version %d.%d.%d\n",
               header_ver >> 16, (header_ver >> 8) & 0xFF, header_ver & 0xFF);
    mp_msg(type, MSGL_V, "Configuration: %s\n", conf);
}

void init_avformat(void)
{
    if (!avformat_initialized) {
        show_av_version(MSGT_DEMUX, "libavformat", LIBAVFORMAT_VERSION_INT,
                        (int)avformat_version(), avformat_configuration());
        av_register_all();
        avformat_network_init();
        avformat_initialized = 1;
        //av_log_set_callback(mp_msp_av_log_callback);
        av_log_set_level(AV_LOG_ERROR);
    }

    int log_level = mlog_check_gobal_log_level();
    if (log_level) {
        av_log_set_level(log_level);
    }
}
