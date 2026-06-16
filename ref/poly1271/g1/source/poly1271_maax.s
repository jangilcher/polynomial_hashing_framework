/* assembly to compute the poly1271 hash function using Horner's method */

	.p2align 5
	.globl poly1271maax

poly1271maax:

	movq 	%rsp,%r11
	andq    $-32,%rsp
	subq 	$84,%rsp

	movq 	%r11,0(%rsp)
	movq 	%r12,8(%rsp)
	movq 	%r13,16(%rsp)
	movq 	%r14,24(%rsp)
	movq 	%r15,32(%rsp)
	movq 	%rbx,40(%rsp)
	movq 	%rbp,48(%rsp)
	movq 	%rdi,56(%rsp)
	movq 	%r8,64(%rsp)
	movq 	%r9,72(%rsp)

	movq    0(%rdx),%r14
	movq    8(%rdx),%r15

	movq    0(%rsi),%r13
	movq    8(%rsi),%rax

	cmpq    $1,%rcx
	je      .L3

	andq    mask56(%rip),%rax
	orq     c(%rip),%rax

.L1:
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

	shld    $1,%r10,%r11
	shld    $1,%r9,%r10

	andq	mask63(%rip),%r9
	addq    %r10,%r8
	adcq    %r11,%r9

	movq    %r9,%r13
	shrq    $63,%r13
	andq	mask63(%rip),%r9
	addq    %r8,%r13
	adcq    $0,%r9

	addq    $15,%rsi

	movq    8(%rsi),%rax
	andq    mask56(%rip),%rax
	orq     c(%rip),%rax

	addq    0(%rsi),%r13
	adcq    %r9,%rax

	subq    $1,%rcx
	cmpq    $1,%rcx

	jg      .L1

	cmpq    $1,64(%rsp)
	je      .L2

	movq    0(%rsi),%r10
	movq    8(%rsi),%r11
	movq    %r11,%r12
	andq    mask56(%rip),%r11
	orq     c(%rip),%r11

	subq    %r10,%r13
	sbbq    %r11,%rax

	addq    %r10,%r13
	adcq    %r12,%rax

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

	shld    $1,%r10,%r11
	shld    $1,%r9,%r10

	andq	mask63(%rip),%r9
	addq    %r10,%r8
	adcq    %r11,%r9

	movq    %r9,%r10
	shrq    $63,%r10
	andq	mask63(%rip),%r9
	addq    %r10,%r8
	adcq    $0,%r9

	jmp     .L6

.L3:
	cmpq    $8,72(%rsp)
	jle     .L5

	cmpq    $0,64(%rsp)
	je      .L4

	andq    mask56(%rip),%rax
	orq     c(%rip),%rax

.L4:
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

	shld    $1,%r10,%r11
	shld    $1,%r9,%r10

	andq	mask63(%rip),%r9
	addq    %r10,%r8
	adcq    %r11,%r9

	movq    %r9,%r10
	shrq    $63,%r10
	andq	mask63(%rip),%r9
	addq    %r10,%r8
	adcq    $0,%r9

	jmp     .L6

.L5:
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	shld    $1,%r10,%r11
	shld    $1,%r9,%r10

	andq	mask63(%rip),%r9
	addq    %r10,%r8
	adcq    %r11,%r9

	movq    %r9,%r10
	shrq    $63,%r10
	andq	mask63(%rip),%r9
	addq    %r10,%r8
	adcq    $0,%r9

.L6:
	movq    %r8,%r11
	movq    %r9,%r12

	subq    p0(%rip),%r8
	sbbq    p1(%rip),%r9

	movq    %r9,%rcx
	shlq    $1,%rcx

	cmovc   %r11,%r8
	cmovc   %r12,%r9

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
