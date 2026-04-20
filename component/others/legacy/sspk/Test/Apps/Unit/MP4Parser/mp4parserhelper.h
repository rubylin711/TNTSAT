///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "MP4Info.h"

// Constants
const int VectorSize = 512;
const int VectorFilenameLength = 256;
const int VectorLength = 256;

// position of individual components in the vector
const int VectorNumberPosition = 0;
const int ChunkTypePosition = 3;
const int EncryptedPosition = 6;
const int FileSizePostion = 9;


// Data Structures
struct tTestVector
{
    int VectorNumber;
    // Chunk Type   01 Video 02 Audio 04 Subtitle 05 Invalid
    int ChunkType;
    // Encrypted    00 Unencrypted 01 Encrypted
    int Encrypted;
    // File Size    AB CD    => ABCD k};
    int FileSize;
};

// Function Prototypes
MP4TrackType GetMP4TrackType(string fileType);

void ParseVector(char * chunkData, tTestVector * testVector);

pkRESULT ParseScript(pkHANDLE hLog, char * pScriptBuf, uint8_t * TestVector);

bool LaunchMP4Parser(pkHANDLE hLog, char * fileType, char * chunkFile, char * chunkData);


