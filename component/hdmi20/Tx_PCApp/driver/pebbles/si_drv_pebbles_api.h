/******************************************************************************
*
* Copyright 2013, Deco, Inc.  All rights reserved.
* No part of this work may be reproduced, modified, distributed, transmitted,
* transcribed, or translated into any language or computer format, in any form
* or by any means without written permission of
* Deco, Inc., 1140 East Arques Avenue, Sunnyvale, California 94085
*
*****************************************************************************/
/**
* @file si_drv_pebbles_api.h
*
* @brief
*
*****************************************************************************/

#ifndef __SI_DRV_PEBBLES_API_H__
#define __SI_DRV_PEBBLES_API_H__

/***** #include statements ***************************************************/
#include "si_drv_tx_api.h"

/***** public macro definitions **********************************************/

#define DEVICE_ID_PEBBLES               0x68

// Pebbles Info frame Register Offsets

#define SII_INFO_FRAME_OFFSET__AVI         (0x40)
#define SII_INFO_FRAME_OFFSET__AUDIO       (0x80)
#define SII_INFO_FRAME_OFFSET__SPD         (0x60)
#define SII_INFO_FRAME_OFFSET__MPEG        (0xA0)
#define SII_INFO_FRAME_OFFSET__ACP         (0xE0)

#define SII_INFO_FRAME_OFFSET__VS          (0x40)		    // To be Corrected
#define SII_INFO_FRAME_OFFSET__GBD         (0x40)		    // To be Corrected
#define SII_INFO_FRAME_OFFSET__ISRC        (0x40)		    // To be Corrected
#define SII_INFO_FRAME_OFFSET__ISRC2       (0x40)		    // To be Corrected

#define SII_INFO_FRAME_LEN__AVI         (17)            // ( (AVI Type (1) + AVI Ver(1) + AVI Len (1) + Checksum (1)) + data (13))
// ( (   0x82 +         0x02 +      0x0D)

#define SII_INFO_FRAME_LEN__AUDIO       (14)		    // ( (AUD Type (1) + AUD Ver(1) + AUD Len (1) + Checksum (1)) + data (13))
// ( (   0x84 +         0x01 +      0x0A)

#define SII_INFO_FRAME_LEN__VS          (31)		    // ( (VSIF Type (1) + VSIF Ver(1) + VSIF Len (1) + IEEE ID (3)) + data (Len-3))
// ((   0x01 +         0x01      )

#define SII_INFO_FRAME_LEN__SPD         (31)		    // ( (AVI Type (1) + AVI Ver(1) + AVI Len (1) + Checksum (1)) + data (13))
// ( (   0x83 +         0x01 +      0x19)

#define SII_INFO_FRAME_LEN__GBD         (31)		    // ( (Header (3) ) + data (28))
// ( (   0x04    )            )

#define SII_INFO_FRAME_LEN__MPEG        (14)		    // ( (MPEG Type (1) + MPEG Ver(1) + MPEG Len (1) + Checksum (1)) + data (13))
// ( (   0x85 +         0x01 +      0x0A)

#define SII_INFO_FRAME_LEN__ISRC        (31)		    // ( (Header (3) ) + data (28))
// ( (   0x05    )            )

#define SII_INFO_FRAME_LEN__ISRC2       (31)		    // ( (Header (3) ) + data (28))
// ( (   0x06    )            )

#define SII_INFO_FRAME_LEN__ACP         (31)		    // ( (Header (3) ) + data (28))
// ( (   0x04    )            )

#define SII_INFO_FRAME_LEN__HDR         (29)        // ( (Header (3) ) + data (26))
/***** public functions ******************************************************/

void SiiPebblesInfoframeGet(SiiInst_t inst, SiiInfoFrameId_t ifId, SiiInfoFrame_t *pInfoFrame);

#endif // __SI_DRV_PEBBLES_API_H__
