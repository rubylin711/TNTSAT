/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/


#ifndef _PIC16F7X_H
#define _PIC16F7X_H


static volatile unsigned char   INDF    @ 0x00; /* TMR0 Register, configed by OPTION-reg       */

static volatile unsigned char   TMR0    @ 0x01; /* TMR0 Register, configed by OPTION-reg       */
static volatile unsigned char   PCL     @ 0x02; /* PC Lower 8 bits                             */
static volatile unsigned char   STATUS  @ 0x03; /* STATUS Register                             */
static          unsigned char   FSR     @ 0x04; /* Indirect data memory address pointer        */
static          unsigned char   PORTB   @ 0x06; /* output port for debug */

static          unsigned char   PCLATH  @ 0x0A; /* Write Buffer for the upper 5 bits of the PC */

/*
 * External interrupt flags, INTF must be cleared in software in the Interrupt
 * Service Routine(ISR) before re-enabling this interrupt.
 * INTF are set regardless of the status of the INTE and GIE, so In ISR should
 * access INTF and INTE to judged which interrupt vector occur.
 *
 */
static volatile unsigned char   INTCON  @ 0x0B; /* Register for interrupts */
static volatile unsigned char   INTF    @ 0x0C;
/* interrupt can be disabled by clearing enable bit INTE */
static          unsigned char   INTE    @ 0x0D;
/* to configure the TMR0 prescaler */
static          unsigned char   OPTION  @ 0x0E;
static          unsigned char   OPTION2 @ 0x0F;


/* Watchdog Timer (WDT), ReadOnly , clear used clrwdt */
static volatile unsigned char   WDT               @ 0x10;

/* EXTERNAL PORT REGINSTER  */
static          unsigned char   EXPORT_ADDRESS_L  @ 0x11; /* ex-port address <7:0>  */
static          unsigned char   EXPORT_ADDRESS_H  @ 0x12; /* ex-port address<15:8>  */

static          unsigned char   EXPORT_RDATA_EN   @ 0x15; /* bit<0> , 1: read enable */
static volatile unsigned char   EXPORT_RDATA      @ 0x16; /* input data, read only   */

static          unsigned char   EXPORT_WDATA      @ 0x19; /* output data */


/*  STATUS bits     */
/* IRP: Register Bank Select bit (used for indirect addressing) */
static volatile bit     IRP             @ (unsigned)&STATUS*8+7;
/* RP1:RP0: Register Bank Select bits (used for direct addressing) */
static volatile bit     RP1             @ (unsigned)&STATUS*8+6;
static volatile bit     RP0             @ (unsigned)&STATUS*8+5;
/* TO&PD(STATUS<4:3>) NOT USED */

static volatile bit     TO              @ (unsigned)&STATUS*8+4;
/* ZERO: Zero bit, 1 = The result is zero  */
static volatile bit     ZERO            @ (unsigned)&STATUS*8+2;
/* DC: Digit carry/borrow bit */
static volatile bit     DC              @ (unsigned)&STATUS*8+1;
/* CARRY: Carry/borrow bit */
static volatile bit     CARRY           @ (unsigned)&STATUS*8+0;

/*
 * INTCON
 *  GIE  : Global Interrupt Enable bit, 1 = Enables all un-masked interrupts
 *  T0IE : TMR0 Overflow Interrupt Enable bit,1 = Enables the TMR0 interrupt
 *  T0IF : TMR0 Overflow Interrupt Flag bit, 1 = TMR0 overflowed
 */
static volatile bit     GIE             @ (unsigned)&INTCON*8+7;
static volatile bit     T0IE            @ (unsigned)&INTCON*8+5;
static volatile bit     T0IF            @ (unsigned)&INTCON*8+2;

/*
 * OPTION2 bits
 * - T0CS: WDT Clock Source Select bit
 *    1 = WDT Enable
 *    0 = Disable
 *
 * - WDTPS3:WDTPS0: Prescaler Rate Select bits
 *  Bit Value    TMR0 Rate
 *   0000          1 : 2
 *   0001          1 : 4
 *   0010          1 : 8
 *   0011          1 : 16
 *   0100          1 : 32
 *   0101          1 : 64
 *   0110          1 : 128
 *   0111          1 : 256
 *   1000          1 : 512
 *   1001          1 : 1024
 *   1010          1 : 2048
 *   1011          1 : 4096
 *   others        1 : 1
 */
static bit  WDTE       @ (unsigned)&OPTION2*8+4;
static bit  WDTPS3     @ (unsigned)&OPTION2*8+3;
static bit  WDTPS2     @ (unsigned)&OPTION2*8+2;
static bit  WDTPS1     @ (unsigned)&OPTION2*8+1;
static bit  WDTPS0     @ (unsigned)&OPTION2*8+0;

/*
 * OPTION bits
 * - T0CS: TMR0 Clock Source Select bit
 *    1 = TMR0 not used
 *    0 = Internal instruction cycle clock
 *
 * - PS3:PS0: Prescaler Rate Select bits
 *  Bit Value    TMR0 Rate
 *   0000          1 : 2
 *   0001          1 : 4
 *   0010          1 : 8
 *   0011          1 : 16
 *   0100          1 : 32
 *   0101          1 : 64
 *   0110          1 : 128
 *   0111          1 : 256
 *   1000          1 : 512
 *   1001          1 : 1024
 *   1010          1 : 2048
 *   1011          1 : 4096
 *   1100          1 : 8192
 *   others        1 : 1
 */
static bit  T0CS    @ (unsigned)&OPTION*8+5;
static bit  PS3     @ (unsigned)&OPTION*8+3;
static bit  PS2     @ (unsigned)&OPTION*8+2;
static bit  PS1     @ (unsigned)&OPTION*8+1;
static bit  PS0     @ (unsigned)&OPTION*8+0;



/* INTF External interrupt flags */
static volatile bit  INTF_B9    @ (unsigned)&INTCON*8+1;
static volatile bit  INTF_B8    @ (unsigned)&INTCON*8+0;

static volatile bit  INTF_B7    @ (unsigned)&INTF*8+7;
static volatile bit  INTF_B6    @ (unsigned)&INTF*8+6;
static volatile bit  INTF_B5    @ (unsigned)&INTF*8+5;
static volatile bit  INTF_B4    @ (unsigned)&INTF*8+4;
static volatile bit  INTF_B3    @ (unsigned)&INTF*8+3;
static volatile bit  INTF_B2    @ (unsigned)&INTF*8+2;
static volatile bit  INTF_B1    @ (unsigned)&INTF*8+1;
static volatile bit  INTF_B0    @ (unsigned)&INTF*8+0;

/* INTE  interupt_enable bits */
static bit  INTE_B9    @ (unsigned)&INTCON*8+4;
static bit  INTE_B8    @ (unsigned)&INTCON*8+3;

static bit  INTE_B7    @ (unsigned)&INTE*8+7;
static bit  INTE_B6    @ (unsigned)&INTE*8+6;
static bit  INTE_B5    @ (unsigned)&INTE*8+5;
static bit  INTE_B4    @ (unsigned)&INTE*8+4;
static bit  INTE_B3    @ (unsigned)&INTE*8+3;
static bit  INTE_B2    @ (unsigned)&INTE*8+2;
static bit  INTE_B1    @ (unsigned)&INTE*8+1;
static bit  INTE_B0    @ (unsigned)&INTE*8+0;


#endif  /* _PIC16F7X_H */
