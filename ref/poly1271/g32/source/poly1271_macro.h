#ifndef __POLY1271ASM__
#define __POLY1271ASM__

/* macros used in the main assembly */

#define mul_taun(m, t)                                                         \
                                                                               \
    xorq % rdx, % rdx;                                                         \
    movq m + 0(% rsi), % rdx;                                                  \
                                                                               \
    mulx t + 0(% rdi), % r12, % r13;                                           \
    mulx t + 8(% rdi), % rbx, % r14;                                           \
    adcx % rbx, % r13;                                                         \
    adcx zero(% rip), % r14;                                                   \
                                                                               \
    xorq % r15, % r15;                                                         \
    movq m + 8(% rsi), % rdx;                                                  \
    andq mask56(% rip), % rdx;                                                 \
    orq c(% rip), % rdx;                                                       \
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
    mulx t + 0(% rdi), % r8, % r12;                                            \
    mulx t + 8(% rdi), % rbx, % r13;                                           \
    adcx % rbx, % r12;                                                         \
    adcx zero(% rip), % r13;                                                   \
                                                                               \
    xorq % r11, % r11;                                                         \
    movq % r9, % rdx;                                                          \
                                                                               \
    mulx t + 0(% rdi), % r9, % r10;                                            \
    adcx % r12, % r9;                                                          \
    adox % r13, % r10;                                                         \
                                                                               \
    mulx t + 8(% rdi), % rbx, % rbp;                                           \
    adcx % rbx, % r10;                                                         \
    adox % rbp, % r11;                                                         \
    adcx zero(% rip), % r11;

#define add_product()                                                          \
                                                                               \
    xorq % rdx, % rdx;                                                         \
                                                                               \
    adcx % r12, % r8;                                                          \
    adcx % r13, % r9;                                                          \
    adcx % r14, % r10;                                                         \
    adcx % r15, % r11;

#define add_msg_block(m)                                                       \
                                                                               \
    movq m + 8(% rsi), % rbp;                                                  \
    andq mask56(% rip), % rbp;                                                 \
    orq c(% rip), % rbp;                                                       \
    xorq % rdx, % rdx;                                                         \
    adcx m + 0(% rsi), % r8;                                                   \
    adcx % rbp, % r9;

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

#define mul_tau_taun(x, y)                                                     \
                                                                               \
    movq x + 0(% rdi), % rdx;                                                  \
                                                                               \
    mulx 0(% rdi), % r8, % r9;                                                 \
    mulx 8(% rdi), % rcx, % r10;                                               \
    addq % rcx, % r9;                                                          \
    adcq $0, % r10;                                                            \
                                                                               \
    xorq % r11, % r11;                                                         \
    movq x + 8(% rdi), % rdx;                                                  \
                                                                               \
    mulx 0(% rdi), % rcx, % rax;                                               \
    adcx % rcx, % r9;                                                          \
    adox % rax, % r10;                                                         \
                                                                               \
    mulx 8(% rdi), % rcx, % rax;                                               \
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
    movq % r8, y + 0(% rdi);                                                   \
    movq % r9, y + 8(% rdi);

#endif
