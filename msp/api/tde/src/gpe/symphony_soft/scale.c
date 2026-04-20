/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2014 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
#ifdef CONFIG_MT_FPGA_GPE
#include "sys_define.h"
//#include "sys_types.h"
//#include "common.h"
//#include "lib_rect.h"
//#include "gpe_vsb.h"
//#include "mtos_mem.h"
//#include "../symphony/gpe_symphony_vsb.h"
#include "mt_common.h"
#include "mpi_memdev.h"
#include "../driver/gpe.h"


#include "scale.h"

#pragma GCC diagnostic push 
#pragma GCC diagnostic ignored "-Wimplicit-function-declaration" 
#pragma GCC diagnostic ignored "-Wbuiltin-declaration-mismatch"
#pragma GCC diagnostic ignored "-Wpointer-sign" 
#pragma GCC diagnostic ignored "-Wunused-variable"
#pragma GCC diagnostic ignored "-Wenum-compare" 
#pragma GCC diagnostic ignored "-Wint-conversion"
#pragma GCC diagnostic ignored "-Wunused-but-set-variable" 
#pragma GCC diagnostic ignored "-Wmissing-prototypes"
#pragma GCC diagnostic ignored "-Wunused-function" 
#pragma GCC diagnostic ignored "-Wshadow"

//extern int s_cnt;
#define FAST_2D_SCALE

int coeff_0[32][7] = {
{0,0, 28, 192,   36,    0, 0},  
{0,0, 25, 190,   41,    0, 0},  
{0,0, 21, 190,   45,    0, 0},  
{0,0, 18, 188,   50,    0, 0},  
{0,0, 15, 186,   55,    0, 0},  
{0,0, 13, 182,   61,    0, 0},  
{0,0, 10, 180,   66,    0, 0},  
{0,0,   8, 176,  72,    0, 0},  
{0,0,   6, 172,  78,    0, 0},  
{0,0,   5, 166,  85,    0, 0},  
{0,0,   3, 162,  91,    0, 0},  
{0,0,   2, 156,  98,    0, 0},  
{0,0,   1, 150, 105,    0, 0},  
{0,0,   1, 142, 113,    0, 0},  
{0,0,   0, 136, 120,    0, 0},  
{0,0,   0, 128, 128,    0, 0},  
{0,0,   0, 120, 136,    0, 0},  
{0,0,   0, 112, 143,    1, 0},  
{0,0,   0, 105, 150,    1, 0},  
{0,0,   0,  98, 156,    2, 0},  
{0,0,   0,  91, 162,    3, 0},  
{0,0,   0,  84, 167,    5, 0},  
{0,0,   0,  78, 172,    6, 0},  
{0,0,   0,  72, 176,    8, 0},  
{0,0,   0,  66, 180, 10, 0},    
{0,0,   0,  60, 183, 13, 0},    
{0,0,   0,  55, 186, 15, 0},    
{0,0,   0,  50, 188, 18, 0},    
{0,0,   0,  45, 190, 21, 0},    
{0,0,   0,  40, 191, 25, 0},    
{0,0,   0,  36, 192, 28, 0},    
{0,0,   0,  32, 192, 32, 0},    
};

static int (*coeff)[7] =   COEF;
static int coeff_h[32][7] = {0};
static int coeff_v[32][7] = {0};

static u8 filter6(u8 line[7], int phase)
{

    int k, sum = 0, fdata;


    for(k=0; k<7;   k++)
    {
            sum+=   (int)line[k]    *   coeff[phase][k];
    }
    fdata = sum>>8;

    if(fdata    >   255) fdata  =   255;
    else if(fdata < 0)  fdata=  0;

    return (u8)fdata;
}

static u8 filter6_h(u8 line[7], int phase)
{

    int k, sum = 0, fdata;


    for(k=0; k<7;   k++)
    {
            sum+=   (int)line[k]    *   coeff_h[phase][k];
    }
    fdata = sum>>8;

    if(fdata    >   255) fdata  =   255;
    else if(fdata < 0)  fdata=  0;

    return (u8)fdata;
}

static u8 filter6_v(u8 line[7], int phase)
{

    int k, sum = 0, fdata;


    for(k=0; k<7;   k++)
    {
            sum+=   (int)line[k]    *   coeff_v[phase][k];
    }
    fdata = sum>>8;

    if(fdata    >   255) fdata  =   255;
    else if(fdata < 0)  fdata=  0;

    return (u8)fdata;
}
void    scale_vb(u8 *pRGB_src,int src_op_x,int src_op_y, int src_op_w,int src_op_h, int src_width, int src_height,
                 u8 *pRGB_dst, int dst_width,   int dst_height, scale_cfg_t *p_scale, 
                 u8 *key_src,  int key_stride, u8 *key_dst, u8 *msk_dst, int edge_option)
{
    long long sor,i_src,i_int,i_fra,j_int,p22_s,tmp,one;
    u8 line_a[7],line_r[7],line_g[7],line_b[7], key[7];
    u8 dat_a,dat_r,dat_g,dat_b;
    int i,j,k,blk_len,dat_out;
    int src_stride = src_width * 4;
    int dst_stride = dst_width * 4;
    int dst_key_stride = dst_width;
    int msk_stride = dst_width;
    long long p22 = p_scale->coef[3];
    long long p23 = p_scale->coef[4]; 
    int init_phase = p_scale->init_phase;
    MT_BOOL disable_alpha_filter = p_scale->disable_alpha_filter;
    MT_BOOL disable_color_filter = p_scale->disable_color_filter;

	int src_real_h,src_real_y;
	src_real_h = (edge_option == 2) ? (src_op_h+src_op_y) : src_height;
	src_real_y = (edge_option == 2) ? src_op_y : 0;

#if 0
printf("\r\n p22:0x%08x, p23:0x%08x", (s32)p22, (s32)p23);
printf("\r\n 0x%08x, %d,%d,%d,%d, %d,%d, 0x%08x, %d,%d, %d", pRGB_src,
    src_op_x, src_op_y, src_op_w, src_op_h, src_width, src_height, pRGB_dst, dst_width, dst_height, init_phase);
#endif
    blk_len = 8-(src_op_x&0x7);

    for (k=0;k<dst_width;k+=blk_len)
    {
        if (k==0)
        {
            if (dst_width<=blk_len)
                blk_len=dst_width;
        }
        else
        {
            if ((dst_width-k)>8)
                blk_len = 8;
            else
                blk_len = dst_width-k;
        }

        for(i=0; i<dst_height; i++)
        {
            if (i==(dst_height-20))
                one=1;
            one = 1;
            sor=((one<<(PARA_FRA_23_33+SOR_FRA))+(p23*i+(one<<PARA_FRA_23_33))/2)/(p23*i+(one<<PARA_FRA_23_33));

            p22_s=(((p22))*(sor>>0));
            tmp=(p22_s>>(SOR_FRA-0+PARA_FRA_22-FLI_FRA));
            if (p23 == 0)
                i_src=(p22*i)>>(SOR_FRA-FLI_FRA);
            else
                i_src=(p22>>(PARA_FRA_22-FLI_FRA))-(p22_s>>(SOR_FRA-0+PARA_FRA_22-FLI_FRA));

            i_src = i_src+(init_phase<<(FLI_FRA-5));
            i_int=(i_src>>FLI_FRA)+src_op_y-1;
            i_fra=(i_src>>(FLI_FRA-5))&0x1f;
            for(j=0; j<blk_len; j++)
            {
                j_int = j+k+src_op_x;
                if ((i_int-3)<src_real_y)
                {
                if (edge_option&0x1)
                {
                    line_a[0] = 0;
                    line_r[0] = 0;
                    line_g[0] = 0;
                    line_b[0] = 0;
                    key[0]=0;
                }
                else
                {
                    line_a[0] = *(pRGB_src+src_stride*src_real_y+j_int*4 +3  );
                    line_r[0] = *(pRGB_src+src_stride*src_real_y+j_int*4 +2 );
                    line_g[0] = *(pRGB_src+src_stride*src_real_y+j_int*4 +1 );
                    line_b[0] = *(pRGB_src+src_stride*src_real_y+j_int*4 +0 );
                    key[0]=*(key_src+key_stride*src_real_y+j_int);
                }
                }
                else
                {
                    line_a[0] = *(pRGB_src+src_stride*(i_int-3)+j_int*4 +3  );
                    line_r[0] = *(pRGB_src+src_stride*(i_int-3)+j_int*4 +2 );
                    line_g[0] = *(pRGB_src+src_stride*(i_int-3)+j_int*4 +1 );
                    line_b[0] = *(pRGB_src+src_stride*(i_int-3)+j_int*4 +0 );
                    key[0]=*(key_src+key_stride*(i_int-3)+j_int);
                }

                if ((i_int-2)<src_real_y)
                {
            if (edge_option&0x1)
            {
                line_a[1] = 0;
                line_r[1] = 0;
                line_g[1] = 0;
                line_b[1] = 0;
                key[1]=0;
            }
            else
            {
                line_a[1] = *(pRGB_src+src_stride*src_real_y+j_int*4 +3  );
                line_r[1] = *(pRGB_src+src_stride*src_real_y+j_int*4 +2 );
                line_g[1] = *(pRGB_src+src_stride*src_real_y+j_int*4 +1 );
                line_b[1] = *(pRGB_src+src_stride*src_real_y+j_int*4 +0 );
                key[1]=*(key_src+key_stride*src_real_y+j_int);
            }
                }
                else
                {
                    line_a[1] = *(pRGB_src+src_stride*(i_int-2)+j_int*4 +3  );
                    line_r[1] = *(pRGB_src+src_stride*(i_int-2)+j_int*4 +2 );
                    line_g[1] = *(pRGB_src+src_stride*(i_int-2)+j_int*4 +1 );
                    line_b[1] = *(pRGB_src+src_stride*(i_int-2)+j_int*4 +0 );
                    key[1]=*(key_src+key_stride*(i_int-2)+j_int);
                }

                if ((i_int-1)<src_real_y)
                {
            if (edge_option&0x1)
            {
                line_a[2] = 0;
                line_r[2] = 0;
                line_g[2] = 0;
                line_b[2] = 0;
                key[2]=0;
            }
            else
            {
                line_a[2] = *(pRGB_src+src_stride*src_real_y+j_int*4 +3  );
                line_r[2] = *(pRGB_src+src_stride*src_real_y+j_int*4 +2 );
                line_g[2] = *(pRGB_src+src_stride*src_real_y+j_int*4 +1 );
                line_b[2] = *(pRGB_src+src_stride*src_real_y+j_int*4 +0 );
                key[2]=*(key_src+key_stride*src_real_y+j_int);
            }
                }
                else
                {
                    line_a[2] = *(pRGB_src+src_stride*(i_int-1)+j_int*4 +3  );
                    line_r[2] = *(pRGB_src+src_stride*(i_int-1)+j_int*4 +2 );
                    line_g[2] = *(pRGB_src+src_stride*(i_int-1)+j_int*4 +1 );
                    line_b[2] = *(pRGB_src+src_stride*(i_int-1)+j_int*4 +0 );
                    key[2]=*(key_src+key_stride*(i_int-1)+j_int);
                }


                if ((i_int)>(src_real_h-1))
                {
            if (edge_option&0x1)
            {
                line_a[3] = 0;
                line_r[3] = 0;
                line_g[3] = 0;
                line_b[3] = 0;
                key[3]=0;
            }
            else
            {
                line_a[3] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +3   );
                line_r[3] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +2 );
                line_g[3] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +1 );
                line_b[3] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +0 );
                key[3]=*(key_src+key_stride*(src_real_h-1)+j_int);
            }
                }
                else if (i_int<src_real_y)
                {
            if (edge_option&0x1)
            {
                line_a[3] = 0;
                line_r[3] = 0;
                line_g[3] = 0;
                line_b[3] = 0;
                key[3]=0;
            }
            else
            {
                line_a[3] = *(pRGB_src+src_stride*src_real_y+j_int*4 +3  );
                line_r[3] = *(pRGB_src+src_stride*src_real_y+j_int*4 +2 );
                line_g[3] = *(pRGB_src+src_stride*src_real_y+j_int*4 +1 );
                line_b[3] = *(pRGB_src+src_stride*src_real_y+j_int*4 +0 );
                key[3]=*(key_src+key_stride*src_real_y+j_int);
            }
                }
                else
                {
                    line_a[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +3      );
                    line_r[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +2 );
                    line_g[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +1 );
                    line_b[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +0 );
            key[3]=*(key_src+key_stride*(i_int)+j_int);                
                }

                if ((i_int+1)>(src_real_h-1))
                {
            if (edge_option&0x1)
            {
                line_a[4] = 0;
                line_r[4] = 0;
                line_g[4] = 0;
                line_b[4] = 0;
                key[4]=0;
            }
            else
            {
                line_a[4] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +3   );
                line_r[4] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +2 );
                line_g[4] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +1 );
                line_b[4] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +0 );
                key[4]=*(key_src+key_stride*(src_real_h-1)+j_int);
            }
                }
                else
                {
                    line_a[4] = *(pRGB_src+src_stride*(i_int+1)+j_int*4 +3 );
                    line_r[4] = *(pRGB_src+src_stride*(i_int+1)+j_int*4 +2 );
                    line_g[4] = *(pRGB_src+src_stride*(i_int+1)+j_int*4 +1 );
                    line_b[4] = *(pRGB_src+src_stride*(i_int+1)+j_int*4 +0 );
                    key[4]=*(key_src+key_stride*(i_int+1)+j_int);
                }

                if ((i_int+2)>(src_real_h-1))
                {
            if (edge_option&0x1)
            {
                line_a[5] = 0;
                line_r[5] = 0;
                line_g[5] = 0;
                line_b[5] = 0;
                key[5]=0;
            }
            else
            {
                line_a[5] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +3 );
                line_r[5] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +2 );
                line_g[5] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +1 );
                line_b[5] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +0 );
                key[5]=*(key_src+key_stride*(src_real_h-1)+j_int);
            }
                }
                else
                {
                    line_a[5] = *(pRGB_src+src_stride*(i_int+2)+j_int*4 +3  );
                    line_r[5] = *(pRGB_src+src_stride*(i_int+2)+j_int*4 +2 );
                    line_g[5] = *(pRGB_src+src_stride*(i_int+2)+j_int*4 +1 );
                    line_b[5] = *(pRGB_src+src_stride*(i_int+2)+j_int*4 +0 );
                    key[5]=*(key_src+key_stride*(i_int+2)+j_int);
                }

                if ((i_int+3)>(src_real_h-1))
                {
            if (edge_option&0x1)
            {
                line_a[6] = 0;
                line_r[6] = 0;
                line_g[6] = 0;
                line_b[6] = 0;
                key[6]=0;
            }
            else
            {
                line_a[6] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +3   );
                line_r[6] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +2 );
                line_g[6] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +1 );
                line_b[6] = *(pRGB_src+src_stride*(src_real_h-1)+j_int*4 +0 );
                key[6]=*(key_src+key_stride*(src_real_h-1)+j_int);
            }
                }
                else
                {
                    line_a[6] = *(pRGB_src+src_stride*(i_int+3)+j_int*4 +3  );
                    line_r[6] = *(pRGB_src+src_stride*(i_int+3)+j_int*4 +2 );
                    line_g[6] = *(pRGB_src+src_stride*(i_int+3)+j_int*4 +1 );
                    line_b[6] = *(pRGB_src+src_stride*(i_int+3)+j_int*4 +0 );
                    key[6]=*(key_src+key_stride*(i_int+3)+j_int);
                }

        if (key[2]>0)
        {
            line_a[2]=line_a[3];
            line_r[2]=line_r[3];
            line_g[2]=line_g[3];
            line_b[2]=line_b[3];
            line_a[1]=line_a[3];
            line_r[1]=line_r[3];
            line_g[1]=line_g[3];
            line_b[1]=line_b[3];
            line_a[0]=line_a[3];
            line_r[0]=line_r[3];
            line_g[0]=line_g[3];
            line_b[0]=line_b[3];
        }
        else if (key[1]>0)
        {
            line_a[1]=line_a[2];
            line_r[1]=line_r[2];
            line_g[1]=line_g[2];
            line_b[1]=line_b[2];
            line_a[0]=line_a[2];
            line_r[0]=line_r[2];
            line_g[0]=line_g[2];
            line_b[0]=line_b[2];
        }
        else if (key[0]>0)
        {
            line_a[0]=line_a[1];
            line_r[0]=line_r[1];
            line_g[0]=line_g[1];
            line_b[0]=line_b[1];
        }

        if (key[4]>0)
        {
            line_a[4]=line_a[3];
            line_r[4]=line_r[3];
            line_g[4]=line_g[3];
            line_b[4]=line_b[3];
            line_a[5]=line_a[3];
            line_r[5]=line_r[3];
            line_g[5]=line_g[3];
            line_b[5]=line_b[3];
            line_a[6]=line_a[3];
            line_r[6]=line_r[3];
            line_g[6]=line_g[3];
            line_b[6]=line_b[3];
        }
        else if (key[5]>0)
        {
            line_a[5]=line_a[4];
            line_r[5]=line_r[4];
            line_g[5]=line_g[4];
            line_b[5]=line_b[4];
            line_a[6]=line_a[4];
            line_r[6]=line_r[4];
            line_g[6]=line_g[4];
            line_b[6]=line_b[4];
        }
        else if (key[6]>0)
        {
            line_a[6]=line_a[5];
            line_r[6]=line_r[5];
            line_g[6]=line_g[5];
            line_b[6]=line_b[5];
        }

                if(disable_alpha_filter)
                    dat_a = line_a[3];
                else
                    dat_a=filter6(line_a,i_fra);
                if(disable_color_filter)
                {
                    dat_r = line_r[3];
                    dat_g = line_g[3];
                    dat_b = line_b[3];
                }
                else
                {
                    dat_r=filter6(line_r,i_fra);
                    dat_g=filter6(line_g,i_fra);
                    dat_b=filter6(line_b,i_fra);
                }

        if (key[3]>0)
        {
            dat_a=line_a[3];
            dat_r=line_r[3];
            dat_g=line_g[3];
            dat_b=line_b[3];
        }
                if ((j_int-src_op_x)<src_op_w && (j_int-src_op_x)>=-1 && (i_int-src_op_y)<src_op_h && (i_int-src_op_y)>=-1)
                {
                     *(msk_dst+msk_stride*(i)+(j + k))=0;
                }
                else
                {
                    dat_a=0;
                    dat_r=0;
                    dat_g=0;
                    dat_b=0;
                     *(msk_dst+msk_stride*(i)+(j + k))=1;
                }

                *(pRGB_dst+dst_stride*(i)+(j+k)*4+3)=dat_a;
                *(pRGB_dst+dst_stride*(i)+(j+k)*4+2)=dat_r;
                *(pRGB_dst+dst_stride*(i)+(j+k)*4+1)=dat_g;
                *(pRGB_dst+dst_stride*(i)+(j+k)*4+0)=dat_b;
            *(key_dst+dst_key_stride*(i)+(j+k))=key[3];
            }
        }
    }
}


void    scale_hb(u8 *pRGB_src,int src_op_x,int src_op_y,int src_op_w,int src_op_h,int src_width, int src_height,
                 u8 *pRGB_dst, int dst_width, int dst_height, scale_cfg_t *p_scale,
                 u8 *key_src, int key_stride, u8 *key_dst, u8 *msk_dst, int edge_option)
{
    long long sor,i_int,j_src,j_int,j_fra,p22_s,tmp,one;
    u8 line_a[7],line_r[7],line_g[7],line_b[7], key[7];
    u8 dat_a,dat_r,dat_g,dat_b;
    int i,j,k,blk_len,dat_out;
    int src_stride = src_width * 4;
    int dst_stride = dst_width * 4;
    int dst_key_stride = dst_width;
    int msk_stride = dst_width;
    long long p22 = p_scale->coef[3];
    long long p23 = p_scale->coef[4]; 
    int init_phase = p_scale->init_phase;
    MT_BOOL disable_alpha_filter = p_scale->disable_alpha_filter;
    MT_BOOL disable_color_filter = p_scale->disable_color_filter;

	int src_real_w,src_real_x;
	src_real_w = (edge_option == 2) ? (src_op_w+src_op_x) : src_width;
	src_real_x = (edge_option == 2) ? src_op_x : 0;

    printf("\r\n p22:0x%08x, p23:0x%08x\n", (s32)p22, (s32)p23);
    blk_len = 8-(src_op_y&0x7);
    for (k=0;k<dst_height;k+=blk_len)
    {
        if (k==0)
        {
            if (dst_height<=blk_len)
            blk_len=dst_height;
        }
        else
        {
            if ((dst_height-k)>8)
                blk_len = 8;
            else
                blk_len = dst_height-k;
        }
        for(i=0; i<dst_width; i++)
        {
            if (i==(dst_width-3))
                one=1;
            one = 1;
            tmp=(p23*i+(one<<PARA_FRA_23_33));
            sor=((one<<(PARA_FRA_23_33+SOR_FRA))+(p23*i+(one<<PARA_FRA_23_33))/2)/(p23*i+(one<<PARA_FRA_23_33));

            p22_s=(((p22))*(sor>>0));
            tmp=(p22_s>>(SOR_FRA-0+PARA_FRA_22-FLI_FRA));

            if (p23 == 0)
                j_src=(p22*i)>>(SOR_FRA-FLI_FRA);
            else
                j_src=(p22>>(PARA_FRA_22-FLI_FRA))-(p22_s>>(SOR_FRA-0+PARA_FRA_22-FLI_FRA));

            j_src=j_src+(init_phase<<(FLI_FRA-5));
            j_int=(j_src>>FLI_FRA)+src_op_x-1;
            j_fra=(j_src>>(FLI_FRA-5))&0x1f;
            for(j=0; j<blk_len; j++)
            {
                i_int = j+k+src_op_y;

                if ((j_int-3)<src_real_x)
                {
            if (edge_option&0x1)
            {
                line_a[0] = 0;
                line_r[0] = 0;
                line_g[0] = 0;
                line_b[0] = 0;
                key[0]=0;
            }
            else
            {
                line_a[0] = *(pRGB_src+src_stride*i_int+src_real_x*4 +3 );
                line_r[0] = *(pRGB_src+src_stride*i_int+src_real_x*4 +2 );
                line_g[0] = *(pRGB_src+src_stride*i_int+src_real_x*4 +1 );
                line_b[0] = *(pRGB_src+src_stride*i_int+src_real_x*4 +0 );
                key[0]=*(key_src+key_stride*i_int+src_real_x);
            }
                }
                else
                {
                    line_a[0] = *(pRGB_src+src_stride*i_int+(j_int-3)*4 +3  );
                    line_r[0] = *(pRGB_src+src_stride*i_int+(j_int-3)*4 +2 );
                    line_g[0] = *(pRGB_src+src_stride*i_int+(j_int-3)*4 +1 );
                    line_b[0] = *(pRGB_src+src_stride*i_int+(j_int-3)*4 +0 );
            key[0]=*(key_src+key_stride*i_int+(j_int-3));                    
                }

                if ((j_int-2)<src_real_x)
                {
                if (edge_option&0x1)
                {
                    line_a[1] = 0;
                    line_r[1] = 0;
                    line_g[1] = 0;
                    line_b[1] = 0;
                    key[1]=0;
                }
                else
                {
                    line_a[1] = *(pRGB_src+src_stride*i_int+src_real_x*4 +3 );
                    line_r[1] = *(pRGB_src+src_stride*i_int+src_real_x*4 +2 );
                    line_g[1] = *(pRGB_src+src_stride*i_int+src_real_x*4 +1 );
                    line_b[1] = *(pRGB_src+src_stride*i_int+src_real_x*4 +0 );
                    key[1]=*(key_src+key_stride*i_int+src_real_x);
                }
                }
                else
                {
                    line_a[1] = *(pRGB_src+src_stride*i_int+(j_int-2)*4 +3  );
                    line_r[1] = *(pRGB_src+src_stride*i_int+(j_int-2)*4 +2 );
                    line_g[1] = *(pRGB_src+src_stride*i_int+(j_int-2)*4 +1 );
                    line_b[1] = *(pRGB_src+src_stride*i_int+(j_int-2)*4 +0 );
                    key[1]=*(key_src+key_stride*i_int+(j_int-2));
                }

                if ((j_int-1)<src_real_x)
                {
            if (edge_option&0x1)
            {
                line_a[2] = 0;
                line_r[2] = 0;
                line_g[2] = 0;
                line_b[2] = 0;
                key[2]=0;
            }
            else
            {
                line_a[2] = *(pRGB_src+src_stride*i_int+src_real_x*4 +3 );
                line_r[2] = *(pRGB_src+src_stride*i_int+src_real_x*4 +2 );
                line_g[2] = *(pRGB_src+src_stride*i_int+src_real_x*4 +1 );
                line_b[2] = *(pRGB_src+src_stride*i_int+src_real_x*4 +0 );
                key[2]=*(key_src+key_stride*i_int+src_real_x);
            }
                }
                else
                {
                    line_a[2] = *(pRGB_src+src_stride*(i_int)+(j_int-1)*4 +3 );
                    line_r[2] = *(pRGB_src+src_stride*(i_int)+(j_int-1)*4 +2 );
                    line_g[2] = *(pRGB_src+src_stride*(i_int)+(j_int-1)*4 +1 );
                    line_b[2] = *(pRGB_src+src_stride*(i_int)+(j_int-1)*4 +0 );
                    key[2]=*(key_src+key_stride*i_int+(j_int-1));
                }

                if ((j_int)>(src_real_w-1))
                {
            if (edge_option&0x1)
            {
                line_a[3] = 0;
                line_r[3] = 0;
                line_g[3] = 0;
                line_b[3] = 0;
                key[3]=0;
            }
            else
            {
                line_a[3] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                line_r[3] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                line_g[3] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                line_b[3] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                key[3]=*(key_src+key_stride*(i_int)+(src_real_w-1));
            }
                }
                else if (i_int<src_real_x)
                {
            if (edge_option&0x1)
            {
                line_a[3] = 0;
                line_r[3] = 0;
                line_g[3] = 0;
                line_b[3] = 0;
                key[3]=0;
            }
            else
            {
                line_a[3] = *(pRGB_src+j_int*4 +src_real_x*4+3  );
                line_r[3] = *(pRGB_src+j_int*4 +src_real_x*4+2 );
                line_g[3] = *(pRGB_src+j_int*4 +src_real_x*4+1 );
                line_b[3] = *(pRGB_src+j_int*4 +src_real_x*4+0 );
                key[3]=*(key_src+key_stride*(i_int)+src_real_x);
            }
                }
                else
                {
                    line_a[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +3 );
                    line_r[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +2 );
                    line_g[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +1 );
                    line_b[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +0 );
                    key[3]=*(key_src+key_stride*i_int+j_int);
                }


                if ((j_int+1)>(src_real_w-1))
                {
            if (edge_option&0x1)
            {
                line_a[4] = 0;
                line_r[4] = 0;
                line_g[4] = 0;
                line_b[4] = 0;
                key[4]=0;
            }
            else
            {
                line_a[4] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                line_r[4] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                line_g[4] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                line_b[4] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                key[4]=*(key_src+key_stride*(i_int)+(src_real_w-1));
            }
                }
                else
                {
                    line_a[4] = *(pRGB_src+src_stride*(i_int)+(j_int+1)*4   +3 );
                    line_r[4] = *(pRGB_src+src_stride*(i_int)+(j_int+1)*4 +2 );
                    line_g[4] = *(pRGB_src+src_stride*(i_int)+(j_int+1)*4 +1 );
                    line_b[4] = *(pRGB_src+src_stride*(i_int)+(j_int+1)*4 +0 );
                    key[4]=*(key_src+key_stride*i_int+(j_int+1));
                }

                if ((j_int+2)>(src_real_w-1))
                {
            if (edge_option&0x1)
            {
                line_a[5] = 0;
                line_r[5] = 0;
                line_g[5] = 0;
                line_b[5] = 0;
                key[5]=0;
            }
            else
            {
                line_a[5] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                line_r[5] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                line_g[5] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                line_b[5] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                key[5]=*(key_src+key_stride*(i_int)+(src_real_w-1));
            }
                }
                else
                {
                    line_a[5] = *(pRGB_src+src_stride*(i_int)+(j_int+2)*4   +3  );
                    line_r[5] = *(pRGB_src+src_stride*(i_int)+(j_int+2)*4 +2 );
                    line_g[5] = *(pRGB_src+src_stride*(i_int)+(j_int+2)*4 +1 );
                    line_b[5] = *(pRGB_src+src_stride*(i_int)+(j_int+2)*4 +0 );
                    key[5]=*(key_src+key_stride*i_int+(j_int+2));
                }

                if ((j_int+3)>(src_real_w-1))
                {
            if (edge_option&0x01)
            {
                line_a[6] = 0;
                line_r[6] = 0;
                line_g[6] = 0;
                line_b[6] = 0;
                key[6]=0;
            }
            else
            {
                line_a[6] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                line_r[6] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                line_g[6] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                line_b[6] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                key[6]=*(key_src+key_stride*(i_int)+(src_real_w-1));
            }
                }
                else
                {
                    line_a[6] = *(pRGB_src+src_stride*(i_int)+(j_int+3)*4   +3  );
                    line_r[6] = *(pRGB_src+src_stride*(i_int)+(j_int+3)*4 +2 );
                    line_g[6] = *(pRGB_src+src_stride*(i_int)+(j_int+3)*4 +1 );
                    line_b[6] = *(pRGB_src+src_stride*(i_int)+(j_int+3)*4 +0 );
                    key[6]=*(key_src+key_stride*i_int+(j_int+3));
                }


        if (key[2]>0)
        {
            line_a[2]=line_a[3];
            line_r[2]=line_r[3];
            line_g[2]=line_g[3];
            line_b[2]=line_b[3];
            line_a[1]=line_a[3];
            line_r[1]=line_r[3];
            line_g[1]=line_g[3];
            line_b[1]=line_b[3];
            line_a[0]=line_a[3];
            line_r[0]=line_r[3];
            line_g[0]=line_g[3];
            line_b[0]=line_b[3];
        }
        else if (key[1]>0)
        {
            line_a[1]=line_a[2];
            line_r[1]=line_r[2];
            line_g[1]=line_g[2];
            line_b[1]=line_b[2];
            line_a[0]=line_a[2];
            line_r[0]=line_r[2];
            line_g[0]=line_g[2];
            line_b[0]=line_b[2];
        }
        else if (key[0]>0)
        {
            line_a[0]=line_a[1];
            line_r[0]=line_r[1];
            line_g[0]=line_g[1];
            line_b[0]=line_b[1];
        }

        if (key[4]>0)
        {
            line_a[4]=line_a[3];
            line_r[4]=line_r[3];
            line_g[4]=line_g[3];
            line_b[4]=line_b[3];
            line_a[5]=line_a[3];
            line_r[5]=line_r[3];
            line_g[5]=line_g[3];
            line_b[5]=line_b[3];
            line_a[6]=line_a[3];
            line_r[6]=line_r[3];
            line_g[6]=line_g[3];
            line_b[6]=line_b[3];
        }
        else if (key[5]>0)
        {
            line_a[5]=line_a[4];
            line_r[5]=line_r[4];
            line_g[5]=line_g[4];
            line_b[5]=line_b[4];
            line_a[6]=line_a[4];
            line_r[6]=line_r[4];
            line_g[6]=line_g[4];
            line_b[6]=line_b[4];
        }
        else if (key[6]>0)
        {
            line_a[6]=line_a[5];
            line_r[6]=line_r[5];
            line_g[6]=line_g[5];
            line_b[6]=line_b[5];
        }
        
                if(disable_alpha_filter)
                    dat_a = line_a[3];
                else
                    dat_a=filter6(line_a,j_fra);
                if(disable_color_filter)
                {
                    dat_r = line_r[3];
                    dat_g = line_g[3];
                    dat_b = line_b[3];
                }
                else
                {
                    dat_r=filter6(line_r,j_fra);
                    dat_g=filter6(line_g,j_fra);
                    dat_b=filter6(line_b,j_fra);
                }

        if (key[3]>0)
        {
            dat_a=line_a[3];
            dat_r=line_r[3];
            dat_g=line_g[3];
            dat_b=line_b[3];
        }
            
                if ((j_int-src_op_x)<src_op_w && (j_int-src_op_x)>=-1 && (i_int-src_op_y)<src_op_h && (i_int-src_op_y)>=-1)
                {
                 *(msk_dst+msk_stride*(j + k)+(i))=0;
                }
                else
                {
                    dat_a=0;
                    dat_r=0;
                    dat_g=0;
                    dat_b=0;
                     *(msk_dst+msk_stride*(j+k)+(i))=1;
                }

                *(pRGB_dst+dst_stride*(j+k)+(i)*4+3)=dat_a;
                *(pRGB_dst+dst_stride*(j+k)+(i)*4+2)=dat_r;
                *(pRGB_dst+dst_stride*(j+k)+(i)*4+1)=dat_g;
                *(pRGB_dst+dst_stride*(j+k)+(i)*4+0)=dat_b;
            *(key_dst+dst_key_stride *(j+k)+(i))=key[3];
            }
        }
    }
}

void    scale_hl(u8 *pRGB_src,int src_op_x,int src_op_y,int src_op_w,int src_op_h,int src_width, int src_height,    
                 u8 *pRGB_dst, int dst_width,   int dst_height, scale_cfg_t *p_scale, 
                 u8 *key_src, int key_stride, u8 *key_dst, u8 *msk_dst, int edge_option)
{
    long long sor,i_int,j_src,j_int,j_fra,temp,p22_s,p11_s,p21_s,p31_s,tmp,one,tmp1,tmp2;
    u8 line_a[7],line_r[7],line_g[7],line_b[7], key[7];
    u8 dat_a,dat_r,dat_g,dat_b;
    int i,j,k,blk_len,dat_out,j_src_tmp,k_sum;
    int src_stride = src_width * 4;
    int dst_stride = dst_width * 4;
    int dst_key_stride = dst_width;
    int msk_stride = dst_width;
    long long p11 = p_scale->coef[0];
    long long p21 = p_scale->coef[1];
    long long p31 = p_scale->coef[2];
    long long p22 = p_scale->coef[3];
    long long p23 = p_scale->coef[4]; 
    int init_phase = p_scale->init_phase;
    MT_BOOL alpha_anti_flicker = !p_scale->disable_alpha_anti_flicker;
    MT_BOOL color_anti_flicker = !p_scale->disable_color_anti_flicker;
    MT_BOOL disable_alpha_filter = p_scale->disable_alpha_filter;
    MT_BOOL disable_color_filter = p_scale->disable_color_filter;

	int src_real_w,src_real_x;
	src_real_w = (edge_option == 2) ? (src_op_w+src_op_x) : src_width;
	src_real_x = (edge_option == 2) ? src_op_x : 0;

  // printf("\r\n p11:0x%08x, p21:0x%08x, p31:0x%08x, p22:0x%08x, p23:0x%08x", (s32)p11, (s32)p21, (s32)p31, (s32)p22, (s32)p23);
    blk_len = 8-(src_op_y&0x7);
    j_src_tmp=0;
    for(i=0; i<dst_height; i++)
    {
        if (i==520)
            i=i;
        one = 1;
        sor=((one<<(PARA_FRA_23_33+SOR_FRA))+(p23*i+(one<<PARA_FRA_23_33))/2)/(p23*i+(one<<PARA_FRA_23_33));
        p11_s=(p11*(sor>>0));
        if (p23==0)
            p21_s=((p21*sor)>>0);
        else
            p21_s=((p21*sor)>>(SOR_FRA+PARA_FRA-FLI_FRA));
        p31_s=((p31*sor)>>(SOR_FRA+PARA_FRA-FLI_FRA));

        p22_s=(((p22))*(sor>>0));
        tmp=(p22_s>>(SOR_FRA-0+PARA_FRA_22-FLI_FRA));
        i_int=i+src_op_y;
        //i_fra=i_src&0x1f;
        j_src_tmp=(init_phase<<(FLI_FRA-5));
        for(j=0; j<dst_width; j++)
        {
            if (j==(520))
                one=1;
            tmp=(p11_s>>0)*j;
            tmp1=p21_s*i;
            tmp2=((p21_s*i)>>(SOR_FRA+SOR_FRA-FLI_FRA));
            if (p23==0)
                j_src=(tmp>>(SOR_FRA+PARA_FRA-FLI_FRA))+((p21_s*i)>>(SOR_FRA+SOR_FRA-FLI_FRA))+p31_s;
            else
                j_src=(tmp>>(SOR_FRA+PARA_FRA-FLI_FRA))-p21_s+p31_s+(p21>>(PARA_FRA_22-FLI_FRA));
            j_src=j_src+(init_phase<<(FLI_FRA-5));
            j_int=(j_src>>(FLI_FRA))+src_op_x-1;          
            j_fra=(j_src>>(FLI_FRA-5))&0x1f;


            if (((j_int-3)>(src_real_w-1)) || ((j_int-3)<src_real_x))
            {
            if (edge_option&0x1)
            {
                line_a[0] = 0;//*(pRGB_src+src_stride*i_int+0*4 +3 );
                line_r[0] = 0;//*(pRGB_src+src_stride*i_int+0*4 +2 );
                line_g[0] = 0;//*(pRGB_src+src_stride*i_int+0*4 +1 );
                line_b[0] = 0;//*(pRGB_src+src_stride*i_int+0*4 +0 );
                key[0]=0;
            }
            else if ((j_int-3)>(src_real_w-1))
            {
				line_a[0] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                line_r[0] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                line_g[0] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                line_b[0] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                key[0]=*(key_src+key_stride*i_int+(src_real_w-1));
            }
            else
            {
                line_a[0] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+src_real_x*4 +3 );
                line_r[0] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +2 );
                line_g[0] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +1 );
                line_b[0] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +0 );
                key[0]=*(key_src+key_stride*i_int+src_real_x);
            }

            }
            else
            {
                line_a[0] = *(pRGB_src+src_stride*i_int+(j_int-3)*4 +3  );
                line_r[0] = *(pRGB_src+src_stride*i_int+(j_int-3)*4 +2 );
                line_g[0] = *(pRGB_src+src_stride*i_int+(j_int-3)*4 +1 );
                line_b[0] = *(pRGB_src+src_stride*i_int+(j_int-3)*4 +0 );
                key[0]=*(key_src+key_stride*i_int+(j_int-3));
            }


            if (((j_int-2)>(src_real_w-1)) || ((j_int-2)<src_real_x))
            {
                if (edge_option&0x1)
                {
                    line_a[1] = 0;//*(pRGB_src+src_stride*i_int+0*4 +3 );
                    line_r[1] = 0;//*(pRGB_src+src_stride*i_int+0*4 +2 );
                    line_g[1] = 0;//*(pRGB_src+src_stride*i_int+0*4 +1 );
                    line_b[1] = 0;//*(pRGB_src+src_stride*i_int+0*4 +0 );
                    key[1]=0;
                }
                else if ((j_int-2)>(src_real_w-1))
                {
                    line_a[1] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                    line_r[1] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                    line_g[1] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                    line_b[1] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                    key[1]=*(key_src+key_stride*i_int+(src_real_w-1));
                }
                else
                {
                    line_a[1] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+src_real_x*4 +3 );
                    line_r[1] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +2 );
                    line_g[1] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +1 );
                    line_b[1] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +0 );
                    key[1]=*(key_src+key_stride*i_int+src_real_x);
                }

            }
            else
            {
                line_a[1] = *(pRGB_src+src_stride*i_int+(j_int-2)*4 +3  );
                line_r[1] = *(pRGB_src+src_stride*i_int+(j_int-2)*4 +2 );
                line_g[1] = *(pRGB_src+src_stride*i_int+(j_int-2)*4 +1 );
                line_b[1] = *(pRGB_src+src_stride*i_int+(j_int-2)*4 +0 );
                key[1]=*(key_src+key_stride*i_int+(j_int-2));
            }

            if (((j_int-1)>(src_real_w-1)) || ((j_int-1)<src_real_x))
            {
                if (edge_option&0x1)
                {
                    line_a[2] = 0;//*(pRGB_src+src_stride*i_int+0*4 +3 );
                    line_r[2] = 0;//*(pRGB_src+src_stride*i_int+0*4 +2 );
                    line_g[2] = 0;//*(pRGB_src+src_stride*i_int+0*4 +1 );
                    line_b[2] = 0;//*(pRGB_src+src_stride*i_int+0*4 +0 );
                    key[2]=0;
                }
                else if ((j_int-1)>(src_width-1))
                {
                    line_a[2] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                    line_r[2] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                    line_g[2] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                    line_b[2] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                    key[2]=*(key_src+key_stride*i_int+(src_real_w-1));
                }
                else
                {
                    line_a[2] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+src_real_x*4 +3 );
                    line_r[2] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +2 );
                    line_g[2] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +1 );
                    line_b[2] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +0 );
                    key[2]=*(key_src+key_stride*i_int+src_real_x);
                }
            }
            else
            {
                line_a[2] = *(pRGB_src+src_stride*(i_int)+(j_int-1)*4 +3 );
                line_r[2] = *(pRGB_src+src_stride*(i_int)+(j_int-1)*4 +2 );
                line_g[2] = *(pRGB_src+src_stride*(i_int)+(j_int-1)*4 +1 );
                line_b[2] = *(pRGB_src+src_stride*(i_int)+(j_int-1)*4 +0 );
                key[2]=*(key_src+key_stride*i_int+(j_int-1));
            }

            if (((j_int)>(src_real_w-1)) || ((j_int)<src_real_x))
            {
                if (edge_option&0x1)
                {
                    line_a[3] = 0;//*(pRGB_src+src_stride*i_int+0*4 +3 );
                    line_r[3] = 0;//*(pRGB_src+src_stride*i_int+0*4 +2 );
                    line_g[3] = 0;//*(pRGB_src+src_stride*i_int+0*4 +1 );
                    line_b[3] = 0;//*(pRGB_src+src_stride*i_int+0*4 +0 );
                    key[3]=0;
                }
                else if ((j_int)>(src_real_w-1))
                {
                    line_a[3] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                    line_r[3] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                    line_g[3] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                    line_b[3] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                    key[3]=*(key_src+key_stride*i_int+(src_real_w-1));
                }
                else
                {
                    line_a[3] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+src_real_x*4 +3 );
                    line_r[3] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +2 );
                    line_g[3] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +1 );
                    line_b[3] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +0 );
                    key[3]=*(key_src+key_stride*i_int+src_real_x);
                }
            }
            else
            {
                line_a[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +3 );
                line_r[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +2 );
                line_g[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +1 );
                line_b[3] = *(pRGB_src+src_stride*(i_int)+j_int*4 +0 );
                key[3]=*(key_src+key_stride*i_int+(j_int));
            }

            if (((j_int+1)>(src_real_w-1)) || ((j_int+1)<src_real_x))
            {
                if (edge_option&0x1)
                {
                    line_a[4] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +3 );
                    line_r[4] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +2 );
                    line_g[4] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +1 );
                    line_b[4] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +0 );
                    key[4]=0;
                }
                else if ((j_int+1)>(src_real_w-1))
                {
                    line_a[4] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                    line_r[4] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                    line_g[4] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                    line_b[4] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                    key[4]=*(key_src+key_stride*i_int+(src_real_w-1));
                }
                else
                {
                    line_a[4] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+src_real_x*4 +3 );
                    line_r[4] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +2 );
                    line_g[4] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +1 );
                    line_b[4] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +0 );
                    key[4]=*(key_src+key_stride*i_int+src_real_x);
                }
            }
            else
            {
                line_a[4] = *(pRGB_src+src_stride*(i_int)+(j_int+1)*4   +3 );
                line_r[4] = *(pRGB_src+src_stride*(i_int)+(j_int+1)*4 +2 );
                line_g[4] = *(pRGB_src+src_stride*(i_int)+(j_int+1)*4 +1 );
                line_b[4] = *(pRGB_src+src_stride*(i_int)+(j_int+1)*4 +0 );
                key[4]=*(key_src+key_stride*i_int+(j_int+1));
            }

            if (((j_int+2)>(src_real_w-1)) || ((j_int+2)<src_real_x))
            {
                if (edge_option&0x1)
                {
                    line_a[5] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +3 );
                    line_r[5] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +2 );
                    line_g[5] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +1 );
                    line_b[5] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +0 );
                    key[5]=0;
                }
                else if ((j_int+2)>(src_real_w-1))
                {
                    line_a[5] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                    line_r[5] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                    line_g[5] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                    line_b[5] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                    key[5]=*(key_src+key_stride*i_int+(src_real_w-1));
                }
                else
                {
                    line_a[5] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+src_real_x*4 +3 );
                    line_r[5] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +2 );
                    line_g[5] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +1 );
                    line_b[5] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +0 );
                    key[5]=*(key_src+key_stride*i_int+src_real_x);
                }
            }
            else
            {
                line_a[5] = *(pRGB_src+src_stride*(i_int)+(j_int+2)*4   +3  );
                line_r[5] = *(pRGB_src+src_stride*(i_int)+(j_int+2)*4 +2 );
                line_g[5] = *(pRGB_src+src_stride*(i_int)+(j_int+2)*4 +1 );
                line_b[5] = *(pRGB_src+src_stride*(i_int)+(j_int+2)*4 +0 );
                key[5]=*(key_src+key_stride*i_int+(j_int+2));
            }

            if (((j_int+3)>(src_real_w-1)) || ((j_int+3)<src_real_x))
            {
                if (edge_option&0x1)
                {
                    line_a[6] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +3 );
                    line_r[6] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +2 );
                    line_g[6] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +1 );
                    line_b[6] = 0;//*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +0 );
                    key[6]=0;
                }
                else if ((j_int+3)>(src_real_w-1))
                {
                    line_a[6] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +3 );
                    line_r[6] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +2 );
                    line_g[6] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +1 );
                    line_b[6] = *(pRGB_src+src_stride*(i_int)+(src_real_w-1)*4 +0 );
                    key[6]=*(key_src+key_stride*i_int+(src_real_w-1));
                }
                else
                {
                    line_a[6] = alpha_anti_flicker ? 0 : *(pRGB_src+src_stride*(i_int)+src_real_x*4 +3 );
                    line_r[6] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +2 );
                    line_g[6] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +1 );
                    line_b[6] = *(pRGB_src+src_stride*(i_int)+src_real_x*4 +0 );
                    key[6]=*(key_src+key_stride*i_int+src_real_x);
                }
            }
            else
            {
                line_a[6] = *(pRGB_src+src_stride*(i_int)+(j_int+3)*4   +3  );
                line_r[6] = *(pRGB_src+src_stride*(i_int)+(j_int+3)*4 +2 );
                line_g[6] = *(pRGB_src+src_stride*(i_int)+(j_int+3)*4 +1 );
                line_b[6] = *(pRGB_src+src_stride*(i_int)+(j_int+3)*4 +0 );
                key[6]=*(key_src+key_stride*i_int+(j_int+3));
            }


            if (key[2]>0)
            {
                line_a[2]=alpha_anti_flicker ? 0 : line_a[3]; 
                line_r[2]=line_r[3];
                line_g[2]=line_g[3];
                line_b[2]=line_b[3];
                line_a[1]=alpha_anti_flicker ? 0 : line_a[3]; 
                line_r[1]=line_r[3];
                line_g[1]=line_g[3];
                line_b[1]=line_b[3];
                line_a[0]=alpha_anti_flicker ? 0 : line_a[3]; 
                line_r[0]=line_r[3];
                line_g[0]=line_g[3];
                line_b[0]=line_b[3];
            }
            else if (key[1]>0)
            {
                line_a[1]=alpha_anti_flicker ? 0 : line_a[2]; 
                line_r[1]=line_r[2];
                line_g[1]=line_g[2];
                line_b[1]=line_b[2];
                line_a[0]=alpha_anti_flicker ? 0 : line_a[2]; 
                line_r[0]=line_r[2];
                line_g[0]=line_g[2];
                line_b[0]=line_b[2];
            }
            else if (key[0]>0)
            {
                line_a[0]=alpha_anti_flicker ? 0 : line_a[1]; 
                line_r[0]=line_r[1];
                line_g[0]=line_g[1];
                line_b[0]=line_b[1];
            }

            if (key[4]>0)
            {
                line_a[4]=alpha_anti_flicker ? 0 : line_a[3]; 
                line_r[4]=line_r[3];
                line_g[4]=line_g[3];
                line_b[4]=line_b[3];
                line_a[5]=alpha_anti_flicker ? 0 : line_a[3]; 
                line_r[5]=line_r[3];
                line_g[5]=line_g[3];
                line_b[5]=line_b[3];
                line_a[6]=alpha_anti_flicker ? 0 : line_a[3]; 
                line_r[6]=line_r[3];
                line_g[6]=line_g[3];
                line_b[6]=line_b[3];
            }
            else if (key[5]>0)
            {
                line_a[5]=alpha_anti_flicker ? 0 : line_a[4]; 
                line_r[5]=line_r[4];
                line_g[5]=line_g[4];
                line_b[5]=line_b[4];
                line_a[6]=alpha_anti_flicker ? 0 : line_a[4]; 
                line_r[6]=line_r[4];
                line_g[6]=line_g[4];
                line_b[6]=line_b[4];
            }
            else if (key[6]>0)
            {
                line_a[6]=alpha_anti_flicker ? 0 : line_a[5]; 
                line_r[6]=line_r[5];
                line_g[6]=line_g[5];
                line_b[6]=line_b[5];
            }
            

            if(disable_alpha_filter)
                dat_a = line_a[3];
            else
                dat_a=filter6(line_a,j_fra);
            if(disable_color_filter)
            {
                dat_r = line_r[3];
                dat_g = line_g[3];
                dat_b = line_b[3];
            }
            else
            {
                dat_r=filter6(line_r,j_fra);
                dat_g=filter6(line_g,j_fra);
                dat_b=filter6(line_b,j_fra);
            }   

            if (key[3]>0)
            {
                dat_a=line_a[3];
                dat_r=line_r[3];
                dat_g=line_g[3];
                dat_b=line_b[3];
            }
            
            /*k_sum=8;
            if (j_src_tmp<32 && j_src>=32)
            {
                for(k=1;k<=k_sum;k++)
                {
                    if ((j_src-32)<=(j_src-j_src_tmp)*k/k_sum)
                    {
                        //k=k_sum/4+k*3/4;
                        break;
                    }
                }
                if (color_anti_flicker)
                {                
                    dat_r=((k)*dat_r)/k_sum;
                    dat_g=((k)*dat_g)/k_sum;
                    dat_b=((k)*dat_b)/k_sum;
                }
                if (alpha_anti_flicker)
                {
                    dat_a=((k)*dat_a)/k_sum;
                }                
            }

            temp=(p11_s>>(SOR_FRA+PARA_FRA-5));
            if ((j_src>>5)<(src_op_w+1) && ((j_src)+j_src-j_src_tmp)>=((src_op_w+1)<<5))
            {
                for(k=1;k<=k_sum;k++)
                {
                    if ((((src_op_w+1)<<5)-j_src)<=(j_src-j_src_tmp)*k/k_sum)
                    {
                       // k=k_sum/4+k*3/4;
                        break;
                    }
                }    
                if(alpha_anti_flicker)
                    dat_a=((k)*dat_a)/k_sum;
                if(color_anti_flicker)
                {
                    dat_r=((k)*dat_r)/k_sum;
                    dat_g=((k)*dat_g)/k_sum;
                    dat_b=((k)*dat_b)/k_sum;
                }
            }*/
            if ((j_int-src_op_x)<(src_op_w+1) && (j_int-src_op_x)>=(-2) && (i_int-src_op_y)<(src_op_h) && (i_int-src_op_y)>=(0))
            {
                *(msk_dst+msk_stride*(i)+(j))=0;
            }
            else
            {
                dat_a=0;
                dat_r=0;
                dat_g=0;
                dat_b=0;
                *(msk_dst+msk_stride*(i)+(j))=1;
            }

            *(pRGB_dst+dst_stride*(i)+(j)*4+3)=dat_a;
            *(pRGB_dst+dst_stride*(i)+(j)*4+2)=dat_r;
            *(pRGB_dst+dst_stride*(i)+(j)*4+1)=dat_g;
            *(pRGB_dst+dst_stride*(i)+(j)*4+0)=dat_b;
            *(key_dst+dst_key_stride*(i)+(j))=key[3];
            j_src_tmp = j_src;
            }
        }
    }

void	scale_blur(u8	*pRGB_src,int src_op_x,int src_op_y,int src_op_w,int src_op_h, int src_width, int src_height,	
				 u8	*pRGB_dst, int dst_width,	int	dst_height,int blur_num)
{
	long long i_src,i_int,i_fra,j_src,j_int,j_fra;
	u8 dat_a,dat_r,dat_g,dat_b;
	int i,j,k,sum_a,sum_r,sum_g,sum_b,dat_out,temp,temp1;
       int src_stride = src_width * 4;
       int dst_stride = dst_width * 4;
    
	for(i=0; i<dst_height; i++)
	{
		i_int=i+src_op_y;
		for(j=0; j<dst_width; j++)
		{
			j_int=j+src_op_x;

			if (j==(dst_width-1))
				j=j;
			sum_a=0;
			sum_r=0;
			sum_g=0;
			sum_b=0;
			for(k=0;k<(blur_num+1)/2;k++)
			{				
				if ((j_int-k)<0)
				{
					sum_a = sum_a + (*(pRGB_src+src_stride*(i_int)+0*4 +3 ))*((blur_num+1)/2-k);
					sum_r = sum_r + (*(pRGB_src+src_stride*(i_int)+0*4 +2 ))*((blur_num+1)/2-k);
					sum_g = sum_g + (*(pRGB_src+src_stride*(i_int)+0*4 +1 ))*((blur_num+1)/2-k);
					sum_b = sum_b + (*(pRGB_src+src_stride*(i_int)+0*4 +0 ))*((blur_num+1)/2-k);
				}
				else
				{
					sum_a = sum_a + (*(pRGB_src+src_stride*(i_int)+(j_int-k)*4 +3 ))*((blur_num+1)/2-k);
					sum_r = sum_r + (*(pRGB_src+src_stride*(i_int)+(j_int-k)*4 +2 ))*((blur_num+1)/2-k);
					sum_g = sum_g + (*(pRGB_src+src_stride*(i_int)+(j_int-k)*4 +1 ))*((blur_num+1)/2-k);
					sum_b = sum_b + (*(pRGB_src+src_stride*(i_int)+(j_int-k)*4 +0 ))*((blur_num+1)/2-k);
				}
			}
			for(k=1;k<(blur_num+1)/2;k++)
			{
				temp1 = (*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +0 ));
				if ((j_int+k)>=src_width)
				{
					sum_a = sum_a + (*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +3 ))*((blur_num+1)/2-k);
					sum_r = sum_r + (*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +2 ))*((blur_num+1)/2-k);
					sum_g = sum_g + (*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +1 ))*((blur_num+1)/2-k);
					sum_b = sum_b + (*(pRGB_src+src_stride*(i_int)+(src_width-1)*4 +0 ))*((blur_num+1)/2-k);
				}
				else
				{
					sum_a = sum_a + (*(pRGB_src+src_stride*(i_int)+(j_int+k)*4 +3 ))*((blur_num+1)/2-k);
					sum_r = sum_r + (*(pRGB_src+src_stride*(i_int)+(j_int+k)*4 +2 ))*((blur_num+1)/2-k);
					sum_g = sum_g + (*(pRGB_src+src_stride*(i_int)+(j_int+k)*4 +1 ))*((blur_num+1)/2-k);
					sum_b = sum_b + (*(pRGB_src+src_stride*(i_int)+(j_int+k)*4 +0 ))*((blur_num+1)/2-k);
				}
			}
			temp=4096*4/((blur_num+1)*(blur_num+1));
			dat_a=(u8)((sum_a*temp)>>12);
			dat_r=(u8)((sum_r*temp)>>12);
			dat_g=(u8)((sum_g*temp)>>12);
			dat_b=(u8)((sum_b*temp)>>12);
		
			dat_out = (dat_a<<24)+(dat_r<<16)+(dat_g<<8)+(dat_b);
			
			
			*(pRGB_dst+dst_stride*(i)+(j)*4+3)=dat_a;
			*(pRGB_dst+dst_stride*(i)+(j)*4+2)=dat_r;
			*(pRGB_dst+dst_stride*(i)+(j)*4+1)=dat_g;
			*(pRGB_dst+dst_stride*(i)+(j)*4+0)=dat_b;

		}
	}
}

void get_scale_coeff_h(u32 *p_coef)
{
  int i;
  u32 data0;
  u32 data1;
  u32 para[7];
  for(i = 0; i < 32; i++)
  {
    data0 = p_coef[i*2];
    data1 = p_coef[i*2+1];
    coeff_h[i][6] = data0 & 0x1ff;
    coeff_h[i][5] = (data0 >> 9) & 0x1ff;
    coeff_h[i][4] = (data0 >> 18) & 0x1ff;
    coeff_h[i][3] = ((data0 >> 27) & 0x1f) | ((data1 & 0xf) << 5);
    coeff_h[i][2] = (data1 >> 4) & 0x1ff;
    coeff_h[i][1] = (data1 >> 13) & 0x1ff;
    coeff_h[i][0] = (data1 >> 22) & 0x1ff;
    printf("\r\n {%d,%d,%d,%d,%d,%d,%d},", coeff_h[i][0], coeff_h[i][1],coeff_h[i][2],coeff_h[i][3],coeff_h[i][4],coeff_h[i][5],coeff_h[i][6]);
  }
}
void get_scale_coeff_v(u32 *p_coef)
{
  int i;
  u32 data0;
  u32 data1;
  u32 para[7];
  for(i = 0; i < 32; i++)
  {
    data0 = p_coef[i*2];
    data1 = p_coef[i*2+1];
    coeff_v[i][6] = data0 & 0x1ff;
    coeff_v[i][5] = (data0 >> 9) & 0x1ff;
    coeff_v[i][4] = (data0 >> 18) & 0x1ff;
    coeff_v[i][3] = ((data0 >> 27) & 0x1f) | ((data1 & 0xf) << 5);
    coeff_v[i][2] = (data1 >> 4) & 0x1ff;
    coeff_v[i][1] = (data1 >> 13) & 0x1ff;
    coeff_v[i][0] = (data1 >> 22) & 0x1ff;
    printf("\r\n {%d,%d,%d,%d,%d,%d,%d},", coeff_v[i][0], coeff_v[i][1],coeff_v[i][2],coeff_v[i][3],coeff_v[i][4],coeff_v[i][5],coeff_v[i][6]);
  }
}
#ifdef FAST_2D_SCALE
void	scale_fast(u8	*pRGB_src,int src_op_x,int src_op_y,int src_op_w,int src_op_h, int src_width, int src_height,	
				 u8	*pRGB_dst, int dst_width,	int	dst_height, scale_cfg_t *p_scale,
				 u8	*key_src, u8	*key_dst,int key_stride, int edge_option)
{
	long long i_int,i_fra,j_int,j_fra,coef_x,coef_y,j_int_calc,i_int_calc;
	u8 line_y_a[7],line_y_r[7],line_y_g[7],line_y_b[7],key_y[7],key_x[7];
	u8 line_x_a[7],line_x_r[7],line_x_g[7],line_x_b[7];
	u8 dat_a,dat_r,dat_g,dat_b;
	int i,j,coef[4],dat_out,w,h;
    int src_stride = src_width * 4;
    int dst_stride = dst_width * 4;
    int dst_key_stride = dst_width;
    int msk_stride = dst_width;
    long long p11 = p_scale->coef[0];
    long long p21 = p_scale->coef[1]; 	
	long long p31 = p_scale->coef[2];
    long long p22 = p_scale->coef[3];
    long long p23 = p_scale->coef[4]; 
	
    int init_phase_x = p_scale->init_phase_x;
	int init_phase_y = p_scale->init_phase_y;
    MT_BOOL disable_alpha_filter = p_scale->disable_alpha_filter;
    MT_BOOL disable_color_filter = p_scale->disable_color_filter;
    MT_BOOL disable_alpha_filter_h = p_scale->disable_alpha_filter;
    MT_BOOL disable_color_filter_h = p_scale->disable_color_filter;   	
	int src_real_h,src_real_y;
	int src_real_w,src_real_x;	
  //  printf("<%s> : <%d> :  x %d y %d a %d  c %d\n", __FUNCTION__, __LINE__, init_phase_x, init_phase_y, disable_alpha_filter, disable_color_filter);
 //   printf("<%s> : <%d> :  src x %d y %d\n", __FUNCTION__, __LINE__, src_op_x, src_op_y);
#if 0 //here need to move after 
	src_real_w = (edge_option == 2) ? (src_op_w+src_op_x) : src_width;
	src_real_x = (edge_option == 2) ? src_op_x : 0;  
	src_real_h = (edge_option == 2) ? (src_op_h+src_op_y) : src_height;
	src_real_y = (edge_option == 2) ? src_op_y : 0;
#endif    
  if(p_scale->coef[5] == 0)
  {
    disable_alpha_filter_h = p_scale->disable_alpha_filter_h;
    disable_color_filter_h = p_scale->disable_color_filter_h;     
  }
  printf("\r\n disable filter[%d,%d,%d,%d]", 
    disable_alpha_filter,disable_color_filter,disable_alpha_filter_h,disable_color_filter_h);
#if 1
  if(dst_width > src_op_w)//horizontal scale up
  {
    if(src_op_x)
    {
      src_op_x = init_phase_x / 32 + src_op_x - 1;
      init_phase_x += 32;
    }
  }
  if(dst_height > src_op_h)//vertical scale up
  {
    if(src_op_y)
    {
      src_op_y = init_phase_y / 32 + src_op_y - 1;
      init_phase_y += 32;
    }
  }
#else
  if(src_op_x)
    src_op_x = init_phase_x / 32 + src_op_x - 1;
  if(src_op_y)
    src_op_y = init_phase_y / 32 + src_op_y - 1;
  
  if(src_op_x)
    init_phase_x += 32;

  if(src_op_y)
    init_phase_y += 32;
#endif
	src_real_w = (edge_option == 2) ? (src_op_w+src_op_x) : src_width;
	src_real_x = (edge_option == 2) ? src_op_x : 0;  
	src_real_h = (edge_option == 2) ? (src_op_h+src_op_y) : src_height;
	src_real_y = (edge_option == 2) ? src_op_y : 0;
    
	coef_y = (init_phase_y<<9);
/*
printf("<%s> : <%d> :  src x %d y %d %d %d %x\n", __FUNCTION__, __LINE__, src_op_x, src_op_y, init_phase_x, init_phase_y , coef_y);
printf("<%s> : dst_height :%d dst_width : %d src %d %d\n", __FUNCTION__, dst_height, dst_width, src_width, src_height);
for(i = 0; i < src_width; i++)
{
     printf(" %02x",  *(pRGB_src + i));
}
printf("\n\n");
//*/
	for(i=0; i<dst_height; i++)
	{		
		coef_x = (init_phase_x<<9);

		i_int=(coef_y>>14)-1;
#ifndef CONFIG_MT_CHIP_SYMPHONY6 
		if (i_int < 0)
			i_int = 0;
#endif
		i_fra=(coef_y>>9)&0x1f;
		for(j=0; j<dst_width; j++)
		{
			
			j_int=(coef_x>>14)-1;
#ifndef CONFIG_MT_CHIP_SYMPHONY6      
			if (j_int < 0)
				j_int = 0;
#endif      
			j_fra=(coef_x>>9)&0x1f;

			for (w=0;w<7;w++)
			{
#if 0
				for (h=0;h<7;h++)
				{
					if ((src_op_x+j_int+w- 3)<0)
						j_int_calc = 0;
					else if ((src_op_x+j_int+w- 3)>(src_width-1))
						j_int_calc = (src_width-1);
					else
						j_int_calc = src_op_x+j_int+w- 3;

					if ((src_op_y+i_int+h- 3)<0)
						i_int_calc = 0;
					else if ((src_op_y+i_int+h- 3)>(src_height-1))
						i_int_calc = (src_height-1);
					else
						i_int_calc = src_op_y+i_int+h- 3;


					line_y_a[h] = *(pRGB_src+src_stride*(i_int_calc)+(j_int_calc)*4 +3 );
					line_y_r[h] = *(pRGB_src+src_stride*(i_int_calc)+(j_int_calc)*4 +2 );
					line_y_g[h] = *(pRGB_src+src_stride*(i_int_calc)+(j_int_calc)*4 +1 );
					line_y_b[h] = *(pRGB_src+src_stride*(i_int_calc)+(j_int_calc)*4 +0 );
         //        printf("->pRGB_src<%d %d %d> %d : %02x %02x %02x %02x\n",src_stride,(u32)i_int_calc, (u32)j_int_calc,  (u32)(src_stride*(i_int_calc)+(j_int_calc)*4), line_y_a[h], line_y_r[h] , line_y_g[h], line_y_b[h]);
					key_y[h]=*(key_src+key_stride*i_int_calc+j_int_calc);				
#else
        for (h=0;h<7;h++)
        {          
          if ((src_op_x+j_int+w- 3)<src_real_x)
            j_int_calc = src_real_x;
          else if ((src_op_x+j_int+w- 3)>(src_real_w-1))
            j_int_calc = (src_real_w-1);
          else
            j_int_calc = src_op_x+j_int+w- 3;

          if ((src_op_y+i_int+h- 3)<src_real_y)
            i_int_calc = src_real_y;
          else if ((src_op_y+i_int+h- 3)>(src_real_h-1))
            i_int_calc = (src_real_h-1);
          else
            i_int_calc = src_op_y+i_int+h- 3;


          if((edge_option==1) && (((src_op_y+i_int+h- 3)<src_real_y) || ((src_op_x+j_int+w- 3)<src_real_x)))
          {
  					line_y_a[h] = 0;
  					line_y_r[h] = 0;
  					line_y_g[h] = 0;
  					line_y_b[h] = 0;          
  					key_y[h]=0;	
          }
          else
          {            
  					line_y_a[h] = *(pRGB_src+src_stride*(i_int_calc)+(j_int_calc)*4 +3 );
  					line_y_r[h] = *(pRGB_src+src_stride*(i_int_calc)+(j_int_calc)*4 +2 );
  					line_y_g[h] = *(pRGB_src+src_stride*(i_int_calc)+(j_int_calc)*4 +1 );
  					line_y_b[h] = *(pRGB_src+src_stride*(i_int_calc)+(j_int_calc)*4 +0 );          
  					key_y[h]=*(key_src+key_stride*i_int_calc+j_int_calc);		
          }
#endif
				}
				if (key_y[2]>0)
					{
						line_y_a[2]=line_y_a[3];
						line_y_r[2]=line_y_r[3];
						line_y_g[2]=line_y_g[3];
						line_y_b[2]=line_y_b[3];
						line_y_a[1]=line_y_a[3];
						line_y_r[1]=line_y_r[3];
						line_y_g[1]=line_y_g[3];
						line_y_b[1]=line_y_b[3];
						line_y_a[0]=line_y_a[3];
						line_y_r[0]=line_y_r[3];
						line_y_g[0]=line_y_g[3];
						line_y_b[0]=line_y_b[3];
					}
					else if (key_y[1]>0)
					{
						line_y_a[1]=line_y_a[2];
						line_y_r[1]=line_y_r[2];
						line_y_g[1]=line_y_g[2];
						line_y_b[1]=line_y_b[2];
						line_y_a[0]=line_y_a[2];
						line_y_r[0]=line_y_r[2];
						line_y_g[0]=line_y_g[2];
						line_y_b[0]=line_y_b[2];
					}
					else if (key_y[0]>0)
					{
						line_y_a[0]=line_y_a[1];
						line_y_r[0]=line_y_r[1];
						line_y_g[0]=line_y_g[1];
						line_y_b[0]=line_y_b[1];
					}
            		
					if (key_y[4]>0)
					{
						line_y_a[4]=line_y_a[3];
						line_y_r[4]=line_y_r[3];
						line_y_g[4]=line_y_g[3];
						line_y_b[4]=line_y_b[3];
						line_y_a[5]=line_y_a[3];
						line_y_r[5]=line_y_r[3];
						line_y_g[5]=line_y_g[3];
						line_y_b[5]=line_y_b[3];
						line_y_a[6]=line_y_a[3];
						line_y_r[6]=line_y_r[3];
						line_y_g[6]=line_y_g[3];
						line_y_b[6]=line_y_b[3];
					}
					else if (key_y[5]>0)
					{
						line_y_a[5]=line_y_a[4];
						line_y_r[5]=line_y_r[4];
						line_y_g[5]=line_y_g[4];
						line_y_b[5]=line_y_b[4];
						line_y_a[6]=line_y_a[4];
						line_y_r[6]=line_y_r[4];
						line_y_g[6]=line_y_g[4];
						line_y_b[6]=line_y_b[4];
					}
					else if (key_y[6]>0)
					{
						line_y_a[6]=line_y_a[5];
						line_y_r[6]=line_y_r[5];
						line_y_g[6]=line_y_g[5];
						line_y_b[6]=line_y_b[5];
					}
				//line_x_a[w]=filter6(line_y_a,i_fra);
				//line_x_r[w]=filter6(line_y_r,i_fra);
				//line_x_g[w]=filter6(line_y_g,i_fra);
				//line_x_b[w]=filter6(line_y_b,i_fra);

                if(disable_alpha_filter)
                    line_x_a[w] = line_y_a[3];
                else
#ifdef SYMPHONY6_TEST
                    line_x_a[w] = filter6_v(line_y_a,i_fra);
#else
                    line_x_a[w] = filter6(line_y_a,i_fra);
#endif
                if(disable_color_filter)
                {
                    line_x_r[w] = line_y_r[3];
                    line_x_g[w] = line_y_g[3];
                    line_x_b[w] = line_y_b[3];
                }
                else
                {
#ifdef SYMPHONY6_TEST
                    line_x_r[w]=filter6_v(line_y_r,i_fra);
                    line_x_g[w]=filter6_v(line_y_g,i_fra);
                    line_x_b[w]=filter6_v(line_y_b,i_fra);
#else                
                    line_x_r[w]=filter6(line_y_r,i_fra);
                    line_x_g[w]=filter6(line_y_g,i_fra);
                    line_x_b[w]=filter6(line_y_b,i_fra);
#endif                    
                }

				key_x[w]=key_y[3];
			}
			if (key_x[2]>0)
			{
				line_x_a[2]=line_x_a[3];
				line_x_r[2]=line_x_r[3];
				line_x_g[2]=line_x_g[3];
				line_x_b[2]=line_x_b[3];
				line_x_a[1]=line_x_a[3];
				line_x_r[1]=line_x_r[3];
				line_x_g[1]=line_x_g[3];
				line_x_b[1]=line_x_b[3];
				line_x_a[0]=line_x_a[3];
				line_x_r[0]=line_x_r[3];
				line_x_g[0]=line_x_g[3];
				line_x_b[0]=line_x_b[3];
			}
			else if (key_x[1]>0)
			{
				line_x_a[1]=line_x_a[2];
				line_x_r[1]=line_x_r[2];
				line_x_g[1]=line_x_g[2];
				line_x_b[1]=line_x_b[2];
				line_x_a[0]=line_x_a[2];
				line_x_r[0]=line_x_r[2];
				line_x_g[0]=line_x_g[2];
				line_x_b[0]=line_x_b[2];
			}
			else if (key_x[0]>0)
			{
				line_x_a[0]=line_x_a[1];
				line_x_r[0]=line_x_r[1];
				line_x_g[0]=line_x_g[1];
				line_x_b[0]=line_x_b[1];
			}
            
			if (key_x[4]>0)
			{
				line_x_a[4]=line_x_a[3];
				line_x_r[4]=line_x_r[3];
				line_x_g[4]=line_x_g[3];
				line_x_b[4]=line_x_b[3];
				line_x_a[5]=line_x_a[3];
				line_x_r[5]=line_x_r[3];
				line_x_g[5]=line_x_g[3];
				line_x_b[5]=line_x_b[3];
				line_x_a[6]=line_x_a[3];
				line_x_r[6]=line_x_r[3];
				line_x_g[6]=line_x_g[3];
				line_x_b[6]=line_x_b[3];
			}
			else if (key_x[5]>0)
			{
				line_x_a[5]=line_x_a[4];
				line_x_r[5]=line_x_r[4];
				line_x_g[5]=line_x_g[4];
				line_x_b[5]=line_x_b[4];
				line_x_a[6]=line_x_a[4];
				line_x_r[6]=line_x_r[4];
				line_x_g[6]=line_x_g[4];
				line_x_b[6]=line_x_b[4];
			}
			else if (key_x[6]>0)
			{
				line_x_a[6]=line_x_a[5];
				line_x_r[6]=line_x_r[5];
				line_x_g[6]=line_x_g[5];
				line_x_b[6]=line_x_b[5];
			}
			//dat_a=filter6(line_x_a,j_fra);
			//dat_r=filter6(line_x_r,j_fra);
			//dat_g=filter6(line_x_g,j_fra);
			//dat_b=filter6(line_x_b,j_fra);

//            if(disable_alpha_filter)
            if(disable_alpha_filter_h)
                dat_a = line_x_a[3];
            else
#ifdef SYMPHONY6_TEST
                dat_a=filter6_h(line_x_a,j_fra);
#else               
                dat_a=filter6(line_x_a,j_fra);
#endif
//            if(disable_color_filter)
            if(disable_color_filter_h)
            {
                dat_r = line_x_r[3];
                dat_g = line_x_g[3];
                dat_b = line_x_b[3];
            }
            else
            {
#ifdef SYMPHONY6_TEST
                dat_r=filter6_h(line_x_r,j_fra);
                dat_g=filter6_h(line_x_g,j_fra);
                dat_b=filter6_h(line_x_b,j_fra);
#else             
                dat_r=filter6(line_x_r,j_fra);
                dat_g=filter6(line_x_g,j_fra);
                dat_b=filter6(line_x_b,j_fra);
#endif                
            }

			dat_out = (dat_a<<24)+(dat_r<<16)+(dat_g<<8)+(dat_b);
    //	printf("dst pix : %02x %02x %02x %02x\n",dat_a , dat_r, dat_g, dat_b);
			*(pRGB_dst+dst_stride*(i)+(j)*4+3)=(u8)dat_a;
			*(pRGB_dst+dst_stride*(i)+(j)*4+2)=(u8)dat_r;
			*(pRGB_dst+dst_stride*(i)+(j)*4+1)=(u8)dat_g;
			*(pRGB_dst+dst_stride*(i)+(j)*4+0)=(u8)dat_b;
			*(key_dst+dst_key_stride*(i)+(j))=(u8)key_x[3];
			coef_x = coef_x + (p11);

		}
		coef_y = coef_y + (p22>>4);
	}
}
#endif

#pragma GCC diagnostic pop

#endif
