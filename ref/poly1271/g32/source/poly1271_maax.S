/* assembly to compute the poly1271 hash function using
 * Horner's method with the help of  precomputed key powers
 */

#include "poly1271_macro.h"

	.p2align 5
	.globl poly1271maax_precomputed_keypowers
poly1271maax_precomputed_keypowers:

	movq 	%rsp,%r11
	andq    $-32,%rsp
	subq 	$72,%rsp

	movq 	%r11,0(%rsp)
	movq 	%r12,8(%rsp)
	movq 	%r13,16(%rsp)
	movq 	%r14,24(%rsp)
	movq 	%r15,32(%rsp)
	movq 	%rbx,40(%rsp)
	movq 	%rbp,48(%rsp)

	movq 	%rdi,56(%rsp)
	movq 	%r8,64(%rsp)
	movq 	%r9,%rax

	cmpq    $1,%rcx
	je      .L1

	movq    %rdx,%rdi

	movq    $0,%r8
	movq    $0,%r9
	movq    $0,%r10
	movq    $0,%r11

	cmpq    $2,%rcx
	je      .LB2

	cmpq    $3,%rcx
	je      .LB3

	cmpq    $4,%rcx
	je      .LB4

	cmpq    $5,%rcx
	je      .LB5

	cmpq    $6,%rcx
	je      .LB6

	cmpq    $7,%rcx
	je      .LB7

	cmpq    $8,%rcx
	je      .LB8

	cmpq    $9,%rcx
	je      .LB9

	cmpq    $10,%rcx
	je      .LB10

	cmpq    $11,%rcx
	je      .LB11

	cmpq    $12,%rcx
	je      .LB12

	cmpq    $13,%rcx
	je      .LB13

	cmpq    $14,%rcx
	je      .LB14

	cmpq    $15,%rcx
	je      .LB15

	cmpq    $16,%rcx
	je      .LB16

	cmpq    $17,%rcx
	je      .LB17

	cmpq    $18,%rcx
	je      .LB18

	cmpq    $19,%rcx
	je      .LB19

	cmpq    $20,%rcx
	je      .LB20

	cmpq    $21,%rcx
	je      .LB21

	cmpq    $22,%rcx
	je      .LB22

	cmpq    $23,%rcx
	je      .LB23

	cmpq    $24,%rcx
	je      .LB24

	cmpq    $25,%rcx
	je      .LB25

	cmpq    $26,%rcx
	je      .LB26

	cmpq    $27,%rcx
	je      .LB27

	cmpq    $28,%rcx
	je      .LB28

	cmpq    $29,%rcx
	je      .LB29

	cmpq    $30,%rcx
	je      .LB30

	cmpq    $31,%rcx
	je      .LB31

.LB32:
	mul_taun(0,480)
	add_product()

	mul_taun(15,464)
	add_product()

	mul_taun(30,448)
	add_product()

	mul_taun(45,432)
	add_product()

	mul_taun(60,416)
	add_product()

	mul_taun(75,400)
	add_product()

	mul_taun(90,384)
	add_product()

	mul_taun(105,368)
	add_product()

	mul_taun(120,352)
	add_product()

	mul_taun(135,336)
	add_product()

	mul_taun(150,320)
	add_product()

	mul_taun(165,304)
	add_product()

	mul_taun(180,288)
	add_product()

	mul_taun(195,272)
	add_product()

	mul_taun(210,256)
	add_product()

	mul_taun(225,240)
	add_product()

	mul_taun(240,224)
	add_product()

	mul_taun(255,208)
	add_product()

	mul_taun(270,192)
	add_product()

	mul_taun(285,176)
	add_product()

	mul_taun(300,160)
	add_product()

	mul_taun(315,144)
	add_product()

	mul_taun(330,128)
	add_product()

	mul_taun(345,112)
	add_product()

	mul_taun(360,96)
	add_product()

	mul_taun(375,80)
	add_product()

	mul_taun(390,64)
	add_product()

	mul_taun(405,48)
	add_product()

	mul_taun(420,32)
	add_product()

	mul_taun(435,16)
	add_product()

	mul_taun(450,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(465)

	addq	$480,%rsi
	subq    $32,%rcx

	cmpq    $0,%rcx
	je      .LB0

.LT1:
	cmpq    $1,%rcx
	jg      .LT2

	mul_taunr(0)
	jmp     .LB1

.LT2:
	cmpq    $2,%rcx
	jg      .LT3

	mul_taunr(16)
	jmp     .LB2

.LT3:
	cmpq    $3,%rcx
	jg      .LT4

	mul_taunr(32)
	jmp     .LB3

.LT4:
	cmpq    $4,%rcx
	jg      .LT5

	mul_taunr(48)
	jmp     .LB4

.LT5:
	cmpq    $5,%rcx
	jg      .LT6

	mul_taunr(64)
	jmp     .LB5

.LT6:
	cmpq    $6,%rcx
	jg      .LT7

	mul_taunr(80)
	jmp     .LB6

.LT7:
	cmpq    $7,%rcx
	jg      .LT8

	mul_taunr(96)
	jmp     .LB7

.LT8:
	cmpq    $8,%rcx
	jg      .LT9

	mul_taunr(112)
	jmp     .LB8

.LT9:
	cmpq    $9,%rcx
	jg      .LT10

	mul_taunr(128)
	jmp     .LB9

.LT10:
	cmpq    $10,%rcx
	jg      .LT11

	mul_taunr(144)
	jmp     .LB10

.LT11:
	cmpq    $11,%rcx
	jg      .LT12

	mul_taunr(160)
	jmp     .LB11

.LT12:
	cmpq    $12,%rcx
	jg      .LT13

	mul_taunr(176)
	jmp     .LB12

.LT13:
	cmpq    $13,%rcx
	jg      .LT14

	mul_taunr(192)
	jmp     .LB13

.LT14:
	cmpq    $14,%rcx
	jg      .LT15

	mul_taunr(208)
	jmp     .LB14

.LT15:
	cmpq    $15,%rcx
	jg      .LT16

	mul_taunr(224)
	jmp     .LB15

.LT16:
	cmpq    $16,%rcx
	jg      .LT17

	mul_taunr(240)
	jmp     .LB16

.LT17:
	cmpq    $17,%rcx
	jg      .LT18

	mul_taunr(256)
	jmp     .LB17

.LT18:
	cmpq    $18,%rcx
	jg      .LT19

	mul_taunr(272)
	jmp     .LB18

.LT19:
	cmpq    $19,%rcx
	jg      .LT20

	mul_taunr(288)
	jmp     .LB19

.LT20:
	cmpq    $20,%rcx
	jg      .LT21

	mul_taunr(304)
	jmp     .LB20

.LT21:
	cmpq    $21,%rcx
	jg      .LT22

	mul_taunr(320)
	jmp     .LB21

.LT22:
	cmpq    $22,%rcx
	jg      .LT23

	mul_taunr(336)
	jmp     .LB22

.LT23:
	cmpq    $23,%rcx
	jg      .LT24

	mul_taunr(352)
	jmp     .LB23

.LT24:
	cmpq    $24,%rcx
	jg      .LT25

	mul_taunr(368)
	jmp     .LB24

.LT25:
	cmpq    $25,%rcx
	jg      .LT26

	mul_taunr(384)
	jmp     .LB25

.LT26:
	cmpq    $26,%rcx
	jg      .LT27

	mul_taunr(400)
	jmp     .LB26

.LT27:
	cmpq    $27,%rcx
	jg      .LT28

	mul_taunr(416)
	jmp     .LB27

.LT28:
	cmpq    $28,%rcx
	jg      .LT29

	mul_taunr(432)
	jmp     .LB28

.LT29:
	cmpq    $29,%rcx
	jg      .LT30

	mul_taunr(448)
	jmp     .LB29

.LT30:
	cmpq    $30,%rcx
	jg      .LT31

	mul_taunr(464)
	jmp     .LB30

.LT31:
	cmpq    $31,%rcx
	jg      .LT32

	mul_taunr(480)
	jmp     .LB31

.LT32:
	mul_taunr(496)
	jmp     .LB32

.LB1:
	reduce_4limb()
	reduce_2limb()

	add_msg_block(0)

	addq	$15,%rsi

	jmp     .LB0

.LB2:
	mul_taun(0,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(15)

	addq	$30,%rsi

	jmp     .LB0

.LB3:
	mul_taun(0,16)
	add_product()

	mul_taun(15,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(30)

	addq	$45,%rsi

	jmp     .LB0

.LB4:
	mul_taun(0,32)
	add_product()

	mul_taun(15,16)
	add_product()

	mul_taun(30,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(45)

	addq	$60,%rsi

	jmp     .LB0

.LB5:
	mul_taun(0,48)
	add_product()

	mul_taun(15,32)
	add_product()

	mul_taun(30,16)
	add_product()

	mul_taun(45,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(60)

	addq	$75,%rsi

	jmp     .LB0

.LB6:
	mul_taun(0,64)
	add_product()

	mul_taun(15,48)
	add_product()

	mul_taun(30,32)
	add_product()

	mul_taun(45,16)
	add_product()

	mul_taun(60,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(75)

	addq	$90,%rsi

	jmp     .LB0

.LB7:
	mul_taun(0,80)
	add_product()

	mul_taun(15,64)
	add_product()

	mul_taun(30,48)
	add_product()

	mul_taun(45,32)
	add_product()

	mul_taun(60,16)
	add_product()

	mul_taun(75,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(90)

	addq	$105,%rsi

	jmp     .LB0

.LB8:
	mul_taun(0,96)
	add_product()

	mul_taun(15,80)
	add_product()

	mul_taun(30,64)
	add_product()

	mul_taun(45,48)
	add_product()

	mul_taun(60,32)
	add_product()

	mul_taun(75,16)
	add_product()

	mul_taun(90,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(105)

	addq	$120,%rsi

	jmp     .LB0

.LB9:
	mul_taun(0,112)
	add_product()

	mul_taun(15,96)
	add_product()

	mul_taun(30,80)
	add_product()

	mul_taun(45,64)
	add_product()

	mul_taun(60,48)
	add_product()

	mul_taun(75,32)
	add_product()

	mul_taun(90,16)
	add_product()

	mul_taun(105,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(120)

	addq	$135,%rsi

	jmp     .LB0

.LB10:
	mul_taun(0,128)
	add_product()

	mul_taun(15,112)
	add_product()

	mul_taun(30,96)
	add_product()

	mul_taun(45,80)
	add_product()

	mul_taun(60,64)
	add_product()

	mul_taun(75,48)
	add_product()

	mul_taun(90,32)
	add_product()

	mul_taun(105,16)
	add_product()

	mul_taun(120,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(135)

	addq	$150,%rsi

	jmp     .LB0

.LB11:
	mul_taun(0,144)
	add_product()

	mul_taun(15,128)
	add_product()

	mul_taun(30,112)
	add_product()

	mul_taun(45,96)
	add_product()

	mul_taun(60,80)
	add_product()

	mul_taun(75,64)
	add_product()

	mul_taun(90,48)
	add_product()

	mul_taun(105,32)
	add_product()

	mul_taun(120,16)
	add_product()

	mul_taun(135,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(150)

	addq	$165,%rsi

	jmp     .LB0

.LB12:
	mul_taun(0,160)
	add_product()

	mul_taun(15,144)
	add_product()

	mul_taun(30,128)
	add_product()

	mul_taun(45,112)
	add_product()

	mul_taun(60,96)
	add_product()

	mul_taun(75,80)
	add_product()

	mul_taun(90,64)
	add_product()

	mul_taun(105,48)
	add_product()

	mul_taun(120,32)
	add_product()

	mul_taun(135,16)
	add_product()

	mul_taun(150,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(165)

	addq	$180,%rsi

	jmp     .LB0

.LB13:
	mul_taun(0,176)
	add_product()

	mul_taun(15,160)
	add_product()

	mul_taun(30,144)
	add_product()

	mul_taun(45,128)
	add_product()

	mul_taun(60,112)
	add_product()

	mul_taun(75,96)
	add_product()

	mul_taun(90,80)
	add_product()

	mul_taun(105,64)
	add_product()

	mul_taun(120,48)
	add_product()

	mul_taun(135,32)
	add_product()

	mul_taun(150,16)
	add_product()

	mul_taun(165,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(180)

	addq	$195,%rsi

	jmp     .LB0

.LB14:
	mul_taun(0,192)
	add_product()

	mul_taun(15,176)
	add_product()

	mul_taun(30,160)
	add_product()

	mul_taun(45,144)
	add_product()

	mul_taun(60,128)
	add_product()

	mul_taun(75,112)
	add_product()

	mul_taun(90,96)
	add_product()

	mul_taun(105,80)
	add_product()

	mul_taun(120,64)
	add_product()

	mul_taun(135,48)
	add_product()

	mul_taun(150,32)
	add_product()

	mul_taun(165,16)
	add_product()

	mul_taun(180,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(195)

	addq	$210,%rsi

	jmp     .LB0

.LB15:
	mul_taun(0,208)
	add_product()

	mul_taun(15,192)
	add_product()

	mul_taun(30,176)
	add_product()

	mul_taun(45,160)
	add_product()

	mul_taun(60,144)
	add_product()

	mul_taun(75,128)
	add_product()

	mul_taun(90,112)
	add_product()

	mul_taun(105,96)
	add_product()

	mul_taun(120,80)
	add_product()

	mul_taun(135,64)
	add_product()

	mul_taun(150,48)
	add_product()

	mul_taun(165,32)
	add_product()

	mul_taun(180,16)
	add_product()

	mul_taun(195,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(210)

	addq	$225,%rsi

	jmp     .LB0

.LB16:
	mul_taun(0,224)
	add_product()

	mul_taun(15,208)
	add_product()

	mul_taun(30,192)
	add_product()

	mul_taun(45,176)
	add_product()

	mul_taun(60,160)
	add_product()

	mul_taun(75,144)
	add_product()

	mul_taun(90,128)
	add_product()

	mul_taun(105,112)
	add_product()

	mul_taun(120,96)
	add_product()

	mul_taun(135,80)
	add_product()

	mul_taun(150,64)
	add_product()

	mul_taun(165,48)
	add_product()

	mul_taun(180,32)
	add_product()

	mul_taun(195,16)
	add_product()

	mul_taun(210,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(225)

	addq	$240,%rsi

	jmp     .LB0

.LB17:
	mul_taun(0,240)
	add_product()

	mul_taun(15,224)
	add_product()

	mul_taun(30,208)
	add_product()

	mul_taun(45,192)
	add_product()

	mul_taun(60,176)
	add_product()

	mul_taun(75,160)
	add_product()

	mul_taun(90,144)
	add_product()

	mul_taun(105,128)
	add_product()

	mul_taun(120,112)
	add_product()

	mul_taun(135,96)
	add_product()

	mul_taun(150,80)
	add_product()

	mul_taun(165,64)
	add_product()

	mul_taun(180,48)
	add_product()

	mul_taun(195,32)
	add_product()

	mul_taun(210,16)
	add_product()

	mul_taun(225,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(240)

	addq	$255,%rsi

	jmp     .LB0

.LB18:
	mul_taun(0,256)
	add_product()

	mul_taun(15,240)
	add_product()

	mul_taun(30,224)
	add_product()

	mul_taun(45,208)
	add_product()

	mul_taun(60,192)
	add_product()

	mul_taun(75,176)
	add_product()

	mul_taun(90,160)
	add_product()

	mul_taun(105,144)
	add_product()

	mul_taun(120,128)
	add_product()

	mul_taun(135,112)
	add_product()

	mul_taun(150,96)
	add_product()

	mul_taun(165,80)
	add_product()

	mul_taun(180,64)
	add_product()

	mul_taun(195,48)
	add_product()

	mul_taun(210,32)
	add_product()

	mul_taun(225,16)
	add_product()

	mul_taun(240,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(255)

	addq	$270,%rsi

	jmp     .LB0

.LB19:
	mul_taun(0,272)
	add_product()

	mul_taun(15,256)
	add_product()

	mul_taun(30,240)
	add_product()

	mul_taun(45,224)
	add_product()

	mul_taun(60,208)
	add_product()

	mul_taun(75,192)
	add_product()

	mul_taun(90,176)
	add_product()

	mul_taun(105,160)
	add_product()

	mul_taun(120,144)
	add_product()

	mul_taun(135,128)
	add_product()

	mul_taun(150,112)
	add_product()

	mul_taun(165,96)
	add_product()

	mul_taun(180,80)
	add_product()

	mul_taun(195,64)
	add_product()

	mul_taun(210,48)
	add_product()

	mul_taun(225,32)
	add_product()

	mul_taun(240,16)
	add_product()

	mul_taun(255,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(270)

	addq	$285,%rsi

	jmp     .LB0

.LB20:
	mul_taun(0,288)
	add_product()

	mul_taun(15,272)
	add_product()

	mul_taun(30,256)
	add_product()

	mul_taun(45,240)
	add_product()

	mul_taun(60,224)
	add_product()

	mul_taun(75,208)
	add_product()

	mul_taun(90,192)
	add_product()

	mul_taun(105,176)
	add_product()

	mul_taun(120,160)
	add_product()

	mul_taun(135,144)
	add_product()

	mul_taun(150,128)
	add_product()

	mul_taun(165,112)
	add_product()

	mul_taun(180,96)
	add_product()

	mul_taun(195,80)
	add_product()

	mul_taun(210,64)
	add_product()

	mul_taun(225,48)
	add_product()

	mul_taun(240,32)
	add_product()

	mul_taun(255,16)
	add_product()

	mul_taun(270,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(285)

	addq	$300,%rsi

	jmp     .LB0

.LB21:
	mul_taun(0,304)
	add_product()

	mul_taun(15,288)
	add_product()

	mul_taun(30,272)
	add_product()

	mul_taun(45,256)
	add_product()

	mul_taun(60,240)
	add_product()

	mul_taun(75,224)
	add_product()

	mul_taun(90,208)
	add_product()

	mul_taun(105,192)
	add_product()

	mul_taun(120,176)
	add_product()

	mul_taun(135,160)
	add_product()

	mul_taun(150,144)
	add_product()

	mul_taun(165,128)
	add_product()

	mul_taun(180,112)
	add_product()

	mul_taun(195,96)
	add_product()

	mul_taun(210,80)
	add_product()

	mul_taun(225,64)
	add_product()

	mul_taun(240,48)
	add_product()

	mul_taun(255,32)
	add_product()

	mul_taun(270,16)
	add_product()

	mul_taun(285,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(300)

	addq	$315,%rsi

	jmp     .LB0

.LB22:
	mul_taun(0,320)
	add_product()

	mul_taun(15,304)
	add_product()

	mul_taun(30,288)
	add_product()

	mul_taun(45,272)
	add_product()

	mul_taun(60,256)
	add_product()

	mul_taun(75,240)
	add_product()

	mul_taun(90,224)
	add_product()

	mul_taun(105,208)
	add_product()

	mul_taun(120,192)
	add_product()

	mul_taun(135,176)
	add_product()

	mul_taun(150,160)
	add_product()

	mul_taun(165,144)
	add_product()

	mul_taun(180,128)
	add_product()

	mul_taun(195,112)
	add_product()

	mul_taun(210,96)
	add_product()

	mul_taun(225,80)
	add_product()

	mul_taun(240,64)
	add_product()

	mul_taun(255,48)
	add_product()

	mul_taun(270,32)
	add_product()

	mul_taun(285,16)
	add_product()

	mul_taun(300,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(315)

	addq	$330,%rsi

	jmp     .LB0

.LB23:
	mul_taun(0,336)
	add_product()

	mul_taun(15,320)
	add_product()

	mul_taun(30,304)
	add_product()

	mul_taun(45,288)
	add_product()

	mul_taun(60,272)
	add_product()

	mul_taun(75,256)
	add_product()

	mul_taun(90,240)
	add_product()

	mul_taun(105,224)
	add_product()

	mul_taun(120,208)
	add_product()

	mul_taun(135,192)
	add_product()

	mul_taun(150,176)
	add_product()

	mul_taun(165,160)
	add_product()

	mul_taun(180,144)
	add_product()

	mul_taun(195,128)
	add_product()

	mul_taun(210,112)
	add_product()

	mul_taun(225,96)
	add_product()

	mul_taun(240,80)
	add_product()

	mul_taun(255,64)
	add_product()

	mul_taun(270,48)
	add_product()

	mul_taun(285,32)
	add_product()

	mul_taun(300,16)
	add_product()

	mul_taun(315,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(330)

	addq	$345,%rsi

	jmp     .LB0

.LB24:
	mul_taun(0,352)
	add_product()

	mul_taun(15,336)
	add_product()

	mul_taun(30,320)
	add_product()

	mul_taun(45,304)
	add_product()

	mul_taun(60,288)
	add_product()

	mul_taun(75,272)
	add_product()

	mul_taun(90,256)
	add_product()

	mul_taun(105,240)
	add_product()

	mul_taun(120,224)
	add_product()

	mul_taun(135,208)
	add_product()

	mul_taun(150,192)
	add_product()

	mul_taun(165,176)
	add_product()

	mul_taun(180,160)
	add_product()

	mul_taun(195,144)
	add_product()

	mul_taun(210,128)
	add_product()

	mul_taun(225,112)
	add_product()

	mul_taun(240,96)
	add_product()

	mul_taun(255,80)
	add_product()

	mul_taun(270,64)
	add_product()

	mul_taun(285,48)
	add_product()

	mul_taun(300,32)
	add_product()

	mul_taun(315,16)
	add_product()

	mul_taun(330,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(345)

	addq	$360,%rsi

	jmp     .LB0

.LB25:
	mul_taun(0,368)
	add_product()

	mul_taun(15,352)
	add_product()

	mul_taun(30,336)
	add_product()

	mul_taun(45,320)
	add_product()

	mul_taun(60,304)
	add_product()

	mul_taun(75,288)
	add_product()

	mul_taun(90,272)
	add_product()

	mul_taun(105,256)
	add_product()

	mul_taun(120,240)
	add_product()

	mul_taun(135,224)
	add_product()

	mul_taun(150,208)
	add_product()

	mul_taun(165,192)
	add_product()

	mul_taun(180,176)
	add_product()

	mul_taun(195,160)
	add_product()

	mul_taun(210,144)
	add_product()

	mul_taun(225,128)
	add_product()

	mul_taun(240,112)
	add_product()

	mul_taun(255,96)
	add_product()

	mul_taun(270,80)
	add_product()

	mul_taun(285,64)
	add_product()

	mul_taun(300,48)
	add_product()

	mul_taun(315,32)
	add_product()

	mul_taun(330,16)
	add_product()

	mul_taun(345,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(360)

	addq	$375,%rsi

	jmp     .LB0

.LB26:
	mul_taun(0,384)
	add_product()

	mul_taun(15,368)
	add_product()

	mul_taun(30,352)
	add_product()

	mul_taun(45,336)
	add_product()

	mul_taun(60,320)
	add_product()

	mul_taun(75,304)
	add_product()

	mul_taun(90,288)
	add_product()

	mul_taun(105,272)
	add_product()

	mul_taun(120,256)
	add_product()

	mul_taun(135,240)
	add_product()

	mul_taun(150,224)
	add_product()

	mul_taun(165,208)
	add_product()

	mul_taun(180,192)
	add_product()

	mul_taun(195,176)
	add_product()

	mul_taun(210,160)
	add_product()

	mul_taun(225,144)
	add_product()

	mul_taun(240,128)
	add_product()

	mul_taun(255,112)
	add_product()

	mul_taun(270,96)
	add_product()

	mul_taun(285,80)
	add_product()

	mul_taun(300,64)
	add_product()

	mul_taun(315,48)
	add_product()

	mul_taun(330,32)
	add_product()

	mul_taun(345,16)
	add_product()

	mul_taun(360,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(375)

	addq	$390,%rsi

	jmp     .LB0

.LB27:
	mul_taun(0,400)
	add_product()

	mul_taun(15,384)
	add_product()

	mul_taun(30,368)
	add_product()

	mul_taun(45,352)
	add_product()

	mul_taun(60,336)
	add_product()

	mul_taun(75,320)
	add_product()

	mul_taun(90,304)
	add_product()

	mul_taun(105,288)
	add_product()

	mul_taun(120,272)
	add_product()

	mul_taun(135,256)
	add_product()

	mul_taun(150,240)
	add_product()

	mul_taun(165,224)
	add_product()

	mul_taun(180,208)
	add_product()

	mul_taun(195,192)
	add_product()

	mul_taun(210,176)
	add_product()

	mul_taun(225,160)
	add_product()

	mul_taun(240,144)
	add_product()

	mul_taun(255,128)
	add_product()

	mul_taun(270,112)
	add_product()

	mul_taun(285,96)
	add_product()

	mul_taun(300,80)
	add_product()

	mul_taun(315,64)
	add_product()

	mul_taun(330,48)
	add_product()

	mul_taun(345,32)
	add_product()

	mul_taun(360,16)
	add_product()

	mul_taun(375,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(390)

	addq	$405,%rsi

	jmp     .LB0

.LB28:
	mul_taun(0,416)
	add_product()

	mul_taun(15,400)
	add_product()

	mul_taun(30,384)
	add_product()

	mul_taun(45,368)
	add_product()

	mul_taun(60,352)
	add_product()

	mul_taun(75,336)
	add_product()

	mul_taun(90,320)
	add_product()

	mul_taun(105,304)
	add_product()

	mul_taun(120,288)
	add_product()

	mul_taun(135,272)
	add_product()

	mul_taun(150,256)
	add_product()

	mul_taun(165,240)
	add_product()

	mul_taun(180,224)
	add_product()

	mul_taun(195,208)
	add_product()

	mul_taun(210,192)
	add_product()

	mul_taun(225,176)
	add_product()

	mul_taun(240,160)
	add_product()

	mul_taun(255,144)
	add_product()

	mul_taun(270,128)
	add_product()

	mul_taun(285,112)
	add_product()

	mul_taun(300,96)
	add_product()

	mul_taun(315,80)
	add_product()

	mul_taun(330,64)
	add_product()

	mul_taun(345,48)
	add_product()

	mul_taun(360,32)
	add_product()

	mul_taun(375,16)
	add_product()

	mul_taun(390,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(405)

	addq	$420,%rsi

	jmp     .LB0

.LB29:
	mul_taun(0,432)
	add_product()

	mul_taun(15,416)
	add_product()

	mul_taun(30,400)
	add_product()

	mul_taun(45,384)
	add_product()

	mul_taun(60,368)
	add_product()

	mul_taun(75,352)
	add_product()

	mul_taun(90,336)
	add_product()

	mul_taun(105,320)
	add_product()

	mul_taun(120,304)
	add_product()

	mul_taun(135,288)
	add_product()

	mul_taun(150,272)
	add_product()

	mul_taun(165,256)
	add_product()

	mul_taun(180,240)
	add_product()

	mul_taun(195,224)
	add_product()

	mul_taun(210,208)
	add_product()

	mul_taun(225,192)
	add_product()

	mul_taun(240,176)
	add_product()

	mul_taun(255,160)
	add_product()

	mul_taun(270,144)
	add_product()

	mul_taun(285,128)
	add_product()

	mul_taun(300,112)
	add_product()

	mul_taun(315,96)
	add_product()

	mul_taun(330,80)
	add_product()

	mul_taun(345,64)
	add_product()

	mul_taun(360,48)
	add_product()

	mul_taun(375,32)
	add_product()

	mul_taun(390,16)
	add_product()

	mul_taun(405,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(420)

	addq	$435,%rsi

	jmp     .LB0

.LB30:
	mul_taun(0,448)
	add_product()

	mul_taun(15,432)
	add_product()

	mul_taun(30,416)
	add_product()

	mul_taun(45,400)
	add_product()

	mul_taun(60,384)
	add_product()

	mul_taun(75,368)
	add_product()

	mul_taun(90,352)
	add_product()

	mul_taun(105,336)
	add_product()

	mul_taun(120,320)
	add_product()

	mul_taun(135,304)
	add_product()

	mul_taun(150,288)
	add_product()

	mul_taun(165,272)
	add_product()

	mul_taun(180,256)
	add_product()

	mul_taun(195,240)
	add_product()

	mul_taun(210,224)
	add_product()

	mul_taun(225,208)
	add_product()

	mul_taun(240,192)
	add_product()

	mul_taun(255,176)
	add_product()

	mul_taun(270,160)
	add_product()

	mul_taun(285,144)
	add_product()

	mul_taun(300,128)
	add_product()

	mul_taun(315,112)
	add_product()

	mul_taun(330,96)
	add_product()

	mul_taun(345,80)
	add_product()

	mul_taun(360,64)
	add_product()

	mul_taun(375,48)
	add_product()

	mul_taun(390,32)
	add_product()

	mul_taun(405,16)
	add_product()

	mul_taun(420,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(435)

	addq	$450,%rsi

	jmp     .LB0

.LB31:
	mul_taun(0,464)
	add_product()

	mul_taun(15,448)
	add_product()

	mul_taun(30,432)
	add_product()

	mul_taun(45,416)
	add_product()

	mul_taun(60,400)
	add_product()

	mul_taun(75,384)
	add_product()

	mul_taun(90,368)
	add_product()

	mul_taun(105,352)
	add_product()

	mul_taun(120,336)
	add_product()

	mul_taun(135,320)
	add_product()

	mul_taun(150,304)
	add_product()

	mul_taun(165,288)
	add_product()

	mul_taun(180,272)
	add_product()

	mul_taun(195,256)
	add_product()

	mul_taun(210,240)
	add_product()

	mul_taun(225,224)
	add_product()

	mul_taun(240,208)
	add_product()

	mul_taun(255,192)
	add_product()

	mul_taun(270,176)
	add_product()

	mul_taun(285,160)
	add_product()

	mul_taun(300,144)
	add_product()

	mul_taun(315,128)
	add_product()

	mul_taun(330,112)
	add_product()

	mul_taun(345,96)
	add_product()

	mul_taun(360,80)
	add_product()

	mul_taun(375,64)
	add_product()

	mul_taun(390,48)
	add_product()

	mul_taun(405,32)
	add_product()

	mul_taun(420,16)
	add_product()

	mul_taun(435,0)
	add_product()

	reduce_4limb()
	reduce_2limb()

	add_msg_block(450)

	addq	$465,%rsi

	jmp     .LB0

.LB0:
	cmpq	$0,64(%rsp)
	jg	.L0

	subq	$15,%rsi
	movq    0(%rsi),%r10
	movq    8(%rsi),%r11
	movq    %r11,%r12
	andq    mask56(%rip),%r11
	orq     c(%rip),%r11

	subq    %r10,%r8
	sbbq    %r11,%r9

	addq    %r10,%r8
	adcq    %r12,%r9

.L0:
	mul_taunr(0)

	reduce_4limb()
	reduce_2limb()

	jmp	.LF

.L1:
	movq    0(%rdx),%r14
	movq    8(%rdx),%r15

	movq    0(%rsi),%r13
	movq    8(%rsi),%r12

	cmpq    $8,%rax
	jle     .L3

	cmpq    $0,64(%rsp)
	je      .L2

	andq    mask56(%rip),%r12
	orq     c(%rip),%r12

.L2:
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	movq    %r12,%rdx
	xorq    %r12,%r12

	mulx    %r14,%rbx,%rbp
	adcx    %rbx,%r9
	adox    %rbp,%r10

	mulx    %r15,%rbx,%rbp
	adcx    %rbx,%r10
	adox    %rbp,%r11
	adcx    %r12,%r11

	reduce_4limb()
	reduce_2limb()

	jmp     .LF

.L3:
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	reduce_4limb()
	reduce_2limb()

.LF:
	make_unique()

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
