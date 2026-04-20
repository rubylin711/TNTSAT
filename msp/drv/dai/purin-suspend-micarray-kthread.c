/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/kthread.h>
#include <asm/mach-panther/panther.h>
#include "purin-audio.h"
extern volatile u32 purin_suspend_mic;
extern unsigned int i2c_dev_num;

static int purin_micarray_suspend_kthread(void *priv)
{
    struct external_audio_data *data = (struct external_audio_data *) priv;
    unsigned int echo_cancellation_idx = i2c_dev_num - 1;

    do {
        if (purin_suspend_mic == MICARRAY_POWERSAVING_SUSPEND) {
            // i2c control es7243 suspend
            if (data->micarray_suspend) {
                data->micarray_suspend(echo_cancellation_idx);
                purin_suspend_mic = MICARRAY_POWERSAVING_NONE;
            }
        }
        else if (purin_suspend_mic == MICARRAY_POWERSAVING_RESUME) {
            // i2c control es7243 resume
            if (data->micarray_resume) {
                data->micarray_resume(echo_cancellation_idx);
            }
            purin_suspend_mic = MICARRAY_POWERSAVING_NONE;
        }

        // minimum is 1 jiffies = 10 ms, even if filled with HZ/200
        schedule_timeout_interruptible(HZ/100);
    } while(!kthread_should_stop());

    return 0;
}

int purin_suspend_micarray_kthread_init(struct external_audio_data *data)
{
    int ret = 0;
    purin_suspend_mic = MICARRAY_POWERSAVING_NONE;

    if (data->micarray_task == NULL) {

        data->micarray_task = kthread_run(purin_micarray_suspend_kthread, data, "purin-mic-powersaving");

        if (IS_ERR(data->micarray_task)) {

            data->micarray_task = NULL;
            ret = -ENOMEM;
        }
    }
    else
    {
        ret = -ENOMEM;
    }

    return ret;
}
void purin_suspend_micarray_kthread_cleanup(struct external_audio_data *data)
{
    if (data->micarray_task) {
        kthread_stop(data->micarray_task);
    }
    data->micarray_task = NULL;
}

