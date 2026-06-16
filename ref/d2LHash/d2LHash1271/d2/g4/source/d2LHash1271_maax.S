/* assembly to compute d2LHash1271  */

#include "d2LHash1271_macro.h"

	.p2align 5
	.globl d2LHash1271maax

 d2LHash1271maax:

	movq 	%rsp,%r11
	andq    $-32,%rsp
	subq 	$120,%rsp

	movq 	%r11,0(%rsp)
	movq 	%r12,8(%rsp)
	movq 	%r13,16(%rsp)
	movq 	%r14,24(%rsp)
	movq 	%r15,32(%rsp)
	movq 	%rbx,40(%rsp)
	movq 	%rbp,48(%rsp)

	movq 	%rdi,56(%rsp)
	movq 	%rsi,64(%rsp)
	movq 	%rdx,72(%rsp)
	movq 	%rcx,80(%rsp)

	movq    %rdx,%rdi

	xorq 	%rdx,%rdx
	movq	%rcx,%rax
	movq	$3,%rbx
	divq	%rbx

	movq	%rax,88(%rsp)
	movq	%rdx,96(%rsp)
	movq 	%r8,104(%rsp)
	movq	%rax,112(%rsp)

	cmpq	$0,%rax
	je	.L0

	movq    %rsi,%rcx

.LBRW:
	brw_add_block(0,0,%r12,%r13)
	brw_add_block(1,1,%r14,%r15)
	brw_mul(%r12,%r13)
	reduce_4limb()
	reduce_2limb()
	brw_add_block3(2,%r8,%r9,%rax)
	brw_store_output()

	addq	$45,%rsi
	addq	$16,%rcx

	movq	112(%rsp),%rax
	subq    $1,%rax
	movq	%rax,112(%rsp)

	cmpq    $0,%rax
	jg      .LBRW

	movq	72(%rsp),%rdi
	movq	64(%rsp),%rsi
	movq	88(%rsp),%rcx

	addq	$48,%rdi

	cmpq    $1,%rcx
	je      .L1

	movq    $0,%r8
	movq    $0,%r9
	movq    $0,%r10
	movq    $0,%r11

	cmpq    $2,%rcx
	je      .LB2

	cmpq    $3,%rcx
	je      .LB3

.LB4:
	movq	$0,%r12

	mul_taun_brw(0,32)
	add_product2()

	mul_taun_brw(16,16)
	add_product2()

	mul_taun_brw(32,0)
	add_product2()

	reduce_5limb()
	reduce_4limb()
	reduce_2limb()

	add_msg_block(48)

	addq	$64,%rsi
	subq    $4,%rcx

	cmpq    $0,%rcx
	je      .L2

.LT1:
	cmpq    $1,%rcx
	jg      .LT2

	mul_taunr(0)
	jmp	.LB1

.LT2:
	cmpq    $2,%rcx
	jg      .LT3

	mul_taunr(16)
	jmp     .LB2

.LT3:
	cmpq    $3,%rcx
	jg      .LT4

	mul_taunr(32)
	jmp     .LB3

.LT4:
	mul_taunr(48)
	jmp     .LB4

.LB1:
	reduce_4limb()
	reduce_2limb()

	add_msg_block(0)

	jmp     .L2

.LB2:
	movq	$0,%r12
	mul_taun_brw(0,0)
	add_product2()

	reduce_5limb()
	reduce_4limb()
	reduce_2limb()

	add_msg_block(16)

	jmp     .L2

.LB3:
	movq	$0,%r12
	mul_taun_brw(0,16)
	add_product1()

	mul_taun_brw(16,0)
	add_product1()

	reduce_5limb()
	reduce_4limb()
	reduce_2limb()

	add_msg_block(32)

	jmp     .L2

.L0:
	movq	$0,%r8
	movq	$0,%r9

	jmp	.L2

.L1:
	movq	0(%rsi),%r8
	movq	8(%rsi),%r9

.L2:
	movq	96(%rsp),%rax

	cmpq	$0,%rax
	je	.L3

	cmpq	$1,%rax
	je	.L4

	cmpq	$2,%rax
	je	.L5

.L3:
	movq	72(%rsp),%rdi

	mul_taunr(16)

	mul_len_tau(104)
	add_product1()

	reduce_4limb()
	reduce_2limb()

	jmp	.LF

.L4:
	movq	72(%rsp),%rdi

	mul_taunr(32)

	movq	80(%rsp),%rax
	subq	96(%rsp),%rax
	imul	$15,%rax,%rax

	movq	64(%rsp),%rsi
	addq	%rax,%rsi

	mul_taun(0,16)
	add_product1()

	mul_len_tau(104)
	add_product1()

	reduce_4limb()
	reduce_2limb()

	jmp	.LF

.L5:
	movq	72(%rsp),%rdi

	mul_taunr(48)

	movq	80(%rsp),%rax
	subq	96(%rsp),%rax
	imul	$15,%rax,%rax

	movq	64(%rsp),%rsi
	addq	%rax,%rsi

	mul_taun(0,32)
	add_product1()

	mul_taun(15,16)
	add_product1()

	mul_len_tau(104)
	add_product1()

	reduce_4limb()
	reduce_2limb()

.LF:
	make_unique()

	andq	mask62(%rip),%r9
	movq 	56(%rsp),%rdi
	movq    %r8,0(%rdi)
	movq    %r9,8(%rdi)

	movq 	0(%rsp),%r11
	movq 	8(%rsp),%r12
	movq 	16(%rsp),%r13
	movq 	24(%rsp),%r14
	movq 	32(%rsp),%r15
	movq 	40(%rsp),%rbx
	movq 	48(%rsp),%rbp

	movq 	%r11,%rsp

	ret
