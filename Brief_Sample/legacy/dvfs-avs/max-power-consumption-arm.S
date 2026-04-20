/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
	.text
	.global test_start

.equ NUM_ITERS, 50

test_start:
	push	{r4 - r11}
	mov     r11, #NUM_ITERS

	// Set initial register values
#if 0
	ldr     r0,=0x00000000
	ldr     r1,=0xfffffffd
	ldr     r2,=0xfffffffc
	ldr     r3,=0x00000006
	ldr     r4,=0x00000000
	ldr     r5,=0xfffffffd
	ldr     r6,=0xfffffffc
	ldr     r7,=0x00000006
#else
	ldr     r0,=0x00000000
	ldr     r1,=0x000ffffd
	ldr     r2,=0x000ffffc
	ldr     r3,=0x00020006
	ldr     r4,=0x00000000
	ldr     r5,=0x000ffffd
	ldr     r6,=0x000ffffc
	ldr     r7,=0x00000006
#endif

	vmov.i8 q1, #0x66  // q1 = 0x66666666
	vmov.i8 q2, #0xAA  // q2 = 0xAAAAAAAA
	vmov.i8 q3, #0x66  // q3 = 0x66666666
	vmov.i8 q4, #0xAA  // q4 = 0xAAAAAAAA
	vmov.i8 q5, #0x66  // q5 = 0x66666666
	vmov.i8 q6, #0xAA  // q6 = 0xAAAAAAAA
	vmov.i8 q7, #0x66  // q7 = 0x66666666
	vmov.i8 q8, #0xAA  // q8 = 0xAAAAAAAA
	vmov.i8 q9, #0x66  // q9 = 0x66666666
	b       power_loop

	// Align the loop to an 8-byte boundary so that two instructions
	// will be in the same instruction cache data bank.  This allows
	// us to dual-issue all pairs of instructions in the loop for
	// maximum power in the pipeline.
	.balign 8
power_loop:
	// Group 1
	vfma.f32  q1, q2, q3           // Dual Issue Pair 0, Instruction 0
	ldr       r8, [r0,+r1,LSL #1]! // Dual Issue Pair 0, Instruction 1

	ldr       r8, [r2,+r3]!        // Dual Issue Pair 1, Instruction 0
	add       r0, r1, r9, LSR r12  // Dual Issue Pair 1, Instruction 1

	sub       r2, r3, r10, LSL r12 // Dual Issue Pair 2, Instruction 0
	ldr       r8, [r4,+r5,LSL #1]! // Dual Issue Pair 2, Instruction 1

	ldr       r8, [r6,+r7]         // Dual Issue Pair 3, Instruction 0
	add       r4, r5, r9, LSR r12  // Dual Issue Pair 3, Instruction 1

	// Group 2
	vfma.f32  q4, q5, q6           // Dual Issue Pair 0, Instruction 0
	ldr       r8, [r0,+r1,LSL #1]! // Dual Issue Pair 0, Instruction 1

	ldr       r8, [r2,+r3]!        // Dual Issue Pair 1, Instruction 0
	add       r0, r1, r9, LSR r12  // Dual Issue Pair 1, Instruction 1

	sub       r2, r3, r10, LSL r12 // Dual Issue Pair 2, Instruction 0
	ldr       r8, [r4,+r5,LSL #1]! // Dual Issue Pair 2, Instruction 1

	ldr       r8, [r6,+r7]         // Dual Issue Pair 3, Instruction 0
	add       r4, r5, r9, LSR r12  // Dual Issue Pair 3, Instruction 1

	// Group 3
	vfma.f32  q7, q8, q9           // Dual Issue Pair 0, Instruction 0
	ldr       r8, [r0,+r1,LSL #1]! // Dual Issue Pair 0, Instruction 1

	ldr       r8, [r2,+r3]!        // Dual Issue Pair 1, Instruction 0
	add       r0, r1, r9, LSR r12  // Dual Issue Pair 1, Instruction 1

	sub       r2, r3, r10, LSL r12 // Dual Issue Pair 2, Instruction 0
	ldr       r8, [r4,+r5,LSL #1]! // Dual Issue Pair 2, Instruction 1

	ldr       r8, [r6,+r7]         // Dual Issue Pair 3, Instruction 0
	add       r4, r5, r9, LSR r12  // Dual Issue Pair 3, Instruction 1

	// Group 4
	vfma.f32  q4, q5, q6           // Dual Issue Pair 0, Instruction 0
	ldr       r8, [r0,+r1,LSL #1]! // Dual Issue Pair 0, Instruction 1

	ldr       r8, [r2,+r3]!        // Dual Issue Pair 1, Instruction 0
	add       r0, r1, r9, LSR r12  // Dual Issue Pair 1, Instruction 1

	sub       r2, r3, r10, LSL r12 // Dual Issue Pair 2, Instruction 0
	ldr       r8, [r4,+r5,LSL #1]! // Dual Issue Pair 2, Instruction 1

	ldr       r8, [r6,+r7]         // Dual Issue Pair 3, Instruction 0
	add       r4, r5, r9, LSR r12  // Dual Issue Pair 3, Instruction 1

	// End of loop check
	subs    r11, r11, #1
	bne     power_loop

	pop	{r4 - r11}
	bx      lr
