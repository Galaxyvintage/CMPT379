	.text
	.file	"/home/galaxyvintage/Documents/CMPT/CMPT379/CMPT379/hw3/decafexpr/output/dev/llvm/charvals.llvm.bc"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	pushq	%rax
.Ltmp0:
	.cfi_def_cfa_offset 16
	movl	$97, %edi
	callq	print_int
	movl	$109, %edi
	callq	print_int
	movl	$122, %edi
	callq	print_int
	movl	$65, %edi
	callq	print_int
	movl	$77, %edi
	callq	print_int
	movl	$90, %edi
	callq	print_int
	movl	$48, %edi
	callq	print_int
	movl	$55, %edi
	callq	print_int
	movl	$57, %edi
	callq	print_int
	movl	$45, %edi
	callq	print_int
	movl	$125, %edi
	callq	print_int
	movl	$34, %edi
	callq	print_int
	movl	$94, %edi
	callq	print_int
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc


	.section	".note.GNU-stack","",@progbits
