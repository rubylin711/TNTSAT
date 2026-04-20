/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _HAL_DMX_REGS_H
#define _HAL_DMX_REGS_H

extern ulong reg_dmx_regs_base_vit;

/*----------------------------------------------------------------------------*/
/* registers                                                                  */
/*----------------------------------------------------------------------------*/
#define reg_dmx_regs_base_phy   	0xffc00000  
//#define reg_dmx_regs_base_vit 		0xf8c00000


#define dmx_regs_base 			reg_dmx_regs_base_vit


//#define STI_TUNER_INIT          		0xf8f00158
//enum {
    #define reg_dmx_ts0_sample_ctrl             (dmx_regs_base + 0x00000)    /*read/write */
    #define reg_dmx_ts0_sample_sta              (dmx_regs_base + 0x00004)    /* read */
    #define reg_dmx_ts1_sample_ctrl             (dmx_regs_base + 0x00010)    /* read/write */
    #define reg_dmx_ts1_sample_sta              (dmx_regs_base + 0x00014)    /* read */
    #define reg_dmx_ts2_sample_ctrl             (dmx_regs_base + 0x00020)    /* read/write */
    #define reg_dmx_ts2_sample_sta              (dmx_regs_base + 0x00024)    /* read */
    #define reg_dmx_ts3_sample_ctrl             (dmx_regs_base + 0x00030)    /* read/write */
    #define reg_dmx_ts3_sample_sta              (dmx_regs_base + 0x00034)    /* read */
    #define reg_dmx_ts_stop_cnt_len             (dmx_regs_base + 0x00080)    /* read/write */
    #define reg_dmx_swtsi_urgent_cfg            (dmx_regs_base + 0x10010)    /* read/write */
    #define reg_dmx_swtsi_chn_lln_addr          (dmx_regs_base + 0x10020)    /* read/write */
    #define reg_dmx_swtsi_chn_control           (dmx_regs_base + 0x10024)    /* read/write */
    #define reg_dmx_swtsi_chn_state             (dmx_regs_base + 0x10028)    /* read */
    #define reg_dmx_swtsi_chn_af_cfg0           (dmx_regs_base + 0x10040)    /* read/write */
    #define reg_dmx_swtsi_chn_af_cfg1           (dmx_regs_base + 0x10044)    /* read/write */
    #define reg_dmx_swtsi_chn_af_cfg2           (dmx_regs_base + 0x10048)    /* read/write */
    #define reg_dmx_swtsi_chn_af_cfg3           (dmx_regs_base + 0x1004c)    /* read/write */
    #define reg_dmx_swtsi_chn_af_cfg4           (dmx_regs_base + 0x10050)    /* read/write */
    #define reg_dmx_swtsi_chn_af_cfg5           (dmx_regs_base + 0x10054)    /* read/write */
    #define reg_dmx_swtsi_chn_next_lln          (dmx_regs_base + 0x10080)    /* read */
    #define reg_dmx_swtsi_chn_dbuf_staddr    	(dmx_regs_base + 0x10084)    /* read */
    #define reg_dmx_swtsi_chn_dbuf_cfg          (dmx_regs_base + 0x10088)    /* read */
    #define reg_dmx_swtsi_chn_dbuf_pid          (dmx_regs_base + 0x1008c)    /* read */
    #define reg_dmx_swtsi_chn_dbuf_pts          (dmx_regs_base + 0x10090)    /* read */
    #define reg_dmx_swtsi_chn_dbuf_dts          (dmx_regs_base + 0x10094)    /* read */
    #define reg_dmx_swtsi_chn_dbuf_rdpoint      (dmx_regs_base + 0x10098)    /* read */
    #define reg_dmx_swtsi_chn_dbuf_sublen       (dmx_regs_base + 0x1009c)    /* read */
    #define reg_dmx_swtsi_chn_dbuf_wrpoint      (dmx_regs_base + 0x100a0)    /* read/write */
    #define reg_dmx_tsp_pcrsetn                 (dmx_regs_base + 0x20000)    /* read/write */
    #define reg_dmx_tsp_threshold0              (dmx_regs_base + 0x20040)    /* read/write */
    #define reg_dmx_tsp_threshold1              (dmx_regs_base + 0x20044)    /* read/write */
    #define reg_dmx_tsp_tag_clr 				(dmx_regs_base + 0x20060)    /* read/write */
    #define reg_dmx_demux_slotn_cfg0            (dmx_regs_base + 0x30000)    /* read/write */
    #define reg_dmx_demux_slotn_cfg1            (dmx_regs_base + 0x30004)    /* read/write */
    #define reg_dmx_demux_pause_cfg0            (dmx_regs_base + 0x30600)    /* read/write */
    #define reg_dmx_demux_pause_cfg1            (dmx_regs_base + 0x30604)    /* read/write */
	#define reg_dmx_demux_multi_rec_en          (dmx_regs_base + 0x3060c)    /* read/write */
    #define reg_dmx_demux_state                 (dmx_regs_base + 0x30610)    /* read */

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	#define reg_dmx_fp_set_cfg	 				(dmx_regs_base + 0x40000)    /* read/write */
	#define reg_dmx_lln_num_start 				(dmx_regs_base + 0x40008)    /* read/write */
	#define reg_dmx_fp_status	 				(dmx_regs_base + 0x4000C)    /* read */
	#define reg_dmx_fp_swtsi_ch_set				(dmx_regs_base + 0x40010)    /* read/write */    
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1)
	#define reg_dmx_tsi_ds_dsch                 (dmx_regs_base + 0x4000c)    /* read */
    #define reg_dmx_tsi_ds_cw_op                (dmx_regs_base + 0x40010)    /* read/write */
#endif

    #define reg_dmx_tsi_ades_ive0_init          (dmx_regs_base + 0x4001c)    /* read/write */
    #define reg_dmx_tsi_ades_ive1_init          (dmx_regs_base + 0x40020)    /* read/write */
    #define reg_dmx_tsi_ades_ive2_init          (dmx_regs_base + 0x40024)    /* read/write */
    #define reg_dmx_tsi_ades_ive3_init          (dmx_regs_base + 0x40028)    /* read/write */
    #define reg_dmx_tsi_ds_chn_odd0             (dmx_regs_base + 0x41000)    /* read/write */
    #define reg_dmx_tsi_ds_chn_odd1             (dmx_regs_base + 0x41004)    /* read/write */
    #define reg_dmx_tsi_ds_chn_odd2             (dmx_regs_base + 0x41008)    /* read/write */
    #define reg_dmx_tsi_ds_chn_odd3             (dmx_regs_base + 0x4100c)    /* read/write */
    #define reg_dmx_tsi_ds_chn_odd4             (dmx_regs_base + 0x41010)    /* read/write */
    #define reg_dmx_tsi_ds_chn_odd5             (dmx_regs_base + 0x41014)    /* read/write */
    #define reg_dmx_tsi_ds_chn_even0            (dmx_regs_base + 0x41020)    /* read/write */
    #define reg_dmx_tsi_ds_chn_even1            (dmx_regs_base + 0x41024)    /* read/write */
    #define reg_dmx_tsi_ds_chn_even2            (dmx_regs_base + 0x41028)    /* read/write */
    #define reg_dmx_tsi_ds_chn_even3            (dmx_regs_base + 0x4102c)    /* read/write */
    #define reg_dmx_tsi_ds_chn_even4            (dmx_regs_base + 0x41030)    /* read/write */
    #define reg_dmx_tsi_ds_chn_even5            (dmx_regs_base + 0x41034)    /* read/write */
    #define reg_dmx_tsi_ds_chn_info             (dmx_regs_base + 0x41040)    /* read */
    
    #define reg_dmx_tsi_ds_chn_tscfg            (dmx_regs_base + 0x41044)    /* read/write */
    #define reg_dmx_tsi_ds_core                 (dmx_regs_base + 0x41048)    /* read/write */
    #define reg_dmx_tsi_aes_ive                 (dmx_regs_base + 0x4104c)    /* read/write */
    #define reg_dmx_tsi_ades_disc_mode          (dmx_regs_base + 0x41050)    /* read/write */
    #define reg_dmx_tsi_ades_pktmode            (dmx_regs_base + 0x41054)    /* read/write */
    /*for symphony2*************************************************************************/
#if defined(CONFIG_MT_CHIP_SYMPHONY2) || defined(CONFIG_MT_CHIP_SYMPHONY4) 	
    #define reg_dmx_tsi_ds_dsch_for_symphony2		(dmx_regs_base +  0x4000c)  /* read          for  debug*/
    #define reg_dmx_tsi_ds_kill_cnt_for_symphony2	(dmx_regs_base +  0x40010) /* read/write  for debug*/
    #define reg_dmx_ds_big_little_endian			(dmx_regs_base +  0x40028) /* read/write   for key big-little-endian*/
    #define reg_dmx_tsi_ds_chn_tscfg_sym2 			(dmx_regs_base + 0x41000)    /* read/write */
    #define reg_dmx_tsi_ds_core_sym2                (dmx_regs_base + 0x41004)       /* read/write */
    #define reg_dmx_tsi_aes_ive_sym2           		(dmx_regs_base + 0x41008)       /* read/write */
    #define reg_dmx_tsi_ades_disc_mode_sym2     	(dmx_regs_base + 0x4100c)       /* read/write */
    #define reg_dmx_tsi_ades_pktmode_sym2    		(dmx_regs_base + 0x41010)       /* read/write */
    #define reg_dmx_tsi_algo_cw_ive_port         	(dmx_regs_base + 0x41014)       /* read/write */
    #define reg_dmx_tsi_keyslot_tab                	(dmx_regs_base + 0x42000)       /* read/write */

    #define REG_KEYTABLE_BASE					0xBF300000
  
    #define reg_dmx_kt_operation                (REG_KEYTABLE_BASE + 0x04000)       /* read/write */	
    #define reg_dmx_kt_start                    (REG_KEYTABLE_BASE + 0x04004)       /* read/write */  
    #define reg_dmx_kt_endian                   (REG_KEYTABLE_BASE + 0x04008)       /* read/write */   
    #define reg_dmx_kt_write_data_127t96        (REG_KEYTABLE_BASE + 0x04010)       /* write */   
    #define reg_dmx_kt_write_data_95t64         (REG_KEYTABLE_BASE + 0x04014)       /* write */   
    #define reg_dmx_kt_write_data_63t32         (REG_KEYTABLE_BASE + 0x04018)       /* write */   
    #define reg_dmx_kt_write_data_31t0          (REG_KEYTABLE_BASE + 0x0401c)       /* write */
#elif defined(CONFIG_MT_CHIP_SYMPHONY6)   
    #define desc_regs_offset                    0x100000
    #define reg_dmx_tsi_ds_dsch_sym6            (dmx_regs_base + desc_regs_offset + 0x4000c)  /* read          for  debug*/
    #define reg_dmx_tsi_ds_kill_cnt_sym6        (dmx_regs_base +  desc_regs_offset + 0x40010) /* read/write  for debug*/

    #define reg_dmx_tsi_ds_kpt_path_force_sym6  (dmx_regs_base +  desc_regs_offset + 0x40014) /* key path control */
    #define reg_dmx_tsi_ds_section_wr_mode_sym6 (dmx_regs_base +  desc_regs_offset + 0x40018) /* section wr mode*/
    #define reg_dmx_tsi_ds_csa2_bigendian_sym6  (dmx_regs_base +  desc_regs_offset + 0x40028) /* read/write  for debug*/

    #define reg_dmx_tsi_ds_sechd1_busy_sym6     (dmx_regs_base +  desc_regs_offset + 0x4034) /* read/write  for debug*/
    #define reg_dmx_tsi_ds_sechd1_status_sym6   (dmx_regs_base +  desc_regs_offset + 0x40038) /* key path control */
    #define reg_dmx_tsi_ds_dbg0_attr_err_sym6   (dmx_regs_base +  desc_regs_offset + 0x4003c) /* section wr mode*/
    #define reg_dmx_tsi_ds_dbg1_sym6            (dmx_regs_base +  desc_regs_offset + 0x40040) /* read/write  for debug*/
    #define reg_dmx_tsi_ds_dbg2_sym6            (dmx_regs_base +  desc_regs_offset + 0x40044) /* read/write  for debug*/

    #define reg_dmx_tsi_ds_hwcg_mode_sym6       (dmx_regs_base +  desc_regs_offset + 0x40048) /* key path control */
    #define reg_dmx_tsi_ds_tskey_src_opt_sym6   (dmx_regs_base +  desc_regs_offset + 0x4004c) /* section wr mode*/
    #define reg_dmx_tsi_ds_fw_sym6              (dmx_regs_base +  desc_regs_offset + 0x40050) /* read/write  for debug*/

    #define reg_dmx_tsi_ds_status_rdata0_sym6   (dmx_regs_base +  desc_regs_offset + 0x40054) /* read/write  for debug*/
    #define reg_dmx_tsi_ds_status_rdata1_sym6   (dmx_regs_base +  desc_regs_offset + 0x40058) /* key path control */
    #define reg_dmx_tsi_ds_status_rdata2_sym6   (dmx_regs_base +  desc_regs_offset + 0x4005c) /* key path control */

    #define reg_dmx_tsi_multi2_syskey_31_0      (dmx_regs_base + desc_regs_offset +  0x40064) /* section wr mode*/
    #define reg_dmx_tsi_multi2_syskey_63_32     (dmx_regs_base + desc_regs_offset +  0x40068) /* read/write  for debug*/
    #define reg_dmx_tsi_multi2_syskey_95_64     (dmx_regs_base + desc_regs_offset +  0x4006c) /* read/write  for debug*/
    #define reg_dmx_tsi_multi2_syskey_127_96    (dmx_regs_base + desc_regs_offset +  0x40070) /* key path control */
    #define reg_dmx_tsi_multi2_syskey_159_128   (dmx_regs_base + desc_regs_offset +  0x40074) /* section wr mode*/
    #define reg_dmx_tsi_multi2_syskey_191_160   (dmx_regs_base + desc_regs_offset +  0x40078) /* read/write  for debug*/
    #define reg_dmx_tsi_multi2_syskey_223_192   (dmx_regs_base + desc_regs_offset +  0x4007c) /* read/write  for debug*/
    #define reg_dmx_tsi_multi2_syskey_255_224   (dmx_regs_base + desc_regs_offset +  0x40080) /* key path control */
    #define reg_dmx_tsi_multi2_syskey_cpu_dis   (dmx_regs_base + desc_regs_offset +  0x40084) /* section wr mode*/
    #define reg_dmx_tsi_multi2_round            (dmx_regs_base + desc_regs_offset +  0x40088) /* read/write  for debug*/

    #define reg_dmx_tsi_ds_chn_tscfg_sym6      	(dmx_regs_base + desc_regs_offset + 0x41000) /* read/write  for debug*/
    #define reg_dmx_tsi_ds_core_sym6            (dmx_regs_base +  desc_regs_offset + 0x41004) /* key path control */
    #define reg_dmx_tsi_ades_ive_sym6           (dmx_regs_base + desc_regs_offset + 0x41008) /* section wr mode*/
    #define reg_dmx_tsi_ades_disc_mode_sym6     (dmx_regs_base + desc_regs_offset + 0x4100c) /* read/write  for debug*/
    #define reg_dmx_tsi_ades_pktmode_sym6       (dmx_regs_base +  desc_regs_offset + 0x41010) /* csa2/csa2conformance key big-little-endian*/
    #define reg_dmx_tsi_algo_cw_ive_port        (dmx_regs_base + 0x41014  )       /* read/write */
    #define reg_dmx_tsi_keyslot_tab             (dmx_regs_base + desc_regs_offset + 0x42000    )    /* read/write */

    #define REG_KEYTABLE_BASE				    0xBF300000
    #define reg_dmx_kt_operation                (REG_KEYTABLE_BASE + 0x04000  )       /* read/write */	
    #define reg_dmx_kt_start                    (REG_KEYTABLE_BASE + 0x04004  )       /* read/write */  
    #define reg_dmx_kt_endian                   (REG_KEYTABLE_BASE + 0x04008  )       /* read/write */   
    #define reg_dmx_kt_write_data_127t96        (REG_KEYTABLE_BASE + 0x04010  )       /* write */   
    #define reg_dmx_kt_write_data_95t64         (REG_KEYTABLE_BASE + 0x04014  )       /* write */   
    #define reg_dmx_kt_write_data_63t32         (REG_KEYTABLE_BASE + 0x04018  )       /* write */   
    #define reg_dmx_kt_write_data_31t0          (REG_KEYTABLE_BASE + 0x0401c  )       /* write */
#endif
    /*for symphony2*************************************************************************/
    #define reg_dmx_bufn_staddr                 (dmx_regs_base + 0x50000)    /* read/write */
    #define reg_dmx_bufn_size                   (dmx_regs_base + 0x50400)    /* read/write */
    #define reg_dmx_bufn_disc_wptr              (dmx_regs_base + 0x50800)    /* read/write */
    #define reg_dmx_bufn_ts_int_cfg             (dmx_regs_base + 0x50c00)    /* read/write */
    #define reg_dmx_bufn_data_wptr              (dmx_regs_base + 0x51000)    /* read/write */
    #define reg_dmx_bufn_int_sta                (dmx_regs_base + 0x51400)    /* read */
    #define reg_dmx_bufn_cursec_len             (dmx_regs_base + 0x51800)    /* read/write */
    #define reg_dmx_bufn_crc_value              (dmx_regs_base + 0x51c00)    /* read/write */
    #define reg_dmx_filtern_config              (dmx_regs_base + 0x54000)    /* read/write */
    #define reg_dmx_funit_filter_data           (dmx_regs_base + 0x56000)    /* read/write */
    #define reg_dmx_funit_filter_mask           (dmx_regs_base + 0x56800)    /* read/write */
    #define reg_dmx_funit_filter_mode           (dmx_regs_base + 0x57000)    /* read/write */
    #define reg_dmx_sf_in_clear                 (dmx_regs_base + 0x58014)    /* read/write */
    #define reg_dmx_trpp_channel_parse_en       (dmx_regs_base + 0x60000)    /* read/write */
    #define reg_dmx_trpp_ch_clear_status        (dmx_regs_base + 0x60004)    /* read/write */
    #define reg_dmx_trpp_bus_urgent             (dmx_regs_base + 0x6000c)    /* read/write */
    #define reg_dmx_trpp_channel_record_en      (dmx_regs_base + 0x60014)    /* read/write */
    #define reg_dmx_trpp_sc_index_flt1_4        (dmx_regs_base + 0x60018)    /* read/write */
    #define reg_dmx_trpp_sc_index_flt5_8        (dmx_regs_base + 0x6001c)    /* read/write */
    #define reg_dmx_trpp_sc_index_flt9_10       (dmx_regs_base + 0x60020)    /* read/write */
    #define reg_dmx_trpp_sc_index_flt11_12      (dmx_regs_base + 0x60024)    /* read/write */
    #define reg_dmx_trpp_sc_index_flt0          (dmx_regs_base + 0x60028)    /* read/write */
    #define reg_dmx_trpp_esbuf_ch               (dmx_regs_base + 0x6002c)    /* read/write */
	#define reg_dmx_trpp_mode               	(dmx_regs_base + 0x60034)    /* read/write */
    #define reg_dmx_trpp_ch_property            (dmx_regs_base + 0x60100)    /* read/write */
    #define reg_dmx_trpp_ch_parse_set           (dmx_regs_base + 0x60104)    /* read/write */
    #define reg_dmx_trpp_ch_start_code1         (dmx_regs_base + 0x60108)    /* read/write */
    #define reg_dmx_trpp_ch_frm_start_code_m1   (dmx_regs_base + 0x6010c)    /* read/write */
    #define reg_dmx_trpp_ch_start_code2         (dmx_regs_base + 0x60110)    /* read/write */
    #define reg_dmx_trpp_ch_frm_start_code_m2   (dmx_regs_base + 0x60114)    /* read/write */
    #define reg_dmx_trpp_ch_dscrpt_start_addr   (dmx_regs_base + 0x60118)    /* read/write */
    #define reg_dmx_trpp_ch_data_start_addr     (dmx_regs_base + 0x6011c)    /* read/write */
    #define reg_dmx_trpp_ch_dscrpt_end_addr     (dmx_regs_base + 0x60120)    /* read/write */
    #define reg_dmx_trpp_ch_data_end_addr       (dmx_regs_base + 0x60124)    /* read/write */
    #define reg_dmx_trpp_ch_dscrpt_rd_addr      (dmx_regs_base + 0x60128)    /* read/write */
    #define reg_dmx_trpp_ch_data_rd_addr        (dmx_regs_base + 0x6012c)    /* read/write */
    #define reg_dmx_trpp_ch_dscrpt_wr_addr      (dmx_regs_base + 0x60130)    /* read/write */
    #define reg_dmx_trpp_ch_data_wr_addr        (dmx_regs_base + 0x60134)    /* read/write */
    #define reg_dmx_trpp_ch1_ini_info1          (dmx_regs_base + 0x60138)    /* read/write */
    #define reg_dmx_trpp_ch1_ini_info2          (dmx_regs_base + 0x6013c)    /* read/write */
    #define reg_dmx_trpp_ch1_ini_info3          (dmx_regs_base + 0x60140)    /* read/write */
    #define reg_dmx_trpp_ch1_ini_info4          (dmx_regs_base + 0x60144)    /* read/write */
    #define reg_dmx_trpp_ch1_ini_info5          (dmx_regs_base + 0x60148)    /* read/write */
    #define reg_dmx_trpp_ch1_ini_info6          (dmx_regs_base + 0x6014c)    /* read/write */
    #define reg_dmx_trpp_ch1_ini_info7          (dmx_regs_base + 0x60150)    /* read/write */
    #define reg_dmx_trpp_ch1_ini_info8          (dmx_regs_base + 0x60154)    /* read/write */
    #define reg_dmx_trpp_ch1_ini_info9          (dmx_regs_base + 0x60158)    /* read/write */
    #define reg_dmx_trpp_ch_rec_set             (dmx_regs_base + 0x61100)    /* read/write */
    #define reg_dmx_trpp_ch_rec_start_addr      (dmx_regs_base + 0x61104)    /* read/write */
    #define reg_dmx_trpp_ch_rec_end_addr        (dmx_regs_base + 0x61108)    /* read/write */
    #define reg_dmx_trpp_ch_rec_rd_addr         (dmx_regs_base + 0x6110c)    /* read/write */
    #define reg_dmx_trpp_ch_rec_wr_addr         (dmx_regs_base + 0x61110)    /* read/write */
    #define reg_dmx_trpp_ch11_ini_info1         (dmx_regs_base + 0x61114)    /* read/write */
    #define reg_dmx_trpp_ch11_ini_info2         (dmx_regs_base + 0x61118)    /* read/write */
    #define reg_dmx_trpp_ch11_ini_info3         (dmx_regs_base + 0x6111c)    /* read/write */
    #define reg_dmx_trpp_ch_ts_sn               (dmx_regs_base + 0x61118)    /* read/write */

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	#define reg_dmx_trpp_ch11_lln_set0          (dmx_regs_base + 0x61120)
    #define reg_dmx_trpp_ch11_lln_set1          (dmx_regs_base + 0x61124)
	#define reg_dmx_trpp_ch11_lln_set2          (dmx_regs_base + 0x61128)
	#define reg_dmx_trpp_ch11_lln_set3          (dmx_regs_base + 0x6112c)
	
	#define reg_dmx_trpp_ch11_lln_id0_set0      (dmx_regs_base + 0x61130)
	#define reg_dmx_trpp_ch11_lln_id0_set1      (dmx_regs_base + 0x61134)
	#define reg_dmx_trpp_ch11_lln_id1_set0      (dmx_regs_base + 0x61138)
	#define reg_dmx_trpp_ch11_lln_id1_set1      (dmx_regs_base + 0x6113c)
	#define reg_dmx_trpp_ch11_lln_id2_set0      (dmx_regs_base + 0x61140)
	#define reg_dmx_trpp_ch11_lln_id2_set1      (dmx_regs_base + 0x61144)
	#define reg_dmx_trpp_ch11_lln_id3_set0      (dmx_regs_base + 0x61148)
	#define reg_dmx_trpp_ch11_lln_id3_set1      (dmx_regs_base + 0x6114c)
	#define reg_dmx_trpp_ch11_lln_id4_set0      (dmx_regs_base + 0x61150)
	#define reg_dmx_trpp_ch11_lln_id4_set1      (dmx_regs_base + 0x61154)
	#define reg_dmx_trpp_ch11_lln_id5_set0      (dmx_regs_base + 0x61158)
	#define reg_dmx_trpp_ch11_lln_id5_set1      (dmx_regs_base + 0x6115c)
	#define reg_dmx_trpp_ch11_lln_id6_set0      (dmx_regs_base + 0x61160)
	#define reg_dmx_trpp_ch11_lln_id6_set1      (dmx_regs_base + 0x61164)
	#define reg_dmx_trpp_ch11_lln_id7_set0      (dmx_regs_base + 0x61168)
	#define reg_dmx_trpp_ch11_lln_id7_set1      (dmx_regs_base + 0x6116c)

	#define reg_dmx_trpp_ch11_lln_full_0      	(dmx_regs_base + 0x61170)
	#define reg_dmx_trpp_ch11_lln_full_1      	(dmx_regs_base + 0x61174)

	#define reg_dmx_trpp_ch11_lln_rd_byte_set   (dmx_regs_base + 0x61178)
	#define reg_dmx_trpp_ch11_lln_data_byte_num (dmx_regs_base + 0x6117c)
#endif

    #define reg_dmx_trpp_ch_idx_mode            (dmx_regs_base + 0x61500)    /* read/write */
    #define reg_dmx_trpp_ch_idx_enable          (dmx_regs_base + 0x61504)    /* read/write */
    #define reg_dmx_trpp_ch_idx_start_addr      (dmx_regs_base + 0x61508)    /* read/write */
    #define reg_dmx_trpp_ch_idx_end_addr        (dmx_regs_base + 0x6150c)    /* read/write */
    #define reg_dmx_trpp_ch_idx_rd_addr         (dmx_regs_base + 0x61510)    /* read/write */
    #define reg_dmx_trpp_ch_idx_wr_addr         (dmx_regs_base + 0x61514)    /* read/write */
    #define reg_dmx_trpp_ch15_ini_info1         (dmx_regs_base + 0x61518)    /* read/write */
    #define reg_dmx_trpp_ch15_ini_info2         (dmx_regs_base + 0x6151c)    /* read/write */
    #define reg_dmx_trpp_ch15_ini_info3         (dmx_regs_base + 0x61520)    /* read/write */
    #define reg_dmx_trpp_ch15_ini_info4         (dmx_regs_base + 0x61524)    /* read/write */    /*add 2015.12.17 for Bug 71888*/
    #define reg_dmx_trpp_ch15_ini_info5         (dmx_regs_base + 0x61528)    /* read/write */
    #define reg_dmx_trpp_ch15_ini_info6         (dmx_regs_base + 0x6152c)    /* read/write */
    #define reg_dmx_trpp_ch15_ini_info7         (dmx_regs_base + 0x61530)    /* read/write */

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	#define reg_dmx_trpp_ch15_sc_flt_set0       (dmx_regs_base + 0x61538)    /* read/write */
	#define reg_dmx_trpp_ch15_sc_flt_set1       (dmx_regs_base + 0x6153C)    /* read/write */
	#define reg_dmx_trpp_ch15_sc_flt_set2       (dmx_regs_base + 0x61540)    /* read/write */
	#define reg_dmx_trpp_ch15_sc_flt_set3       (dmx_regs_base + 0x61544)    /* read/write */
	#define reg_dmx_trpp_ch15_sc_flt_set4       (dmx_regs_base + 0x61548)    /* read/write */
#endif

    #define reg_dmx_gm_ch_ctrl					(dmx_regs_base + 0x70000)    /* read/write */
    #define reg_dmx_ts_sample_int_mask          (dmx_regs_base + 0x70010)    /* read/write */
    #define reg_dmx_ts_sample_int_edge          (dmx_regs_base + 0x70014)    /* read/write */
    #define reg_dmx_ts_sample_int_clr           (dmx_regs_base + 0x70018)    /* read/write */
    #define reg_dmx_ts_sample_int_state         (dmx_regs_base + 0x7001c)    /* read */
    
    #define reg_dmx_ds_int_mask                 (dmx_regs_base + 0x70030)    /* read/write */
    #define reg_dmx_ds_int_edge                 (dmx_regs_base + 0x70034)    /* read/write */
    #define reg_dmx_ds_int_clr                  (dmx_regs_base + 0x70038)    /* read/write */
    #define reg_dmx_ds_int_state                (dmx_regs_base + 0x7003c)    /* read/write */
    #define reg_dmx_trpp0_int_mask              (dmx_regs_base + 0x70040)    /* read/write */
    #define reg_dmx_trpp0_int_edge              (dmx_regs_base + 0x70044)    /* read/write */
    #define reg_dmx_trpp0_int_clr               (dmx_regs_base + 0x70048)    /* read/write */
    #define reg_dmx_trpp0_int_state             (dmx_regs_base + 0x7004c)    /* read/write */
    #define reg_dmx_trpp1_int_mask              (dmx_regs_base + 0x70050)    /* read/write */
    #define reg_dmx_trpp1_int_edge              (dmx_regs_base + 0x70054)    /* read/write */
    #define reg_dmx_trpp1_int_clr               (dmx_regs_base + 0x70058)    /* read/write */
    #define reg_dmx_trpp1_int_state             (dmx_regs_base + 0x7005c)    /* read/write */
    #define reg_dmx_trpp2_int_mask              (dmx_regs_base + 0x70060)    /* read/write */
    #define reg_dmx_trpp2_int_edge              (dmx_regs_base + 0x70064)    /* read/write */
    #define reg_dmx_trpp2_int_clr               (dmx_regs_base + 0x70068)    /* read/write */
    #define reg_dmx_trpp2_int_state             (dmx_regs_base + 0x7006c)    /* read/write */
    #define reg_dmx_trpp3_int_mask              (dmx_regs_base + 0x70070)    /* read/write */
    #define reg_dmx_trpp3_int_edge              (dmx_regs_base + 0x70074)    /* read/write */
    #define reg_dmx_trpp3_int_clr               (dmx_regs_base + 0x70078)    /* read/write */
    #define reg_dmx_trpp3_int_state             (dmx_regs_base + 0x7007c)    /* read/write */    /*16+16+4+8=44 full_int*/
    #define reg_dmx_trpp4_int_mask              (dmx_regs_base + 0x70080)    /* read/write */
    #define reg_dmx_trpp4_int_edge              (dmx_regs_base + 0x70084)    /* read/write */
    #define reg_dmx_trpp4_int_clr               (dmx_regs_base + 0x70088)    /* read/write */
    #define reg_dmx_trpp4_int_state             (dmx_regs_base + 0x7008c)    /* read/write */
    #define reg_dmx_trpp5_int_mask              (dmx_regs_base + 0x70090)    /* read/write */
    #define reg_dmx_trpp5_int_edge              (dmx_regs_base + 0x70094)    /* read/write */
    #define reg_dmx_trpp5_int_clr               (dmx_regs_base + 0x70098)    /* read/write */
    #define reg_dmx_trpp5_int_state             (dmx_regs_base + 0x7009c)    /* read/write */	  /*16+16+4+8=44 empty_int*/
    #define reg_dmx_trpp6_int_mask              (dmx_regs_base + 0x700a0)    /* read/write */
    #define reg_dmx_trpp6_int_edge              (dmx_regs_base + 0x700a4)    /* read/write */
    #define reg_dmx_trpp6_int_clr               (dmx_regs_base + 0x700a8)    /* read/write */
    #define reg_dmx_trpp6_int_state             (dmx_regs_base + 0x700ac)    /* read/write */
    #define reg_dmx_trpp7_int_mask              (dmx_regs_base + 0x700b0)    /* read/write */
    #define reg_dmx_trpp7_int_edge              (dmx_regs_base + 0x700b4)    /* read/write */
    #define reg_dmx_trpp7_int_clr               (dmx_regs_base + 0x700b8)    /* read/write */
    #define reg_dmx_trpp7_int_state             (dmx_regs_base + 0x700bc)    /* read/write */    /*16+16+4+8=44 overflow_int*/
    #define reg_dmx_trpp8_int_mask              (dmx_regs_base + 0x700c0)    /* read/write */
    #define reg_dmx_trpp8_int_edge              (dmx_regs_base + 0x700c4)    /* read/write */
    #define reg_dmx_trpp8_int_clr               (dmx_regs_base + 0x700c8)    /* read/write */
    #define reg_dmx_trpp8_int_state             (dmx_regs_base + 0x700cc)    /* read/write */
    #define reg_dmx_trpp9_int_mask              (dmx_regs_base + 0x700d0)    /* read/write */
    #define reg_dmx_trpp9_int_edge              (dmx_regs_base + 0x700d4)    /* read/write */
    #define reg_dmx_trpp9_int_clr               (dmx_regs_base + 0x700d8)    /* read/write */
    #define reg_dmx_trpp9_int_state             (dmx_regs_base + 0x700dc)    /* read/write */

	#define reg_dmx_trpp10_int_mask             (dmx_regs_base + 0x7041C)    /* read/write */
    #define reg_dmx_trpp10_int_edge             (dmx_regs_base + 0x70420)    /* read/write */
    #define reg_dmx_trpp10_int_clr              (dmx_regs_base + 0x70424)    /* read/write */
    #define reg_dmx_trpp10_int_state            (dmx_regs_base + 0x70428)    /* read/write */

	#define reg_dmx_trpp11_int_mask             (dmx_regs_base + 0x7042C)    /* read/write */
    #define reg_dmx_trpp11_int_edge             (dmx_regs_base + 0x70430)    /* read/write */
    #define reg_dmx_trpp11_int_clr              (dmx_regs_base + 0x70434)    /* read/write */
    #define reg_dmx_trpp11_int_state            (dmx_regs_base + 0x70438)    /* read/write */

	#define reg_dmx_trpp12_int_mask             (dmx_regs_base + 0x7043C)    /* read/write */
    #define reg_dmx_trpp12_int_edge             (dmx_regs_base + 0x70440)    /* read/write */
    #define reg_dmx_trpp12_int_clr              (dmx_regs_base + 0x70444)    /* read/write */
    #define reg_dmx_trpp12_int_state            (dmx_regs_base + 0x70448)    /* read/write */
	
    #define reg_dmx_trpp_pvr_int_mask           (dmx_regs_base + 0x700e0)
    #define reg_dmx_trpp_pvr_int_edge           (dmx_regs_base + 0x700e4)
    #define reg_dmx_trpp_pvr_int_clr            (dmx_regs_base + 0x700e8)
    #define reg_dmx_trpp_pvr_int_state          (dmx_regs_base + 0x700ec)

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
	#define reg_dmx_trpp_pvr1_int_mask          (dmx_regs_base + 0x703f4)
    #define reg_dmx_trpp_pvr1_int_edge          (dmx_regs_base + 0x703f8)
    #define reg_dmx_trpp_pvr1_int_clr           (dmx_regs_base + 0x703fc)
	#define reg_dmx_trpp_pvr1_int_rd            (dmx_regs_base + 0x70400)
    #define reg_dmx_trpp_pvr1_int_state         (dmx_regs_base + 0x70404)

	#define reg_dmx_trpp_pvr2_int_mask          (dmx_regs_base + 0x70408)
    #define reg_dmx_trpp_pvr2_int_edge          (dmx_regs_base + 0x7040c)
    #define reg_dmx_trpp_pvr2_int_clr           (dmx_regs_base + 0x70410)
	#define reg_dmx_trpp_pvr2_int_rd            (dmx_regs_base + 0x70414)
    #define reg_dmx_trpp_pvr2_int_state         (dmx_regs_base + 0x70418)
#endif

    #define reg_dmx_gglb_int_mask               (dmx_regs_base + 0x700f0)
    #define reg_dmx_gglb_int_edge               (dmx_regs_base + 0x700f4)
    #define reg_dmx_gglb_int_clr                (dmx_regs_base + 0x700f8)
    #define reg_dmx_gglb_int_state              (dmx_regs_base + 0x700fc)

    #define reg_dmx_pcr_fifo_cnt                (dmx_regs_base + 0x701c0)    /* read */
    #define reg_dmx_pcr_value_low               (dmx_regs_base + 0x701c4)    /* read */
    #define reg_dmx_pcr_value_high              (dmx_regs_base + 0x701c8)    /* read */

    #define reg_dmx_pts_fifo_cnt                (dmx_regs_base + 0x701d0)    /* read */
    #define reg_dmx_pts_value_low               (dmx_regs_base + 0x701d4)    /* read */
    #define reg_dmx_pts_value_high              (dmx_regs_base + 0x701d8)    /* read */

	#define reg_dmx_hwcg_mode	               	(dmx_regs_base + 0x701e4)
	
    #define reg_dmx_bus_debug_chn_staddr        (dmx_regs_base + 0x70280)    /* read/write */
    #define reg_dmx_bus_debug_chn_endaddr       (dmx_regs_base + 0x70284)    /* read/write */
    #define reg_dmx_bus_debug_hit_addr          (dmx_regs_base + 0x702c0)    /* read */

    #define reg_dmx_sf_process_sta              (dmx_regs_base + 0x58010)

	/***   add for t2mi 			  ****************************************************************/
	#define reg_dmx_t2mi_en						(dmx_regs_base + 0x70104)
	#define reg_dmx_t2mi_int_mask				(dmx_regs_base + 0x70108)
	#define reg_dmx_t2mi_int_edge				(dmx_regs_base + 0x7010c)
	#define reg_dmx_t2mi_int_clr				(dmx_regs_base + 0x70110)
	#define reg_dmx_t2mi_int_state				(dmx_regs_base + 0x70114)
	#define reg_dmx_t2mi_set1					(dmx_regs_base + 0x80000)
	#define reg_dmx_t2mi_set2					(dmx_regs_base + 0x80004)
	
	//add for address cross-border
	#define reg_dmx_debug_trpp_sec_cfg			(dmx_regs_base + 0x702BC)
	#define reg_dmx_debug_addr_sta				(dmx_regs_base + 0x702C0)
	#define reg_dmx_debug_ch0_start_addr		(dmx_regs_base + 0x70540)
	#define reg_dmx_debug_ch0_end_addr			(dmx_regs_base + 0x70544)
//};

/*add for sym6 verify,please fixme*/

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
#define reg_tsi_ci_set_cfg						(dmx_regs_base + 0x90000)
#define reg_tsi_ci_lln_num_start				(dmx_regs_base + 0x90008)
#define reg_tsi_ci_status						(dmx_regs_base + 0x9000c)  /*read only*/
#define reg_tsi_swtsi_ch_set					(dmx_regs_base + 0x90010)
#define reg_tsi_ci_cicam_set					(dmx_regs_base + 0x90014)
#define reg_tsi_ci_debug						(dmx_regs_base + 0x90018)
#endif

#define reg_ts_src_sel							(0xbf138008)
#define reg_tsi_clk_sel							(0xbf50b004)
/*add end*/

/*----------------------------------------------------------------------------*/
/* bit group structures                                                       */
/*----------------------------------------------------------------------------*/
typedef union { /* dmx_ts0_sample_ctrl */
    mt_u32 all;
    struct {
        mt_u32 syncon_th                   : 4;
        mt_u32 syncoff_th                  : 4;
        mt_u32 ts_188_reg                  : 1;
        mt_u32 ts_188_reg_en               : 1;
        mt_u32                             : 6;
        mt_u32 brk_sel                     : 2;
        mt_u32 sop_token                   : 1;
        mt_u32 sync_bypass                 : 1;
        mt_u32 val_bypass                  : 1;
        mt_u32 sync_err_bypass             : 1;
        mt_u32 tei_bypass                  : 1;
        mt_u32 err_bypass                  : 1;
        mt_u32 err_pol                     : 1;
        mt_u32 val_pol                     : 1;
        mt_u32 sync_pol                    : 1;
        mt_u32 serial_sel                  : 3;
        mt_u32 serial_en                   : 1;
        mt_u32 ts_en                       : 1;
    } bitc;
} reg_ts0_sample_ctrl_t;

typedef union { /* dmx_ts0_sample_sta */
    mt_u32 all;
    struct {
        mt_u32 err_cnt                     : 4;
        mt_u32                             : 4;
        mt_u32 ts_cnt                      : 4;
        mt_u32                             : 11;
        mt_u32 ts_188_en                   : 1;
        mt_u32                             : 6;
        mt_u32 sync_lock                   : 1;
        mt_u32 ts_lock                     : 1;
    } bitc;
} reg_ts0_sample_sta_t;

typedef union { /* dmx_ts1_sample_ctrl */
    mt_u32 all;
    struct {
        mt_u32 syncon_th                   : 4;
        mt_u32 syncoff_th                  : 4;
        mt_u32 ts_188_reg                  : 1;
        mt_u32 ts_188_reg_en             : 1;
        mt_u32 ts_pkt_set                       : 1;
        mt_u32 ts_first_byte_mode         : 1;
        mt_u32 ts_3line_mode                  : 1;      //bit12
        mt_u32 ts_endian               : 1;   //bit13		
        mt_u32                             : 2;
        mt_u32 brk_sel                     : 2;
        mt_u32 sop_token                   : 1;
        mt_u32 sync_bypass                 : 1;
        mt_u32 val_bypass                  : 1;
        mt_u32 sync_err_bypass             : 1;
        mt_u32 tei_bypass                  : 1;
        mt_u32 err_bypass                  : 1;
        mt_u32 err_pol                     : 1;
        mt_u32 val_pol                     : 1;
        mt_u32 sync_pol                    : 1;
        mt_u32 serial_sel                  : 3;
        mt_u32 serial_en                   : 1;
        mt_u32 ts_en                       : 1;
    } bitc;
} reg_ts1_sample_ctrl_t;

typedef union { /* dmx_ts1_sample_sta */
    mt_u32 all;
    struct {
        mt_u32 err_cnt                     : 4;
        mt_u32                             : 4;
        mt_u32 ts_cnt                      : 4;
        mt_u32                             : 11;
        mt_u32 ts_188_en                   : 1;
        mt_u32                             : 6;
        mt_u32 sync_lock                   : 1;
        mt_u32 ts_lock                     : 1;
    } bitc;
} reg_ts1_sample_sta_t;

typedef union { /* dmx_ts2_sample_ctrl */
    mt_u32 all;
    struct {
        mt_u32 syncon_th                   : 4;
        mt_u32 syncoff_th                  : 4;
        mt_u32 ts_188_reg                  : 1;
        mt_u32 ts_188_reg_en               : 1;
        mt_u32                             : 6;
        mt_u32 brk_sel                     : 2;
        mt_u32 sop_token                   : 1;
        mt_u32 sync_bypass                 : 1;
        mt_u32 val_bypass                  : 1;
        mt_u32 sync_err_bypass             : 1;
        mt_u32 tei_bypass                  : 1;
        mt_u32 err_bypass                  : 1;
        mt_u32 err_pol                     : 1;
        mt_u32 val_pol                     : 1;
        mt_u32 sync_pol                    : 1;
        mt_u32 serial_sel                  : 3;
        mt_u32 serial_en                   : 1;
        mt_u32 ts_en                       : 1;
    } bitc;
} reg_ts2_sample_ctrl_t;

typedef union { /* dmx_ts2_sample_sta */
    mt_u32 all;
    struct {
        mt_u32 err_cnt                     : 4;
        mt_u32                             : 4;
        mt_u32 ts_cnt                      : 4;
        mt_u32                             : 11;
        mt_u32 ts_188_en                   : 1;
        mt_u32                             : 6;
        mt_u32 sync_lock                   : 1;
        mt_u32 ts_lock                     : 1;
    } bitc;
} reg_ts2_sample_sta_t;

typedef union { /* dmx_ts3_sample_ctrl */
    mt_u32 all;
    struct {
        mt_u32 syncon_th                   : 4;
        mt_u32 syncoff_th                  : 4;
        mt_u32 ts_188_reg                  : 1;
        mt_u32 ts_188_reg_en               : 1;
        mt_u32                             : 6;
        mt_u32 brk_sel                     : 2;
        mt_u32 sop_token                   : 1;
        mt_u32 sync_bypass                 : 1;
        mt_u32 val_bypass                  : 1;
        mt_u32 sync_err_bypass             : 1;
        mt_u32 tei_bypass                  : 1;
        mt_u32 err_bypass                  : 1;
        mt_u32 err_pol                     : 1;
        mt_u32 val_pol                     : 1;
        mt_u32 sync_pol                    : 1;
        mt_u32 serial_sel                  : 3;
        mt_u32 serial_en                   : 1;
        mt_u32 ts_en                       : 1;
    } bitc;
} reg_ts3_sample_ctrl_t;

typedef union { /* dmx_ts3_sample_sta */
    mt_u32 all;
    struct {
        mt_u32 err_cnt                     : 4;
        mt_u32                             : 4;
        mt_u32 ts_cnt                      : 4;
        mt_u32                             : 11;
        mt_u32 ts_188_en                   : 1;
        mt_u32                             : 6;
        mt_u32 sync_lock                   : 1;
        mt_u32 ts_lock                     : 1;
    } bitc;
} reg_ts3_sample_sta_t;

typedef union { /* dmx_ts_stop_cnt_len */
    mt_u32 all;
    struct {
        mt_u32 ts_stop_cnt_len             : 1;
        mt_u32                             : 31;
    } bitc;
} reg_ts_stop_cnt_len_t;

typedef union { /* dmx_swtsi_urgent_cfg */
    mt_u32 all;
    struct {
        mt_u32 swtsi_urgent_mode           : 1;
        mt_u32                             : 3;
        mt_u32 swtsi_burst_mode            : 1;
        mt_u32                             : 27;
    } bitc;
} reg_swtsi_urgent_cfg_t;

typedef union { /* dmx_swtsi_chn_lln_addr */
    mt_u32 all;
    struct {
        mt_u32 ch_lln_addr                 : 29;
        mt_u32                             : 3;
    } bitc;
} reg_swtsi_chn_lln_addr_t;

typedef union { /* dmx_swtsi_chn_control */
    mt_u32 all;
    struct {
        mt_u32 ch_enable                   : 1;
        mt_u32                             : 3;
        mt_u32 ch_load_en                  : 1;
        mt_u32 ch_lln_reload_en            : 1;
        mt_u32                             : 2;
        mt_u32 ch_src_endian               : 1;
        mt_u32                             : 3;
        mt_u32 ch_time_care                : 1;
        mt_u32                             : 19;
    } bitc;
} reg_swtsi_chn_control_t;

typedef union { /* dmx_swtsi_chn_state */
    mt_u32 all;
    struct {
        mt_u32 ch_busy_state               : 1;
        mt_u32                             : 3;
        mt_u32 ch_load_busy_state          : 1;
        mt_u32                             : 3;
        mt_u32 ch_buf_data_valid           : 1;
        mt_u32 ch_lln_info_valid           : 1;
        mt_u32 ch_time_interval_valid      : 1;
        mt_u32                             : 1;
        mt_u32 ch_sync0_err                : 1;
        mt_u32 ch_sync1_err                : 1;
        mt_u32 ch_len_err                  : 1;
        mt_u32                             : 17;
    } bitc;
} reg_swtsi_chn_state_t;

typedef union { /* dmx_swtsi_chn_af_cfg0 */
    mt_u32 all;
    struct {
        mt_u32 swtsi_af_cfg0               : 32;
    } bitc;
} reg_swtsi_chn_af_cfg0_t;

typedef union { /* dmx_swtsi_chn_af_cfg1 */
    mt_u32 all;
    struct {
        mt_u32 swtsi_af_cfg1               : 32;
    } bitc;
} reg_swtsi_chn_af_cfg1_t;

typedef union { /* dmx_swtsi_chn_af_cfg2 */
    mt_u32 all;
    struct {
        mt_u32 swtsi_af_cfg2               : 32;
    } bitc;
} reg_swtsi_chn_af_cfg2_t;

typedef union { /* dmx_swtsi_chn_af_cfg3 */
    mt_u32 all;
    struct {
        mt_u32 swtsi_af_cfg3               : 32;
    } bitc;
} reg_swtsi_chn_af_cfg3_t;

typedef union { /* dmx_swtsi_chn_af_cfg4 */
    mt_u32 all;
    struct {
        mt_u32 swtsi_af_cfg4               : 32;
    } bitc;
} reg_swtsi_chn_af_cfg4_t;

typedef union { /* dmx_swtsi_chn_af_cfg5 */
    mt_u32 all;
    struct {
        mt_u32 swtsi_af_cfg5               : 32;
    } bitc;
} reg_swtsi_chn_af_cfg5_t;

typedef union { /* dmx_swtsi_chn_next_lln */
    mt_u32 all;
    struct {
        mt_u32 ch_next_lln                 : 29;
        mt_u32                             : 2;
        mt_u32 ch_lln_vld                  : 1;
    } bitc;
} reg_swtsi_chn_next_lln_t;

typedef union { /* dmx_swtsi_chn_dbuf_staddr */
    mt_u32 all;
    struct {
        mt_u32 ch_dbuf_staddr              : 29;
        mt_u32                             : 3;
    } bitc;
} reg_swtsi_chn_dbuf_staddr_t;

typedef union { /* dmx_swtsi_chn_dbuf_cfg */
    mt_u32 all;
    struct {
        mt_u32 ch_dbuf_pid                 : 13;
        mt_u32                             : 3;
        mt_u32 ch_dbuf_vdts                : 1;
        mt_u32 ch_dbuf_vpts                : 1;
        mt_u32 ch_wpont_care_mode          : 1;
        mt_u32                             : 1;
        mt_u32 ch_node_num                 : 8;
        mt_u32 ch_dbuf_type                : 3;
        mt_u32                             : 1;
    } bitc;
} reg_swtsi_chn_dbuf_cfg_t;

typedef union { /* dmx_swtsi_chn_dbuf_pid */
    mt_u32 all;
    struct {
        mt_u32 ch_dbuf_length              : 24;
        mt_u32 ch_dbuf_streamid            : 8;
    } bitc;
} reg_swtsi_chn_dbuf_pid_t;

typedef union { /* dmx_swtsi_chn_dbuf_pts */
    mt_u32 all;
    struct {
        mt_u32 ch_dbuf_pts                 : 32;
    } bitc;
} reg_swtsi_chn_dbuf_pts_t;

typedef union { /* dmx_swtsi_chn_dbuf_dts */
    mt_u32 all;
    struct {
        mt_u32 ch_dbuf_dts                 : 32;
    } bitc;
} reg_swtsi_chn_dbuf_dts_t;

typedef union { /* dmx_swtsi_chn_dbuf_rdpoint */
    mt_u32 all;
    struct {
        mt_u32 ch_dbuf_rdpoint             : 32;
    } bitc;
} reg_swtsi_chn_dbuf_rdpoint_t;

typedef union { /* dmx_swtsi_chn_dbuf_sublen */
    mt_u32 all;
    struct {
        mt_u32 ch_dbuf_sublen              : 24;
        mt_u32                             : 8;
    } bitc;
} reg_swtsi_chn_dbuf_sublen_t;

typedef union { /* dmx_swtsi_chn_dbuf_wrpoint */
    mt_u32 all;
    struct {
        mt_u32 ch_dbuf_wrpoint             : 32;
    } bitc;
} reg_swtsi_chn_dbuf_wrpoint_t;

typedef union { /* dmx_tsp_pcrsetn */
    mt_u32 all;
    struct {
        mt_u32 pcr_ena                     : 1;
        mt_u32                             : 3;
        mt_u32 pcr_ch                      : 3;
        mt_u32                             : 1;
        mt_u32 pcr_id                      : 13;
        mt_u32                             : 11;
    } bitc;
}  reg_dmx_tsp_pcrsetn_t;

typedef union { /* dmx_demux_slotn_cfg0 */
    mt_u32 all;
    struct {
        mt_u32 pid                         : 13;
        mt_u32 pid_filter_en               : 1;
        mt_u32 pid_filter_mode             : 1;
        mt_u32 sc_fetch_ch                 : 1;
        mt_u32 cc_judge_mode               : 8;
        mt_u32 src                         : 3;
        mt_u32 rec_ch                      : 1;
        mt_u32 errts_del_en                : 1;
        mt_u32                             : 2;
        mt_u32 slot_en                     : 1;
    } bitc;
} reg_demux_slotn_cfg0_t;

typedef union { /* dmx_demux_slotn_cfg1 */
    mt_u32 all;
    struct {
        mt_u32 process_type                : 4;
        mt_u32 cw_ch_0_3                       : 4;
        mt_u32 descrambler_en              : 1;
        mt_u32 cw_ch_4                         : 1;
		mt_u32                          : 1;
	 	mt_u32 ci_en                            : 1;  /*add for sym6 verify,please fixme*/
        mt_u32 rec_ch                      : 2;
        mt_u32 buf_full_mode               : 1;
        mt_u32 sec_filter_mode             : 1;
        mt_u32 multisec_dis                : 1;
        mt_u32 sec_discard_mode            : 1;
        mt_u32 sec_mode                    : 2;
        mt_u32 play_ch                     : 8;
        mt_u32 sc_fetch_en                 : 1;
        mt_u32 sc_fetch_ch                 : 3;
    } bitc;
} reg_demux_slotn_cfg1_t;

typedef union { /* dmx_demux_pause_cfg0 */
    mt_u32 all;
    struct {
        mt_u32 stop_slot0_num              : 7;
        mt_u32                             : 1;
        mt_u32 stop_slot1_num              : 7;
        mt_u32                             : 1;
        mt_u32 rec_slot_num                : 7;
        mt_u32                             : 1;
        mt_u32 pause_en                    : 1;
        mt_u32 stop_slot0_en               : 1;
        mt_u32 stop_slot1_en               : 1;
        mt_u32 stop_slot2_en               : 1;
        mt_u32 stop_slot3_en               : 1;
        mt_u32 stop_slot4_en               : 1;
        mt_u32 stop_slot5_en               : 1;
        mt_u32                             : 1;
    } bitc;
} reg_demux_pause_cfg0_t;

typedef union { /* dmx_demux_pause_cfg1 */
    mt_u32 all;
    struct {
        mt_u32 stop_slot2_num              : 7;
        mt_u32                             : 1;
        mt_u32 stop_slot3_num              : 7;
        mt_u32                             : 1;
        mt_u32 stop_slot4_num              : 7;
        mt_u32                             : 1;
        mt_u32 stop_slot5_num              : 7;
        mt_u32                             : 1;
    } bitc;
} reg_demux_pause_cfg1_t;

typedef union { /* dmx_demux_state */
    mt_u32 all;
    struct {
        mt_u32 demux_busy                  : 1;
        mt_u32                             : 3;
        mt_u32 pause_flag_uninserted       : 1;
        mt_u32                             : 3;
        mt_u32 demux_req_cnt               : 4;
        mt_u32 demux_grant_cn              : 4;
        mt_u32                             : 16;
    } bitc;
} reg_demux_state_t;

typedef union { /* dmx_tsi_ds_dsch */
    mt_u32 all;
    struct {
        mt_u32 err_ch                      : 4;
        mt_u32                             : 4;
        mt_u32 ds_busy                     : 1;
        mt_u32                             : 23;
    } bitc;
} reg_tsi_ds_dsch_t;

typedef union { /* dmx_tsi_ds_cw_op */
    mt_u32 all;
    struct {
        mt_u32 clr_en                      : 1;
        mt_u32 odd_push_en                 : 1;
        mt_u32 even_push_en                : 1;
        mt_u32                             : 1;
        mt_u32 cw_ch                       : 4;
        mt_u32                             : 24;
    } bitc;
} reg_tsi_ds_cw_op_t;

typedef union { /* dmx_tsi_ades_ive0_init */
    mt_u32 all;
    struct {
        mt_u32 ive_init0                   : 32;
    } bitc;
} reg_tsi_ades_ive0_init_t;

typedef union { /* dmx_tsi_ades_ive1_init */
    mt_u32 all;
    struct {
        mt_u32 ive_init1                   : 32;
    } bitc;
} reg_tsi_ades_ive1_init_t;

typedef union { /* dmx_tsi_ades_ive2_init */
    mt_u32 all;
    struct {
        mt_u32 ive_init2                   : 32;
    } bitc;
} reg_tsi_ades_ive2_init_t;

typedef union { /* dmx_tsi_ades_ive3_init */
    mt_u32 all;
    struct {
        mt_u32 ive_init3                   : 32;
    } bitc;
} reg_tsi_ades_ive3_init_t;

typedef union { /* dmx_tsi_ds_chn_odd0 */
    mt_u32 all;
    struct {
        mt_u32 cw_odd0                     : 32;
    } bitc;
} reg_tsi_ds_chn_odd0_t;

typedef union { /* dmx_tsi_ds_chn_odd1 */
    mt_u32 all;
    struct {
        mt_u32 cw_odd1                     : 32;
    } bitc;
} reg_tsi_ds_chn_odd1_t;

typedef union { /* dmx_tsi_ds_chn_odd2 */
    mt_u32 all;
    struct {
        mt_u32 cw_odd2                     : 32;
    } bitc;
} reg_tsi_ds_chn_odd2_t;

typedef union { /* dmx_tsi_ds_chn_odd3 */
    mt_u32 all;
    struct {
        mt_u32 cw_odd3                     : 32;
    } bitc;
} reg_tsi_ds_chn_odd3_t;

typedef union { /* dmx_tsi_ds_chn_odd4 */
    mt_u32 all;
    struct {
        mt_u32 cw_odd4                     : 32;
    } bitc;
} reg_tsi_ds_chn_odd4_t;

typedef union { /* dmx_tsi_ds_chn_odd5 */
    mt_u32 all;
    struct {
        mt_u32 cw_odd5                     : 32;
    } bitc;
} reg_tsi_ds_chn_odd5_t;

typedef union { /* dmx_tsi_ds_chn_even0 */
    mt_u32 all;
    struct {
        mt_u32 cw_even0                    : 32;
    } bitc;
} reg_tsi_ds_chn_even0_t;

typedef union { /* dmx_tsi_ds_chn_even1 */
    mt_u32 all;
    struct {
        mt_u32 cw_even1                    : 32;
    } bitc;
} reg_tsi_ds_chn_even1_t;

typedef union { /* dmx_tsi_ds_chn_even2 */
    mt_u32 all;
    struct {
        mt_u32 cw_even2                    : 32;
    } bitc;
} reg_tsi_ds_chn_even2_t;

typedef union { /* dmx_tsi_ds_chn_even3 */
    mt_u32 all;
    struct {
        mt_u32 cw_even3                    : 32;
    } bitc;
} reg_tsi_ds_chn_even3_t;

typedef union { /* dmx_tsi_ds_chn_even4 */
    mt_u32 all;
    struct {
        mt_u32 cw_even4                    : 32;
    } bitc;
} reg_tsi_ds_chn_even4_t;

typedef union { /* dmx_tsi_ds_chn_even5 */
    mt_u32 all;
    struct {
        mt_u32 cw_even5                    : 32;
    } bitc;
} reg_tsi_ds_chn_even5_t;

typedef union { /* dmx_tsi_ds_chn_info */
    mt_u32 all;
    struct {
        mt_u32 odd_cw_sta                  : 1;
        mt_u32 even_cw_sta                 : 1;
        mt_u32                             : 30;
    } bitc;
} reg_tsi_ds_chn_info_t;
/*************************************************add for symphony2******************/
#if defined(CONFIG_MT_CHIP_SYMPHONY6) 
typedef union { /* dmx_tsi_ds_chn_tscfg */
    mt_u32 all;
    struct {
        mt_u32 ds_mode                     : 2;
        mt_u32                             : 2;
        mt_u32 scrtag_clr                  : 1;
        mt_u32 tsscr_clr_range             : 1;
        mt_u32                             : 2;
        mt_u32 ts_cwopt1_mode              : 1;
        mt_u32                             : 3;
        mt_u32 pes_cwopt1_mode             : 1;
        mt_u32 enc_odd_even_eco            : 1;    //1:odd   0:even
        mt_u32 enc_mode_eco             : 1;
        mt_u32 scr_enc_force              : 1;
	 mt_u32 						:2;
	 mt_u32 des_key_msb64             :1;   //bit18  des key use high_64bit key
	 mt_u32 csa2_key_msb64             :1;   //bit19 csa2/csa2conformance caiy
	 mt_u32 multi2_key_msb64             :1;   //bit20
	 mt_u32 gost_sbox_sel             :3;   //bit23:21
	 mt_u32 gost_bit_inv             :1;  //bit24
	 mt_u32                             : 7;	
    } bitc;
} reg_tsi_ds_chn_tscfg_t_sym6;

typedef union { /* dmx_tsi_ds_core */
    mt_u32 all;
    struct {
        mt_u32 ds_core_sel                 : 3;
        mt_u32                             : 1;
	    mt_u32 csa3_opti                   : 2;
	    mt_u32                             : 26;
    } bitc;
} reg_tsi_ds_core_t_sym6;

typedef union { /* dmx_tsi_aes_ive */
    mt_u32 all;
    struct {
        mt_u32 ivecal_en                   : 1;
        mt_u32                             : 3;
        mt_u32 ivecal_mode       : 6;
        mt_u32                             : 22;
    } bitc;
} reg_tsi_aes_ive_t_sym6;


typedef union { /* dmx_tsi_ades_disc_mode */
    mt_u32 all;
    struct {
        mt_u32 disc_mode                   : 8;
        mt_u32 disc_mode_ctr               : 2;
        mt_u32                             : 20;
	 mt_u32  dis_mode_b8        :1;
	 mt_u32                                :1;
    } bitc;
} reg_tsi_ades_disc_mode_t_sym6;

typedef union { /* dmx_tsi_ades_pktmode */
    mt_u32 all;
    struct {
        mt_u32 short_pkt_mode              : 2;
        mt_u32                             : 2;
        mt_u32 small_pkt_mode              : 2;
        mt_u32                             : 26;
    } bitc;
} reg_tsi_ades_pktmode_t_sym6;

typedef union { /* dmx_tsi_algo_cw_ive_port */
    mt_u32 all;
    struct {
        mt_u32 csa2_byte_port              : 1;
        mt_u32 csa2_dw_port                : 1;
        mt_u32                             : 2;
        mt_u32 csa3_byte_port              : 1;
        mt_u32 csa3_dw_port                : 1;
        mt_u32                             : 2;		
        mt_u32 ades_byte_port              : 1;
        mt_u32 ades_dw_port                : 1;
        mt_u32                             : 22;		
    } bitc;
} reg_tsi_algo_cw_ive_port_t;


typedef union { /* dmx_tsi_keyslot_tab */
    mt_u32 all;
    struct {
        mt_u32 entry_valid                 : 1;
        mt_u32                             : 3;
        mt_u32 odd_key_slot_index          : 7;
        mt_u32                             : 1;
        mt_u32 even_key_slot_index         : 7;
        mt_u32                             : 13;
    } bitc;
} reg_tsi_tsi_keyslot_tab_t;


typedef union { /* dmx_key_attribute */
    mt_u32 all;
    struct {
        mt_u32 key_usage          : 17;
        mt_u32 m2m_key_user                            : 1;
	 mt_u32 enc_dec                            : 2;
	 mt_u32 key_size                             : 2;
	 mt_u32 key_souce                            : 3;
	 mt_u32 tdes_key_check                    : 2;
	 mt_u32                                               : 5;
    } bitc;
} reg_dmx_key_attribute_t;

typedef union { /* kt_operation */
    mt_u32 all;
    struct {
        mt_u32 operation          : 3;
        mt_u32                             : 5;
	 mt_u32 target_key_slot                            : 6;
	 mt_u32                              : 2;
	 mt_u32 auto_validate                            : 1;
	 mt_u32                     : 15;
    } bitc;
} reg_dmx_kt_operation_t_sym6;

#else //#elif defined(CONFIG_MT_CHIP_SYMPHONY6) 

/*************************************************add for symphony2******************/
typedef union { /* dmx_tsi_ds_chn_tscfg */
    mt_u32 all;
    struct {
        mt_u32 ds_mode                     : 2;
        mt_u32                             : 2;
        mt_u32 scrtag_clr                  : 1;
        mt_u32 tsscr_clr_range             : 1;
        mt_u32                             : 2;
        mt_u32 ts_cwopt1_mode              : 1;
        mt_u32                             : 3;
        mt_u32 pes_cwopt1_mode             : 1;
        mt_u32 enc_odd_even_eco            : 1;    //1:odd   0:even
        mt_u32 enc_mode_eco             : 1;
        mt_u32 scr_enc_force              : 1;
	 mt_u32                             : 16;	
    } bitc;
} reg_tsi_ds_chn_tscfg_t_sym2;

typedef union { /* dmx_tsi_ds_core */
    mt_u32 all;
    struct {
        mt_u32 ds_core_sel                 : 2;
        mt_u32                             : 2;
	    mt_u32 csa3_opti                   : 1;
	    mt_u32                             : 27;
    } bitc;
} reg_tsi_ds_core_t_sym2;

typedef union { /* dmx_tsi_aes_ive */
    mt_u32 all;
    struct {
        mt_u32 ivecal_en                   : 1;
        mt_u32                             : 3;
        mt_u32 ivecal_mode                 : 5;
        mt_u32 iveinit_reg_sel             : 2;
        mt_u32                             : 21;
    } bitc;
} reg_tsi_aes_ive_t_sym2;


typedef union { /* dmx_tsi_aes_ive */
    mt_u32 all;
    struct {
        mt_u32 ivecal_en                   : 1;
        mt_u32                             : 3;
        mt_u32 ivecal_mode                 : 6;
        mt_u32                             : 22;
    } bitc;
} reg_tsi_aes_ive_t_sym4;


typedef union { /* dmx_tsi_ades_disc_mode */
    mt_u32 all;
    struct {
        mt_u32 disc_mode                   : 8;
        mt_u32 disc_mode_ctr               : 2;
        mt_u32                             : 22;
    } bitc;
} reg_tsi_ades_disc_mode_t_sym2;

typedef union { /* dmx_tsi_ades_pktmode */
    mt_u32 all;
    struct {
        mt_u32 short_pkt_mode              : 1;
        mt_u32                             : 3;
        mt_u32 small_pkt_mode              : 2;
        mt_u32                             : 26;
    } bitc;
} reg_tsi_ades_pktmode_t_sym2;
typedef union { /* dmx_tsi_algo_cw_ive_port */
    mt_u32 all;
    struct {
        mt_u32 csa2_byte_port              : 1;
        mt_u32 csa2_dw_port                : 1;
        mt_u32                             : 2;
        mt_u32 csa3_byte_port              : 1;
        mt_u32 csa3_dw_port                : 1;
        mt_u32                             : 2;		
        mt_u32 ades_byte_port              : 1;
        mt_u32 ades_dw_port                : 1;
        mt_u32                             : 22;		
    } bitc;
} reg_tsi_algo_cw_ive_port_t;

#ifdef CONFIG_MT_CHIP_SYMPHONY4
typedef union { /* dmx_tsi_keyslot_tab */
    mt_u32 all;
    struct {
        mt_u32 entry_valid                 : 1;
        mt_u32                             : 3;
        mt_u32 odd_key_slot_index          : 7;
        mt_u32                             : 1;
        mt_u32 even_key_slot_index         : 7;
        mt_u32                             : 13;
    } bitc;
} reg_tsi_tsi_keyslot_tab_t;
#else
typedef union { /* dmx_tsi_keyslot_tab */
    mt_u32 all;
    struct {
        mt_u32 entry_valid                 : 1;
        mt_u32                             : 3;
        mt_u32 odd_key_slot_index          : 6;
        mt_u32                             : 2;
        mt_u32 even_key_slot_index         : 6;
        mt_u32                             : 14;
    } bitc;
} reg_tsi_tsi_keyslot_tab_t;
#endif

typedef union { /* dmx_key_attribute */
    mt_u32 all;
    struct {
        mt_u32 key_usage          : 17;
        mt_u32 m2m_key_user                            : 1;
	 mt_u32 enc_dec                            : 2;
	 mt_u32 key_size                             : 2;
	 mt_u32 key_souce                            : 3;
	 mt_u32 tdes_key_check                    : 2;
	 mt_u32                                               : 5;
    } bitc;
} reg_dmx_key_attribute_t;

typedef union { /* kt_operation */
    mt_u32 all;
    struct {
        mt_u32 operation          : 3;
        mt_u32                             : 5;
	 mt_u32 target_key_slot                            : 6;
	 mt_u32                              : 2;
	 mt_u32 auto_validate                            : 1;
	 mt_u32                     : 15;
    } bitc;
} reg_dmx_kt_operation_t;


/*************************************************add for symphony2******************/


typedef union { /* dmx_tsi_ds_chn_tscfg */
    mt_u32 all;
    struct {
        mt_u32 ds_mode                     : 2;
        mt_u32                             : 2;
        mt_u32 scrtag_clr                  : 1;
        mt_u32 tsscr_clr_range             : 1;
        mt_u32                             : 2;
        mt_u32 ts_cwopt1_mode              : 1;
        mt_u32                             : 3;
        mt_u32 pes_cwopt1_mode             : 1;
        mt_u32                             : 1;
        mt_u32 pes_cwopt3_mode             : 1;
        mt_u32                             : 17;
    } bitc;
} reg_tsi_ds_chn_tscfg_t;

typedef union { /* dmx_tsi_ds_core */
    mt_u32 all;
    struct {
        mt_u32 ds_core_sel                 : 1;
        mt_u32                             : 31;
    } bitc;
} reg_tsi_ds_core_t;

typedef union { /* dmx_tsi_aes_ive */
    mt_u32 all;
    struct {
        mt_u32 ivecal_en                   : 1;
        mt_u32                             : 3;
        mt_u32 ivecal_mode                 : 5;
        mt_u32 iveinit_reg_sel             : 1;
        mt_u32                             : 22;
    } bitc;
} reg_tsi_aes_ive_t;

typedef union { /* dmx_tsi_ades_disc_mode */
    mt_u32 all;
    struct {
        mt_u32 disc_mode                   : 7;
        mt_u32                             : 25;
    } bitc;
} reg_tsi_ades_disc_mode_t;

typedef union { /* dmx_tsi_ades_pktmode */
    mt_u32 all;
    struct {
        mt_u32 short_pkt_mode              : 1;
        mt_u32                             : 3;
        mt_u32 small_pkt_mode              : 2;
        mt_u32                             : 26;
    } bitc;
} reg_tsi_ades_pktmode_t;

#endif  /*#elif defined(CONFIG_MT_CHIP_SYMPHONY6) */

typedef union { /* dmx_bufn_staddr */
    mt_u32 all;
    struct {
        mt_u32 buf_ch_staddr               : 29;
        mt_u32                             : 3;
    } bitc;
} reg_bufn_staddr_t;

typedef union { /* dmx_bufn_size */
    mt_u32 all;
    struct {
        mt_u32 disc_ch_size                : 6;
        mt_u32 data_ch_size                : 10;
        mt_u32 disc_ch_rptr                : 6;
        mt_u32 data_ch_rptr                : 10;
    } bitc;
} reg_bufn_size_t;

typedef union { /* dmx_bufn_disc_wptr */
    mt_u32 all;
    struct {
        mt_u32 disc_ch_wptr                : 16;
        mt_u32                             : 16;
    } bitc;
} reg_bufn_disc_wptr_t;

typedef union { /* dmx_bufn_ts_int_cfg */
    mt_u32 all;
    struct {
        mt_u32 ts_inf_cfg                  : 3;
        mt_u32                             : 5;
        mt_u32 ts_rcv_cnt                  : 7;
        mt_u32                             : 17;
    } bitc;
} reg_bufn_ts_int_cfg_t;

typedef union { /* dmx_bufn_data_wptr */
    mt_u32 all;
    struct {
        mt_u32 data_ch_wptr                : 20;
        mt_u32                             : 12;
    } bitc;
} reg_bufn_data_wptr_t;

typedef union { /* dmx_bufn_int_sta */
    mt_u32 all;
    struct {
        mt_u32 buf_ch_int_sta              : 8;
        mt_u32                             : 24;
    } bitc;
} reg_bufn_int_sta_t;

typedef union { /* dmx_bufn_cursec_len */
    mt_u32 all;
    struct {
        mt_u32 vld_byte                    : 13;
        mt_u32                             : 3;
        mt_u32 res_length                  : 12;
        mt_u32 syntax                      : 1;
        mt_u32                             : 3;
    } bitc;
} reg_bufn_cursec_len_t;

typedef union { /* dmx_filtern_config */
    mt_u32 all;
    struct {
        mt_u32 filter_root                 : 8;
        mt_u32 filter_en                   : 1;
        mt_u32 rcv_mode                    : 1;
        mt_u32 buf_id                      : 7;
        mt_u32                             : 4;
        mt_u32 filt_sta                    : 2;
        mt_u32 single_end_flag             : 1;
        mt_u32 filter_store                : 8;
    } bitc;
} reg_filtern_config_t;

typedef union { /* dmx_funit_filter_data */
    mt_u32 all;
    struct {
        mt_u32 filter_data_byte3           : 8;
        mt_u32 filter_data_byte2           : 8;
        mt_u32 filter_data_byte1           : 8;
        mt_u32 filter_data_byte0           : 8;
    } bitc;
} reg_funit_filter_data_t;

typedef union { /* dmx_funit_filter_mask */
    mt_u32 all;
    struct {
        mt_u32 filter_mask_byte3           : 8;
        mt_u32 filter_mask_byte2           : 8;
        mt_u32 filter_mask_byte1           : 8;
        mt_u32 filter_mask_byte0           : 8;
    } bitc;
} reg_funit_filter_mask_t;

typedef union { /* dmx_funit_filter_mode */
    mt_u32 all;
    struct {
        mt_u32 filter_next                 : 8;
        mt_u32 filter_mode                 : 4;
        mt_u32 filter_root_end             : 1;
        mt_u32                             : 19;
    } bitc;
} reg_funit_filter_mode_t;

typedef union { /* dmx_trpp_channel_parse_en */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_par_en             : 1;
        mt_u32 trpp_ch2_par_en             : 1;
        mt_u32 trpp_ch3_par_en             : 1;
        mt_u32 trpp_ch4_par_en             : 1;
        mt_u32 trpp_ch5_par_en             : 1;
        mt_u32 trpp_ch6_par_en             : 1;
        mt_u32 trpp_ch7_par_en             : 1;
        mt_u32 trpp_ch8_par_en             : 1;
        mt_u32 trpp_ch9_par_en             : 1;
        mt_u32 trpp_ch10_par_en            : 1;
        mt_u32 trpp_ch11_par_en            : 1;
        mt_u32 trpp_ch12_par_en            : 1;
        mt_u32 trpp_ch13_par_en            : 1;
        mt_u32 trpp_ch14_par_en            : 1;
        mt_u32 trpp_ch15_par_en            : 1;
        mt_u32 trpp_ch16_par_en            : 1;
        mt_u32                             : 16;
    } bitc;
} reg_trpp_channel_parse_en_t;

typedef union { /* dmx_trpp_ch_clear_status */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_clr_ok              : 1;
        mt_u32                             : 31;
    } bitc;
} reg_trpp_ch_clear_status_t;

typedef union { /* dmx_trpp_bus_urgent */
    mt_u32 all;
    struct {
        mt_u32 trpp_urgent_mod             : 2;
        mt_u32                             : 6;
        mt_u32 trpp_burst_len_mod          : 1;
        mt_u32                             : 21;
		mt_u32 trpp_ram_wr_mode            : 1;
		mt_u32 trpp_en_mode                : 1;
    } bitc;
} reg_trpp_bus_urgent_t;

typedef union { /* dmx_trpp_channel_record_en */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_rec_en             : 1;
        mt_u32 trpp_ch2_rec_en             : 1;
        mt_u32 trpp_ch3_rec_en             : 1;
        mt_u32 trpp_ch4_rec_en             : 1;
        mt_u32                             : 4;
        mt_u32 trpp_ch1_idx_en             : 1;
        mt_u32 trpp_ch2_idx_en             : 1;
        mt_u32 trpp_ch3_idx_en             : 1;
        mt_u32 trpp_ch4_idx_en             : 1;
        mt_u32 trpp_ch5_idx_en             : 1;
        mt_u32 trpp_ch6_idx_en             : 1;
        mt_u32 trpp_ch7_idx_en             : 1;
        mt_u32 trpp_ch8_idx_en             : 1;
		mt_u32 trpp_ch9_idx_en             : 1;
        mt_u32 trpp_ch10_idx_en            : 1;
        mt_u32 trpp_ch11_idx_en            : 1;
        mt_u32 trpp_ch12_idx_en            : 1;
		mt_u32 trpp_ch5_rec_en             : 1;
        mt_u32 trpp_ch6_rec_en             : 1;
        mt_u32 trpp_ch7_rec_en             : 1;
        mt_u32                             : 9;
    } bitc;
} reg_trpp_channel_record_en_t;

typedef union { /* dmx_trpp_sc_index_flt1_4 */
    mt_u32 all;
    struct {
        mt_u32 trpp_sc_idx_flt1            : 8;
        mt_u32 trpp_sc_idx_flt2            : 8;
        mt_u32 trpp_sc_idx_flt3            : 8;
        mt_u32 trpp_sc_idx_flt4            : 8;
    } bitc;
} reg_trpp_sc_index_flt1_4_t;

typedef union { /* dmx_trpp_sc_index_flt5_8 */
    mt_u32 all;
    struct {
        mt_u32 trpp_sc_idx_flt5            : 8;
        mt_u32 trpp_sc_idx_flt6            : 8;
        mt_u32 trpp_sc_idx_flt7            : 8;
        mt_u32 trpp_sc_idx_flt8            : 8;
    } bitc;
} reg_trpp_sc_index_flt5_8_t;

typedef union { /* dmx_trpp_sc_index_flt9_10 */
    mt_u32 all;
    struct {
        mt_u32 trpp_sc_idx_flt9_l          : 8;
        mt_u32 trpp_sc_idx_flt9_h          : 8;
        mt_u32 trpp_sc_idx_flt10_l         : 8;
        mt_u32 trpp_sc_idx_flt10_h         : 8;
    } bitc;
} reg_trpp_sc_index_flt9_10_t;

typedef union { /* dmx_trpp_sc_index_flt11_12 */
    mt_u32 all;
    struct {
        mt_u32 trpp_sc_idx_flt11_l         : 8;
        mt_u32 trpp_sc_idx_flt11_h         : 8;
        mt_u32 trpp_sc_idx_flt12_l         : 8;
        mt_u32 trpp_sc_idx_flt12_h         : 8;
    } bitc;
} reg_trpp_sc_index_flt11_12_t;

typedef union { /* dmx_trpp_sc_index_flt0 */
    mt_u32 all;
    struct {
        mt_u32 trpp_sc_idx_byte31          : 8;
        mt_u32 trpp_sc_idx_byte32          : 8;
        mt_u32                             : 16;
    } bitc;
} reg_trpp_sc_index_flt0_t;

typedef union { /* dmx_trpp_esbuf_ch */
    mt_u32 all;
    struct {
        mt_u32 trpp_esbufwp0_chsel         : 4;
        mt_u32 trpp_esbufwp1_chsel         : 4;
        mt_u32 trpp_esbufwp2_chsel         : 4;
        mt_u32 trpp_esbufwp3_chsel         : 4;
        mt_u32                             : 16;
    } bitc;
} reg_trpp_esbuf_ch_t;

typedef union { /* dmx_trpp_ch_property */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_es_mode             : 1;
        mt_u32                             : 3;
        mt_u32 trpp_ch_pusi_detect         : 1;
        mt_u32 trpp_ch_pusi_mode           : 1;
        mt_u32 trpp_ch_strid_mod           : 1;
        mt_u32 trpp_ch_pusi_mode2          : 1;
        mt_u32 trpp_ch_time_info           : 2;
        mt_u32                             : 2;
        mt_u32 trpp_ch_dscrpt_en           : 1;
        mt_u32 trpp_ch_pes_head_en         : 1;
        mt_u32 trpp_ch_fsc_en              : 1;
        mt_u32                             : 1;
        mt_u32 trpp_ch_stream_id           : 8;
        mt_u32 trpp_ch_str_id_msk          : 8;
    } bitc;
} reg_trpp_ch_property_t;

typedef union { /* dmx_trpp_ch_parse_set */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_fsc_cp_en           : 1;
        mt_u32 trpp_ch_sh_detect           : 1;
        mt_u32 trpp_ch_sh_en               : 1;
        mt_u32 trpp_ch_pes_len_mod         : 1;
        mt_u32 trpp_ch_fsc_nbytes          : 3;
        mt_u32                             : 1;
        mt_u32 trpp_ch_insrt_nbytes        : 5;
        mt_u32                             : 3;
        mt_u32 trpp_ch_int_nbytes          : 15;
        mt_u32                             : 1;
    } bitc;
} reg_trpp_ch_parse_set_t;

typedef union { /* dmx_trpp_ch_start_code1 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_fsc_31              : 8;
        mt_u32 trpp_ch_fsc_32              : 8;
        mt_u32 trpp_ch_fsc_41              : 8;
        mt_u32 trpp_ch_fsc_42              : 8;
    } bitc;
} reg_trpp_ch_start_code1_t;

typedef union { /* dmx_trpp_ch_frm_start_code_m1 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_fscm_31             : 8;
        mt_u32 trpp_ch_fscm_32             : 8;
        mt_u32 trpp_ch_fscm_41             : 8;
        mt_u32 trpp_ch_fscm_42             : 8;
    } bitc;
} reg_trpp_ch_frm_start_code_m1_t;

typedef union { /* dmx_trpp_ch_start_code2 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_fsc_51              : 8;
        mt_u32 trpp_ch_fsc_52              : 8;
        mt_u32 trpp_ch_fsc_61              : 8;
        mt_u32 trpp_ch_fsc_62              : 8;
    } bitc;
} reg_trpp_ch_start_code2_t;

typedef union { /* dmx_trpp_ch_frm_start_code_m2 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_fscm_51             : 8;
        mt_u32 trpp_ch_fscm_52             : 8;
        mt_u32 trpp_ch_fscm_61             : 8;
        mt_u32 trpp_ch_fscm_62             : 8;
    } bitc;
} reg_trpp_ch_frm_start_code_m2_t;

typedef union { /* dmx_trpp_ch_dscrpt_start_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_data_mem_th         : 3;
        mt_u32 trpp_ch_data_saddr          : 29;
    } bitc;
} reg_trpp_ch_dscrpt_start_addr_t;

typedef union { /* dmx_trpp_ch_data_start_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_data_mem_th         : 3;
        mt_u32 trpp_ch_data_saddr          : 29;
    } bitc;
} reg_trpp_ch_data_start_addr_t;

typedef union { /* dmx_trpp_ch_dscrpt_end_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_data_eaddr          : 32;
    } bitc;
} reg_trpp_ch_dscrpt_end_addr_t;

typedef union { /* dmx_trpp_ch_data_end_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_data_eaddr          : 32;
    } bitc;
} reg_trpp_ch_data_end_addr_t;

typedef union { /* dmx_trpp_ch_dscrpt_rd_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_dscrpt_raddr        : 27;
        mt_u32                             : 5;
    } bitc;
} reg_trpp_ch_dscrpt_rd_addr_t;

typedef union { /* dmx_trpp_ch_data_rd_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_data_raddr          : 27;
        mt_u32                             : 5;
    } bitc;
} reg_trpp_ch_data_rd_addr_t;

typedef union { /* dmx_trpp_ch_dscrpt_wr_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_dscrpt_waddr        : 27;
        mt_u32                             : 5;
    } bitc;
} reg_trpp_ch_dscrpt_wr_addr_t;

typedef union { /* dmx_trpp_ch_data_wr_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_data_waddr          : 27;
        mt_u32                             : 5;
    } bitc;
} reg_trpp_ch_data_wr_addr_t;

typedef union { /* dmx_trpp_ch1_ini_info1 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_reserved           : 31;
        mt_u32 trpp_ch1_ini_flag_par       : 1;
    } bitc;
} reg_trpp_ch1_ini_info1_t;

typedef union { /* dmx_trpp_ch1_ini_info2 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_dts                : 32;
    } bitc;
} reg_trpp_ch1_ini_info2_t;

typedef union { /* dmx_trpp_ch1_ini_info3 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_pts                : 32;
    } bitc;
} reg_trpp_ch1_ini_info3_t;

typedef union { /* dmx_trpp_ch1_ini_info4 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_reserved           : 32;
    } bitc;
} reg_trpp_ch1_ini_info4_t;

typedef union { /* dmx_trpp_ch1_ini_info5 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_reserved           : 32;
    } bitc;
} reg_trpp_ch1_ini_info5_t;

typedef union { /* dmx_trpp_ch1_ini_info6 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_frame_cnt          : 16;
        mt_u32                             : 16;
    } bitc;
} reg_trpp_ch1_ini_info6_t;

typedef union { /* dmx_trpp_ch1_ini_info7 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_reserved           : 16;
        mt_u32 trpp_ch1_chnum              : 4;
        mt_u32 trpp_ch1_gotstrid           : 1;
        mt_u32 trpp_ch1_discard_flag       : 1;
        mt_u32                             : 2;
        mt_u32 trpp_ch1_reserve            : 8;
    } bitc;
} reg_trpp_ch1_ini_info7_t;

typedef union { /* dmx_trpp_ch1_ini_info8 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_reserved           : 32;
    } bitc;
} reg_trpp_ch1_ini_info8_t;

typedef union { /* dmx_trpp_ch1_ini_info9 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch1_pes_data_cnt       : 17;
        mt_u32 trpp_ch1_reserved           : 15;
    } bitc;
} reg_trpp_ch1_ini_info9_t;

typedef union { /* dmx_trpp_ch_rec_set */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_rec_cnt_th          : 12;
        mt_u32                             : 8;
        mt_u32 trpp_ch_rec_sel             : 1;
        mt_u32                             : 3;
        mt_u32 trpp_ch_rec_mod             : 1;
        mt_u32                             : 7;
    } bitc;
} reg_trpp_ch_rec_set_t;

typedef union { /* dmx_trpp_ch_rec_start_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_rec_mem_th          : 3;
        mt_u32 trpp_ch_rec_saddr           : 29;
    } bitc;
} reg_trpp_ch_rec_start_addr_t;

typedef union { /* dmx_trpp_ch_rec_end_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_rec_eaddr           : 32;
    } bitc;
} reg_trpp_ch_rec_end_addr_t;

typedef union { /* dmx_trpp_ch_rec_rd_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_rec_raddr           : 27;
        mt_u32                             : 5;
    } bitc;
} reg_trpp_ch_rec_rd_addr_t;

typedef union { /* dmx_trpp_ch_rec_wr_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_rec_waddr           : 27;
        mt_u32                             : 5;
    } bitc;
} reg_trpp_ch_rec_wr_addr_t;

typedef union { /* dmx_trpp_ch11_ini_info1 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch11_reserved          : 32;
    } bitc;
} reg_trpp_ch11_ini_info1_t;

typedef union { /* dmx_trpp_ch11_ini_info2 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch11_reserved          : 32;
    } bitc;
} reg_trpp_ch11_ini_info2_t;

typedef union { /* dmx_trpp_ch11_ini_info3 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch11_reserved          : 32;
    } bitc;
} reg_trpp_ch11_ini_info3_t;

typedef union { /* dmx_trpp_ch_ts_sn */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_tsn                 : 32;
    } bitc;
} reg_trpp_ch_ts_sn_t;

typedef union { /* reg_trpp_ch11_lln_set0_t */
    mt_u32 all;
    struct {		
		mt_u32 ch_total_num		: 30;
        mt_u32 lln_type		    : 1;
		mt_u32					: 1;
    } bitc;
} reg_trpp_ch11_lln_set0_t;

typedef union { /* reg_trpp_ch11_lln_set1_t */
    mt_u32 all;
    struct {		
		mt_u32 rec_mem_th_lln   : 3;
		mt_u32					: 29;
    } bitc;
} reg_trpp_ch11_lln_set1_t;

typedef union { /* reg_trpp_ch11_lln_set3_t */
    mt_u32 all;
    struct {
		mt_u32 lln_next_id		: 3;
		mt_u32					: 29;
    } bitc;
} reg_trpp_ch11_lln_set3_t;

typedef union { /* reg_trpp_ch11_lln_id_set0_t */
    mt_u32 all;
    struct {
		mt_u32 lln_id_start_addr   : 32;
    } bitc;
} reg_trpp_ch11_lln_idx_set0_t;

typedef union { /* reg_trpp_ch11_lln_id_set1_t */
    mt_u32 all;
    struct {		
		mt_u32 lln_id_data_len  : 27;
		mt_u32					: 1;
		mt_u32 next_id			: 3;
		mt_u32					: 1;
    } bitc;
} reg_trpp_ch11_lln_idx_set1_t;

typedef union { /* reg_trpp_ch11_lln_full_0_t */
    mt_u32 all;
    struct {		
		mt_u32 lln_id0_full  : 1;
		mt_u32				 : 7;
		mt_u32 lln_id1_full  : 1;
		mt_u32				 : 7;
		mt_u32 lln_id2_full  : 1;
		mt_u32				 : 7;
		mt_u32 lln_id3_full  : 1;
		mt_u32				 : 7;
    } bitc;
} reg_trpp_ch11_lln_full_0_t;

typedef union { /* reg_trpp_ch11_lln_full_0_t */
    mt_u32 all;
    struct {		
		mt_u32 lln_id4_full  : 1;
		mt_u32				 : 7;
		mt_u32 lln_id5_full  : 1;
		mt_u32				 : 7;
		mt_u32 lln_id6_full  : 1;
		mt_u32				 : 7;
		mt_u32 lln_id7_full  : 1;
		mt_u32				 : 7;
    } bitc;
} reg_trpp_ch11_lln_full_1_t;

typedef union { /* reg_trpp_ch11_lln_rd_byte_set_t */
    mt_u32 all;
    struct {
		mt_u32 lln_rd_byte_set  : 30;
		mt_u32				    : 2;
    } bitc;
} reg_trpp_ch11_lln_rd_byte_set_t;

typedef union { /* reg_trpp_ch11_lln_data_byte_num_t */
    mt_u32 all;
    struct {
		mt_u32 lln_data_byte_num  : 30;
		mt_u32				 	  : 2;
    } bitc;
} reg_trpp_ch11_lln_data_byte_num_t;

typedef union { /* dmx_trpp_ch_idx_mode */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_pes_len_mod         : 1;
        mt_u32                             : 3;
        mt_u32 trpp_ch_sc_3byte_en         : 2;
        mt_u32                             : 2;
        mt_u32 trpp_ch_stream_id           : 8;
        mt_u32 trpp_ch_str_id_msk          : 5;
        mt_u32                             : 3;
        mt_u32 trpp_ch_str_id_mod          : 1;
        mt_u32                             : 7;
    } bitc;
} reg_trpp_ch_idx_mode_t;

typedef union { /* dmx_trpp_ch_idx_enable */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_flt_ens             : 12;
        mt_u32 trpp_ch_pau_en              : 1;
        mt_u32                             : 3;
        mt_u32 trpp_ch_pusi_en             : 1;
        mt_u32                             : 3;
        mt_u32 trpp_ch_hd_en               : 1;
        mt_u32                             : 3;
        mt_u32 trpp_ch_pts_en              : 1;
        mt_u32                             : 3;
        mt_u32 trpp_ch_sc_en               : 1;
        mt_u32                             : 2;
        mt_u32 trpp_ch_afld_en             : 1;
    } bitc;
} reg_trpp_ch_idx_enable_t;

typedef union { /* dmx_trpp_ch_idx_start_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_idx_mem_th          : 3;
        mt_u32 trpp_ch_idx_saddr           : 29;
    } bitc;
} reg_trpp_ch_idx_start_addr_t;

typedef union { /* dmx_trpp_ch_idx_end_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_idx_eaddr           : 32;
    } bitc;
} reg_trpp_ch_idx_end_addr_t;

typedef union { /* dmx_trpp_ch_idx_rd_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_idx_raddr           : 27;
        mt_u32                             : 5;
    } bitc;
} reg_trpp_ch_idx_rd_addr_t;

typedef union { /* dmx_trpp_ch_idx_wr_addr */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch_idx_waddr           : 27;
        mt_u32                             : 5;
    } bitc;
} reg_trpp_ch_idx_wr_addr_t;

typedef union { /* dmx_trpp_ch15_ini_info1 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch15_reserved          : 14;
        mt_u32 trpp_ch15_got_strid         : 1;
        mt_u32 trpp_ch15_discard_idx       : 1;
        mt_u32 trpp_ch15_reserved_2        : 16;
    } bitc;
} reg_trpp_ch15_ini_info1_t;

typedef union { /* dmx_trpp_ch15_ini_info2 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch15_reserved          : 32;
    } bitc;
} reg_trpp_ch15_ini_info2_t;

typedef union { /* dmx_trpp_ch15_ini_info3 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch15_reserved          : 32;
    } bitc;
} reg_trpp_ch15_ini_info3_t;

typedef union { /* dmx_trpp_ch15_ini_info4 */
    mt_u32 all;
    struct {
        mt_u32 trpp_ch15_pes_data_cnt      : 16;
        mt_u32 trpp_ch15_reserved          : 16;
    } bitc;
} reg_trpp_ch15_ini_info4_t;

typedef union { /* dmx_ds_int_mask */
    mt_u32 all;
    struct {
        mt_u32 cw_unvld_im                 : 1;
        mt_u32 ts_err1_im                  : 1;
        mt_u32 pes_err1_im                 : 1;
        mt_u32 pes_err2_im                 : 1;
        mt_u32 pes_err3_im                 : 1;
        mt_u32 pes_err4_im                 : 1;
        mt_u32 pes_err5_im                 : 1;
        mt_u32 pes_err6_im                 : 1;
        mt_u32 pes_err7_im                 : 1;
        mt_u32                             : 23;
    } bitc;
} reg_ds_int_mask_t;

typedef union { /* dmx_ds_int_edge */
    mt_u32 all;
    struct {
        mt_u32 cw_unvld_iedge              : 1;
        mt_u32 ts_err1_iedge               : 1;
        mt_u32 pes_err1_iedge              : 1;
        mt_u32 pes_err2_iedge              : 1;
        mt_u32 pes_err3_iedge              : 1;
        mt_u32 pes_err4_iedge              : 1;
        mt_u32 pes_err5_iedge              : 1;
        mt_u32 pes_err6_iedge              : 1;
        mt_u32 pes_err7_iedge              : 1;
        mt_u32                             : 23;
    } bitc;
} reg_ds_int_edge_t;

typedef union { /* dmx_ds_int_clr */
    mt_u32 all;
    struct {
        mt_u32 ds_int_clr                  : 9;
        mt_u32                             : 3;
        mt_u32 ds_int_rd                   : 1;
        mt_u32                             : 19;
    } bitc;
} reg_ds_int_clr_t;

typedef union { /* dmx_ds_int_state */
    mt_u32 all;
    struct {
        mt_u32 cw_unvld_ista               : 1;
        mt_u32 ts_err1_ista                : 1;
        mt_u32 pes_err1_ista               : 1;
        mt_u32 pes_err2_ista               : 1;
        mt_u32 pes_err3_ista               : 1;
        mt_u32 pes_err4_ista               : 1;
        mt_u32 pes_err5_ista               : 1;
        mt_u32 pes_err6_ista               : 1;
        mt_u32 pes_err7_ista               : 1;
        mt_u32                             : 23;
    } bitc;
} reg_ds_int_state_t;

typedef union { /* dmx_trpp0_int_mask */
    mt_u32 all;
    struct {
        mt_u32 ch0_crc_err_im              : 1;
        mt_u32 ch0_str_id_err_im           : 1;
        mt_u32 ch0_pes_wr_err_im           : 1;
        mt_u32 ch0_ts_err_im               : 1;
        mt_u32 ch0_pes_sc_err_im           : 1;
        mt_u32 ch0_pes_data_cnt_im         : 1;
        mt_u32                             : 1;
        mt_u32 ch1_crc_err_im              : 1;
        mt_u32 ch1_str_id_err_im           : 1;
        mt_u32 ch1_pes_wr_err_im           : 1;
        mt_u32 ch1_ts_err_im               : 1;
        mt_u32 ch1_pes_sc_err_im           : 1;
        mt_u32 ch1_pes_data_cnt_im         : 1;
        mt_u32                             : 1;
        mt_u32 ch2_crc_err_im              : 1;
        mt_u32 ch2_str_id_err_im           : 1;
        mt_u32 ch2_pes_wr_err_im           : 1;
        mt_u32 ch2_ts_err_im               : 1;
        mt_u32 ch2_pes_sc_err_im           : 1;
        mt_u32 ch2_pes_data_cnt_im         : 1;
        mt_u32                             : 1;
        mt_u32 ch3_crc_err_im              : 1;
        mt_u32 ch3_str_id_err_im           : 1;
        mt_u32 ch3_pes_wr_err_im           : 1;
        mt_u32 ch3_ts_err_im               : 1;
        mt_u32 ch3_pes_sc_err_im           : 1;
        mt_u32 ch3_pes_data_cnt_im         : 1;
        mt_u32                             : 5;
    } bitc;
} reg_trpp0_int_mask_t;

typedef union { /* dmx_trpp0_int_edge */
    mt_u32 all;
    struct {
        mt_u32 ch0_crc_err_iedge           : 1;
        mt_u32 ch0_str_id_err_iedge        : 1;
        mt_u32 ch0_pes_wr_err_iedge        : 1;
        mt_u32 ch0_ts_err_iedge            : 1;
        mt_u32 ch0_pes_sc_err_iedge        : 1;
        mt_u32 ch0_pes_data_cnt_iedge      : 1;
        mt_u32                             : 1;
        mt_u32 ch1_crc_err_iedge           : 1;
        mt_u32 ch1_str_id_err_iedge        : 1;
        mt_u32 ch1_pes_wr_err_iedge        : 1;
        mt_u32 ch1_ts_err_iedge            : 1;
        mt_u32 ch1_pes_sc_err_iedge        : 1;
        mt_u32 ch1_pes_data_cnt_iedge      : 1;
        mt_u32                             : 1;
        mt_u32 ch2_crc_err_iedge           : 1;
        mt_u32 ch2_str_id_err_iedge        : 1;
        mt_u32 ch2_pes_wr_err_iedge        : 1;
        mt_u32 ch2_ts_err_iedge            : 1;
        mt_u32 ch2_pes_sc_err_iedge        : 1;
        mt_u32 ch2_pes_data_cnt_iedge      : 1;
        mt_u32                             : 1;
        mt_u32 ch3_crc_err_iedge           : 1;
        mt_u32 ch3_str_id_err_iedge        : 1;
        mt_u32 ch3_pes_wr_err_iedge        : 1;
        mt_u32 ch3_ts_err_iedge            : 1;
        mt_u32 ch3_pes_sc_err_iedge        : 1;
        mt_u32 ch3_pes_data_cnt_iedge      : 1;
        mt_u32                             : 5;
    } bitc;
} reg_trpp0_int_edge_t;

typedef union { /* dmx_trpp0_int_clr */
    mt_u32 all;
    struct {
        mt_u32 trpp0_int_clr               : 28;
        mt_u32 trpp0_int_rd                : 1;
        mt_u32                             : 3;
    } bitc;
} reg_trpp0_int_clr_t;

typedef union { /* dmx_trpp0_int_state */
    mt_u32 all;
    struct {
        mt_u32 ch0_crc_err_ista            : 1;
        mt_u32 ch0_str_id_err_ista         : 1;
        mt_u32 ch0_pes_wr_err_ista         : 1;
        mt_u32 ch0_ts_err_ista             : 1;
        mt_u32 ch0_pes_sc_err_ista         : 1;
        mt_u32 ch0_pes_data_cnt_ista       : 1;
        mt_u32                             : 1;
        mt_u32 ch1_crc_err_ista            : 1;
        mt_u32 ch1_str_id_err_ista         : 1;
        mt_u32 ch1_pes_wr_err_ista         : 1;
        mt_u32 ch1_ts_err_ista             : 1;
        mt_u32 ch1_pes_sc_err_ista         : 1;
        mt_u32 ch1_pes_data_cnt_ista       : 1;
        mt_u32                             : 1;
        mt_u32 ch2_crc_err_ista            : 1;
        mt_u32 ch2_str_id_err_ista         : 1;
        mt_u32 ch2_pes_wr_err_ista         : 1;
        mt_u32 ch2_ts_err_ista             : 1;
        mt_u32 ch2_pes_sc_err_ista         : 1;
        mt_u32 ch2_pes_data_cnt_ista       : 1;
        mt_u32                             : 1;
        mt_u32 ch3_crc_err_ista            : 1;
        mt_u32 ch3_str_id_err_ista         : 1;
        mt_u32 ch3_pes_wr_err_ista         : 1;
        mt_u32 ch3_ts_err_ista             : 1;
        mt_u32 ch3_pes_sc_err_ista         : 1;
        mt_u32 ch3_pes_data_cnt_ista       : 1;
        mt_u32                             : 5;
    } bitc;
} reg_trpp0_int_state_t;

typedef union { /* dmx_trpp1_int_mask */
    mt_u32 all;
    struct {
        mt_u32 ch4_crc_err_im              : 1;
        mt_u32 ch4_str_id_err_im           : 1;
        mt_u32 ch4_pes_wr_err_im           : 1;
        mt_u32 ch4_ts_err_im               : 1;
        mt_u32 ch4_pes_sc_err_im           : 1;
        mt_u32 ch4_pes_data_cnt_im         : 1;
        mt_u32                             : 1;
        mt_u32 ch5_crc_err_im              : 1;
        mt_u32 ch5_str_id_err_im           : 1;
        mt_u32 ch5_pes_wr_err_im           : 1;
        mt_u32 ch5_ts_err_im               : 1;
        mt_u32 ch5_pes_sc_err_im           : 1;
        mt_u32 ch5_pes_data_cnt_im         : 1;
        mt_u32                             : 1;
        mt_u32 ch6_crc_err_im              : 1;
        mt_u32 ch6_str_id_err_im           : 1;
        mt_u32 ch6_pes_wr_err_im           : 1;
        mt_u32 ch6_ts_err_im               : 1;
        mt_u32 ch6_pes_sc_err_im           : 1;
        mt_u32 ch6_pes_data_cnt_im         : 1;
        mt_u32                             : 1;
        mt_u32 ch7_crc_err_im              : 1;
        mt_u32 ch7_str_id_err_im           : 1;
        mt_u32 ch7_pes_wr_err_im           : 1;
        mt_u32 ch7_ts_err_im               : 1;
        mt_u32 ch7_pes_sc_err_im           : 1;
        mt_u32 ch7_pes_data_cnt_im         : 1;
        mt_u32                             : 5;
    } bitc;
} reg_trpp1_int_mask_t;

typedef union { /* dmx_trpp1_int_edge */
    mt_u32 all;
    struct {
        mt_u32 ch4_crc_err_iedge           : 1;
        mt_u32 ch4_str_id_err_iedge        : 1;
        mt_u32 ch4_pes_wr_err_iedge        : 1;
        mt_u32 ch4_ts_err_iedge            : 1;
        mt_u32 ch4_pes_sc_err_iedge        : 1;
        mt_u32 ch4_pes_data_cnt_iedge      : 1;
        mt_u32                             : 1;
        mt_u32 ch5_crc_err_iedge           : 1;
        mt_u32 ch5_str_id_err_iedge        : 1;
        mt_u32 ch5_pes_wr_err_iedge        : 1;
        mt_u32 ch5_ts_err_iedge            : 1;
        mt_u32 ch5_pes_sc_err_iedge        : 1;
        mt_u32 ch5_pes_data_cnt_iedge      : 1;
        mt_u32                             : 1;
        mt_u32 ch6_crc_err_iedge           : 1;
        mt_u32 ch6_str_id_err_iedge        : 1;
        mt_u32 ch6_pes_wr_err_iedge        : 1;
        mt_u32 ch6_ts_err_iedge            : 1;
        mt_u32 ch6_pes_sc_err_iedge        : 1;
        mt_u32 ch6_pes_data_cnt_iedge      : 1;
        mt_u32                             : 1;
        mt_u32 ch7_crc_err_iedge           : 1;
        mt_u32 ch7_str_id_err_iedge        : 1;
        mt_u32 ch7_pes_wr_err_iedge        : 1;
        mt_u32 ch7_ts_err_iedge            : 1;
        mt_u32 ch7_pes_sc_err_iedge        : 1;
        mt_u32 ch7_pes_data_cnt_iedge      : 1;
        mt_u32                             : 5;
    } bitc;
} reg_trpp1_int_edge_t;

typedef union { /* dmx_trpp1_int_clr */
    mt_u32 all;
    struct {
        mt_u32 trpp1_int_clr               : 28;
        mt_u32 trpp1_int_rd                : 1;
        mt_u32                             : 3;
    } bitc;
} reg_trpp1_int_clr_t;

typedef union { /* dmx_trpp1_int_state */
    mt_u32 all;
    struct {
        mt_u32 ch4_crc_err_ista            : 1;
        mt_u32 ch4_str_id_err_ista         : 1;
        mt_u32 ch4_pes_wr_err_ista         : 1;
        mt_u32 ch4_ts_err_ista             : 1;
        mt_u32 ch4_pes_sc_err_ista         : 1;
        mt_u32 ch4_pes_data_cnt_ista       : 1;
        mt_u32                             : 1;
        mt_u32 ch5_crc_err_ista            : 1;
        mt_u32 ch5_str_id_err_ista         : 1;
        mt_u32 ch5_pes_wr_err_ista         : 1;
        mt_u32 ch5_ts_err_ista             : 1;
        mt_u32 ch5_pes_sc_err_ista         : 1;
        mt_u32 ch5_pes_data_cnt_ista       : 1;
        mt_u32                             : 1;
        mt_u32 ch6_crc_err_ista            : 1;
        mt_u32 ch6_str_id_err_ista         : 1;
        mt_u32 ch6_pes_wr_err_ista         : 1;
        mt_u32 ch6_ts_err_ista             : 1;
        mt_u32 ch6_pes_sc_err_ista         : 1;
        mt_u32 ch6_pes_data_cnt_ista       : 1;
        mt_u32                             : 1;
        mt_u32 ch7_crc_err_ista            : 1;
        mt_u32 ch7_str_id_err_ista         : 1;
        mt_u32 ch7_pes_wr_err_ista         : 1;
        mt_u32 ch7_ts_err_ista             : 1;
        mt_u32 ch7_pes_sc_err_ista         : 1;
        mt_u32 ch7_pes_data_cnt_ista       : 1;
        mt_u32                             : 5;
    } bitc;
} reg_trpp1_int_state_t;

typedef union { /* dmx_pcr_fifo_cnt */
    mt_u32 all;
    struct {
        mt_u32 pcr_fifo_cnt                : 5;
        mt_u32                             : 27;
    } bitc;
}  reg_dmx_pcr_fifo_cnt_t;

typedef union { /* dmx_pcr_value_low */
    mt_u32 all;
    struct {
        mt_u32 pcr_value_low               : 32;
    } bitc;
}  reg_dmx_pcr_value_low_t;

typedef union { /* dmx_pcr_value_high */
    mt_u32 all;
    struct {
        mt_u32 pcr_value_high              : 10;
        mt_u32 pcr_value_ch                : 3;
        mt_u32 dis_indicator               : 1;
        mt_u32                             : 18;
    } bitc;
}  reg_dmx_pcr_value_high_t;

typedef union { /* dmx_bus_debug_chn_staddr */
    mt_u32 all;
    struct {
        mt_u32 bus_debug_chn_staddr        : 26;
        mt_u32                             : 6;
    } bitc;
}  reg_dmx_bus_debug_chn_staddr_t;

typedef union { /* dmx_bus_debug_chn_endaddr */
    mt_u32 all;
    struct {
        mt_u32 bus_debug_chn_endaddr       : 26;
        mt_u32                             : 6;
    } bitc;
} reg_dmx_bus_debug_chn_endaddr_t;

typedef union { /* dmx_bus_debug_hit_addr */
    mt_u32 all;
    struct {
        mt_u32 bus_debug_hit_addr          : 26;
        mt_u32                             : 6;
    } bitc;
} reg_dmx_bus_debug_hit_addr_t;

typedef union {
    u32 all;
    struct {
        u32 t2mi_pid        : 13;
	 	u32                 : 3;
        u32 t2mi_plp_id     : 8;
        u32 t2mi_output_ch  : 2;   /*00:ts0 01:ts1 10:ts2 11:ts3*/
        u32                 : 2;
#ifdef CONFIG_MT_CHIP_SYMPHONY6
		u32 t2mi_input_ch	: 3;   /*000:ts0 001:ts1 010:ts2 011:ts3, 100:ram0, 101:ram1, 110:ram2, 111:ram3*/
	 	u32 				: 1;
#else
	 	u32 t2mi_input_ch	: 2;   /*00:ts0 01:ts1 10:ts2 11:ts3*/
	 	u32 				: 2;
#endif
    } bitc;
} reg_dmx_t2mi_set1_t;

typedef union {
    u32 all;
    struct {
        u32 t2mi_pusi_det_en  	:1;
	 	u32                   	:3;
        u32 t2mi_pusi_det_mod 	:1;
        u32 			      	:3;   
        u32 t2mi_hd_rfu_det 	:1;
	 	u32 					:3;	
	 	u32 t2mi_pl_rfu_det 	:1;
	 	u32 					:3;
	 	u32 t2mi_pc_det     	:1;
#ifdef CONFIG_MT_CHIP_SYMPHONY6
		u32 					:15;
#else
	 	u32 					:3;
	 	u32 t2mi_swtsi_en   	:1;
	 	u32 					:3;
	 	u32 t2mi_swtsi_src_ch   :2;    /*00:ts0 01:ts1 10:ts2 11:ts3*/
	 	u32 					:6;
#endif
    } bitc;
} reg_dmx_t2mi_set2_t;

/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
typedef union {
	u32 all;
	struct {
		u32 ci_reg_ch 			:3;
		u32 		  			:1;
		u32 ci_swtsi_ch			:2;
		u32 					:2;
		u32 ci_ahb_rd_delay_set	:3;
		u32 					:5;
		u32 reg_ci_en			:1;
		u32 					:3;
		u32 reg_ci_buf_en		:1;
		u32						:3;
		u32 time_out_en			:1;
		u32 					:7;		
		}bitc;
}reg_tsi_ci_set_cfg_t;

typedef union {
	u32 all;
	struct {
		u32 ci_lln_num_start 	:3;
		u32 					:29;		
		}bitc;
}reg_tsi_ci_lln_num_start_t;

typedef union {
	u32 all;
	struct {
		u32 ci_swtsi_ch_time_care 	:1;
		u32 						:3;
		u32 ci_swtsi_ch_src_endian 	:1;
		u32 						:27;
		}bitc;
}reg_tsi_ci_swtsi_ch_set_t;

typedef union {
	u32 all;
	struct {
		u32 cicam_clk_div		:9;
		u32						:3;
		u32 ts_pkt_interval		:8;
		u32 					:10;
		u32 tso_mp_clk_en		:1;
		u32 tso_mp_clk_sel		:1;
		}bitc;
}reg_tsi_ci_cicam_set_t;

typedef union {
	u32 all;
	struct {
		u32 ci_busy		:1;
		u32	ci_buf_busy					:1;
		u32 		:2;
		u32 ci_lln_id					:3;
		u32 			:25;
		}bitc;
}reg_tsi_ci_status_t;

#endif
/*add end*/

/*----------------------------------------------------------------------------*/
/* mirror variables                                                           */
/*----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*/
/* register dmx_ts0_sample_ctrl (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts0_sample_ctrl(mt_u32 data);
mt_u32  reg_get_ts0_sample_ctrl(mt_void);
mt_void reg_set_ts0_sample_ctrl_syncon_th(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_syncon_th(mt_void);
mt_void reg_set_ts0_sample_ctrl_syncoff_th(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_syncoff_th(mt_void);
mt_void reg_set_ts0_sample_ctrl_ts_188_reg(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_ts_188_reg(mt_void);
mt_void reg_set_ts0_sample_ctrl_ts_188_reg_en(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_ts_188_reg_en(mt_void);
mt_void reg_set_ts0_sample_ctrl_brk_sel(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_brk_sel(mt_void);
mt_void reg_set_ts0_sample_ctrl_sop_token(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_sop_token(mt_void);
mt_void reg_set_ts0_sample_ctrl_sync_bypass(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_sync_bypass(mt_void);
mt_void reg_set_ts0_sample_ctrl_val_bypass(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_val_bypass(mt_void);
mt_void reg_set_ts0_sample_ctrl_sync_err_bypass(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_sync_err_bypass(mt_void);
mt_void reg_set_ts0_sample_ctrl_tei_bypass(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_tei_bypass(mt_void);
mt_void reg_set_ts0_sample_ctrl_err_bypass(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_err_bypass(mt_void);
mt_void reg_set_ts0_sample_ctrl_err_pol(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_err_pol(mt_void);
mt_void reg_set_ts0_sample_ctrl_val_pol(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_val_pol(mt_void);
mt_void reg_set_ts0_sample_ctrl_sync_pol(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_sync_pol(mt_void);
mt_void reg_set_ts0_sample_ctrl_serial_sel(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_serial_sel(mt_void);
mt_void reg_set_ts0_sample_ctrl_serial_en(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_serial_en(mt_void);
mt_void reg_set_ts0_sample_ctrl_ts_en(mt_u8 data);
mt_u8   reg_get_ts0_sample_ctrl_ts_en(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ts0_sample_sta (read)                                         */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_ts0_sample_sta(mt_void);
mt_u8   reg_get_ts0_sample_sta_err_cnt(mt_void);
mt_u8   reg_get_ts0_sample_sta_ts_cnt(mt_void);
mt_u8   reg_get_ts0_sample_sta_ts_188_en(mt_void);
mt_u8   reg_get_ts0_sample_sta_sync_lock(mt_void);
mt_u8   reg_get_ts0_sample_sta_ts_lock(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ts1_sample_ctrl (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts1_sample_ctrl(mt_u32 data);
mt_u32  reg_get_ts1_sample_ctrl(mt_void);
mt_void reg_set_ts1_sample_ctrl_syncon_th(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_syncon_th(mt_void);
mt_void reg_set_ts1_sample_ctrl_syncoff_th(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_syncoff_th(mt_void);
mt_void reg_set_ts1_sample_ctrl_ts_188_reg(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_ts_188_reg(mt_void);
mt_void reg_set_ts1_sample_ctrl_ts_188_reg_en(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_ts_188_reg_en(mt_void);
mt_void reg_set_ts1_sample_ctrl_brk_sel(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_brk_sel(mt_void);
mt_void reg_set_ts1_sample_ctrl_sop_token(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_sop_token(mt_void);
mt_void reg_set_ts1_sample_ctrl_sync_bypass(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_sync_bypass(mt_void);
mt_void reg_set_ts1_sample_ctrl_val_bypass(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_val_bypass(mt_void);
mt_void reg_set_ts1_sample_ctrl_sync_err_bypass(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_sync_err_bypass(mt_void);
mt_void reg_set_ts1_sample_ctrl_tei_bypass(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_tei_bypass(mt_void);
mt_void reg_set_ts1_sample_ctrl_err_bypass(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_err_bypass(mt_void);
mt_void reg_set_ts1_sample_ctrl_err_pol(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_err_pol(mt_void);
mt_void reg_set_ts1_sample_ctrl_val_pol(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_val_pol(mt_void);
mt_void reg_set_ts1_sample_ctrl_sync_pol(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_sync_pol(mt_void);
mt_void reg_set_ts1_sample_ctrl_serial_sel(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_serial_sel(mt_void);
mt_void reg_set_ts1_sample_ctrl_serial_en(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_serial_en(mt_void);
mt_void reg_set_ts1_sample_ctrl_ts_en(mt_u8 data);
mt_u8   reg_get_ts1_sample_ctrl_ts_en(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ts1_sample_sta (read)                                         */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_ts1_sample_sta(mt_void);
mt_u8   reg_get_ts1_sample_sta_err_cnt(mt_void);
mt_u8   reg_get_ts1_sample_sta_ts_cnt(mt_void);
mt_u8   reg_get_ts1_sample_sta_ts_188_en(mt_void);
mt_u8   reg_get_ts1_sample_sta_sync_lock(mt_void);
mt_u8   reg_get_ts1_sample_sta_ts_lock(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ts2_sample_ctrl (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts2_sample_ctrl(mt_u32 data);
mt_u32  reg_get_ts2_sample_ctrl(mt_void);
mt_void reg_set_ts2_sample_ctrl_syncon_th(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_syncon_th(mt_void);
mt_void reg_set_ts2_sample_ctrl_syncoff_th(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_syncoff_th(mt_void);
mt_void reg_set_ts2_sample_ctrl_ts_188_reg(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_ts_188_reg(mt_void);
mt_void reg_set_ts2_sample_ctrl_ts_188_reg_en(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_ts_188_reg_en(mt_void);
mt_void reg_set_ts2_sample_ctrl_brk_sel(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_brk_sel(mt_void);
mt_void reg_set_ts2_sample_ctrl_sop_token(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_sop_token(mt_void);
mt_void reg_set_ts2_sample_ctrl_sync_bypass(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_sync_bypass(mt_void);
mt_void reg_set_ts2_sample_ctrl_val_bypass(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_val_bypass(mt_void);
mt_void reg_set_ts2_sample_ctrl_sync_err_bypass(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_sync_err_bypass(mt_void);
mt_void reg_set_ts2_sample_ctrl_tei_bypass(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_tei_bypass(mt_void);
mt_void reg_set_ts2_sample_ctrl_err_bypass(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_err_bypass(mt_void);
mt_void reg_set_ts2_sample_ctrl_err_pol(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_err_pol(mt_void);
mt_void reg_set_ts2_sample_ctrl_val_pol(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_val_pol(mt_void);
mt_void reg_set_ts2_sample_ctrl_sync_pol(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_sync_pol(mt_void);
mt_void reg_set_ts2_sample_ctrl_serial_sel(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_serial_sel(mt_void);
mt_void reg_set_ts2_sample_ctrl_serial_en(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_serial_en(mt_void);
mt_void reg_set_ts2_sample_ctrl_ts_en(mt_u8 data);
mt_u8   reg_get_ts2_sample_ctrl_ts_en(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ts2_sample_sta (read)                                         */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_ts2_sample_sta(mt_void);
mt_u8   reg_get_ts2_sample_sta_err_cnt(mt_void);
mt_u8   reg_get_ts2_sample_sta_ts_cnt(mt_void);
mt_u8   reg_get_ts2_sample_sta_ts_188_en(mt_void);
mt_u8   reg_get_ts2_sample_sta_sync_lock(mt_void);
mt_u8   reg_get_ts2_sample_sta_ts_lock(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ts3_sample_ctrl (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts3_sample_ctrl(mt_u32 data);
mt_u32  reg_get_ts3_sample_ctrl(mt_void);
mt_void reg_set_ts3_sample_ctrl_syncon_th(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_syncon_th(mt_void);
mt_void reg_set_ts3_sample_ctrl_syncoff_th(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_syncoff_th(mt_void);
mt_void reg_set_ts3_sample_ctrl_ts_188_reg(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_ts_188_reg(mt_void);
mt_void reg_set_ts3_sample_ctrl_ts_188_reg_en(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_ts_188_reg_en(mt_void);
mt_void reg_set_ts3_sample_ctrl_brk_sel(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_brk_sel(mt_void);
mt_void reg_set_ts3_sample_ctrl_sop_token(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_sop_token(mt_void);
mt_void reg_set_ts3_sample_ctrl_sync_bypass(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_sync_bypass(mt_void);
mt_void reg_set_ts3_sample_ctrl_val_bypass(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_val_bypass(mt_void);
mt_void reg_set_ts3_sample_ctrl_sync_err_bypass(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_sync_err_bypass(mt_void);
mt_void reg_set_ts3_sample_ctrl_tei_bypass(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_tei_bypass(mt_void);
mt_void reg_set_ts3_sample_ctrl_err_bypass(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_err_bypass(mt_void);
mt_void reg_set_ts3_sample_ctrl_err_pol(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_err_pol(mt_void);
mt_void reg_set_ts3_sample_ctrl_val_pol(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_val_pol(mt_void);
mt_void reg_set_ts3_sample_ctrl_sync_pol(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_sync_pol(mt_void);
mt_void reg_set_ts3_sample_ctrl_serial_sel(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_serial_sel(mt_void);
mt_void reg_set_ts3_sample_ctrl_serial_en(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_serial_en(mt_void);
mt_void reg_set_ts3_sample_ctrl_ts_en(mt_u8 data);
mt_u8   reg_get_ts3_sample_ctrl_ts_en(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ts3_sample_sta (read)                                         */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_ts3_sample_sta(mt_void);
mt_u8   reg_get_ts3_sample_sta_err_cnt(mt_void);
mt_u8   reg_get_ts3_sample_sta_ts_cnt(mt_void);
mt_u8   reg_get_ts3_sample_sta_ts_188_en(mt_void);
mt_u8   reg_get_ts3_sample_sta_sync_lock(mt_void);
mt_u8   reg_get_ts3_sample_sta_ts_lock(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ts_stop_cnt_len (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ts_stop_cnt_len(mt_u32 data);
mt_u32  reg_get_ts_stop_cnt_len(mt_void);
mt_void reg_set_ts_stop_cnt_len_ts_stop_cnt_len(mt_u8 data);
mt_u8   reg_get_ts_stop_cnt_len_ts_stop_cnt_len(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_urgent_cfg (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_urgent_cfg(mt_u32 data);
mt_u32  reg_get_swtsi_urgent_cfg(mt_void);
mt_void reg_set_swtsi_urgent_cfg_swtsi_urgent_mode(mt_u8 data);
mt_u8   reg_get_swtsi_urgent_cfg_swtsi_urgent_mode(mt_void);
mt_void reg_set_swtsi_urgent_cfg_swtsi_burst_mode(mt_u8 data);
mt_u8   reg_get_swtsi_urgent_cfg_swtsi_burst_mode(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_lln_addr (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_lln_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_lln_addr(mt_u8 index);
mt_void reg_set_swtsi_chn_lln_addr_ch_lln_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_lln_addr_ch_lln_addr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_control (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_control(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_control(mt_u8 index);
mt_void reg_set_swtsi_chn_control_ch_enable(mt_u8 index, mt_u8 data);
mt_u8   reg_get_swtsi_chn_control_ch_enable(mt_u8 index);
mt_void reg_set_swtsi_chn_control_ch_load_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_swtsi_chn_control_ch_load_en(mt_u8 index);
mt_void reg_set_swtsi_chn_control_ch_lln_reload_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_swtsi_chn_control_ch_lln_reload_en(mt_u8 index);
mt_void reg_set_swtsi_chn_control_ch_src_endian(mt_u8 index, mt_u8 data);
mt_u8   reg_get_swtsi_chn_control_ch_src_endian(mt_u8 index);
mt_void reg_set_swtsi_chn_control_ch_time_care(mt_u8 index, mt_u8 data);
mt_u8   reg_get_swtsi_chn_control_ch_time_care(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_state (read)                                        */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_state(mt_u8 index);
mt_u8   reg_get_swtsi_chn_state_ch_busy_state(mt_u8 index);
mt_u8   reg_get_swtsi_chn_state_ch_load_busy_state(mt_u8 index);
mt_u8   reg_get_swtsi_chn_state_ch_buf_data_valid(mt_u8 index);
mt_u8   reg_get_swtsi_chn_state_ch_lln_info_valid(mt_u8 index);
mt_u8   reg_get_swtsi_chn_state_ch_time_interval_valid(mt_u8 index);
mt_u8   reg_get_swtsi_chn_state_ch_sync0_err(mt_u8 index);
mt_u8   reg_get_swtsi_chn_state_ch_sync1_err(mt_u8 index);
mt_u8   reg_get_swtsi_chn_state_ch_len_err(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg0 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg0(mt_u8 index);
mt_void reg_set_swtsi_chn_af_cfg0_swtsi_af_cfg0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg0_swtsi_af_cfg0(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg1 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg1(mt_u8 index);
mt_void reg_set_swtsi_chn_af_cfg1_swtsi_af_cfg1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg1_swtsi_af_cfg1(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg2 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg2(mt_u8 index);
mt_void reg_set_swtsi_chn_af_cfg2_swtsi_af_cfg2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg2_swtsi_af_cfg2(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg3 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg3(mt_u8 index);
mt_void reg_set_swtsi_chn_af_cfg3_swtsi_af_cfg3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg3_swtsi_af_cfg3(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg4 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg4(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg4(mt_u8 index);
mt_void reg_set_swtsi_chn_af_cfg4_swtsi_af_cfg4(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg4_swtsi_af_cfg4(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_af_cfg5 (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_af_cfg5(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg5(mt_u8 index);
mt_void reg_set_swtsi_chn_af_cfg5_swtsi_af_cfg5(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_af_cfg5_swtsi_af_cfg5(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_next_lln (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_next_lln(mt_u8 index);
mt_u32  reg_get_swtsi_chn_next_lln_ch_next_lln(mt_u8 index);
mt_u8   reg_get_swtsi_chn_next_lln_ch_lln_vld(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_staddr (read)                                  */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_staddr(mt_u8 index);
mt_u32  reg_get_swtsi_chn_dbuf_staddr_ch_dbuf_staddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_cfg (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_cfg(mt_u8 index);
mt_u16  reg_get_swtsi_chn_dbuf_cfg_ch_dbuf_pid(mt_u8 index);
mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_dbuf_vpts(mt_u8 index);
mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_dbuf_vdts(mt_u8 index);
mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_wpont_care_mode(mt_u8 index);
mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_node_num(mt_u8 index);
mt_u8   reg_get_swtsi_chn_dbuf_cfg_ch_dbuf_type(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_pid (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_pid(mt_u8 index);
mt_u32  reg_get_swtsi_chn_dbuf_pid_ch_dbuf_length(mt_u8 index);
mt_u8   reg_get_swtsi_chn_dbuf_pid_ch_dbuf_streamid(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_pts (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_pts(mt_u8 index);
mt_u32  reg_get_swtsi_chn_dbuf_pts_ch_dbuf_pts(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_dts (read)                                     */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_dts(mt_u8 index);
mt_u32  reg_get_swtsi_chn_dbuf_dts_ch_dbuf_dts(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_rdpoint (read)                                 */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_rdpoint(mt_u8 index);
mt_u32  reg_get_swtsi_chn_dbuf_rdpoint_ch_dbuf_rdpoint(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_sublen (read)                                  */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_swtsi_chn_dbuf_sublen(mt_u8 index);
mt_u32  reg_get_swtsi_chn_dbuf_sublen_ch_dbuf_sublen(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_swtsi_chn_dbuf_wrpoint (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_swtsi_chn_dbuf_wrpoint(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_dbuf_wrpoint(mt_u8 index);
mt_void reg_set_swtsi_chn_dbuf_wrpoint_ch_dbuf_wrpoint(mt_u8 index, mt_u32 data);
mt_u32  reg_get_swtsi_chn_dbuf_wrpoint_ch_dbuf_wrpoint(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsp_pcrset0 (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_tsp_pcrsetn(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_tsp_pcrsetn(mt_u8 index);
mt_void reg_set_dmx_tsp_pcrsetn_pcr_ena(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_tsp_pcrsetn_pcr_ena(mt_u8 index);
mt_void reg_set_dmx_tsp_pcrsetn_pcr_ch(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_tsp_pcrsetn_pcr_ch(mt_u8 index);
mt_void reg_set_dmx_tsp_pcrsetn_pcr_id(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_tsp_pcrsetn_pcr_id(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_demux_slotn_cfg0 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_demux_slotn_cfg0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_demux_slotn_cfg0(mt_u8 index);
mt_void reg_set_demux_slotn_cfg0_pid(mt_u8 index, mt_u16 data);
mt_u16  reg_get_demux_slotn_cfg0_pid(mt_u8 index);
mt_void reg_set_demux_slotn_cfg0_pid_filter_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg0_pid_filter_en(mt_u8 index);
mt_void reg_set_demux_slotn_cfg0_pid_filter_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg0_pid_filter_mode(mt_u8 index);
mt_void reg_set_demux_slotn_cfg0_cc_judge_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg0_cc_judge_mode(mt_u8 index);
mt_void reg_set_demux_slotn_cfg0_src(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg0_src(mt_u8 index);
mt_void reg_set_demux_slotn_cfg0_errts_del_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg0_errts_del_en(mt_u8 index);
mt_void reg_set_demux_slotn_cfg0_slot_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg0_slot_en(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_demux_slotn_cfg1 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_demux_slotn_cfg1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_demux_slotn_cfg1(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_process_type(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_process_type(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_cw_ch(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_cw_ch(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_descrambler_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_descrambler_en(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_rec_ch(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_rec_ch(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_buf_full_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_buf_full_mode(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_sec_filter_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_sec_filter_mode(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_multisec_dis(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_multisec_dis(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_sec_discard_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_sec_discard_mode(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_sec_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_sec_mode(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_play_ch(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_play_ch(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_sc_fetch_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_sc_fetch_en(mt_u8 index);
mt_void reg_set_demux_slotn_cfg1_sc_fetch_ch(mt_u8 index, mt_u8 data);
mt_u8   reg_get_demux_slotn_cfg1_sc_fetch_ch(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_demux_pause_cfg0 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_demux_pause_cfg0(mt_u32 data);
mt_u32  reg_get_demux_pause_cfg0(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_demux_pause_cfg1 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_demux_pause_cfg1(mt_u32 data);
mt_u32  reg_get_demux_pause_cfg1(mt_void);

/*----------------------------------------------------------------------------*/
/* register reg_dmx_demux_multi_rec_en (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_demux_multi_rec_en(mt_void);
mt_void reg_set_demux_multi_rec_en(mt_u32 data);

/*----------------------------------------------------------------------------*/
/* register dmx_demux_state (read)                                            */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_demux_state(mt_void);
mt_u8   reg_get_demux_state_demux_busy(mt_void);
mt_u8   reg_get_demux_state_pause_flag_uninserted(mt_void);
mt_u8   reg_get_demux_state_demux_req_cnt(mt_void);
mt_u8   reg_get_demux_state_demux_grant_cn(mt_void);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
/*----------------------------------------------------------------------------*/
/* register dmx fast play                                                     */
/*----------------------------------------------------------------------------*/
mt_u32   reg_get_demux_fp_set_cfg(mt_void);
mt_void reg_set_demux_fp_set_cfg(mt_u32 data);
mt_u32   reg_get_demux_lln_num_start(mt_void);
mt_void reg_set_demux_lln_num_start(mt_u32 data);
mt_u32   reg_get_demux_fp_status(mt_void);
mt_u32   reg_get_demux_fp_swtsi_ch_set(mt_void);
mt_void reg_set_demux_fp_swtsi_ch_set(mt_u32 data);
#endif

#if defined(CONFIG_MT_CHIP_SYMPHONY1)
/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_dsch (read)                                            */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_tsi_ds_dsch(mt_void);
mt_u8   reg_get_tsi_ds_dsch_err_ch(mt_void);
mt_u8   reg_get_tsi_ds_dsch_ds_busy(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_cw_op (read/write)                                     */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_cw_op(mt_u32 data);
mt_u32  reg_get_tsi_ds_cw_op(mt_void);
mt_void reg_set_tsi_ds_cw_op_clr_en(mt_u8 data);
mt_u8   reg_get_tsi_ds_cw_op_clr_en(mt_void);
mt_void reg_set_tsi_ds_cw_op_odd_push_en(mt_u8 data);
mt_u8   reg_get_tsi_ds_cw_op_odd_push_en(mt_void);
mt_void reg_set_tsi_ds_cw_op_even_push_en(mt_u8 data);
mt_u8   reg_get_tsi_ds_cw_op_even_push_en(mt_void);
mt_void reg_set_tsi_ds_cw_op_cw_ch(mt_u8 data);
mt_u8   reg_get_tsi_ds_cw_op_cw_ch(mt_void);
#endif

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_ive0_init (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_ive0_init(mt_u32 data);
mt_u32  reg_get_tsi_ades_ive0_init(mt_void);
mt_void reg_set_tsi_ades_ive0_init_ive_init0(mt_u32 data);
mt_u32  reg_get_tsi_ades_ive0_init_ive_init0(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_ive1_init (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_ive1_init(mt_u32 data);
mt_u32  reg_get_tsi_ades_ive1_init(mt_void);
mt_void reg_set_tsi_ades_ive1_init_ive_init1(mt_u32 data);
mt_u32  reg_get_tsi_ades_ive1_init_ive_init1(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_ive2_init (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_ive2_init(mt_u32 data);
mt_u32  reg_get_tsi_ades_ive2_init(mt_void);
mt_void reg_set_tsi_ades_ive2_init_ive_init2(mt_u32 data);
mt_u32  reg_get_tsi_ades_ive2_init_ive_init2(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_ive3_init (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_ive3_init(mt_u32 data);
mt_u32  reg_get_tsi_ades_ive3_init(mt_void);
mt_void reg_set_tsi_ades_ive3_init_ive_init3(mt_u32 data);
mt_u32  reg_get_tsi_ades_ive3_init_ive_init3(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd0 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd0(mt_u8 index);
mt_void reg_set_tsi_ds_chn_odd0_cw_odd0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd0_cw_odd0(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd1 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd1(mt_u8 index);
mt_void reg_set_tsi_ds_chn_odd1_cw_odd1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd1_cw_odd1(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd2 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd2(mt_u8 index);
mt_void reg_set_tsi_ds_chn_odd2_cw_odd2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd2_cw_odd2(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd3 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd3(mt_u8 index);
mt_void reg_set_tsi_ds_chn_odd3_cw_odd3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd3_cw_odd3(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd4 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd4(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd4(mt_u8 index);
mt_void reg_set_tsi_ds_chn_odd4_cw_odd4(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd4_cw_odd4(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_odd5 (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_odd5(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd5(mt_u8 index);
mt_void reg_set_tsi_ds_chn_odd5_cw_odd5(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_odd5_cw_odd5(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even0 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even0(mt_u8 index);
mt_void reg_set_tsi_ds_chn_even0_cw_even0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even0_cw_even0(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even1 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even1(mt_u8 index);
mt_void reg_set_tsi_ds_chn_even1_cw_even1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even1_cw_even1(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even2 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even2(mt_u8 index);
mt_void reg_set_tsi_ds_chn_even2_cw_even2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even2_cw_even2(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even3 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even3(mt_u8 index);
mt_void reg_set_tsi_ds_chn_even3_cw_even3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even3_cw_even3(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even4 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even4(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even4(mt_u8 index);
mt_void reg_set_tsi_ds_chn_even4_cw_even4(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even4_cw_even4(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_even5 (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_even5(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even5(mt_u8 index);
mt_void reg_set_tsi_ds_chn_even5_cw_even5(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_even5_cw_even5(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_info (read)                                        */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_tsi_ds_chn_info(mt_u8 index);
mt_u8   reg_get_tsi_ds_chn_info_odd_cw_sta(mt_u8 index);
mt_u8   reg_get_tsi_ds_chn_info_even_cw_sta(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_chn_tscfg (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_chn_tscfg(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_tscfg(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_ds_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_ds_mode(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_scrtag_clr(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_scrtag_clr(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_tsscr_clr_range(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_tsscr_clr_range(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_ts_cwopt1_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_ts_cwopt1_mode(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_pes_cwopt1_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_cwopt1_mode(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_pes_cwopt3_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_cwopt3_mode(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_core (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_core(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_core(mt_u8 index);
mt_void reg_set_tsi_ds_core_ds_core_sel(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_core_ds_core_sel(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_aes_ive (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_aes_ive(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_aes_ive(mt_u8 index);
mt_void reg_set_tsi_aes_ive_ivecal_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_aes_ive_ivecal_en(mt_u8 index);
mt_void reg_set_tsi_aes_ive_ivecal_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_aes_ive_ivecal_mode(mt_u8 index);
mt_void reg_set_tsi_aes_ive_iveinit_reg_sel(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_aes_ive_iveinit_reg_sel(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_disc_mode (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_disc_mode(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ades_disc_mode(mt_u8 index);
mt_void reg_set_tsi_ades_disc_mode_disc_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ades_disc_mode_disc_mode(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_pktmode (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_pktmode(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ades_pktmode(mt_u8 index);
mt_void reg_set_tsi_ades_pktmode_short_pkt_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ades_pktmode_short_pkt_mode(mt_u8 index);
mt_void reg_set_tsi_ades_pktmode_small_pkt_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ades_pktmode_small_pkt_mode(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_staddr (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_staddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_bufn_staddr(mt_u8 index);
mt_void reg_set_bufn_staddr_buf_ch_staddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_bufn_staddr_buf_ch_staddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_size (read/write)                                        */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_size(mt_u8 index, mt_u32 data);
mt_u32  reg_get_bufn_size(mt_u8 index);
mt_void reg_set_bufn_size_disc_ch_size(mt_u8 index, mt_u8 data);
mt_u8   reg_get_bufn_size_disc_ch_size(mt_u8 index);
mt_void reg_set_bufn_size_data_ch_size(mt_u8 index, mt_u16 data);
mt_u16  reg_get_bufn_size_data_ch_size(mt_u8 index);
mt_void reg_set_bufn_size_disc_ch_rptr(mt_u8 index, mt_u8 data);
mt_u8   reg_get_bufn_size_disc_ch_rptr(mt_u8 index);
mt_void reg_set_bufn_size_data_ch_rptr(mt_u8 index, mt_u16 data);
mt_u16  reg_get_bufn_size_data_ch_rptr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_disc_wptr (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_disc_wptr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_bufn_disc_wptr(mt_u8 index);
mt_void reg_set_bufn_disc_wptr_disc_ch_wptr(mt_u8 index, mt_u16 data);
mt_u16  reg_get_bufn_disc_wptr_disc_ch_wptr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_ts_int_cfg (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_ts_int_cfg(mt_u8 index, mt_u32 data);
mt_u32  reg_get_bufn_ts_int_cfg(mt_u8 index);
mt_void reg_set_bufn_ts_int_cfg_ts_inf_cfg(mt_u8 index, mt_u8 data);
mt_u8   reg_get_bufn_ts_int_cfg_ts_inf_cfg(mt_u8 index);
mt_void reg_set_bufn_ts_int_cfg_ts_rcv_cnt(mt_u8 index, mt_u8 data);
mt_u8   reg_get_bufn_ts_int_cfg_ts_rcv_cnt(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_data_wptr (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_data_wptr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_bufn_data_wptr(mt_u8 index);
mt_void reg_set_bufn_data_wptr_data_ch_wptr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_bufn_data_wptr_data_ch_wptr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_int_sta (read)                                           */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_bufn_int_sta(mt_u8 index);
mt_u8   reg_get_bufn_int_sta_buf_ch_int_sta(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_bufn_cursec_len (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_bufn_cursec_len(mt_u8 index, mt_u32 data);
mt_u32  reg_get_bufn_cursec_len(mt_u8 index);
mt_void reg_set_bufn_cursec_len_vld_byte(mt_u8 index, mt_u16 data);
mt_u16  reg_get_bufn_cursec_len_vld_byte(mt_u8 index);
mt_void reg_set_bufn_cursec_len_res_length(mt_u8 index, mt_u16 data);
mt_u16  reg_get_bufn_cursec_len_res_length(mt_u8 index);
mt_void reg_set_bufn_cursec_len_syntax(mt_u8 index, mt_u8 data);
mt_u8   reg_get_bufn_cursec_len_syntax(mt_u8 index);

mt_u32  reg_get_bufn_crc_value(mt_u8 index);
/*----------------------------------------------------------------------------*/
/* register dmx_filtern_config (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_filtern_config(mt_u8 index, mt_u32 data);
mt_u32  reg_get_filtern_config(mt_u8 index);
mt_void reg_set_filtern_config_filter_root(mt_u8 index, mt_u8 data);
mt_u8   reg_get_filtern_config_filter_root(mt_u8 index);
mt_void reg_set_filtern_config_filter_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_filtern_config_filter_en(mt_u8 index);
mt_void reg_set_filtern_config_rcv_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_filtern_config_rcv_mode(mt_u8 index);
mt_void reg_set_filtern_config_slot_num(mt_u8 index, mt_u8 data);
mt_u8   reg_get_filtern_config_slot_num(mt_u8 index);
mt_void reg_set_filtern_config_filt_sta(mt_u8 index, mt_u8 data);
mt_u8   reg_get_filtern_config_filt_sta(mt_u8 index);
mt_void reg_set_filtern_config_single_end_flag(mt_u8 index, mt_u8 data);
mt_u8   reg_get_filtern_config_single_end_flag(mt_u8 index);
mt_void reg_set_filtern_config_filter_store(mt_u8 index, mt_u8 data);
mt_u8   reg_get_filtern_config_filter_store(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_funit_filter_data (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_funit_filter_data(mt_u8 index, mt_u32 data);
mt_u32  reg_get_funit_filter_data(mt_u8 index);
mt_void reg_set_funit_filter_data_filter_data_byte3(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_data_filter_data_byte3(mt_u8 index);
mt_void reg_set_funit_filter_data_filter_data_byte2(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_data_filter_data_byte2(mt_u8 index);
mt_void reg_set_funit_filter_data_filter_data_byte1(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_data_filter_data_byte1(mt_u8 index);
mt_void reg_set_funit_filter_data_filter_data_byte0(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_data_filter_data_byte0(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_funit_filter_mask (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_funit_filter_mask(mt_u8 index, mt_u32 data);
mt_u32  reg_get_funit_filter_mask(mt_u8 index);
mt_void reg_set_funit_filter_mask_filter_mask_byte3(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_mask_filter_mask_byte3(mt_u8 index);
mt_void reg_set_funit_filter_mask_filter_mask_byte2(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_mask_filter_mask_byte2(mt_u8 index);
mt_void reg_set_funit_filter_mask_filter_mask_byte1(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_mask_filter_mask_byte1(mt_u8 index);
mt_void reg_set_funit_filter_mask_filter_mask_byte0(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_mask_filter_mask_byte0(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_funit_filter_mode (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_funit_filter_mode(mt_u8 index, mt_u32 data);
mt_u32  reg_get_funit_filter_mode(mt_u8 index);
mt_void reg_set_funit_filter_mode_filter_next(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_mode_filter_next(mt_u8 index);
mt_void reg_set_funit_filter_mode_filter_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_mode_filter_mode(mt_u8 index);
mt_void reg_set_funit_filter_mode_filter_root_end(mt_u8 index, mt_u8 data);
mt_u8   reg_get_funit_filter_mode_filter_root_end(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_channel_parse_en (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_channel_parse_en(mt_u32 data);
mt_u32  reg_get_trpp_channel_parse_en(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_clear_status (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_clear_status(mt_u32 data);
mt_u32  reg_get_trpp_ch_clear_status(mt_void);
mt_void reg_set_trpp_ch_clear_status_trpp_ch_clr_ok(mt_u8 data);
mt_u8   reg_get_trpp_ch_clear_status_trpp_ch_clr_ok(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_bus_urgent (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_bus_urgent(mt_u32 data);
mt_u32  reg_get_trpp_bus_urgent(mt_void);
mt_void reg_set_trpp_bus_urgent_trpp_urgent_mod(mt_u8 data);
mt_u8   reg_get_trpp_bus_urgent_trpp_urgent_mod(mt_void);
mt_void reg_set_trpp_bus_urgent_trpp_burst_len_mod(mt_u8 data);
mt_u8   reg_get_trpp_bus_urgent_trpp_burst_len_mod(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_channel_record_en (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_channel_record_en(mt_u32 data);
mt_u32  reg_get_trpp_channel_record_en(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt1_4 (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt1_4(mt_u32 data);
mt_u32  reg_get_trpp_sc_index_flt1_4(mt_void);
mt_void reg_set_trpp_sc_index_flt1_4_trpp_sc_idx_flt1(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt1_4_trpp_sc_idx_flt1(mt_void);
mt_void reg_set_trpp_sc_index_flt1_4_trpp_sc_idx_flt2(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt1_4_trpp_sc_idx_flt2(mt_void);
mt_void reg_set_trpp_sc_index_flt1_4_trpp_sc_idx_flt3(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt1_4_trpp_sc_idx_flt3(mt_void);
mt_void reg_set_trpp_sc_index_flt1_4_trpp_sc_idx_flt4(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt1_4_trpp_sc_idx_flt4(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt5_8 (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt5_8(mt_u32 data);
mt_u32  reg_get_trpp_sc_index_flt5_8(mt_void);
mt_void reg_set_trpp_sc_index_flt5_8_trpp_sc_idx_flt5(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt5_8_trpp_sc_idx_flt5(mt_void);
mt_void reg_set_trpp_sc_index_flt5_8_trpp_sc_idx_flt6(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt5_8_trpp_sc_idx_flt6(mt_void);
mt_void reg_set_trpp_sc_index_flt5_8_trpp_sc_idx_flt7(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt5_8_trpp_sc_idx_flt7(mt_void);
mt_void reg_set_trpp_sc_index_flt5_8_trpp_sc_idx_flt8(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt5_8_trpp_sc_idx_flt8(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt9_10 (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt9_10(mt_u32 data);
mt_u32  reg_get_trpp_sc_index_flt9_10(mt_void);
mt_void reg_set_trpp_sc_index_flt9_10_trpp_sc_idx_flt9_l(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt9_10_trpp_sc_idx_flt9_l(mt_void);
mt_void reg_set_trpp_sc_index_flt9_10_trpp_sc_idx_flt9_h(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt9_10_trpp_sc_idx_flt9_h(mt_void);
mt_void reg_set_trpp_sc_index_flt9_10_trpp_sc_idx_flt10_l(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt9_10_trpp_sc_idx_flt10_l(mt_void);
mt_void reg_set_trpp_sc_index_flt9_10_trpp_sc_idx_flt10_h(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt9_10_trpp_sc_idx_flt10_h(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt11_12 (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt11_12(mt_u32 data);
mt_u32  reg_get_trpp_sc_index_flt11_12(mt_void);
mt_void reg_set_trpp_sc_index_flt11_12_trpp_sc_idx_flt11_l(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt11_12_trpp_sc_idx_flt11_l(mt_void);
mt_void reg_set_trpp_sc_index_flt11_12_trpp_sc_idx_flt11_h(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt11_12_trpp_sc_idx_flt11_h(mt_void);
mt_void reg_set_trpp_sc_index_flt11_12_trpp_sc_idx_flt12_l(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt11_12_trpp_sc_idx_flt12_l(mt_void);
mt_void reg_set_trpp_sc_index_flt11_12_trpp_sc_idx_flt12_h(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt11_12_trpp_sc_idx_flt12_h(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_sc_index_flt0 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_sc_index_flt0(mt_u32 data);
mt_u32  reg_get_trpp_sc_index_flt0(mt_void);
mt_void reg_set_trpp_sc_index_flt0_trpp_sc_idx_byte31(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt0_trpp_sc_idx_byte31(mt_void);
mt_void reg_set_trpp_sc_index_flt0_trpp_sc_idx_byte32(mt_u8 data);
mt_u8   reg_get_trpp_sc_index_flt0_trpp_sc_idx_byte32(mt_void);

mt_void reg_set_trpp_mode(mt_u32 data);
mt_u32  reg_get_trpp_mode(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_esbuf_ch (read/write)                                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_esbuf_ch(mt_u32 data);
mt_u32  reg_get_trpp_esbuf_ch(mt_void);
mt_void reg_set_trpp_esbuf_ch_trpp_esbufwp0_chsel(mt_u8 data);
mt_u8   reg_get_trpp_esbuf_ch_trpp_esbufwp0_chsel(mt_void);
mt_void reg_set_trpp_esbuf_ch_trpp_esbufwp1_chsel(mt_u8 data);
mt_u8   reg_get_trpp_esbuf_ch_trpp_esbufwp1_chsel(mt_void);
mt_void reg_set_trpp_esbuf_ch_trpp_esbufwp2_chsel(mt_u8 data);
mt_u8   reg_get_trpp_esbuf_ch_trpp_esbufwp2_chsel(mt_void);
mt_void reg_set_trpp_esbuf_ch_trpp_esbufwp3_chsel(mt_u8 data);
mt_u8   reg_get_trpp_esbuf_ch_trpp_esbufwp3_chsel(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_property (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_property(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_property(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_es_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_es_mode(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_pusi_detect(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_pusi_detect(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_pusi_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_pusi_mode(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_strid_mod(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_strid_mod(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_pusi_mode2(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_pusi_mode2(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_time_info(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_time_info(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_dscrpt_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_dscrpt_en(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_pes_head_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_pes_head_en(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_fsc_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_fsc_en(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_stream_id(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_stream_id(mt_u8 index);
mt_void reg_set_trpp_ch_property_trpp_ch_str_id_msk(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_property_trpp_ch_str_id_msk(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_parse_set (read/write)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_parse_set(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_parse_set(mt_u8 index);
mt_void reg_set_trpp_ch_parse_set_trpp_ch_fsc_cp_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_fsc_cp_en(mt_u8 index);
mt_void reg_set_trpp_ch_parse_set_trpp_ch_sh_detect(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_sh_detect(mt_u8 index);
mt_void reg_set_trpp_ch_parse_set_trpp_ch_sh_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_sh_en(mt_u8 index);
mt_void reg_set_trpp_ch_parse_set_trpp_ch_pes_len_mod(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_pes_len_mod(mt_u8 index);
mt_void reg_set_trpp_ch_parse_set_trpp_ch_fsc_nbytes(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_fsc_nbytes(mt_u8 index);
mt_void reg_set_trpp_ch_parse_set_trpp_ch_insrt_nbytes(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_parse_set_trpp_ch_insrt_nbytes(mt_u8 index);
mt_void reg_set_trpp_ch_parse_set_trpp_ch_int_nbytes(mt_u8 index, mt_u16 data);
mt_u16  reg_get_trpp_ch_parse_set_trpp_ch_int_nbytes(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_start_code1 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_start_code1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_start_code1(mt_u8 index);
mt_void reg_set_trpp_ch_start_code1_trpp_ch_fsc_31(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_start_code1_trpp_ch_fsc_31(mt_u8 index);
mt_void reg_set_trpp_ch_start_code1_trpp_ch_fsc_32(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_start_code1_trpp_ch_fsc_32(mt_u8 index);
mt_void reg_set_trpp_ch_start_code1_trpp_ch_fsc_41(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_start_code1_trpp_ch_fsc_41(mt_u8 index);
mt_void reg_set_trpp_ch_start_code1_trpp_ch_fsc_42(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_start_code1_trpp_ch_fsc_42(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_frm_start_code_m1 (read/write)                        */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_frm_start_code_m1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_frm_start_code_m1(mt_u8 index);
mt_void reg_set_trpp_ch_frm_start_code_m1_trpp_ch_fscm_31(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_frm_start_code_m1_trpp_ch_fscm_31(mt_u8 index);
mt_void reg_set_trpp_ch_frm_start_code_m1_trpp_ch_fscm_32(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_frm_start_code_m1_trpp_ch_fscm_32(mt_u8 index);
mt_void reg_set_trpp_ch_frm_start_code_m1_trpp_ch_fscm_41(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_frm_start_code_m1_trpp_ch_fscm_41(mt_u8 index);
mt_void reg_set_trpp_ch_frm_start_code_m1_trpp_ch_fscm_42(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_frm_start_code_m1_trpp_ch_fscm_42(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_start_code2 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_start_code2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_start_code2(mt_u8 index);
mt_void reg_set_trpp_ch_start_code2_trpp_ch_fsc_51(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_start_code2_trpp_ch_fsc_51(mt_u8 index);
mt_void reg_set_trpp_ch_start_code2_trpp_ch_fsc_52(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_start_code2_trpp_ch_fsc_52(mt_u8 index);
mt_void reg_set_trpp_ch_start_code2_trpp_ch_fsc_61(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_start_code2_trpp_ch_fsc_61(mt_u8 index);
mt_void reg_set_trpp_ch_start_code2_trpp_ch_fsc_62(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_start_code2_trpp_ch_fsc_62(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_frm_start_code_m2 (read/write)                        */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_frm_start_code_m2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_frm_start_code_m2(mt_u8 index);
mt_void reg_set_trpp_ch_frm_start_code_m2_trpp_ch_fscm_51(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_frm_start_code_m2_trpp_ch_fscm_51(mt_u8 index);
mt_void reg_set_trpp_ch_frm_start_code_m2_trpp_ch_fscm_52(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_frm_start_code_m2_trpp_ch_fscm_52(mt_u8 index);
mt_void reg_set_trpp_ch_frm_start_code_m2_trpp_ch_fscm_61(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_frm_start_code_m2_trpp_ch_fscm_61(mt_u8 index);
mt_void reg_set_trpp_ch_frm_start_code_m2_trpp_ch_fscm_62(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_frm_start_code_m2_trpp_ch_fscm_62(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_dscrpt_start_addr (read/write)                        */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_dscrpt_start_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_dscrpt_start_addr(mt_u8 index);
mt_void reg_set_trpp_ch_dscrpt_start_addr_trpp_ch_data_mem_th(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_dscrpt_start_addr_trpp_ch_data_mem_th(mt_u8 index);
mt_void reg_set_trpp_ch_dscrpt_start_addr_trpp_ch_data_saddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_dscrpt_start_addr_trpp_ch_data_saddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_data_start_addr (read/write)                          */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_data_start_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_data_start_addr(mt_u8 index);
mt_void reg_set_trpp_ch_data_start_addr_trpp_ch_data_mem_th(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_data_start_addr_trpp_ch_data_mem_th(mt_u8 index);
mt_void reg_set_trpp_ch_data_start_addr_trpp_ch_data_saddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_data_start_addr_trpp_ch_data_saddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_dscrpt_end_addr (read/write)                          */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_dscrpt_end_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_dscrpt_end_addr(mt_u8 index);
mt_void reg_set_trpp_ch_dscrpt_end_addr_trpp_ch_data_eaddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_dscrpt_end_addr_trpp_ch_data_eaddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_data_end_addr (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_data_end_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_data_end_addr(mt_u8 index);
mt_void reg_set_trpp_ch_data_end_addr_trpp_ch_data_eaddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_data_end_addr_trpp_ch_data_eaddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_dscrpt_rd_addr (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_dscrpt_rd_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_dscrpt_rd_addr(mt_u8 index);
mt_void reg_set_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_dscrpt_rd_addr_trpp_ch_dscrpt_raddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_data_rd_addr (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_data_rd_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_data_rd_addr(mt_u8 index);
mt_void reg_set_trpp_ch_data_rd_addr_trpp_ch_data_raddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_data_rd_addr_trpp_ch_data_raddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_dscrpt_wr_addr (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_dscrpt_wr_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_dscrpt_wr_addr(mt_u8 index);
mt_void reg_set_trpp_ch_dscrpt_wr_addr_trpp_ch_dscrpt_waddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_dscrpt_wr_addr_trpp_ch_dscrpt_waddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_data_wr_addr (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_data_wr_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_data_wr_addr(mt_u8 index);
mt_void reg_set_trpp_ch_data_wr_addr_trpp_ch_data_waddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_data_wr_addr_trpp_ch_data_waddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info1 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info1(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info1_trpp_ch1_reserved(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info1_trpp_ch1_reserved(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info1_trpp_ch1_ini_flag_par(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch1_ini_info1_trpp_ch1_ini_flag_par(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info2 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info2(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info2_trpp_ch1_dts(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info2_trpp_ch1_dts(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info3 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info3(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info3_trpp_ch1_pts(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info3_trpp_ch1_pts(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info4 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info4(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info4(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info4_trpp_ch1_reserved(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info4_trpp_ch1_reserved(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info5 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info5(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info5(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info5_trpp_ch1_reserved(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info5_trpp_ch1_reserved(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info6 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info6(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info6(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info6_trpp_ch1_frame_cnt(mt_u8 index, mt_u16 data);
mt_u16  reg_get_trpp_ch1_ini_info6_trpp_ch1_frame_cnt(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info7 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info7(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info7(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_reserved(mt_u8 index, mt_u16 data);
mt_u16  reg_get_trpp_ch1_ini_info7_trpp_ch1_reserved(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_chnum(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch1_ini_info7_trpp_ch1_chnum(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_gotstrid(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch1_ini_info7_trpp_ch1_gotstrid(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_discard_flag(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch1_ini_info7_trpp_ch1_discard_flag(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info7_trpp_ch1_reserve(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch1_ini_info7_trpp_ch1_reserve(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info8 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info8(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info8(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info8_trpp_ch1_reserved(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info8_trpp_ch1_reserved(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch1_ini_info9 (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch1_ini_info9(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info9(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info9_trpp_ch1_pes_data_cnt(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch1_ini_info9_trpp_ch1_pes_data_cnt(mt_u8 index);
mt_void reg_set_trpp_ch1_ini_info9_trpp_ch1_reserved(mt_u8 index, mt_u16 data);
mt_u16  reg_get_trpp_ch1_ini_info9_trpp_ch1_reserved(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_set (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_set(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_rec_set(mt_u8 index);
mt_void reg_set_trpp_ch_rec_set_trpp_ch_rec_cnt_th(mt_u8 index, mt_u16 data);
mt_u16  reg_get_trpp_ch_rec_set_trpp_ch_rec_cnt_th(mt_u8 index);
mt_void reg_set_trpp_ch_rec_set_trpp_ch_rec_sel(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_rec_set_trpp_ch_rec_sel(mt_u8 index);
mt_void reg_set_trpp_ch_rec_set_trpp_ch_rec_mod(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_rec_set_trpp_ch_rec_mod(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_start_addr (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_start_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_rec_start_addr(mt_u8 index);
mt_void reg_set_trpp_ch_rec_start_addr_trpp_ch_rec_mem_th(mt_u8 index, mt_u8 data);
mt_u8   reg_get_trpp_ch_rec_start_addr_trpp_ch_rec_mem_th(mt_u8 index);
mt_void reg_set_trpp_ch_rec_start_addr_trpp_ch_rec_saddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_rec_start_addr_trpp_ch_rec_saddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_end_addr (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_end_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_rec_end_addr(mt_u8 index);
mt_void reg_set_trpp_ch_rec_end_addr_trpp_ch_rec_eaddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_rec_end_addr_trpp_ch_rec_eaddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_rd_addr (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_rd_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_rec_rd_addr(mt_u8 index);
mt_void reg_set_trpp_ch_rec_rd_addr_trpp_ch_rec_raddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_rec_rd_addr_trpp_ch_rec_raddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_rec_wr_addr (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_rec_wr_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_rec_wr_addr(mt_u8 index);
mt_void reg_set_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_rec_wr_addr_trpp_ch_rec_waddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch11_ini_info1 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch11_ini_info1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_ini_info1(mt_u8 index);
mt_void reg_set_trpp_ch11_ini_info1_trpp_ch11_reserved(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_ini_info1_trpp_ch11_reserved(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch11_ini_info2 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch11_ini_info2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_ini_info2(mt_u8 index);
mt_void reg_set_trpp_ch11_ini_info2_trpp_ch11_reserved(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_ini_info2_trpp_ch11_reserved(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch11_ini_info3 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch11_ini_info3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_ini_info3(mt_u8 index);
mt_void reg_set_trpp_ch11_ini_info3_trpp_ch11_reserved(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_ini_info3_trpp_ch11_reserved(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_ts_sn (read/write)                                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_ts_sn(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_ts_sn(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx link node register (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch11_lln_set0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_lln_set0(mt_u8 index);

mt_void reg_set_trpp_ch11_lln_set1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_lln_set1(mt_u8 index);

mt_void reg_set_trpp_ch11_lln_set2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_lln_set2(mt_u8 index);

mt_void reg_set_trpp_ch11_lln_set3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_lln_set3(mt_u8 index);

mt_void reg_set_trpp_ch11_lln_idx_set0(mt_u8 rec_index, mt_u8 id_index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_lln_idx_set0(mt_u8 rec_index, mt_u8 id_index);

mt_void reg_set_trpp_ch11_lln_idx_set1(mt_u8 rec_index, mt_u8 id_index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_lln_idx_set1(mt_u8 rec_index, mt_u8 id_index);

mt_void reg_set_trpp_ch11_lln_full_0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_lln_full_0(mt_u8 index);

mt_void reg_set_trpp_ch11_lln_full_1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_lln_full_1(mt_u8 index);

mt_void reg_set_trpp_ch11_lln_rd_byte_set(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch11_lln_data_byte_num(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_mode (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_mode(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_mode(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_enable (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_enable(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_enable(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_start_addr (read/write)                           */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_start_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_start_addr(mt_u8 index);
mt_void reg_set_trpp_ch_idx_start_addr_trpp_ch_idx_saddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_start_addr_trpp_ch_idx_saddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_end_addr (read/write)                             */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_end_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_end_addr(mt_u8 index);
mt_void reg_set_trpp_ch_idx_end_addr_trpp_ch_idx_eaddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_end_addr_trpp_ch_idx_eaddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_rd_addr (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_rd_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_rd_addr(mt_u8 index);
mt_void reg_set_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_rd_addr_trpp_ch_idx_raddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch_idx_wr_addr (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch_idx_wr_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_wr_addr(mt_u8 index);
mt_void reg_set_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch_idx_wr_addr_trpp_ch_idx_waddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch15_ini_info1 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch15_ini_info1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_ini_info1(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch15_ini_info2 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch15_ini_info2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_ini_info2(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch15_ini_info3 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch15_ini_info3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_ini_info3(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_ch15_ini_info4 (read/write)                              */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_ch15_ini_info4(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_ini_info4(mt_u8 index);

mt_void reg_set_trpp_ch15_ini_info5(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_ini_info5(mt_u8 index);
mt_void reg_set_trpp_ch15_ini_info6(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_ini_info6(mt_u8 index);
mt_void reg_set_trpp_ch15_ini_info7(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_ini_info7(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register reg_dmx_trpp_ch15_sc_flt_set0~4 (read/write)                                      */
/*----------------------------------------------------------------------------*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_void reg_set_trpp_ch15_sc_flt_set0(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_sc_flt_set0(mt_u8 index);
mt_void reg_set_trpp_ch15_sc_flt_set1(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_sc_flt_set1(mt_u8 index);
mt_void reg_set_trpp_ch15_sc_flt_set2(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_sc_flt_set2(mt_u8 index);
mt_void reg_set_trpp_ch15_sc_flt_set3(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_sc_flt_set3(mt_u8 index);
mt_void reg_set_trpp_ch15_sc_flt_set4(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_ch15_sc_flt_set4(mt_u8 index);

#endif

/*----------------------------------------------------------------------------*/
/* register dmx_ds_int_mask (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ds_int_mask(mt_u32 data);
mt_u32  reg_get_ds_int_mask(mt_void);
mt_void reg_set_ds_int_mask_cw_unvld_im(mt_u8 data);
mt_u8   reg_get_ds_int_mask_cw_unvld_im(mt_void);
mt_void reg_set_ds_int_mask_ts_err1_im(mt_u8 data);
mt_u8   reg_get_ds_int_mask_ts_err1_im(mt_void);
mt_void reg_set_ds_int_mask_pes_err1_im(mt_u8 data);
mt_u8   reg_get_ds_int_mask_pes_err1_im(mt_void);
mt_void reg_set_ds_int_mask_pes_err2_im(mt_u8 data);
mt_u8   reg_get_ds_int_mask_pes_err2_im(mt_void);
mt_void reg_set_ds_int_mask_pes_err3_im(mt_u8 data);
mt_u8   reg_get_ds_int_mask_pes_err3_im(mt_void);
mt_void reg_set_ds_int_mask_pes_err4_im(mt_u8 data);
mt_u8   reg_get_ds_int_mask_pes_err4_im(mt_void);
mt_void reg_set_ds_int_mask_pes_err5_im(mt_u8 data);
mt_u8   reg_get_ds_int_mask_pes_err5_im(mt_void);
mt_void reg_set_ds_int_mask_pes_err6_im(mt_u8 data);
mt_u8   reg_get_ds_int_mask_pes_err6_im(mt_void);
mt_void reg_set_ds_int_mask_pes_err7_im(mt_u8 data);
mt_u8   reg_get_ds_int_mask_pes_err7_im(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ds_int_edge (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ds_int_edge(mt_u32 data);
mt_u32  reg_get_ds_int_edge(mt_void);
mt_void reg_set_ds_int_edge_cw_unvld_iedge(mt_u8 data);
mt_u8   reg_get_ds_int_edge_cw_unvld_iedge(mt_void);
mt_void reg_set_ds_int_edge_ts_err1_iedge(mt_u8 data);
mt_u8   reg_get_ds_int_edge_ts_err1_iedge(mt_void);
mt_void reg_set_ds_int_edge_pes_err1_iedge(mt_u8 data);
mt_u8   reg_get_ds_int_edge_pes_err1_iedge(mt_void);
mt_void reg_set_ds_int_edge_pes_err2_iedge(mt_u8 data);
mt_u8   reg_get_ds_int_edge_pes_err2_iedge(mt_void);
mt_void reg_set_ds_int_edge_pes_err3_iedge(mt_u8 data);
mt_u8   reg_get_ds_int_edge_pes_err3_iedge(mt_void);
mt_void reg_set_ds_int_edge_pes_err4_iedge(mt_u8 data);
mt_u8   reg_get_ds_int_edge_pes_err4_iedge(mt_void);
mt_void reg_set_ds_int_edge_pes_err5_iedge(mt_u8 data);
mt_u8   reg_get_ds_int_edge_pes_err5_iedge(mt_void);
mt_void reg_set_ds_int_edge_pes_err6_iedge(mt_u8 data);
mt_u8   reg_get_ds_int_edge_pes_err6_iedge(mt_void);
mt_void reg_set_ds_int_edge_pes_err7_iedge(mt_u8 data);
mt_u8   reg_get_ds_int_edge_pes_err7_iedge(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ds_int_clr (read/write)                                       */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ds_int_clr(mt_u32 data);
mt_u32  reg_get_ds_int_clr(mt_void);
mt_void reg_set_ds_int_clr_ds_int_clr(mt_u16 data);
mt_u16  reg_get_ds_int_clr_ds_int_clr(mt_void);
mt_void reg_set_ds_int_clr_ds_int_rd(mt_u8 data);
mt_u8   reg_get_ds_int_clr_ds_int_rd(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_ds_int_state (read/write)                                     */
/*----------------------------------------------------------------------------*/
mt_void reg_set_ds_int_state(mt_u32 data);
mt_u32  reg_get_ds_int_state(mt_void);
mt_void reg_set_ds_int_state_cw_unvld_ista(mt_u8 data);
mt_u8   reg_get_ds_int_state_cw_unvld_ista(mt_void);
mt_void reg_set_ds_int_state_ts_err1_ista(mt_u8 data);
mt_u8   reg_get_ds_int_state_ts_err1_ista(mt_void);
mt_void reg_set_ds_int_state_pes_err1_ista(mt_u8 data);
mt_u8   reg_get_ds_int_state_pes_err1_ista(mt_void);
mt_void reg_set_ds_int_state_pes_err2_ista(mt_u8 data);
mt_u8   reg_get_ds_int_state_pes_err2_ista(mt_void);
mt_void reg_set_ds_int_state_pes_err3_ista(mt_u8 data);
mt_u8   reg_get_ds_int_state_pes_err3_ista(mt_void);
mt_void reg_set_ds_int_state_pes_err4_ista(mt_u8 data);
mt_u8   reg_get_ds_int_state_pes_err4_ista(mt_void);
mt_void reg_set_ds_int_state_pes_err5_ista(mt_u8 data);
mt_u8   reg_get_ds_int_state_pes_err5_ista(mt_void);
mt_void reg_set_ds_int_state_pes_err6_ista(mt_u8 data);
mt_u8   reg_get_ds_int_state_pes_err6_ista(mt_void);
mt_void reg_set_ds_int_state_pes_err7_ista(mt_u8 data);
mt_u8   reg_get_ds_int_state_pes_err7_ista(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp_int_mask (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp_int_mask(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_int_mask(mt_u8 index);
mt_void reg_set_trpp_int_edge(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_int_edge(mt_u8 index);
mt_void reg_set_trpp_int_clr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_int_clr(mt_u8 index);
mt_void reg_set_trpp_int_state(mt_u8 index, mt_u32 data);
mt_u32  reg_get_trpp_int_state(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp0_int_mask (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp0_int_mask(mt_u32 data);
mt_u32  reg_get_trpp0_int_mask(mt_void);
mt_void reg_set_trpp0_int_mask_ch0_crc_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch0_crc_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch0_str_id_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch0_str_id_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch0_pes_wr_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch0_pes_wr_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch0_ts_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch0_ts_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch0_pes_sc_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch0_pes_sc_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch0_pes_data_cnt_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch0_pes_data_cnt_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch1_crc_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch1_crc_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch1_str_id_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch1_str_id_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch1_pes_wr_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch1_pes_wr_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch1_ts_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch1_ts_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch1_pes_sc_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch1_pes_sc_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch1_pes_data_cnt_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch1_pes_data_cnt_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch2_crc_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch2_crc_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch2_str_id_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch2_str_id_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch2_pes_wr_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch2_pes_wr_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch2_ts_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch2_ts_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch2_pes_sc_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch2_pes_sc_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch2_pes_data_cnt_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch2_pes_data_cnt_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch3_crc_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch3_crc_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch3_str_id_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch3_str_id_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch3_pes_wr_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch3_pes_wr_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch3_ts_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch3_ts_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch3_pes_sc_err_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch3_pes_sc_err_im(mt_void);
mt_void reg_set_trpp0_int_mask_ch3_pes_data_cnt_im(mt_u8 data);
mt_u8   reg_get_trpp0_int_mask_ch3_pes_data_cnt_im(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp0_int_edge (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp0_int_edge(mt_u32 data);
mt_u32  reg_get_trpp0_int_edge(mt_void);
mt_void reg_set_trpp0_int_edge_ch0_crc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch0_crc_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch0_str_id_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch0_str_id_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch0_pes_wr_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch0_pes_wr_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch0_ts_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch0_ts_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch0_pes_sc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch0_pes_sc_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch0_pes_data_cnt_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch0_pes_data_cnt_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch1_crc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch1_crc_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch1_str_id_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch1_str_id_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch1_pes_wr_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch1_pes_wr_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch1_ts_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch1_ts_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch1_pes_sc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch1_pes_sc_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch1_pes_data_cnt_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch1_pes_data_cnt_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch2_crc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch2_crc_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch2_str_id_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch2_str_id_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch2_pes_wr_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch2_pes_wr_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch2_ts_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch2_ts_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch2_pes_sc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch2_pes_sc_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch2_pes_data_cnt_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch2_pes_data_cnt_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch3_crc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch3_crc_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch3_str_id_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch3_str_id_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch3_pes_wr_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch3_pes_wr_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch3_ts_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch3_ts_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch3_pes_sc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch3_pes_sc_err_iedge(mt_void);
mt_void reg_set_trpp0_int_edge_ch3_pes_data_cnt_iedge(mt_u8 data);
mt_u8   reg_get_trpp0_int_edge_ch3_pes_data_cnt_iedge(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp0_int_clr (read/write)                                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp0_int_clr(mt_u32 data);
mt_u32  reg_get_trpp0_int_clr(mt_void);
mt_void reg_set_trpp0_int_clr_trpp0_int_clr(mt_u32 data);
mt_u32  reg_get_trpp0_int_clr_trpp0_int_clr(mt_void);
mt_void reg_set_trpp0_int_clr_trpp0_int_rd(mt_u8 data);
mt_u8   reg_get_trpp0_int_clr_trpp0_int_rd(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp0_int_state (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp0_int_state(mt_u32 data);
mt_u32  reg_get_trpp0_int_state(mt_void);
mt_void reg_set_trpp0_int_state_ch0_crc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch0_crc_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch0_str_id_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch0_str_id_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch0_pes_wr_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch0_pes_wr_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch0_ts_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch0_ts_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch0_pes_sc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch0_pes_sc_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch0_pes_data_cnt_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch0_pes_data_cnt_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch1_crc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch1_crc_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch1_str_id_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch1_str_id_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch1_pes_wr_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch1_pes_wr_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch1_ts_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch1_ts_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch1_pes_sc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch1_pes_sc_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch1_pes_data_cnt_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch1_pes_data_cnt_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch2_crc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch2_crc_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch2_str_id_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch2_str_id_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch2_pes_wr_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch2_pes_wr_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch2_ts_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch2_ts_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch2_pes_sc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch2_pes_sc_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch2_pes_data_cnt_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch2_pes_data_cnt_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch3_crc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch3_crc_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch3_str_id_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch3_str_id_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch3_pes_wr_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch3_pes_wr_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch3_ts_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch3_ts_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch3_pes_sc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch3_pes_sc_err_ista(mt_void);
mt_void reg_set_trpp0_int_state_ch3_pes_data_cnt_ista(mt_u8 data);
mt_u8   reg_get_trpp0_int_state_ch3_pes_data_cnt_ista(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp1_int_mask (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp1_int_mask(mt_u32 data);
mt_u32  reg_get_trpp1_int_mask(mt_void);
mt_void reg_set_trpp1_int_mask_ch4_crc_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch4_crc_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch4_str_id_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch4_str_id_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch4_pes_wr_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch4_pes_wr_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch4_ts_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch4_ts_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch4_pes_sc_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch4_pes_sc_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch4_pes_data_cnt_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch4_pes_data_cnt_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch5_crc_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch5_crc_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch5_str_id_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch5_str_id_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch5_pes_wr_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch5_pes_wr_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch5_ts_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch5_ts_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch5_pes_sc_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch5_pes_sc_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch5_pes_data_cnt_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch5_pes_data_cnt_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch6_crc_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch6_crc_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch6_str_id_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch6_str_id_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch6_pes_wr_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch6_pes_wr_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch6_ts_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch6_ts_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch6_pes_sc_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch6_pes_sc_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch6_pes_data_cnt_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch6_pes_data_cnt_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch7_crc_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch7_crc_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch7_str_id_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch7_str_id_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch7_pes_wr_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch7_pes_wr_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch7_ts_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch7_ts_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch7_pes_sc_err_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch7_pes_sc_err_im(mt_void);
mt_void reg_set_trpp1_int_mask_ch7_pes_data_cnt_im(mt_u8 data);
mt_u8   reg_get_trpp1_int_mask_ch7_pes_data_cnt_im(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp1_int_edge (read/write)                                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp1_int_edge(mt_u32 data);
mt_u32  reg_get_trpp1_int_edge(mt_void);
mt_void reg_set_trpp1_int_edge_ch4_crc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch4_crc_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch4_str_id_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch4_str_id_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch4_pes_wr_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch4_pes_wr_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch4_ts_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch4_ts_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch4_pes_sc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch4_pes_sc_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch4_pes_data_cnt_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch4_pes_data_cnt_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch5_crc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch5_crc_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch5_str_id_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch5_str_id_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch5_pes_wr_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch5_pes_wr_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch5_ts_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch5_ts_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch5_pes_sc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch5_pes_sc_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch5_pes_data_cnt_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch5_pes_data_cnt_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch6_crc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch6_crc_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch6_str_id_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch6_str_id_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch6_pes_wr_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch6_pes_wr_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch6_ts_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch6_ts_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch6_pes_sc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch6_pes_sc_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch6_pes_data_cnt_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch6_pes_data_cnt_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch7_crc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch7_crc_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch7_str_id_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch7_str_id_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch7_pes_wr_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch7_pes_wr_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch7_ts_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch7_ts_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch7_pes_sc_err_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch7_pes_sc_err_iedge(mt_void);
mt_void reg_set_trpp1_int_edge_ch7_pes_data_cnt_iedge(mt_u8 data);
mt_u8   reg_get_trpp1_int_edge_ch7_pes_data_cnt_iedge(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp1_int_clr (read/write)                                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp1_int_clr(mt_u32 data);
mt_u32  reg_get_trpp1_int_clr(mt_void);
mt_void reg_set_trpp1_int_clr_trpp1_int_clr(mt_u32 data);
mt_u32  reg_get_trpp1_int_clr_trpp1_int_clr(mt_void);
mt_void reg_set_trpp1_int_clr_trpp1_int_rd(mt_u8 data);
mt_u8   reg_get_trpp1_int_clr_trpp1_int_rd(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_trpp1_int_state (read/write)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_trpp1_int_state(mt_u32 data);
mt_u32  reg_get_trpp1_int_state(mt_void);
mt_void reg_set_trpp1_int_state_ch4_crc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch4_crc_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch4_str_id_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch4_str_id_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch4_pes_wr_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch4_pes_wr_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch4_ts_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch4_ts_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch4_pes_sc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch4_pes_sc_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch4_pes_data_cnt_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch4_pes_data_cnt_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch5_crc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch5_crc_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch5_str_id_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch5_str_id_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch5_pes_wr_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch5_pes_wr_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch5_ts_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch5_ts_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch5_pes_sc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch5_pes_sc_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch5_pes_data_cnt_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch5_pes_data_cnt_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch6_crc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch6_crc_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch6_str_id_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch6_str_id_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch6_pes_wr_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch6_pes_wr_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch6_ts_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch6_ts_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch6_pes_sc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch6_pes_sc_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch6_pes_data_cnt_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch6_pes_data_cnt_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch7_crc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch7_crc_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch7_str_id_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch7_str_id_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch7_pes_wr_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch7_pes_wr_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch7_ts_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch7_ts_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch7_pes_sc_err_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch7_pes_sc_err_ista(mt_void);
mt_void reg_set_trpp1_int_state_ch7_pes_data_cnt_ista(mt_u8 data);
mt_u8   reg_get_trpp1_int_state_ch7_pes_data_cnt_ista(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_pcr_fifo_cnt function (read)                                  */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_pcr_fifo_cnt(mt_u32 data);
mt_u32  reg_get_dmx_pcr_fifo_cnt(mt_void);
mt_void reg_set_dmx_pcr_fifo_cnt_pcr_fifo_cnt(mt_u32 data);
mt_u32  reg_get_dmx_pcr_fifo_cnt_pcr_fifo_cnt(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_pcr_value_low function (read)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_pcr_value_low(mt_u32 data);
mt_u32  reg_get_dmx_pcr_value_low(mt_void);
mt_void reg_set_dmx_pcr_value_low_pcr_value_low(mt_u32 data);
mt_u32  reg_get_dmx_pcr_value_low_pcr_value_low(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_pcr_value_high function (read)                                */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_pcr_value_high(mt_u32 data);
mt_u32  reg_get_dmx_pcr_value_high(mt_void);
mt_void reg_set_dmx_pcr_value_high_pcr_value_high(mt_u32 data);
mt_u32  reg_get_dmx_pcr_value_high_pcr_value_high(mt_void);
mt_void reg_set_dmx_pcr_value_high_pcr_value_ch(mt_u32 data);
mt_u32  reg_get_dmx_pcr_value_high_pcr_value_ch(mt_void);
mt_void reg_set_dmx_pcr_value_high_dis_indicator(mt_u32 data);
mt_u32  reg_get_dmx_pcr_value_high_dis_indicator(mt_void);

/*----------------------------------------------------------------------------*/
/* register dmx_hwcg_mode function (read/write)                    			  */
/*----------------------------------------------------------------------------*/

void reg_set_dmx_hwcg_mode(mt_u32 data);
mt_u32  reg_get_dmx_hwcg_mode(void);

/*----------------------------------------------------------------------------*/
/* register dmx_bus_debug_chn_staddr function (read/write)                    */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_bus_debug_chn_staddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_bus_debug_chn_staddr(mt_u8 index);
mt_void reg_set_dmx_bus_debug_chn_staddr_bus_debug_chn_staddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_bus_debug_chn_staddr_bus_debug_chn_staddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_bus_debug_chn_endaddr function (read/write)                   */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_bus_debug_chn_endaddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_bus_debug_chn_endaddr(mt_u8 index);
mt_void reg_set_dmx_bus_debug_chn_endaddr_bus_debug_chn_endaddr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_bus_debug_chn_endaddr_bus_debug_chn_endaddr(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_bus_debug_chn_endaddr function (read/write)                   */
/*----------------------------------------------------------------------------*/
mt_u32  reg_get_dmx_bus_debug_hit_addr(mt_void);
mt_u32  reg_get_dmx_bus_debug_hit_addr_dmx_bus_debug_hit_addr(mt_void);

mt_u32  reg_get_dmx_sf_process_sta(mt_void);

/*----------------------------------------------------------------------------*/
/* init function                                                              */
/*----------------------------------------------------------------------------*/
mt_void reg_dmx_init(mt_void);

mt_void reg_set_pvr_int_mask(mt_u32 data);
mt_u32  reg_get_pvr_int_mask(mt_void);
mt_void reg_set_pvr_int_edge(mt_u32 data);
mt_u32 reg_get_pvr_int_edge(mt_void);
mt_void reg_set_pvr_int_clr(mt_u32 data);
mt_u32 reg_get_pvr_int_clr(mt_void);
mt_void reg_set_pvr_int_state(mt_u32 data);
mt_u32 reg_get_pvr_int_state(mt_void);

#if defined(CONFIG_MT_CHIP_SYMPHONY6)
mt_u32  reg_get_pvr1_int_mask(mt_void);
mt_void reg_set_pvr1_int_edge(mt_u32 data);
mt_u32 reg_get_pvr1_int_edge(mt_void);
mt_void reg_set_pvr1_int_clr(mt_u32 data);
mt_u32 reg_get_pvr1_int_clr(mt_void);
mt_void reg_set_pvr1_int_rd(mt_u32 data);
mt_u32 reg_get_pvr1_int_rd(mt_void);
mt_u32 reg_get_pvr1_int_state(mt_void);

mt_void reg_set_pvr2_int_mask(mt_u32 data);
mt_u32  reg_get_pvr2_int_mask(mt_void);
mt_void reg_set_pvr2_int_edge(mt_u32 data);
mt_u32 reg_get_pvr2_int_edge(mt_void);
mt_void reg_set_pvr2_int_clr(mt_u32 data);
mt_u32 reg_get_pvr2_int_clr(mt_void);
mt_void reg_set_pvr2_int_rd(mt_u32 data);
mt_u32 reg_get_pvr2_int_rd(mt_void);
mt_u32 reg_get_pvr2_int_state(mt_void);
#endif

mt_void reg_set_trpp4_int_clr(mt_u32 data);
mt_u32 reg_get_trpp4_int_clr(mt_void);

mt_void reg_set_ts_sample_int_mask(mt_u32 data);
mt_u32  reg_get_ts_sample_int_mask(mt_void);

mt_void reg_set_ts_sample_int_edge(mt_u32 data);
mt_u32  reg_get_ts_sample_int_edge(mt_void);

mt_void reg_set_ts_sample_int_clr(mt_u32 data);
mt_u32 reg_get_ts_sample_int_clr(mt_void);

mt_u32 reg_get_ts_sample_int_state(mt_void);

mt_void reg_set_trpp5_int_mask(mt_u32 data);
mt_void reg_set_trpp5_int_clr(mt_u32 data);
mt_u32  reg_get_trpp5_int_state(mt_void);

mt_void reg_set_trpp10_int_mask(mt_u32 data);
mt_u32  reg_get_trpp10_int_mask(mt_void);
mt_void reg_set_trpp10_int_clr(mt_u32 data);
mt_u32  reg_get_trpp10_int_state(mt_void);

mt_void reg_set_trpp11_int_mask(mt_u32 data);
mt_u32  reg_get_trpp11_int_mask(mt_void);
mt_void reg_set_trpp11_int_clr(mt_u32 data);
mt_u32  reg_get_trpp11_int_state(mt_void);

mt_void reg_set_trpp12_int_mask(mt_u32 data);
mt_u32  reg_get_trpp12_int_mask(mt_void);
mt_void reg_set_trpp12_int_clr(mt_u32 data);
mt_u32  reg_get_trpp12_int_state(mt_void);


mt_void reg_set_dmx_gglb_int_mask(mt_u32 data);
mt_u32 reg_get_dmx_gglb_int_mask(mt_void);
mt_void reg_set_dmx_gglb_int_edge(mt_u32 data);
mt_u32 reg_get_dmx_gglb_int_edge(mt_void);
mt_void reg_set_dmx_gglb_int_clr(mt_u32 data);
mt_u32 reg_get_dmx_gglb_int_clr(mt_void);
mt_void reg_set_dmx_gglb_int_state(mt_u32 data);
mt_u32 reg_get_dmx_gglb_int_state(mt_void);

#if defined(CONFIG_MT_CHIP_SYMPHONY6) 
mt_void reg_set_tsi_ds_chn_tscfg(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_chn_tscfg(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_ds_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_ds_mode(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_scrtag_clr(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_scrtag_clr(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_tsscr_clr_range(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_tsscr_clr_range(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_ts_cwopt1_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_ts_cwopt1_mode(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_pes_cwopt1_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_cwopt1_mode(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_pes_enc_odd_even_eco(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_enc_odd_even_eco(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_pes_enc_mode_eco(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_enc_mode_eco(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_pes_scr_enc_force(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_scr_enc_force(mt_u8 index);
mt_void reg_set_tsi_ds_chn_tscfg_pes_des_key_msb64(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_des_key_msb64(mt_u8 index);

mt_void reg_set_tsi_ds_chn_tscfg_pes_csa2_key_msb64(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_csa2_key_msb64(mt_u8 index);

mt_void reg_set_tsi_ds_chn_tscfg_pes_multi2_key_msb64(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_multi2_key_msb64(mt_u8 index);

mt_void reg_set_tsi_ds_chn_tscfg_pes_gost_sbox_sel(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_gost_sbox_sel(mt_u8 index);


mt_void reg_set_tsi_ds_chn_tscfg_pes_gost_bit_inv(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_chn_tscfg_pes_gost_bit_inv(mt_u8 index);


/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ds_core (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ds_core(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ds_core(mt_u8 index);
mt_void reg_set_tsi_ds_core_ds_core_sel(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_core_ds_core_sel(mt_u8 index);
mt_void reg_set_tsi_ds_core_csa3_opti(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_ds_core_csa3_opti(mt_u8 index);
/*----------------------------------------------------------------------------*/
/* register dmx_tsi_aes_ive (read/write)                                      */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_aes_ive(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_aes_ive(mt_u8 index);
mt_void reg_set_tsi_aes_ive_ivecal_en(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_aes_ive_ivecal_en(mt_u8 index);
mt_void reg_set_tsi_aes_ive_ivecal_mode(mt_u8 index, mt_u8 data);
mt_u8   reg_get_tsi_aes_ive_ivecal_mode(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_disc_mode (read/write)                               */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_disc_mode(mt_u8 index, mt_u32 data);
mt_u32  reg_get_tsi_ades_disc_mode(mt_u8 index);

mt_void reg_set_tsi_ades_disc_mode_disc_mode(mt_u8 index, mt_u8 data);

mt_u8   reg_get_tsi_ades_disc_mode_disc_mode(mt_u8 index);


/*----------------------------------------------------------------------------*/
/* register dmx_tsi_ades_pktmode (read/write)                                 */
/*----------------------------------------------------------------------------*/
mt_void reg_set_tsi_ades_pktmode(mt_u8 index, mt_u32 data);

mt_u32  reg_get_tsi_ades_pktmode(mt_u8 index);

mt_void reg_set_tsi_ades_pktmode_short_pkt_mode(mt_u8 index, mt_u8 data);

mt_u8   reg_get_tsi_ades_pktmode_short_pkt_mode(mt_u8 index);

mt_void reg_set_tsi_ades_pktmode_small_pkt_mode(mt_u8 index, mt_u8 data);

mt_u8   reg_get_tsi_ades_pktmode_small_pkt_mode(mt_u8 index);


/*----------------------------------------------------------------------------*/
/* register dmx_tsi_keyslot_tab (read/write)                                   */
/*----------------------------------------------------------------------------*/
void reg_set_tsi_keyslot_tab(mt_u8 index, mt_u32 data);

mt_u32  reg_get_tsi_keyslot_tab(mt_u8 index);

void reg_set_tsi_keyslot_tab_entry_valid(mt_u8 index, mt_u32 data);

mt_u32  reg_get_tsi_keyslot_tab_entry_valid(mt_u8 index);

void reg_set_tsi_keyslot_tab_even_key_slot_index(mt_u8 index, mt_u32 data);

mt_u32  reg_get_tsi_keyslot_tab_even_key_slot_index(mt_u8 index);

void reg_set_tsi_keyslot_tab_odd_key_slot_index(mt_u8 index, mt_u32 data);

mt_u32  reg_get_tsi_keyslot_tab_odd_key_slot_index(mt_u8 index);

/*----------------------------------------------------------------------------*/
/* register dmx_kt_endian (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void reg_set_dmx_kt_endian(mt_u32 data);

mt_u32  reg_get_dmx_kt_endian(mt_void);

mt_void reg_set_dmx_ds_big_little_endian(mt_u32 data);

mt_u32 reg_get_dmx_ds_big_little_endian(mt_void);

/*
**  debug regs
*/
mt_u32 reg_get_dmx_ds_sechd1_busy(mt_void);

mt_u32 reg_get_dmx_ds_sechd1_status(mt_void);

mt_u32 reg_get_dmx_ds_dbg0_attr_err(mt_void);

mt_u32 reg_get_dmx_ds_dbg1(mt_void);

mt_u32 reg_get_dmx_ds_dbg2(mt_void);

mt_u32 reg_get_dmx_ds_status_rdata0(mt_void);

mt_u32 reg_get_dmx_ds_status_rdata1(mt_void);

mt_u32 reg_get_dmx_ds_status_rdata2(mt_void);

mt_void reg_set_dmx_ds_hwcg_mode(mt_u32 data);

mt_u32 reg_get_dmx_ds_hwcg_mode(mt_void);


mt_void reg_set_dmx_ds_tskey_src_opt(mt_u32 data);

mt_u32 reg_get_dmx_ds_tskey_src_opt(mt_void);


mt_void reg_set_dmx_ds_fw(mt_u32 data);

mt_u32 reg_get_dmx_ds_fw(mt_void);
#endif

void reg_set_tsi_keyslot_tab(mt_u8 index, mt_u32 data);


/*----------------------------------------------------------------------------*/
/* register dmx_kt_endian (read/write)                            */
/*----------------------------------------------------------------------------*/
mt_void  reg_set_dmx_kt_endian(mt_u32 data);
mt_u32  reg_get_dmx_kt_endian(mt_void);
mt_void reg_set_dmx_ds_big_little_endian(mt_u32 data);
mt_u32 reg_get_dmx_ds_big_little_endian(mt_void);

/*-------------------------------------------------*/
/* register dmx_t2mi  (read/write)                 */
/*-------------------------------------------------*/
void reg_set_dmx_t2mi_en(u32 data);
u32  reg_get_dmx_t2mi_en(void);
void reg_set_dmx_t2mi_int_mask(u32 data);
u32  reg_get_dmx_t2mi_int_mask(void);
u32  reg_get_dmx_t2mi_int_state(void);
void reg_set_dmx_t2mi_int_clr(mt_u32 data);
mt_u32 reg_get_dmx_t2mi_int_clr(void);
void reg_set_dmx_t2mi_set1(u32 data);
u32  reg_get_dmx_t2mi_set1(void);
void reg_set_dmx_t2mi_set1_t2mi_pid(u32 data);
u32  reg_get_dmx_t2mi_set1_t2mi_pid(void);
void reg_set_dmx_t2mi_set1_t2mi_plp_id(u32 data);
u32  reg_get_dmx_t2mi_set1_t2mi_plp_id(void);
void reg_set_dmx_t2mi_set1_t2mi_output_ch(u32 data);
u32  reg_get_dmx_t2mi_set1_t2mi_output_ch(void);
void reg_set_dmx_t2mi_set1_t2mi_input_ch(u32 data);
u32  reg_get_dmx_t2mi_set1_t2mi_input_ch(void);
void reg_set_dmx_t2mi_set2(u32 data);
u32  reg_get_dmx_t2mi_set2(void);
void reg_set_dmx_t2mi_set2_t2mi_pusi_det_en(u32 data);
u32  reg_get_dmx_t2mi_set2_t2mi_pusi_det_en(void);
void reg_set_dmx_t2mi_set2_t2mi_swtsi_en(u32 data);
u32  reg_get_dmx_t2mi_set2_t2mi_swtsi_en(void);
void reg_set_dmx_t2mi_set2_t2mi_src_ch(u32 data);
u32  reg_get_dmx_t2mi_set2_t2mi_src_ch(void);

/*----------------------------------------------------------------------------*/
/* register address cross-border                                              */
/*----------------------------------------------------------------------------*/
void reg_set_dmx_debug_trpp_sec_cfg(mt_u32 data);
mt_u32  reg_get_dmx_debug_trpp_sec_cfg(void);
mt_u32  reg_get_dmx_debug_addr_sta(void);
void reg_set_dmx_debug_chx_start_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_debug_chx_start_addr(mt_u8 index);
void reg_set_dmx_debug_chx_end_addr(mt_u8 index, mt_u32 data);
mt_u32  reg_get_dmx_debug_chx_end_addr(mt_u8 index);

mt_u32 reg_dmx_get_clk(void);
void reg_dmx_set_clk(mt_u32 value);

mt_u32 reg_dmx_get_src(void);
void reg_dmx_set_src(mt_u32 value);

/*add for sym6 verify,please fixme*/
#if defined(CONFIG_MT_CHIP_SYMPHONY6)
void reg_set_tsi_ci_set_cfg(mt_u32 value);
mt_u32 reg_get_tsi_ci_set_cfg(void);
void reg_set_tsi_ci_cicam_cfg(mt_u32 value);
mt_u32 reg_get_tsi_ci_cicam_cfg(void);
void reg_set_tsi_ci_lln_num_start_cfg(mt_u32 value);
mt_u32 reg_get_tsi_ci_lln_num_start_cfg(void);
mt_u32 reg_get_tsi_ci_swtsich_cfg(void);
void reg_set_tsi_ci_swtsich_cfg(mt_u32 value);
mt_u32 reg_get_tsi_ci_status(void);

#endif

/*add end*/
#ifdef __cplusplus
}
#endif

#endif /* _DMX_REGS_SYMPHONY_H */

/*----------------------------------------------------------------------------*/
/* end of file                                                                */
/*----------------------------------------------------------------------------*/

