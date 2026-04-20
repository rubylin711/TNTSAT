/*
 * Copyright (C) 2019 Montage Technology Group Limited and its affiliated companies
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TEE_CLIENT_NGWM_H
#define TEE_CLIENT_NGWM_H

#include <tee_client_api.h>

#ifdef __cplusplus
extern "C" {
#endif


/*
 * PTA_CMD_REG_OPS - read or write registers in byte, halfword or word.
 *
 * param[0] (in value) - read or write operation(.a = read or write, .b = byte, halfworld or word)
 * param[1] (inout value) - io address and value (.a=physical address, .b= value)
 * param[2] unused
 * param[3] unused
 */
#define PTA_CMD_REG_OPS		1

#define CA_NGWM_OPS_UUID { 0xdee067b0, 0x0834, 0x4c86,  \
		{0xb9, 0x9e, 0x09, 0xf0, 0x0f, 0xd1, 0x32, 0x9c} }

/* The function IDs implemented in this TA */
enum {
    TEEC_NGWM_CMD_CONFIG = 0,
    TEEC_NGWM_CMD_CONFIG_BYPIPE,
    //set main screen TSID value
    TEEC_NGWM_CMD_CONFIG_MAINID = 0x100,
};

#ifdef __cplusplus
}
#endif

#endif /* TEE_CLIENT_NEXGUARD_H */
