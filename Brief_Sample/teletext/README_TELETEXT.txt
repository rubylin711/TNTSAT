1. 整体介绍
该sample包含多个基本文件
sample_teletext.c
该文件是关于DVB或文件播放和获取ttx的主要步骤以及涉及函数。
sample_ttx_data.c
该文件是关于获取数据
sample_teletext_out.c
该文件是关于内容输出到屏幕
Makefile
该文件为编译所用到的makefile

2. 编译
执行Makefile进行编译
make

3. 使用
拷贝 sample_teletext 到u盘,确认你要播放的源中有teletext
执行以下指令：
mount /dev/sda1 /mnt/
将linux/pub目录下的lib文件夹拷贝到U盘
export LD_LIBRARY_PATH=/mnt/shared_lib_striped/:/usr/local/lib
cd /mnt
./sample_teletext -f ./ttx.ts(文件播放)
./sample_teletext -c 314  6875  64 -> frequency SymbolRate QAM(DVBC)
./sample_teletext -s 3840 27500 1 0 0 -> frequency SymbolRate 22k(0:off,1:on)(DVBS)

4.流程介绍
  1) mt_sys_init()  //系统初始化
  2) MTADP_Fe_Init()  //初始化tuner
  3) MTADP_Fe_Connect_Dvbc()  //dvbc锁频
  4) MTADP_Fe_Connect_Dvbs()  //dvbs锁频
  5）MTADP_HDMI_Init()  //HDMI初始化
  6) MTADP_Disp_Init()  //display初始化
  7) MTADP_Snd_Init()  //sound初始化
  8) MTADP_VO_Init()  //voice初始化
  9) MT_DmxInit  //dmx初始化
	(1) MT_UNF_DMX_Init()  //dmx初始化
	(2) MT_UNF_DMX_AttachTSPort()  //dmx端口连接端口	
  10) InjectTsTask()  //线程函数，读取U盘视频文件信息
  11) MTADP_Search_Init()  //搜索节目初始化
  12) MTADP_Search_GetAllPmt()  //获取pmt节目信息
  13) MT_AVplayInit()  //AV播放器初始化
  14) MT_AVPlay_Start()  //设置播放参数，并开始播放
  15) MT_StartTtx()  //获取TTX数据
	(1)MT_UNF_TTX_DataRecv_Create()  //创建获取数据buf
	(2)Mtgo_Teletext_Init()  //Mtgo的Teletext组件初始化
	(3)MT_UNF_TTX_Init()  //ttx初始化
	(4)MT_UNF_TTX_Create()  //创建ttx句柄
	(5)MT_UNF_TTX_SwitchContent()  //选择content类型
	(6)MT_UNF_TTX_Output()  //输出ttx数据
  16) MT_StartDataFilter()  //数据过滤
	(1)Ttx_Data_Init()  //ttx数据组件初始化
	(2)Ttx_Data_Install()  //数据组件安装
  17) Ttx_Data_Uninstall()  //卸载数据组件
  18) Ttx_Data_DeInit()  //ttx数据组件去初始化
  19) MT_UNF_TTX_Destroy()  //销毁ttx句柄
  20) MT_UNF_TTX_DataRecv_Destroy()  //销毁数据buf
  21) Mtgo_Teletext_DeInit()  //Mtgo的Teletext组件去初始化
  22) MT_UNF_TTX_DeInit()  //ttx去初始化
  23) MT_AvplayDeInit()  //av播放器去初始化
  24) MTADP_Search_FreeAllPmt()  //释放获取到的pmt信息
  25) MTADP_Search_DeInit()  //搜索节目去初始化
  26）MT_DmxDeInit()  //dmx去初始化
	(1) MT_UNF_DMX_DeInit()  //dmx去初始化
	(2) MT_UNF_DMX_DetachTSPort()  //dmx端口断开RAM端口
  27) MTADP_VO_DeInit()  //voice去初始化
  28) MTADP_Snd_DeInit()  //sound去初始化
  29) MTADP_Disp_DeInit()  //display去初始化
  30) MTADP_HDMI_DeInit()  //HDMI去初始化
  31) MTADP_Fe_Deinit()  //Tuner去初始化
  32) mt_sys_deinit()  //系统去初始化
