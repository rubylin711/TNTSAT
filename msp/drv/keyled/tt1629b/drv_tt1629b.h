/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2018 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifndef __DRV_FD650_H__
#define __DRV_FD650_H__

#include <linux/delay.h>
#include "drv_gpio_ioctl.h"

#define ENABLE_TT1629        1
#define ENABLE_TT1629B      0

#define LEVEL1_BRIGHT           (0x88) // 1 level brightness
#define LEVEL2_BRIGHT           (0x89) // 2 level brightness
#define LEVEL3_BRIGHT           (0x8A) // 3 level brightness
#define LEVEL4_BRIGHT           (0x8B) // 4 level brightness
#define LEVEL5_BRIGHT           (0x8C) // 5 level brightness
#define LEVEL6_BRIGHT           (0x8D) // 6 level brightness
#define LEVEL7_BRIGHT           (0x8E) // 7 level brightness
#define LEVEL8_BRIGHT           (0x8F) // 8 level brightness

#define AUTO_ADDRESS_MODE   		(0x40)
#define READ_DATA_MODE      		(0x42)
#define FIXED_ADDRESS_MODE  		(0x44)

#define START_ADDRESS       		(0xc0)

#define GRID1_FIXED_ADDR    		(START_ADDRESS)
#define GRID2_FIXED_ADDR    		(GRID1_FIXED_ADDR+0x02)
#define GRID3_FIXED_ADDR    		(GRID2_FIXED_ADDR+0x02)
#define GRID4_FIXED_ADDR    		(GRID3_FIXED_ADDR+0x02)
#define GRID5_FIXED_ADDR    		(GRID4_FIXED_ADDR+0x02)
#define GRID6_FIXED_ADDR    		(GRID5_FIXED_ADDR+0x02)
#define GRID7_FIXED_ADDR    		(GRID6_FIXED_ADDR+0x02)
#define GRID8_FIXED_ADDR    		(GRID7_FIXED_ADDR+0x02)


#if ENABLE_TT1629
//depends on customer HW design
#define FIRST_GRID_ADDR          	 (GRID6_FIXED_ADDR)
#define SECOND_GRID_ADDR          (GRID7_FIXED_ADDR)
#define THIRD_GRID_ADDR          	 (GRID5_FIXED_ADDR)
#define FOURTH_GRID_ADDR          (GRID4_FIXED_ADDR)
#define COLON_ADDR          		 (GRID7_FIXED_ADDR)
#define SPOT_ADDR 				 (GRID5_FIXED_ADDR)
#define GREEN_LED_ADDR          	 (GRID3_FIXED_ADDR)
#define RED_LED_ADDR          		 (GRID2_FIXED_ADDR)

#define SEG1  						(0x01)  		//BIT0
#define SEG2  						(0x02)  		//BIT1
#define SEG3  						(0x04)  		//BIT2
#define SEG4  						(0x08)  		//BIT3
#define SEG5  						(0x10)  		//BIT4
#define SEG6  						(0x20)  		//BIT5
#define SEG7  						(0x40)  		//BIT6
#define SEG8  						(0x80)  		//BIT7
#define SEG9  						(0x100) 		//BIT8
#define SEG10  						(0x200)    	//BIT9
#define SEG11  						(0x400)    	//BIT10
#define SEG12  						(0x800)    	//BIT11
#define SEG13  						(0x1000)    	//BIT12
#define SEG14  						(0x2000)    	//BIT13
#define SEG15  						(0x4000)    	//BIT14
#define SEG16  						(0x8000)    	//BIT15



#define BIT_A	   				   	(SEG16)  
#define BIT_B 						(SEG9)  
#define BIT_C						(SEG10)      
#define BIT_D						(SEG7)
#define BIT_E 						(SEG1)
#define BIT_F 						(SEG11)
#define BIT_G1 						(SEG15)
#define BIT_G2						(SEG6)
#define BIT_H 						(SEG12)
#define BIT_J 						(SEG13)  
#define BIT_K						(SEG14)
#define BIT_L						(SEG5)
#define BIT_M						(SEG4)
#define BIT_N						(SEG2)
#define BIT_DP						(SEG8)
#define BIT_D1_D2					(SEG3)


#define TM1629B_STB          		   (MT_UNF_AO_GPIO_5)
#define TM1629B_CLK          		   (MT_UNF_AO_GPIO_6)
#define TM1629B_DAT          		   (MT_UNF_AO_GPIO_7)

#define GRID_CNT					(8)
#define GRID_DATA_CNT				(2*GRID_CNT)
#define FP_MAX_LED_NUM  			(4)
#endif

#if ENABLE_TT1629B
//depends on customer HW design
#define FIRST_GRID_ADDR          	(GRID8_FIXED_ADDR)
#define SECOND_GRID_ADDR          	(GRID7_FIXED_ADDR)
#define THIRD_GRID_ADDR          	(GRID6_FIXED_ADDR)
#define FOURTH_GRID_ADDR          	(GRID5_FIXED_ADDR)
#define COLON_ADDR          		(GRID2_FIXED_ADDR)
#define GREEN_LED_ADDR          	(GRID3_FIXED_ADDR)
#define RED_LED_ADDR          		(GRID4_FIXED_ADDR)

#define SEG1  						(0x01)  		//BIT0
#define SEG2  						(0x02)  		//BIT1
#define SEG3  						(0x04)  		//BIT2
#define SEG4  						(0x08)  		//BIT3
#define SEG5  						(0x10)  		//BIT4
#define SEG6  						(0x20)  		//BIT5
#define SEG7  						(0x40)  		//BIT6
#define SEG8  						(0x80)  		//BIT7
#define SEG9  						(0x100) 		//BIT8
#define SEG10  						(0x200)    	//BIT9
#define SEG11  						(0x400)    	//BIT10
#define SEG12  						(0x800)    	//BIT11

#define bit_a 						(SEG3)
#define bit_b 						(SEG1)
#define bit_c 						       (SEG2)
#define bit_d 						(SEG9)
#define bit_e 						(SEG4)
#define bit_f 						       (SEG5)
#define bit_g1 						(SEG10)
#define bit_g2 						(SEG11)
#define bit_h 						(SEG12)
#define bit_j 						       (SEG6)

#define TM1629B_STB          		(MT_UNF_AO_GPIO_5)
#define TM1629B_CLK          		(MT_UNF_AO_GPIO_6)
#define TM1629B_DAT          		(MT_UNF_AO_GPIO_7)

#define GRID_CNT					(8)
#define GRID_DATA_CNT				(2*GRID_CNT)
#define FP_MAX_LED_NUM  			(4)
#endif

#endif
