	.text
	.file	"/home/galaxyvintage/Documents/CMPT/CMPT379/CMPT379/hw3/decafexpr/output/dev/llvm/charescapes.llvm.bc"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	pushq	%rax
.Ltmp0:
	.cfi_def_cfa_offset 16
	movl	$9, 4(%rsp)
	movl	$9, %edi
	callq	print_int
	movl	$11, 4(%rsp)
	movl	$11, %edi
	callq	print_int
	movl	$13, 4(%rsp)
	movl	$13, %edi
	callq	print_int
	movl	$10, 4(%rsp)
	movl	$10, %edi
	callq	print_int
	movl	$7, 4(%rsp)
	movl	$7, %edi
	callq	print_int
	movl	$12, 4(%rsp)
	movl	$12, %edi
	callq	print_int
	movl	$8, 4(%rsp)
	movl	$8, %edi
	callq	print_int
	movl	$92, 4(%rsp)
	movl	$92, %edi
	callq	print_int
	movl	$39, 4(%rsp)
	movl	$39, %edi
	callq	print_int
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc


	.section	".note.GNU-stack","",@progbits
