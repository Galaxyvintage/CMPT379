	.text
	.file	"/home/galaxyvintage/Documents/CMPT/CMPT379/CMPT379/hw3/decafexpr/output/test/llvm/2bool-blocks.llvm.bc"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	subq	$24, %rsp
.Ltmp0:
	.cfi_def_cfa_offset 32
	movb	$1, 23(%rsp)
	movb	$0, 7(%rsp)
	movl	$1, 12(%rsp)
	movl	$1, 8(%rsp)
	movl	12(%rsp), %edi
	callq	print_int
	movl	8(%rsp), %edi
	callq	print_int
	movzbl	23(%rsp), %edi
	andl	$1, %edi
	callq	print_int
	movzbl	7(%rsp), %edi
	andl	$1, %edi
	callq	print_int
	xorl	%eax, %eax
	addq	$24, %rsp
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc


	.section	".note.GNU-stack","",@progbits
