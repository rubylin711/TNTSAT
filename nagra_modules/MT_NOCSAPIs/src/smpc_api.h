/*
 * Copyright (C) 2020 Montage Technology Group Limited and its affiliated companies
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
#ifndef SMPC_API_H
#define SMPC_API_H

#include <tee_client_api.h>

/**
 * The type of a key slot.
 */
typedef enum {
	/* Slot used for Audio. */
	SMPC_SLOT_TYPE_AUDIO = 0,
	/* Slot used for Video. */
	SMPC_SLOT_TYPE_VIDEO = 0x10,
	SMPC_SLOT_TYPE_SUB_VIDEO,
	/* Slot used for PVR decryption. */
	SMPC_SLOT_TYPE_PVR_DEC = 0x20,
	SMPC_SLOT_TYPE_PVR1_DEC,
	SMPC_SLOT_TYPE_PVR2_DEC,
	SMPC_SLOT_TYPE_PVR3_DEC,
	/* Slot used for PVR encryption. */
	SMPC_SLOT_TYPE_PVR_ENC = 0x30,
	SMPC_SLOT_TYPE_PVR1_ENC,
	SMPC_SLOT_TYPE_PVR2_ENC,
	SMPC_SLOT_TYPE_PVR3_ENC,
	/* Slot used for OTT. */
	SMPC_SLOT_TYPE_OTT = 0x40,
	SMPC_SLOT_TYPE_SUB_OTT,
	/* Slot used for EXPORT OTT. */
       SMPC_SLOT_TYPE_EXPORT_OTT_DEC = 0x50,
       SMPC_SLOT_TYPE_EXPORT_OTT_ENC = 0x60,
} SMPC_SlotType;

/**
 * The type of a block of memory.
 */
typedef enum {
	/* Memory used for Audio. */
	SMPC_MEM_TYPE_AUDIO = 0,
	/* Memory used for Video. */
	SMPC_MEM_TYPE_VIDEO = 0x100,
	SMPC_MEM_TYPE_SUB_VIDEO,
	/* Memory used for PVR. */
	SMPC_MEM_TYPE_PVR = 0x200,
	SMPC_MEM_TYPE_PVR1,
	SMPC_MEM_TYPE_PVR2,
	SMPC_MEM_TYPE_PVR3,
	/* Memory used for OTT. */
	SMPC_MEM_TYPE_OTT = 0x300,
	SMPC_MEM_TYPE_ENC_OTT = 0x400,
} SMPC_MemType;

/**
 * @brief Opens a new SMP context with the specified key slot.
 *
 * @param keySlot  the key slot to be bound with the context.
 * @param slotType the type of the key slot.
 *
 * @return TEEC_SUCCESS              Successfully opened a new context.
 * @return TEEC_ERROR_BAD_PARAMETERS Input parameters were invalid.
 * @return TEEC_ERROR_OUT_OF_MEMORY  System ran out of resources.
 */
TEEC_Result SMPC_Open(uint32_t keySlot, SMPC_SlotType slotType);

/**
 * @brief Register a block of memory to the SMP context specified
 * by the key slot.
 *
 * @param keySlot     the key slot bound with the context.
 * @param memType     the type of the block of memory.
 * @param memPhysAddr the physical address of the block of memory.
 * @param memSize     the size of the block of memory.
 *
 * @return TEEC_SUCCESS              Successfully registered a block of memory.
 * @return TEEC_ERROR_BAD_PARAMETERS Input parameters were invalid.
 */
TEEC_Result SMPC_RegisterMemory(uint32_t keySlot, SMPC_MemType memType, uint32_t memPhysAddr, size_t memSize);

/**
 * @brief Set IV to the specific key slot.
 *
 * @param keySlot     the key slot bound with a SMP context.
 * @param IV          the IV needed to be set.
 * @param IVSize      the size of the IV.
 *
 * @return TEEC_SUCCESS              Successfully registered a block of memory.
 * @return TEEC_ERROR_BAD_PARAMETERS Input parameters were invalid.
 */
TEEC_Result SMPC_SetIV(uint32_t keySlot, const void *IV, size_t IVSize);

/**
 * @brief Closes the SMP context which has been opened with the
 * specified key slot.
 *
 * @param keySlot the key slot bound with the context.
 *
 * @return TEEC_SUCCESS              Successfully closed the context.
 * @return TEEC_ERROR_BAD_PARAMETERS Input parameters were invalid.
 * @return TEEC_ERROR_SECURITY       A security fault was detected.
 */
TEEC_Result SMPC_Close(uint32_t keySlot);

#endif /* SMPC_API_H */
