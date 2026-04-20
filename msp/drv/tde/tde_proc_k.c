#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif  /* __cplusplus */
#endif  /* __cplusplus */

#include "tde_hal.h"

#include "tde_proc.h"
#include "tde_config.h"

#ifndef CONFIG_TDE_PROC_DISABLE
typedef struct _hiTDE_PROCINFO_S
{
    mt_u32          u32CurNode;
    TDE_HWNode_S    stTdeHwNode[TDE_MAX_PROC_NUM];
    MT_BOOL         bProcEnable;
}TDE_PROCINFO_S;

#ifdef USE_USER_SPACE_GPE
static MT_TDE_PROC_INFO_S g_stTdeProc = {0};
#endif


TDE_PROCINFO_S g_stTdeProcInfo =
{
	.u32CurNode = 0,
	.bProcEnable = MT_TRUE,
};

mt_void TDEProcEnable(MT_BOOL bEnable)
{
    g_stTdeProcInfo.bProcEnable = bEnable;
}


mt_void TDEProcRecordNode(TDE_HWNode_S* pHWNode)
{
    if ((!g_stTdeProcInfo.bProcEnable) || (MT_NULL == pHWNode))
    {
        return;
    }

    memcpy(&g_stTdeProcInfo.stTdeHwNode[g_stTdeProcInfo.u32CurNode], pHWNode, sizeof(TDE_HWNode_S));

    g_stTdeProcInfo.u32CurNode++;
    g_stTdeProcInfo.u32CurNode = (g_stTdeProcInfo.u32CurNode)%TDE_MAX_PROC_NUM;
}

mt_void TDEProcClearNode(mt_void)
{
    memset(&g_stTdeProcInfo.stTdeHwNode[0], 0, sizeof(g_stTdeProcInfo.stTdeHwNode));
    g_stTdeProcInfo.u32CurNode = 0;
}

mt_void TDEProcSetInfo(MT_TDE_PROC_INFO_S *pProc)
{
  if(g_stTdeProcInfo.bProcEnable)
    memcpy(&g_stTdeProc, pProc, sizeof(MT_TDE_PROC_INFO_S));
}


mt_void TDE_Proc_init(mt_void)
{
		GFX_PROC_ITEM_S pProcItem;
		MT_CHAR *pEntry_name = "tde";

		pProcItem.fnRead   = tde_read_proc;
		pProcItem.fnWrite  = tde_write_proc;
		pProcItem.fnIoctl  = NULL;

  	MT_GFX_PROC_AddModule(pEntry_name,&pProcItem,(MT_VOID *)(&g_stTdeProc));
}

mt_void TDE_Proc_Cleanup(mt_void)
{
     MT_CHAR *pEntry_name =  "tde";
     MT_GFX_PROC_RemoveModule(pEntry_name);
}


int tde_write_proc(struct file * file,
    const char __user * buf, size_t count, loff_t *ppos)
{
    char buffer[128] = {0};

    if(count > sizeof(buffer))
    {
        TDE_TRACE(TDE_KERN_INFO, "The command string is out of buf space :%d bytes !\n", sizeof(buffer));
        return 0;
    }
    else
    {

        if(copy_from_user(buffer, buf, count))
        {
            TDE_TRACE(TDE_KERN_INFO, "<%s> Line %d:copy_from_user failed!\n", __FUNCTION__, __LINE__);
            return 0;
        }
        else
        {

            buffer[sizeof(buffer) - 1] = '\0';
            if(strstr(buffer, "proc on"))
            {

                TDEProcEnable(MT_TRUE);
                TDE_TRACE(TDE_KERN_INFO, "tde proc on\n");
            }
            else if (strstr(buffer, "proc off"))
            {
                TDEProcEnable(MT_FALSE);
                TDE_TRACE(TDE_KERN_INFO, "tde proc off\n");
            }
            else if (strstr(buffer, "node clear"))
            {
                TDEProcClearNode();
                TDE_TRACE(TDE_KERN_INFO, "node buffer was cleared\n");
            }
            else
            {
                TDE_TRACE(TDE_KERN_INFO, "The command string is illegimate\n");
                return 0;
            }
        }
    }
    return count;
}
int tde_read_proc(struct seq_file *p, mt_void *v)
{
#ifndef USE_USER_SPACE_GPE

    mt_s32 len = 0;
    mt_s32 i, j;
    mt_u32* pu32Cur;

    /* see define of TDE_HWNode_S */
    mt_u8*  chUpdate[] =
    {
        "INS         ",
        "S1_ADDR     ",
        "S1_TYPE     ",
        "S1_XY       ",
        "S1_FILL     ",
        "S2_ADDR     ",
        "S2_TYPE     ",
        "S2_XY       ",
        "S2_SIZE     ",
        "S2_FILL     ",
        "TAR_ADDR    ",
        "TAR_TYPE    ",
        "TAR_XY      ",
        "TS_SIZE     ",
        "COLOR_CONV  ",
        "CLUT_ADDR   ",
        "2D_RSZ      ",
        "HF_COEF_ADDR",
        "VF_COEF_ADDR",
        "RSZ_STEP    ",
        "RSZ_Y_OFST  ",
        "RSZ_X_OFST  ",
        "DFE_COEF0   ",
        "DFE_COEF1   ",
        "DFE_COEF2   ",
        "DFE_COEF3   ",
        "ALU         ",
        "CK_MIN      ",
        "CK_MAX      ",
        "CLIP_START  ",
        "CLIP_STOP   ",
        "Y1_ADDR     ",
        "Y1_PITCH    ",
        "Y2_ADDR     ",
        "Y2_PITCH    ",
        "RSZ_VSTEP   ",
        "ARGB_ORDER  ",
        "CK_MASK     ",
        "COLORIZE    ",
        "ALPHA_BLEND ",
        "ICSC_ADDR   ",
        "OCSC_ADDR   "
    };

    TDE_HWNode_S *pstHwNode = g_stTdeProcInfo.stTdeHwNode;
    p = wprintinfo(p+len);
     #ifndef CONFIG_TDE_STR_DISABLE

    for (j = 0 ; j < g_stTdeProcInfo.u32CurNode; j++)
    {
        pu32Cur = (mt_u32*)&pstHwNode[j];
         /* print node information */
        PROC_PRINT(p,"\n--------- Montage TDE Node params Info ---------\n");

        for (i = 0; i < sizeof(TDE_HWNode_S) / 4; i++)
        {
            PROC_PRINT(p, "(%s):\t0x%08x\n", chUpdate[i], *(pu32Cur + i));
        }
    }

    #endif
    return 0;
#else
    mt_u32 i;
		mt_proc_entry_t 	*item  = NULL;
		MT_TDE_PROC_INFO_S *p_proc = NULL;

		item = (mt_proc_entry_t *)(p->private);
		p_proc = (MT_TDE_PROC_INFO_S *)(item->data);

    if(g_stTdeProcInfo.bProcEnable)
    {
      MT_TDE_PROC_PARAM_S *p_ctx = &(p_proc->tde_param);

      if(p_ctx->src_img_en)
      {
        PROC_PRINT(p,"\n--------- Montage TDE params Info ---------\n");
        PROC_PRINT(p,"\n--------- src Info ---------\n");
        PROC_PRINT(p,"\r\n src_en:%d", p_ctx->src_img_en);
        PROC_PRINT(p,"\r\n pitch:%d, %dx%d, rect:[%d, %d, %d, %d],", p_ctx->src_img.pitch, p_ctx->src_img.width, p_ctx->src_img.height,
          p_ctx->src_img.rect.x, p_ctx->src_img.rect.y, p_ctx->src_img.rect.w, p_ctx->src_img.rect.h);
        PROC_PRINT(p,"\r\n addr:0x%08x", p_ctx->src_img.buf);
        PROC_PRINT(p,"\r\n format:%d", p_ctx->src_img.pix_format);
      }
      if(p_ctx->bg_img_en)
      {
        PROC_PRINT(p,"\n--------- bg Info ---------\n");
        PROC_PRINT(p,"\r\n bg_en:%d", p_ctx->bg_img_en);
        PROC_PRINT(p,"\r\n pitch:%d, %dx%d, rect:[%d, %d, %d, %d],", p_ctx->bg_img.pitch, p_ctx->bg_img.width, p_ctx->bg_img.height,
          p_ctx->bg_img.rect.x, p_ctx->bg_img.rect.y, p_ctx->bg_img.rect.w, p_ctx->bg_img.rect.h);
        PROC_PRINT(p,"\r\n addr:0x%08x", p_ctx->bg_img.buf);
      }
      if(p_ctx->ex_img_en)
      {
        PROC_PRINT(p,"\n--------- ex Info ---------\n");
        PROC_PRINT(p,"\r\n ex_en:%d", p_ctx->ex_img_en);
        PROC_PRINT(p,"\r\n pitch:%d, %dx%d, rect:[%d, %d, %d, %d],", p_ctx->ex_img.pitch, p_ctx->ex_img.width, p_ctx->ex_img.height,
          p_ctx->ex_img.rect.x, p_ctx->ex_img.rect.y, p_ctx->ex_img.rect.w, p_ctx->ex_img.rect.h);
        PROC_PRINT(p,"\r\n addr:0x%08x", p_ctx->ex_img.buf);
      }

      PROC_PRINT(p,"\n--------- dst Info ---------\n");
      PROC_PRINT(p,"\r\n pitch:%d, %dx%d, rect:[%d, %d, %d, %d],", p_ctx->dst_img.pitch, p_ctx->dst_img.width, p_ctx->dst_img.height,
        p_ctx->dst_img.rect.x, p_ctx->dst_img.rect.y, p_ctx->dst_img.rect.w, p_ctx->dst_img.rect.h);
      PROC_PRINT(p,"\r\n addr:0x%08x", p_ctx->dst_img.buf);

      PROC_PRINT(p,"\n--------- opt Info ---------\n");
      if((p_ctx->gpe_op & GPE_OP_BLEND) == GPE_OP_BLEND)
      {
        PROC_PRINT(p,"\r\n blend enable, blend mode:[%d, %d], alpha blend mode:[%d, %d]",
          p_ctx->blend.src_blend_fact, p_ctx->blend.dst_blend_fact, p_ctx->blend_alpha.src_blend_fact, p_ctx->blend_alpha.dst_blend_fact);
      }
      if((p_ctx->gpe_op & GPE_OP_ROP) == GPE_OP_ROP)
      {
        PROC_PRINT(p,"\r\n rop enable, rop mode:[%d, %d, 0x%08x]", p_ctx->rop.rop_a_id, p_ctx->rop.rop_c_id, p_ctx->rop.rop_pattern);
      }
      if((p_ctx->gpe_op & GPE_OP_SCALE) == GPE_OP_SCALE)
        PROC_PRINT(p,"\r\n scale enable");
      if((p_ctx->gpe_op & GPE_OP_PAINT) == GPE_OP_PAINT)
        PROC_PRINT(p,"\r\n paint enable");
      if((p_ctx->gpe_op & GPE_OP_DSTEN) == GPE_OP_DSTEN)
        PROC_PRINT(p,"\r\n draw stencil enable");
      if((p_ctx->gpe_op & GPE_OP_DMULT) == GPE_OP_DMULT)
        PROC_PRINT(p,"\r\n draw multiply enable");
      if((p_ctx->gpe_op & GPE_OP_ROTATE) == GPE_OP_ROTATE)
        PROC_PRINT(p,"\r\n rotator enable, mode:%d", p_ctx->rotator_op);
      if((p_ctx->gpe_op & GPE_OP_ALPHAMAP) == GPE_OP_ALPHAMAP)
        PROC_PRINT(p,"\r\n rotator enable, mode:%d", p_ctx->alpha_map_mod);

      PROC_PRINT(p,"\n--------- colorkey Info ---------\n");
      if(p_ctx->src_img.ck_en)
      {
        PROC_PRINT(p,"\r\n src colorkey enable, [%d, %d], mode:%d, select:%d",
          p_ctx->src_img.ck_min, p_ctx->src_img.ck_max, p_ctx->src_img.key_color_mod, p_ctx->src_img.key_color_select);
      }
      if(p_ctx->dst_img.ck_en)
      {
        PROC_PRINT(p,"\r\n dst colorkey enable, [%d, %d], mode:%d, select:%d",
          p_ctx->dst_img.ck_min, p_ctx->dst_img.ck_max, p_ctx->dst_img.key_color_mod, p_ctx->dst_img.key_color_select);
      }
      if(p_ctx->ex_img.ck_en)
      {
        PROC_PRINT(p,"\r\n ex colorkey enable, [%d, %d], mode:%d, select:%d",
          p_ctx->ex_img.ck_min, p_ctx->ex_img.ck_max, p_ctx->ex_img.key_color_mod, p_ctx->ex_img.key_color_select);
      }

      PROC_PRINT(p,"\n--------- regisiter Info ---------\n");

      for(i = 0; i < TDE_REG_MAX / 4; i++)
        PROC_PRINT(p, "\r\n REG 0x%08x = 0x%08x", p_proc->tde_reg[i].u32RegAddr, p_proc->tde_reg[i].u32RegVal);

      return 0;

    }
#endif

	return 0;
}
#endif

#ifdef __cplusplus
 #if __cplusplus
}
 #endif /* __cplusplus */
#endif  /* __cplusplus */
