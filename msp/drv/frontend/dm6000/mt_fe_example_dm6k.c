/********************************************************************************************/
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/* Montage Proprietary and Confidential                                                     */
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/********************************************************************************************/
/****************************************************************************
* MONTAGE PROPRIETARY AND CONFIDENTIAL
* Montage Technology (Shanghai) Inc.
* All Rights Reserved
* --------------------------------------------------------------------------
*
* File:				mt_fe_example_dm6k.c
*
* Current version:	0.10.00
*
* Description:		M88DM6000 IC Driver.
*
* Log:	Description							Version		Date			Author
*		---------------------------------------------------------------------
*		Create						0.00.10		2014.02.15		angbingju
*		Modify						0.01.00		2014.05.18		Wangbingju
*		Modify						0.10.00		2014.08.08		Wangbingju
****************************************************************************/
#define QUEUE_ID 0x8888

#define OS_QUEUE	unsigned long

typedef enum _MT_FE_OS_STATE
{
	OS_TIMEOUT,
	OS_MESSAGE_OK
}MT_FE_OS_STATE;


typedef enum _MT_FE_STATE
{
	MT_FE_STATE_IDLE
	,MT_FE_STATE_CONNETING
	,MT_FE_STATE_CONNETED
	,MT_FE_STATE_NO_SIGNAL
}MT_FE_STATE;


typedef enum _MT_FE_CMD
{
	MT_FE_CMD_CONNECT
	,MT_FE_CMD_GET_CHAN_INFO
	,MT_FE_CMD_GET_SIGNAL_INFO
	,MT_FE_CMD_GET_LOCK_STATE
	,MT_FE_CMD_GET_CHAN_INFO_CODE
}MT_FE_CMD;

U32	mt_fe_time_now(void)
{
	U32 time_now_ms = 0;

	/*TODO:
		Call system API to get current time.
		For example: In VC++, the function is clock();
		time_now_ms = clock();
	*/

	return time_now_ms;
}



void os_get_mssage(OS_QUEUE queue_id, OS_MESSAGE message, U32 time_out);

void main(void)
{
	MT_FE_CMD	message;
	MT_FE_STATE			state;
	MT_FE_LOCK_STATE *lock_state;
	MT_FE_TYPE						demod_set_type = MtFeType_Undef;
	S32			ret;
	U32			cnt;
	U32			freq;
	U32 			sym;
	U16 			qam;
	U8 			inverted;
	S32 			ret ;
	U32			chan_info_code;
	U8			 *p_quality,*p_strength;

	MT_FE_DM6K_Device_Handle dm6000_handle ;//定义DM6000结构体句柄
	MT_FE_DM6K_DEVICE_SETTINGS dm6k_setting;
	memset(&dm6k_setting,0,sizeof(MT_FE_DM6K_DEVICE_SETTINGS));//初始化变量
	dm6000_handle = &dm6k_setting;// DM6000结构体句柄赋值

	dm6k_setting.sys_dev_addr = 0x18;//指定系统地址，根据管脚选择确定
	dm6k_setting. sys_dev_xtal= MtFeXTALMode_27M;//设置系统的晶振，请与硬件工程师确认
	mt_fe_system_init_dm6k(dm6000_handle);//初始化DM6000系统

	dm6000_handle->m_device_ctt2.tuner_cfg.tuner_dev_addr == 0xc6;//设置tuner地址
	mt_fe_dmd_select_tuner_ctt2_dm6k(dm6000_handle, MtFeTN_TC6800);//选择tuner

	//以下设置DVBC/T/T2部分
	dm6k_setting.m_device_ctt2.ts_out_mode = MtFeTsOutMode_Serial;//设置DVBC/T/T2 的TS流接口模式
	dm6k_setting.m_device_ctt2.m_iSerialTSNo = 1;//如果是串行TS输出，还要选择端口号，TS设置请与硬件工程师确认
	mt_fe_dmd_select_tuner_ctt2_dm6k(dm6000_handle, MtFeTN_MxL603);//选择DVBC/T/T2部分使用的调谐器
	mt_fe_dmd_open_ctt2_dm6k(dm6000_handle,MtFeType_DVBT);//打开DVBT模块

//以下设置DVBS/S2部分，同上类似
#if MT_FE_DMD_DVBS_S2_SUPPORT//
	dm6k_setting.m_device_ss2.ts_out_mode = MtFeTsOutMode_Serial;
	dm6k_setting.m_device_ss2.m_iSerialTSNo = 2;
	mt_fe_dmd_select_tuner_ss2_dm6k(dm6000_handle, MtFeTn_TS2022);
	mt_fe_dmd_open_ss2_dm6k(dm6000_handle,MtFeType_DVBS);
#endif


	if(ret!=MtFeErr_Ok)
	{
		/*
			TODO:
				send message to APP, that demodulater download firmware error.
 		*/
	}

	while (1)
	{
		ret = os_get_mssage(QUEUE_ID, message, 100)	/*	timeout = 100ms	*/


		if (ret == OS_TIMEOUT)
		{
			switch (state)
			{
				case MT_FE_STATE_NO_SIGNAL:
				case MT_FE_STATE_CONNETING:
					cnt ++;

					if((demod_set_type == MtFeType_DVBC)||(demod_set_type == MtFeType_DVBT)||(demod_set_type == MtFeType_DVBT2))
						mt_fe_dmd_get_lock_state_ctt2_dm6k(dm6000_handle, &lock_state);
					if((demod_set_type == MtFeType_DVBS)||(demod_set_type == MtFeType_DVBS2))
						mt_fe_dmd_get_lock_state_ss2_dm6k(dm6000_handle, &lock_state);
					if (lock_state == MtFeLockState_Locked)
					{
						/*	change state	*/
						state = MT_FE_STATE_CONNETED;

						/*
							TODO:
								send message to APP, that demodulater has Locked !

 						*/
						cnt = 0;
						break;
					}

					if(mt_dd_handle->demod_cur_mode!=MtFeType_DVBC)
					{
						if(cnt=5)///400ms
						{
							mt_fe_dmd_get_signal_state_dd3k_t(&signal_state);
							if(signal_state==MtFeSignalState_NoSignal)
								{
									state = MT_FE_STATE_NO_SIGNAL;
									cnt   = 0;
								}

						}

						if (cnt > 20)
					 	{
							/*
								TODO:
									send message to APP, that demodulater Unlock !
							*/
							state = MT_FE_STATE_NO_SIGNAL;
							cnt            = 0;
						}
					}
					else
					{
						if (cnt > 8)
					 	{
							/*
								TODO:
									send message to APP, that demodulater Unlock !
							*/
							state = MT_FE_STATE_NO_SIGNAL;
							cnt            = 0;
						}
					}
					break;
				case MT_FE_STATE_CONNETED:
					cnt ++;

					mt_fe_dmd_get_lock_state_dd3k(mt_dd_handle,&lock_state);

					if (lock_state == MtFeLockState_Locked)
					{
						cnt = 0;
						break;
					}

					if (cnt > 40)
					{
						/*
							TODO:
								send message to APP, that no signal !
						*/
						/*	re-connect	*/
						mt_fe_dmd_soft_reset_dd3k(mt_dd_handle);

						state = MT_FE_STATE_CONNETING;
						cnt            = 0;
					}
					break;

			case MT_FE_STATE_IDLE:
				default:
					break;
			}

			continue;
		}


		switch(message)
		{
			case MT_FE_CMD_CONNECT:
				/*
					set "frequency and sym, qam, inverted, xtal "parameter to driver for connect one channel
					if signal is CTTB,"sym, qam, inverted, xta" not be used.

				*/
				if((demod_set_type == MtFeType_DVBC)||(demod_set_type == MtFeType_DVBT)||(demod_set_type == MtFeType_DVBT2))
				{
					dm6000_handle ->m_device_ctt2.demod_type = demod_set_type;//信号类型,DVBC/T/T2
					dm6000_handle ->m_device_ctt2.input_params.demod_bandwidth = MtFeBandwidth_8M;//带宽
					dm6000_handle ->m_device_ctt2.input_params.input_freq_kHz = 672000;//频点，单位KHZ
					dm6000_handle ->m_device_ctt2.input_params.qam= qam;//QAM类型,对于DVBT/T2信号设置为0,DVBC信号设置为16/32/64/128/256
					dm6000_handle ->m_device_ctt2.input_params.symbol_rate_KSs = sym;//符号率，单位KSs,DVBT/T2信号设置为0,对于DVBC信号正常设置
					dm6000_handle ->m_device_ctt2.input_params.inverted= inverted;//信号是否反转。1反转，0	正常对于DVBT/T2信号设置为0

					if(demod_set_type == MtFeType_DVBT2)
					{
						U8 plp_id = 0;//ELSE other
						mt_fe_dmd_select_plp_dm6k_t2(plp_id);//首次锁台不知道PLP数目，设置PLP为0,其他情况根据实际设置
					}

					mt_fe_dmd_connect_ctt2_dm6k(dm6000_handle);
				}
				if((demod_set_type == MtFeType_DVBS)||(demod_set_type == MtFeType_DVBS2))
				{
					dm6000_handle->m_device_ss2.demod_type = MtFeType_DVBS;///DVBS2; //信号类型,DVBS/S2
					dm6000_handle->m_device_ss2.input_params.input_freq_kHz =?;//频点，单位KHZ
					dm6000_handle->m_device_ss2.input_params.symbol_rate_KSs = ?;//符号率,单位KSs
					mt_fe_dmd_connect_ss2_dm6k(dm6000_handle);
				}
				state = MT_FE_STATE_CONNETING;
				cnt   = 0;
				break;
			case MT_FE_CMD_GET_LOCK_STATE:

				if((demod_set_type == MtFeType_DVBC)||(demod_set_type == MtFeType_DVBT)||(demod_set_type == MtFeType_DVBT2))
					mt_fe_dmd_get_lock_state_ctt2_dm6k(dm6000_handle, &lock_state);
				if((demod_set_type == MtFeType_DVBS)||(demod_set_type == MtFeType_DVBS2))
					mt_fe_dmd_get_lock_state_ss2_dm6k(dm6000_handle, &lock_state);

				/*
					TODO:
						send message to APP, that demodulater lock state !
				*/
				break;
			case MT_FE_CMD_GET_CHAN_INFO_CODE:
				/*
					TODO:
						call function 'mt_fe_dmd_get_chan_info_code_dd3k' to
						get the channel infomation code, and send it to APP.

						you can call function 'mt_fe_dmd_chan_info_code_parse_dd3k'
						to parse the chan_info_code.

						pls see structure MT_FE_DD_CHAN_INFO
				*/
				break;

			case MT_FE_CMD_GET_SIGNAL_INFO:

				if((demod_set_type == MtFeType_DVBC)||(demod_set_type == MtFeType_DVBT)||(demod_set_type == MtFeType_DVBT2))
				{
					mt_fe_dmd_get_lock_state_ctt2_dm6k(dm6000_handle, &lock_state);
					if(lock_state!=MtFeLockState_Locked)
					{
						p_quality = 0;
						p_strength = 0;
					}
					else
					{
						mt_fe_dmd_get_quality_ctt2_dm6k(dm6000_handle, &p_quality);
						mt_fe_dmd_get_strength_ctt2_dm6k(dm6000_handle, p_strength);
					}


				}
				if((demod_set_type == MtFeType_DVBS)||(demod_set_type == MtFeType_DVBS2))
				{
					mt_fe_dmd_get_lock_state_ss2_dm6k(dm6000_handle, &lock_state);
					if(lock_state!=MtFeLockState_Locked)
					{
						p_quality = 0;
						p_strength = 0;
					}
					else
					{
						mt_fe_dmd_get_quality_ss2_dm6k(dm6000_handle, &p_quality);
						mt_fe_dmd_get_strength_ss2_dm6k(dm6000_handle, p_strength);
					}

				}
				/*
					TODO:
						 get the CTTB signal quality and strength, than send they to APP.
				*/
				break;

			default:
				break;
		}
	}
}



