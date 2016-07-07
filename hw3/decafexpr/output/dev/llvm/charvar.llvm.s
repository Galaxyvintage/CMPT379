	.text
	.file	"/home/galaxyvintage/Documents/CMPT/CMPT379/CMPT379/hw3/decafexpr/output/dev/llvm/charvar.llvm.bc"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	pushq	%rax
.Ltmp0:
	.cfi_def_cfa_offset 16
	movl	$102, 4(%rsp)
	movl	$97, (%rsp)
	movl	4(%rsp), %edi
	addl	$-97, %edi
	callq	print_int
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc


	.section	".note.GNU-stack","",@progbits
