/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/

#include    "pic16f7x.h"
#include    "ledkb_regs.h"
#include    "irda_regs.h"
#include    "mcu_regs.h"
#include    "datatype.h"


#define VERSION            100

#define wr_port(port_addr,data) do { \
                                    EXPORT_ADDRESS_L = port_addr;    		\
                                    EXPORT_ADDRESS_H = (port_addr>>0x08);	\
                                    EXPORT_WDATA = data;					\
                                } while(0)
                                
#define rd_port(port_addr,data) do { \
                                    EXPORT_ADDRESS_L = port_addr;   		\
                                    EXPORT_ADDRESS_H = (port_addr>>0x08);	\
                                    EXPORT_RDATA_EN = 1;                   \
                                    data=EXPORT_RDATA;						\
                                } while(0)       



#define FD650_TEST	(0)

U8 smc_data[8]={0x00};
U8 smc_wdata[8]={0x00};
U8 smc_irdata[5]={0x00};


#define  CLK_GATE_CFG0_REG_ADDR   0x0010  
#define  CLK_GATE_CFG1_REG_ADDR   0x0014  
#define  CLK_GATE_CFG2_REG_ADDR   0x0018  
#define  CLK_GATE_CFG3_REG_ADDR   0x001c  
#define  LPM_GLB_CTRL 0x001C

#define SMC_BITDELAYCFG	(249)
#define SMC_T0_CMD_LEN		(5)


U8 bitdata[12]={0x00};

U16 tick = 0;
U16 ctick = 0; 
U16 timeout = 0; 
U16 timetick = 0;
U8 temp = 0;
U8 bitcount = 0;
U8 bytecount = 0;
U8 check = 0; 
U8 flag = 0;
U8 status = 0;
U8 read_num = 0;
U8 write_num = 0;
U8 cur_num = 0;
U8 ibit = 0;
U8 testcount = 0;
U8  ready = 0;
U8 interruptstatus = 0;
U8 bitstop = 0;
U8 wr_flag = 0 ;
U8 interrupt_flag = 0 ;
U8 bitdelay = 0;
U8 readins = 0;
U8 ircount = 0;
U8 data_offset = 0;
int count = 0;
int etu = 233;
U32 irbuffaddr = IRWF_BUFF_BASE;




#define AO_GPIO_STATE 0x502C
void aomcu_delay(int times)
{	
	int i =0;
	int j = 0;
	for(j=0;j<times;j++)
	{
		for(i=0;i<times;i++)
		asm ("nop");			
	}			
}

void aomcu_udelay(int times)
{	
	int i =0;
	for(i=0;i<times;i++)
	{
		asm ("nop");			
	}			
}

 void x_zero_delay(void)
 {
	aomcu_delay(1);
 }

void msdelay(U8 ms)
{
		ctick = 0;
		while(((ctick * 24) /10) < ms)
                ;	
}

#if FD650_TEST
/*!
  I2C register bit defination
  */

/*!
  I2C controller enable
  */
#define I2C_CTR_EN          0x80
/*!
  I2C interrupts enable
  */
#define I2C_CTR_IEN        0x40

/*!
  I2C cmd: Start Transmit
  */
#define I2C_CR_STA                0x80
/*!
  I2C cmd: Stop Transmit
  */
#define I2C_CR_STP                0x40
/*!
  I2C writing command
  */
#define I2C_CR_WR                 0x20
/*!
  I2C reading command
  */
#define I2C_CR_RD                 0x10
/*!
  I2C cmd: Intterrupt acknowledge
  */
#define I2C_CR_IACK               0x08
/*!
  I2C cmd: Not acknowledge to slave
  */
#define I2C_CR_NACK               0x04

/*!
  I2C status: not recieve ACK from slave
  */
#define I2C_SR_RXNACK              0x80
/*!
  I2C status: I2c bus arbitrage invalidation flag
  */
#define I2C_SR_AL                 0x20
/*!
  I2C status: Read or Write transmit running flag
  */
#define I2C_SR_TIP                0x10
/*!
  I2C status: Interrupts Request flag
  */
#define I2C_SR_IFLAG              0x08
/*!
  define I2C delay time in us under polling mode
  */
#define I2C_TIME_OUT   20000

/*!
  define I2C default clock frequency in KHz
  */
#define I2C_DEF_CLK_KHZ 200
/*!
  Return code type. Please reference return code macro for the values.
  */
#define U8 S8

/*!
  Success return
  */
//#define 0 ((S8)0)       
/*!
  Fail for common reason
  */
#define ERR_FAILURE ((S8)-1)  
/*!
  Fail for waiting timeout
  */     
#define ERR_TIMEOUT ((S8)-2)      
/*!
  Fail for function param invalid
  */ 
#define ERR_PARAM ((S8)-3) 
/*!
  Fail for module status invalid
  */      
#define ERR_STATUS ((S8)-4)  
/*!
  Fail for module busy
  */     
#define ERR_BUSY ((S8)-5)
/*!
  Fail for no enough memory
  */
#define ERR_NO_MEM ((S8)-6)  
/*!
  Fail for no enough resource
  */     
#define ERR_NO_RSRC ((S8)-7)
/*!
  Fail for hardware error
  */ 
#define ERR_HARDWARE ((S8)-8) 
/*!
  Fail for feature not support
  */      
#define ERR_NOFEATURE ((S8)-9)       


#define CPU0_TI_INIT0 0x00010080
#define CPU0_TI_CW 0x000100a0

/*!
  define I2C delay function in us
  */
//#define I2C_DELAY_US mtos_task_delay_us

#define R_I2C_PRER_H      	0xbf158018
#define R_I2C_PRER_L         	0xbf15801c
#define R_I2C_TXR              	0xbf158008
#define R_I2C_RXR             	0xbf15800c
#define R_I2C_SR           		0xbf158004
#define R_I2C_CTR         		0xbf158010
#define R_I2C_CR         		0xbf158014


/*!
 * RTC enable register
 */
#define RTC_EN            0x2000
/*!
 * RTC param register
 */
#define RTC_PARAMETER_H   0x2004
/*!
 * RTC param register
 */
#define RTC_PARAMETER_L   0x2008
/*!
 * RTC status register
 */
#define RTC_STA           0x200C


#define WAKEUP_EN   	0x0054

#define LPM_POW_CTRL	0x0004

#define IR_INT_RAWSTA           0xBF151014


// command of read key input
#define FD650_GET_KEY 0x0700 // read key input and return the key value

#define FD650_DIG0 0x1400 // display data on seg 0(need 8-bits-data)
#define FD650_DIG1 0x1500 // display data on seg 1(need 8-bits-data)
#define FD650_DIG2 0x1600 // display data on seg 2(need 8-bits-data)
#define FD650_DIG3 0x1700 // display data on seg 3(need 8-bits-data)


//set the command of system parameter
#define FD650_BIT_ENABLE  0x01 // open/close bit
#define FD650_BIT_SLEEP   0x04 // sleep control bit
#define FD650_BIT_7SEG    0x08 // 7 segment control bit
#define FD650_BIT_INTENS1 0x10 // 1 level brightness
#define FD650_BIT_INTENS2 0x20 // 2 level brightness
#define FD650_BIT_INTENS3 0x30 // 3 level brightness
#define FD650_BIT_INTENS4 0x40 // 4 level brightness
#define FD650_BIT_INTENS5 0x50 // 5 level brightness
#define FD650_BIT_INTENS6 0x60 // 6 level brightness
#define FD650_BIT_INTENS7 0x70 // 7 level brightness
#define FD650_BIT_INTENS8 0x00 // 8 level brightness

#define FD650_SYSOFF 0x0400 // disable display and key input
#define FD650_SYSON  (FD650_SYSOFF | FD650_BIT_ENABLE) // enable display and key input
#define FD650_8SEG_ON (FD650_SYSON | 0x00) // enable 8seg mode
#define FD650_SYSON_4 (FD650_SYSON | FD650_BIT_INTENS4) // enable display, key input, 4 level brightness
#define FD650_SYSON_8 (FD650_SYSON | FD650_BIT_INTENS8) // enable display, key input, 8 level brightness

static S8 i2c_check_opdone_status(U8 i2c_id, U8 cmd)
{
  U8 data = 0;
 
  rd_port(R_I2C_SR,data);
  x_zero_delay();
  if(I2C_SR_AL == (data & I2C_SR_AL))
  {
    return ERR_HARDWARE;
  }

  if(cmd == I2C_CR_WR)
  {
    /* check if receive ACK from slave */
    if(I2C_SR_RXNACK == (data & I2C_SR_RXNACK)) /* bit 9 */
    {
      return ERR_FAILURE;
    }
  }

  return 0;
}

static S8 mcu_i2c_stop(U8 i2c_id)
{
  volatile U32 retry_cnt = 0;
  S8 ret = 0;
  U8 temp = 0;

  wr_port(R_I2C_CR, I2C_CR_STP);
   x_zero_delay();
  rd_port(R_I2C_CR,temp);
   x_zero_delay();
  while(temp != 0x04) // default value
  {
    if(retry_cnt == I2C_TIME_OUT)
    {
      /* time out */
      return ERR_HARDWARE;
    }
    retry_cnt++;
  }      

  /* check result status */
  ret = i2c_check_opdone_status(i2c_id, I2C_CR_STP);

  return ret;
}

static U8 mcu_i2c_wbyte(U8 i2c_id, U8 ucWdata, U8 bSetStart)
{
  U8 data = 0;
  volatile U32 retry_cnt = 0;
  S8 ret = 0;
  U8 temp = 0;
  #if 0
  //mtos_printk("i2c_wbyte id=%d, data=0x%x, start=%d\n",
  i2c_id, ucWdata, bSetStart);
  #endif
  wr_port(R_I2C_TXR, ucWdata);
  x_zero_delay();
  if(1 == bSetStart)
  {
    data = I2C_CR_WR | I2C_CR_STA; // Write|Start command.
  }
  else
  {
    data = I2C_CR_WR;  // Write command.
  }

  /* start write */
  wr_port(R_I2C_CR, data);
  x_zero_delay();

  rd_port(R_I2C_CR,temp);
  x_zero_delay();
  /* Wait write byte done. */ 
  while(temp != 0x04) // default value
  {
    if(retry_cnt == I2C_TIME_OUT)
    {
      /* time out */   
      return ERR_HARDWARE;
    }
    retry_cnt++;
  }    

  /* check result status */
  ret = i2c_check_opdone_status(i2c_id, I2C_CR_WR);

  return ret;
}

static S8 mcu_i2c_rbyte(U8 i2c_id, U8 *p_value, U8 nack)
{
  U8 data = 0;
  volatile U32 retry_cnt = 0;
  S8 ret = 0;
  U8 temp =0;
  ////mtos_printk("i2c_rbyte id=%d\n", i2c_id);
  if(1 == nack)
  {
    data = I2C_CR_RD | I2C_CR_NACK;  // Read data without ACK.
  }
  else
  {
    data = I2C_CR_RD;  // Read data with ACK.
  }

  /* start read */
  wr_port(R_I2C_CR, data);
  x_zero_delay();
  /* Wait read byte done. */ 
  while(1) // default value
  {
  	rd_port(R_I2C_CR,temp);
	x_zero_delay();
	if(temp == 0x04)
	      break;
    if(retry_cnt == I2C_TIME_OUT)
    {
      /* time out */
      return ERR_HARDWARE;
    }
    retry_cnt++;
  }  

  /* check result status */
  ret = i2c_check_opdone_status(i2c_id, I2C_CR_RD);

  #if I2C_DEBUG	
  wr_port(0x24, 0x11);
  x_zero_delay();
  wr_port(0x0054, 0x1);
  x_zero_delay();
  while(1);
  #endif
	
  if(0 == ret)
  {
    /* get read data */
    rd_port(R_I2C_RXR,temp);	
    x_zero_delay();
    #if I2C_DEBUG
    if(temp !=0)
    {
	wr_port(0x24, temp);
	x_zero_delay();
	wr_port(0x0054, 0x1);
	x_zero_delay();
	while(1);
     }	
     #endif
    *p_value = temp;
  }
  return ret;
}

static S8 mcu_i2c_concerto_raw_read_byte(U8  *p_data, U8 b_nack)
{ 
  return mcu_i2c_rbyte(2,p_data, b_nack);
}

static S8 mcu_i2c_concerto_raw_write_byte(U8 data, U8 b_start)
{  
 return mcu_i2c_wbyte(2,data, b_start);
}

static S8 mcu_i2c_concerto_raw_stop(void)
{
  return mcu_i2c_stop(2);
}

U8 mcu_i2c_raw_write_byte(U8 data, U8 b_start)
{
  U8 ret = ERR_NOFEATURE; 
    mcu_i2c_concerto_raw_write_byte(data, b_start);
  return ret;
}

U8 mcu_i2c_raw_read_byte(U8 *p_data, U8 b_nack)
{
	U8 ret = ERR_NOFEATURE; 
	ret = mcu_i2c_concerto_raw_read_byte(p_data, b_nack);
	return ret;
}

U8 mcu_i2c_raw_stop(void)
{
  U8 ret = ERR_NOFEATURE;
  mcu_i2c_concerto_raw_stop();   
  return ret;
}

static U8 mcu_fd650_read() //read the key value
{
	U8 keycode = 0;  
	mcu_i2c_raw_write_byte(((FD650_GET_KEY/128) & 0x3E) | 0x01 | 0x40, 1);
	mcu_i2c_raw_read_byte(&keycode, 1);
	mcu_i2c_raw_stop();

	if((keycode & 0x40) == 0)
	{
		keycode = 0;
	}

	return keycode;
}

static void mcu_fd650_write(U16 cmd) //write command
{  

#if 1
	U8 H_byte = (U8)(cmd/128);
	U8 L_byte = (U8)cmd;	

	#if 0
	//store to reseve reg 
	wr_port(0x24,(cmd>>7));
	//wakeup
	wr_port(0x0054, 0x1);	
	while(1);
	#endif
	
      mcu_i2c_raw_write_byte(((U8)(cmd/128) & 0x3E) | 0x40, 1);
      mcu_i2c_raw_write_byte((U8)cmd, 0);
      mcu_i2c_raw_stop();
#else
	U8 temp = 0;
	temp = (0x1400>>7);
#endif
}
#endif


void test_ao_gpio3_init(void)
{
	U8 tmp = 0;
	rd_port(AOGPIO3_SET_CFG+3,tmp); 
	tmp &= 0xF0;
	tmp |= 0X01;
	wr_port(AOGPIO3_SET_CFG+3,tmp);

	tmp = 0;
	rd_port(AOGPIO0_SET_DIRECTION,tmp); 
	tmp &= 0xF7;
	wr_port(AOGPIO0_SET_DIRECTION,tmp);

	tmp = 0;
	rd_port(AOGPIO0_SET_WRITEEN,tmp); 
	tmp |= 0x08;
	wr_port(AOGPIO0_SET_WRITEEN,tmp);

	tmp = 0;
	rd_port(AOGPIO0_SET_DATA,tmp); 
	tmp &= 0xF7;
	wr_port(AOGPIO0_SET_DATA,tmp);
}

void test_ao_gpio3_set(U8 value)
{
	U8 tmp = 0;
	rd_port(AOGPIO0_SET_DATA,tmp);
	if(value>0)
	{
		tmp |= 0x08;
	}
	else
	{
		tmp &= 0xF7;
	}
	wr_port(AOGPIO0_SET_DATA,tmp);
}

U8 parse_data (void)
{
	U8 i = 0;
	U8	AP_DATA[16] = {0};
	for (i=0; i<16; i++)
	{
		rd_port(LPM_MESSAGE2AO_0 + i, AP_DATA[i]);
	}
	bitstop = AP_DATA[3];
	status = AP_DATA[2];
	etu =  (AP_DATA[7] << 24) | (AP_DATA[6] << 16) | (AP_DATA[5] << 8) | AP_DATA[4];
	read_num = (AP_DATA[11] << 8) | AP_DATA[10];
	write_num = (AP_DATA[9] << 8) | AP_DATA[8]; 
	timeout = (AP_DATA[15] << 24) | (AP_DATA[14] << 16) | (AP_DATA[13] << 8) | AP_DATA[12];
	bitdelay = 56;
	
	if(AP_DATA[0] != 0)
	{
		return 1;
	}
	else
	{
		return 0;
	}
}


U8 check_IO_level()
{
	count = 0;
	while(1)
	{
		if(count > 1000)
		{
			return 0;
		}
		count++;
		rd_port(AOGPIO0_GET_DATA,temp);
		if(0x02== (temp & 0x02))
		{
			return 1;
		}
		aomcu_udelay(20);
	}
}

U8 readbyte ()
{
	rd_port(AOGPIO0_GET_DATA,temp);
	if(0x00 == (temp & 0x02))
	{
		aomcu_udelay(bitdelay);
		return 1;
	}
	return 0;
}

void writebyte ()
{
	temp &= 0xFD;
	wr_port(AOGPIO0_SET_DATA,temp);
}

U8 readerror ()
{
	U8 i = 0;

	for(i=0;i<8;i++)
	{
		rd_port(AOGPIO0_GET_DATA,temp);
		if(0x00 == (temp & 0x02))
		{
			return 1;
		}
		aomcu_udelay(2);
	}
	return 0;
}

void writeerror ()
{
	//aomcu_udelay(bitdelay);
	temp &= 0xFD;
	wr_port(AOGPIO0_SET_DATA,temp);
	aomcu_udelay(bitdelay+(bitdelay/2));
	temp |= 0x02;
	wr_port(AOGPIO0_SET_DATA,temp);
}

U8 smc_getstatus()
{
	U8 cur_status;
	rd_port(SMC0_STA,cur_status);
	if(0x00 == (cur_status & 0x02))
	{
		return 1;
	}
	return 0;
}

void smc_get_key(void)
{
	U8 u8KeyValue = 0;
	u8KeyValue = mcu_fd650_read();

	if (u8KeyValue != 0  && u8KeyValue != 0xff )
	{
		wr_port(LPM_MESSAGE2AO_3+2,u8KeyValue);
	}
}
void smc_cfg(U8 wrflag)
{
	if(1 == wrflag)
	{
		rd_port(AOGPIO0_SET_WRITEEN,temp); 
		temp |= 0x02;
		wr_port(AOGPIO0_SET_WRITEEN,temp);		
		rd_port(AOGPIO0_SET_DIRECTION,temp); 
		temp &= 0xFD;
		wr_port(AOGPIO0_SET_DIRECTION,temp);
		interrupt_flag = 1;
	}
	else
	{
		rd_port(AOGPIO0_SET_DIRECTION,temp); 
		temp |= 0x02;
		wr_port(AOGPIO0_SET_DIRECTION,temp);		
		rd_port(AOGPIO0_SET_WRITEEN,temp); 
		temp &= 0xFD;
		wr_port(AOGPIO0_SET_WRITEEN,temp);
		interrupt_flag = 0;
	}
}


void smc_write_reg(U32 offset,U8 value)
{
	U32 offset_tmp = 0;
	U8 tmp_ontvalue = 0;
	U8 tmp_prdvalue = 0;
	offset_tmp  = offset;
	if(0 == (offset&0x01))
	{
		offset_tmp = offset_tmp/2;
		offset_tmp = offset_tmp*8;
		rd_port(IRWF_ONTEST_BASE+offset_tmp,tmp_ontvalue);
		rd_port(IRWF_PRDEST_BASE+offset_tmp,tmp_prdvalue);
		wr_port(IRWF_ONTEST_BASE+offset_tmp,value);
		wr_port(IRWF_PRDEST_BASE+offset_tmp,tmp_prdvalue);
	}
	else
	{
		offset_tmp = offset_tmp + 1;
		offset_tmp = offset_tmp/2;
		offset_tmp = offset_tmp -1;
		offset_tmp = offset_tmp*8;
		rd_port(IRWF_ONTEST_BASE+offset_tmp,tmp_ontvalue);
		rd_port(IRWF_PRDEST_BASE+offset_tmp,tmp_prdvalue);
		wr_port(IRWF_ONTEST_BASE+offset_tmp,tmp_ontvalue);
		wr_port(IRWF_PRDEST_BASE+offset_tmp,value);
	}
}

void smc_write_16reg(U32 offset)
{
	if(offset < IRWF_MIN_BASE)
	{
		return;
	}
	wr_port(offset,smc_irdata[0]);
	wr_port(offset+2,smc_irdata[1]);
	wr_port(offset+4,smc_irdata[2]);
	wr_port(offset+6,smc_irdata[3]);
}

U8 smc_read_reg(U32 offset)
{
	U32 offset_tmp = 0;
	U8 tmp_ontvalue = 0;
	U8 tmp_prdvalue = 0;
	U8 value = 0;
	offset_tmp  = offset;
	if(0 == (offset&0x01))
	{
		offset_tmp = offset_tmp/2;
		offset_tmp = offset_tmp*8;
		rd_port(IRWF_ONTEST_BASE+offset_tmp,tmp_ontvalue);
		rd_port(IRWF_PRDEST_BASE+offset_tmp,tmp_prdvalue);
		value = tmp_ontvalue;
	}
	else
	{
		offset_tmp = offset_tmp + 1;
		offset_tmp = offset_tmp/2;
		offset_tmp = offset_tmp -1;
		offset_tmp = offset_tmp*8;
		rd_port(IRWF_ONTEST_BASE+offset_tmp,tmp_ontvalue);
		rd_port(IRWF_PRDEST_BASE+offset_tmp,tmp_prdvalue);
		value = tmp_prdvalue;
	}
	return value;
}

U8 smc_write(U8 cmd,U32 num)
{
	U8 value = 0;
	U8 curflag = 0;
	U32 write_offset = 0;
	check = 0;
	bitcount = 0;
	ready = 0;
	cur_num = 0;
	count = 0;
	flag = 0;
	interruptstatus = 0;
	if(cmd)
	{
		write_offset = 0;
		smc_wdata[1]  = smc_read_reg(write_offset+1);
	}
	else
	{
		write_offset = 5;
	}
	
	while(1)
	{
		if(!smc_getstatus())
		{
			return 0;
		}
		if((1 != interruptstatus) && (cur_num < num) && (0 == ready))
		{
			curflag++;
			if((irbuffaddr >= IRWF_MIN_BASE) && (curflag>20))
			{
				curflag = 0;
				rd_port(IRINT_RAWSTA_BASE,value);
				if(value&0x01)
				{
					rd_port(IRNEC_DATA_BASE,smc_irdata[0]);
					rd_port(IRNEC_DATA_BASE+1,smc_irdata[1]);
					rd_port(IRNEC_DATA_BASE+2,smc_irdata[2]);
					rd_port(IRGLOBAL_STA_BASE,smc_irdata[3]);
					smc_write_16reg(irbuffaddr);
					irbuffaddr -= 8; 
					ircount++;
				}
			}
			smc_wdata[0] = smc_read_reg(write_offset++);
			interruptstatus = 1;
			bitcount = 0;
			check = 0;
			writebyte();
			TMR0 =etu;
			INTCON = 0xa0;
			count = 0;
		}
		if(count > 8000)
		{
			wr_port(LPM_MESSAGE2AO_3,3);
			return 0;
		}
		if(1 == ready)
		{
			INTCON = 0x80;
			cur_num++;
			ready = 0;
			count = 0;
#if 0			
			aomcu_udelay(bitdelay);
			smc_cfg(0);
			if(readerror())
			{
				cur_num--;
			}
			smc_cfg(1);
			aomcu_udelay(1360);
#endif		
			//aomcu_udelay(1580);
		}
		if(cur_num == num)
		{
			INTCON = 0x80;
			wr_port(LPM_MESSAGE2AO_3,9);
			flag = 0xaa;
			return 1;
		}
		count++;
		aomcu_udelay(2);
	}
}

U8 smc_read(unsigned int ins,unsigned int num)
{
	U8 value = 0;
	check = 0;
	bitcount = 0;
	ready = 0;
	cur_num= data_offset;
	count = 0;
	flag = 0;
	timetick = 0;
	interruptstatus = 0;
	while(1)
	{
		if(!smc_getstatus())
		{
			return 0;
		}
 #if  FD650_TEST
		if(count > 8000)
		{
			timetick++;
			count = 0;
			
			if(0 == (timetick%10))
			{
				smc_get_key();
				if(irbuffaddr >= IRWF_MIN_BASE)
				{
					rd_port(IRINT_RAWSTA_BASE,value);
					if(value&0x01)
					{
						rd_port(IRNEC_DATA_BASE,smc_irdata[0]);
						rd_port(IRNEC_DATA_BASE+1,smc_irdata[1]);
						rd_port(IRNEC_DATA_BASE+2,smc_irdata[2]);
						rd_port(IRGLOBAL_STA_BASE,smc_irdata[3]);
						smc_write_16reg(irbuffaddr);
						irbuffaddr -= 8;
						ircount++;
					}
				}
			}
			if(timetick>timeout)
			{
				wr_port(LPM_MESSAGE2AO_3,1);
				return 0;
			}
		}
#endif
		if(1 == ready)
		{
			INTCON = 0x80;
			ready = 0;
			if(1 == (check&0x01))
			//if(1)
			{

				smc_data[0] |= bitdata[0] << 7;
				smc_data[0] |= bitdata[1] << 6;
				smc_data[0] |= bitdata[2] << 5;
				smc_data[0] |= bitdata[3] << 4;
				smc_data[0] |= bitdata[4] << 3;
				smc_data[0] |= bitdata[5] << 2;
				smc_data[0] |= bitdata[6] << 1;
				smc_data[0] |= bitdata[7] ;
				smc_data[0] = ~smc_data[0]; 
				if(0x00 == ins)
				{
					smc_write_reg(cur_num,smc_data[0]);
					smc_data[0] = 0;
				}

				if(cur_num>0)
				{
					if((0x90 == smc_read_reg(cur_num-1)) && (0x00 == smc_data[0]))
					{
						wr_port(LPM_MESSAGE2AO_3,12);
						flag = 0xaa;
						cur_num++;
						return 1;
					}
				}
				cur_num++;
			}
			else
			{
				smc_cfg(1);
				writeerror();
				smc_cfg(0);
			}
			check = 0;
			count = 0;
			if(cur_num == num)
			{
				INTCON = 0x80;
				wr_port(LPM_MESSAGE2AO_3,9);
				flag = 0xaa;
				return 1;
			}
		}
		if((1 != interruptstatus) && (1 == readbyte()))
		{
			ibit = 0;
			interruptstatus = 1;
			bitcount = 10;
			TMR0 =etu;
			INTCON = 0xa0;
			
		}
		count++;
		aomcu_udelay(3);
	}
}

void smc_save_data(U8 bytenum)
{
	U8 i = 0;
	U8 j = 0;
	U8 offset = 0;
	U8 totalcount = 0;
	if(0 == (bytenum&0x01))
	{
		totalcount = bytenum/2;
		
	}
	else
	{
		totalcount = (bytenum+1)/2;
	}
	do
	{
		offset = j *8;
		wr_port(IRWF_ONTEST_BASE+offset,smc_data[i]);
		wr_port(IRWF_PRDEST_BASE+offset,smc_data[i+1]);
		i+=2;
		j++;
		if(0 != totalcount)
		{
			totalcount--;
		}
	}
	while(totalcount>0);
	for(i = 0;i<bytenum;i++)
	{
		smc_data[i]=0x00;
	}
}
void sys_init( void );
void main(void ) 
{
	U8 i = 0;
	U8 ret = 0;
	U8 MoreProcedure = 0;

#if  FD650_TEST
	mcu_fd650_write(FD650_SYSON_4 | FD650_8SEG_ON);
#endif
//	test_ao_gpio3_init();
	count = 0;
	wr_flag = parse_data();
	sys_init();
	if(bitstop == 1)
	{
		bitstop = 10;
	}
	else
	{
		bitstop = 11;
	}
		
	flag = 0;
	readins = 0;
 #if  FD650_TEST
	mcu_fd650_write(FD650_SYSON_4 | FD650_8SEG_ON);
 #endif
	rd_port(AOGPIO0_SET_CFG,temp); 
	temp &= 0xF0;
	temp |= 0x01;
	wr_port(AOGPIO0_SET_CFG,temp);
	temp = 0;
#if 1	
	//test_ao_gpio3_set(0);
	if(wr_flag == 1)
	{
		if(write_num>=SMC_T0_CMD_LEN)
		{

			if(write_num == SMC_T0_CMD_LEN)
			{
				if(0 == smc_read_reg(4))
				{
					read_num = 255;
				}
				else
				{
					read_num =  smc_read_reg(4);
				}
			}
			else
			{
				read_num = 2;
			}
			write_num = write_num -SMC_T0_CMD_LEN;
			smc_cfg(1);
			ret = smc_write(1,SMC_T0_CMD_LEN);
			if(1 == ret)
			{
				smc_cfg(0);
				//aomcu_udelay(1580);
				do{
					if(smc_read(1,1))
					{
							//wr_port(IRWF_ONTEST_BASE+104,0x99);
							//wr_port(IRWF_PRDEST_BASE+104,smc_data[0]);
						if(((smc_wdata[1] ^ smc_data[0]) & 0xfe) == 0x00)
						{
							
							readins = 1;
							break;
						}
						else if(((smc_wdata[1] ^ smc_data[0]) & 0xfe) == 0xfe)
						{
							if(write_num)
							{
								write_num = 1;
							}
							else if(0 == read_num)
							{
								MoreProcedure = 1;
								continue;
							}
							else
							{
								read_num = 1;
							}
							readins = 1;
							break;
						}
						else if(0x60 == smc_data[0])
						{
							MoreProcedure = 1;
							continue;
						}
						else  if (((smc_data[0] & 0xf0) == 0x60) || ((smc_data[0] & 0xf0) == 0x90))
						{
							smc_write_reg(0,smc_data[0]);
							data_offset = 1;
							read_num = 2;
							readins = 2;
							break;
						}
					}
				}while(MoreProcedure);
				if(2 == readins)
				{
					smc_read(0,1);
					//wr_port(IRWF_ONTEST_BASE+136,0xDD);
					//wr_port(IRWF_PRDEST_BASE+136,read_num);
				}
				//wr_port(IRWF_ONTEST_BASE+112,0xCC);
				//wr_port(IRWF_PRDEST_BASE+112,read_num);
				if((1 == readins) && write_num)
				{
					readins = 0;
					//wr_port(IRWF_ONTEST_BASE+120,0xBB);
					//wr_port(IRWF_PRDEST_BASE+120,read_num);
					smc_data[0] = 0;
					smc_cfg(1);
					//aomcu_udelay(1560);
					aomcu_udelay(1500);
					ret = smc_write(0,write_num);
					if(1 == ret)
					{
						smc_cfg(0);
						//aomcu_udelay(1560);
						smc_read(0,2);
					}
				}
				if((1 == readins) && read_num)
				{
					readins = 0;
					smc_data[0] = 0;
					read_num+=2;
					//wr_port(IRWF_ONTEST_BASE+128,0xAA);
					//wr_port(IRWF_PRDEST_BASE+128,read_num);
					smc_read(0,read_num);
				}
			}
		}
	}
	else
	{
		smc_cfg(0);
		smc_read(0,read_num);
	}
#endif
#if  FD650_TEST
	smc_get_key();
#endif
#if 0
	//smc_save_data(cur_num);
	wr_port(IRWF_ONTEST_BASE+64,read_num);
	wr_port(IRWF_PRDEST_BASE+64,write_num);
	wr_port(IRWF_ONTEST_BASE+72,bitstop);
	wr_port(IRWF_PRDEST_BASE+72,timeout);
	
	wr_port(IRWF_ONTEST_BASE+80,wr_flag);
	wr_port(IRWF_PRDEST_BASE+80,etu);
	wr_port(IRWF_ONTEST_BASE+88,cur_num);
	wr_port(IRWF_PRDEST_BASE+88,flag);
	wr_port(IRWF_ONTEST_BASE+96,smc_wdata[0]);
	wr_port(IRWF_PRDEST_BASE+96,smc_wdata[1]);
	//wr_port(IRWF_ONTEST_BASE+104,smc_wdata[2]);
	//wr_port(IRWF_PRDEST_BASE+104,smc_wdata[3]);
	//wr_port(IRWF_ONTEST_BASE+112,smc_wdata[4]);
	//wr_port(IRWF_PRDEST_BASE+112,0x3A);
	
	//smc_write_16reg(IRWF_ONTEST_BASE+1008);
#endif
#if  FD650_TEST
	if(irbuffaddr >= IRWF_MIN_BASE)
	{
		rd_port(IRINT_RAWSTA_BASE,ret);
		if(ret&0x01)
		{
			rd_port(IRNEC_DATA_BASE,smc_irdata[0]);
			rd_port(IRNEC_DATA_BASE+1,smc_irdata[1]);
			rd_port(IRNEC_DATA_BASE+2,smc_irdata[2]);
			rd_port(IRGLOBAL_STA_BASE,smc_irdata[3]);
			smc_write_16reg(irbuffaddr);
			irbuffaddr -= 8;
			ircount++;
		}
	}
#endif
	wr_port(LPM_MESSAGE2AO_3+1,ircount);
	wr_port(LPM_MESSAGE2AO_2,read_num);
	check_IO_level();
	
	
	rd_port(AOGPIO0_SET_INTERRUPTEN,temp); 
	temp |= 0x01;
	wr_port(AOGPIO0_SET_INTERRUPTEN,temp);	

	check_IO_level();
	//test_ao_gpio3_set(0);
	

	rd_port(AOGPIO0_SET_CFG,temp); 
	temp &= 0xF0;
	wr_port(AOGPIO0_SET_CFG,temp);	

	temp |= 0x03;
	wr_port(AOGPIO0_SET_CFG,temp);	

	//test_ao_gpio3_set(1);
	wr_port(LPM_AOMCU_EN,0);
	

}



//---------------------------------------------------------------------------------
// Function name: sys_init()
// Description  : Initialize mcu and ledkb register.
// Parameters   : None.
// Return       : None.
// Notes        : None.
//--------------------------------------------------------------------------------- 

void sys_init(void)
{
    U8 u8Temp ;
    u8Temp = 0;
    tick = 0;
    ctick = 0;

    //**********************************
    // Reset the chip core
    //**********************************
//wr_port((MCU_REG_BASE + RESET_TO_CORE),CORE_RESET_ENABLE); //unused
    //**********************************
    // Disable Interrupt
    //**********************************
    GIE =  0;   // Global Interrupt
    INTE    = 0b00000000 ; // disable INTE_B7~INTE_B0
    INTE_B8 = 0 ;
    INTE_B9 = 0 ;
    // set OPTION_reg for TMR0
    T0IE = 0;

    //**********************************
    // Watchdog Timer (WDT)
    //**********************************
    // wdt prescale 1 : 2048
    WDTPS3  = 1 ;
    WDTPS2  = 0 ;
    WDTPS1  = 1 ;
    WDTPS0  = 0 ;
    // wdt enable
    WDTE    = 0 ;

    //**********************************
    // TMR0 Register
    //**********************************
    // claer tmr0 and prescale counter
    TMR0 =etu;
    // Prescaler 1:4096
    PS3  = 1;
    PS2  = 0;
    PS1  = 1;
    PS0  = 1;
    // TMR0 Enable, tmr0 start
    T0CS = 1; // = 1 ,off ; =0 , on


    OPTION=0x04;          // Prescaler :256                           //add for test


    //**********************************
    // enable Interrupt
    //**********************************
    INTE    = 0b00000110 ; // enable INTE_B2~1
    GIE =  1;   // enable Global Interrupt



       // enable Interrupt
    //**********************************
    INTE = 0b00001110; // enable INTE_B3~1

    	
    INTCON = 0x80;                                                //add for test
    GIE = 1;    // enable Global Interrupt
}

/*
Internal clock: 0x84MHz?
Clock cycle : 1/0x84 = 1.19us

首先1.19us时钟周期4分频后变成4.76us指令周期；

然后预分频器 2 分频后变成 9.52us周期 供给定时器；

定时器每隔9.52us加一 ，加到256次  256X9.52us=2437us溢出中断 ；

*/

void interrupt ISR(void) {
	U8 U8data;//, U8temp4;
  U8 tmp_export_address_l,tmp_export_address_h;
  tmp_export_address_l = EXPORT_ADDRESS_L ;
  tmp_export_address_h = EXPORT_ADDRESS_H ;

	if(1 != interrupt_flag)
	{
		if((1 == ready) || (0  == bitcount) || (0xaa == flag))
		{
			TMR0 =0;
			T0IF = 0; 
			EXPORT_ADDRESS_L = tmp_export_address_l;
	  		EXPORT_ADDRESS_H = tmp_export_address_h;
			return;
		}
	
		bitcount--;
		if(0 == bitcount)
		{
			ready = 1;
			interruptstatus = 0;
			TMR0 =0;
			T0IF = 0; 
			EXPORT_ADDRESS_L = tmp_export_address_l;
			EXPORT_ADDRESS_H = tmp_export_address_h;
			return;
		}
		rd_port(AOGPIO0_GET_DATA,temp);
		bitdata[ibit] = (temp >> 1) & 0x01;
		if(1 == bitdata[ibit])
		{
			check++;
			//test_ao_gpio3_set(1);
		}
		else
		{
			//test_ao_gpio3_set(0);
		}
		ibit++;
	}
	else
	{
		if((1 == ready) || (bitstop  < bitcount) || (0xaa == flag))
		{
			TMR0 =0;
			T0IF = 0; 
			EXPORT_ADDRESS_L = tmp_export_address_l;
	  		EXPORT_ADDRESS_H = tmp_export_address_h;
			return;
		}
	
		if(bitcount<8)
		{
			if(smc_wdata[0] & (0x80>>bitcount))
			{
				//temp |= 0x02;
				//check++;
				temp &= 0xFD;
				//test_ao_gpio3_set(0);
				
			}
			else
			{
				check++;
				temp |= 0x02;
				//temp &= 0xFD;
				//test_ao_gpio3_set(1);
			}
		}
		else if(8 == bitcount)
		{
			if(check&0x01)
			{
				//temp |= 0x02;
				temp &= 0xFD;
				//test_ao_gpio3_set(0);
			}
			else
			{
				//temp &= 0xFD;
				temp |= 0x02;
				//test_ao_gpio3_set(1);
			}
		}
		else
		{
			temp |= 0x02;
			//test_ao_gpio3_set(1);
		}
		wr_port(AOGPIO0_SET_DATA,temp);
		bitcount++;
		if(bitstop < bitcount)
		{
			ready = 1;
			interruptstatus = 0;
			TMR0 =0;
			T0IF = 0; 
			EXPORT_ADDRESS_L = tmp_export_address_l;
			EXPORT_ADDRESS_H = tmp_export_address_h;
			return;
		}
	}
	
	TMR0 =etu; 
	T0IF = 0; //clear interupt
  
  EXPORT_ADDRESS_L = tmp_export_address_l;
  EXPORT_ADDRESS_H = tmp_export_address_h;
}


 
