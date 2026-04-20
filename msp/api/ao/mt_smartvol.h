#ifndef SMARTVOL_H
#define SMARTVOL_H

#include "mt_type.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENA_FIXPOINT

#ifdef ENA_FIXPOINT
typedef mt_s32    sample_t;
#else
typedef mt_double sample_t;
#endif

#ifndef MT_ERR_SMARTVOL_PREFIX
#define MT_ERR_SMARTVOL_PREFIX 0x80000000
#endif

/* input handle is invalid */
#define MT_ERR_SMARTVOL_HANDLE       (MT_ERR_SMARTVOL_PREFIX | 0x0001)

/* PCM buffer is invalid */
#define MT_ERR_SMARTVOL_PCMBUF       (MT_ERR_SMARTVOL_PREFIX | 0x0002)

/* PCM buffer is invalid */
#define MT_ERR_SMARTVOL_MALLOC_FAIL  (MT_ERR_SMARTVOL_PREFIX | 0x0003)

/* input parameter is invalid */
#define MT_ERR_SMARTVOL_PARA_INVALID (MT_ERR_SMARTVOL_PREFIX | 0x0004)

/* unknown error */
#define MT_ERR_SMARTVOL_UNKNOWN      (MT_ERR_SMARTVOL_PREFIX | 0x0005)

#define ENA_INTERLEAVED_FORMAT  /* Interleaved pcm format, add by z40717 */


#define REALTIME_DETECT_ENABLE       (0x1)
#define AMPLIFER_RESTRICT_ENABLE     (0x2)
#define BIGVOLUME_DETECT_ENABLE      (0x4)
#define VAD_ENABLE                   (0x8)

typedef struct _SamrtVol_In_Para_S
{
    MT_S32 samprate;               // sample rate
    MT_S32 nchans;                 // number of channels
    MT_S32 framelen;               // samples in one frame
    MT_S32 bitdepth;               // bit depth
    MT_S32 fadetime;               // time for fade (ms)
    MT_S32 analyzetime;            // time for one analyze cycle(ms)

    mt_u32 ControlPara;
    mt_u32 ChannelNo;
    MT_S32 RandomVol;
#ifdef ENA_INTERLEAVED_FORMAT
    MT_S32 Interleaved;           // 0:none-Interleaved, 1: Interleaved
#endif
} SamrtVol_In_Para_S, *SamrtVol_In_Para_P;

typedef void* HSmartVol;

/*****************************************************************************
Function:    MT_SmartGetDefaultConfig
Description: get default config parameters
Calls:
Called By:
Input:       InPara -- Vol controller parameters
Output:
Return:      Return:      =0 success
             <0 error code
Others:
*****************************************************************************/
MT_S32 MT_SmartGetDefaultConfig    (SamrtVol_In_Para_P pInPara);

/*****************************************************************************
Function:    MT_SmartVolOpen
Description: create and initial smartvol controller
Calls:
Called By:
Input:       InPara -- Vol controller parameters
Output:
Return:      >0 handle of smartvol controller
             =0 error
Others:
*****************************************************************************/
HSmartVol MT_SmartVolOpen    (SamrtVol_In_Para_P pInPara);

/*****************************************************************************
Function:    MT_SmartVolPro
Description: do smartvol control
Calls:
Called By:
Input:       hSmartVol -- handle of smartvol controller
             inbuf     -- input buffer
             GainMod   -- gain modified
             len       -- sample number of input buffer per channel
Output:      inbuf     -- output buffer == input buffer
Return:      =0 success
             <0 error code
Others:
*****************************************************************************/
MT_S32    MT_SmartVolPro     (HSmartVol hSmartVol, MT_S32* inLbuf, MT_S32* inRbuf, MT_S32 GainMod, MT_S32 len);

/*****************************************************************************
Function:    MT_SmartVolClear
Description: if channel changed, clear the status
Calls:
Called By:
Input:       hSmartVol -- handle of smartvol controller
             InPara    -- Vol controller parameters
Output:
Return:      =0 success
             <0 error code
Others:
*****************************************************************************/
MT_S32    MT_SmartVolClear   (HSmartVol hSmartVol, SamrtVol_In_Para_P pInPara);

/*****************************************************************************
Function:    MT_SmartVolEnable
Description: enable the smartvol control function
Calls:
Called By:
Input:       hSmartVol -- handle of smartvol controller
Output:
Return:      =0 success
             <0 error code
Others:
*****************************************************************************/
MT_S32    MT_SmartVolEnable  (HSmartVol hSmartVol);

/*****************************************************************************
Function:    MT_SmartVolDisable
Description: disable the smartvol control function
Calls:
Called By:
Input:       hSmartVol -- handle of smartvol controller
Output:
Return:      =0 success
             <0 error code
Others:
*****************************************************************************/
MT_S32    MT_SmartVolDisable (HSmartVol hSmartVol);

/*****************************************************************************
Function:    MT_SmartVolClose
Description: close smartvol controller, and free memory
Calls:
Called By:
Input:       hSmartVol -- handle of smartvol controller
Output:
Return:
Others:
*****************************************************************************/
mt_void   MT_SmartVolClose   (HSmartVol hSmartVol);

#ifdef __cplusplus
}
#endif

#endif /* SMARTVOL_H */
