/* assembly to compute the poly1305 hash function using Horner's method */

	.p2align 5
	.globl poly1305maax

poly1305maax:

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

	/* key = (r15 : r14) */
	movq    0(%rdx),%r14
	movq    8(%rdx),%r15

	/* message block = (rax : r13) */
	movq    0(%rsi),%r13
	movq    8(%rsi),%rax

	/* handle messages having a single block */
	cmpq    $1,%rcx
	je      .L3

	/* else loop around and multiply the 129-byte (3-limb)
	 * message block with the 128-byte (2-limb) key;
	 * read the 129th bit in %rdi before proceeding
	 */

	movq    $1,%rdi
	/* message block = (rdi : rax: r13) */
.L1:
	/* integer multiplication */
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r13,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	xorq    %r12,%r12
	movq    %rax,%rdx

	mulx    %r14,%rax,%rbp
	adcx    %r9,%rax
	adox    %rbp,%r10

	mulx    %r15,%rbx,%rbp
	adcx    %rbx,%r10
	adox    %rbp,%r11
	adcx    %r12,%r11

	movq    %rdi,%rdx
	xorq    %rdi,%rdi

	mulx    %r14,%rbx,%rbp
	adcx    %rbx,%r10
	adox    %rbp,%r11

	mulx    %r15,%rbx,%rbp
	adcx    %rbx,%r11
	adox    %rbp,%r12
	adcx    %rdi,%r12

	/* reduction on the integer product (r12 : r11 : r10 : rax : r13) */
	movq    %r10,%rdi

	andq    mask2(%rip),%rdi
	andq    mask2c(%rip),%r10

	addq    %r10,%r13
	adcq    %r11,%rax
	adcq    %r12,%rdi

	shrd    $2,%r11,%r10
	shrd    $2,%r12,%r11
	shrq    $2,%r12

	addq    %r10,%r13
	adcq    %r11,%rax
	adcq    %r12,%rdi

	movq    %rdi,%r11
	andq    mask2(%rip),%rdi

	shrq    $2,%r11

	imul    $5,%r11,%r11
	addq    %r11,%r13
	adcq    $0,%rax
	adcq    $0,%rdi
	/* at this point the partially reduced field element stored in (rdi : r9 : r13) */

	/* advance the message pointer*/
	addq    $16,%rsi

	/* add the next message block */
	addq    0(%rsi),%r13
	adcq    8(%rsi),%rax
	adcq    $1,%rdi

	/* at this point the next field element to be multiplied  with the key is stored
	 * in (rdi : rax : r13) and we are fine to go with the next iteration
	 */

	subq    $1,%rcx
	cmpq    $1,%rcx

	jg      .L1

	/* if the last block is full the we are fine to go with the final multiplication */
	cmpq    $1,64(%rsp)
	je      .L2

	/* else subtract the added 1 before the final multiplication as 1 is already appended
	 * at the end of the message for the partial block
	 */
	subq    $1,%rdi

.L2:
	/* integer multiplication */
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
	movq    %rdi,%rdx

	mulx    %r14,%rbx,%rbp
	adcx    %rbx,%r10
	adox    %rbp,%r11

	mulx    %r15,%rbx,%rbp
	adcx    %rbx,%r11
	adox    %rbp,%r12
	adcx    %rax,%r12

	/* reduction on the integer product (r12 : r11 : r10 : r9 : r8) */
	movq    %r10,%rbx

	andq    mask2(%rip),%r10
	andq    mask2c(%rip),%rbx

	addq    %rbx,%r8
	adcq    %r11,%r9
	adcq    %r12,%r10

	shrd    $2,%r11,%rbx
	shrd    $2,%r12,%r11
	shrq    $2,%r12

	addq    %rbx,%r8
	adcq    %r11,%r9
	adcq    %r12,%r10

	movq    %r10,%r11
	andq    mask2(%rip),%r10
	shrq    $2,%r11

	imul    $5,%r11,%r11
	addq    %r11,%r8
	adcq    $0,%r9
	adcq    $0,%r10

	/* jump to finalize the computation */
	jmp     .L6

.L3:
	/* if the single message block is full */
	cmpq    $1,64(%rsp)
	je      .L5

	/* if the single message block is partial and its size is between 1 to 64 bits */
	cmpq    $8,72(%rsp)
	jle     .L4

	/* if the single message block is partial and its size is between 65 to 128 bits */

	/* integer multiplication */
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

	/* reduction on the integer product (r11 : r10 : r9 : r8) */
	movq    %r10,%rbx

	andq    mask2(%rip),%r10
	andq    mask2c(%rip),%rbx

	addq    %rbx,%r8
	adcq    %r11,%r9
	adcq    %r12,%r10

	shrd    $2,%r11,%rbx
	shrq    $2,%r11

	addq    %rbx,%r8
	adcq    %r11,%r9
	adcq    %r12,%r10

	/* jump to finalize the computation */
	jmp     .L6

.L4:
	/* integer multiplication */
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	/* reduction on the integer product (r10 : r9 : r8) */
	movq    %r10,%r11
	andq    mask2(%rip),%r10
	shrq    $2,%r11

	imul    $5,%r11,%r11
	addq    %r11,%r8
	adcq    $0,%r9
	adcq    $0,%r10

	/* jump to finalize the computation */
	jmp     .L6

.L5:
	/* integer multiplication */
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
	adox    %rax,%r11
	adcx    %r15,%r11
	adox    %rax,%r12

	/* reduction on the integer product (r12 : r11 : r10 : r9 : r8) */
	movq    %r10,%rbx

	andq    mask2(%rip),%r10
	andq    mask2c(%rip),%rbx

	addq    %rbx,%r8
	adcq    %r11,%r9
	adcq    %r12,%r10

	shrd    $2,%r11,%rbx
	shrd    $2,%r12,%r11
	shrq    $2,%r12

	addq    %rbx,%r8
	adcq    %r11,%r9
	adcq    %r12,%r10

.L6:
	/* final reduction on (r10 : r9 : r8) */
	movq    %r10,%r11
	shrq    $2,%r11
	andq	mask2(%rip),%r10

	imul    $5,%r11,%r11
	addq    %r11,%r8
	adcq    $0,%r9
	adcq    $0,%r10

	/* make unique on the fully reduced field element (r10 : r9 : r8) */
	movq    %r8,%r11
	movq    %r9,%r12
	movq    %r10,%r13

	subq    p0(%rip),%r8
	sbbq    p1(%rip),%r9
	sbbq    p2(%rip),%r10

	movq    %r10,%rcx
	shlq    $62,%rcx

	cmovc   %r11,%r8
	cmovc   %r12,%r9
	cmovc   %r13,%r10

	/* store only first 128 bytes of the result */
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
