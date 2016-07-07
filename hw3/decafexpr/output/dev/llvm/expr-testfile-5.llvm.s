	.text
	.file	"/home/galaxyvintage/Documents/CMPT/CMPT379/CMPT379/hw3/decafexpr/output/dev/llvm/expr-testfile-5.llvm.bc"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	pushq	%rax
.Ltmp0:
	.cfi_def_cfa_offset 16
	movb	$1, 7(%rsp)
	movb	$1, 6(%rsp)
	movb	7(%rsp), %al
	andb	$1, %al
	movb	%al, 5(%rsp)
	movzbl	5(%rsp), %edi
	andl	$1, %edi
	callq	print_int
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc


	.section	".note.GNU-stack","",@progbits
