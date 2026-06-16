#ifndef __d2LHash1271asm__
#define __d2LHash1271asm__

/* macros used in the main assembly */

#define brw_add_block(x, y, a, b)                                              \
                                                                               \
    movq 15 * x + 0(% rsi), a;                                                 \
    movq 15 * x + 8(% rsi), b;                                                 \
    andq mask56(% rip), b;                                                     \
                                                                               \
    addq 16 * y + 0(% rdi), a;                                                 \
    adcq 16 * y + 8(% rdi), b;

#define brw_mul(a, b)                                                          \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq a, % rdx;                                                             \
                                                                               \
    mulx % r14, % r8, % rbp;                                                   \
    mulx % r15, % rbx, % r10;                                                  \
    adcx % rbx, % rbp;                                                         \
    adcx zero(% rip), % r10;                                                   \
                                                                               \
    xorq % r11, % r11;                                                         \
    movq b, % rdx;                                                             \
                                                                               \
    mulx % r14, % r9, % r12;                                                   \
    adcx % rbp, % r9;                                                          \
    adox % r12, % r10;                                                         \
                                                                               \
    mulx % r15, % rbx, % r12;                                                  \
    adcx % rbx, % r10;                                                         \
    adox % r12, % r11;                                                         \
    adcx zero(% rip), % r11;

#define brw_add_block3(x, a, b, c)                                             \
                                                                               \
    movq 15 * x + 8(% rsi), c;                                                 \
    andq mask56(% rip), c;                                                     \
    addq 15 * x + 0(% rsi), a;                                                 \
    adcq c, b;

#define brw_store_temp1(x)                                                     \
                                                                               \
    movq % r8, x + 0(% rsp);                                                   \
    movq % r9, x + 8(% rsp);                                                   \
    movq % r10, x + 16(% rsp);                                                 \
    movq % r11, x + 24(% rsp);

#define brw_store_temp2(x)                                                     \
                                                                               \
    movq % r8, x + 0(% rsp);                                                   \
    movq % r9, x + 8(% rsp);                                                   \
    movq % r10, x + 16(% rsp);                                                 \
    movq % r11, x + 24(% rsp);                                                 \
    movq % r12, x + 32(% rsp);

#define brw_add_temp1(x)                                                       \
                                                                               \
    xorq % r12, % r12;                                                         \
    adcx x + 0(% rsp), % r8;                                                   \
    adcx x + 8(% rsp), % r9;                                                   \
    adcx x + 16(% rsp), % r10;                                                 \
    adcx x + 24(% rsp), % r11;                                                 \
    adcx % r12, % r12;

#define brw_add_temp2(x)                                                       \
                                                                               \
    xorq % r12, % r12;                                                         \
    adcx x + 0(% rsp), % r8;                                                   \
    adcx x + 8(% rsp), % r9;                                                   \
    adcx x + 16(% rsp), % r10;                                                 \
    adcx x + 24(% rsp), % r11;                                                 \
    adcx x + 32(% rsp), % r12;

#define brw_store_output()                                                     \
                                                                               \
    movq % r8, 0(% rcx);                                                       \
    movq % r9, 8(% rcx);

#define mul_taun_brw(m, t)                                                     \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq m + 0(% rsi), % rdx;                                                  \
                                                                               \
    mulx t + 0(% rdi), % rax, % r13;                                           \
    mulx t + 8(% rdi), % rbx, % r14;                                           \
    adcx % rbx, % r13;                                                         \
    adcx zero(% rip), % r14;                                                   \
                                                                               \
    xorq % r15, % r15;                                                         \
    movq m + 8(% rsi), % rdx;                                                  \
                                                                               \
    mulx t + 0(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r13;                                                         \
    adox % rbp, % r14;                                                         \
                                                                               \
    mulx t + 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r14;                                                         \
    adox % rbp, % r15;                                                         \
    adcx zero(% rip), % r15;

#define mul_taun(m, t)                                                         \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq m + 0(% rsi), % rdx;                                                  \
                                                                               \
    mulx t + 0(% rdi), % rax, % r13;                                           \
    mulx t + 8(% rdi), % rbx, % r14;                                           \
    adcx % rbx, % r13;                                                         \
    adcx zero(% rip), % r14;                                                   \
                                                                               \
    xorq % r15, % r15;                                                         \
    movq m + 8(% rsi), % rdx;                                                  \
    andq mask56(% rip), % rdx;                                                 \
                                                                               \
    mulx t + 0(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r13;                                                         \
    adox % rbp, % r14;                                                         \
                                                                               \
    mulx t + 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r14;                                                         \
    adox % rbp, % r15;                                                         \
    adcx zero(% rip), % r15;

#define mul_taunr(t)                                                           \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq % r8, % rdx;                                                          \
                                                                               \
    mulx t + 0(% rdi), % r8, % rax;                                            \
    mulx t + 8(% rdi), % rbx, % r13;                                           \
    adcx % rbx, % rax;                                                         \
    adcx zero(% rip), % r13;                                                   \
                                                                               \
    xorq % r11, % r11;                                                         \
    movq % r9, % rdx;                                                          \
                                                                               \
    mulx t + 0(% rdi), % r9, % r10;                                            \
    adcx % rax, % r9;                                                          \
    adox % r13, % r10;                                                         \
                                                                               \
    mulx t + 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r10;                                                         \
    adox % rbp, % r11;                                                         \
    adcx zero(% rip), % r11;

#define mul_len_tau(m)                                                         \
                                                                               \
    xorq % r15, % r15;                                                         \
    movq m(% rsp), % rdx;                                                      \
                                                                               \
    mulx 0(% rdi), % rax, % r13;                                               \
    mulx 8(% rdi), % rbx, % r14;                                               \
    adcx % rbx, % r13;                                                         \
    adcx % r15, % r14;

#define add_product1()                                                         \
                                                                               \
    xorq % rdx, % rdx;                                                         \
                                                                               \
    adcx % rax, % r8;                                                          \
    adcx % r13, % r9;                                                          \
    adcx % r14, % r10;                                                         \
    adcx % r15, % r11;

#define add_product2()                                                         \
                                                                               \
    xorq % rdx, % rdx;                                                         \
                                                                               \
    adcx % rax, % r8;                                                          \
    adcx % r13, % r9;                                                          \
    adcx % r14, % r10;                                                         \
    adcx % r15, % r11;                                                         \
    adcx zero(% rip), % r12;

#define add_msg_block1(m)                                                      \
                                                                               \
    addq m + 0(% rsi), % r8;                                                   \
    adcq m + 8(% rsi), % r9;

#define add_msg_block2(m)                                                      \
                                                                               \
    movq m + 8(% rsi), % rdx;                                                  \
    andq mask56(% rip), % rdx;                                                 \
    addq m + 0(% rsi), % r8;                                                   \
    adcq % rdx, % r9;

#define reduce_5limb()                                                         \
                                                                               \
    shld $2, % r11, % r12;                                                     \
                                                                               \
    andq mask62(% rip), % r11;                                                 \
    xorq % rdx, % rdx;                                                         \
    adcx % r12, % r8;                                                          \
    adcx % rdx, % r9;                                                          \
    adcx % rdx, % r10;                                                         \
    adcx % rdx, % r11;

#define reduce_4limb()                                                         \
                                                                               \
    shld $1, % r10, % r11;                                                     \
    shld $1, % r9, % r10;                                                      \
                                                                               \
    andq mask63(% rip), % r9;                                                  \
    xorq % rdx, % rdx;                                                         \
    adcx % r10, % r8;                                                          \
    adcx % r11, % r9;

#define reduce_2limb()                                                         \
                                                                               \
    movq % r9, % r10;                                                          \
    shrq $63, % r10;                                                           \
    andq mask63(% rip), % r9;                                                  \
    addq % r10, % r8;                                                          \
    adcq zero(% rip), % r9;

#define make_unique()                                                          \
                                                                               \
    movq % r8, % r11;                                                          \
    movq % r9, % r12;                                                          \
                                                                               \
    subq p0(% rip), % r8;                                                      \
    sbbq p1(% rip), % r9;                                                      \
                                                                               \
    movq % r9, % r10;                                                          \
    shlq $1, % r10;                                                            \
                                                                               \
    cmovc % r11, % r8;                                                         \
    cmovc % r12, % r9;

/* macros used for computing the key powers */

#define tau_squaren(t)                                                         \
                                                                               \
    movq % rcx, % rdx;                                                         \
    addq % rdx, % rdx;                                                         \
    mulx % rax, % r9, % r10;                                                   \
                                                                               \
    movq % rax, % rdx;                                                         \
    mulx % rdx, % rax, % rdx;                                                  \
    addq % rdx, % r9;                                                          \
                                                                               \
    movq % rcx, % rdx;                                                         \
    mulx % rdx, % rcx, % r11;                                                  \
    adcq % rcx, % r10;                                                         \
    adcq $0, % r11;                                                            \
                                                                               \
    shld $1, % r10, % r11;                                                     \
    shld $1, % r9, % r10;                                                      \
                                                                               \
    andq mask63(% rip), % r9;                                                  \
    xorq % rdx, % rdx;                                                         \
    adcx % r10, % rax;                                                         \
    adcx % r11, % r9;                                                          \
                                                                               \
    movq % r9, % rcx;                                                          \
    andq mask63(% rip), % rcx;                                                 \
    shrq $63, % r9;                                                            \
    addq % r9, % rax;                                                          \
    adcq $0, % rcx;                                                            \
                                                                               \
    movq % rax, t + 0(% rdi);                                                  \
    movq % rcx, t + 8(% rdi);

#define mul_tau_powers(x, y, z)                                                \
                                                                               \
    movq x + 0(% rdi), % rdx;                                                  \
                                                                               \
    mulx y + 0(% rdi), % r8, % r9;                                             \
    mulx y + 8(% rdi), % rcx, % r10;                                           \
    addq % rcx, % r9;                                                          \
    adcq $0, % r10;                                                            \
                                                                               \
    xorq % r11, % r11;                                                         \
    movq x + 8(% rdi), % rdx;                                                  \
                                                                               \
    mulx y + 0(% rdi), % rcx, % rax;                                           \
    adcx % rcx, % r9;                                                          \
    adox % rax, % r10;                                                         \
                                                                               \
    mulx y + 8(% rdi), % rcx, % rax;                                           \
    adcx % rcx, % r10;                                                         \
    adox % rax, % r11;                                                         \
    adcx zero(% rip), % r11;                                                   \
                                                                               \
    shld $1, % r10, % r11;                                                     \
    shld $1, % r9, % r10;                                                      \
                                                                               \
    andq mask63(% rip), % r9;                                                  \
    xorq % rdx, % rdx;                                                         \
    adcx % r10, % r8;                                                          \
    adcx % r11, % r9;                                                          \
                                                                               \
    movq % r9, % r10;                                                          \
    shrq $63, % r10;                                                           \
    andq mask63(% rip), % r9;                                                  \
    addq % r10, % r8;                                                          \
    adcq $0, % r9;                                                             \
                                                                               \
    movq % r8, z + 0(% rdi);                                                   \
    movq % r9, z + 8(% rdi);

#endif
