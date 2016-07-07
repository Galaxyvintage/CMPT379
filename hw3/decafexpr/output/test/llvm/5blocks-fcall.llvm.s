	.text
	.file	"/home/galaxyvintage/Documents/CMPT/CMPT379/CMPT379/hw3/decafexpr/output/test/llvm/5blocks-fcall.llvm.bc"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	subq	$24, %rsp
.Ltmp0:
	.cfi_def_cfa_offset 32
	movl	$1, 20(%rsp)
	movl	$100, 16(%rsp)
	movl	20(%rsp), %edi
	callq	print_int
	movl	16(%rsp), %edi
	callq	print_int
	movl	$2, 12(%rsp)
	movl	$2, %edi
	callq	print_int
	movl	16(%rsp), %edi
	callq	print_int
	movl	$3, 8(%rsp)
	movl	$3, %edi
	callq	print_int
	movl	16(%rsp), %edi
	callq	print_int
	movl	$4, 4(%rsp)
	movl	$4, %edi
	callq	print_int
	movl	16(%rsp), %edi
	callq	print_int
	movl	$5, (%rsp)
	movl	$5, %edi
	callq	print_int
	movl	16(%rsp), %edi
	callq	print_int
	xorl	%eax, %eax
	addq	$24, %rsp
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc


	.section	".note.GNU-stack","",@progbits
