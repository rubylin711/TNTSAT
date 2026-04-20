/*
 * Copyright (C) 2024 Montage Technology Group Limited and its affiliated companies
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
#ifndef TEEC_PVRX_H
#define TEEC_PVRX_H

#include <stdint.h>

/**
 * @brief Initialize pvrx context.
 *
 * @param ctx context to be initialized.
 *
 * @return 0 Success.
 * @return -1 Failed.
 */
int32_t TEEC_RecDataRd_Init(void **ctx);

/**
 * @brief Destroy the pvrx context.
 *
 * @param ctx context to be destroyed.
 *
 * @return 0 Success.
 * @return -1 Failed.
 */
int32_t TEEC_RecDataRd_Deinit(void *ctx);

/**
 * @brief Open a session with this ctx.
 *
 * @param ctx The context.
 *
 * @return 0 Success.
 * @return -1 Failed.
 */
int32_t TEEC_RecDataRd_Open(void *ctx);

/**
 * @brief Close the session of this context.
 *
 * @param ctx The context.
 *
 * @return 0 Success.
 * @return -1 Failed.
 */
int32_t TEEC_RecDataRd_Close(void *ctx);

/**
 * @brief Read data from buf_i, and write to buf_o.
 *
 * @param ctx		The context.
 * @param buf_i		input buffer to read data from.
 * @param size_i	input size.
 * @param buf_o		output buffer to write to.
 * @param size_o	output size.
 *
 * @return 0	Success.
 * @return -1	Failed.
 */
int32_t TEEC_RecDataRd_Read(void *ctx, void *buf_i, size_t size_i, void *buf_o, size_t size_o);

#endif /*TEEC_PVRX_H*/
