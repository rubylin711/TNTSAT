/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2017 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"
#include "hw_ce_common.h"
#include "hw_ce_register.h"
#include "hw_ce_if.h"
#include <linux/ktime.h>

mt_s32 hw_ce_ades_config(HW_CE_ADES_CTRL_S *hw_ctrl)
{
//16:0		Key Usage		[0]:	can be used on AES
//							[1]:	can be used on DES
//							[2]:	can be used on TDES
//							[3]:	can be used on CSAv2
//							[4]:	can be used on CSAv3
//							[5]:	can be used on SM2/3/4
//							[6]:	can be used on HMAC
//							[7]:	can be used on M2M(PVR)
//							[8]:	can be used on ASA
//							[9]:	can be used on Multi2
//							[10]:reserved
//							[11]:reserved
//							[12]:reserved
//							[13]:reserved
//							[14]:reserved
//							[15]:reserved
//							[16]:reserved
//17		M2M Key User		M2M perspective
//							0: can be used in REE
//							1: forbid using in REE
//19:18		Enc/Dec			[18]	0:decryption operation is not allowed
//								1:decryption operation is allowed
//							[19]	0:encryption operation is not allowed
//								1:encryption operation is allowed
//21:20		Key Size			size of the associated key:
//							00: 64-bit
//							01: 128-bit
//							10: 192-bit
//							11: 256-bit
//24:22		KeySource		000:HostCPU
//							001:SCPU
//							010:ALK
//							011:CW KL
//							100:PVR KL
//							Others:reserved
//26:25		TDES key check	00:no check
//							01:check Akey=Bkey with parity bits
//							10:check Akey=Bkey without parity bits
//							11:reserved
//31:27		Reserved
#if 0
	if (hw_ctrl->algo_sel == HW_CE_ADES_ALGO_SEL_AES)
	{
		hw_ce_kt_write_attribute(hw_ctrl->key_slot, 0x001C03ff);
		hw_ce_kt_write_key16(hw_ctrl->key_slot, hw_ctrl->key);
		hw_ce_kt_write_iv16(hw_ctrl->key_slot,  hw_ctrl->iv);
	}
	else if (hw_ctrl->algo_sel == HW_CE_ADES_ALGO_SEL_DES)
	{
		if (HW_CE_ADES_ALGO_MODE_DES_XXX_ENC == hw_ctrl->algo_mode || HW_CE_ADES_ALGO_MODE_DES_XXX_DEC == hw_ctrl->algo_mode)
		{
			hw_ce_kt_write_attribute(hw_ctrl->key_slot, 0x000C03ff);
			hw_ce_kt_write_key8(hw_ctrl->key_slot, hw_ctrl->key);
		}
		else
		{
			hw_ce_kt_write_attribute(hw_ctrl->key_slot, 0x001C03ff);
			hw_ce_kt_write_key16(hw_ctrl->key_slot, hw_ctrl->key);
		}
		hw_ce_kt_write_iv8(hw_ctrl->key_slot,  hw_ctrl->iv);
	}
	hw_ce_kt_slot_active(hw_ctrl->key_slot, 1);
#endif
    HW_CE_ALGO_MODE_REG mode_reg;
    HW_CE_TS_PID_FILT_CFG_REG pid_cfg_reg;
    HW_CE_TS_PID01_FILT_REG pid01_num_reg;
    HW_CE_TS_PID23_FILT_REG pid23_num_reg;
    HW_CE_TS_PID45_FILT_REG pid45_num_reg;
    HW_CE_TS_PID67_FILT_REG pid67_num_reg;
    HW_CE_MISC_CFG_REG misc_cfg_reg;

    HAL_PUT_U32((volatile u32 *)(ALGORITHM_EVEN_KEY_ADDR(hw_ctrl->chan_num)), hw_ctrl->even_key_slot);
    HAL_PUT_U32((volatile u32 *)(ALGORITHM_ODD_KEY_ADDR(hw_ctrl->chan_num)), hw_ctrl->odd_key_slot);

    mode_reg.all = HAL_GET_U32((volatile u32 *)(ALGORITHMn_MODE(hw_ctrl->chan_num)));
    mode_reg.bitc.ADES_MODE = hw_ctrl->work_mode;
    mode_reg.bitc.ALGORITHM_MODE = hw_ctrl->algo_mode;
    mode_reg.bitc.ALGORITHM_SEL = hw_ctrl->algo_sel;

    mode_reg.bitc.TSPARSE_DEC = hw_ctrl->ts_enc_dec;
    mode_reg.bitc.TSPARSE_KEY_SEL = hw_ctrl->ts_enc_ksel;
    mode_reg.bitc.TSPARSE_IND = hw_ctrl->ts_dec_ind;
    mode_reg.bitc.TSPARSE_LEN = hw_ctrl->ts_pkt_len;
    mode_reg.bitc.TSPARSE_EN = hw_ctrl->ts_on_off;

    //printk("hw:ts_onoff:%d,pkt_len:%d, ts_enc:%d\n", 
    //hw_ctrl->ts_on_off, hw_ctrl->ts_pkt_len, hw_ctrl->ts_enc_dec);

    mode_reg.bitc.TSPARSE_IVE_CAL_EN = hw_ctrl->ts_ive_cal_en;
    mode_reg.bitc.TSPARSE_IVE_MODE = hw_ctrl->ts_ive_mode;
    mode_reg.bitc.TSPARSE_CTS_MODE = hw_ctrl->ts_cts_mode;
    mode_reg.bitc.TSPARSE_SHORT_MODE = hw_ctrl->ts_short_mode;
    mode_reg.bitc.TSPARSE_SMALL_MODE = hw_ctrl->ts_small_mode;

    HAL_PUT_U32((volatile u32 *)(ALGORITHMn_MODE(hw_ctrl->chan_num)), mode_reg.all);

    pid_cfg_reg.all = HAL_GET_U32((volatile u32 *)(PID_FILT_CFG));
    pid_cfg_reg.bitc.PID0_FILT_EN = hw_ctrl->ts_pid0_filt_en;
    pid_cfg_reg.bitc.PID1_FILT_EN = hw_ctrl->ts_pid1_filt_en;
    pid_cfg_reg.bitc.PID2_FILT_EN = hw_ctrl->ts_pid2_filt_en;
    pid_cfg_reg.bitc.PID3_FILT_EN = hw_ctrl->ts_pid3_filt_en;
    pid_cfg_reg.bitc.PID4_FILT_EN = hw_ctrl->ts_pid4_filt_en;
    pid_cfg_reg.bitc.PID5_FILT_EN = hw_ctrl->ts_pid5_filt_en;
    pid_cfg_reg.bitc.PID6_FILT_EN = hw_ctrl->ts_pid6_filt_en;
    pid_cfg_reg.bitc.PID7_FILT_EN = hw_ctrl->ts_pid7_filt_en;
    pid_cfg_reg.bitc.PID_FILT_CH = hw_ctrl->chan_num;
    HAL_PUT_U32((volatile u32 *)(PID_FILT_CFG), pid_cfg_reg.all);

    pid01_num_reg.all = HAL_GET_U32((volatile u32 *)(PID01_FILT));
    pid23_num_reg.all = HAL_GET_U32((volatile u32 *)(PID23_FILT));
    pid45_num_reg.all = HAL_GET_U32((volatile u32 *)(PID45_FILT));
    pid67_num_reg.all = HAL_GET_U32((volatile u32 *)(PID67_FILT));
    pid01_num_reg.bitc.PID0_FILT = hw_ctrl->ts_pid0_filt_num;
    pid01_num_reg.bitc.PID1_FILT = hw_ctrl->ts_pid1_filt_num;
    pid23_num_reg.bitc.PID2_FILT = hw_ctrl->ts_pid2_filt_num;
    pid23_num_reg.bitc.PID3_FILT = hw_ctrl->ts_pid3_filt_num;
    pid45_num_reg.bitc.PID4_FILT = hw_ctrl->ts_pid4_filt_num;
    pid45_num_reg.bitc.PID5_FILT = hw_ctrl->ts_pid5_filt_num;
    pid67_num_reg.bitc.PID6_FILT = hw_ctrl->ts_pid6_filt_num;
    pid67_num_reg.bitc.PID7_FILT = hw_ctrl->ts_pid7_filt_num;
    HAL_PUT_U32((volatile u32 *)(PID01_FILT), pid01_num_reg.all);
    HAL_PUT_U32((volatile u32 *)(PID23_FILT), pid23_num_reg.all);
    HAL_PUT_U32((volatile u32 *)(PID45_FILT), pid45_num_reg.all);
    HAL_PUT_U32((volatile u32 *)(PID67_FILT), pid67_num_reg.all);

    misc_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_MISC_CFG));
    misc_cfg_reg.bitc.FORCE_ENC_MODE = (hw_ctrl->ts_force_enc_en) << (hw_ctrl->chan_num);
    HAL_PUT_U32((volatile u32 *)(CRYPTO_MISC_CFG), misc_cfg_reg.all);

    return CE_SUCCESS;
}

mt_s32 hw_ce_ades_process(HW_CE_ADES_CTRL_S *hw_ctrl, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length)
{
    mt_u32 i, timeout, wordlength, sumlength = 0, bytelength = length, trimlength = length & 0x3;
    mt_u32 *input_data = (mt_u32 *)p_src_addr;
    mt_u32 *output_data = (mt_u32 *)p_dst_addr;
    mt_u32 input_addr = (mt_u32)p_src_addr;
    mt_u32 output_addr = (mt_u32)p_dst_addr;
    mt_u32 ce_fifo_depth = (HW_CE_TS_SWITCH_OFF == hw_ctrl->ts_on_off) ? HW_CE_FIFO_DEPTH : ((HW_CE_TS_PKT_LEN_188BYTE == hw_ctrl->ts_pkt_len) ? HW_TS_PKT188_FIFO_DEPTH : HW_TS_PKT192_FIFO_DEPTH);
    HW_CE_CHANNEL_GRP_REG grp_mod_reg;
    HW_CE_CHANNEL_CFG_REG chan_cfg_reg;

    if (trimlength) {
        if (HW_CE_ADES_WORK_MODE_ECB == hw_ctrl->work_mode || HW_CE_ADES_WORK_MODE_CBC == hw_ctrl->work_mode)
            return CE_ADES_CPU_LENGTH_MISMATCH;
    }

    grp_mod_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_GRP(hw_ctrl->chan_num)));
    chan_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)));

    HAL_PUT_U32((volatile u32 *)(ALGORITHMn_LEN(hw_ctrl->chan_num)), length);
    grp_mod_reg.bitc.GRP_MODE = hw_ctrl->grp_mode;
    HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_GRP(hw_ctrl->chan_num)), grp_mod_reg.all);

    if ((HW_CE_PROC_CPU == hw_ctrl->proc_mode) || ((HW_CE_PROC_AUTO == hw_ctrl->proc_mode) && (CHK_IF_CPU_MODE_ADES))) {
        /* ades cpu mode */
        if ((input_addr & 0x00000003) || (output_addr & 0x00000003)) {
            printk("ades at cpu mode: input or output address is not 4bytes multiple\n");
            return CE_ADES_CPU_ADDR_ERROR;
        }

        //HW_CE_BUS_MODE_REG bus_mode_reg;
        //bus_mode_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE));
        //bus_mode_reg.bitc.SW_SRC_ENDIAN_CFG &= (~(1<<chan_num));
        //bus_mode_reg.bitc.SW_DST_ENDIAN_CFG |= (1<<chan_num);
        //HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE), bus_mode_reg.all);
        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE), 0x0f0);
        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);

        chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 1;
        chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 1;
        chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);

        while (length > 0) {
            if (HW_CE_TS_SWITCH_OFF == hw_ctrl->ts_on_off) {
                if (length > 160) {
                    ce_fifo_depth = HW_CE_FIFO_DEPTH;
                } else {
                    ce_fifo_depth = 160;
                }
            }

            if (length > ce_fifo_depth) {
                wordlength = ce_fifo_depth >> 2;
                length -= ce_fifo_depth;
            } else {
                wordlength = length >> 2;
                length = 0;
            }

            for (i = 0; i < wordlength; i++) {
                //timeout = 0;
                //mt_u32 temp = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num)));
                //while (temp < 4)
                ktime_t ktimeout = ktime_add_ns(ktime_get(), 500000);
                while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num))) < 4) {
                    /*
                       hw_ce_msdelay_customize(1);
                       timeout++;
                       if (timeout > 500) {*/
                    if (ktime_compare (ktime_get(), ktimeout) > 0) {
                        printk("====CE_ADES_CPU_WRITE_TIMEOUT====\n");
                        chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
                        chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
                        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
                        return CE_ADES_CPU_WRITE_TIMEOUT;
                    }
                }

                HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), input_data[sumlength + i]);
            }

            if (0 == length && 0 != trimlength) {
                mt_u32 lastword;

                //timeout = 0;
                ktime_t ktimeout = ktime_add_ns(ktime_get(), 500000);
                while (trimlength != HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num)))) {
                    /*
                       hw_ce_msdelay_customize(1);
                       timeout++;
                       if (timeout > 500) {*/
                    if (ktime_compare (ktime_get(), ktimeout) > 0) {
                        printk("====CE_ADES_CPU_WRITE_TRIM_TIMEOUT====\n");
                        chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
                        chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
                        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
                        return CE_ADES_CPU_WRITE_LENGTH_ERROR;
                    }
                }

                lastword = 0;
                for (i = 0; i < trimlength; i++) {
                    lastword += p_src_addr[bytelength - trimlength + i] << (8 * i);
                }
                HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), lastword);
            }

            for (i = 0; i < wordlength; i++) {
                //timeout = 0;
                //mt_u32 temp = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num)));
                //while (temp < 4)
                ktime_t ktimeout = ktime_add_ns(ktime_get(), 500000);
                while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num))) < 4) {
                    /*
                       hw_ce_msdelay_customize(1);
                       timeout++;
                       if (timeout > 500) {*/
                    if (ktime_compare (ktime_get(), ktimeout) > 0) {
                        printk("====CE_ADES_CPU_READ_TIMEOUT====\n");
                        chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
                        chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
                        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
                        return CE_ADES_CPU_READ_TIMEOUT;
                    }
                }

                output_data[sumlength + i] = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
            }

            if (0 == length && 0 != trimlength) {
                mt_u32 lastword;

                //timeout = 0;
                ktime_t ktimeout = ktime_add_ns(ktime_get(), 500000);
                while (trimlength != HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num)))) {
                    /*
                       hw_ce_msdelay_customize(1);
                       timeout++;
                       if (timeout > 500) {*/
                    if (ktime_compare (ktime_get(), ktimeout) > 0) {
                        printk("====CE_ADES_CPU_READ_TRIM_TIMEOUT====\n");
                        chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
                        chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
                        /* close operation */
                        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); 
                        return CE_ADES_CPU_READ_LENGTH_ERROR;
                    }
                }

                lastword = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
                for (i = 0; i < trimlength; i++) {
                    p_dst_addr[bytelength - trimlength + i] = (mt_u8)(lastword >> (i * 8));
                }
            }
            sumlength += wordlength;
        } //end-of while (length > 0)
    } else {
        /* ades dma mode *****************************************************************/
        if ((input_addr & 0x00000007) || (output_addr & 0x00000007)) {
            printk("ades at dma mode: input or output address is not 8bytes aligned\n");
            return CE_ADES_DMA_ADDR_ERROR;
        }

        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_SRC(hw_ctrl->chan_num)), (input_addr & 0x1FFFFFFF) >> 3);
        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_DST(hw_ctrl->chan_num)), (output_addr & 0x1FFFFFFF) >> 3);
        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);
        chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 1;
        chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 0;
        chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
        HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);
        timeout = 0;
		//hw_ce_msdelay_customize(10);
		usleep_range(1000, 10000);
        while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_STATE(hw_ctrl->chan_num))) & 0x00000001) {
			//hw_ce_msdelay_customize(1);
			usleep_range(100, 1000);
            timeout++;
            if (timeout > 6000000 /*600000000*/) {
                printk("====CE_ADES_DMA_CALC_TIMEOUT====\n");
                chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
                chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
                HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
                return CE_ADES_DMA_READ_TIMEOUT;
            }
        }
    }
    chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
    chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
    HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
    return CE_SUCCESS;
}
#if 1
mt_s32 hw_ce_ades_process_start(HW_CE_ADES_CTRL_S *hw_ctrl, mt_u8 *p_src_addr, mt_u8 *p_dst_addr, mt_u32 length)
{
    mt_u32 i, timeout, wordlength, sumlength = 0, bytelength = length, trimlength = length & 0x3;
    mt_u32 *input_data = (mt_u32 *)p_src_addr;
    mt_u32 *output_data = (mt_u32 *)p_dst_addr;
    mt_u32 input_addr = (mt_u32)p_src_addr;
    mt_u32 output_addr = (mt_u32)p_dst_addr;
    mt_u32 ce_fifo_depth = (HW_CE_TS_SWITCH_OFF == hw_ctrl->ts_on_off) ? HW_CE_FIFO_DEPTH : ((HW_CE_TS_PKT_LEN_188BYTE == hw_ctrl->ts_pkt_len) ? HW_TS_PKT188_FIFO_DEPTH : HW_TS_PKT192_FIFO_DEPTH);
    HW_CE_CHANNEL_GRP_REG grp_mod_reg;
    HW_CE_CHANNEL_CFG_REG chan_cfg_reg;

    if (trimlength) {
	if (HW_CE_ADES_WORK_MODE_ECB == hw_ctrl->work_mode || HW_CE_ADES_WORK_MODE_CBC == hw_ctrl->work_mode)
	    return CE_ADES_CPU_LENGTH_MISMATCH;
    }

    grp_mod_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_GRP(hw_ctrl->chan_num)));
    chan_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)));

    //printk("chan_num=%d\n",chan_num);
    //printk("input_addr=0x%x\n",input_addr);
    //printk("output_addr=0x%x\n",output_addr);
    //printk("length=%d\n",length);
    //printk("chan_cfg_reg.all=0x%x\n",chan_cfg_reg.all);

    HAL_PUT_U32((volatile u32 *)(ALGORITHMn_LEN(hw_ctrl->chan_num)), length);
    //printk("addr(0x%x) = 0x%x\n", ALGORITHMn_LEN(hw_ctrl->chan_num), HAL_GET_U32((volatile u32 *)(ALGORITHMn_LEN(hw_ctrl->chan_num))));
    grp_mod_reg.bitc.GRP_MODE = hw_ctrl->grp_mode;
    HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_GRP(hw_ctrl->chan_num)), grp_mod_reg.all);
    //printk("addr(0x%x) = 0x%x\n", CRYPTO_DMA_CHn_GRP(hw_ctrl->chan_num), HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_GRP(hw_ctrl->chan_num))));
    if ((HW_CE_PROC_CPU == hw_ctrl->proc_mode) || ((HW_CE_PROC_AUTO == hw_ctrl->proc_mode) && (CHK_IF_CPU_MODE_ADES))) {
	//printk("ades cpu mode\n");
	if ((input_addr & 0x00000003) || (output_addr & 0x00000003)) {
	    printk("ades at cpu mode: input or output address is not 4bytes multiple\n");
	    return CE_ADES_CPU_ADDR_ERROR;
	}

	//HW_CE_BUS_MODE_REG bus_mode_reg;
	//bus_mode_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE));
	//bus_mode_reg.bitc.SW_SRC_ENDIAN_CFG &= (~(1<<chan_num));
	//bus_mode_reg.bitc.SW_DST_ENDIAN_CFG |= (1<<chan_num);
	//HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE), bus_mode_reg.all);
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE), 0x0f0);
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);

	chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 1;
	chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 1;
	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);

	while (length > 0) {
	    if (HW_CE_TS_SWITCH_OFF == hw_ctrl->ts_on_off) {
		if (length > 160) {
		    ce_fifo_depth = HW_CE_FIFO_DEPTH;
		} else {
		    ce_fifo_depth = 160;
		}
	    }

	    if (length > ce_fifo_depth) {
		wordlength = ce_fifo_depth >> 2;
		length -= ce_fifo_depth;
	    } else {
		wordlength = length >> 2;
		length = 0;
	    }

	    for (i = 0; i < wordlength; i++) {
		timeout = 0;
		//mt_u32 temp = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num)));
		//printk("CRYPTO_DMA_CHn_IBUF %d = %d\n", i, temp);
		//while (temp < 4)
		while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num))) < 4) {
		    hw_ce_msdelay_customize(1);
		    timeout++;
		    if (timeout > 500) {
			printk("====CE_ADES_CPU_WRITE_TIMEOUT====\n");
			chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
			return CE_ADES_CPU_WRITE_TIMEOUT;
		    }
		}

		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), input_data[sumlength + i]);
	    }

	    if (0 == length && 0 != trimlength) {
		mt_u32 lastword;

		timeout = 0;
		while (trimlength != HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num)))) {
		    hw_ce_msdelay_customize(1);
		    timeout++;
		    if (timeout > 500) {
			printk("====CE_ADES_CPU_WRITE_TRIM_TIMEOUT====\n");
			chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
			return CE_ADES_CPU_WRITE_LENGTH_ERROR;
		    }
		}

		lastword = 0;
		for (i = 0; i < trimlength; i++) {
		    lastword += p_src_addr[bytelength - trimlength + i] << (8 * i);
		}
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), lastword);

		//if (trimlength == HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num))))
		//{
		//	mt_u32 lastword = 0;
		//	for (i=0; i<trimlength; i++)
		//	{
		//		lastword += p_src_addr[bytelength-trimlength+i]<<(8*i);
		//	}
		//	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), lastword);
		//}
		//else
		//{
		//	return CE_ADES_CPU_WRITE_LENGTH_ERROR;
		//}
	    }

	    for (i = 0; i < wordlength; i++) {
		timeout = 0;
		//mt_u32 temp = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num)));
		//printk("CRYPTO_DMA_CHn_OBUF %d = %d\n", i, temp);
		//while (temp < 4)
		while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num))) < 4) {
		    hw_ce_msdelay_customize(1);
		    timeout++;
		    if (timeout > 500) {
			printk("====CE_ADES_CPU_READ_TIMEOUT====\n");
			chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
			return CE_ADES_CPU_READ_TIMEOUT;
		    }
		}

		output_data[sumlength + i] = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
	    }

	    if (0 == length && 0 != trimlength) {
		mt_u32 lastword;

		timeout = 0;
		while (trimlength != HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num)))) {
		    hw_ce_msdelay_customize(1);
		    timeout++;
		    if (timeout > 500) {
			printk("====CE_ADES_CPU_READ_TRIM_TIMEOUT====\n");
			chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
			return CE_ADES_CPU_READ_LENGTH_ERROR;
		    }
		}

		lastword = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
		for (i = 0; i < trimlength; i++) {
		    p_dst_addr[bytelength - trimlength + i] = (mt_u8)(lastword >> (i * 8));
		}

		//if (trimlength == HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num))))
		//{
		//	mt_u32 lastword = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
		//	for (i=0; i<trimlength; i++)
		//	{
		//		p_dst_addr[bytelength-trimlength+i] = (mt_u8 )(lastword>>(i*8));
		//	}
		//}
		//else
		//{
		//	return CE_ADES_CPU_READ_LENGTH_ERROR;
		//}
	    }

	    sumlength += wordlength;
	}

	chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
    } else {
	//printk("ades dma mode\n");
	if ((input_addr & 0x00000007) || (output_addr & 0x00000007)) {
	    printk("ades at dma mode: input or output address is not 8bytes multiple\n");
	    return CE_ADES_DMA_ADDR_ERROR;
	}

	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_SRC(hw_ctrl->chan_num)), (input_addr & 0x1FFFFFFF) >> 3);
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_DST(hw_ctrl->chan_num)), (output_addr & 0x1FFFFFFF) >> 3);
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);
	//printk("addr(0x%x) = 0x%x\n", CRYPTO_DMA_CHn_SRC(hw_ctrl->chan_num), HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_SRC(hw_ctrl->chan_num))));
	//printk("addr(0x%x) = 0x%x\n", CRYPTO_DMA_CHn_DST(hw_ctrl->chan_num), HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_DST(hw_ctrl->chan_num))));
	//printk("addr(0x%x) = 0x%x\n", CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num), HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num))));
	chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 1;
	chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 0;
	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);
	//printk("addr(0x%x) = 0x%x\n", CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num), HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num))));
    }

    return CE_SUCCESS;
}

mt_s32 hw_ce_ades_process_polling(HW_CE_ADES_CTRL_S *hw_ctrl, mt_u32 time_out)
{
    mt_u32 timeout = 0;

	//hw_ce_msdelay_customize(10);
	usleep_range(1000, 10000);
    while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_STATE(hw_ctrl->chan_num))) & 0x00000001) {
		//hw_ce_msdelay_customize(1);
		usleep_range(100, 1000);
		timeout++;
		if (timeout > time_out) {
			//printk("====ch%d CE_ADES_DMA_STILL BUSY NOW====\n", hw_ctrl->chan_num);
			return CE_ADES_DMA_READ_TIMEOUT;
		}
	}

    return CE_SUCCESS;
}

mt_s32 hw_ce_ades_process_stop(HW_CE_ADES_CTRL_S *hw_ctrl)
{
    HW_CE_CHANNEL_CFG_REG chan_cfg_reg;

    chan_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)));

    if (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_STATE(hw_ctrl->chan_num))) & 0x00000001) {
	//printk("====CE_ADES DMA STILL BUSY NOW====\n");
	//printk("====SO DO CANCLE====\n");
	chan_cfg_reg.bitc.DMA_CHn_CANCEL = 1;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //cancle operation

	hw_ce_msdelay_customize(1);
	while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_STATE(hw_ctrl->chan_num))) & 0x00000001) {
	    printk("-");
	}
	printk("\n");

	//if (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_STATE(hw_ctrl->chan_num))) & 0x00000001)
	//{
	//	printk("====ch%d CANCLE OPERATION FAILED====\n", hw_ctrl->chan_num);
	//}
	//else
	//{
	//	printk("====ch%d CANCLE OPERATION SUCCESSFUL====\n", hw_ctrl->chan_num);

	chan_cfg_reg.bitc.DMA_CHn_CANCEL = 0;
	chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation

	//}
    } else {
	//printk("====CE_ADES DMA ALREADY IDLE NOW====\n");
	//printk("====SO NOT NEED DO CANCLE====\n");
	chan_cfg_reg.bitc.DMA_CHn_CANCEL = 0;
	chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
    }

    return CE_SUCCESS;
}
#endif

s32 hw_ce_ades_process_lastblock(HW_CE_ADES_CTRL_S *hw_ctrl, u8 *p_src_addr, u8 *p_dst_addr, u32 length)
{
    u32 i, timeout, wordlength, sumlength = 0, bytelength = length, trimlength = length & 0x3;
    u32 *input_data = (u32 *)p_src_addr;
    u32 *output_data = (u32 *)p_dst_addr;
    u32 input_addr = (u32)p_src_addr;
    u32 output_addr = (u32)p_dst_addr;
    u32 ce_fifo_depth = (HW_CE_TS_SWITCH_OFF == hw_ctrl->ts_on_off) ? HW_CE_FIFO_DEPTH : ((HW_CE_TS_PKT_LEN_188BYTE == hw_ctrl->ts_pkt_len) ? HW_TS_PKT188_FIFO_DEPTH : HW_TS_PKT192_FIFO_DEPTH);
    HW_CE_CHANNEL_GRP_REG grp_mod_reg;
    HW_CE_CHANNEL_CFG_REG chan_cfg_reg;

    if (trimlength) {
	if (HW_CE_ADES_WORK_MODE_ECB == hw_ctrl->work_mode || HW_CE_ADES_WORK_MODE_CBC == hw_ctrl->work_mode)
	    return CE_ADES_CPU_LENGTH_MISMATCH;
    }

    grp_mod_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_GRP(hw_ctrl->chan_num)));
    chan_cfg_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)));

    HAL_PUT_U32((volatile u32 *)(ALGORITHMn_LEN(hw_ctrl->chan_num)), length);
    grp_mod_reg.bitc.GRP_MODE = hw_ctrl->grp_mode;
    HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_GRP(hw_ctrl->chan_num)), grp_mod_reg.all);
    if ((HW_CE_PROC_CPU == hw_ctrl->proc_mode) || ((HW_CE_PROC_AUTO == hw_ctrl->proc_mode) && (CHK_IF_CPU_MODE_ADES))) {
	if ((input_addr & 0x00000003) || (output_addr & 0x00000003)) {
	    printk("ades at cpu mode: input or output address is not 4bytes multiple\n");
	    return CE_ADES_CPU_ADDR_ERROR;
	}

	//HW_CE_BUS_MODE_REG bus_mode_reg;
	//bus_mode_reg.all = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE));
	//bus_mode_reg.bitc.SW_SRC_ENDIAN_CFG &= (~(1<<chan_num));
	//bus_mode_reg.bitc.SW_DST_ENDIAN_CFG |= (1<<chan_num);
	//HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE), bus_mode_reg.all);
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_BUS_MODE), 0x0f0);
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);

	chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 1;
	chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 1;
	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);

	while (length > 0) {
	    if (HW_CE_TS_SWITCH_OFF == hw_ctrl->ts_on_off) {
		if (length > 160) {
		    ce_fifo_depth = HW_CE_FIFO_DEPTH;
		} else {
		    ce_fifo_depth = 160;
		}
	    }

	    if (length > ce_fifo_depth) {
		wordlength = ce_fifo_depth >> 2;
		length -= ce_fifo_depth;
	    } else {
		wordlength = length >> 2;
		length = 0;
	    }

	    for (i = 0; i < wordlength; i++) {
		timeout = 0;
		//u32 temp = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num)));
		//while (temp < 4)
		while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num))) < 4) {
		    hw_ce_msdelay_customize(1);
		    timeout++;
		    if (timeout > 500) {
			printk("====CE_ADES_CPU_WRITE_TIMEOUT====\n");
			chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
			return CE_ADES_CPU_WRITE_TIMEOUT;
		    }
		}

		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), input_data[sumlength + i]);
	    }

	    if (0 == length && 0 != trimlength) {
		u32 lastword;

		timeout = 0;
		while (trimlength != HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num)))) {
		    hw_ce_msdelay_customize(1);
		    timeout++;
		    if (timeout > 500) {
			printk("====CE_ADES_CPU_WRITE_TRIM_TIMEOUT====\n");
			chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
			return CE_ADES_CPU_WRITE_LENGTH_ERROR;
		    }
		}

		lastword = 0;
		for (i = 0; i < trimlength; i++) {
		    lastword += p_src_addr[bytelength - trimlength + i] << (8 * i);
		}
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), lastword);

		//if (trimlength == HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_IBUF(hw_ctrl->chan_num))))
		//{
		//	u32 lastword = 0;
		//	for (i=0; i<trimlength; i++)
		//	{
		//		lastword += p_src_addr[bytelength-trimlength+i]<<(8*i);
		//	}
		//	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_WSLV(hw_ctrl->chan_num)), lastword);
		//}
		//else
		//{
		//	return CE_ADES_CPU_WRITE_LENGTH_ERROR;
		//}
	    }

	    for (i = 0; i < wordlength; i++) {
		timeout = 0;
		//u32 temp = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num)));
		//while (temp < 4)
		while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num))) < 4) {
		    hw_ce_msdelay_customize(1);
		    timeout++;
		    if (timeout > 500) {
			printk("====CE_ADES_CPU_READ_TIMEOUT====\n");
			chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
			return CE_ADES_CPU_READ_TIMEOUT;
		    }
		}

#if 0
				output_data[sumlength + i] = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
#else
		if (((bytelength >> 2) - (sumlength + i)) <= 4) {
		    output_data[4 - ((bytelength >> 2) - (sumlength + i))] = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
		} else {
		    output_data[0] = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
		    ;
		}
#endif
	    }

	    if (0 == length && 0 != trimlength) {
		u32 lastword;

		timeout = 0;
		while (trimlength != HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num)))) {
		    hw_ce_msdelay_customize(1);
		    timeout++;
		    if (timeout > 500) {
			printk("====CE_ADES_CPU_READ_TRIM_TIMEOUT====\n");
			chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
			chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
			HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
			return CE_ADES_CPU_READ_LENGTH_ERROR;
		    }
		}

		lastword = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
		for (i = 0; i < trimlength; i++) {
		    p_dst_addr[bytelength - trimlength + i] = (u8)(lastword >> (i * 8));
		}

		//if (trimlength == HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_OBUF(hw_ctrl->chan_num))))
		//{
		//	u32 lastword = HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_RSLV(hw_ctrl->chan_num)));
		//	for (i=0; i<trimlength; i++)
		//	{
		//		p_dst_addr[bytelength-trimlength+i] = (u8 )(lastword>>(i*8));
		//	}
		//}
		//else
		//{
		//	return CE_ADES_CPU_READ_LENGTH_ERROR;
		//}
	    }

	    sumlength += wordlength;
	}
    } else {
	if ((input_addr & 0x00000007) || (output_addr & 0x00000007)) {
	    printk("ades at dma mode: input or output address is not 8bytes multiple\n");
	    return CE_ADES_DMA_ADDR_ERROR;
	}

	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_SRC(hw_ctrl->chan_num)), (input_addr & 0x1FFFFFFF) >> 3);
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_DST(hw_ctrl->chan_num)), (output_addr & 0x1FFFFFFF) >> 3);
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_LEN(hw_ctrl->chan_num)), length);
	chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 1;
	chan_cfg_reg.bitc.DMA_CHn_MODE_CFG = 0;
	chan_cfg_reg.bitc.DMA_CHn_ENABLE = 1;
	HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all);
	timeout = 0;
	while (HAL_GET_U32((volatile u32 *)(CRYPTO_DMA_CHn_STATE(hw_ctrl->chan_num))) & 0x00000001) {
	    hw_ce_msdelay_customize(1);
	    timeout++;
	    if (timeout > 600000000 /*600000000*/) {
		printk("====CE_ADES_DMA_CALC_TIMEOUT====\n");
		chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
		chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
		HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
		return CE_ADES_DMA_READ_TIMEOUT;
	    }
	}
    }
    chan_cfg_reg.bitc.DMA_CHn_OUT_EN = 0;
    chan_cfg_reg.bitc.DMA_CHn_ENABLE = 0;
    HAL_PUT_U32((volatile u32 *)(CRYPTO_DMA_CHn_CFG(hw_ctrl->chan_num)), chan_cfg_reg.all); //close operation
    return CE_SUCCESS;
}
