#ifndef __d2LHash1305asm__
#define __d2LHash1305asm__

/* macros used in the main assembly */

#define brw_add_block(x, y, a, b, c)                                           \
                                                                               \
    movq 16 * x + 0(% rsi), a;                                                 \
    movq 16 * x + 8(% rsi), b;                                                 \
    movq $0, c;                                                                \
                                                                               \
    addq 24 * y + 0(% rdi), a;                                                 \
    adcq 24 * y + 8(% rdi), b;                                                 \
    adcq 24 * y + 16(% rdi), c;

#define brw_mul()                                                              \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq % r8, % rdx;                                                          \
                                                                               \
    mulx % r13, % r8, % r9;                                                    \
    mulx % r14, % rbx, % r10;                                                  \
    adcx % rbx, % r9;                                                          \
                                                                               \
    mulx % r15, % rbx, % rax;                                                  \
    adcx % rbx, % r10;                                                         \
    adcx zero(% rip), % rax;                                                   \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq % r11, % rdx;                                                         \
                                                                               \
    mulx % r13, % rbx, % rbp;                                                  \
    adcx % rbx, % r9;                                                          \
    adox % rbp, % r10;                                                         \
                                                                               \
    mulx % r14, % rbx, % r11;                                                  \
    adcx % rbx, % r10;                                                         \
    adox % rax, % r11;                                                         \
                                                                               \
    mulx % r15, % rbx, % rax;                                                  \
    adcx % rbx, % r11;                                                         \
    adox zero(% rip), % rax;                                                   \
    adcx zero(% rip), % rax;                                                   \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq % r12, % rdx;                                                         \
                                                                               \
    mulx % r13, % rbx, % rbp;                                                  \
    adcx % rbx, % r10;                                                         \
    adox % rbp, % r11;                                                         \
                                                                               \
    mulx % r14, % rbx, % r12;                                                  \
    adcx % rbx, % r11;                                                         \
    adox % rax, % r12;                                                         \
                                                                               \
    mulx % r15, % rbx, % rbp;                                                  \
    adcx % rbx, % r12;

#define brw_add_block3(x, a, b, c)                                             \
                                                                               \
    addq 16 * x + 0(% rsi), a;                                                 \
    adcq 16 * x + 8(% rsi), b;                                                 \
    adcq zero(% rip), c;

#define brw_store_temp(x)                                                      \
                                                                               \
    movq % r8, x + 0(% rsp);                                                   \
    movq % r9, x + 8(% rsp);                                                   \
    movq % r10, x + 16(% rsp);                                                 \
    movq % r11, x + 24(% rsp);                                                 \
    movq % r12, x + 32(% rsp);

#define brw_add_temp(x)                                                        \
                                                                               \
    addq x + 0(% rsp), % r8;                                                   \
    adcq x + 8(% rsp), % r9;                                                   \
    adcq x + 16(% rsp), % r10;                                                 \
    adcq x + 24(% rsp), % r11;                                                 \
    adcq x + 32(% rsp), % r12;

#define brw_store_output()                                                     \
                                                                               \
    movq % r8, 0(% rcx);                                                       \
    movq % r9, 8(% rcx);                                                       \
    movq % r10, 16(% rcx);

#define mul_gamman(m, t)                                                       \
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
    movq m + 16(% rsi), % rdx;                                                 \
                                                                               \
    mulx t + 0(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r15;                                                         \
    adox % rbp, % rax;                                                         \
                                                                               \
    mulx t + 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % rax;                                                         \
    adox % rbp, % rcx;                                                         \
                                                                               \
    mulx t + 16(% rdi), % rbx, % rbp;                                          \
    adcx % rbx, % rcx;

#define mul_gammanr(t)                                                         \
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
    adcx zero(% rip), % rcx;

#define mul_taunr mul_gammanr

#define mul_len_tau_and_add(m)                                                 \
                                                                               \
    xorq % rbp, % rbp;                                                         \
    movq m(% rsp), % rdx;                                                      \
                                                                               \
    mulx 0(% rdi), % r13, % r14;                                               \
    mulx 8(% rdi), % rbx, % r15;                                               \
    adcx % rbx, % r14;                                                         \
    adcx % rbp, % r15;                                                         \
                                                                               \
    xorq % rdx, % rdx;                                                         \
                                                                               \
    adcx % r13, % r8;                                                          \
    adcx % r14, % r9;                                                          \
    adcx % r15, % r10;                                                         \
    adcx % rbp, % r11;                                                         \
    adcx % rbp, % r12;

#define add_product()                                                          \
                                                                               \
    xorq % rdx, % rdx;                                                         \
                                                                               \
    adcx % r13, % r8;                                                          \
    adcx % r14, % r9;                                                          \
    adcx % r15, % r10;                                                         \
    adcx % rax, % r11;                                                         \
    adcx % rcx, % r12;

#define add_msg_block(m)                                                       \
                                                                               \
    addq m + 0(% rsi), % r8;                                                   \
    adcq m + 8(% rsi), % r9;                                                   \
    adcq m + 16(% rsi), % r10;

#define reduce_5limb(a, b, c, d)                                               \
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
    adcq c, a;                                                                 \
    adcq d, b;

#define reduce_3limb(a, b, c)                                                  \
                                                                               \
    movq c, % r13;                                                             \
    andq mask2(% rip), c;                                                      \
    shrq $2, % r13;                                                            \
                                                                               \
    imul $5, % r13, % r13;                                                     \
    addq % r13, a;                                                             \
    adcq $0, b;                                                                \
    adcq $0, c;

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

#define mul_tau(x, y)                                                          \
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

#define mul(x, y, z)                                                           \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq 24 * x + 0(% rdi), % rdx;                                             \
                                                                               \
    mulx 24 * y + 0(% rdi), % r8, % r9;                                        \
    mulx 24 * y + 8(% rdi), % rbx, % r10;                                      \
    adcx % rbx, % r9;                                                          \
                                                                               \
    mulx 24 * y + 16(% rdi), % rbx, % rax;                                     \
    adcx % rbx, % r10;                                                         \
    adcx zero(% rip), % rax;                                                   \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq 24 * x + 8(% rdi), % rdx;                                             \
                                                                               \
    mulx 24 * y + 0(% rdi), % rbx, % rbp;                                      \
    adcx % rbx, % r9;                                                          \
    adox % rbp, % r10;                                                         \
                                                                               \
    mulx 24 * y + 8(% rdi), % rbx, % r11;                                      \
    adcx % rbx, % r10;                                                         \
    adox % rax, % r11;                                                         \
                                                                               \
    mulx 24 * y + 16(% rdi), % rbx, % rax;                                     \
    adcx % rbx, % r11;                                                         \
    adox zero(% rip), % rax;                                                   \
    adcx zero(% rip), % rax;                                                   \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq 24 * x + 16(% rdi), % rdx;                                            \
                                                                               \
    mulx 24 * y + 0(% rdi), % rbx, % rbp;                                      \
    adcx % rbx, % r10;                                                         \
    adox % rbp, % r11;                                                         \
                                                                               \
    mulx 24 * y + 8(% rdi), % rbx, % r12;                                      \
    adcx % rbx, % r11;                                                         \
    adox % rax, % r12;                                                         \
                                                                               \
    mulx 24 * y + 16(% rdi), % rbx, % rbp;                                     \
    adcx % rbx, % r12;                                                         \
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
    adcq % r12, % r10;                                                         \
                                                                               \
    movq % r10, % r11;                                                         \
    andq mask2(% rip), % r10;                                                  \
    shrq $2, % r11;                                                            \
                                                                               \
    imul $5, % r11, % r11;                                                     \
    addq % r11, % r8;                                                          \
    adcq $0, % r9;                                                             \
    adcq $0, % r10;                                                            \
                                                                               \
    movq % r8, 24 * z + 0(% rdi);                                              \
    movq % r9, 24 * z + 8(% rdi);                                              \
    movq % r10, 24 * z + 16(% rdi);

#endif
