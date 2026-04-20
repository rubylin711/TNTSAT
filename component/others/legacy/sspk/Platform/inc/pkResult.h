///////////////////////////////////////////////////////////////////////////////
//
//  Copyright (c) Microsoft Corporation.  All Rights Reserved.
//
//  This file is marked as "read only".
//
///////////////////////////////////////////////////////////////////////////////

#pragma once

// PAL result code definitions
// Macros for testing success or failure of a pkRESULT
#define pkSUCCEEDED(s)                              ((pkRESULT)(s) >= 0)
#define pkFAILED(s)                                 ((pkRESULT)(s) < 0)

#define MAKE_PKRESULT(sev,fac,code) ((pkRESULT)(((pkRESULT)sev<<31)|((pkRESULT)fac<<16)|((pkRESULT)code)))

//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +-+-+-+-+-+---------------------+-------------------------------+
//  |S|R|C|N|r|    Facility         |               Code            |
//  +-+-+-+-+-+---------------------+-------------------------------+
//
//  where
//
//      S - Severity - indicates success/fail
//
//          0 - Success
//          1 - Fail (COERROR)
//
//      R - reserved portion of the facility code, corresponds to NT's
//              second severity bit.
//
//      C - reserved portion of the facility code, corresponds to NT's
//              C field.
//
//      N - reserved portion of the facility code. Used to indicate a
//              mapped NT status value.
//
//      r - reserved portion of the facility code. Reserved for internal
//              use. Used to indicate HRESULT values that are not status
//              values, but are instead message ids for display strings.
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//

//
// Severity values
//

#define PK_SEVERITY_SUCCESS    0
#define PK_SEVERITY_ERROR      1

//
// Define the facility codes
//
#define PK_FAC_GENERAL       0x000
#define PK_FAC_WIN32         0x007
#define PK_FAC_MF            0x00D
#define PK_FAC_SSPK          0x100 //custom SSPK error codes

// Result code definitions

//
// MessageId: pkS_OK
//
// MessageText:
//
// The operation completed successfully.
//
#define pkS_OK                                      ((pkRESULT) 0x00000000L)

//
// MessageId: pkS_FALSE
//
// MessageText:
//
// Operation was successful, but returned a FALSE test condition.
//
#define pkS_FALSE                                   ((pkRESULT) 0x00000001L)

// ============================================================
//
// General errors (0x8000xxxx)
//
// ============================================================
//

//
// MessageId: pkE_UNEXPECTED
//
// MessageText:
//
// Catastrophic failure
//
#define pkE_UNEXPECTED                              ((pkRESULT) 0x8000FFFFL)

//
// MessageId: pkE_PENDING
//
// MessageText:
//
// The data necessary to complete this operation is not yet available.
//
#define pkE_PENDING                                 ((pkRESULT) 0x8000000AL)

//
// MessageId: pkE_NOTIMPL
//
// MessageText:
//
// Not implemented
//
#define pkE_NOTIMPL                                 ((pkRESULT) 0x80004001L)

//
// MessageId: pkE_NOINTERFACE
//
// MessageText:
//
// No such interface supported
//
#define pkE_NOINTERFACE                             ((pkRESULT) 0x80004002L)

//
// MessageId: pkE_POINTER
//
// MessageText:
//
// Invalid pointer
//
#define pkE_POINTER                                 ((pkRESULT) 0x80004003L)

//
// MessageId: pkE_ABORT
//
// MessageText:
//
// Operation aborted
//
#define pkE_ABORT                                   ((pkRESULT) 0x80004004L)

//
// MessageId: pkE_FAIL
//
// MessageText:
//
// Unspecified error
//
#define pkE_FAIL                                    ((pkRESULT) 0x80004005L)

//
// MessageId: pkE_INVALID_REQUEST
//
// MessageText:
//
// Invalid request
//
#define pkE_INVALID_REQUEST                         ((pkRESULT) 0x80004007L)

//
// MessageId: pkE_TIMEOUT
//
// MessageText:
//
// The wait operation timed out
//
#define pkE_TIMEOUT                                 ((pkRESULT) 0x800005B4L)

// ============================================================
//
// Win32 errors (0x8007xxxx)
//
// ============================================================
//

//
// MessageId: pkE_FILE_NOT_FOUND
//
// MessageText:
//
// The system cannot find the file specified
//
#define pkE_FILE_NOT_FOUND                          ((pkRESULT) 0x80070002L)

//
// MessageId: pkE_ACCESSDENIED
//
// MessageText:
//
// General access denied error
//
#define pkE_ACCESSDENIED                            ((pkRESULT) 0x80070005L)

//
// MessageId: pkE_HANDLE
//
// MessageText:
//
// Invalid handle
//
#define pkE_HANDLE                                  ((pkRESULT) 0x80070006L)

//
// MessageId: pkE_OUTOFMEMORY
//
// MessageText:
//
// Ran out of memory
//
#define pkE_OUTOFMEMORY                             ((pkRESULT) 0x8007000EL)

//
// MessageId: pkE_NOT_READY
//
// MessageText:
//
// Not ready to execute command
//
#define pkE_NOT_READY                               ((pkRESULT) 0x80070015L)

//
// MessageId: pkE_INVALIDARG
//
// MessageText:
//
// One or more arguments are invalid
//
#define pkE_INVALIDARG                              ((pkRESULT) 0x80070057L)

//
// MessageId: pkE_INSUFFICIENT_BUFFER
//
// MessageText:
//
// The data area passed to a system call is too small
//
#define pkE_INSUFFICIENT_BUFFER                     ((pkRESULT) 0x8007007AL)

//
// MessageId: pkE_NO_MORE_ITEMS
//
// MessageText:
//
// No more data is available
//
#define pkE_NO_MORE_ITEMS                           ((pkRESULT) 0x80070103L)

//
// MessageId: pkE_NOT_CONNECTED
//
// MessageText:
//
// This network connection does not exist.
//
#define pkE_NOT_CONNECTED                           ((pkRESULT) 0x800708CAL)

//
// MessageId: pkE_SOCKET_WOULDBLOCK
//
// MessageText:
//
// A non-blocking socket operation could not be completed immediately
//
#define pkE_SOCKET_WOULDBLOCK                       ((pkRESULT) 0x80072733L)

//
// MessageId: pkE_SOCKET_ADDRINUSE
//
// MessageText:
//
// Only one usage of each socket address (protocol/network address/port) is normally permitted
//
#define pkE_SOCKET_ADDRINUSE                        ((pkRESULT) 0x80072740L)

//
// MessageId: pkE_SOCKET_NOTSOCK
//
// MessageText:
//
// An operation was attempted on something that is not a socket
//
#define pkE_SOCKET_NOTSOCK                          ((pkRESULT) 0x80072736L)

//
// MessageId: pkE_SOCKET_SHUTDOWN
//
// MessageText:
//
// A request to send or receive data was disallowed because the socket had already been shut down
// in that direction with a previous shutdown call.
//
#define pkE_SOCKET_SHUTDOWN                         ((pkRESULT) 0x8007274AL)

// ============================================================
//
// MF errors (0xC00Dxxxx)
//
// ============================================================
//

//
// MessageId: pkE_INVALID_FORMAT
//
// MessageText:
//
// The Media format is recognized but is invalid
//
#define pkE_INVALID_FORMAT                          ((pkRESULT) 0xC00D3E8CL)

//
// MessageId: pkE_UNSUPPORTED_FORMAT
//
// MessageText:
//
// The Media format is recognized but not supported
//
#define pkE_UNSUPPORTED_FORMAT                      ((pkRESULT) 0xC00D3E98L)

// ============================================================
//
// SSPK specific errors (0x8100xxxx)
//
// ============================================================
//

//
// MessageId: pkE_SINGLE_INSTANCE_OP
//
// MessageText:
//
// Cannot run more than one instance of the specified operation.
// The current instance must complete or be aborted before a new instance
// can start.
//
#define pkE_SINGLE_INSTANCE_OP                      ((pkRESULT) 0x81000100L)

//
// MessageId: pkE_BEFORE_VALID_RANGE
//
// MessageText:
//
// The index or iterator refers to a position before the current
// valid range
//
#define pkE_BEFORE_VALID_RANGE                      ((pkRESULT) 0x81000101L)

//
// MessageId: pkE_INVALID_XML_SYNTAX
//
// MessageText:
//
// Invalid XML syntax
//
#define pkE_INVALID_XML_SYNTAX                      ((pkRESULT) 0x81000102L)

//
// MessageId: pkE_NO_MORE_ROOM
//
// MessageText:
//
// No more room to add item
//
#define pkE_NO_MORE_ROOM                            ((pkRESULT) 0x81000103L)

