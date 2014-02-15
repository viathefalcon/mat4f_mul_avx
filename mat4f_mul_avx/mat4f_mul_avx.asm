;
; Author: Stephen Higgins <sjh@viathefalcon.net>
; Blog: http://blog.viathefalcon.net/
; Twitter: @viathefalcon
;

.CODE

; This was my first attempt, clumsily transposing the 2nd matrix at the same
; time as aligning it on the stack for use with AVX
mat4f_mul_avx0 PROC PUBLIC FRAME
	push rbx
	.pushreg rbx
	push rsi
	.pushreg rsi
	push rdi
	.pushreg rdi
	push rbp
	.pushreg rbp
    sub rsp, 28h
    .allocstack 28h
	mov rbp, rsp
	.setframe rbp, 0
.ENDPROLOG

	; Stash the values in RCX, RDX while we faff about with them
	mov r10, rcx
	mov r11, rdx

	; Capture the current value of the timestamp counter
	xor eax, eax
	cpuid
	rdtscp
	shl rdx, 32
	or rdx, rax
	mov [r9], rdx

	; Restore the registers as received
	mov rcx, r10
	xchg rdx, r11

	; Adjust the stack pointer for 256 bit (32 byte) alignment,
	; (if it isn't already)
	mov rax, rsp
	mov rbx, 20h
	mov r10, rdx				; Save the pointer to the first matrix
	mov rdx, 0
	div rbx
	mov r11, rdx
	add r11, 80h				; Add enough space to contain a copy of the matrix, starting at the newly-aligned stack pointer
	sub rsp, r11

	; Prime the loop to transpose the 2nd matrix
	mov rax, 4					; 4 rows/columns
	mov rbx, r8					; Pointer to 2nd input matrix
	mov rdi, rsp				; Pointer to output matrix

mtranspose:
	; Move the value for each column into the next location on the stack
	mov rsi, rbx
	movsd
	lea rsi, [rsi+12]			; Skip to the next column in the same row
	movsd
	lea rsi, [rsi+12]			; Skip to the next column in the same row
	movsd
	lea rsi, [rsi+12]			; Skip to the next column in the same row
	movsd

	; Advance to the next row
	lea rbx, [rbx+4]

	; Test against the loop bound
	dec rax
	jnz mtranspose

	; Load each pair of rows of the 2nd matrix pairwise into a pair of YMM registers
	mov rsi, rsp
	vzeroupper
	vmovaps ymm0, ymmword ptr [rsi]
	lea rsi, [rsi+20h]
	vmovaps ymm1, ymmword ptr [rsi]

	; Prime the loop to do the multiplication
	mov rsi, r10
	mov rax, rcx
	mov rbx, 4

mmultiply:
	; Broadcast copies of the nth column into both halves of a YMM register..
	vbroadcastf128 ymm2, xmmword ptr [rsi]
	lea rsi, [rsi+16]

	; Do the multiplication with the first pair of rows
	vmulps ymm3, ymm0, ymm2

	; Addition
	vhaddps ymm3, ymm3, ymm3
	vhaddps ymm3, ymm3, ymm3

	; Move the first result - given in the lower half of YMM out to the output matrix
	vmovss real4 ptr [rcx], xmm3
	lea rcx, [rcx+4]

	; Swop the halves of the register to get at the next result
	vperm2f128 ymm3, ymm3, ymm3, 1
	vmovss real4 ptr [rcx], xmm3
	lea rcx, [rcx+4]

	; Do the multiplication with the second pair of rows
	vmulps ymm3, ymm1, ymm2

	; Addition
	vhaddps ymm3, ymm3, ymm3
	vhaddps ymm3, ymm3, ymm3

	; Move the first result - given in the lower half of YMM out to the output matrix
	vmovss real4 ptr [rcx], xmm3
	lea rcx, [rcx+4]

	; Swop the halves of the register to get at the next result
	vperm2f128 ymm3, ymm3, ymm3, 1
	vmovss real4 ptr [rcx], xmm3
	lea rcx, [rcx+4]

	; Test against the loop bound
	dec rbx
	jnz mmultiply
	vzeroupper

	; Epilogue 
	add rsp, r11
	add rsp, 28h
	pop rbp
	pop rdi
	pop rsi
	pop rbx

	; Get the timestamp counter
	mov r11, rax
	rdtscp
	shl rdx, 32
	or rax, rdx

	; Get the difference
	sub rax, [r9]
	mov [r9], rax
	xor eax, eax
	cpuid

	; Return with a pointer to the output matrix in RAX
	mov rax, r11
	ret

mat4f_mul_avx0 ENDP

; Tranposes the 2nd matrix using VEX-encoded shuffles and permutes,
; but the multiplication step still iteratively computes a single
; output value at a time
mat4f_mul_avx1 PROC PUBLIC FRAME
	push rbx
	.pushreg rbx
	push rsi
	.pushreg rsi
.ENDPROLOG

	; Stash the values in RCX, RDX while we faff about with them
	mov r10, rcx
	mov r11, rdx

	; Capture the current value of the timestamp counter
	xor eax, eax
	cpuid
	rdtscp
	shl rdx, 32
	or rdx, rax

	; Restore the registers as received
	mov rcx, r10
	xchg rdx, r11

	; Unaligned load of the columns of the 2nd matrix
	vzeroupper
	vmovups ymm0, [r8]
	vmovups ymm1, [r8+20h]

transpose:
	; Swop the first pair of columns into another register to allow some cross-lane shenanigans,
	; and interleave the first elements of each column to form doubles
	vperm2f128 ymm3, ymm0, ymm0, 1
	vunpcklps ymm2, ymm0, ymm3
	vunpckhps ymm4, ymm0, ymm3

	; Same for the second pair
	vperm2f128 ymm3, ymm1, ymm1, 1
	vunpckhps ymm5, ymm1, ymm3
	vunpcklps ymm3, ymm1, ymm3

	; Interleave the doubles to get the rows,
	; and then pack them into a single register
	vunpcklpd ymm1, ymm2, ymm3
	vunpckhpd ymm2, ymm2, ymm3
	vinsertf128 ymm0, ymm1, xmm2, 1

	; Same for the second pair
	vunpcklpd ymm2, ymm4, ymm5
	vunpckhpd ymm3, ymm4, ymm5
	vinsertf128 ymm1, ymm2, xmm3, 1

	; ymm0 contains the 1st 2 rows
	; ymm1 contains the 2nd 2 rows

	; Prime the loop to do the multiplication
	vxorps ymm4, ymm4, ymm4				; Zero this register
	mov rsi, rdx
	mov rax, rcx
	mov rbx, 4

mmultiply:
	; Broadcast copies of the nth column into both halves of a YMM register..
	vbroadcastf128 ymm2, xmmword ptr [rsi]
	lea rsi, [rsi+10h]

	; Do the multiplication with the first pair of rows
	vmulps ymm3, ymm0, ymm2

	; Addition
	vhaddps ymm3, ymm3, ymm4
	vhaddps ymm3, ymm3, ymm4

	; Move the first result - given in the lower half of YMM out to the output matrix
	vmovss real4 ptr [rcx], xmm3
	lea rcx, [rcx+4]

	; Swop the halves of the register to get at the next result
	vperm2f128 ymm3, ymm3, ymm3, 1
	vmovss real4 ptr [rcx], xmm3
	lea rcx, [rcx+4]

	; Do the multiplication with the second pair of rows
	vmulps ymm3, ymm1, ymm2

	; Addition
	vhaddps ymm3, ymm3, ymm4
	vhaddps ymm3, ymm3, ymm4

	; Move the first result - given in the lower half of YMM out to the output matrix
	vmovss real4 ptr [rcx], xmm3
	lea rcx, [rcx+4]

	; Swop the halves of the register to get at the next result
	vperm2f128 ymm3, ymm3, ymm3, 1
	vmovss real4 ptr [rcx], xmm3
	lea rcx, [rcx+4]

	; Test against the loop bound
	dec rbx
	cmp rbx, 0
	jne mmultiply
	vzeroupper

	; Get the timestamp counter
	rdtscp
	shl rdx, 32
	or rax, rdx

	; Get the difference
	sub rax, r11
	mov [r9], rax
	xor eax, eax
	cpuid

	; Epilogue 
	pop rsi
	pop rbx

	; Return with a pointer to the output matrix in RAX 
	mov rax, r10
	ret

mat4f_mul_avx1 ENDP

; Unrolls the loop in the multiplication step, and operates on two
; input columns per iteration, but still outputs one value at a time
mat4f_mul_avx2 PROC PUBLIC

	; Stash the values in RCX, RDX while we faff about with them
	mov r10, rcx
	mov r11, rdx

	; Capture the current value of the timestamp counter
	xor eax, eax
	cpuid
	rdtscp
	shl rdx, 32
	or rdx, rax

	; Restore the registers as received
	mov rcx, r10
	xchg rdx, r11

	; Unaligned load of the columns of the 2nd matrix
	vzeroupper
	vmovups ymm0, [r8]
	vmovups ymm1, [r8+20h]

transpose:
	; Swop the first pair of columns into another register to allow some cross-lane shenanigans,
	; and interleave the first elements of each column to form doubles
	vperm2f128 ymm3, ymm0, ymm0, 1
	vunpcklps ymm2, ymm0, ymm3
	vunpckhps ymm4, ymm0, ymm3

	; Same for the second pair
	vperm2f128 ymm3, ymm1, ymm1, 1
	vunpckhps ymm5, ymm1, ymm3
	vunpcklps ymm3, ymm1, ymm3

	; Interleave the doubles to get the rows,
	; and then pack them into a single register
	vunpcklpd ymm1, ymm2, ymm3
	vunpckhpd ymm2, ymm2, ymm3
	vinsertf128 ymm0, ymm1, xmm2, 1

	; Same for the second pair
	vunpcklpd ymm2, ymm4, ymm5
	vunpckhpd ymm3, ymm4, ymm5
	vinsertf128 ymm1, ymm2, xmm3, 1

	; ymm0 contains the 1st 2 rows
	; ymm1 contains the 2nd 2 rows

	; Prime the loop to do the multiplication
	mov rax, rcx
	mov r10, 2

mmultiply:
	; Load in the next pair of columns from the first matrix
	vmovups ymm2, [rdx]
	lea rdx, [rdx+20h]

	; Permute the first column into both halves of another register
	vperm2f128 ymm3, ymm2, ymm2, 0

	; Do the multiplication/addition with the 1st 2 rows
	vmulps ymm4, ymm0, ymm3
	vhaddps ymm4, ymm4, ymm4
	vhaddps ymm4, ymm4, ymm4

	; Move the first result - given in the lower half of YMM out to the output matrix
	vmovss real4 ptr [rcx], xmm4
	lea rcx, [rcx+4]

	; Swop the halves of the register to get at the next result
	vperm2f128 ymm4, ymm4, ymm4, 1
	vmovss real4 ptr [rcx], xmm4
	lea rcx, [rcx+4]

	; Do the multiplication/addition with the 2nd 2 rows
	vmulps ymm4, ymm1, ymm3
	vhaddps ymm4, ymm4, ymm4
	vhaddps ymm4, ymm4, ymm4

	; Move the first result - given in the lower half of YMM out to the output matrix
	vmovss real4 ptr [rcx], xmm4
	lea rcx, [rcx+4]

	; Swop the halves of the register to get at the next result
	vperm2f128 ymm4, ymm4, ymm4, 1
	vmovss real4 ptr [rcx], xmm4
	lea rcx, [rcx+4]

	; Permute the second column into both halves of the register
	vperm2f128 ymm2, ymm2, ymm2, 11h

	; Do the multiplication/addition with the 1st pair of rows
	vmulps ymm4, ymm0, ymm2
	vhaddps ymm4, ymm4, ymm4
	vhaddps ymm4, ymm4, ymm4

	; Move the first result - given in the lower half of YMM out to the output matrix
	vmovss real4 ptr [rcx], xmm4
	lea rcx, [rcx+4]

	; Swop the halves of the register to get at the next result
	vperm2f128 ymm4, ymm4, ymm4, 1
	vmovss real4 ptr [rcx], xmm4
	lea rcx, [rcx+4]

	; Do the multiplication/addition with the 2nd pair of rows
	vmulps ymm4, ymm1, ymm2
	vhaddps ymm4, ymm4, ymm4
	vhaddps ymm4, ymm4, ymm4

	; Move the first result - given in the lower half of YMM out to the output matrix
	vmovss real4 ptr [rcx], xmm4
	lea rcx, [rcx+4]

	; Swop the halves of the register to get at the next result
	vperm2f128 ymm4, ymm4, ymm4, 1
	vmovss real4 ptr [rcx], xmm4
	lea rcx, [rcx+4]

	; Test against the loop bound
	dec r10
	jnz mmultiply
	vzeroupper

	; Get the timestamp counter
	mov r10, rax
	rdtscp
	shl rdx, 32
	or rax, rdx

	; Get the difference
	sub rax, r11
	mov [r9], rax
	xor eax, eax
	cpuid

	; Return with a pointer to the output matrix in RAX 
	mov rax, r10
	ret

mat4f_mul_avx2 ENDP

; Folds the transposition step into the multiplication step, and
; outputs a column per iteration
mat4f_mul_avx3 PROC PUBLIC

	; Stash the values in RCX, RDX while we faff about with them
	mov r10, rcx
	mov r11, rdx

	; Capture the current value of the timestamp counter
	cpuid
	xor eax, eax
	rdtscp
	shl rdx, 32
	or rdx, rax

	; Restore the registers as received
	mov rcx, r10
	xchg rdx, r11

	; Get in the columns from the second matrix
	vzeroupper
	vmovups ymm0, [r8]
	vmovups ymm1, [r8+20h]

	; Prime the loop to do the multiplication
	mov rax, 4

mmultiply:
	; Get in the next column from the first matrix
	vmovups xmm2, [rdx]
	lea rdx, [rdx+10h]
	vshufps xmm3, xmm2, xmm2, 0			; 1st element
	vshufps xmm4, xmm2, xmm2, 55h		; 2nd element
	vperm2f128 ymm5, ymm3, ymm4, 20h
	vshufps xmm3, xmm2, xmm2, 0AAh		; 3rd element
	vshufps xmm4, xmm2, xmm2, 0FFh		; 4th element
	vperm2f128 ymm2, ymm3, ymm4, 20h

	; Do the multiplication and addition, and output the first column of the result
	vmulps ymm3, ymm5, ymm0
	vmulps ymm4, ymm2, ymm1
	vaddps ymm5, ymm3, ymm4
	vperm2f128 ymm4, ymm5, ymm5, 1
	vaddps ymm3, ymm5, ymm4
	vmovups [rcx], xmm3
	lea rcx, [rcx+10h]

	; Test against the loop bound
	sub rax, 1
	jnz mmultiply
	vzeroupper

	; Get the timestamp counter
	rdtscp
	shl rdx, 32
	or rax, rdx
	sub rax, r11
	mov [r9], rax
	xor eax, eax
	cpuid

	; Return with a pointer to the output matrix in RAX
	mov rax, r10
	ret

mat4f_mul_avx3 ENDP

END
