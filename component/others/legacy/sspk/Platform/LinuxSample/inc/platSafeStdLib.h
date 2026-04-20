///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////


// TODO: these need to be declared normally and implemented in Platform/Compatibility/SafeStdLib
//
//   Using sscanf for sscanf_s only works for format specifiers other than c, C, s, S and [
//   because sscanf_s requires an extra unsigned int param after each of those parameter types.
//
#define sscanf_s sscanf

#define memcpy_s( dptr, dsize, sptr, ssize ) memcpy( dptr, sptr, ((dsize)<(ssize)?(dsize):(ssize)) )

// NOTE: the following only works when results of fopen_s is not used!
#define fopen_s( pfile, filename, mode ) *(pfile) = fopen(filename, mode)

