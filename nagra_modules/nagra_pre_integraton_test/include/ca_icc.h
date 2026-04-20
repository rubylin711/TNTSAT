/**
 *  @defgroup ICC Integrated Circuit Card
 *  @brief
 *    Integrated Circuit Card driver component
 *
 *    Smartcard driver, supporting multiple smartcards at the same time.
*/
/**
**  @file ca_icc.h
**
**  @brief
**    This document defines the Smartcard Driver Application Programming
**    Interface seen by the Nagravision Conditional Access Library.
**    This API is designed to support the protocol T=1, T=1 raw and T=0.
**
**  @ingroup  ICC
**
**  @author Bernard Krummenacher
**
**  COPYRIGHT:
**   © 2003 Nagravision S.A.
**
**  HISTORY:
**   $Log: ca_icc.h,v $
**   Revision 1.20  2011/07/14 10:28:15  moriceau
**   PRMS ID: NONE, MERLIN-692, ALAD-743, CCUBE-268, ELK-33, CLME-37, TIGER-694
**   - Revert last modif for this file
**
**   Revision 1.19  2011/06/16 09:18:47  moriceau
**   PRMS ID: NONE, MERLIN-692, ALAD-743, CCUBE-268, ELK-33, CLME-37, TIGER-694
**   - [Rework] ca_cak.h inclusion in all CAK object file (automatic ca_cak.h generation will not be functional anymore)
**
**   Revision 1.18  2008/03/04 07:20:30  bussy
**   PRMS ID: NONE
**   Added a note on iccRegister and iccCancelRegistration to activate and deactivate the electrical circuit.
**
**   Revision 1.17  2007/06/27 10:47:57  bussy
**   PRMS ID: NONE
**   - Reworked iccT0Send and iccT0Receive
**   - Removed verification of ATR checksum
**   - Detailed when a mute or timeout status is returned
**
**   Revision 1.16  2006/11/08 17:44:11  hacheman
**   PRMS ID: NONE
**   - Including iccwrap.h if a wrapper is defined.
**
**   Revision 1.15  2006/07/18 07:37:33  bussy
**   PRMS ID: NONE
**   Merged KDA branch
**
**   Revision 1.14.4.2  2005/12/12 07:34:46  hacheman
**   PRMS ID: 30461
**   - Corrected the name of the parameters of iccT0Exchange().
**
**   Revision 1.14.4.1  2005/12/02 16:15:22  hacheman
**   PRMS ID: 30461
**   - Added function iccT0Exchange().
**
**   Revision 1.14  2004/03/08 17:15:25  hacheman
**   PRMS ID: 22239
**   - Added optional inclusion of ICC wrapper to debug layer (iccwrpsh.h).
**
**   Revision 1.13  2003/07/24 07:43:24  hacheman
**   PRMS ID: NONE
**   - Back to the revision 1.11.
**
**   Revision 1.11  2003/01/31 14:47:43  mueller
**   PRMS ID: NONE
**   - Changed copyright and some comments for consistency with StbIccApi.doc
**
**   Revision 1.10  2003/01/29 09:59:44  mueller
**   PRMS ID: NONE
**   Added const qualificatif to pxAtr argument of the event notification
**   callback.
**
**   Revision 1.9  2002/09/05 15:12:44  robyr
**   PRMS ID: NONE
**   - Moved function iccT1ApduExchange() to new header file ca_icct1.h
**
**   Revision 1.8  2002/03/15 16:43:00  schyrr
**   PRMS ID: NONE
**   - Prevent definitions of ca_cak.h-intended definitions in case CA_CAK_h is
**     defined.
**
**   Revision 1.7  2001/12/04 14:50:30  erard
**   PRMS ID: NONE
**   - Added back TIccSessionId.
**   - Changed bakc type of TIccAtr to TUnsignedInt8*.
**   - Changed the prototypes for the above change.
**
**   Revision 1.6  2001/11/20 11:20:20  erard
**   PRMS ID: NONE
**   - updated the file to be compliant with the Smartcard Driver API described
**     by 'StbIccApi.doc' version 2.0.3.
**   - adapted comments of the file do be compliant with doxsygen.
**
**   Revision 1.5  2001/01/31 10:33:07  krummen
**   PRMS ID: NONE
**   Added IccModeChange() function prototype.
**
**   Revision 1.4  2000/11/07 15:40:46  krummen
**   PRMS ID: NONE
**   Split iccT0Exchange() in iccT0Send() and iccT0Receive.
**
**   Revision 1.3  2000/10/13 12:28:23  krummen
**   PRMS ID: NONE
**   Added definition of TIccRegistrationId type.
**
**   Revision 1.2  2000/09/14 09:50:11  krummen
**   PRMS ID: NONE
**   The interface is now completely defined.
**
**   Revision 1.1  2000/09/13 10:35:45  krummen
**   PRMS ID: NONE
**   First issue.
**
*/
#ifndef CA_ICC_H
#define CA_ICC_H

/******************************************************************************/
/*                                                                            */
/*                               INCLUDE FILES                                */
/*                                                                            */
/******************************************************************************/

#include "ca_defs.h"

#if defined ICC_DEBUG_SHELL || defined ICC_WRAP
#include "iccwrap.h"
#endif /* ICC_DEBUG_SHELL or ICC_WRAP */

/******************************************************************************/
/*                                                                            */
/*                             CONSTANTS & TYPES                              */
/*                                                                            */
/******************************************************************************/


/******************************************************************************/
/* The following comment triggers the automatic exportation of the following  */
/* definitions to ca_cak.h. Do not modify it.                                 */
#ifndef CA_CAK_H
/* CAK_TYPEDEF_EXPORT_ON */

/**
 *  @brief
 *    Smartcard session identifier.
*/
typedef TUnsignedInt32  TIccSessionId;

/**
 *  @brief
 *    Special ID to specify no smartcard.
*/
#define ICC_SESSION_ID_NONE ((TIccSessionId)-1)

/* CAK_TYPEDEF_EXPORT_OFF */
#endif /* CA_CAK_H */
/* The preceding comment stops the automatic exportation of the preceding     */
/* definitions to ca_cak.h. Do not modify it.                                 */
/******************************************************************************/


/**
 *  @brief
 *    Smartcard registration identifier.
 *
 *    Each registered client has is own registration identifier.
*/
typedef TUnsignedInt32 TIccRegistrationId;


/**
 *  @brief
 *    Smartcard answer to reset buffer.
 *
 *  @see ICC_ATR_MAX_LEN
*/

typedef TUnsignedInt8*     TIccAtr;


/**
 *  @brief
 *    Maximum length of the ICC ATR.
 *
 *  @see TIccAtr
*/
#define ICC_ATR_MAX_LEN   33


/**
 *  @brief
 *    Smartcard clock frequency. Clock frequency in hertz send to the smartcard.
*/
typedef TUnsignedInt32    TIccClockFrequency;


/**
 *  @brief
 *    This enumeration define the smartcard access modes.
*/
typedef enum
{
  ICC_ACCESS_NONE,
    /**< No smartcard access.
    */
  ICC_ACCESS_EXCLUSIVE,
    /**< Exclusively smartcard access.
    */
  ICC_ACCESS_SHARED
    /**< Shared smartcard access.
    */
} TIccAccessMode;

/**
 *  @brief
 *    Smartcard notification event type. At each notification,
 *    the ICC driver provides one of these events, depending of
 *    the smartcard change.
*/
typedef enum
{
  ICC_EVENT_NO_AVAILABLE_CARD,
    /**< Special event, sent only just after the client registration
     *   if there is no available smartcard at this time.
    */
  ICC_EVENT_CARD_INSERTED,
    /**< A new smartcard has been inserted in the device. It is now
     *   presented to the client for use.
    */
  ICC_EVENT_CARD_INSERTED_SINGLE_CLIENT_SUPPORT,
    /**< A new smartcard has been inserted in the device. The driver
     *   is designed only to support one client at a time.
    */
  ICC_EVENT_CARD_SHARED,
    /**< A new smartcard has been inserted in the device. Another client
     *   has already claimed it but in shared mode. The next clients may
     *   only accept it in shared mode too.
    */
  ICC_EVENT_CARD_REMOVED,
    /**< A smartcard was removed from the device.
    */
  ICC_EVENT_CARD_MUTE
    /**< A new smartcard has been inserted in the device. However,
     *   it does not send its ATR.
    */
} TIccEventType;


/**
 *  @brief
 *    General error statuses returned by the smartcard functions.
*/
typedef enum
{
  ICC_NO_ERROR,
     /**< Success.
     */
  ICC_ERROR_SESSION_ID,
     /**< The session identifier is wrong.
     */
  ICC_ERROR_CARD_REMOVED,
     /**< There is no smartcard to communicate with.
     */
  ICC_ERROR_CARD_MUTE,
     /**< The smartcard does not send any data. May be upside down.
     */
  ICC_ERROR_TIMEOUT,
     /**< The smartcard didn't answer in the required time.
     */
  ICC_ERROR_MODE,
     /**< The operation requested cannot be performed due to a mode
      *   constraint.
     */
  ICC_ERROR_CONFLICT,
     /**< The smartcard mode cannot be changed, as it would lead to
      *   a mode conflict.
     */
  ICC_ERROR
     /**< Other error.
     */
} TIccStatus;


/**
 *  @brief
 *    Format of the 5-byte T=0 command header (CLA, INS, P1, P2, P3).
*/
typedef TUnsignedInt8 TIccT0Header[5];


/**
 *  @brief
 *    Format of the 2-byte array carrying the two statuses SW1 and SW2 returned
 *    by a T=0 command. SW1 is located in the first byte of the array and SW2
 *    in the second byte.
*/
typedef TUnsignedInt8 TIccT0StatusWords[2];


/**
 *  @brief
 *    The decoder calls this function whenever the smartcard is inserted
 *    into or removed from the reader. It informs the CA of the new
 *    smartcard state.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xEventType
 *             Type of event.
 *  @param   xSessionId
 *             Identifier of the concerned smartcard session.
 *  @param   pxAtr
 *             Answer to reset data buffer. The driver must preserve this
 *             buffer until smartcard extraction notification or mode change
 *             to ICC_ACCESS_NONE. The application may not modify this buffer.
 *  @param   xClockFrequency
 *             Frequency of the clock signal applied to the smartcard in Hz.
 *
 *  @retval  ICC_ACCESS_NONE
 *             The CA will not use this smartcard.
 *  @retval  ICC_ACCESS_EXCLUSIVE
 *             The CA accepts the smartcard for its exclusive usage.
 *  @retval  ICC_ACCESS_SHARED
 *             The CA accepts the smartcard and is ready to share it with
 *             other applications.
 *
*/
typedef TIccAccessMode (*TIccEventNotification)
(
        TIccEventType         xEventType,
        TIccSessionId         xSessionId,
  const TIccAtr              pxAtr,
        TIccClockFrequency    xClockFrequency
);

/******************************************************************************/
/*                                                                            */
/*                            FUNCTIONS PROTOTYPE                             */
/*                                                                            */
/******************************************************************************/

/**
 *  @brief
 *    This function allows the CA to register a notification callback function
 *    in order to be informed that a smartcard is inserted to or removed
 *    from the reader.
 *
 *    This notification must be called as soon as the state of the smartcard
 *    changes. After the registration the smartcard driver must asynchronously
 *    call the notification function in order to inform the CA of the initial
 *    state of the card.
 *    If no smartcard is available at registration time, the application is
 *    notified as soon as it is registered with a dedicated notification.
 *    The registration is performed at the start of the CA. As the CA is able
 *    to start and terminate at will, the registration call may happen at
 *    any time.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param    xIccEventNotification
 *             Notification callback function.
 *  @param    pxRegistrationId
 *             Identifier for this specific registration.
 *             It is required to cancel the registration.
 *
 *  @retval   ICC_NO_ERROR
 *             Notification callback has been successfully registred.
 *  @retval   ICC_ERROR
 *             A general error occured. The notification callback could not
 *             be registred.
 *
 *  @remarks
 *    -# This function shall perform an activation of the electrical circuits
 *       according to ISO/IEC 7816-3.
*/
TIccStatus iccRegister
(
  TIccEventNotification   xIccEventNotification,
  TIccRegistrationId*     pxRegistrationId
);


/**
 *  @brief
 *    This function allows the CA to cancel its registration in order to inform
 *    the driver that it will not use the smartcard driver any more.
 *
 *
 *    The registration cancellation is performed just before the CA
 *    termination.  As the CA is able to start and terminate at will, this
 *    function call may happen at any time.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xRegistrationId
 *              Identifier of the registration to cancel.
 *              It is provided by the registration function.
 *
 *  @retval  ICC_NO_ERROR
 *              Registration has been successfully canceled.
 *  @retval  ICC_ERROR
 *              A general error occured.
 *
 *  @remarks
 *    -# This function shall perform a deactivation of the electrical circuits
 *       according to ISO/IEC 7816-3.
*/
TIccStatus iccCancelRegistration
(
  TIccRegistrationId    xRegistrationId
);

/**
 *  @brief
 *    This function is responsible of sending a T=1 data block to the smartcard
 *    and retrieving the related reply data block from the smartcard.
 *
 *    The function stops receiving bytes when the xReplyMaxLen value is reached
 *    or when no character has been received for more than CharacterWaitingTime.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface session, as given in the
 *             event notification callback.
 *  @param   xSendLen
 *             Length of send sequence.
 *  @param   pxSendBlock
 *             Pointer to the buffer containing the send sequence to be sent
 *             to the smartcard or NULL if only receive transfer.
 *  @param   xReplyMaxLen
 *             Maximum length of the reply sequence or 0 if only send
 *             transfer.
 *  @param   pxReplyLen
 *             Length of the reply block.
 *  @param   pxReplyBlock
 *             Pointer to the buffer where the smartcard reply block will
 *             be stored.
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session ID doesn't exist.
 *  @retval   ICC_ERROR_CARD_REMOVED
 *              The smartcard in not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *  @retval  ICC_ERROR_TIMEOUT
 *             The data exchange stopped due to excessive time between
 *             successive characters.
 *  @retval  ICC_ERROR
 *             The data exchange has failed due to communication errors.
 *
 *  @remarks
 *    -# This function is synchronous. Data pointed to by pxReplyBlock and
 *       pxReplyLen must be valid as soon as the CA returns from this function.
 *    -# This function shall return the error status ICC_ERROR_CARD_MUTE if no
 *       character is returned by the smart card within the block waiting time
 *       after sending the last byte of the command to the smart card.
 *    -# This function shall stop receiving bytes from the smart card when the
 *       xReplyMaxLen value is reached or if a byte is not received within the
 *       character waiting time. In this latter case, the function shall return
 *       the error status ICC_ERROR_TIMEOUT.
*/
TIccStatus iccT1RawExchange
(
        TIccSessionId     xSessionId,
        TSize             xSendLen,
  const TUnsignedInt8*    pxSendBlock,
        TSize             xReplyMaxLen,
        TSize*            pxReplyLen,
        TUnsignedInt8*    pxReplyBlock
);


/**
 *  @brief
 *    This function is responsible of handling a T=0 smart card incoming
 *    command. When processing such a command, the ICC driver is in charge
 *    of handling T=0 procedure bytes.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface session, as given in
 *             the event notification callback.
 *  @param   pxHeader
 *             5-byte T=0 command header (CLA, INS, P1, P2, P3) to be sent to
 *             the smart card.
 *  @param   xDataLen
 *             Length in bytes of the data to be sent to the smart card.
 *  @param   pxData
 *             Buffer containg the data to be sent to the smart card.
 *  @param   pxStatusWords
 *             2-byte array containing the two statuses SW1 and SW2 returned
 *             by the smart card upon completion of a T=0 command. SW1
 *             corresponds to *pxStatusWords[0] and SW2 to *pxStatusWords[1].
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session ID doesn't exist.
 *  @retval   ICC_ERROR_CARD_REMOVED
 *              The smartcard in not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *  @retval  ICC_ERROR_TIMEOUT
 *             The data exchange stopped due to excessive time between
 *             successive characters.
 *  @retval  ICC_ERROR
 *             Any other error.
 *
 *  @remarks
 *    -# This function is synchronous. If the data exchange is successful,
 *       data pointed to by pxStatusWords must be valid as soon as the
 *       function returns to the caller.
 *    -# This function shall return the status ICC_ERROR_CARD_MUTE if no
 *       character is received from the smart card within the work waiting
 *       time after sending the last byte of the header.
 *    -# The function shall stop receiving bytes from the smart card as soon
 *       as it has received the two statuses SW1 and SW2 or if a byte is not
 *       received within the work waiting time. In this latter case, the
 *       function shall return the error status ICC_ERROR_TIMEOUT.
 *    -# After receiving the command header, the smart card may directly return
 *       SW1 and SW2 to report an error (SW1 is considered as a procedure byte
 *       and replaces the ACK byte).
*/
TIccStatus iccT0Send
(
        TIccSessionId      xSessionId,
  const TIccT0Header      pxHeader,
        TSize              xDataLen,
  const TUnsignedInt8*    pxData,
        TIccT0StatusWords pxStatusWords
);



/**
 *  @brief
 *    This function is responsible of handling a T=0 smart card outgoing
 *    command. When processing such a command, the ICC driver is in charge
 *    of handling T=0 procedure bytes. 
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface to use.
 *  @param   pxHeader
 *             5-byte T=0 command header (CLA, INS, P1, P2, P3) to be sent to
 *             the smart card.
 *  @param   xDataExpectedLen
 *             Expected number of data bytes that should be returned by the
 *             smart card (SW1 and SW2 not included).
 *  @param   pxDataLen
 *             Number of data bytes actually returned by the smart card.
 *  @param   pxData
 *             Buffer containing the data bytes returned by the smart card.
 *  @param   pxStatusWords
 *             2-byte array containing the two statuses SW1 and SW2 returned
 *             by the smart card upon completion of a T=0 command. SW1
 *             corresponds to *pxStatusWords[0] and SW2 to *pxStatusWords[1].
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_CARD_REMOVED
 *              The smartcard in not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *  @retval  ICC_ERROR_TIMEOUT
 *             The data exchange stopped due to excessive time between
 *             successive characters.
 *  @retval  ICC_ERROR
 *             Any other error.
 *
 *  @remarks
 *    -# This function is synchronous. If the data exchange is successful,
 *       data pointed to by pxDataLen, pxData and pxStatusWords must be valid
 *       as soon as the function returns to the caller.
 *    -# This function shall return the status ICC_ERROR_CARD_MUTE if no
 *       character is received from the smart card for more than the work
 *       waiting time after sending the last byte of the header.
 *    -# The function shall stop receiving bytes from the smart card as soon as
 *       it has received xDataExpectedLen+2 bytes (procedure byte not included)
 *       or if a byte is not received within the work waiting time. In this
 *       latter case, the function shall return the error status
 *       ICC_ERROR_TIMEOUT and data pointed to by pxDataLen and pxData shall be
 *       valid and correct.
 *    -# After receiving the command header, the smart card may directly return
 *       SW1 and SW2 to report an error (SW1 is considered as a procedure byte
 *       and replaces the ACK byte) without sending back any data bytes.
 *       Although in that case the ICC driver does not receive the number of
 *       expected data bytes, it shall not return the error status
 *       ICC_ERROR_TIMEOUT and *pxDataLen shall be equal to 0.
*/
TIccStatus iccT0Receive
(
        TIccSessionId      xSessionId,
  const TIccT0Header      pxHeader,
        TSize              xDataExpectedLen,
        TSize*            pxDataLen,
        TUnsignedInt8*    pxData,
        TIccT0StatusWords pxStatusWords
);


/**
 *  @brief
 *    This function is responsible of sending a T=0 data block to the smartcard
 *    and retrieving the related reply data block from the smartcard.
 *
 *    The function stops receiving bytes when the xReplyMaxLen value is reached
 *    or when no character has been received for more than CharacterWaitingTime.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface session, as given in the
 *             event notification callback.
 *  @param   xSendLen
 *             Length of send sequence.
 *  @param   pxSendBytes
 *             Pointer to the buffer containing the send sequence to be sent
 *             to the smartcard or NULL if only receive transfer.
 *  @param   xReceiveMaxLen
 *             Maximum length of the reply sequence or 0 if only send
 *             transfer.
 *  @param   pxReceiveLen
 *             Length of the reply block.
 *  @param   pxReceiveBytes
 *             Pointer to the buffer where the smartcard reply block will
 *             be stored.
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_CARD_REMOVED
 *              The smartcard in not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *  @retval  ICC_ERROR_TIMEOUT
 *             The data exchange stopped due to excessive time between
 *             successive characters.
 *  @retval  ICC_ERROR
 *             The data exchange has failed due to communication errors.
 *
 *  @remarks
 *    -# This function is synchronous. Data pointed to by pxReplyBlock must be
 *       valid as soon as the CA returns from this function.
*/
TIccStatus iccT0Exchange
(
        TIccSessionId     xSessionId,
        TSize             xSendLen,
  const TUnsignedInt8*   pxSendBytes,
        TSize             xReceiveMaxLen,
        TSize*           pxReceiveLen,
        TUnsignedInt8*   pxReceiveBytes
);


/**
 *  @brief
 *    This function allows the CA to change the smartcard access mode. It may
 *    be called at any time between insertion and extraction notifications.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard session to modify, as given in the
 *             event notification callback.
 *  @param   xMode
 *             New smartcard access mode. ICC_ACCESS_NONE is used to release
 *             the smartcard.
 *
 *  @retval   ICC_NO_ERROR
 *              The data exchange has been performed successfully.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_MODE
 *              The requested access mode is not supported.
 *  @retval   ICC_ERROR_CONFLICT
 *              The requested access mode is in conflict with another
 *              application.
*/
TIccStatus iccModeChange
(
  TIccSessionId       xSessionId,
  TIccAccessMode      xMode
);


/**
 *  @brief
 *    This function allows the CA to reset the smartcard.
 *
 *    It may be called at any time between insertion and extraction
 *    notifications. This call is only allowed if the application
 *    communicates with the smartcard in exclusive mode.
 *
 *  @pre
 *    None.
 *
 *  @post
 *    None.
 *
 *  @param   xSessionId
 *             Identifier of the smartcard interface session to reset,
 *             as given in the event notification callback.
 *  @param   xColdReset
 *             TRUE if the driver has to initiate a cold reset.
 *             FALSE if the driver has to initiate a warm reset.
 *
 *  @retval   ICC_NO_ERROR
 *              The smartcard has been successfully reset.
 *  @retval   ICC_ERROR_SESSION_ID
 *              The session id doesn't exist.
 *  @retval   ICC_ERROR_MODE
 *              The current session is as shared session and thus the
 *              smartcard cannot be reset.
 *  @retval   ICC_ERROR_REMOVED
 *              The smart card is not inserted in the reader.
 *  @retval   ICC_ERROR_CARD_MUTE
 *              Something is inserted in the reader but there is no
 *              communication at all. The card may be inserted upside down.
 *
 *  @remarks
 *    -# This function is synchronous. The ATR record must have been updated
 *       when this function returns.
*/
TIccStatus iccSmartcardReset
(
  TIccSessionId       xSessionId,
  TBoolean            xColdReset
);


#endif /* CA_ICC_H */
