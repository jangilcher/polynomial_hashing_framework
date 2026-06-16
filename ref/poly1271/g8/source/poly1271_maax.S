/* assembly to compute the poly1271 hash function using
 * Horner's method with the help of  precomputed key powers
 */

#include "poly1271_macro.h"

	.p2align 5
	.globl poly1271maax_precomputed_keypowers

poly1271maax_precomputed_keypowers:

	movq 	%rsp,%r11
	andq    $-32,%rsp
	subq 	$72,%rsp

	movq 	%r11,0(%rsp)
	movq 	%r12,8(%rsp)
	movq 	%r13,16(%rsp)
	movq 	%r14,24(%rsp)
	movq 	%r15,32(%rsp)
	movq 	%rbx,40(%rsp)
	movq 	%rbp,48(%rsp)

	movq 	%rdi,56(%rsp)
	movq 	%r8,64(%rsp)
	movq	%r9,%rax

	cmpq    $1,%rcx
	je      .L1

	movq    %rdx,%rdi

	movq    $0,%r8
	movq    $0,%r9
	movq    $0,%r10
	movq    $0,%r11

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
	mul_taun(0,96)
	add_product()

	mul_taun(15,80)
	add_product()

	mul_taun(30,64)
	add_product()

	mul_taun(45,48)
	add_product()

	mul_taun(60,32)
	add_product()

	mul_taun(75,16)
	add_product()

	mul_taun(90,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(105)

	addq	$120,%rsi
	subq    $8,%rcx

	cmpq    $0,%rcx
	je      .LB0

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
	cmpq    $4,%rcx
	jg      .LT5

	mul_taunr(48)
	jmp     .LB4

.LT5:
	cmpq    $5,%rcx
	jg      .LT6

	mul_taunr(64)
	jmp     .LB5

.LT6:
	cmpq    $6,%rcx
	jg      .LT7

	mul_taunr(80)
	jmp     .LB6

.LT7:
	cmpq    $7,%rcx
	jg      .LT8

	mul_taunr(96)
	jmp     .LB7

.LT8:
	mul_taunr(112)
	jmp     .LB8

.LB1:
	reduce_4limb()
	reduce_2limb()

	add_msg_block(0)

	addq	$15,%rsi

	jmp     .LB0

.LB2:
	mul_taun(0,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(15)

	addq	$30,%rsi

	jmp     .LB0

.LB3:
	mul_taun(0,16)
	add_product()

	mul_taun(15,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(30)

	addq	$45,%rsi

	jmp     .LB0

.LB4:
	mul_taun(0,32)
	add_product()

	mul_taun(15,16)
	add_product()

	mul_taun(30,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(45)

	addq	$60,%rsi

	jmp     .LB0

.LB5:
	mul_taun(0,48)
	add_product()

	mul_taun(15,32)
	add_product()

	mul_taun(30,16)
	add_product()

	mul_taun(45,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(60)

	addq	$75,%rsi

	jmp     .LB0

.LB6:
	mul_taun(0,64)
	add_product()

	mul_taun(15,48)
	add_product()

	mul_taun(30,32)
	add_product()

	mul_taun(45,16)
	add_product()

	mul_taun(60,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(75)

	addq	$90,%rsi

	jmp     .LB0

.LB7:
	mul_taun(0,80)
	add_product()

	mul_taun(15,64)
	add_product()

	mul_taun(30,48)
	add_product()

	mul_taun(45,32)
	add_product()

	mul_taun(60,16)
	add_product()

	mul_taun(75,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(90)

	addq	$105,%rsi

	jmp     .LB0

.LB0:
	cmpq	$0,64(%rsp)
	jg	.L0

	subq	$15,%rsi
	movq    0(%rsi),%r10
	movq    8(%rsi),%r11
	movq    %r11,%r12
	andq    mask56(%rip),%r11
	orq     c(%rip),%r11

	subq    %r10,%r8
	sbbq    %r11,%r9

	addq    %r10,%r8
	adcq    %r12,%r9

.L0:
	mul_taunr(0)

	reduce_4limb()
	reduce_2limb()

	jmp	.LF

.L1:
	movq    0(%rdx),%r14
	movq    8(%rdx),%r15

	movq    0(%rsi),%r13
	movq    8(%rsi),%r12

	cmpq    $8,%rax
	jle     .L3

	cmpq    $0,64(%rsp)
	je      .L2

	andq    mask56(%rip),%r12
	orq     c(%rip),%r12

.L2:
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	movq    %r12,%rdx
	xorq    %r12,%r12

	mulx    %r14,%rbx,%rbp
	adcx    %rbx,%r9
	adox    %rbp,%r10

	mulx    %r15,%rbx,%rbp
	adcx    %rbx,%r10
	adox    %rbp,%r11
	adcx    %r12,%r11

	reduce_4limb()
	reduce_2limb()

	jmp     .LF

.L3:
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	reduce_4limb()
	reduce_2limb()

.LF:
	make_unique()

	andq    mask62(%rip),%r9
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
