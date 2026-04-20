/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef _FD612DRV_H_
#define	_FD612DRV_H_

#include <linux/delay.h>
#include "drv_gpio_ioctl.h"

#define HARD_I2C__

#ifdef  FD612DRV_GROBLE
    #define FD612_EXT 
#else
    #define FD612_EXT  extern
#endif

/** @addtogroup FD612_DRIVER FD612 驱动
  * @{
*/
/** @addtogroup  FD612_REMOVE  移植需知
  * @{
*/
/** @addtogroup  FD612_REMOVE_CHANGE  需要修改
  * 根据平台和软件需要进行相应的修改
  * @{
*/
#define FD628_NEGA_DISP         1///<1为共阴数码管，0为共阳数码管
#define	FD612_DECODE_TAB_EN     1///<是否开启FD612_DecodeTab[]功能 0为关闭,关闭该功能可以省CODE
#define	FD612_COMBINA_SW_EN     1 ///<是否开启组合键功能 0为关闭,关闭该功能可以省RAM

typedef  MT_BOOL BOOLEAN ;   ///<定义布尔类型
typedef  mt_u8 INT8U  ;      ///<定义无符号8位数类型
typedef  mt_u32 INT32U ;     ///<定义无符号32位数类型

#define CONST const         ///<定义常量
/********FD612.C的内部宏定义 *********/
/*串行IO口设置，根据平台IO口配置*/

#define FD612_DATA     AO_GPIO_0
#define	FD612_SCL      AO_GPIO_1

#define		FD612_SDA_IN(val)   drv_gpio_get_value(FD612_DATA, &val)        ///< 当SDA设为输入方向时，读取到的电平值
#if 1
#define		FD612_SCL_D_OUT	    drv_gpio_set_dir(FD612_SCL, GPIO_DIR_OUTPUT)    ///< 设置SCL接口为输出方向（最好设置为开漏输出，不可以设置成为推挽输出）		
#define		FD612_SDA_D_OUT     drv_gpio_set_dir(FD612_DATA, GPIO_DIR_OUTPUT)   ///< 设置SDA接口为输出方向（最好设置为开漏输出，不可以设置成为推挽输出）		
#define		FD612_SDA_D_IN		drv_gpio_set_dir(FD612_DATA, GPIO_DIR_INPUT)///< 设置SDA接口为输入方向（最好设置为高阻输入，不可以设置成为推挽输出）
#define		FD612_SCL_SET		drv_gpio_set_value(FD612_SCL, GPIO_VALUE_HIGH_LEVEL)///< 将SCL设置为高电平
#define		FD612_SCL_CLR	   	drv_gpio_set_value(FD612_SCL, GPIO_VALUE_LOW_LEVEL)///< 将SCL设置为低电平
#define		FD612_SDA_SET		drv_gpio_set_value(FD612_DATA, GPIO_VALUE_HIGH_LEVEL)///< 将SDA设置为高电平
#define		FD612_SDA_CLR		drv_gpio_set_value(FD612_DATA, GPIO_VALUE_LOW_LEVEL)///< 将SDA设置为低电平
#else
#define		FD612_SCL_D_OUT	    do{drv_gpio_set_dir(FD612_SCL, GPIO_DIR_OUTPUT);printk(KERN_EMERG "%s,%d.do.scl\n",__FUNCTION__,__LINE__);}while(0)    ///< 设置SCL接口为输出方向（最好设置为开漏输出，不可以设置成为推挽输出）		
#define		FD612_SDA_D_OUT     do{drv_gpio_set_dir(FD612_DATA, GPIO_DIR_OUTPUT);printk(KERN_EMERG "%s,%d.do.sda\n",__FUNCTION__,__LINE__);}while(0)   ///< 设置SDA接口为输出方向（最好设置为开漏输出，不可以设置成为推挽输出）		
#define		FD612_SDA_D_IN		do{drv_gpio_set_dir(FD612_DATA, GPIO_DIR_INPUT);printk(KERN_EMERG "%s,%d.di.sda\n",__FUNCTION__,__LINE__);}while(0)///< 设置SDA接口为输入方向（最好设置为高阻输入，不可以设置成为推挽输出）
#define		FD612_SCL_SET		do{drv_gpio_set_value(FD612_SCL, GPIO_VALUE_HIGH_LEVEL);printk(KERN_EMERG "%s,%d.scl.h\n",__FUNCTION__,__LINE__);}while(0)///< 将SCL设置为高电平
#define		FD612_SCL_CLR	   	do{drv_gpio_set_value(FD612_SCL, GPIO_VALUE_LOW_LEVEL);printk(KERN_EMERG "%s,%d.scl.l\n",__FUNCTION__,__LINE__);}while(0)///< 将SCL设置为低电平
#define		FD612_SDA_SET		do{drv_gpio_set_value(FD612_DATA, GPIO_VALUE_HIGH_LEVEL);printk(KERN_EMERG "%s,%d.sda.h\n",__FUNCTION__,__LINE__);}while(0)///< 将SDA设置为高电平
#define		FD612_SDA_CLR		do{drv_gpio_set_value(FD612_DATA, GPIO_VALUE_LOW_LEVEL);printk(KERN_EMERG "%s,%d.sda.l\n",__FUNCTION__,__LINE__);}while(0)///< 将SDA设置为低电平
#endif
#define		FD612_DELAY_1US     udelay(1)   ///< 延时的宏定义，最小延时时间为1uS
/* @} FD612_REMOVE_CHANGE */
/**
 * @addtogroup  FD612_REMOVE_CHANGEABLE   可以修改
 * 根据应用需要进行相应的修改 
 * 可以修改的函数 FD612_Init();FD612_Refresh();
 * @{
 */
/* @} FD612_REMOVE_CHANGEABLE */
/* @} FD612_REMOVE */
/** @addtogroup  FD612_APP 应用相关
  * @{
*/
/** @addtogroup  FD612_APP_MACRO   宏定义
  * @{
*/
/* *******************************************数码管******************************************************************************** 
 *            a
 *         -------
 *        |       |
 *      f |       | b
 *         ---g---		
 *        |       |	c
 *      e |       |	
 *         ---d---   dp
 * *************************************************************************************************************************************** *
 *码字		| 0		| 1		| 2		| 3		| 4		| 5		| 6		| 7		| 8		| 9		| A		| b		| C 	| d		| E		| F		|
 *编码		|0x3F	|0x06	|0x5B	|0x4F	|0x66	|0x6D	|0x7D	|0x07	|0x7F	|0x6F	|0x77	|0x7c	|0x39	|0x5E	|0x79	|0x71	|
 ************************************************************************************************************************************* */
 /* ********数码管显示字符宏定义 ******** */
#define		FD612_DISP_NONE 0X00
#define		FD612_DISP_0 0X3F
#define		FD612_DISP_1 0x06
#define		FD612_DISP_2 0x5B
#define		FD612_DISP_3 0x4F
#define		FD612_DISP_4 0x66
#define		FD612_DISP_5 0X6d
#define		FD612_DISP_6 0x7D
#define		FD612_DISP_7 0x07
#define		FD612_DISP_8 0x7f
#define		FD612_DISP_9 0x6F
#define		FD612_DISP_A 0X77 
#define		FD612_DISP_b 0x7c 
#define		FD612_DISP_C 0X39
#define		FD612_DISP_c 0X58
#define		FD612_DISP_d 0x5E
#define		FD612_DISP_E 0X79
#define		FD612_DISP_e 0X7b
#define		FD612_DISP_F 0x71
#define		FD612_DISP_I 0X60
#define		FD612_DISP_L 0X38
#define		FD612_DISP_r 0X72		
#define		FD612_DISP_n 0X54
#define		FD612_DISP_N 0X37
#define		FD612_DISP_O 0X3F
#define		FD612_DISP_P 0XF3
#define		FD612_DISP_S 0X6d
#define		FD612_DISP_y 0X6e
#define		FD612_DISP__ 0x08
/********ACK位********/
#define FD612_NACK 1///<无应答信号
#define FD612_ACK  0///<有应答信号

#define	FD612_DISP_MAX_ADDR 11    ///<FD612显示地址的最大值
#define	FD612_DISP_MIN_ADDR 0     ///<FD612显示地址的最小值
/** @addtogroup  FD612_APP_MACRO_CMD    控制命令
  * @{
*/
#define FD612_KEY_RDCMD 0x42

//显示模式命令设置
#define FD612_8SEG_CMD	0x00		///<8段12位
#define FD612_7SEG_CMD	0x03		///<7段10位
//数据命令设置
#define FD612_ADDR_INC_DIGWR_CMD	      	0x40		    ///< 自动地址增加，写数据	
#define FD612_ADDR_STATIC_DIGWR_CMD			  0x4c		    ///< 固定地址模式。写显示
//地址命令设置
/**
 *  @brief 写入显示数据的地址的命令
 *  @note  使用方法：FD612_DIGADDR_WRCMD|相应的地址 
 *  @par Example
 *  @code
 *  FD612_Command(FD612_DIGADDR_WRCMD|FD612_DIG5_ADDR);
 *  @endcode 
 */
#define FD612_DIGADDR_WRCMD	0xc0       
//显示位相应的地址
#define FD612_DIG1_ADDR 0x00
#define FD612_DIG2_ADDR 0x01
#define FD612_DIG3_ADDR 0x02
#define FD612_DIG4_ADDR 0x03
#define FD612_DIG5_ADDR 0x04
#define FD612_DIG6_ADDR 0x05
#define FD612_DIG7_ADDR 0x06
#define FD612_DIG8_ADDR 0x07
#define FD612_DIG9_ADDR 0x08
#define FD612_DIG10_ADDR 0x09
#define FD612_DIG11_ADDR 0x0A
#define FD612_DIG12_ADDR 0x0B
//显示亮度和显示开关之间用或的关系
#define FD612_INTENS1		0x80		///< 1级亮度	
#define FD612_INTENS2		0x81		///< 2级亮度	
#define FD612_INTENS3		0x82		///< 3级亮度
#define FD612_INTENS4		0x83		///< 4级亮度	
#define FD612_INTENS5		0x84  	///< 5级亮度	
#define FD612_INTENS6		0x85		///< 6级亮度
#define FD612_INTENS7		0x86		///< 7级亮度	
#define FD612_INTENS8		0x87		///< 8级亮度

#define FD612_DISP_ON   0x88	  ///<打开FD612显示
#define FD612_DISP_OFF  0x80		///<关闭FD612显示
/* @} FD612_APP_MACRO_CMD */

/** @brief  FD612 8段12位模式显示 */
#define FD612_8SEG_MODE   do{ \
                          FD612_DispBuff.DispSEG_MODE=FD612_8SEG_CMD;   \
                          FD612_Command(FD612_8SEG_CMD);  \
                          } while(0)
/** @brief  FD612 7段10位模式显示 */                      
#define FD612_7SEG_MODE do{  \
                          FD612_DispBuff.DispSEG_MODE=FD612_7SEG_CMD;   \
                          FD612_Command(FD612_7SEG_CMD);  \
                          }while(0)
/**
 *  @brief  FD612显示亮度和显示开关控制
 *  @note   显示亮度和显示开关之间使用或的关系
 *  @par Example
 *  @code
 *  FD612_DispStateWr(FD612_INTENS6|FD612_DISP_ON);
 *  @endcode 
 */                         
#define FD612_DispStateWr(DispStateTemp)  do{  \
                                        FD612_DispBuff.DispState=DispStateTemp; \
                                        FD612_Command(DispStateTemp); \
                                        }while(0)

/* @} FD612_APP_MACRO */
/** @addtogroup  FD612_APP_DATA   相关数据
  * @{
*/
//FD612_EXT	CONST  INT8U DISP_TAB[0x10]; ///<数码管码值表
/* @} FD612_APP_DATA */
/** @addtogroup  FD612_APP_FUNC   相关函数
  * @{
*/
/*
void  FD612_Command(INT8U CMD ); //发送一字节的命令
void FD612_Init(void);  //初始化FD612芯片
#if FD612_DECODE_TAB_EN!=0
INT8U DispGetCode(char cTemp);  //数码管的查表函数
INT8U FD612_DispString( INT8U Addr,char *PStr); //某个显示地址开始显示相应的字符串
#endif
#if FD628_NEGA_DISP==1
void FD612_SingleDisp(INT8U addr,INT8U dat); //向共阴数码管某个显示地址写入一字节的显示数据
#else
void FD612_PotiveTNage(INT8U addr,INT8U dat); //向共阳数码管某个显示地址写入一字节的显示数据
#endif
void FD612_Refresh(void); //刷新FD612的相关数据
void  FD612_DispDataRefresh(void);  //刷新FD612的显示数据
*/
/* @} FD612_APP_FUNC */
/**
 * @addtogroup  FD612_APP_DATA   相关数据
 * @{
 */ 
typedef struct {
	INT8U DispData[FD612_DISP_MAX_ADDR-FD612_DISP_MIN_ADDR+1]; ///<12位显示数据缓存器
	INT8U DispState;      ///<显示亮度和开关状态缓存器
	INT8U DispSEG_MODE;   ///<显示位段模式缓存器
} FD612_Struct_DispBuff;

/* @} FD612_APP_DATA */
/* @} FD612_APP */
/* @} FD612_DRIVER */
#endif
/******************* (C) COPYRIGHT 2013 FDHISI *****END OF FILE****/
