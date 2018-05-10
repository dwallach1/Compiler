.text
	.globl go
go:
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	call _main
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbp
	popq %rbx
	retq
_main:
	subq $24, %rsp
	movq %r13, 8(%rsp)
	movq %r12, 0(%rsp)
	movq $5, %rdi
	movq $5, %rsi
	movq %r10, 16(%rsp)
	call allocate # runtime system call
	movq 16(%rsp), %r10
	movq %rax, %r12
	movq %r12, %r10
	addq $8, %r10
	movq %r10, %r13
	movq $85, 0(%r13)
	movq %r12, %rdi
	movq %r10, 16(%rsp)
	call print # runtime system call
	movq 16(%rsp), %r10
	movq %r12, %rdi
	movq $_SuchyAndWallachprintArraymainR_E_T_U_R_N_L_A_B_E_L2, -8(%rsp)
	movq %r10, 16(%rsp)
	subq $8, %rsp
	jmp _SuchyAndWallachprintArray
	_SuchyAndWallachprintArraymainR_E_T_U_R_N_L_A_B_E_L2:
	movq 16(%rsp), %r10
	movq $5, %rdi
	movq $5, %rsi
	movq %r10, 16(%rsp)
	call allocate # runtime system call
	movq 16(%rsp), %r10
	movq %rax, %r10
	movq %r10, 0(%r13)
	movq %r12, %rdi
	movq %r10, 16(%rsp)
	call print # runtime system call
	movq 16(%rsp), %r10
	movq %r12, %rdi
	movq $_SuchyAndWallachprintArraymainR_E_T_U_R_N_L_A_B_E_L1, -8(%rsp)
	movq %r10, 16(%rsp)
	subq $8, %rsp
	jmp _SuchyAndWallachprintArray
	_SuchyAndWallachprintArraymainR_E_T_U_R_N_L_A_B_E_L1:
	movq 16(%rsp), %r10
	movq 0(%rsp), %r12
	movq 8(%rsp), %r13
	addq $24, %rsp
	retq
_SuchyAndWallachprintArray:
	subq $32, %rsp
	movq %r14, 16(%rsp)
	movq %r13, 8(%rsp)
	movq %r12, 0(%rsp)
	movq %rdi, %r12
	movq $0, %r14
	movq 0(%r12), %r13
	movq %r12, %r10
	andq $1, %r10
	movq %r10, %r10
	cmpq $1, %r10
	je _isAnIntSuchyAndWallachprintArray
	jmp _isAnArraySuchyAndWallachprintArray
	_isAnIntSuchyAndWallachprintArray:
	movq $0, %rax
	movq 0(%rsp), %r12
	movq 8(%rsp), %r13
	movq 16(%rsp), %r14
	addq $32, %rsp
	retq
	_isAnArraySuchyAndWallachprintArray:
	_checkSuchyAndWallachprintArray:
	cmpq %r13, %r14
	setl %r10b
	movzbq %r10b, %r10
	cmpq $1, %r10
	je _trueSuchyAndWallachprintArray
	jmp _falseSuchyAndWallachprintArray
	_trueSuchyAndWallachprintArray:
	movq %r14, %r10
	salq $1, %r10
	movq %r10, %r10
	movq %r10, %r10
	addq $1, %r10
	movq %r10, %r10
	movq %r12, %rdi
	movq %r10, %rsi
	movq $_SuchyAndWallachprintDataSuchyAndWallachprintArrayR_E_T_U_R_N_L_A_B_E_L0, -8(%rsp)
	movq %r10, 24(%rsp)
	subq $8, %rsp
	jmp _SuchyAndWallachprintData
	_SuchyAndWallachprintDataSuchyAndWallachprintArrayR_E_T_U_R_N_L_A_B_E_L0:
	movq 24(%rsp), %r10
	movq %r14, %r10
	addq $1, %r10
	movq %r10, %r14
jmp _checkSuchyAndWallachprintArray
	_falseSuchyAndWallachprintArray:
	movq $1, %rax
	movq 0(%rsp), %r12
	movq 8(%rsp), %r13
	movq 16(%rsp), %r14
	addq $32, %rsp
	retq
_SuchyAndWallachprintData:
	subq $40, %rsp
	movq %r14, 16(%rsp)
	movq %r13, 8(%rsp)
	movq %r12, 0(%rsp)
	movq %rsi, %r14
	movq %rdi, %r13
	movq %r14, %rdi
	movq %r10, 24(%rsp)
	movq %r11, 32(%rsp)
	call print # runtime system call
	movq 32(%rsp), %r11
	movq 24(%rsp), %r10
	movq 0(%r13), %r10
	movq %r10, %r10
	salq $1, %r10
	movq %r10, %r10
	movq %r10, %r10
	addq $1, %r10
	movq %r10, %r10
	cmpq %r10, %r14
	setl %r10b
	movzbq %r10b, %r10
	cmpq $1, %r10
	je _correctSuchyAndWallachprintData
	jmp _wrongSuchyAndWallachprintData
	_wrongSuchyAndWallachprintData:
	movq %r13, %rdi
	movq %r14, %rsi
	movq %r10, 24(%rsp)
	movq %r11, 32(%rsp)
	subq $8, %rsp
	jmp *%r12
	movq 32(%rsp), %r11
	movq 24(%rsp), %r10
	_correctSuchyAndWallachprintData:
	movq %r14, %r10
	sarq $1, %r10
	movq %r10, %r12
	movq %r12, %r10
	imulq $8, %r10
	movq %r10, %r12
	movq %r12, %r10
	addq $8, %r10
	movq %r10, %r12
	movq %r12, %r10
	salq $1, %r10
	movq %r10, %r10
	movq %r10, %r10
	addq $1, %r10
	movq %r10, %r10
	movq %r10, %rdi
	movq %r10, 24(%rsp)
	movq %r11, 32(%rsp)
	call print # runtime system call
	movq 32(%rsp), %r11
	movq 24(%rsp), %r10
	movq %r12, %r10
	addq %r13, %r10
	movq %r10, %r10
	movq 0(%r10), %r10
	movq %r10, %r11
	andq $1, %r11
	movq %r11, %r11
	cmpq $1, %r11
	je _isAnInt2SuchyAndWallachprintData
	jmp _isAnArray2SuchyAndWallachprintData
	_isAnInt2SuchyAndWallachprintData:
	movq %r10, %rdi
	movq %r10, 24(%rsp)
	movq %r11, 32(%rsp)
	call print # runtime system call
	movq 32(%rsp), %r11
	movq 24(%rsp), %r10
jmp _exitFunctionSuchyAndWallachprintData
	_isAnArray2SuchyAndWallachprintData:
	movq %r10, %rdi
	movq $_SuchyAndWallachprintArraySuchyAndWallachprintDataR_E_T_U_R_N_L_A_B_E_L0, -8(%rsp)
	movq %r10, 24(%rsp)
	movq %r11, 32(%rsp)
	subq $8, %rsp
	jmp _SuchyAndWallachprintArray
	_SuchyAndWallachprintArraySuchyAndWallachprintDataR_E_T_U_R_N_L_A_B_E_L0:
	movq 32(%rsp), %r11
	movq 24(%rsp), %r10
	_exitFunctionSuchyAndWallachprintData:
	movq 0(%rsp), %r12
	movq 8(%rsp), %r13
	movq 16(%rsp), %r14
	addq $40, %rsp
	retq
