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
* @file
*
* @brief Infoframes Setting API
*
*****************************************************************************/

#ifndef __SI_TEST_SET_INFOFRAMES_API_H__
#define __SI_TEST_SET_INFOFRAMES_API_H__

void SetAVIF(int level, SiiInst_t inst);
void SetAudioIF(SiiInst_t inst);
void SetVSIF( SiiInst_t inst);
void SetHDRIF( SiiInst_t inst);
#endif //__SI_TEST_SET_INFOFRAMES_API_H__
