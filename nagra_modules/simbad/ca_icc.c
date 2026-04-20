/*
 * @file ca_icc.c
 * @brief Nagravision Stream Processing APIs implemetation on Montage Symphony4 platform
 *
 * Copyright (C) 2021 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 */
#include "ca_icc.h"

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
)
{
	//TODO:
	printf("[error] iccRegister iccRegister iccRegister \n");
	return ICC_NO_ERROR;
}


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
)
{
	//TODO:
	return ICC_NO_ERROR;
}

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
)
{
	//TODO:
	return ICC_NO_ERROR;
}


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
)
{
	//TODO:
	return ICC_NO_ERROR;
}



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
)
{
	//TODO:
	return ICC_NO_ERROR;
}


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
)
{
	//TODO:
	return ICC_NO_ERROR;
}


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
)
{
	//TODO:
	return ICC_NO_ERROR;
}


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
)
{
	//TODO:
	return ICC_NO_ERROR;
}


