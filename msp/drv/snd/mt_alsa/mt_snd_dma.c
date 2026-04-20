/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2016 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/errno.h>
#include <linux/ioctl.h>
#include <linux/delay.h>
#include <linux/slab.h>
#include <linux/timer.h>
#include <linux/kthread.h>
#include <linux/interrupt.h>
#include <linux/dma-mapping.h>
#include <asm/io.h>
#include <linux/jiffies.h>
#include <linux/time.h>
#include <linux/wait.h>
#ifdef CONFIG_PM
 #include <linux/pm.h>
#endif
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/control.h>
#include <sound/initval.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include "mt_type.h"
#include "mt_snd_comm.h"
#include "mach/symphony_reg_base_addr.h"
#include "mt_mach/symphony_io.h"
#include "mt_drv_mmz.h"
#include "mt_common.h"
#include "mt_drv_dma.h"
#include "mt_cache.h"
#include <linux/kthread.h>
#include <uapi/linux/sched/types.h>
#include <linux/err.h>

mmz_buffer_s aout_buf;
mmz_buffer_s pcm_buf;
mmz_buffer_s tmp_buf;

unsigned int aout_size;
static unsigned int tmp_size;
static unsigned int pcm_size;
static unsigned int pcm_rd;
static unsigned int pcm_wt;
static unsigned int aout_wt;
static struct task_struct * g_snd_process_task = NULL;

static int snd_started;
static int snd_stopped;

extern void *audio_dma_memcpy(void *to, const void *from, size_t n, unsigned int ch_id);

#define MT_SND_BUFFER_SIZE    1024*256
static const struct snd_pcm_hardware snd_hw_hardware = {
	.info		= SNDRV_PCM_INFO_MMAP |
				  SNDRV_PCM_INFO_MMAP_VALID |
				  SNDRV_PCM_INFO_INTERLEAVED |
				  SNDRV_PCM_INFO_BLOCK_TRANSFER |
				  SNDRV_PCM_INFO_PAUSE |
				  SNDRV_PCM_INFO_RESUME,
	.formats	= MT_SND_FORMATS,
    .rates 		= MT_SND_RATES,
	.channels_min		= 1,
	.channels_max		= 8,
	.period_bytes_min	= 640,     
    .period_bytes_max	= 640*8,    
	.periods_min		= 100,
	.periods_max		= 200,
	.buffer_bytes_max	= MT_SND_BUFFER_SIZE,
};

static int mt_aout_buffer_full(unsigned int size)
{
	unsigned int aout_len = (MT_SND_REG_READ32(0x1c)) << 3;
	unsigned int aout_thrhd = (MT_SND_REG_READ32(0x20)) << 3;
	unsigned int aout_cnt = MT_SND_REG_READ32(0x0104) << 3;
	
	if ((aout_cnt + aout_thrhd + size) >= aout_len - 0x20)
		return 1;
	
	return 0;
}

static int mt_pcm_buffer_avail(unsigned int size)
{
	unsigned int pcm_avail = 0;

	if(pcm_wt >= pcm_rd)
		pcm_avail = pcm_wt - pcm_rd;
	else
		pcm_avail = pcm_size - pcm_rd + pcm_wt;

	return (pcm_avail >= size)?1:0;
}

static int mt_snd_process_thread(void *data)
{
	int i = 0;
	int j = 0;
	int ch = 0;
	int size = 0;
	int size1st = 0;
	int size2nd = 0;
	int depth = 2;//16bits pcm
	int samples = 0;
	int sample_bytes = 0;
	int zero_ch = 0;
	int copy_user_size = 0;
	//struct snd_soc_pcm_runtime *rtd = NULL;
	//struct snd_soc_component *component = NULL;
	char *pcm_addr = (char *)pcm_buf.startVirAddr;
	char *tmp_addr = (char *)tmp_buf.startVirAddr;

	struct snd_pcm_substream *substream = (struct snd_pcm_substream *)data;
	struct snd_pcm_runtime *runtime = substream->runtime;

	while(snd_started == 0){
		msleep(1);
	}

	//rtd = asoc_substream_to_rtd(substream);
	//for(i=0; i<rtd->num_components; i++) {
		//component = rtd->components[i];
		//pr_err("name:%s %s\n",component->name, component->driver->name);
	//}
	
	size = runtime->period_size * (runtime->sample_bits>>3);

	ch = runtime->channels;
	if(ch > 2) {
		size <<= 3;
		sample_bytes = 16;//8chnnel mode 2B*8ch
		zero_ch = 8 - ch;
		copy_user_size = (size/8)*ch;
	} else {
		size <<= 1;
		sample_bytes = 4;//2channel mode 2B*2ch
		zero_ch = 2 - ch;
		copy_user_size = (size/2)*ch;
	}
	//pr_err("process thread process size:0x%x, period size:0x%lx, sample bits:0x%x, frame bits:0x%x, pcm unit:0x%lx, ch=%d\n",
		//size, runtime->period_size,runtime->sample_bits,runtime->frame_bits, runtime->period_size * (runtime->frame_bits>>3), ch);

	if(size > tmp_size) 
		pr_err("pcm temp buffer is too small, need 0x%x, actual:0x%x\n",size,tmp_size);

	//transfer pcm buffer data to tmp buffer
	while(1) {
		while(!mt_pcm_buffer_avail(runtime->period_size * (runtime->frame_bits>>3)) && !snd_stopped)
			msleep(1);

		if(snd_stopped)
			break;
		
		samples = size/sample_bytes;
		for(i=0; i<samples; i++){
			for(j=0; j<depth*ch; j++) {
				tmp_addr[sample_bytes*i+j] = pcm_addr[pcm_rd+j];
			}

			pcm_rd += depth*ch;
			pcm_rd %= pcm_size;

			for(j=0; j<depth*zero_ch; j++) {
				tmp_addr[sample_bytes*i+depth*ch+j] = 0;
			}
		}

		while(mt_aout_buffer_full(size) && !snd_stopped)
			msleep(1);

		mt_dcache_flush(tmp_buf.startVirAddr, size);
		if(aout_size - aout_wt > size){
			audio_dma_memcpy((void *)aout_buf.startPhyAddr+aout_wt, (void *)tmp_buf.startPhyAddr, size, DMA_CHANNEL_SECURE_AUDIO_1);
			MT_SND_REG_WRITE32(REG_AUD_PCM_BUF_WRCMD,  (size >> 3) | 0x80000000);
		} else {
			size1st = aout_size - aout_wt;
			audio_dma_memcpy((void *)aout_buf.startPhyAddr+aout_wt, (void *)tmp_buf.startPhyAddr, size1st, DMA_CHANNEL_SECURE_AUDIO_1);
			MT_SND_REG_WRITE32(REG_AUD_PCM_BUF_WRCMD,  (size1st >> 3) | 0x80000000);
			
			size2nd = size - size1st;
			audio_dma_memcpy((void *)aout_buf.startPhyAddr+aout_wt+size1st, (void *)tmp_buf.startPhyAddr+size1st, size2nd, DMA_CHANNEL_SECURE_AUDIO_1);
			MT_SND_REG_WRITE32(REG_AUD_PCM_BUF_WRCMD,  (size2nd >> 3) | 0x80000000);
		}	

		aout_wt += size;
		aout_wt %=aout_size;

	}
	
	return 0;
}

static int mt_snd_dma_copy_user(struct snd_soc_component *component,
			       struct snd_pcm_substream *substream, int channel,
			       unsigned long pos, struct iov_iter *buf,
			       unsigned long bytes)
{
	unsigned int retry_cnt = 0;
	unsigned int pcm_free = 0;
	unsigned int no_wrap_size = 0;

	while(1) {
		if(pcm_wt >= pcm_rd)
			pcm_free = pcm_size - pcm_wt + pcm_rd;
		else
			pcm_free = pcm_rd - pcm_wt;

		if(pcm_free > bytes)
			break;
		
		if(retry_cnt > 100){
			pr_err("write failed!\n");
			return MT_FAILURE;
		}
		
		msleep(1);
		retry_cnt++;
	}	

	if(pcm_wt >= pcm_rd) {
		no_wrap_size = pcm_size - pcm_wt;
		if(no_wrap_size >= bytes) {
			if (copy_from_iter((void __force *)pcm_buf.startVirAddr+pcm_wt, bytes, buf) != bytes) {
				return MT_FAILURE;
			}
		} else {
			if (copy_from_iter((void *)pcm_buf.startVirAddr+pcm_wt, no_wrap_size, buf) != no_wrap_size) {
				return MT_FAILURE;
			}

			if (copy_from_iter((void *)pcm_buf.startVirAddr, bytes-no_wrap_size, buf) != bytes-no_wrap_size) {
				return MT_FAILURE;
			}
		}
		
	} else {
		if (copy_from_iter((void *)pcm_buf.startVirAddr+pcm_wt, bytes, buf) != bytes) {
			return MT_FAILURE;
		}
	}

	pcm_wt += bytes;
	pcm_wt %= pcm_size;
	
	return MT_SUCCESS;
}

static irqreturn_t mt_snd_isr(int irq, void *dev)
{
	struct snd_pcm_substream *substream = dev;
	int int_status = 0;

	int_status = MT_SND_REG_READ32(REG_AUD_INTR_SET);
	MT_SND_REG_WRITE32(REG_AUD_INTR_SET,  int_status);
	snd_pcm_period_elapsed(substream); 

	return IRQ_HANDLED;
}

static int mt_snd_dma_prepare(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
	struct mt_soundaudio_data *priv = snd_soc_component_get_drvdata(component);

    priv->snd_started = 1;
	snd_started = 1;
	//dump_done = 0;

    return 0;    
}
static int mt_snd_dma_trigger(struct snd_soc_component *component, struct snd_pcm_substream *substream, int cmd)
{
    int ret = 0;

    switch(cmd) {   
	    case SNDRV_PCM_TRIGGER_START:
			MT_SND_REG_WRITE32(REG_AUD_PLAY_STOP, 0);
	        break;

	    case SNDRV_PCM_TRIGGER_RESUME:
	    case SNDRV_PCM_TRIGGER_PAUSE_RELEASE:
	        break;
			
	    case SNDRV_PCM_TRIGGER_STOP:
	    case SNDRV_PCM_TRIGGER_SUSPEND:
	    case SNDRV_PCM_TRIGGER_PAUSE_PUSH:
	        break;  
			
	    default:
	        ret = -EINVAL;
	        break;
    }
    return 0;
}

static snd_pcm_uframes_t mt_snd_dma_pointer(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{	
	int len = 0;
	int r_index = 0;
	int offset = 0;
	int pp_buf_base = 0;
	int pp_buf_raddr = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	
	len = MT_SND_REG_READ32(REG_AUD_PCM_BUF_LEN);
	pp_buf_base = MT_SND_REG_READ32(REG_AUD_PCM_BUF_BASE);
	pp_buf_raddr = MT_SND_REG_READ32(REG_AUD_PCM_RADDR);
	r_index = (pp_buf_raddr - pp_buf_base)<<3;//bytes
	if(runtime->channels > 2) {
		r_index >>= 3;//per channel
		r_index *= runtime->channels;
	} else {
		r_index >>= 1;//per channel
		r_index *= runtime->channels;		
	}
	r_index %= runtime->dma_bytes;
	offset = bytes_to_frames(runtime, r_index);
	
	return offset;
}

static int mt_snd_dma_hwparams(struct snd_soc_component *component, struct snd_pcm_substream *substream,
    struct snd_pcm_hw_params *params)
{
	int ret = 0;
	int err = 0;
	//unsigned int sample_bytes = 0;
    struct snd_pcm_runtime *runtime = substream->runtime;
	struct mt_soundaudio_data *priv = snd_soc_component_get_drvdata(component);
	struct snd_dma_buffer *dmab = NULL; 
	struct sched_param param;

	//pr_err("mt_snd_dma_hwparams get priv:%p %p!!!\n",priv, substream->private_data);
	//sample_bytes = params_width(params)>>3;
    //pcm_size = params_buffer_size(params) * sample_bytes * 8;//max 8 channel
    pcm_size = params_buffer_bytes(params)/params_channels(params) * 8;//max 8 channel

	//substream->dma_buffer.dev.type = SNDRV_DMA_TYPE_DEV;

    //if(snd_pcm_lib_malloc_pages(substream, buffer_bytes) < 0){
	//	pr_err("malloc dma buffer failed!");
    //    return -ENOMEM;
    //}

	//pr_err("param buffer size:%d %d, pcm_size:%d ch:%d, sample bytes:%d",
	//params_buffer_size(params) * sample_bytes, params_buffer_bytes(params),pcm_size, params_channels(params),sample_bytes);
	dmab = kzalloc(sizeof(*dmab), GFP_KERNEL);
	if (! dmab)
		return -ENOMEM;

	priv->dmab = dmab;

	pcm_rd = 0;
	pcm_wt = 0;
	memset(&pcm_buf, 0, sizeof(mmz_buffer_s));
    ret = mt_drv_mmz_alloc_and_map("snd_pcm", MMZ_ZONE_DDR, pcm_size, 128, &pcm_buf);
    if (MT_SUCCESS != ret) {
        pr_err("MMZ_AllocAndMap for pcm buffer failed\n");
        return -ENOMEM;
    }


	dmab->dev = substream->dma_buffer.dev;
	dmab->dev.type = substream->dma_buffer.dev.type;
	dmab->dev.dev = substream->dma_buffer.dev.dev;
	dmab->area = (unsigned char *)pcm_buf.startVirAddr;
	dmab->addr = pcm_buf.startPhyAddr;
	dmab->bytes = pcm_size;

	runtime->dma_buffer_p = dmab;
	runtime->dma_area = dmab->area;
	runtime->dma_addr = dmab->addr;
	runtime->dma_bytes = (dmab->bytes>>3)*params_channels(params);//we malloc 8ch actually, alsa only see the real channels
	//pr_err("malloc dma addr 0x%x size:0x%x, pcm size:0x%x\n",runtime->dma_addr, runtime->dma_bytes,pcm_size);

	tmp_size = 0x2000;
	memset(&tmp_buf, 0, sizeof(mmz_buffer_s));
    ret = mt_drv_mmz_alloc_and_map("snd_tmp", MMZ_ZONE_DDR, tmp_size, 128, &tmp_buf);
    if (MT_SUCCESS != ret) {
        pr_err("MMZ_AllocAndMap for tmp buffer failed\n");
        return -ENOMEM;
    }

	aout_wt = 0;
	aout_size = pcm_size;
	memset(&aout_buf, 0, sizeof(mmz_buffer_s));
	ret = mt_drv_mmz_alloc_and_map("snd_aout", MMZ_ZONE_PCM, aout_size, 128, &aout_buf);
	if (MT_SUCCESS != ret) {
		pr_err("MMZ_AllocAndMap for aout buffer failed\n");
		return -ENOMEM;
	}

    priv->hwparam.channels = params_channels(params);
    priv->hwparam.rate = params_rate(params);
    priv->hwparam.format = params_format(params);
    priv->hwparam.periods = params_periods(params);
    priv->hwparam.period_size = params_period_size(params);
    priv->hwparam.buffer_size = params_buffer_size(params);
    priv->hwparam.buffer_bytes = params_buffer_bytes(params);
	priv->snd_started = 0;
	priv->snd_stopped = 0;
	snd_started = 0;
	snd_stopped = 0;

    switch (priv->hwparam.format) {
        case SNDRV_PCM_FORMAT_S16_LE:
        	priv->hwparam.frame_size = 2 * priv->hwparam.channels;
        	break;

        default:
            break;
    }

	
	g_snd_process_task = kthread_create(mt_snd_process_thread, substream, "mt_snd_process_thread"); 
	if(IS_ERR(g_snd_process_task)){ 
	  printk("Unable to start kernel thread: mt_snd_process_thread "); 
	  err = PTR_ERR(g_snd_process_task); 
	  g_snd_process_task = NULL; 
	  return err; 
	} 
	
	param.sched_priority = 99;
	sched_setscheduler(g_snd_process_task, SCHED_RR, &param);
	wake_up_process(g_snd_process_task); 

    return 0;
}

static int mt_snd_dma_hwfree(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
    int ret = MT_SUCCESS;
    return ret;
}
static int mt_snd_dma_open(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{
    int ret = 0;
	struct snd_pcm_runtime *runtime = substream->runtime;
	
	snd_pcm_hw_constraint_step(runtime, 0,
				   SNDRV_PCM_HW_PARAM_PERIOD_BYTES, 128);
	snd_pcm_hw_constraint_step(runtime, 0,
				   SNDRV_PCM_HW_PARAM_BUFFER_BYTES, 128);
	snd_pcm_hw_constraint_integer(substream->runtime,
				      SNDRV_PCM_HW_PARAM_PERIODS);

    snd_soc_set_runtime_hwparams(substream, &snd_hw_hardware);

	ret = request_irq(IRQ_AOUT_ID, mt_snd_isr, IRQF_SHARED, "MT_SND", (void *)substream);
		
    return ret;
}

static int mt_snd_dma_close(struct snd_soc_component *component, struct snd_pcm_substream *substream)
{	
	struct mt_soundaudio_data *priv = snd_soc_component_get_drvdata(component);

	priv->snd_stopped = 1;
	snd_stopped = 1;
	free_irq(IRQ_AOUT_ID, (void *)substream);
    //snd_pcm_lib_free_pages(substream);
   	mt_drv_mmz_unmap(&aout_buf);
	mt_drv_mmz_release(&aout_buf);
   	mt_drv_mmz_unmap(&pcm_buf);
	mt_drv_mmz_release(&pcm_buf);
   	mt_drv_mmz_unmap(&tmp_buf);
	mt_drv_mmz_release(&tmp_buf);
	if(priv->dmab)
		kfree(priv->dmab);
	
    return MT_SUCCESS;
}

static const struct snd_soc_component_driver mt_soc_sound_platform_drv = {
	.open = mt_snd_dma_open,
	.close = mt_snd_dma_close,
	//.ioctl	= snd_pcm_lib_ioctl,
	.hw_params = mt_snd_dma_hwparams,
	.hw_free = mt_snd_dma_hwfree,
	.prepare = mt_snd_dma_prepare,
	.trigger = mt_snd_dma_trigger,
	.pointer = mt_snd_dma_pointer,
	.copy = mt_snd_dma_copy_user,
};

static int __init soc_sound_snd_platform_probe(struct platform_device *pdev)
{
    struct mt_soundaudio_data *priv;
    int ret = -EINVAL;

    priv = kzalloc(sizeof(struct mt_soundaudio_data), GFP_KERNEL);
    if(priv == NULL){
        return -ENOMEM;
    }

    platform_set_drvdata(pdev, priv);

    //init priv data struct
    mutex_init(&priv->mutex);
    memset(&priv->hwparam, 0, sizeof(struct mt_snd_sound_hwparams));

    ret = devm_snd_soc_register_component(&pdev->dev, &mt_soc_sound_platform_drv, NULL, 0);
    if (ret < 0)
        goto err;

    return ret;
err:
    kfree(priv);
    return ret;
}

static int __exit soc_sound_snd_platform_remove(struct platform_device *pdev)
{
    struct mt_soundaudio_data *priv = dev_get_drvdata(&pdev->dev);

    if(priv)
        kfree(priv);

    return 0;
}

static struct platform_driver audio_sound_dma_driver = {
    .driver = {
        .name = "mt-sound-dma",
        .owner = THIS_MODULE,
    },
    .probe = soc_sound_snd_platform_probe,
    .remove = __exit_p(soc_sound_snd_platform_remove),
};

static struct platform_device *audio_sound_dma_device;

int mt_snd_dma_init(void)
{
    int ret =0;
    
    audio_sound_dma_device = platform_device_alloc("mt-sound-dma", -1);
    if (!audio_sound_dma_device) {
    	pr_err("mt-audio-dma SoC platform device: Unable to register\n");
    	return -ENOMEM;
    }
    
    ret = platform_device_add(audio_sound_dma_device);
    if (ret) {
    	pr_err("mt-audio-dma SoC platform device: Unable to add\n");
    	platform_device_put(audio_sound_dma_device);
        return ret;
    }

    return platform_driver_register(&audio_sound_dma_driver);
}

void mt_snd_dma_deinit(void)
{
    platform_device_unregister(audio_sound_dma_device);
    platform_driver_unregister(&audio_sound_dma_driver);
}
