.text

.globl neon_test;
.align 2;
neon_test:
	dsb st
	vmov.u8 q2, #254
	vmov.u8 q3, #3
	vadd.u8 q1, q2, q3
	bx lr
.type neon_test, %function;
.size neon_test, .-neon_test

.end
