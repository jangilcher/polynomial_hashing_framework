/* assembly constants */

.data

.globl p0
.globl p1
.globl p2
.globl mask2
.globl mask2c
.globl zero

.p2align 5

p0	: .quad 0xFFFFFFFFFFFFFFFB
p1	: .quad 0xFFFFFFFFFFFFFFFF
p2	: .quad 0x0000000000000003
mask2	: .quad 0x0000000000000003
mask2c	: .quad 0xFFFFFFFFFFFFFFFC
zero	: .quad 0x0
