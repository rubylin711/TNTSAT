///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

//Enumeration of times segments tracked during XDrm license acquisition
//TODO: Eventually this should fold into IDrmObject interface
enum
{
    XDrmLicenseTimes_Start = 0,            // [0] = Start of license acquisition
    XDrmLicenseTimes_GenerateChallenge,    // [1] = Between GenerateChallenge() -> SendHttp(GETLICENSE)
    XDrmLicenseTimes_GetLicense,        // [2] = Between SendHttp(GETLICENSE) -> ProcessResponse()
    XDrmLicenseTimes_ProcessResponse,    // [3] = Between ProcessResponse() -> GenerateLicenseAck() (or end, if no license ack)
    XDrmLicenseTimes_GenerateLicenseAck,// [4] = Between GenerateLicenseAck() -> SendHttp(GETLICACK) (= 0 if no license ack)
    XDrmLicenseTimes_GetLicAck,            // [5] = Between SendHttp(GETLICACK) -> ProcessLicenseAckResp() (= 0 if no license ack)
    XDrmLicenseTimes_End,                // [6] = End of license acquisition
    XDrmLicenseTimes_Count
};
