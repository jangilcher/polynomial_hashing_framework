#ifndef __POLY1305ASM__
#define __POLY1305ASM__

/* macros used in the main assembly */

/* multiplies a message block with tau^n (n>1);
 * message block is accessed by %rsi with a
 * non-negative offset of m bytes;
 * tau^n is accessed by %rdi with a
 * non-negative offset of t bytes;
 * product is stored in (%rcx,%rax,%r15,%r14,%r13)
 */
#define mul_taun(m, t)                                                         \
                                                                               \
    xorq % rcx, % rcx;                                                         \
    movq m + 0(% rsi), % rdx;                                                  \
                                                                               \
    mulx t + 0(% rdi), % r13, % r14;                                           \
    mulx t + 8(% rdi), % rbx, % r15;                                           \
    adcx % rbx, % r14;                                                         \
                                                                               \
    mulx t + 16(% rdi), % rbx, % rax;                                          \
    adcx % rbx, % r15;                                                         \
    adcx % rcx, % rax;                                                         \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq m + 8(% rsi), % rdx;                                                  \
                                                                               \
    mulx t + 0(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r14;                                                         \
    adox % rbp, % r15;                                                         \
                                                                               \
    mulx t + 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r15;                                                         \
    adox % rbp, % rax;                                                         \
                                                                               \
    mulx t + 16(% rdi), % rbx, % rbp;                                          \
    adcx % rbx, % rax;                                                         \
    adox % rbp, % rcx;                                                         \
    adcx zero(% rip), % rcx;                                                   \
                                                                               \
    xorq % rdx, % rdx;                                                         \
                                                                               \
    adcx t + 0(% rdi), % r15;                                                  \
    adox zero(% rip), % rax;                                                   \
    adcx t + 8(% rdi), % rax;                                                  \
    adox zero(% rip), % rcx;                                                   \
    adcx t + 16(% rdi), % rcx;

/* multiplies a field element with tau^n (n>1);
 * field element is represented by (%r10,%r9,%r8);
 * tau^n is accessed by %rdi with a
 * non-negative offset of t bytes;
 * product is stored in (%r12,%r11,%r10,%r9,%r8)
 */
#define mul_taunr(t)                                                           \
                                                                               \
    xorq % r15, % r15;                                                         \
    movq % r8, % rdx;                                                          \
                                                                               \
    mulx t + 0(% rdi), % r8, % r12;                                            \
    mulx t + 8(% rdi), % rbx, % r13;                                           \
    adcx % rbx, % r12;                                                         \
    adcx % r15, % r13;                                                         \
                                                                               \
    mulx t + 16(% rdi), % rbx, % r14;                                          \
    adcx % rbx, % r13;                                                         \
    adcx % r15, % r14;                                                         \
                                                                               \
    xorq % rax, % rax;                                                         \
    movq % r9, % rdx;                                                          \
                                                                               \
    mulx t + 0(% rdi), % r9, % rbp;                                            \
    adcx % r12, % r9;                                                          \
    adox % rbp, % r13;                                                         \
                                                                               \
    mulx t + 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r13;                                                         \
    adox % rbp, % r14;                                                         \
                                                                               \
    mulx t + 16(% rdi), % rbx, % rbp;                                          \
    adcx % rbx, % r14;                                                         \
    adox % rbp, % r15;                                                         \
    adcx % rax, % r15;                                                         \
                                                                               \
    xorq % rax, % rax;                                                         \
    movq % r10, % rdx;                                                         \
                                                                               \
    mulx t + 0(% rdi), % r10, % rbp;                                           \
    adcx % r13, % r10;                                                         \
    adox % rbp, % r14;                                                         \
                                                                               \
    mulx t + 8(% rdi), % r11, % rbp;                                           \
    adcx % r14, % r11;                                                         \
    adox % rbp, % r15;                                                         \
                                                                               \
    mulx t + 16(% rdi), % r12, % rbp;                                          \
    adcx % r15, % r12;

/* multiplies a message block with tau;
 * message block is accessed by %rsi with a
 * non-negative offset of m bytes;
 * tau^n is accessed by %rdi with a
 * non-negative offset of t bytes;
 * product is stored in (%rcx,%rax,%r15,%r14,%r13)
 */
#define mul_tau(m, t)                                                          \
                                                                               \
    xorq % rax, % rax;                                                         \
    movq m + 0(% rsi), % rdx;                                                  \
                                                                               \
    mulx t + 0(% rdi), % r13, % r14;                                           \
    mulx t + 8(% rdi), % rbx, % r15;                                           \
    adcx % rbx, % r14;                                                         \
    adcx % rax, % r15;                                                         \
                                                                               \
    xorq % rcx, % rcx;                                                         \
    movq m + 8(% rsi), % rdx;                                                  \
                                                                               \
    mulx t + 0(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r14;                                                         \
    adox % rbp, % r15;                                                         \
                                                                               \
    mulx t + 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r15;                                                         \
    adox % rbp, % rax;                                                         \
    adcx % rcx, % rax;                                                         \
                                                                               \
    xorq % rdx, % rdx;                                                         \
                                                                               \
    adcx t + 0(% rdi), % r15;                                                  \
    adox % rdx, % rax;                                                         \
    adcx t + 8(% rdi), % rax;                                                  \
    adox % rdx, % rcx;

/* multiplies a field element with tau;
 * field element is represented by (%r10,%r9,%r8);
 * tau is accessed by %rdi with a
 * non-negative offset of t bytes;
 * product is stored in (%r12,%r11,%r10,%r9,%r8)
 */
#define mul_taur(t)                                                            \
                                                                               \
    xorq % r14, % r14;                                                         \
    movq % r8, % rdx;                                                          \
                                                                               \
    mulx t + 0(% rdi), % r8, % r12;                                            \
    mulx t + 8(% rdi), % rbx, % r13;                                           \
    adcx % rbx, % r12;                                                         \
    adcx % r14, % r13;                                                         \
                                                                               \
    xorq % rax, % rax;                                                         \
    movq % r9, % rdx;                                                          \
                                                                               \
    mulx t + 0(% rdi), % r9, % rbp;                                            \
    adcx % r12, % r9;                                                          \
    adox % rbp, % r13;                                                         \
                                                                               \
    mulx t + 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r13;                                                         \
    adox % rbp, % r14;                                                         \
    adcx % rax, % r14;                                                         \
                                                                               \
    xorq % r12, % r12;                                                         \
    movq % r10, % rdx;                                                         \
                                                                               \
    mulx t + 0(% rdi), % r10, % rbp;                                           \
    adcx % r13, % r10;                                                         \
    adox % rbp, % r14;                                                         \
                                                                               \
    mulx t + 8(% rdi), % r11, % rbp;                                           \
    adcx % r14, % r11;                                                         \
    adox % rbp, % r12;                                                         \
    adcx % rax, % r12;

/* adds two (unreduced) field elements
 * (%rcx,%rax,%r15,%r14,%r13) and
 * (%r12,%r11,%r10,%r9,%r8) and stores
 * in (%r12,%r11,%r10,%r9,%r8)
 */
#define add_product()                                                          \
                                                                               \
    xorq % rdx, % rdx;                                                         \
                                                                               \
    adcx % r13, % r8;                                                          \
    adcx % r14, % r9;                                                          \
    adcx % r15, % r10;                                                         \
    adcx % rax, % r11;                                                         \
    adcx % rcx, % r12;

/* add a message block to the field element
 * represented by (%r10,%r9,%r8);
 * message block is accessed by %rsi
 * with a non-negative offset m;
 * the last limb of the message block is
 * accessed by %rsp with a non-negative
 * offset t;
 * output is stored in (%r10,%r9,%r8)
 */
#define add_msg_block(m, t)                                                    \
                                                                               \
    addq m + 0(% rsi), % r8;                                                   \
    adcq m + 8(% rsi), % r9;                                                   \
    adcq t + 0(% rsp), % r10;

/* reduction on (%r12,%r11,%r10,%r9,%r8);
 * reduced element is (%r10,%r9,%r8);
 */
#define reduce_5limb()                                                         \
                                                                               \
    movq % r10, % r13;                                                         \
    andq mask2(% rip), % r10;                                                  \
    andq mask2c(% rip), % r13;                                                 \
                                                                               \
    addq % r13, % r8;                                                          \
    adcq % r11, % r9;                                                          \
    adcq % r12, % r10;                                                         \
                                                                               \
    shrd $2, % r11, % r13;                                                     \
    shrd $2, % r12, % r11;                                                     \
    shrq $2, % r12;                                                            \
                                                                               \
    addq % r13, % r8;                                                          \
    adcq % r11, % r9;                                                          \
    adcq % r12, % r10;

/* reduction on (%r11,%r10,%r9,%r8);
 * reduced element is (%r10,%r9,%r8);
 * the reduction assumes %r12 holds 0
 */
#define reduce_4limb()                                                         \
                                                                               \
    movq % r10, % r13;                                                         \
                                                                               \
    andq mask2(% rip), % r10;                                                  \
    andq mask2c(% rip), % r13;                                                 \
                                                                               \
    addq % r13, % r8;                                                          \
    adcq % r11, % r9;                                                          \
    adcq % r12, % r10;                                                         \
                                                                               \
    shrd $2, % r11, % r13;                                                     \
    shrq $2, % r11;                                                            \
                                                                               \
    addq % r13, % r8;                                                          \
    adcq % r11, % r9;                                                          \
    adcq % r12, % r10;

/* reduction on (r10,%r9,%r8);
 * reduced element is (%r10,%r9,%r8)
 */
#define reduce_3limb()                                                         \
                                                                               \
    movq % r10, % r11;                                                         \
    andq mask2(% rip), % r10;                                                  \
    shrq $2, % r11;                                                            \
                                                                               \
    imul $5, % r11, % r11;                                                     \
    addq % r11, % r8;                                                          \
    adcq $0, % r9;                                                             \
    adcq $0, % r10;

/* get unique representation of the field
 * element stored in (r10,%r9,%r8);
 * output is in (%r10,%r9,%r8)
 */
#define make_unique()                                                          \
                                                                               \
    movq % r8, % r11;                                                          \
    movq % r9, % r12;                                                          \
    movq % r10, % r13;                                                         \
                                                                               \
    subq p0(% rip), % r8;                                                      \
    sbbq p1(% rip), % r9;                                                      \
    sbbq p2(% rip), % r10;                                                     \
                                                                               \
    movq % r10, % r14;                                                         \
    shlq $62, % r14;                                                           \
                                                                               \
    cmovc % r11, % r8;                                                         \
    cmovc % r12, % r9;                                                         \
    cmovc % r13, % r10;

/* macros used for computing the key powers */

/* input in (%r15,%r14);
 * output in (%r10,%r9,%r8);
 */
#define tau_square()                                                           \
                                                                               \
    movq % r14, % rdx;                                                         \
    mulx % r15, % r9, % r10;                                                   \
    movq $0, % r11;                                                            \
    shld $1, % r10, % r11;                                                     \
    shld $1, % r9, % r10;                                                      \
    shlq $1, % r9;                                                             \
                                                                               \
    xorq % r12, % r12;                                                         \
    mulx % rdx, % r8, % rbx;                                                   \
    adcx % rbx, % r9;                                                          \
                                                                               \
    movq % r15, % rdx;                                                         \
    mulx % rdx, % rax, % rbx;                                                  \
    adcx % rax, % r10;                                                         \
    adcx % rbx, % r11;                                                         \
                                                                               \
    movq % r10, % r13;                                                         \
    andq mask2(% rip), % r10;                                                  \
    andq mask2c(% rip), % r13;                                                 \
                                                                               \
    addq % r13, % r8;                                                          \
    adcq % r11, % r9;                                                          \
    adcq % r12, % r10;                                                         \
                                                                               \
    shrd $2, % r11, % r13;                                                     \
    shrq $2, % r11;                                                            \
                                                                               \
    addq % r13, % r8;                                                          \
    adcq % r11, % r9;                                                          \
    adcq % r12, % r10;                                                         \
                                                                               \
    movq % r10, % r13;                                                         \
    andq mask2(% rip), % r10;                                                  \
    shrq $2, % r13;                                                            \
                                                                               \
    imul $5, % r13, % r13;                                                     \
    addq % r13, % r8;                                                          \
    adcq $0, % r9;                                                             \
    adcq $0, % r10;                                                            \
                                                                               \
    movq % r8, 24 * 1 + 0(% rdi);                                              \
    movq % r9, 24 * 1 + 8(% rdi);                                              \
    movq % r10, 24 * 1 + 16(% rdi);

/* input in (%r10,%r9,%r8);
 * output in (%r10,%r9,%r8);
 * x is the output storing index;
 */
#define tau_squaren(x)                                                         \
                                                                               \
    xorq % r15, % r15;                                                         \
    movq % r8, % rdx;                                                          \
                                                                               \
    mulx % r9, % r12, % r13;                                                   \
                                                                               \
    mulx % r10, % rbx, % r14;                                                  \
    adcx % rbx, % r13;                                                         \
    adcx % r15, % r14;                                                         \
                                                                               \
    xorq % rax, % rax;                                                         \
    movq % r9, % rdx;                                                          \
                                                                               \
    mulx % r10, % rbx, % rdx;                                                  \
    adcx % rbx, % r14;                                                         \
    adox % rdx, % r15;                                                         \
    adcx % rax, % r15;                                                         \
                                                                               \
    shld $1, % r14, % r15;                                                     \
    shld $1, % r13, % r14;                                                     \
    shld $1, % r12, % r13;                                                     \
    shlq $1, % r12;                                                            \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq % r8, % rdx;                                                          \
    mulx % rdx, % r11, % rbx;                                                  \
    adcx % rbx, % r12;                                                         \
                                                                               \
    movq % r9, % rdx;                                                          \
    mulx % rdx, % r8, % r9;                                                    \
    adcx % r13, % r8;                                                          \
    adcx % r14, % r9;                                                          \
                                                                               \
    movq % r10, % rdx;                                                         \
    mulx % rdx, % rax, % rbx;                                                  \
    adcx % rax, % r15;                                                         \
                                                                               \
    movq % r8, % r10;                                                          \
    andq mask2(% rip), % r10;                                                  \
    andq mask2c(% rip), % r8;                                                  \
                                                                               \
    addq % r8, % r11;                                                          \
    adcq % r9, % r12;                                                          \
    adcq % r15, % r10;                                                         \
                                                                               \
    shrd $2, % r9, % r8;                                                       \
    shrd $2, % r15, % r9;                                                      \
    shrq $2, % r15;                                                            \
                                                                               \
    addq % r11, % r8;                                                          \
    adcq % r12, % r9;                                                          \
    adcq % r15, % r10;                                                         \
                                                                               \
    movq % r10, % r15;                                                         \
    andq mask2(% rip), % r10;                                                  \
    shrq $2, % r15;                                                            \
                                                                               \
    imul $5, % r15, % r15;                                                     \
    addq % r15, % r8;                                                          \
    adcq $0, % r9;                                                             \
    adcq $0, % r10;                                                            \
                                                                               \
    movq % r8, 24 * x + 0(% rdi);                                              \
    movq % r9, 24 * x + 8(% rdi);                                              \
    movq % r10, 24 * x + 16(% rdi);

/* multiply tau^n with tau
 * x is the index to tau^n;
 * y is the output storing index;
 */
#define mul_tau_taun(x, y)                                                     \
                                                                               \
    xorq % r11, % r11;                                                         \
    movq 24 * x + 0(% rdi), % rdx;                                             \
                                                                               \
    mulx 0 * 8(% rdi), % r8, % r9;                                             \
    mulx 1 * 8(% rdi), % rbx, % r10;                                           \
    adcx % rbx, % r9;                                                          \
    adcx % r11, % r10;                                                         \
                                                                               \
    xorq % r12, % r12;                                                         \
    movq 24 * x + 8(% rdi), % rdx;                                             \
                                                                               \
    mulx 0 * 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r9;                                                          \
    adox % rbp, % r10;                                                         \
                                                                               \
    mulx 1 * 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r10;                                                         \
    adox % rbp, % r11;                                                         \
    adcx % r12, % r11;                                                         \
                                                                               \
    xorq % rax, % rax;                                                         \
    movq 24 * x + 16(% rdi), % rdx;                                            \
                                                                               \
    mulx 0 * 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r10;                                                         \
    adox % rbp, % r11;                                                         \
                                                                               \
    mulx 1 * 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r11;                                                         \
    adox % rbp, % r12;                                                         \
    adcx % rax, % r12;                                                         \
                                                                               \
    movq % r10, % r13;                                                         \
    andq mask2(% rip), % r10;                                                  \
    andq mask2c(% rip), % r13;                                                 \
                                                                               \
    addq % r13, % r8;                                                          \
    adcq % r11, % r9;                                                          \
    adcq % r12, % r10;                                                         \
                                                                               \
    shrd $2, % r11, % r13;                                                     \
    shrd $2, % r12, % r11;                                                     \
    shrq $2, % r12;                                                            \
                                                                               \
    addq % r8, % r13;                                                          \
    adcq % r9, % r11;                                                          \
    adcq % r10, % r12;                                                         \
                                                                               \
    movq % r12, % rcx;                                                         \
    andq mask2(% rip), % rcx;                                                  \
    shrq $2, % r12;                                                            \
                                                                               \
    imul $5, % r12, % r12;                                                     \
    addq % r12, % r13;                                                         \
    adcq $0, % r11;                                                            \
    adcq $0, % rcx;                                                            \
                                                                               \
    movq % r13, 24 * y + 0(% rdi);                                             \
    movq % r11, 24 * y + 8(% rdi);                                             \
    movq % rcx, 24 * y + 16(% rdi);

#endif
