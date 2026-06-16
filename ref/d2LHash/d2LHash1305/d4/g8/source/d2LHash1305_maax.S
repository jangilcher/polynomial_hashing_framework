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
	brw_add_block(7,7,%r13,%r14,%r15)
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

	addq	$240,%rsi

	brw_store_output()

	addq	$24,%rcx

	movq    88(%rsp),%rax
	subq    $1,%rax
	movq    %rax,88(%rsp)

	cmpq    $0,%rax
	jg      .LBRW

	movq	72(%rsp),%rdi
	movq	64(%rsp),%rsi
	movq	96(%rsp),%rcx

	addq	$192,%rdi

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

	cmpq    $4,%rcx
	je      .LB4

	cmpq    $5,%rcx
	je      .LB5

	cmpq    $6,%rcx
	je      .LB6

	cmpq    $7,%rcx
	je      .LB7

.LB8:
	mul_gamman(0,144)
	add_product()

	mul_gamman(24,120)
	add_product()

	mul_gamman(48,96)
	add_product()

	mul_gamman(72,72)
	add_product()

	mul_gamman(96,48)
	add_product()

	mul_gamman(120,24)
	add_product()

	mul_gamman(144,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block1(168)

	addq	$192,%rsi

	movq    88(%rsp),%rcx
	subq    $8,%rcx
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
	cmpq    $4,%rcx
	jg      .LT5

	mul_gammanr(72)
	jmp     .LB4

.LT5:
	cmpq    $5,%rcx
	jg      .LT6

	mul_gammanr(96)
	jmp     .LB5

.LT6:
	cmpq    $6,%rcx
	jg      .LT7

	mul_gammanr(120)
	jmp     .LB6

.LT7:
	cmpq    $7,%rcx
	jg      .LT8

	mul_gammanr(144)
	jmp     .LB7

.LT8:
	mul_gammanr(168)
	jmp     .LB8

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

	jmp     .L1

.LB5:
	mul_gamman(0,72)
	add_product()

	mul_gamman(24,48)
	add_product()

	mul_gamman(48,24)
	add_product()

	mul_gamman(72,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block1(96)

	jmp     .L1

.LB6:
	mul_gamman(0,96)
	add_product()

	mul_gamman(24,72)
	add_product()

	mul_gamman(48,48)
	add_product()

	mul_gamman(72,24)
	add_product()

	mul_gamman(96,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block1(120)

	jmp     .L1

.LB7:
	mul_gamman(0,120)
	add_product()

	mul_gamman(24,96)
	add_product()

	mul_gamman(48,72)
	add_product()

	mul_gamman(72,48)
	add_product()

	mul_gamman(96,24)
	add_product()

	mul_gamman(120,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block1(144)

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
	cmpq	$4,%rax
	jg	.L5

	mul_taunr(48)

	mul_taun(0,24)
	add_product()

	mul_taun(16,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(32)

	jmp	.LF

.L5:
	cmpq	$5,%rax
	jg	.L6

	mul_taunr(72)

	mul_taun(0,48)
	add_product()

	mul_taun(16,24)
	add_product()

	mul_taun(32,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(48)

	jmp     .LF

.L6:
	cmpq	$6,%rax
	jg	.L7

	mul_taunr(96)

	mul_taun(0,72)
	add_product()

	mul_taun(16,48)
	add_product()

	mul_taun(32,24)
	add_product()

	mul_taun(48,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(64)

	jmp     .LF

.L7:
	cmpq	$7,%rax
	jg	.L8

	mul_taunr(120)

	mul_taun(0,96)
	add_product()

	mul_taun(16,72)
	add_product()

	mul_taun(32,48)
	add_product()

	mul_taun(48,24)
	add_product()

	mul_taun(64,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(80)

	jmp     .LF

.L8:
	mul_taunr(144)

.LH8:
	mul_taun(0,120)
	add_product()

	mul_taun(16,96)
	add_product()

	mul_taun(32,72)
	add_product()

	mul_taun(48,48)
	add_product()

	mul_taun(64,24)
	add_product()

	mul_taun(80,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(96)

	addq	$112,%rsi
	movq	104(%rsp),%rax
	subq    $8,%rax
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
	cmpq    $4,%rax
	jg      .LHT5

	mul_taunr(72)
	jmp     .LH4

.LHT5:
	cmpq    $5,%rax
	jg      .LHT6

	mul_taunr(96)
	jmp     .LH5

.LHT6:
	cmpq    $6,%rax
	jg      .LHT7

	mul_taunr(120)
	jmp     .LH6

.LHT7:
	cmpq    $7,%rax
	jg      .LHT8

	mul_taunr(144)
	jmp     .LH7

.LHT8:
	mul_taunr(168)

	mul_taun(0,144)
	add_product()

	addq	$16,%rsi

	jmp     .LH8

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

	jmp     .LF

.LH4:
	mul_taun(0,48)
	add_product()

	mul_taun(16,24)
	add_product()

	mul_taun(32,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(48)

	jmp     .LF

.LH5:
	mul_taun(0,72)
	add_product()

	mul_taun(16,48)
	add_product()

	mul_taun(32,24)
	add_product()

	mul_taun(48,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(64)

	jmp     .LF

.LH6:
	mul_taun(0,96)
	add_product()

	mul_taun(16,72)
	add_product()

	mul_taun(32,48)
	add_product()

	mul_taun(48,24)
	add_product()

	mul_taun(64,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(80)

	jmp     .LF

.LH7:
	mul_taun(0,120)
	add_product()

	mul_taun(16,96)
	add_product()

	mul_taun(32,72)
	add_product()

	mul_taun(48,48)
	add_product()

	mul_taun(64,24)
	add_product()

	mul_taun(80,0)
	add_product()

	reduce_5limb(%r9,%r10,%r11,%r12)
	reduce_3limb(%r8,%r9,%r10)

	add_msg_block2(96)

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
