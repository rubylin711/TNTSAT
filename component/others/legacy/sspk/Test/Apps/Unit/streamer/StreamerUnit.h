///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

// TODO: need to replace static memory allocation and global variables
char * s_szLogFileURI;
char* s_szVectorFileURI;
char* s_szScriptFileURI;

char s_ScriptBuffer[2048];
char * pScriptBuffer = &s_ScriptBuffer[0];

bool_t g_IsAbandoned = FALSE;
bool_t isVerboseHelpRequested = FALSE;
bool_t t_isScriptAvailable = FALSE;
bool_t t_AreVectorsAvailable = FALSE;

// TODO: Logging has to done per test or for an entire script. Create an enum to track this
bool t_IsLoggingEnabled = false;


