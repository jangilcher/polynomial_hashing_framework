/* assembly constants */

.data

.globl p0
.globl p1
.globl mask56
.globl mask62
.globl mask63
.globl zero

.p2align 5

p0	: .quad 0xFFFFFFFFFFFFFFFF
p1	: .quad 0x7FFFFFFFFFFFFFFF
mask56	: .quad 0x00FFFFFFFFFFFFFF
mask62	: .quad 0x3FFFFFFFFFFFFFFF
mask63	: .quad 0x7FFFFFFFFFFFFFFF
zero	: .quad 0x0
