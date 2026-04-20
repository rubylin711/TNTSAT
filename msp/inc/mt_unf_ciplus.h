/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2021 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef MT_UNF_CIPLUS_H
#define MT_UNF_CIPLUS_H

/*
CAM registeres,defined in EN50221-annex a
*/
enum mtci_io_register
{
	CIC_DATA	= 0,			/* CI data register */
	CIC_CSR,					/* CI command/stauts register */
	CIC_SIZELS,					/* CI size register low bytes */
	CIC_SIZEMS					/* CI size register high bytes */
};

/*
ci tsin clk
*/
enum mtci_tsin_clk
{
	PAD_TS0_CLK=0,
	PAD_TS1_CLK=1,
	DEMOS_TS1_CLK=2,
	DEMOC_TS2_CLK=3,
	PAD_TS0_CLK_INVER=4,	//pad_tso_clk inverted
	PAD_TS1_CLK_INVER=5,	//pad_ts1_clk inverted
	DEMOS_TS1_CLK_INVER=6,	//demos _ts1_clk inverted
	DEMOC_TS2_CLK_INVERT=7, //democ_ts2_clk inverted
};

enum mtci_cam_status
{
	CICAM_STATUS_OUT = 0x00,
	CICAM_STATUS_IN = 0x01,
	CICAM_STATUS_UNKNONW = 0xff,
};

enum mtci_cmd_register
{
	CI_REG_RE				= 0x01,
	CI_REG_WE				= 0x02,
	CI_REG_IIR				= 0x10, /* CI+ */
	CI_REG_FR				= 0x40,
	CI_REG_DA				= 0x80,
	CI_REG_HC				= 0x01,
	CI_REG_SW				= 0x02,
	CI_REG_SR				= 0x04,
	CI_REG_RS				= 0x08,
};


/*!
@~chinese
@brief 初始化CI 设备
@param [in]
@return ::MT_SUCCESS
@~english
@brief initialize CI device object
@param [in]
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_ciplus_init(mt_void);
/*!
@~chinese
@brief 终止CI设备
@param [in]
@return ::MT_SUCCESS
@~english
@brief deinitialize the CI device object.
@param [in]
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_ciplus_deinit(mt_void);

/*!
@~chinese
@brief 打开CI设备
@param [in] const MT_CHAR* pdevfile ::CI设备文件,/dev/mt_cix
@param [out] MT_HANDLE* pdevhandle ::设备句柄
@return ::MT_SUCCESS
@~english
@brief open CI device.
@param [in] cont MT_CHAR* pdevfile: CI device file. /dev/mt_cix
@param [out] MT_HANDLE* pdevhandle::device handle
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_open(const MT_CHAR* pdevfile, MT_HANDLE* pdevhandle);

/*!
@~chinese
@brief 关闭CI设备
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@return ::MT_SUCCESS
@~english
@brief close CI device.
@param [in] MT_HANDLE* pdevhandle::device handle
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_close(MT_HANDLE pdevhandle);

/*!
@~chinese
@brief 向CICAM写入数据
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U8* pdata:: 要写入数据buffer
@param [in] MT_U32 len:写入数据长度
@return ::实际写入数据长度，如果写入失败，返回MT_FAILURE
@~english
@brief write data to CICAM
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U8* pdata::data buffer to be writed
@param [in] MT_U32 len: data length
@return ::data lenght writed,if failed,return MT_FAILURE
*/

MT_S32 mt_unf_ciplus_write(MT_HANDLE pdevhandle,MT_U8* pdata,MT_U32 len);

/*!
@~chinese
@brief 从CICAM读取数据
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U8* pdata:: 读取数据缓存buffer
@param [in] MT_U32 len: 要读取数据长度
@return ::实际读取数据长度，如果读取失败，返回MT_FAILURE
@~english
@brief read data from CICAM
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U8* pdata::data buffer to be readed
@param [in] MT_U32 len: data length want to be readed
@return ::data lenght readed,if failed,return MT_FAILURE
*/

MT_S32 mt_unf_ciplus_read(MT_HANDLE pdevhandle,MT_U8* pdata,MT_U32 len);


/*!
@~chinese
@brief 以IO模式读取数据
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U32  addr: 读取数据地址
@param [in] MT_U8* pdata:: 读取数据缓存buffer
@return ::MT_SUCCESS
@~english
@brief read data from CICAM in IO mode
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U8 addr:: address to be readed
@param [in] MT_U8* pdata::data buffer to be readed
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_ioread(MT_HANDLE pdevhandle,MT_U32 addr,MT_U8* pdata);


/*!
@~chinese
@brief 以IO模式写入数据
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U32  addr: 写入数据地址
@param [in] MT_U8 data:: 写入数据
@return ::MT_SUCCESS
@~english
@brief write data from CICAM in IO mode
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U8 addr:: address to be writed
@param [in] MT_U8 pdata::date to be writed
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_iowrite(MT_HANDLE pdevhandle,MT_U32 addr,MT_U8 data);

/*!
@~chinese
@brief  读取memory attributes
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U8  addr: 读取数据地址
@param [in] MT_U8 pdata:: 读取数据缓存地址
@param [in] MT_U32 len::读取数据长度
@return ::MT_SUCCESS
@~english
@brief read cicam memory attributes
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U8 addr:: address to be readed
@param [in] MT_U8* pdata::data buffer to be readed
@param [in] MT_U32 len::data lenght to be reades
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_memread(MT_HANDLE pdevhandle,MT_U32 addr,MT_U8* pdata,MT_U32 len);


/*!
@~chinese
@brief  写入cicam memory attributes
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U8  addr: 写入数据地址
@param [in] MT_U8 pdata:: 写入数据缓存地址
@param [in] MT_U32 len::写入数据长度
@return ::MT_SUCCESS
@~english
@brief write cicam memory attributes
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U8 addr:: address to be writed
@param [in] MT_U8* pdata::data buffer to be writed
@param [in] MT_U32 len::data lenght to be writed
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_memwrite(MT_HANDLE pdevhandle,MT_U32 addr,MT_U8* pdata,MT_U32 len);

/*!
@~chinese
@brief  配置cam检测时间
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U32 time::检测时间
@return ::MT_SUCCESS
@~english
@brief config cam detect time
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U32 time::detect time
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_cfg_detect_time(MT_HANDLE pdevhandle,MT_U32 time);

/*!
@~chinese
@brief  获取CAM卡状态
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [out] enum mtci_cam_status*::CAM卡状态
@return ::MT_SUCCESS
@~english
@brief get cam card status
@param [in] MT_HANDLE* pdevhandle::device handle
@param [out] enum mtci_cam_status*::cam card status
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_get_camst(MT_HANDLE pdevhandle,enum mtci_cam_status* pcamst);

/*!
@~chinese
@brief  启用/禁用TS1
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_S32 isenable::  0:禁止ts1 in, 1:开启ts1 in
@return ::MT_SUCCESS
@~english
@brief enable/disable ts1 in
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_S32 isenable:: 0:disable ts1 in 1: enable ts1 in
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_tsenable(MT_HANDLE pdevhandle,MT_S32 isenable);

/*!
@~chinese
@brief  配置TS1 串行或者并行模式
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_S32 isenable::  0:并行模式, 1:串行模式
@return ::MT_SUCCESS
@~english
@brief enable/disable ts1 in
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_S32 isenable:: 0:parallel mode 1:serial mode
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_ciplus_serial_enable(MT_HANDLE pdevhandle,MT_S32 isenable);


/*!
@~chinese
@brief  配置过滤PID，一共有64个通道，每个通道可以配置两个PID
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U8 chan:: 要配置通道索引
@param [in] MT_U16 pid1:: 要配置pid1
@param [in] MT_U16 pid0:: 要配置pid0
@return ::MT_SUCCESS
@~english
@brief config pid to be filterd,have 64 changnel in total,have two pids each channel
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U8 chan:: channel index
@param [in] MT_U16 pid1:: config pid1
@param [in] MT_U16 pid0:: config pid0
@return ::MT_SUCCESS
*/
MT_S32 mt_unf_ciplus_set_pidram(MT_HANDLE pdevhandle,MT_U8 chan,MT_U16 pid1,MT_U16 pid0);

/*!
@~chinese
@brief 是否启用PID filter功能，如果启用,如果数据PID存在PID RAM，数据将会被丢弃
如果禁用，数据将会发送给CAM
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_S32 isenable::  0:不启用1:启用
@return ::MT_SUCCESS
@~english
@brief  enable/disable pid filter,if enable ,the data with pid in pid ram will be droped,
if disabled,the data will be pass to CAM
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_S32 isenable:: 0:disable 1:enable
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_pidfiler_enable(MT_HANDLE pdevhandle,MT_S32 isenable);

/*!
@~chinese
@brief CI tsin时钟源选择
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] enum mtci_tsin_clk tsin_clk::ts in clk source
@return ::MT_SUCCESS
@~english
@brief  config ci tsin clock source
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] enum mtci_tsin_clk tsin_clk:: tsin clock sorce
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_tsin_clksel(MT_HANDLE pdevhandle,enum mtci_tsin_clk tsin_clk);

/*!
@~chinese
@brief cam卡供电控制
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] int isonk:: 1:打开电源  0:关闭电源
@return ::MT_SUCCESS
@~english
@brief  cam card power control
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] int ison:: 1: power on 0:power off
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_campower(MT_HANDLE pdevhandle,int ison);

/*!
@~chinese
@brief 设置TS1CTRL寄存器值
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U32 ts1val::寄存器值
@return ::MT_SUCCESS
@~english
@brief  set ts1ctrl register
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U32 ts1val:: register value
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_setts1(MT_HANDLE pdevhandle,MT_U32 ts1val);

/*!
@~chinese
@brief 获取TS1CTRL寄存器值
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [out] MT_U32* ts1val::寄存器值缓存
@return ::MT_SUCCESS
@~english
@brief  get ts1ctrl register
@param [in] MT_HANDLE* pdevhandle::device handle
@param [out] MT_U32* ts1val:: register value to be cache
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_getts1(MT_HANDLE pdevhandle,MT_U32* ts1val);


/*!
@~chinese
@brief 获取pid列表，在列表中的pid会被传输到cicam
@brief 每个表项包含2个pid，最多64个表项
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] MT_U8 index::列表索引
@param [out] MT_U16* pid1:: pid1
@param [out] MT_U16* pid0:: pid0
@return ::MT_SUCCESS
@~english
@brief  get pid list ,the pids in list will be passed to cicam
@brief one item include two pids
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] MT_U8 index:: index of pid list
@param [out] MT_U16* pid1:: pid1
@param [out] MT_U16* pid0:: pid0
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_get_pidram(MT_HANDLE pdevhandle,MT_U8 index,
		MT_U16* pid1,MT_U16* pid0);

/*!
@~chinese
@brief打印ciplus寄存器，只是调试用
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@return ::MT_SUCCESS
@~english
@brief dump register of ciplus,it is only for debug
@param [in] MT_HANDLE* pdevhandle::device handle
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_regdump(MT_HANDLE pdevhandle);

/*!
@~chinese
@brief 切换ciplus地址空间模式
@param [in] MT_HANDLE* pdevhandle ::设备句柄
@param [in] int iospace::地址模式, 0:memory mode,1:io mode
@return ::MT_SUCCESS
@~english
@brief switch ciplus address mode
@param [in] MT_HANDLE* pdevhandle::device handle
@param [in] int iospace::address mode,0:memory mode,1:io mode
@return ::MT_SUCCESS
*/

MT_S32 mt_unf_ciplus_iomem_switch(MT_HANDLE pdevhandle,int iospace);

#endif

