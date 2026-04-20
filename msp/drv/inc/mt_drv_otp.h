/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __INC_MT_DRV_OTP_H__
#define __INC_MT_DRV_OTP_H__

#ifndef MKSTR
#define MKSTR(x) 					#x
#endif

/* OTP Object Names definition */
#define OTP_ProductType				MKSTR(OTP_ProductType)
#define OTP_ChipFamily				MKSTR(OTP_ChipFamily)
#define OTP_ChipGeneration			MKSTR(OTP_ChipGeneration)
#define OTP_DisplayResolution		MKSTR(OTP_DisplayResolution)
#define OTP_SubFeature				MKSTR(OTP_SubFeature)

#define OTP_PackageInfo				MKSTR(OTP_PackageInfo)
#define OTP_SiPDRAMSize				MKSTR(OTP_SiPDRAMSize)
#define OTP_CAVendor				MKSTR(OTP_CAVendor)
#define OTP_Chipset_Version			MKSTR(OTP_Chipset_Version)
#define OTP_Bin_HWIP_Info			MKSTR(OTP_Bin_HWIP_Info)

#define OTP_IPLicense				MKSTR(OTP_IPLicense)

//A1 has no OTP_DolbyEnable
#define OTP_DolbyEnable				MKSTR(OTP_DolbyEnable)
#define OTP_DolbyForceDisable		MKSTR(OTP_DolbyForceDisable)

#define OTP_DRAEnable				MKSTR(OTP_DRAEnable)

//HW Bonding
#define OTP_RVDIS					MKSTR(OTP_RVDIS)
#define OTP_HDR10P_Dis				MKSTR(OTP_HDR10P_Dis)
#define OTP_HDMI_EMP_Dis			MKSTR(OTP_HDMI_EMP_Dis)
#define OTP_HDMIDis					MKSTR(OTP_HDMIDis)
#define OTP_CPCDis					MKSTR(OTP_CPCDis)
#define OTP_ARMCore1Dis				MKSTR(OTP_ARMCore1Dis)
#define OTP_G31_Dis					MKSTR(OTP_G31_Dis)
#define OTP_VP9Dis					MKSTR(OTP_VP9Dis)
#define OTP_USB1_30					MKSTR(OTP_USB1_30)
#define OTP_EMAC_Dis				MKSTR(OTP_EMAC_Dis)
#define OTP_SL_HDR1_Dis				MKSTR(OTP_SL_HDR1_Dis)
#define OTP_SMCDis					MKSTR(OTP_SMCDis)
#define OTP_VDEC10B					MKSTR(OTP_VDEC10B)
#define OTP_HDR10_HLG_Dis			MKSTR(OTP_HDR10_HLG_Dis)
#define OTP_GMAC_Dis				MKSTR(OTP_GMAC_Dis)
#define OTP_DVBS_S2					MKSTR(OTP_DVBS_S2)
#define OTP_DVBS2X					MKSTR(OTP_DVBS2X)
#define OTP_DVBC					MKSTR(OTP_DVBC)
#define OTP_J83B					MKSTR(OTP_J83B)
#define OTP_USB1					MKSTR(OTP_USB1)
#define OTP_USB0					MKSTR(OTP_USB0)
#define OTP_H265					MKSTR(OTP_H265)
#define OTP_Macrovision				MKSTR(OTP_Macrovision)
#define OTP_UHD						MKSTR(OTP_UHD)


#define OTP_FuseMapVersion			MKSTR(OTP_FuseMapVersion)

#define OTP_LotNumber_L				MKSTR(OTP_LotNumber_L)
#define OTP_LotNumber_H				MKSTR(OTP_LotNumber_H)

#define OTP_CA_Chip_ID_L			MKSTR(OTP_CA_Chip_ID_L)
#define OTP_CA_Chip_ID_H			MKSTR(OTP_CA_Chip_ID_H)

#define OTP_APCPUClkCfg				MKSTR(OTP_APCPUClkCfg)

#define OTP_VDAC_CalibrationStatus	MKSTR(OTP_VDAC_CalibrationStatus)
#define OTP_VDAC_V1					MKSTR(OTP_VDAC_V1)
#define OTP_VDAC_V3					MKSTR(OTP_VDAC_V3)

#define OTP_ADAC_PackageType		MKSTR(OTP_ADAC_PackageType)
#define OTP_ADAC_Avddscr_Voltage	MKSTR(OTP_ADAC_Avddscr_Voltage)

#define OTP_HDCP_Key_RAM_RD_Dis		MKSTR(OTP_HDCP_Key_RAM_RD_Dis)

#define OTP_TEE_EN					MKSTR(OTP_TEE_EN)

#define OTP_RINGO_0					MKSTR(OTP_RINGO_0)
#define OTP_RINGO_1					MKSTR(OTP_RINGO_1)
#define OTP_RINGO_2					MKSTR(OTP_RINGO_2)
#define OTP_RINGO_3_L				MKSTR(OTP_RINGO_3_L)
#define OTP_RINGO_3_H				MKSTR(OTP_RINGO_3_H)
#define OTP_RINGO_4					MKSTR(OTP_RINGO_4)

#ifdef __UBOOT__

/**
 * @brief OTP initialize
 *
 * @return
 *	   0: success
 *	  !0: failure
 */
int otp_init(void);

/**
 * @brief read OTP data
 *
 * @param[in] bit_addr bit address
 * @param[in] len bit size, shall <= 32
 * @param[out] p_result OTP data
 *
 * @return
 *	   0: success
 *	  !0: failure
 */
int otp_read_random(unsigned int bit_addr, unsigned char len, unsigned int *p_result);

/** @Hide */
int otp_write_random(unsigned int bit_addr, unsigned int len, unsigned int *val);

/**
 * @brief Unlock OTP protected PPArea0 and/or PPArea1
 *
 * @return
 *	   0: success
 *	  !0: failure
 */
int otp_unlock_write(void);

#endif

/*
 * Get OTP Object information
 *
 * @param[in] name OTP object name
 * @param[out] value OTP object value
 * @param[out] description OTP object detail description
 *
 * @return
 *     0: success
 *    !0: failure
 */
int mt_otp_get_obj(const char *name, unsigned int *value, const char **description);

#define MT_OTP_OBJ_READ(name, v)	do {v=0; (void)mt_otp_get_obj(name, (unsigned int *)&v, NULL);}while(0)

/* silk screen size: 12 character */
#define MAX_OTP_SILKSCREEN_SIZE			12


/* A0 */
#if 0
typedef struct
{
	union
	{
		char string[MAX_OTP_SILKSCREEN_SIZE+1];

		struct
		{
			char mark;		/* fixed 'M' */

			/*
			 * 'C': DVB-C
			 * 'T': DVB-T/T2
			 * 'S': DVB-S/S2/S2X
			 * 'M': Soc multi-demod
			 * 'N': Decoder/OTT/IPTV
			 * 'B': ABS
			 * 'A': ATSC
			 * 'I': ISDB-T
			 */
			char product_line;

			/*
			 * '2': Symphony
			 * '5': Concerto
			 * '8': Maestro
			 */
			char product_family;
			/*
			 * '4': Symphony4
			 * '5': Symphony5
			 * '6': Symphony6
			 */
			char generation_code;

			/*
			 * '0': SD
			 * '1': FHD, >= 1.2K DMIPS
			 * '2': FHD, >= 2K DMIPS
			 * '3': FHD, >= 3K DMIPS
			 * '4': UHD-4K, > 4.5K DMIPS + GPU
			 * '5': UHD-4K, >= 7K DMIPS + GPU
			 * '6': UHD-4K, >= 15K DMIPS + AI
			 * '7': ?
			 * '8': UHD-8K, >= 15K DMIPS + AI
			 * '9': UHD-4K, >= 30K DMIPS + AI
			 */
			char segment;
			/*
			 * SD/FHD
			 *   '0': H.264 zapper(1-T)
			 *   '1': H.264 feature(1-T, 10/100 eth)
			 *   '2': H.264 rich(>=2-T, 10/100 eth)
			 *   '3': H.265 zapper(1-T)
			 *   '4': H.265 feature(1-T, 10/100 eth)
			 *   '5': H.265 rich(>=2-T, 10/100 eth)
			 *   '6': reserved
			 * UHD
			 *   '0': H.265 zapper(1-T)
			 *   '1': H.265 feature(1-T, 10/100 eth)
			 *   '2': H.265 feature(>=2-T, 10/100 eth)
			 *   '3': H.265 feature(>=2-T, Gbe)
			 *   '4': H.265 feature(=1-T, eth) + AVx
			 *   '5': H.265 feature(>=2-T, eth) + AVx
			 *   '6': H.265 rich(>=2-T, eth) + AVx + VVC
			 * All
			 *   '7': TSIO(if different price)
			 *   '8': reserved
			 *   '9': CI/CI+
			 */
			char sub_segment;

			/*
			 * 'Q': QFP
			 * 'N': QFN
			 * 'B': BGA
			 */
			char package_type;
			/*
			 * '0': Non-SIP
			 * '1': 32MB
			 * '2': 64MB DDR2
			 * 'B': 64MB DDR3
			 * '3': 128MB DDR2
			 * 'C': 128MB DDR3
			 * '4': 256MB
			 * '5': 512MB
			 * 'D': 512MB DDR4
			 * '6': 1GB
			 * 'E': 1GB DDR4
			 * '7': 2GB
			 * '8': 4GB
			 */
			char sip_info;
			/*
			 * '0': No CAS
			 * 'A': ABV
			 * 'C': Conax+CRI
			 * 'D': Irdeto
			 * 'G': Nagra+Conax
			 * 'R': Cryptoguard
			 * 'S': Sumavision
			 * 'T': CTI
			 * 'U': VO
			 * 'V': VMX
			 * 'Y': NSTV
			 * 'Z': DCAS
			 *
			 * 'P': Panaccess
			 */
			char cas_vendor;
			/*
			 * '0': A version die
			 * '1': B version die
			 * '2': C version die
			 * '3': D version die
			 * '4': E version die
			 * '5': F version die
			 */
			char chipset_version;
			/*
			 * '0': Bin1(CPU clock 1.5GHz)
			 * '1': Bin2(CPU clock 1.2GHz)
			 * '2': Bin3(CPU clock 1.0GHz)
			 * '3': Bin1 + TSIO (if absent in #6)
			 * '4': Bin2 + TSIO (if absent in #6)
			 * '5': Bin3 + TSIO (if absent in #6)
			 * '6': Bin1 + Nexguard
			 * '7': Bin2 + Nexguard
			 * '8': Bin3 + Nexguard
			 */
			char drm_watermark_cas_extension_bin;
			/*
			 * '0': No License
			 * 'A': Dolby Audio + Dolbyvision
			 * 'B': Dolbyvision
			 * 'D': Dolby Audio
			 */
			char license_royalty;

		} detail;
	};

} mt_chip_silkscreen_t;
#endif

/* A1 */
typedef struct
{
	union
	{
		char string[MAX_OTP_SILKSCREEN_SIZE+1];

		struct
		{
			char mark;		/* fixed 'M' */

			/*
			 * 'S': DVB-S/S2/S2X
			 * 'C': DVB-C
			 * 'T': DVB-T/T2
			 * 'N': Decoder/OTT/IPTV
			 * 'M': Soc multi-demod
			 */
			char product_line;

			/*
			 * '2': Symphony
			 * '5': Concerto
			 * '8': Maestro
			 */
			char product_family;
			/*
			 * '2': Symphony2
			 * '4': Symphony4
			 * '5': Symphony5
			 * '6': Symphony6
			 * '8': Symphony8
			 */
			char generation_code;

			/*
			 * '0': Reserved
			 * '1': Reserved
			 * '2': Reserved
			 * '3': FHD, >= 3K DMIPS
			 * '4': UHD-4K, > 4.5K DMIPS + GPU
			 * '5': UHD-4K, >= 7K DMIPS + GPU
			 * '6': UHD-4K, >= 15K DMIPS + AI
			 * '7': Reserved
			 * '8': UHD-8K, >= 15K DMIPS + AI
			 * '9': UHD-4K, >= 30K DMIPS + AI
			 */
			char segment;
			/*
			 *   '0': H.265 zapper(1-T)
			 *   '1': H.265 feature(1-T, 10/100 eth)
			 *   '2': H.265 feature(>=2-T, 10/100 eth)
			 *   '3': H.265 feature(>=2-T, Gbe)
			 *   '4': Reserved
			 *   '5': H.265 feature(>=2-T, eth) + AVx
			 *   '6': H.265 rich(>=2-T, eth) + AVx + VVC
			 *   '7': Reserved
			 *   '8': reserved
			 *   '9': H.265 feature(>=2-T, 10/100 eth) + CI/CI+
			 */
			char sub_segment;

			/*
			 * 'Q': QFP 1st generation
			 * 'R': QFP 2nd generation
			 * 'S': QFP 3rd generation
			 * 'N': QFN 1st generation
			 * 'O': QFN 2nd generation
			 * 'P': QFN 3rd generation
			 * 'B': BGA 1st generation
			 * 'C': BGA 2nd generation
			 * 'D': BGA 3rd generation
			 */
			char package_type;
			/*
			 * '0': Non-SIP
			 * '1': 256Mb
			 * '2': 512Mb DDR2
			 * 'B': 512Mb DDR3
			 * '3': 1Gb DDR2
			 * 'C': 1Gb DDR3
			 * '4': 2Gb
			 * '5': 4Gb DDR3
			 * 'D': 4Gb DDR4
			 * '6': 8Gb DDR3
			 * 'E': 8Gb DDR4
			 * '7': 16Gb
			 */
			char sip_info;
			/*
			 * '0': No CAS
			 * '2': Customer-1
			 * '3': Customer-2
			 * '5': Customer-3
			 * '6': Customer common Security
			 * 'A': ABV
			 * 'B': iCAS(ByDesign)
			 * 'C': Conax+CRI
			 * 'D': Irdeto
			 * 'E': GS/DRECrypto
			 * 'F': Safeview
			 * 'G': Nagra+Conax
			 * 'H': United
			 * 'J': RSCRYTO
			 * 'K': Gospell
			 * 'L': Topreal
			 * 'M': SMI
			 * 'N': Synamedia
			 * 'P': Panaccess
			 * 'R': Cryptoguard
			 * 'S': Sumavision
			 * 'T': CTI
			 * 'U': Viaccess-ORCA(VO)
			 * 'V': VMX
			 * 'W': SecureTV
			 * 'Y': Novel
			 * 'Z': DCAS
			 */
			char cas_vendor;
			/*
			 * '0': A version die
			 * '1': B version die
			 * '2': C version die
			 * '3': D version die
			 * '4': E version die
			 * '5': F version die
			 * 'A': A version die + CRI/TSIO
			 * 'B': B version die + CRI/TSIO
			 * 'C': C version die + CRI/TSIO
			 * 'D': D version die + CRI/TSIO
			 * 'E': E version die + CRI/TSIO
			 * 'F': F version die + CRI/TSIO
			 */
			char chipset_version;
			/*
			 * '0': Normal Bin1
			 * '1': Middle Bin2
			 * '2': Low Bin3
			 * '3': Normal Bin1 + Nexguard
			 * '4': Middle Bin2 + Nexguard
			 * '5': Low Bin3 + Nexguard
			 */
			char drm_watermark_cas_extension_bin;
			/*
			 * '0': No License
			 * 'A': Dolby Audio + Dolby Vision
			 * 'B': Dolby Vision
			 * 'D': Dolby Audio
			 */
			char license_royalty;

		} detail;
	};

} mt_chip_silkscreen_t;

/**
 * @brief Get Chip Silk Screen
 *
 * @param[out] silkscr chipset silk screen string
 *
 * @return
 *   0: success
 *  !0: failure
 */
int mt_otp_get_chip_silkscreen(mt_chip_silkscreen_t *silkscr);

//---------------------------------------------------------------------------//

#ifdef __UBOOT__

/**
 * @brief get OTP TEE enable flags
 *
 * @return
 *	   0: TEE is not enabled
 *	  !0: TEE is enabled
 */
int is_tee_enabled(void);

/**
 * @brief check if TEE Loader exists
 *
 * @return
 *	   false: TEE Loader is not exist
 *	    true: TEE Loader is exist
 */
bool is_tee_loader_exist(void);

/**
 * @brief check if avcpu is protected(encrypted)
 *
 * @return
 *	   0: avcpu is not protected
 *	  !0: avcpu is protected
 */
int is_avcpu_protected(void);

/**
 * @brief check and update OTP TEE EN bit
 *
 * @param[in] eval evaluate
 *
 * @return
 *	   0: success
 *	  !0: failure
 */
int otp_update_tee_en(MT_BOOL eval);

#endif

#endif

