///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

#include "stdafx.h" // includes pkPAL.h
#include "MP4Info.h"
#include "mp4parserhelper.h"

#include <pkTestFramework.h>

MP4TrackType GetMP4TrackType(string fileType)
{
    MP4TrackType testMP4TrackType = kMP4TrackType_None;

    if (fileType.compare("video"))
    {
        testMP4TrackType = kMP4TrackType_Video;
    }

    if (fileType.compare("audio"))
    {
        testMP4TrackType = kMP4TrackType_Audio;
    }

    if (fileType.compare("subtitle"))
    {
        testMP4TrackType = kMP4TrackType_Text;
    }

    return testMP4TrackType;

}


void ParseVector(char * chunkData, tTestVector * testVector)
{
    int index = 0;
    char temp[16];

    while (index < VectorLength)
    {
        // Vector Number
        if (index == VectorNumberPosition)
        {
            temp[0] = (char)*(chunkData+index);
            temp[1] = (char)*(chunkData+index+1);
            temp[2] = '\0';

            testVector->VectorNumber = atoi(temp);
        }

        if (index == ChunkTypePosition)
        {
            temp[0] = (char)*(chunkData+index);
            temp[1] = (char)*(chunkData+index+1);
            temp[2] = '\0';
            testVector->ChunkType = atoi(temp);
        }

        if (index == EncryptedPosition)
        {
            temp[0] = (char)*(chunkData+index);
            temp[1] = (char)*(chunkData+index+1);
            temp[2] = '\0';
            testVector->Encrypted = atoi(temp);
        }

        if (index == FileSizePostion)
        {
            temp[0] = (char)*(chunkData+index);
            temp[1] = (char)*(chunkData+index+1);
            temp[2] = (char)*(chunkData+index+2);
            temp[3] = (char)*(chunkData+index+3);
            temp[4] = '\0';

            testVector->FileSize = atoi(temp);
        }

        index++;
    }

}

// TODO: Need to move it to helpers
pkRESULT ParseScript(pkHANDLE hLog, char * pScriptBuf, uint8_t * TestVector)
{
    int i = 0;
    int j = 0;
    pkRESULT result = pkS_OK;

    char * testMethod = NULL;
    char * fileType = NULL;
    char * fileChunk = NULL;
    char * chunkData = NULL;

    char ScriptBuffer[2048];
    int index = 0;

    while (*(pScriptBuf+index) != 0)
    {
        ScriptBuffer[index] = *(pScriptBuf+index);
        index++;
    }
    ScriptBuffer[index] = '\0';

    fileChunk = (char *)malloc(VectorFilenameLength);
    chunkData = (char *)malloc(VectorLength);

    // TODO: need to replace static memory allocation
    int numberArguments = 0;
    char arg[VectorFilenameLength];

    while (ScriptBuffer[i] != 0)
    {
        arg[j] = ScriptBuffer[i];

        if ((ScriptBuffer[i] == 0x20) || (ScriptBuffer[i] == 10))
        {
            arg[j] = '\0';

            switch (numberArguments)
            {
            case 0:
                testMethod = new char[j];
                strncpy(testMethod, arg, j+1);

                break;
            case 1:
                fileType = new char[j];
                strncpy(fileType, arg, j+1);
                break;
            case 2:
                fileChunk = new char[j];
                strncpy(fileChunk, arg, j+1);
                break;
            default:
                break;
            }

            j = -1;
            numberArguments++;
        }
        i++;
        j++;
    }

    if (TestVector == NULL)
    {
        // Launch Test
        (void)LaunchMP4Parser(hLog, fileType, fileChunk, chunkData);
    }
    else
    {
        // Copy the Chunk File to a string
        for (i = 0; i < VectorFilenameLength; i++)
        {
            arg[i] = TestVector[i];

            if ((arg[i] == 'd') && (arg[i-1] == 'i') && (arg[i] == 'v'))
            {
                arg[i+1] = '\0';
            }
        }
        strncpy(fileChunk, arg, VectorFilenameLength);

        for (i = VectorLength; i < VectorSize; i++)
        {
            arg[i-VectorLength] = TestVector[i];
        }

        strncpy(chunkData, arg, VectorLength);

        (void)LaunchMP4Parser(hLog, fileType, fileChunk, chunkData);
    }


    free(fileChunk);
    free(chunkData);

    return result;
}
