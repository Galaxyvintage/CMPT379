	.text
	.file	"/home/galaxyvintage/Documents/CMPT/CMPT379/CMPT379/hw3/decafexpr/output/dev/llvm/intconst-var.llvm.bc"
	.globl	main
	.align	16, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# BB#0:                                 # %entry
	pushq	%rax
.Ltmp0:
	.cfi_def_cfa_offset 16
	movl	$0, 4(%rsp)
	xorl	%edi, %edi
	callq	print_int
	movl	$12345, 4(%rsp)         # imm = 0x3039
	movl	$12345, %edi            # imm = 0x3039
	callq	print_int
	movl	$-12345, 4(%rsp)        # imm = 0xFFFFFFFFFFFFCFC7
	movl	$-12345, %edi           # imm = 0xFFFFFFFFFFFFCFC7
	callq	print_int
	xorl	%eax, %eax
	popq	%rcx
	retq
.Lfunc_end0:
	.size	main, .Lfunc_end0-main
	.cfi_endproc


	.section	".note.GNU-stack","",@progbits
