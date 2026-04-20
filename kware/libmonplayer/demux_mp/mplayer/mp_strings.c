/*
 * Strings utilities
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
#ifdef __LINUX__
#include <stdlib.h>
#include <stdarg.h>
#include <stdio.h>
#else
#include <stdarg.h>
#include "mp_func_trans.h"
#endif

#include "mp_strings.h"
#include "file_playback_sequence.h"

char *mthttp_mp_asprintf(const char *fmt, ...)
{
    char *p = NULL;
    va_list va;
    int len;

    va_start(va, fmt);
    len = vsnprintf(NULL, 0, fmt, va);
    va_end(va);
    if (len < 0)
        goto end;
    #ifndef __LINUX__
    p = mthttp_malloc33(len + 1);
    #else
    p = (char *)malloc33(len + 1);
    #endif
    if (!p)
        goto end;

    va_start(va, fmt);
    len = vsnprintf(p, (size_t)len + 1, fmt, va);
    va_end(va);
    if (len < 0)
        #ifndef __LINUX__
        mthttp_free33(p), p = NULL;
        #else
        free33(p), p = NULL;
        #endif

end:
    return p;
}

char *mp_asprintf(const char *fmt, ...)
{
    char *p = NULL;
    va_list va;
    int len;

    va_start(va, fmt);
    len = vsnprintf(NULL, 0, fmt, va);
    va_end(va);
    if (len < 0)
        goto end;

    p = (char *)malloc33(len + 1);
    if (!p)
        goto end;

    va_start(va, fmt);
    len = vsnprintf(p, (size_t)len + 1, fmt, va);
    va_end(va);
    if (len < 0)
        free33(p), p = NULL;

end:
    return p;
}
