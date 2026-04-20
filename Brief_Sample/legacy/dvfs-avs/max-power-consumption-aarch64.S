/********************************************************************************************/
/* Montage Technology (Shanghai) Co., Ltd.                                                  */
/* Montage Proprietary and Confidential                                                     */
/* Copyright (c) 2023 Montage Technology Group Limited and its affiliated companies         */
/********************************************************************************************/
	.text
	.global test_start

.equ NUM_ITERS, 50

test_start:
	mov     x11, #NUM_ITERS

	ldr     x0, =0xFFFFFFFF55555555
	ldr     x1, =0x0000000055555555
	ldr     x2, =0xFFFFFFFDAAAAAAAA
	ldr     x3, =0x00000000AAAAAAAA
	ldr     x4, =0xFFFFFFFF55555555
	ldr     x5, =0x0000000055555555
	ldr     x6, =0xFFFFFFFDAAAAAAAA
	ldr     x7, =0x00000000AAAAAAAA
	ldr     x9, =0xFFFFFFFD55555555
	ldr     x10, =0x0000000155555555

	movi v1.16b, #0x66  // q1 = 0x66666666
	movi v2.16b, #0xAA  // q2 = 0xAAAAAAAA
	movi v3.16b, #0x66  // q3 = 0x66666666
	movi v4.16b, #0xAA  // q4 = 0xAAAAAAAA
	movi v5.16b, #0x66  // q5 = 0x66666666
	movi v6.16b, #0xAA  // q6 = 0xAAAAAAAA
	movi v7.16b, #0x66  // q7 = 0x66666666
	movi v8.16b, #0xAA  // q8 = 0xAAAAAAAA
	movi v9.16b, #0x66  // q9 = 0x66666666
	b       power_loop

	// Align the loop to an 8-byte boundary so that two instructions
	// will be in the same instruction cache data bank.  This allows
	// us to dual-issue all pairs of instructions in the loop for
	// maximum power in the pipeline.
	.balign 8
power_loop:
	// Group 1
	fmla      v1.4s, v2.4s, v3.4s  // Dual Issue Pair 0, Instruction 0
	ldr       w8, [x0, x1, lsl #2] // Dual Issue Pair 0, Instruction 1

	ldr       w8, [x2, x3, lsl #2] // Dual Issue Pair 1, Instruction 0
	add       x0, x1, x9           // Dual Issue Pair 1, Instruction 1

	sub       x2, x3, x10          // Dual Issue Pair 2, Instruction 0
	ldr       w8, [x4, x5, lsl #2] // Dual Issue Pair 2, Instruction 1

	ldr       w8, [x6, x7, lsl #2] // Dual Issue Pair 3, Instruction 0
	add       x4, x9, x5           // Dual Issue Pair 3, Instruction 1

	// Group 2
	fmla      v4.4s, v5.4s, v6.4s  // Dual Issue Pair 0, Instruction 0
	ldr       w8, [x0, x3, lsl #2] // Dual Issue Pair 0, Instruction 1

	ldr       w8, [x2, x1, lsl #2] // Dual Issue Pair 1, Instruction 0
	sub       x0, x3, x10          // Dual Issue Pair 1, Instruction 1

	add       x2, x1, x9           // Dual Issue Pair 2, Instruction 0
	ldr       w8, [x4, x7, lsl #2] // Dual Issue Pair 2, Instruction 1

	ldr       w8, [x6, x7, lsl #2] // Dual Issue Pair 3, Instruction 0
	sub       x4, x7, x10          // Dual Issue Pair 3, Instruction 1

	// Group 3
	fmla      v7.4s, v8.4s, v9.4s  // Dual Issue Pair 0, Instruction 0
	ldr       w8, [x0, x1, lsl #2] // Dual Issue Pair 0, Instruction 1

	ldr       w8, [x2, x3, lsl #2] // Dual Issue Pair 1, Instruction 0
	add       x0, x1, x9           // Dual Issue Pair 1, Instruction 1

	sub       x2, x3, x10          // Dual Issue Pair 2, Instruction 0
	ldr       w8, [x4, x5, lsl #2] // Dual Issue Pair 2, Instruction 1

	ldr       w8, [x6, x7, lsl #2] // Dual Issue Pair 3, Instruction 0
	add       x4, x9, x5           // Dual Issue Pair 3, Instruction 1

	// Group 4
	fmla      v4.4s, v5.4s, v6.4s  // Dual Issue Pair 0, Instruction 0
	ldr       w8, [x0, x3, lsl #2] // Dual Issue Pair 0, Instruction 1

	ldr       w8, [x2, x1, lsl #2] // Dual Issue Pair 1, Instruction 0
	sub       x0, x3, x10          // Dual Issue Pair 1, Instruction 1

	add       x2, x1, x9           // Dual Issue Pair 2, Instruction 0
	ldr       w8, [x4, x7, lsl #2] // Dual Issue Pair 2, Instruction 1

	ldr       w8, [x6, x7, lsl #2] // Dual Issue Pair 3, Instruction 0
	sub       x4, x7, x10          // Dual Issue Pair 3, Instruction 1

	// End of loop check
	subs    x11, x11, #1
	bne     power_loop
	ret
