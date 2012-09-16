/*
Copyright 2012 - 2012 Tomwi
Assumes 8k buffers, and 4n samples to deinterleave. Buffersize can
be adjusted with modifying one define
*/

.equ BUF_BYTES, 8192*2
.syntax unified
.align	2
.global	deInterleave
.thumb
.thumb_func
.type	deInterleave, %function

@ r0 = inbuf
@ r1 = outBuf
@ r2 = len (in samples, not the amount of total samples (left&right)

deInterleave:
	cmp r2, #4
	it lt
	bxlt lr   @ nothing to split

	bic r2, r2, #3
	push {r4-r9}

	add r3, r1, #BUF_BYTES
	mvn r8, #0
	lsl r8,#16

.split:
	ldmia r0!,{r4-r7}

	@ split left
	bic r9, r4, r8
	orr r9, r9, r5, lsl #16
	@ split left
	bic ip, r6, r8
	orr ip, ip, r7, lsl #16
	@ store left
	stmia r1!,{r9,ip}

	@ split right
	and r9, r5, r8
	orr r9, r9, r4, lsr #16
	@ split right
	and ip, r7, r8
	orr ip, ip, r6, lsr #16
	@ store right
	stmia r3!,{r9,ip}

	subs r2,#4
	bne .split

	pop {r4-r9}

	bx lr