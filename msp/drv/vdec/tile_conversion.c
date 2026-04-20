/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2019 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#include <linux/printk.h>

#include "mt_mach/symphony_regs.h"
#include "mt_mach/symphony_io.h"

#ifndef MIN
#define MIN(a, b)	((a)<=(b)?(a):(b))
#endif

#define NBITS(n)        ((1L << (n)) - 1)
#define GETNBITS(x, n)     ((x) & NBITS((n)))

inline unsigned int jointbits(unsigned int x0, unsigned int x1, int nstart) { return (x0 | (x1<<nstart)); }

inline unsigned int selectbits(unsigned int x, int highbit, int lowbit)
{
//    unsigned int x1;
    int nbit;
    nbit = highbit - lowbit + 1;

//    x1 = x >> lowbit;
    return GETNBITS(x>>lowbit, nbit) ;
}

/* old not used */
#if 0
unsigned int joint(int x,int y,int ry1, int ry0, int rx1, int rx0, int bx, int by, int cy1, int cy0, int cx1,int cx0,int y0, int x0, int op)
{
    unsigned int addr;
    unsigned int row_y, row_x, bank_y, bank_x, col_y, col_x, byte_y, byte_x;
    unsigned int row, bank, col;
    int nc, nb, nr, nbyte, ncx, ncy, nbx, nby, nrx, nry;

    row_y=selectbits(y,ry1,ry0);
    row_x=selectbits(x,rx1,rx0);
    bank_y=selectbits(y,by,by);
    bank_x=selectbits(x,bx,bx);
    col_y=selectbits(y,cy1,cy0);
    col_x=selectbits(x,cx1,cx0);
    byte_y = y0 == 1 ? selectbits(y,0,0) : 0;
    byte_x= x0 == 1 ? selectbits(x,0,0) : 0;

    if(op == 0)
    {
        nbyte = x0==1 ? 1 : 0;
        ncx = cx1-cx0+1; ncy = cy1 - cy0 + 1;	nc = ncx+ncy;
        col=jointbits(col_x, col_y, ncx);
        nbx = 1; nby = 1;	nb = nbx+nby;
        bank=jointbits(bank_y, bank_x, nbx);
        nrx = rx1-rx0+1; nry = ry1-ry0 + 1;	nr = nrx + nry;
        row=jointbits(row_x,row_y,nrx);

        if(nbyte == 0)
        {
            addr = jointbits(col, bank, nc);
            addr = jointbits(addr, row, nc+nb);
        }
        else if(nbyte == 1)
        {
            addr = jointbits(byte_x, col, 1);
            addr = jointbits(addr, bank, 1+nc);
            addr = jointbits(addr, row, 1+nc+nb);
        }
    }
    else
    {
        bank_y = bank_y ^ byte_y;

        nbyte = x0==1 ? 1 : 0;
        ncx = cx1-cx0+1; ncy = cy1 - cy0 + 1;	nc = ncx+ncy;
        col=jointbits(col_x, col_y, ncx);
        nbx = 1; nby = 1;	nb = nbx+nby;
        bank=jointbits(bank_y, bank_x, nbx);
        nrx = rx1-rx0+1; nry = ry1-ry0 + 1;	nr = nrx + nry;
        row=jointbits(row_x,row_y,nrx);

        if(nbyte == 0)
        {
            addr = jointbits(col, bank, nc);
            addr = jointbits(addr, row, nc+nb);
        }
        else if(nbyte == 1)
        {
            addr = jointbits(byte_x, col, 1);
            addr = jointbits(addr, bank, 1+nc);
            addr = jointbits(addr, row, 1+nc+nb);
        }
    }

	return addr;
}

// input
// x: pixel_col, for luma, in pixel;  for chroma, in pixel/2;
// y: pixel_row, for luma, in line of a frame picture, or an interleaved luma field-frame format
//				for chroma, in line of a frame chroma data, or an interleaved chroma field-frame format
// dq_width_mode: 0 -- 8 bits ddr, 1 -- 16 bits ddr
// col_size_mode: 0 -- 512; 1 -- 1024; 2 -- 2048; 3 -- 4096
// output addr: address offset in byte
// minimum linear pixels storage: 32 pixels

unsigned int ddr_tile_map(int x, int y, int hd_map_mode, int col_size_mode, int field_picture)
{
    unsigned int mode;
	unsigned addr;

    mode = (hd_map_mode << 2) | col_size_mode;

    if(field_picture == 0)  // progressive picture
    {
        switch (mode)
        {
            case 1: addr = joint(x,y,10,6,9,6,5,5,4,0,4,0,0,0,0); break;
            case 2: addr = joint(x,y,10,6,9,7,6,5,4,0,5,0,0,0,0); break;
            case 3: addr = joint(x,y,10,7,9,7,6,6,5,0,5,0,0,0,0); break;
            case 5: addr = joint(x,y,10,6,10,6,5,5,4,0,4,0,0,0,0); break;
            case 6: addr = joint(x,y,10,6,10,7,6,5,4,0,5,0,0,0,0); break;
            case 7: addr = joint(x,y,10,7,10,7,6,6,5,0,5,0,0,0,0); break;
            default: break;
        }
    }
    else		// interlaced picture
    {
        switch (mode)
        {
            case 1: addr = joint(x,y,10,6,9,6,5,6,5,1,4,0,0,0,1); break;
            case 2: addr = joint(x,y,10,6,9,7,6,6,5,1,5,0,0,0,1); break;
            case 3: addr = joint(x,y,10,7,9,7,6,7,6,1,5,0,0,0,1); break;
            case 5: addr = joint(x,y,10,6,10,6,5,6,5,1,4,0,0,0,1); break;
            case 6: addr = joint(x,y,10,6,10,7,6,6,5,1,5,0,0,0,1); break;
            case 7: addr = joint(x,y,10,7,10,7,6,7,6,1,5,0,0,0,1); break;
            default: break;
        }
    }

    return addr;
}
#endif

#if 1
unsigned int joint_trio(int x,int y,int ry1, int ry0, int rx1, int rx0, int bx1, int bx0, int by1, int by0, int cy1, int cy0, int cx1,int cx0,int rowjump, int jh, int jl, int rh, int rl, int op)
{
    unsigned int addr;
    unsigned int bank_y, bank_x, col_y, col_x, byte_y, byte_x;
    unsigned int bank, col;
    int nc, nb, nbyte, ncx, ncy, nbx, nby;
    int mop1, mop2, addop, rowout_temp, rowout, rowout_1;

    mop1  =selectbits(y,ry1,ry0);
    addop =selectbits(x,rx1,rx0);
    mop2  =selectbits(rowjump,jh,jl);
    rowout_temp = mop1*mop2;
    rowout = selectbits(rowout_temp,9,0) + addop;
    rowout_1 = selectbits(rowout,rh,rl);
    bank_y=selectbits(y,by1,by0);
    bank_x=selectbits(x,bx1,bx0);
    col_y=selectbits(y,cy1,cy0);
    col_x=selectbits(x,cx1,cx0);
    byte_y= selectbits(y,0,0);
    byte_x= selectbits(x,0,0);

    if(op == 0)
    {
        nbyte = 0;
        ncx = cx1-cx0+1; ncy = cy1 - cy0 + 1; nc = ncx+ncy;
        col=jointbits(col_x, col_y, ncx);
        nbx = bx1-bx0+1; nby = by1-by0+1; nb = nbx+nby;
        bank=jointbits(bank_y, bank_x, nby);

        addr = jointbits(col, bank, nc);
        addr = jointbits(addr, rowout_1, nc+nb);
    }
    else
    {
        bank_y = bank_y ^ byte_y;

        nbyte = 0;
        ncx = cx1-cx0+1; ncy = cy1 - cy0 + 1; nc = ncx+ncy;
        col=jointbits(col_x, col_y, ncx);
        nbx = bx1-bx0+1; nby = by1-by0+1; nb = nbx+nby;
        bank=jointbits(bank_y, bank_x, nby);

        addr = jointbits(col, bank, nc);
        addr = jointbits(addr, rowout_1, nc+nb);
    }

    return addr;
}

unsigned int joint_trio_1(int x,int y,int ry1, int ry0, int rx1, int rx0,
                          int bx1, int bx0, int by1, int by0, int by2,
                          int cy1, int cy0, int cx1,int cx0,
                          int rowjump, int jh, int jl,
                          int rh, int rl)
{
    unsigned int addr;
    unsigned int bank_y, bank_x, col_y, col_x, byte_y, byte_x, bank_y_0, bank_1;
    unsigned int bank, col;
    int nc, nb, ncx, ncy, nbx, nby;
    int mop1, mop2, addop, rowout_temp, rowout, rowout_1;

    mop1  =selectbits(y,ry1,ry0);
    addop =selectbits(x,rx1,rx0);
    mop2  =selectbits(rowjump,jh,jl);
    rowout_temp = mop1*mop2;
    rowout = selectbits(rowout_temp,9,0) + addop;
    rowout_1 = selectbits(rowout,rh,rl);
    bank_y=selectbits(y,by1,by0);
    bank_x=selectbits(x,bx1,bx0);
    col_y=selectbits(y,cy1,cy0);
    col_x=selectbits(x,cx1,cx0);
    byte_y= selectbits(y,0,0);
    byte_x= selectbits(x,0,0);
    bank_y_0 = selectbits(y,by2,by2);

    bank_y = bank_y ^ byte_y;
    ncx = cx1-cx0+1; ncy = cy1 - cy0 + 1; nc = ncx+ncy;
    col=jointbits(col_x, col_y, ncx);
    nbx = bx1-bx0+1; nby = by1-by0+1; nb = nbx+nby;   //nbx = 1, nby=1,

    bank=jointbits(bank_y_0, bank_y, 1);
    bank_1= jointbits(bank, bank_x, nby+1);

//  bank=jointbits(bank_y, bank_x, nby);
//  bank_1= jointbits(bank, bank_y_0, 1);

    addr = jointbits(col, bank_1, nc);
    addr = jointbits(addr, rowout_1, nc+nb+1);

 	return addr;
}

//   tile_config: BF410084H, bit: 17-16  [0x1f410084]:0x101021 => 0
//   hd_map_mode: BF410084H, bit: 12                           => 1
//     page_size: BF410084H, bit: 5-4                          => 2
// field_picture: BF410020H, bit: 17  [0x1f410020]:0x161c242d  => 0
// col_size_mode:
//     rowjump00: BF4401C0H  [0x1f4401c0]:0x8040810
//     rowjump01: BF4401C4H  [0x1f4401c4]:0x600
//     rowjump10: BF4401C8H  [0x1f4401c8]:0x8040810
//     rowjump11: BF4401CCH  [0x1f4401cc]:0x10081020

unsigned int ddr_tile_map_trio(
							unsigned int x,
							unsigned int y,
							unsigned int tile_config,
							unsigned int hd_map_mode,
							unsigned int page_size,
							unsigned int field_picture,
							unsigned int col_size_mode,
							unsigned int rowjump00,
							unsigned int rowjump01,
							unsigned int rowjump10,
							unsigned int rowjump11)
{
	unsigned int mode;
	unsigned addr;

    mode = (tile_config<<4)+(field_picture<<3)+(hd_map_mode << 2)+page_size;

    switch (mode)
    {
        //************** 4 banks tile_config_8bank=0 *********************
        case 0:  addr = joint_trio(x,y,10,5,9,8,7,7,4,4,3,0,6,0,rowjump00,18,16,7,0,0);  break;  //frame sd 2048: 128x16
        case 1:  addr = joint_trio(x,y,10,6,9,6,5,5,5,5,4,0,4,0,rowjump00,4, 0, 8,0,0);  break;  //frame sd 1024: 32x32
        case 2:  addr = joint_trio(x,y,10,6,9,7,6,6,5,5,4,0,5,0,rowjump00,11,8, 7,0,0);  break;  //frame sd 2048: 64x32
        case 3:  addr = joint_trio(x,y,10,7,9,7,6,6,6,6,5,0,5,0,rowjump00,27,24,6,0,0);  break;  //frame sd 4096: 64x64

        case 4:  addr = joint_trio(x,y,10,5,10,8,7,7,4,4,3,0,6,0,rowjump01,19,16,8,0,0); break;  //frame hd 2048: 128x16
        case 5:  addr = joint_trio(x,y,10,6,10,6,5,5,5,5,4,0,4,0,rowjump01,5, 0, 9,0,0); break;  //frame hd 1024: 32x32
        case 6:  addr = joint_trio(x,y,10,6,10,7,6,6,5,5,4,0,5,0,rowjump01,12,8, 8,0,0); break;  //frame hd 2048: 64x32
        case 7:  addr = joint_trio(x,y,10,7,10,7,6,6,6,6,5,0,5,0,rowjump01,28,24,7,0,0); break;  //frame hd 4096: 64x64

        case 8:  addr = joint_trio(x,y,10,5,9,8,7,7,5,5,4,1,6,0,rowjump00,18,16,7,0,1);  break;  //frame sd 2048: 128x16
        case 9:  addr = joint_trio(x,y,10,6,9,6,5,5,6,6,5,1,4,0,rowjump00,4, 0, 8,0,1);  break;  //frame sd 1024: 32x32
        case 10: addr = joint_trio(x,y,10,6,9,7,6,6,6,6,5,1,5,0,rowjump00,11,8, 7,0,1);  break;  //frame sd 2048: 64x32
        case 11: addr = joint_trio(x,y,10,7,9,7,6,6,7,7,6,1,5,0,rowjump00,27,24,6,0,1);  break;  //frame sd 4096: 64x64

        case 12: addr = joint_trio(x,y,10,5,10,8,7,7,5,5,4,1,6,0,rowjump01,19,16,8,0,1); break;  //frame hd 2048: 128x16
        case 13: addr = joint_trio(x,y,10,6,10,6,5,5,6,6,5,1,4,0,rowjump01,5, 0, 9,0,1); break;  //frame hd 1024: 32x32
        case 14: addr = joint_trio(x,y,10,6,10,7,6,6,6,6,5,1,5,0,rowjump01,12,8, 8,0,1); break;  //frame hd 2048: 64x32
        case 15: addr = joint_trio(x,y,10,7,10,7,6,6,7,7,6,1,5,0,rowjump01,28,24,7,0,1); break;  //frame hd 4096: 64x64

        //************** 4 banks tile_config_8bank=1 *********************
        case 16: addr = joint_trio(x,y,10,5,9,8,7,7,4,4,3,0,6,0,rowjump00,18,16,7,0,0);  break;  //frame sd 2048: 128x16
        case 17: addr = joint_trio(x,y,10,6,9,6,5,5,5,5,4,0,4,0,rowjump00,4, 0, 8,0,0);  break;  //frame sd 1024: 32x32
        case 18: addr = joint_trio(x,y,10,6,9,7,6,6,5,5,4,0,5,0,rowjump00,11,8, 7,0,0);  break;  //frame sd 2048: 64x32
        case 19: addr = joint_trio(x,y,10,7,9,7,6,6,6,6,5,0,5,0,rowjump00,27,24,6,0,0);  break;  //frame sd 4096: 64x64

        case 20: addr = joint_trio(x,y,10,5,10,8,7,7,4,4,3,0,6,0,rowjump01,19,16,8,0,0); break;  //frame sd 2048: 128x16
        case 21: addr = joint_trio(x,y,10,6,10,6,5,5,5,5,4,0,4,0,rowjump01,5, 0, 9,0,0); break;  //frame sd 1024: 32x32
        case 22: addr = joint_trio(x,y,10,6,10,7,6,6,5,5,4,0,5,0,rowjump01,12,8, 8,0,0); break;  //frame sd 2048: 64x32
        case 23: addr = joint_trio(x,y,10,7,10,7,6,6,6,6,5,0,5,0,rowjump01,28,24,7,0,0); break;  //frame sd 4096: 64x64

        case 24: addr = joint_trio(x,y,10,5,9,8,7,7,5,5,4,1,6,0,rowjump00,18,16,7,0,1);  break;  //frame sd 2048: 128x16
        case 25: addr = joint_trio(x,y,10,6,9,6,5,5,6,6,5,1,4,0,rowjump00,4, 0, 8,0,1);  break;  //frame sd 1024: 32x32
        case 26: addr = joint_trio(x,y,10,6,9,7,6,6,6,6,5,1,5,0,rowjump00,11,8, 7,0,1);  break;  //frame sd 2048: 64x32
        case 27: addr = joint_trio(x,y,10,7,9,7,6,6,7,7,6,1,5,0,rowjump00,27,24,6,0,1);  break;  //frame sd 4096: 64x64

        case 28: addr = joint_trio(x,y,10,5,10,8,7,7,5,5,4,1,6,0,rowjump01,19,16,8,0,1); break;  //frame hd 2048: 128x16
        case 29: addr = joint_trio(x,y,10,6,10,6,5,5,6,6,5,1,4,0,rowjump01,5, 0, 9,0,1); break;  //frame hd 1024: 32x32
        case 30: addr = joint_trio(x,y,10,6,10,7,6,6,6,6,5,1,5,0,rowjump01,12,8, 8,0,1); break;  //frame hd 2048: 64x32
        case 31: addr = joint_trio(x,y,10,7,10,7,6,6,7,7,6,1,5,0,rowjump01,28,24,7,0,1); break;  //frame hd 4096: 64x64

        //************** 8 banks 4x2 tiles per row *********************
        case 32: addr = joint_trio(x,y,10,5,9,9,8,7,4,4,3,0,6,0,rowjump10,17,16,6,0,0);  break;  //frame sd 2048: 128x16
        case 33: addr = joint_trio(x,y,10,6,9,7,6,5,5,5,4,0,4,0,rowjump10,3, 0, 7,0,0);  break;  //frame sd 1024: 32x32
        case 34: addr = joint_trio(x,y,10,6,9,8,7,6,5,5,4,0,5,0,rowjump10,10,8, 6,0,0);  break;  //frame sd 2048: 64x32
        case 35: addr = joint_trio(x,y,10,7,9,8,7,6,6,6,5,0,5,0,rowjump10,26,24,5,0,0);  break;  //frame sd 4096: 64x64

        case 36: addr = joint_trio(x,y,10,5,10,9,8,7,4,4,3,0,6,0,rowjump11,18,16,7,0,0); break;  //frame hd 2048: 128x16
        case 37: addr = joint_trio(x,y,10,6,10,7,6,5,5,5,4,0,4,0,rowjump11,4, 0, 8,0,0); break;  //frame hd 1024: 32x32
        case 38: addr = joint_trio(x,y,10,6,10,8,7,6,5,5,4,0,5,0,rowjump11,11,8, 7,0,0); break;  //frame hd 2048: 64x32
        case 39: addr = joint_trio(x,y,10,7,10,8,7,6,6,6,5,0,5,0,rowjump11,27,24,6,0,0); break;  //frame hd 4096: 64x64

        case 40: addr = joint_trio(x,y,10,5,9,9,8,7,5,5,4,1,6,0,rowjump10,17,16,6,0,1);  break;  //frame sd 2048: 128x16
        case 41: addr = joint_trio(x,y,10,6,9,7,6,5,6,6,5,1,4,0,rowjump10,3, 0, 7,0,1);  break;  //frame sd 1024: 32x32
        case 42: addr = joint_trio(x,y,10,6,9,8,7,6,6,6,5,1,5,0,rowjump10,10,8, 6,0,1);  break;  //frame sd 2048: 64x32
        case 43: addr = joint_trio(x,y,10,7,9,8,7,6,7,7,6,1,5,0,rowjump10,26,24,5,0,1);  break;  //frame sd 4096: 64x64

        case 44: addr = joint_trio(x,y,10,5,10,9,8,7,5,5,4,1,6,0,rowjump11,18,16,7,0,1); break;  //frame hd 2048: 128x16
        case 45: addr = joint_trio(x,y,10,6,10,7,6,5,6,6,5,1,4,0,rowjump11,4, 0, 8,0,1); break;  //frame hd 1024: 32x32
        case 46: addr = joint_trio(x,y,10,6,10,8,7,6,6,6,5,1,5,0,rowjump11,11,8, 7,0,1); break;  //frame hd 2048: 64x32
        case 47: addr = joint_trio(x,y,10,7,10,8,7,6,7,7,6,1,5,0,rowjump11,27,24,6,0,1); break;  //frame hd 4096: 64x64


        //************** 8 banks 2x4 tiles per row *********************
        case 48: addr = joint_trio(x,y,10,6,9,8,7,7,5,4,3,0,6,0,rowjump00,18,16,6,0,0);    break;   //frame sd 2048: 128x16
        case 49: addr = joint_trio(x,y,10,7,9,6,5,5,6,5,4,0,4,0,rowjump00,4, 0, 7,0,0);    break;   //frame sd 1024: 32x32
        case 50: addr = joint_trio(x,y,10,7,9,7,6,6,6,5,4,0,5,0,rowjump00,11,8, 6,0,0);    break;   //frame sd 2048: 64x32
        case 51: addr = joint_trio(x,y,10,8,9,7,6,6,7,6,5,0,5,0,rowjump00,27,24,5,0,0);    break;   //frame sd 4096: 64x64

        case 52: addr = joint_trio(x,y,10,6,10,8,7,7,5,4,3,0,6,0,rowjump01,19,16,7,0,0);   break;  //frame hd 2048: 128x16
        case 53: addr = joint_trio(x,y,10,7,10,6,5,5,6,5,4,0,4,0,rowjump01,5, 0, 8,0,0);   break;  //frame hd 1024: 32x32
        case 54: addr = joint_trio(x,y,10,7,10,7,6,6,6,5,4,0,5,0,rowjump01,12,8, 7,0,0);   break;  //frame hd 2048: 64x32
        case 55: addr = joint_trio(x,y,10,8,10,7,6,6,7,6,5,0,5,0,rowjump01,28,24,6,0,0);   break;  //frame hd 4096: 64x64

        case 56: addr = joint_trio_1(x,y,10,6,9,8,7,7,6,6,5,4,1,6,0,rowjump00,18,16,6,0);  break;  //field sd 2048: 128x16
        case 57: addr = joint_trio_1(x,y,10,7,9,6,5,5,7,7,6,5,1,4,0,rowjump00,4,0,7,0);    break;  //field sd 1024: 32x32
        case 58: addr = joint_trio_1(x,y,10,7,9,7,6,6,7,7,6,5,1,5,0,rowjump00,11,8,6,0);   break;  //field sd 2048: 64x32
        case 59: addr = joint_trio_1(x,y,10,8,9,7,6,6,8,8,7,6,1,5,0,rowjump00,27,24,5,0);  break;  //field sd 4096: 64x64

        case 60: addr = joint_trio_1(x,y,10,6,10,8,7,7,6,6,5,4,1,6,0,rowjump01,19,16,7,0); break;  //field hd 2048: 128x16
        case 61: addr = joint_trio_1(x,y,10,7,10,6,5,5,7,7,6,5,1,4,0,rowjump01,5,0,8,0);   break;  //field hd 1024: 32x32
        case 62: addr = joint_trio_1(x,y,10,7,10,7,6,6,7,7,6,5,1,5,0,rowjump01,12,8,7,0);  break;  //field hd 2048: 64x32
        case 63: addr = joint_trio_1(x,y,10,8,10,7,6,6,8,8,7,6,1,5,0,rowjump01,28,24,6,0); break;  //field hd 4096: 64x64

        default: addr = 0; break;
    }

    return addr;
}
#endif

//---------------------------------------------------------------------------//

//   tile_config: BF410084H, bit: 17-16  [0x1f410084]:0x101021 => 0
//   hd_map_mode: BF410084H, bit: 12                           => 1
//     page_size: BF410084H, bit: 5-4                          => 2
// field_picture: BF410020H, bit: 17  [0x1f410020]:0x161c242d  => 0
// col_size_mode:
//     rowjump00: BF4401C0H  [0x1f4401c0]:0x8040810
//     rowjump01: BF4401C4H  [0x1f4401c4]:0x600
//     rowjump10: BF4401C8H  [0x1f4401c8]:0x8040810
//     rowjump11: BF4401CCH  [0x1f4401cc]:0x10081020

#define REG_VDEC_MPEG_OUT_CFG_1			0x84
#define REG_VDEC_COM_SLICE_CFG_0		0x20
#define REG_DISP_ROW_JUMP_00			0x1C0
#define REG_DISP_ROW_JUMP_01			0x1C4
#define REG_DISP_ROW_JUMP_10			0x1C8
#define REG_DISP_ROW_JUMP_11			0x1CC

#define ACCESS_LEN	32

// hd_map_mode: 0 -- 8 bits ddr, 1 -- 16 bits ddr
int hd_map_mode   = 1;
// col_size_mode: 0 -- 512; 1 -- 1024; 2 -- 2048; 3 -- 4096
int col_size_mode = 2;

unsigned int tile_config = 0;
unsigned int page_size   = 2;
unsigned int rowjump00   = 0x8040810;
unsigned int rowjump01   = 0x600;
unsigned int rowjump10   = 0x8040810;
unsigned int rowjump11   = 0x10081020;

static inline unsigned int get_tile_config(void)
{
	unsigned int val = (unsigned int)readl((volatile void*)(mt_get_vdec_base() + REG_VDEC_MPEG_OUT_CFG_1));

	return (val >> 16) & 0x03;
}

static inline unsigned int get_hd_map_mode(void)
{
	unsigned int val = (unsigned int)readl((volatile void*)(mt_get_vdec_base() + REG_VDEC_MPEG_OUT_CFG_1));

	return (val >> 12) & 0x01;
}

static inline unsigned int get_page_size(void)
{
	unsigned int val = (unsigned int)readl((volatile void*)(mt_get_vdec_base() + REG_VDEC_MPEG_OUT_CFG_1));

	return (val >> 4) & 0x03;
}

static inline unsigned int get_field_picture(void)
{
	unsigned int val = (unsigned int)readl((volatile void*)(mt_get_vdec_base() + REG_VDEC_COM_SLICE_CFG_0));

	return (val >> 17) & 0x01;
}

static inline unsigned int get_rowjump00(void)
{
	return (unsigned int)readl((volatile void*)(mt_get_display_base() + REG_DISP_ROW_JUMP_00));
}

static inline unsigned int get_rowjump01(void)
{
	return (unsigned int)readl((volatile void*)(mt_get_display_base() + REG_DISP_ROW_JUMP_01));
}

static inline unsigned int get_rowjump10(void)
{
	return (unsigned int)readl((volatile void*)(mt_get_display_base() + REG_DISP_ROW_JUMP_10));
}

static inline unsigned int get_rowjump11(void)
{
	return (unsigned int)readl((volatile void*)(mt_get_display_base() + REG_DISP_ROW_JUMP_11));
}

static inline unsigned int revert(unsigned int offset)
{
	// 0 -> 7
	// 1 -> 6
	// 2 -> 5
	// 3 -> 4
	// 4 -> 3
	// 5 -> 2
	// 6 -> 1
	// 7 -> 0
	return (((offset >> 3) << 3) | (7 - (offset & 0x07)));
}

static inline void revert_8(unsigned char *out, unsigned char *in)
{
	out[0] = in[7];
	out[1] = in[6];
	out[2] = in[5];
	out[3] = in[4];
	out[4] = in[3];
	out[5] = in[2];
	out[6] = in[1];
	out[7] = in[0];
}

static inline void revert_bytes(unsigned char *out, unsigned char *in, int len)
{
	int i;

	for (i=0; i<=len-8; i+=8)
	{
		revert_8(out+i, in+i);
	}
}

/* Y: tile 格式转为linear格式 */
int tile_convert(void *in_addr, int width, int height, void *out_addr)
{
	unsigned char *py;
	unsigned char *py_t;
	int i, j, offset;
	int field_flag = 0;	//0: progressive picture
						//1: interlaced picture

	tile_config = get_tile_config();
	hd_map_mode = get_hd_map_mode();
	page_size   = get_page_size();
	field_flag  = get_field_picture();
	rowjump00   = get_rowjump00();
	rowjump01   = get_rowjump01();
	rowjump10   = get_rowjump10();
	rowjump11   = get_rowjump11();

	printk("%s: \n",__FUNCTION__);
	printk("\t  tile_config: %u\n",tile_config);
	printk("\t  hd_map_mode: %u\n",hd_map_mode);
	printk("\t    page_size: %u\n",page_size);
	printk("\tfield_picture: %u\n",field_flag);
	printk("\t    rowjump00: 0x%X\n",rowjump00);
	printk("\t    rowjump01: 0x%X\n",rowjump01);
	printk("\t    rowjump10: 0x%X\n",rowjump10);
	printk("\t    rowjump11: 0x%X\n",rowjump11);

	for (j=0; j<height; j++)
	{
		py = out_addr + j*width;

		for (i=0; i<width; i+=ACCESS_LEN)
		{
			//offset = ddr_tile_map(i, j, hd_map_mode, col_size_mode, field_flag);
			offset = ddr_tile_map_trio(i, j,
										tile_config,
										hd_map_mode,
										page_size,
										field_flag,
										col_size_mode,
										rowjump00,
										rowjump01,
										rowjump10,
										rowjump11);
			py_t = in_addr + offset;
			//memcpy(py, py_t, ACCESS_LEN);
			revert_bytes(py, py_t, MIN(ACCESS_LEN, (width-i)));
			py += ACCESS_LEN;
		}
	}

	return 0;
}

/* UV: tile 格式转为linear格式 */
//U: uv = 0
//V: uv = 1
static int tile_convert_UV(void *in_addr, int width, int height, void *out_addr, int uv)
{
	unsigned char *po;
	unsigned char *po_t;
	int i, j, offset;
	int field_flag = 0;	//0: progressive picture
						//1: interlaced picture

	tile_config = get_tile_config();
	hd_map_mode = get_hd_map_mode();
	page_size   = get_page_size();
	field_flag  = get_field_picture();
	rowjump00   = get_rowjump00();
	rowjump01   = get_rowjump01();
	rowjump10   = get_rowjump10();
	rowjump11   = get_rowjump11();

	printk("%s: \n",__FUNCTION__);
	printk("\t  tile_config: %u\n",tile_config);
	printk("\t  hd_map_mode: %u\n",hd_map_mode);
	printk("\t    page_size: %u\n",page_size);
	printk("\tfield_picture: %u\n",field_flag);
	printk("\t    rowjump00: 0x%X\n",rowjump00);
	printk("\t    rowjump01: 0x%X\n",rowjump01);
	printk("\t    rowjump10: 0x%X\n",rowjump10);
	printk("\t    rowjump11: 0x%X\n",rowjump11);

	for (j=0; j<height; j++)
	{
		po = out_addr + j*width;

		for (i=0; i<width; i++)
		{
			offset = ddr_tile_map_trio(i*2 + uv, j,
										tile_config,
										hd_map_mode,
										page_size,
										field_flag,
										col_size_mode,
										rowjump00,
										rowjump01,
										rowjump10,
										rowjump11);
			offset = revert(offset);

			po_t = in_addr + offset;

			*(po + i) = *po_t;
		}
	}

	return 0;
}

int tile_convert_U(void *in_addr, int width, int height, void *out_addr)
{
	return tile_convert_UV(in_addr, width, width, out_addr, 0);
}

int tile_convert_V(void *in_addr, int width, int height, void *out_addr)
{
	return tile_convert_UV(in_addr, width, width, out_addr, 1);
}

