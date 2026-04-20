.text

.globl neon_test;
.align 2;
neon_test:
	dsb st
	mov w2, #254
	mov w3, #3
	mov v2.b[8], w2
	mov v3.b[8], w3
	add v1.8b, v2.8b, v3.8b
	add v4.16b, v2.16b, v3.16b
	ret lr
.type neon_test, %function;
.size neon_test, .-neon_test

.end
