/* assembly to compute d2LHash1305 */

#include "d2LHash1305_macro.h"

	.p2align 5
	.globl d2LHash1305maax

d2LHash1305maax:

	movq 	%rsp,%r11
	andq    $-32,%rsp
	subq 	$208,%rsp

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

	imul	$16,%rcx,%rax
	addq	%rax,%rsi
	movq	%r8,0(%rsi)
	movq	$0,8(%rsi)

	xorq 	%rdx,%rdx
	movq	%rcx,%rax
	movq	$15,%rbx
	divq	%rbx

	movq	%rax,88(%rsp)
	movq	%rax,96(%rsp)
	movq	%rdx,104(%rsp)

	movq 	%r8,112(%rsp)

	movq	64(%rsp),%rsi

	movq    %rsi,%rcx

	cmpq	$0,%rax
	je	.L0

.LBRW:
	brw_add_block(0,0,%r8,%r11,%r12)
	brw_add_block(1,1,%r13,%r14,%r15)
	brw_mul()
	reduce_5limb(%r11,%r12,%r9,%r10)
	reduce_3limb(%r8,%r11,%r12)
	brw_add_block3(2,%r8,%r11,%r12)
	brw_add_block(3,3,%r13,%r14,%r15)
	brw_mul()
	brw_store_temp(120)

	brw_add_block(4,0,%r8,%r11,%r12)
	brw_add_block(5,1,%r13,%r14,%r15)
	brw_mul()
	brw_add_temp(120)
	reduce_5limb(%r11,%r12,%r9,%r10)
	reduce_3limb(%r8,%r11,%r12)
	brw_add_block3(6,%r8,%r11,%r12)
	brw_add_block(7,4,%r13,%r14,%r15)
	brw_mul()
	brw_store_temp(120)

	brw_add_block(8,0,%r8,%r11,%r12)
	brw_add_block(9,1,%r13,%r14,%r15)
	brw_mul()
	reduce_5limb(%r11,%r12,%r9,%r10)
	reduce_3limb(%r8,%r11,%r12)
	brw_add_block3(10,%r8,%r11,%r12)
	brw_add_block(11,3,%r13,%r14,%r15)
	brw_mul()
	brw_add_temp(120)
	brw_store_temp(120)

	brw_add_block(12,0,%r8,%r11,%r12)
	brw_add_block(13,1,%r13,%r14,%r15)
	brw_mul()
	brw_add_temp(120)
	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)
	brw_add_block3(14,%r8,%r9,%r10)
	brw_store_output()

	addq	$240,%rsi
	addq	$24,%rcx

	movq    88(%rsp),%rax
	subq    $1,%rax
	movq    %rax,88(%rsp)

	cmpq    $0,%rax
	jg      .LBRW

	movq	72(%rsp),%rdi
	movq	64(%rsp),%rsi
	movq	96(%rsp),%rcx

	addq	$120,%rdi

	cmpq    $1,%rcx
	je      .LB1

	movq    %rcx,88(%rsp)

	movq    $0,%r8
	movq    $0,%r9
	movq    $0,%r10
	movq    $0,%r11
	movq    $0,%r12

	cmpq    $2,%rcx
	je      .LB2

	cmpq    $3,%rcx
	je      .LB3

.LB4:
	mul_gamman(0,48)
	add_product()

	mul_gamman(24,24)
	add_product()

	mul_gamman(48,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block1(72)

	addq	$96,%rsi

	movq    88(%rsp),%rcx
	subq    $4,%rcx
	movq    %rcx,88(%rsp)

	cmpq    $0,%rcx
	je      .L1

	cmpq    $1,%rcx
	jg      .LT2

.LT1:
	mul_gammanr(0)

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block1(0)

	jmp     .L1

.LT2:
	cmpq    $2,%rcx
	jg      .LT3

	mul_gammanr(24)
	jmp     .LB2

.LT3:
	cmpq    $3,%rcx
	jg      .LT4

	mul_gammanr(48)
	jmp     .LB3

.LT4:
	mul_gammanr(72)
	jmp     .LB4

.LB1:
	movq	0(%rsi),%r8
	movq	8(%rsi),%r9
	movq	16(%rsi),%r10

	jmp     .L1

.LB2:
	mul_gamman(0,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block1(24)

	jmp     .L1

.LB3:
	mul_gamman(0,24)
	add_product()

	mul_gamman(24,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block1(48)

	jmp     .L1

.L0:
	movq	$0,%r8
	movq	$0,%r9
	movq	$0,%r10

.L1:
	movq	64(%rsp),%rsi
	movq	72(%rsp),%rdi
	movq	80(%rsp),%rbx
	movq	104(%rsp),%rax

	subq	%rax,%rbx
	imul	$16,%rbx,%rbx
	addq	%rbx,%rsi
	addq	$2,%rax

	movq	%rax,104(%rsp)

	cmpq	$2,%rax
	jg	.L3

	mul_taunr(0)

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(0)

	jmp     .LF

.L3:
	cmpq	$3,%rax
	jg	.L4

	mul_taunr(24)

	mul_taun(0,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(16)

	jmp     .LF

.L4:
	mul_taunr(48)

.LH4:
	mul_taun(0,24)
	add_product()

	mul_taun(16,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(32)

	addq	$48,%rsi
	movq	104(%rsp),%rax
	subq    $4,%rax
	movq	%rax,104(%rsp)

	cmpq    $0,%rax
	je      .LF

.LHT1:
	cmpq    $1,%rax
	jg      .LHT2

	mul_taunr(0)
	jmp	.LH1

.LHT2:
	cmpq    $2,%rax
	jg      .LHT3

	mul_taunr(24)
	jmp     .LH2

.LHT3:
	cmpq    $3,%rax
	jg      .LHT4

	mul_taunr(48)
	jmp     .LH3

.LHT4:
	mul_taunr(72)

	mul_taun(0,48)
	add_product()

	addq	$16,%rsi

	jmp     .LH4

.LH1:
	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(0)

	jmp     .LF

.LH2:
	mul_taun(0,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(16)

	jmp     .LF

.LH3:
	mul_taun(0,24)
	add_product()

	mul_taun(16,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(32)

.LF:
	mul_taunr(0)

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	reduce_3limb(%r8,%r9,%r10)
	make_unique()

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
