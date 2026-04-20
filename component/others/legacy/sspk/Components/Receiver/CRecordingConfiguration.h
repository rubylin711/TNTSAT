///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// ===============================================================================================================
// Various buffer pools used for recordings (writer/reader/streamer)
// These are allocated on DVR box only
// ===============================================================================================================

//Number of buffers used in global pool shared by all ongoing recordings on DVR box
#define GLOBAL_WRITER_BUFFER_NUM    (14)
//Size of each buffer used in global pool shared by all ongoing recordings on DVR box
#define GLOBAL_WRITER_BUFFER_SIZE   (128*1024)

//Number of buffers used in global bool shared by playback of recordings on the DVR box
#define GLOBAL_READER_BUFFER_NUM    (1)
//Size of each buffer used in global bool shared by playback of recordings on the DVR box
#define GLOBAL_READER_BUFFER_SIZE   (128*1024)

//Number of buffers used in global bool shared for streaming recordings to the remote box
#define GLOBAL_STREAMER_BUFFER_NUM  (4)
//Size of each buffer used in global bool shared for streaming recordings to the remote box
#define GLOBAL_STREAMER_BUFFER_SIZE (128*1024)

// ===============================================================================================================
// ===============================================================================================================

//Pause buffer priority level
//Needs to match with app defined priority
#define PAUSE_BUFFER_PRIORITY       (1)

// ===============================================================================================================
// ===============================================================================================================
