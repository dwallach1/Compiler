.text
	.globl go
go:
	pushq %rbx
	pushq %rbp
	pushq %r12
	pushq %r13
	pushq %r14
	pushq %r15
	call _go
	popq %r15
	popq %r14
	popq %r13
	popq %r12
	popq %rbp
	popq %rbx
	retq
_go:
	movq $5, %rdi
	movq $_testFunction_ret, -8(%rsp)
	subq $8, %rsp
	jmp _testFunction
_testFunction_ret:
	movq %rax, %rdi
	call print # runtime system call
	retq
_testFunction:
	movq %rdi, %rax
	retq
