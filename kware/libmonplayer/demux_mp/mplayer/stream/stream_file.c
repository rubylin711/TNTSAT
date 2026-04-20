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

#include "config.h"
#define MODULE_TAG "FP STREAM FILE"
#include "mlog.h"
#include "mutil.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#if HAVE_SETMODE
#include <io.h>
#endif
#include "mp_msg.h"
#include "stream.h"
#include "help_mp.h"
#include "m_option.h"
#include "m_struct.h"
#include "mt_type.h"
#include "file_playback_sequence.h"

#ifdef CONFIG_MT_LXC_SUPPORT
#include "lxc_ipc.h"
#include "mt_unf_ipcfs_type.h"

ipc_fs_handle_t g_ipc_fs;
#endif
#include "mt_common.h"

u64 mp_rec_bytes = 0;
static int mp_timeshift_size = 0;
static int file_size = 0;
u64 recv_bytes = 0;
int recding_index = 0;
static int rec_index = 1;
static u64 recv_bytes_inter;
int play_to_recpos = 0;
int video_es_type = -1;

/*********** VMX START **********/
//#define MT_FP_VMX_DECRYPTION 1
#ifdef CONFIG_MT_FP_VMX_DECRYPTION
#define PUSHVOD_PRINTF printf
#define VMX_MMZ_BUF_NUM (2)
typedef int (*ExtraVMXCallBack)(off_t file_pos,
                                u8 *pu8DestVirAddr, u8 *pu8SrcDataVirAddr,
                                off_t u64Offset, off_t u64DataSize);
ExtraVMXCallBack g_vmx_callback = NULL;
int g_vmx_malloc_dma_buf = 0;
mt_mmz_buf_s  g_vmx_decrypt_rec_buff[2];

int MT_FP_RegisterExtraCallback(u32 u32ChnID, ExtraVMXCallBack fCallback, void *args)
{
    int i;
    g_vmx_callback = fCallback;
    if (g_vmx_callback == NULL) {
        if (g_vmx_malloc_dma_buf) {
            for (i = 0; i < VMX_MMZ_BUF_NUM; i++) {
                if (g_vmx_decrypt_rec_buff[i].user_viraddr) {
                    mt_mmz_free(&g_vmx_decrypt_rec_buff[i].user_viraddr);
                }
                g_vmx_decrypt_rec_buff[i].user_viraddr = NULL;
                g_vmx_decrypt_rec_buff[i].bufsize = 0;
            }
        }
        g_vmx_malloc_dma_buf = 0;
    } else {
        if (g_vmx_malloc_dma_buf == 0) {
            for (i = 0; i < VMX_MMZ_BUF_NUM; i++) {
                g_vmx_decrypt_rec_buff[i].user_viraddr = NULL;
                g_vmx_decrypt_rec_buff[i].bufsize = 0;
            }
        }
    }
    return 0;
}

#endif
/*********** VMX END ************/

static int fill_buffer(stream_t *s, char *buffer, int max_len)
{
#ifdef CONFIG_MT_FP_VMX_DECRYPTION
    off_t file_pos = 0;
    int i = 0;
    int ret;
    int align_size = (max_len / (188 * 4)) * 188 * 4;
    if (g_vmx_callback && (g_vmx_malloc_dma_buf == 0 || g_vmx_decrypt_rec_buff[i].bufsize != max_len)) {
        for (i = 0; i < VMX_MMZ_BUF_NUM; i++) {
            if (g_vmx_decrypt_rec_buff[i].user_viraddr) {
                mt_mmz_free(&g_vmx_decrypt_rec_buff[i].user_viraddr);
            }
            g_vmx_decrypt_rec_buff[i].bufsize = max_len;
            ret = mt_mmz_malloc(&g_vmx_decrypt_rec_buff[i]);
            if (MT_SUCCESS != ret) {
                return (-1);
            }
            memset(g_vmx_decrypt_rec_buff[i].user_viraddr, 0, g_vmx_decrypt_rec_buff[i].bufsize);
        }
        g_vmx_malloc_dma_buf = 1;
    }
#endif

#ifdef CONFIG_MT_FP_VMX_DECRYPTION

    if (g_vmx_callback) {
        file_pos = lseek(s->fd, 0, SEEK_CUR);
        file_pos = lseek(s->fd, (file_pos / 188) * 188, SEEK_SET);
    } else {
        align_size = max_len;
        g_vmx_decrypt_rec_buff[0].user_viraddr = buffer;
    }
#ifdef CONFIG_MT_LXC_SUPPORT
    int r = ipc_fs_read(&g_ipc_fs, s->fd, buffer, max_len);
#else
    int r = read(s->fd, g_vmx_decrypt_rec_buff[0].user_viraddr, align_size);
#endif

#else

#ifdef CONFIG_MT_LXC_SUPPORT
    int r = ipc_fs_read(&g_ipc_fs, s->fd, buffer, max_len);
#else
    int r = read(s->fd, buffer, max_len);
#endif

#endif

#ifdef CONFIG_MT_FP_VMX_DECRYPTION
    if (g_vmx_callback && r > 0) {
        g_vmx_callback(file_pos, \
                       g_vmx_decrypt_rec_buff[1].user_viraddr, g_vmx_decrypt_rec_buff[0].user_viraddr, \
                       0, (off_t)r);
        memcpy(buffer, g_vmx_decrypt_rec_buff[1].user_viraddr, r);
    }
#endif
    // We are certain this is EOF, do not retry
    if (max_len && r == 0) {
        s->eof = 1;
    }

#ifdef CONFIG_MT_FP_VMX_DECRYPTION
    if (g_vmx_callback && max_len && r < 188) {
        s->eof = 1;
        r = -1;
    }
#endif
    return (r <= 0) ? -1 : r;
}

static int write_buffer(stream_t *s, char *buffer, int len)
{
    int r;
    int wr = 0;

    while (wr < len) {

#ifdef CONFIG_MT_LXC_SUPPORT
        r = ipc_fs_write(&g_ipc_fs, s->fd, buffer, len);
#else
        r = write(s->fd, buffer, len);
#endif
        if (r <= 0) {
            return -1;
        }
        wr += r;
        buffer += r;
    }

    return len;
}

static int seek(stream_t *s, off_t newpos)
{
    if (mp_timeshift_size > 0) {
        newpos = MAX(newpos, 1024);
    }

    if (newpos > s->end_pos && s->end_pos > 0) {
        return 0;
    }

    if (file_size > 0 && fp_is_timeshift_file() == 0) {
        newpos = MIN(file_size, newpos);
    }

    s->pos = newpos;
    if (fp_is_timeshift_file() == 1) {
        int seek_index = 0;
        u64 mp_timeshift_byte = ((u64)mp_timeshift_size) * 1024;

        if (newpos < mp_rec_bytes) {
            s->pos = newpos;
        } else {
            s->pos = mp_rec_bytes;
        }

        recv_bytes = s->pos;
        seek_index = s->pos / mp_timeshift_byte + 1;

        if (seek_index != rec_index) {
            int ret = 0;
            rec_index = seek_index;
#ifndef __ANDRIOD__
            ret = open_ts_file(s);
#endif
            if (ret != 0) {
                close(s->fd);
                s->fd = 0;
            }
        }

        recv_bytes_inter = s->pos % mp_timeshift_byte;
    }
#ifdef CONFIG_MT_LXC_SUPPORT
    if (ipc_fs_lseek(&g_ipc_fs, s->fd, s->pos, SEEK_SET) < 0) {
        printf("zx ipc_fs_lseek error!!!!\n");
        s->eof = 1;
        return 0;
    }
#else
    if (lseek(s->fd, s->pos, SEEK_SET) < 0) {
        s->eof = 1;
        return 0;
    }
#endif
    return 1;
}

static int seek_forward(stream_t *s, off_t newpos)
{
    if (newpos < s->pos) {
        mp_msg(MSGT_STREAM, MSGL_INFO, "Cannot seek backward in linear streams!\n");
        return 0;
    }

    while (s->pos < newpos) {
        int len = s->fill_buffer(s, s->buffer, io_stream_buffer_size);

        if (len <= 0) {
            s->eof = 1; // EOF
            s->buf_pos = s->buf_len = 0;
            break;
        }

        s->buf_pos = 0;
        s->buf_len = len;
        s->pos += len;
    }
    return 1;
}

static int control(stream_t *s, int cmd, void *arg)
{
    switch (cmd) {
        case STREAM_CTRL_GET_SIZE: {
            off_t size;

#ifdef CONFIG_MT_LXC_SUPPORT
            struct stat data;
            int ret = ipc_fs_fstat(&g_ipc_fs, s->fd, &data);
            size = data.st_size;
            MLOGD("%s %d control zx size %ld\n", __FUNCTION__, __LINE__, size);
#else
            size = lseek(s->fd, 0, SEEK_END);
            lseek(s->fd, s->pos, SEEK_SET);
#endif
            if (size != (off_t) - 1) {
                *(uint64_t *)arg = size;
                MLOGD("[%s] stream_file get size: %lld ,return 1\n", __func__, size);
                return 1;
            }
        }
        case STREAM_CTRL_GET_SOURCE_TYPE: {
            *(unsigned int *) arg = STREAM_SOURCE_TYPE_NORMAL_FILE;
            return STREAM_OK;
        }
    }
    return STREAM_UNSUPPORTED;
}

#ifdef CONFIG_MT_LXC_SUPPORT
static void close_f(stream_t *s)
{
    if (s->fd > 0) {
        /* on unix we define closesocket to close
           on windows however we have to distinguish between
           network socket and file */
        ipc_fs_close(&g_ipc_fs, s->fd);
        ipc_fs_deinit(&g_ipc_fs);
    }
}
#endif

static int open_f(stream_t *stream, int mode, void *opts, int *file_format)
{
    int f = 0;
    mode_t m = 0;
    file_size = 0;
    off_t len;
    unsigned char *filename = NULL;

    if (mode == STREAM_READ) {
        m = O_RDONLY;
    } else if (mode == STREAM_WRITE) {
        m = O_RDWR | O_CREAT | O_TRUNC;
    } else {
        MLOGE("[file] Unknown open mode %d\n", mode);
        return STREAM_UNSUPPORTED;
    }

    filename = (unsigned char *)opts;
    if (!filename) {
        MLOGE("[file] No filename\n");
        return STREAM_ERROR;
    }

    m |= O_BINARY;
    if (!strcmp(filename, "-")) {
        if (mode == STREAM_READ) {
            // read from stdin
            mp_msg(MSGT_OPEN, MSGL_INFO, MSGTR_ReadSTDIN);
            f = 0; // 0=stdin
        } else {
            mp_msg(MSGT_OPEN, MSGL_INFO, "Writing to stdout\n");
            f = 1;
        }
    } else {
        mode_t openmode = S_IRUSR | S_IWUSR;
#ifndef __MINGW32__
        openmode |= S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH;
#endif
#ifdef CONFIG_MT_LXC_SUPPORT
        char fs_name[LXC_IPC_NAME_SIZE] = "ipc_fs_server";
        int ret = ipc_fs_init(&g_ipc_fs, fs_name);
        f = ipc_fs_open(&g_ipc_fs, filename, O_RDWR | O_CREAT, S_IRWXU);
        MLOGI("ipc_fs_fopen[%s] fd %d, ret %d\n", filename, f, ret);
#else
        f = open(filename, m, openmode);
#endif
        if (f < 0) {
            mp_msg(MSGT_OPEN, MSGL_ERR, MSGTR_FileNotFound, filename);
            return STREAM_ERROR;
        }
    }
#ifdef CONFIG_MT_LXC_SUPPORT
    struct stat data;
    int ret = ipc_fs_fstat(&g_ipc_fs, f, &data);
    len = data.st_size;
#else
    len = lseek(f, 0, SEEK_END);
    lseek(f, 0, SEEK_SET);
#endif
    if (len == -1) {
        if (mode == STREAM_READ) {
            stream->seek = seek_forward;
        }
        stream->type = STREAMTYPE_STREAM; // Must be move to STREAMTYPE_FILE
        stream->flags |= MP_STREAM_SEEK_FW;
    } else if (len >= 0) {
        stream->seek = seek;
        stream->end_pos = len;
        stream->type = STREAMTYPE_FILE;
    }
    MLOGI("[file] File size is %" PRId64 " bytes\n", (int64_t)len);
    stream->fd = f;
    stream->fill_buffer = fill_buffer;
    stream->write_buffer = write_buffer;
    stream->control = control;
#ifdef CONFIG_MT_LXC_SUPPORT
    stream->close = close_f;
#endif
    stream->is_chunked = 1;
    stream->read_chunk = 64 * 1024;
    return STREAM_OK;
}

const stream_info_t stream_info_file = {
    "File",
    "file",
    "Albeu",
    "based on the code from (probably Arpi)",
    open_f,
    { "file", "", NULL },
    NULL,
    1 // Urls are an option string
};
