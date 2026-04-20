/*
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
#define MODULE_TAG "FP OPEN"
#include <ctype.h>
#ifdef __LINUX__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#endif

#include <unistd.h>
#include <fcntl.h>


#include "config.h"
#include "mp_msg.h"
#include "help_mp.h"

#ifdef __FreeBSD__
#include <sys/cdrio.h>
#endif

#include "m_option.h"
#include "stream.h"
#include "libmpdemux/demuxer.h"
#ifndef __ANDRIOD__
#include "sys_define.h"
#endif
#include "mt_type.h"
#include "mlog.h"
#include "mtos_printk.h"

/// We keep these 2 for the gui atm, but they will be removed.
int dvd_last_chapter = 0;
stream_t *open_stream(const char *filename, char **options, int *file_format)
{
    int dummy = DEMUXER_TYPE_UNKNOWN;
    if (!file_format) {
        file_format = &dummy;
    }
    // Check if playlist or unknown
    if (*file_format != DEMUXER_TYPE_PLAYLIST) {
        *file_format = DEMUXER_TYPE_UNKNOWN;
    }

    if (!filename) {
        MLOGE("NULL filename, report this bug\n");
        return NULL;
    }
    //============ Open STDIN or plain FILE ============
    return open_stream_full(filename, STREAM_READ, options, file_format);
}
