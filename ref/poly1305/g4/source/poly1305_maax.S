/* assembly to compute the poly1305 hash function using
 * Horner's method with the help of precomputed key powers
 */

#include "poly1305_macro.h"

	.p2align 5
	.globl poly1305maax_precomputed_keypowers

poly1305maax_precomputed_keypowers:

	movq 	%rsp,%r11
	andq    $-32,%rsp
	subq 	$104,%rsp

	movq 	%r11,0(%rsp)
	movq 	%r12,8(%rsp)
	movq 	%r13,16(%rsp)
	movq 	%r14,24(%rsp)
	movq 	%r15,32(%rsp)
	movq 	%rbx,40(%rsp)
	movq 	%rbp,48(%rsp)
	movq 	%rdi,56(%rsp)
	movq 	%rcx,64(%rsp)
	movq 	%r8,72(%rsp)
	movq 	%r9,80(%rsp)

	cmpq    $1,%rcx
	je      .LB1

	movq    %rcx,88(%rsp)
	movq	$1,96(%rsp)

	movq    %rdx,%rdi

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
	mul_taun(0,48)
	add_product()

	mul_taun(16,24)
	add_product()

	mul_tau(32,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(48,96)

	addq	$64,%rsi

	movq    88(%rsp),%rcx
	subq    $4,%rcx
	movq    %rcx,88(%rsp)

	cmpq    $0,%rcx
	je      .LB0

	cmpq    $1,%rcx
	jne     .LT2

.LT1:
	mul_taur(0)

	reduce_5limb()
	reduce_3limb()

	add_msg_block(0,72)

	jmp     .LB0

.LT2:
	cmpq    $2,%rcx
	jg      .LT3

	mul_taunr(24)
	jmp     .LB2

.LT3:
	cmpq    $3,%rcx
	jg      .LT4

	mul_taunr(48)
	jmp     .LB3

.LT4:
	mul_taunr(72)
	jmp     .LB4

.LB2:
	mul_tau(0,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(16,72)

	jmp     .LB0

.LB3:
	mul_taun(0,24)
	add_product()

	mul_tau(16,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(32,72)

.LB0:
	xorq 	%rdx,%rdx
	movq	64(%rsp),%rax
	movq	$8,%rcx
	divq	%rcx

	cmpq	$0,%rdx
	jg	.L0

	/* if the last block is full */
	cmpq	$0,72(%rsp)
	jg	.L0

	/* if the last block is partial */
	subq	96(%rsp),%r10

.L0:
	mul_taur(0)

	reduce_5limb()
	reduce_3limb()

	jmp	.LF

.LB1:
	movq    0(%rdx),%r14
	movq    8(%rdx),%r15

	movq    0(%rsi),%r13
	movq    8(%rsi),%rax

	cmpq    $1,72(%rsp)
	je      .L2

	cmpq    $8,80(%rsp)
	jle     .L1

	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	xorq    %r12,%r12
	movq    %rax,%rdx

	mulx    %r14,%rbx,%rbp
	adcx    %rbx,%r9
	adox    %rbp,%r10

	mulx    %r15,%rbx,%rbp
	adcx    %rbx,%r10
	adox    %rbp,%r11
	adcx    %r12,%r11

	reduce_4limb()

	jmp     .LF

.L1:
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	reduce_3limb()

	jmp     .LF

.L2:
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	xorq    %r12,%r12
	movq    %rax,%rdx

	mulx    %r14,%rbx,%rbp
	adcx    %rbx,%r9
	adox    %rbp,%r10

	mulx    %r15,%rbx,%rbp
	adcx    %rbx,%r10
	adox    %rbp,%r11
	adcx    %r12,%r11

	xorq    %rax,%rax

	adcx    %r14,%r10
	adox    %r15,%r11
	adox    %rax,%r12

	reduce_5limb()

.LF:
	reduce_3limb()

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
