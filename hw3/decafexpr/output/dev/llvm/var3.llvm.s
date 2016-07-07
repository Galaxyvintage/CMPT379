	.text
	.file	"/home/galaxyvintage/Documents/CMPT/CMPT379/CMPT379/hw3/decafexpr/output/dev/llvm/var3.llvm.bc"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	subq	$24, %rsp
.Ltmp0:
	.cfi_def_cfa_offset 32
	movl	20(%rsp), %edi
	callq	print_int
	movl	16(%rsp), %edi
	callq	print_int
	movl	12(%rsp), %edi
	callq	print_int
	movl	$1, 20(%rsp)
	movl	$1, 16(%rsp)
	movl	$1, 12(%rsp)
	movl	20(%rsp), %edi
	callq	print_int
	movl	16(%rsp), %edi
	callq	print_int
	movl	12(%rsp), %edi
	callq	print_int
	xorl	%eax, %eax
	addq	$24, %rsp
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc


	.section	".note.GNU-stack","",@progbits
