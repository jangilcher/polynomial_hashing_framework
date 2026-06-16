/* assembly constants */

.data

.globl p0
.globl p1
.globl mask56
.globl mask62
.globl mask63
.globl c

.p2align 5

p0	: .quad 0xFFFFFFFFFFFFFFFF
p1	: .quad 0x7FFFFFFFFFFFFFFF
mask56	: .quad 0x00FFFFFFFFFFFFFF
mask62	: .quad 0x03FFFFFFFFFFFFFF
mask63	: .quad 0x7FFFFFFFFFFFFFFF
c	: .quad 0x0100000000000000
