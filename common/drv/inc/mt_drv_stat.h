#ifndef __MT_DRV_STAT_H__
#define __MT_DRV_STAT_H__

#include "mt_type.h"

#ifdef __cplusplus
 #if __cplusplus
extern "C" {
 #endif
#endif /* __cplusplus */

#define MT_FATAL_STAT(fmt...) MT_FATAL_PRINT(MT_ID_STAT, fmt)
#define MT_ERR_STAT(fmt...) MT_ERR_PRINT(MT_ID_STAT, fmt)
#define MT_WARN_STAT(fmt...) MT_WARN_PRINT(MT_ID_STAT, fmt)
#define MT_INFO_STAT(fmt...) MT_INFO_PRINT(MT_ID_STAT, fmt)
#define MT_DBG_STAT(fmt...) MT_DBG_PRINT(MT_ID_STAT, fmt)

typedef enum
{
    STAT_ISR_AUDIO = 0x0,
    STAT_ISR_VIDEO,
    STAT_ISR_DEMUX,
    STAT_ISR_SYNC,
    STAT_ISR_VO,
    STAT_ISR_TDE,
    STAT_ISR_BUTT
}STAT_ISR_E;

typedef enum tagSTAT_EVENT
{
	STAT_EVENT_KEYIN,
	STAT_EVENT_KEYOUT,
	STAT_EVENT_ASTOP_IN,
	STAT_EVENT_ASTOP,
	STAT_EVENT_VSTOP_IN,
	STAT_EVENT_VSTOP,
	STAT_EVENT_CONNECT,
	STAT_EVENT_LOCKED,
	STAT_EVENT_ASTART_IN,
	STAT_EVENT_ASTART,
	STAT_EVENT_VSTART_IN,
	STAT_EVENT_VSTART,
	STAT_EVENT_CWSET,
	STAT_EVENT_STREAMIN,
	STAT_EVENT_ISTREAMGET,
	STAT_EVENT_IFRAMEOUT,
	STAT_EVENT_VPSSGETFRM,
	STAT_EVENT_VPSSOUTFRM,
	STAT_EVENT_AVPLAYGETFRM,    
	STAT_EVENT_PRESYNC,
	STAT_EVENT_BUFREADY,
	STAT_EVENT_FRAMESYNCOK,
	STAT_EVENT_VOGETFRM,
	STAT_EVENT_OPENSCREEM,
	STAT_EVENT_SYNCVIDEO,
	STAT_EVENT_IFRAMEINTER,
	STAT_EVENT_SYNCDONE,
	STAT_EVENT_BUTT
}STAT_EVENT_E;


typedef mt_void (*stat_event_fun)(STAT_EVENT_E, mt_u32);

mt_s32  mt_drv_stat_eventfunc_register(mt_void* pFunc);
mt_void mt_drv_stat_eventfunc_unregister(mt_void);
mt_s32  mt_drv_stat_kinit(mt_void);
mt_void mt_drv_stat_kexit(mt_void);

mt_s32  mt_drv_stat_init(mt_void);
mt_void mt_drv_stat_exit(mt_void);

/*interrupt cost time in kernel-state*/
/*CNcomment:内核态中断耗时统计*/
#if defined(MT_STAT_ISR_SUPPORTED)
mt_void mt_drv_stat_isrreset(mt_void);
mt_void mt_drv_stat_isrenable(mt_void);
mt_void mt_drv_stat_isrdisable(mt_void);

mt_void mt_drv_stat_isrbegin(STAT_ISR_E isr);
mt_void mt_drv_stat_isrend(STAT_ISR_E isr);
#endif

mt_void mt_drv_stat_event(STAT_EVENT_E enEvent, mt_u32 Value);
mt_u32  mt_drv_stat_gettick(mt_void);

/* ---------------- low delay statistics structs and export functions ----------------------- */

/*
 * scenes type definition
 */
typedef enum {
    SCENES_VID_PLAY = 0,
    SCENES_VID_CAP,
    SCENES_VID_CAST,
    SCENES_LD_BUTT,
}MT_LD_SCENES_E;

/*
 * event type definition
 */
typedef enum {
    EVENT_VI_FRM_IN = 0,
    EVENT_VI_FRM_OUT,
    EVENT_VPSS_FRM_IN,
    EVENT_VPSS_FRM_OUT,
    EVENT_VDEC_FRM_IN,
    EVENT_VDEC_FRM_OUT,
    EVENT_VENC_FRM_IN,
    EVENT_VENC_FRM_OUT,
    EVENT_AVPLAY_FRM_IN,
    EVENT_AVPLAY_FRM_OUT,
    EVENT_VO_FRM_IN,
    EVENT_VO_FRM_OUT,
    EVENT_CAST_FRM_BEGIN,
    EVENT_CAST_FRM_OUT,
    EVENT_LD_BUTT,
}MT_LD_EVENT_ID_E;

/*
 * low delay event definition
 */
typedef struct {
    MT_LD_EVENT_ID_E    evt_id;
    mt_u32                   handle;
    mt_u32                   frame; 
    mt_u32                   time;
}mt_ld_event_s;

#define MAX_EVENT_QUEUE_SIZE (10 + 1)      /* ring queue need one more entry for diff empty and full status */
#define MAX_SCENES_TYPE_NR    (SCENES_LD_BUTT)
#define MAX_EVENT_TYPE_NR      (EVENT_LD_BUTT) 

/*
 * define scenes what's the composition of which events
 */
static const MT_LD_EVENT_ID_E g_scenes_desc[MAX_SCENES_TYPE_NR][MAX_EVENT_TYPE_NR] __attribute__((unused)) = 
{
    /* SCENES_VID_PLAY: VDEC_IN -> VDEC_OUT ->VPSS_IN -> VPSS_OUT -> AVPLAY_IN-> AVPLAY_OUT -> VO_IN->VO_OUT */
    {EVENT_VDEC_FRM_IN, EVENT_VDEC_FRM_OUT, EVENT_VPSS_FRM_IN, EVENT_VPSS_FRM_OUT, EVENT_AVPLAY_FRM_IN, EVENT_AVPLAY_FRM_OUT, EVENT_VO_FRM_IN, EVENT_VO_FRM_OUT, EVENT_LD_BUTT},
    /* SCENES_VID_CAP: VI_IN -> VPSS_IN -> VPSS_OUT -> VI_OUT -> VENC_IN -> VENC_OUT */
    {EVENT_VI_FRM_IN, EVENT_VPSS_FRM_IN, EVENT_VPSS_FRM_OUT, EVENT_VI_FRM_OUT, EVENT_VENC_FRM_IN,  EVENT_VENC_FRM_OUT, EVENT_LD_BUTT},
    /* SCENES_VID_CAST: DISP_OUT -> VENC_IN -> VENC_OUT */
    {EVENT_CAST_FRM_BEGIN,EVENT_CAST_FRM_OUT, EVENT_VENC_FRM_IN, EVENT_VENC_FRM_OUT, EVENT_LD_BUTT},
};

/*
 * define event name
 */
 static const mt_char * g_event_name[MAX_EVENT_TYPE_NR] __attribute__((unused)) = 
{
    [EVENT_VI_FRM_IN]          = "VI_IN",
    [EVENT_VI_FRM_OUT]       = "VI_OUT",
    [EVENT_VPSS_FRM_IN]      = "VPSS_IN",
    [EVENT_VPSS_FRM_OUT]   = "VPSS_OUT",
    [EVENT_VDEC_FRM_IN]      = "VDEC_IN",
    [EVENT_VDEC_FRM_OUT]    = "VDEC_OUT",
    [EVENT_VENC_FRM_IN]       = "VENC_IN",
    [EVENT_VENC_FRM_OUT]    = "VENC_OUT",
    [EVENT_AVPLAY_FRM_IN]    = "AVPLAY_IN",
    [EVENT_AVPLAY_FRM_OUT] = "AVPLAY_OUT",
    [EVENT_VO_FRM_IN]          = "VO_IN",
    [EVENT_VO_FRM_OUT]       = "VO_OUT",
    [EVENT_CAST_FRM_BEGIN]   = "CAST_FRAME_BEGIN",
    [EVENT_CAST_FRM_OUT]    = "CAST_OUT",
};

mt_s32 mt_drv_ld_start_statistics(MT_LD_SCENES_E scenes_id, mt_void *filter);
mt_void mt_drv_ld_stop_statistics(mt_void);
mt_void mt_drv_ld_notify_event(mt_ld_event_s *evt);



#ifdef __cplusplus
 #if __cplusplus
}
 #endif
#endif /* __cplusplus */

#endif /* __MT_DRV_STAT_H__ */

