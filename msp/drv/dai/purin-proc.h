/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#ifndef _PURIN_PROC_H_
#define _PURIN_PROC_H_

#define SND_WAKELOCK_DISABLE    0
#define SND_WAKELOCK_ENABLE     1

#define SND_DEBUG_TYPE_PM           (1 << 0)
#define SND_DEBUG_TYPE_REGUPDATE    (1 << 1)
#define SND_DEBUG_RECOVERY          (1 << 2)

struct snd_info
{
    int wakelock_enabled;
    int debug_type;
    int wakelock_isr_skiptime;
};

#endif
