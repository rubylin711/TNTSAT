///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "sample".
//
///////////////////////////////////////////////////////////////////////////////

//===========================================================================
// Includes
//===========================================================================
#include <pkPAL.h>
#include <string.h>

#include <pkTestFramework.h>
#include <TLCommon.h>

// Override the ERRTRACE print method
#define ERRTRACE_PRINTMSG(printf_exp)   TF_Printf printf_exp
#include <errTrace.h>

//===========================================================================
// Static variables
//===========================================================================
static int g_argc = 0;
static char **g_argv = NULL;

//===========================================================================
// Basic functions
//===========================================================================
//
// Function: TLC_SetCmdLineArgs
//
// Purpose: Store/init the command-line args so that subsequent calls to
//          parse the command-line args may succeed.
//
// Arguments:
//   argc [in] - Count of command-line arguments, including the name of
//               the executable. Identical to C-language main(argc).
//   argv [in] - Array of char strings, each representing a cmd-line
//               argument. Identical to C-language main(argv). This includes
//               the assumption that the char strings will be available during
//               the entire execution session.
//
// Returns:
//   Nothing.
//
void TLC_SetCmdLineArgs(int argc, char **argv)
{
    // Confirm that the argc, argv passed in matches C-language main defn
    pkASSERT(argc >= 1);
    pkASSERT(argv != NULL);
    pkASSERT(argv[argc] == NULL);

    g_argc = argc;
    g_argv = argv;
}


//
// Function: TLC_ClearCmdLineArgs
//
// Purpose: Null out state info so that subsequent calls to parse cmd-line args
//          will fail. This helps app developers to prohibit initialization
//          after a certain point in the program flow.
//
// Arguments: None.
//
// Returns:
//   Nothing.
//
void TLC_ClearCmdLineArgs(void)
{
    g_argc = 0;
    g_argv = NULL;
}


//
// Function: TLC_CountCmdLineArgs
//
// Purpose: Return a count of the cmd-line args, including program name.
//
// Arguments:
//   pargc [out] - returns count of arguments, including program name.
//
// Returns:
//   pkRESULT indicating success or failure.
//
pkRESULT TLC_CountCmdLineArgs(int *pargc)
{
    pkRESULT pkRes = pkS_OK;

    if (0 == g_argc || NULL == g_argv)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    *pargc = g_argc;

exit:
    return pkRes;
}



//
// Function: TLC_GetCmdLineArg
//
// Purpose: Returns the command-line argument for the given switch.
//
// Arguments:
//   pszSwitch [in] - Case-sensitive string, including dash prefix, which describes
//                    the switch we are looking for (eg. "-input"). The full string
//                    must be matched, abbreviations are not considered matches.
//   iPos [in] - relative position of the argument you would like returned. For
//               example, given cmd line "-input one two three", iPos=1 returns
//               "one", while iPos=3 returns "three". iPos=0 returns *ppArg = NULL
//               and is used to check existence of a switch, with no args.
//   ppArg [out, optional] - Pointer to string containing the requested cmd-line argument.
//
// Returns:
//   pkS_OK means the command-line argument argument was found and returned.
//   pkS_FALSE means the command-line argument was not found.
//   Otherwise, error code indicating fatal error.
//
pkRESULT TLC_GetCmdLineArg(const char *pszSwitch, int iPos, char **ppArg)
{
    pkRESULT pkRes = pkS_OK;
    BOOL fFoundSwitch = FALSE;
    int iSwitch;
    int i;

    if (ppArg)
        *ppArg = NULL;

    if (0 == g_argc || NULL == g_argv)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    if (NULL == pszSwitch)
    {
        pkRes = TraceResult(pkE_INVALIDARG);
        goto exit;
    }

    // First, see if the requested switch exists
    for (iSwitch = 0; iSwitch < g_argc; iSwitch++)
    {
        if (0 == strcmp(g_argv[iSwitch], pszSwitch))
        {
            fFoundSwitch = TRUE;
            break;
        }
    }

    if (FALSE == fFoundSwitch)
    {
        pkRes = pkS_FALSE;
        goto exit;
    }

    // Check if this is a no-argument switch
    if (0 == iPos)
    {
        pkRes = pkS_OK;
        goto exit;
    }

    // Skip the requested number of arguments
    // Does the skip request go past end of command line?
    if (iSwitch + iPos >= g_argc)
    {
        pkRes = pkS_FALSE;
        goto exit;
    }

    // Nope, not past end of command line. Check if another switch starts before we
    // reach the requested position
    for (i = 1; i <= iPos; i++)
    {
        if (g_argv[iSwitch + i][0] == '-')
        {
            // We encountered a command-line switch before reaching the requested pos
            pkRes = pkS_FALSE;
            goto exit;
        }
    }

    // If we reached this point, we have found the requested switch (eg. "-input")
    // and the requested cmd-line argument position (eg. iPos = 3) without finding
    // any new switches in between. Return the argument.
    if (ppArg)
        *ppArg = g_argv[iSwitch + iPos];

    pkRes = pkS_OK;

exit:
    return pkRes;
}



//
// Function: TLC_GetCmdLineArgInt
//
// Purpose: Returns the command-line argument for the given switch, as an integer.
//
// Arguments:
//   pszSwitch [in] - Case-sensitive string, including dash prefix, which describes
//                    the switch we are looking for (eg. "-input"). The full string
//                    must be matched, abbreviations are not considered matches.
//   iPos [in] - relative position of the argument you would like returned. For
//               example, given cmd line "-input one two three", iPos=1 returns
//               "one", while iPos=3 returns "three". iPos=0 returns *ppArg = NULL
//               and is used to check existence of a switch, with no args.
//   ppArg [out, optional] - Pointer to int which will contain the requested cmd-line argument.
//
// Returns:
//   pkS_OK means the command-line argument argument was found and returned.
//   pkS_FALSE means the command-line argument was not found.
//   Otherwise, error code indicating fatal error.
//
pkRESULT TLC_GetCmdLineArgInt(const char *pszSwitch, int iPos, int *piArg)
{
    pkRESULT pkRes = pkS_OK;
    char *pszArg;

    pkRes = TLC_GetCmdLineArg(pszSwitch, iPos, &pszArg);
    if (piArg)
    {
        if (pkS_OK == pkRes)
            *piArg = atoi(pszArg);
        else
            *piArg = 0;
    }

    return pkRes;
}



//
// Function: TLC_IsHelpRequested
//
// Purpose: Indicates whether user has requested help or verbose help.
//
// Arguments:
//   pfVerbose [out, optional] - If non-NULL, sets to TRUE if user requests verbose help
//
// Returns:
//   TRUE if caller should print help (basic/verbose determined by *pfVerbose) and exit.
//   FALSE means no help requested, or error occurred (in which case - no help requested).
//
bool_t TLC_IsHelpRequested(bool_t *pfVerbose)
{
    pkRESULT pkRes; // We don't actually return this
    BOOL fRetval = FALSE;

    if (pfVerbose)
        *pfVerbose = FALSE;

    if (0 == g_argc || NULL == g_argv)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    if (pkS_OK == TLC_GetCmdLineArg("--help", 0, NULL))
    {
        fRetval = TRUE;
        if (pfVerbose)
            *pfVerbose = TRUE;
    }
    else if (1 == g_argc ||
        pkS_OK == TLC_GetCmdLineArg("-help", 0, NULL) ||
        pkS_OK == TLC_GetCmdLineArg("-h", 0, NULL) ||
        pkS_OK == TLC_GetCmdLineArg("-?", 0, NULL))
    {
        fRetval = TRUE;
    }

exit:
    return fRetval;
}



//
// Function: TLC_CmdLine_IsCPUUtilizationRequested
//
// Purpose: Indicates whether user has requested that the application measure
//   CPU utilization.
//
// Arguments: None.
//
// Returns:
//   TRUE if caller should measure CPU utilization.
//   FALSE if caller should not.
//
bool_t TLC_CmdLine_IsCPUUtilizationRequested(void)
{
    pkRESULT pkRes; // We don't actually return this
    BOOL fRetval = FALSE;

    if (0 == g_argc || NULL == g_argv)
    {
        pkRes = TraceResult(pkE_UNEXPECTED);
        goto exit;
    }

    if (pkS_OK == TLC_GetCmdLineArg("-proc", 0, NULL))
        fRetval = TRUE;

exit:
    return fRetval;
}



//===========================================================================
// Video and Graphics command line parsing
//===========================================================================
//
// Function: TLC_GetCmdLineVideoGraphicsConfig
//
// Purpose: Parses standard video output and graphics size command line parameters
//
// Arguments:
//   pConfig - pointer to TF_Config structure to receive parameters
//
// Returns:
//   pkS_OK - zero or more parameters were scanned without any errors
//   pkE_INVALIDARG - invalid value following a tag
//
pkRESULT TLC_GetCmdLineVideoGraphicsConfig(TF_Config *pConfig)
{
    char *pszArg;
    int intArg;
    pkRESULT hr = pkS_OK;

    if (pkS_OK == TLC_GetCmdLineArg("-vo", 1, &pszArg))
    {
        if (0 == strcmp(pszArg, "480i")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_NTSC_M;
            pConfig->graphicsWidth  = 640;
            pConfig->graphicsHeight = 480;
        }
        else if (0 == strcmp(pszArg, "576i")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_PAL_BG;
            pConfig->graphicsWidth  = 1024;
            pConfig->graphicsHeight = 576;
        }
        else if (0 == strcmp(pszArg, "480p")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_480p59;
            pConfig->graphicsWidth  = 640;
            pConfig->graphicsHeight = 480;
        }
        else if (0 == strcmp(pszArg, "720p")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_720p59;
            pConfig->graphicsWidth  = 1024;
            pConfig->graphicsHeight = 576;
        }
        else if (0 == strcmp(pszArg, "720p60")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_720p60;
            pConfig->graphicsWidth  = 1024;
            pConfig->graphicsHeight = 576;
        }
        else if (0 == strcmp(pszArg, "1080i")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_1080i59;
            pConfig->graphicsWidth  = 1280;
            pConfig->graphicsHeight = 720;
        }
        else if (0 == strcmp(pszArg, "1080i60")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_1080i60;
            pConfig->graphicsWidth  = 1280;
            pConfig->graphicsHeight = 720;
        }
        else if (0 == strcmp(pszArg, "1080p")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_1080i59;
            pConfig->graphicsWidth  = 1280;
            pConfig->graphicsHeight = 720;
        }
        else if (0 == strcmp(pszArg, "1080p60")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_1080i60;
            pConfig->graphicsWidth  = 1280;
            pConfig->graphicsHeight = 720;
        }
        else if (0 == strcmp(pszArg, "HDMI_480p")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_HDMI_480p;
            pConfig->graphicsWidth  = 640;
            pConfig->graphicsHeight = 480;
        }
        else if (0 == strcmp(pszArg, "HDMI_720p")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_HDMI_720p;
            pConfig->graphicsWidth  = 1024;
            pConfig->graphicsHeight = 576;
        }
        else if (0 == strcmp(pszArg, "HDMI_1080i")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_HDMI_1080i;
            pConfig->graphicsWidth  = 1280;
            pConfig->graphicsHeight = 720;
        }
        else if (0 == strcmp(pszArg, "HDMI_1080p")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_HDMI_1080p;
            pConfig->graphicsWidth  = 1280;
            pConfig->graphicsHeight = 720;
        }
        else if (0 == strcmp(pszArg, "HDMI")) {
            pConfig->eVideoOutStandard = TF_E_VIDEO_OUT_HDMI_AUTO;
            pConfig->graphicsWidth  = 1280;
            pConfig->graphicsHeight = 720;
        }
        else {
            hr = pkE_INVALIDARG;
            goto bail;
        }
    }

    if (pkS_OK == TLC_GetCmdLineArgInt("-gw", 1, &intArg))
    {
        if (intArg < 480 || intArg > 1920)
        {
            hr = pkE_INVALIDARG;
            goto bail;
        }
        pConfig->graphicsWidth = (uint32_t)intArg;
    }

    if (pkS_OK == TLC_GetCmdLineArgInt("-gh", 1, &intArg))
    {
        if (intArg < 320 || intArg > 1080)
        {
            hr = pkE_INVALIDARG;
            goto bail;
        }
        pConfig->graphicsHeight = (uint32_t)intArg;
    }

bail:
    return hr;
}

//
// Function: TLC_GetCmdLineVideoGraphicsConfigUsage
//
// Purpose: Return usage string for TLC_GetCmdLineVideoGraphicsConfig.
//
// Arguments:
//   none
//
// Returns:
//   const pointer to multi-new-line string.
//
const char* TLC_GetCmdLineVideoGraphicsConfigUsage(void)
{
    return (
        "  -vo SPEC    sets the video output format.\n"
        "              Valid arguments are:\n"
        "                480i, 576i, 480p, 720p, 720p60,\n"
        "                1080i, 1080i60, 1080p, 1080p60,\n"
        "                HDMI_480, HDMI_720, HDMI_1080i,\n"
        "                HDMI_1080 and HDMI (for auto select).\n"
        "\n"
        "  -gw WIDTH   sets the graphics overlay width (480..1920).\n"
        "\n"
        "  -gh HEIGHT  sets the graphics overlay height (320..1080).\n"
        "\n"
        "  -n COUNT    sets the number of test iterations.\n"
        "\n" );
}

