/* assembly to compute the poly1305 hash function using
 * Horner's method with the help of precomputed key powers
 */

#include "poly1305_macro.h"

	.p2align 5
	.globl poly1305maax_precomputed_keypowers

poly1305maax_precomputed_keypowers:

	movq 	%rsp,%r11
	andq    $-32,%rsp
	subq 	$104,%rsp

	movq 	%r11,0(%rsp)
	movq 	%r12,8(%rsp)
	movq 	%r13,16(%rsp)
	movq 	%r14,24(%rsp)
	movq 	%r15,32(%rsp)
	movq 	%rbx,40(%rsp)
	movq 	%rbp,48(%rsp)
	movq 	%rdi,56(%rsp)
	movq 	%rcx,64(%rsp)
	movq 	%r8,72(%rsp)
	movq 	%r9,80(%rsp)

	cmpq    $1,%rcx
	je      .LB1

	movq    %rcx,88(%rsp)
	movq	$1,96(%rsp)

	movq    %rdx,%rdi

	movq    $0,%r8
	movq    $0,%r9
	movq    $0,%r10
	movq    $0,%r11
	movq    $0,%r12

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

	cmpq    $13,%rcx
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
	mul_taun(0,720)
	add_product()

	mul_taun(16,696)
	add_product()

	mul_taun(32,672)
	add_product()

	mul_taun(48,648)
	add_product()

	mul_taun(64,624)
	add_product()

	mul_taun(80,600)
	add_product()

	mul_taun(96,576)
	add_product()

	mul_taun(112,552)
	add_product()

	mul_taun(128,528)
	add_product()

	mul_taun(144,504)
	add_product()

	mul_taun(160,480)
	add_product()

	mul_taun(176,456)
	add_product()

	mul_taun(192,432)
	add_product()

	mul_taun(208,408)
	add_product()

	mul_taun(224,384)
	add_product()

	mul_taun(240,360)
	add_product()

	mul_taun(256,336)
	add_product()

	mul_taun(272,312)
	add_product()

	mul_taun(288,288)
	add_product()

	mul_taun(304,264)
	add_product()

	mul_taun(320,240)
	add_product()

	mul_taun(336,216)
	add_product()

	mul_taun(352,192)
	add_product()

	mul_taun(368,168)
	add_product()

	mul_taun(384,144)
	add_product()

	mul_taun(400,120)
	add_product()

	mul_taun(416,96)
	add_product()

	mul_taun(432,72)
	add_product()

	mul_taun(448,48)
	add_product()

	mul_taun(464,24)
	add_product()

	mul_tau(480,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(496,96)

	addq	$512,%rsi

	movq    88(%rsp),%rcx
	subq    $32,%rcx
	movq    %rcx,88(%rsp)

	cmpq    $0,%rcx
	je      .LB0

	cmpq    $1,%rcx
	jg      .LT2

.LT1:
	mul_taur(0)

	reduce_5limb()
	reduce_3limb()

	add_msg_block(0,72)

	jmp     .LB0

.LT2:
	cmpq    $2,%rcx
	jg      .LT3

	mul_taunr(24)
	jmp     .LB2

.LT3:
	cmpq    $3,%rcx
	jg      .LT4

	mul_taunr(48)
	jmp     .LB3

.LT4:
	cmpq    $4,%rcx
	jg      .LT5

	mul_taunr(72)
	jmp     .LB4

.LT5:
	cmpq    $5,%rcx
	jg      .LT6

	mul_taunr(96)
	jmp     .LB5

.LT6:
	cmpq    $6,%rcx
	jg      .LT7

	mul_taunr(120)
	jmp     .LB6

.LT7:
	cmpq    $7,%rcx
	jg      .LT8

	mul_taunr(144)
	jmp     .LB7

.LT8:
	cmpq    $8,%rcx
	jg      .LT9

	mul_taunr(168)
	jmp     .LB8

.LT9:
	cmpq    $9,%rcx
	jg      .LT10

	mul_taunr(192)
	jmp     .LB9

.LT10:
	cmpq    $10,%rcx
	jg      .LT11

	mul_taunr(216)
	jmp     .LB10

.LT11:
	cmpq    $11,%rcx
	jg      .LT12

	mul_taunr(240)
	jmp     .LB11

.LT12:
	cmpq    $12,%rcx
	jg      .LT13

	mul_taunr(264)
	jmp     .LB12

.LT13:
	cmpq    $13,%rcx
	jg      .LT14

	mul_taunr(288)
	jmp     .LB13

.LT14:
	cmpq    $14,%rcx
	jg      .LT15

	mul_taunr(312)
	jmp     .LB14

.LT15:
	cmpq    $15,%rcx
	jg      .LT16

	mul_taunr(336)
	jmp     .LB15

.LT16:
	cmpq    $16,%rcx
	jg      .LT17

	mul_taunr(360)
	jmp     .LB16

.LT17:
	cmpq    $17,%rcx
	jg      .LT18

	mul_taunr(384)
	jmp     .LB17

.LT18:
	cmpq    $18,%rcx
	jg      .LT19

	mul_taunr(408)
	jmp     .LB18

.LT19:
	cmpq    $19,%rcx
	jg      .LT20

	mul_taunr(432)
	jmp     .LB19

.LT20:
	cmpq    $20,%rcx
	jg      .LT21

	mul_taunr(456)
	jmp     .LB20

.LT21:
	cmpq    $21,%rcx
	jg      .LT22

	mul_taunr(480)
	jmp     .LB21

.LT22:
	cmpq    $22,%rcx
	jg      .LT23

	mul_taunr(504)
	jmp     .LB22

.LT23:
	cmpq    $23,%rcx
	jg      .LT24

	mul_taunr(528)
	jmp     .LB23

.LT24:
	cmpq    $24,%rcx
	jg      .LT25

	mul_taunr(552)
	jmp     .LB24

.LT25:
	cmpq    $25,%rcx
	jg      .LT26

	mul_taunr(576)
	jmp     .LB25

.LT26:
	cmpq    $26,%rcx
	jg      .LT27

	mul_taunr(600)
	jmp     .LB26

.LT27:
	cmpq    $27,%rcx
	jg      .LT28

	mul_taunr(624)
	jmp     .LB27

.LT28:
	cmpq    $28,%rcx
	jg      .LT29

	mul_taunr(648)
	jmp     .LB28

.LT29:
	cmpq    $29,%rcx
	jg      .LT30

	mul_taunr(672)
	jmp     .LB29

.LT30:
	cmpq    $30,%rcx
	jg      .LT31

	mul_taunr(696)
	jmp     .LB30

.LT31:
	cmpq    $31,%rcx
	jg      .LT32

	mul_taunr(720)
	jmp     .LB31

.LT32:
	mul_taunr(744)
	jmp     .LB32

.LB2:
	mul_tau(0,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(16,72)

	jmp     .LB0

.LB3:
	mul_taun(0,24)
	add_product()

	mul_tau(16,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(32,72)

	jmp     .LB0

.LB4:
	mul_taun(0,48)
	add_product()

	mul_taun(16,24)
	add_product()

	mul_tau(32,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(48,72)

	jmp     .LB0

.LB5:
	mul_taun(0,72)
	add_product()

	mul_taun(16,48)
	add_product()

	mul_taun(32,24)
	add_product()

	mul_tau(48,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(64,72)

	jmp     .LB0

.LB6:
	mul_taun(0,96)
	add_product()

	mul_taun(16,72)
	add_product()

	mul_taun(32,48)
	add_product()

	mul_taun(48,24)
	add_product()

	mul_tau(64,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(80,72)

	jmp     .LB0

.LB7:
	mul_taun(0,120)
	add_product()

	mul_taun(16,96)
	add_product()

	mul_taun(32,72)
	add_product()

	mul_taun(48,48)
	add_product()

	mul_taun(64,24)
	add_product()

	mul_tau(80,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(96,72)

	jmp     .LB0

.LB8:
	mul_taun(0,144)
	add_product()

	mul_taun(16,120)
	add_product()

	mul_taun(32,96)
	add_product()

	mul_taun(48,72)
	add_product()

	mul_taun(64,48)
	add_product()

	mul_taun(80,24)
	add_product()

	mul_tau(96,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(112,72)

	jmp     .LB0

.LB9:
	mul_taun(0,168)
	add_product()

	mul_taun(16,144)
	add_product()

	mul_taun(32,120)
	add_product()

	mul_taun(48,96)
	add_product()

	mul_taun(64,72)
	add_product()

	mul_taun(80,48)
	add_product()

	mul_taun(96,24)
	add_product()

	mul_tau(112,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(128,72)

	jmp     .LB0

.LB10:
	mul_taun(0,192)
	add_product()

	mul_taun(16,168)
	add_product()

	mul_taun(32,144)
	add_product()

	mul_taun(48,120)
	add_product()

	mul_taun(64,96)
	add_product()

	mul_taun(80,72)
	add_product()

	mul_taun(96,48)
	add_product()

	mul_taun(112,24)
	add_product()

	mul_tau(128,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(144,72)

	jmp     .LB0

.LB11:
	mul_taun(0,216)
	add_product()

	mul_taun(16,192)
	add_product()

	mul_taun(32,168)
	add_product()

	mul_taun(48,144)
	add_product()

	mul_taun(64,120)
	add_product()

	mul_taun(80,96)
	add_product()

	mul_taun(96,72)
	add_product()

	mul_taun(112,48)
	add_product()

	mul_taun(128,24)
	add_product()

	mul_tau(144,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(160,72)

	jmp     .LB0
.LB12:
	mul_taun(0,240)
	add_product()

	mul_taun(16,216)
	add_product()

	mul_taun(32,192)
	add_product()

	mul_taun(48,168)
	add_product()

	mul_taun(64,144)
	add_product()

	mul_taun(80,120)
	add_product()

	mul_taun(96,96)
	add_product()

	mul_taun(112,72)
	add_product()

	mul_taun(128,48)
	add_product()

	mul_taun(144,24)
	add_product()

	mul_tau(160,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(176,72)

	jmp     .LB0

.LB13:
	mul_taun(0,264)
	add_product()

	mul_taun(16,240)
	add_product()

	mul_taun(32,216)
	add_product()

	mul_taun(48,192)
	add_product()

	mul_taun(64,168)
	add_product()

	mul_taun(80,144)
	add_product()

	mul_taun(96,120)
	add_product()

	mul_taun(112,96)
	add_product()

	mul_taun(128,72)
	add_product()

	mul_taun(144,48)
	add_product()

	mul_taun(160,24)
	add_product()

	mul_tau(176,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(192,72)

	jmp     .LB0

.LB14:
	mul_taun(0,288)
	add_product()

	mul_taun(16,264)
	add_product()

	mul_taun(32,240)
	add_product()

	mul_taun(48,216)
	add_product()

	mul_taun(64,192)
	add_product()

	mul_taun(80,168)
	add_product()

	mul_taun(96,144)
	add_product()

	mul_taun(112,120)
	add_product()

	mul_taun(128,96)
	add_product()

	mul_taun(144,72)
	add_product()

	mul_taun(160,48)
	add_product()

	mul_taun(176,24)
	add_product()

	mul_tau(192,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(208,72)

	jmp     .LB0

.LB15:
	mul_taun(0,312)
	add_product()

	mul_taun(16,288)
	add_product()

	mul_taun(32,264)
	add_product()

	mul_taun(48,240)
	add_product()

	mul_taun(64,216)
	add_product()

	mul_taun(80,192)
	add_product()

	mul_taun(96,168)
	add_product()

	mul_taun(112,144)
	add_product()

	mul_taun(128,120)
	add_product()

	mul_taun(144,96)
	add_product()

	mul_taun(160,72)
	add_product()

	mul_taun(176,48)
	add_product()

	mul_taun(192,24)
	add_product()

	mul_tau(208,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(224,72)

	jmp	.LB0

.LB16:
	mul_taun(0,336)
	add_product()

	mul_taun(16,312)
	add_product()

	mul_taun(32,288)
	add_product()

	mul_taun(48,264)
	add_product()

	mul_taun(64,240)
	add_product()

	mul_taun(80,216)
	add_product()

	mul_taun(96,192)
	add_product()

	mul_taun(112,168)
	add_product()

	mul_taun(128,144)
	add_product()

	mul_taun(144,120)
	add_product()

	mul_taun(160,96)
	add_product()

	mul_taun(176,72)
	add_product()

	mul_taun(192,48)
	add_product()

	mul_taun(208,24)
	add_product()

	mul_tau(224,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(240,72)

	jmp	.LB0

.LB17:
	mul_taun(0,360)
	add_product()

	mul_taun(16,336)
	add_product()

	mul_taun(32,312)
	add_product()

	mul_taun(48,288)
	add_product()

	mul_taun(64,264)
	add_product()

	mul_taun(80,240)
	add_product()

	mul_taun(96,216)
	add_product()

	mul_taun(112,192)
	add_product()

	mul_taun(128,168)
	add_product()

	mul_taun(144,144)
	add_product()

	mul_taun(160,120)
	add_product()

	mul_taun(176,96)
	add_product()

	mul_taun(192,72)
	add_product()

	mul_taun(208,48)
	add_product()

	mul_taun(224,24)
	add_product()

	mul_tau(240,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(256,72)

	jmp	.LB0

.LB18:
	mul_taun(0,384)
	add_product()

	mul_taun(16,360)
	add_product()

	mul_taun(32,336)
	add_product()

	mul_taun(48,312)
	add_product()

	mul_taun(64,288)
	add_product()

	mul_taun(80,264)
	add_product()

	mul_taun(96,240)
	add_product()

	mul_taun(112,216)
	add_product()

	mul_taun(128,192)
	add_product()

	mul_taun(144,168)
	add_product()

	mul_taun(160,144)
	add_product()

	mul_taun(176,120)
	add_product()

	mul_taun(192,96)
	add_product()

	mul_taun(208,72)
	add_product()

	mul_taun(224,48)
	add_product()

	mul_taun(240,24)
	add_product()

	mul_tau(256,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(272,72)

	jmp	.LB0

.LB19:
	mul_taun(0,408)
	add_product()

	mul_taun(16,384)
	add_product()

	mul_taun(32,360)
	add_product()

	mul_taun(48,336)
	add_product()

	mul_taun(64,312)
	add_product()

	mul_taun(80,288)
	add_product()

	mul_taun(96,264)
	add_product()

	mul_taun(112,240)
	add_product()

	mul_taun(128,216)
	add_product()

	mul_taun(144,192)
	add_product()

	mul_taun(160,168)
	add_product()

	mul_taun(176,144)
	add_product()

	mul_taun(192,120)
	add_product()

	mul_taun(208,96)
	add_product()

	mul_taun(224,72)
	add_product()

	mul_taun(240,48)
	add_product()

	mul_taun(256,24)
	add_product()

	mul_tau(272,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(288,72)

	jmp	.LB0

.LB20:
	mul_taun(0,432)
	add_product()

	mul_taun(16,408)
	add_product()

	mul_taun(32,384)
	add_product()

	mul_taun(48,360)
	add_product()

	mul_taun(64,336)
	add_product()

	mul_taun(80,312)
	add_product()

	mul_taun(96,288)
	add_product()

	mul_taun(112,264)
	add_product()

	mul_taun(128,240)
	add_product()

	mul_taun(144,216)
	add_product()

	mul_taun(160,192)
	add_product()

	mul_taun(176,168)
	add_product()

	mul_taun(192,144)
	add_product()

	mul_taun(208,120)
	add_product()

	mul_taun(224,96)
	add_product()

	mul_taun(240,72)
	add_product()

	mul_taun(256,48)
	add_product()

	mul_taun(272,24)
	add_product()

	mul_tau(288,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(304,72)

	jmp	.LB0

.LB21:
	mul_taun(0,456)
	add_product()

	mul_taun(16,432)
	add_product()

	mul_taun(32,408)
	add_product()

	mul_taun(48,384)
	add_product()

	mul_taun(64,360)
	add_product()

	mul_taun(80,336)
	add_product()

	mul_taun(96,312)
	add_product()

	mul_taun(112,288)
	add_product()

	mul_taun(128,264)
	add_product()

	mul_taun(144,240)
	add_product()

	mul_taun(160,216)
	add_product()

	mul_taun(176,192)
	add_product()

	mul_taun(192,168)
	add_product()

	mul_taun(208,144)
	add_product()

	mul_taun(224,120)
	add_product()

	mul_taun(240,96)
	add_product()

	mul_taun(256,72)
	add_product()

	mul_taun(272,48)
	add_product()

	mul_taun(288,24)
	add_product()

	mul_tau(304,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(320,72)

	jmp	.LB0

.LB22:
	mul_taun(0,480)
	add_product()

	mul_taun(16,456)
	add_product()

	mul_taun(32,432)
	add_product()

	mul_taun(48,408)
	add_product()

	mul_taun(64,384)
	add_product()

	mul_taun(80,360)
	add_product()

	mul_taun(96,336)
	add_product()

	mul_taun(112,312)
	add_product()

	mul_taun(128,288)
	add_product()

	mul_taun(144,264)
	add_product()

	mul_taun(160,240)
	add_product()

	mul_taun(176,216)
	add_product()

	mul_taun(192,192)
	add_product()

	mul_taun(208,168)
	add_product()

	mul_taun(224,144)
	add_product()

	mul_taun(240,120)
	add_product()

	mul_taun(256,96)
	add_product()

	mul_taun(272,72)
	add_product()

	mul_taun(288,48)
	add_product()

	mul_taun(304,24)
	add_product()

	mul_tau(320,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(336,72)

	jmp	.LB0

.LB23:
	mul_taun(0,504)
	add_product()

	mul_taun(16,480)
	add_product()

	mul_taun(32,456)
	add_product()

	mul_taun(48,432)
	add_product()

	mul_taun(64,408)
	add_product()

	mul_taun(80,384)
	add_product()

	mul_taun(96,360)
	add_product()

	mul_taun(112,336)
	add_product()

	mul_taun(128,312)
	add_product()

	mul_taun(144,288)
	add_product()

	mul_taun(160,264)
	add_product()

	mul_taun(176,240)
	add_product()

	mul_taun(192,216)
	add_product()

	mul_taun(208,192)
	add_product()

	mul_taun(224,168)
	add_product()

	mul_taun(240,144)
	add_product()

	mul_taun(256,120)
	add_product()

	mul_taun(272,96)
	add_product()

	mul_taun(288,72)
	add_product()

	mul_taun(304,48)
	add_product()

	mul_taun(320,24)
	add_product()

	mul_tau(336,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(352,72)

	jmp	.LB0

.LB24:
	mul_taun(0,528)
	add_product()

	mul_taun(16,504)
	add_product()

	mul_taun(32,480)
	add_product()

	mul_taun(48,456)
	add_product()

	mul_taun(64,432)
	add_product()

	mul_taun(80,408)
	add_product()

	mul_taun(96,384)
	add_product()

	mul_taun(112,360)
	add_product()

	mul_taun(128,336)
	add_product()

	mul_taun(144,312)
	add_product()

	mul_taun(160,288)
	add_product()

	mul_taun(176,264)
	add_product()

	mul_taun(192,240)
	add_product()

	mul_taun(208,216)
	add_product()

	mul_taun(224,192)
	add_product()

	mul_taun(240,168)
	add_product()

	mul_taun(256,144)
	add_product()

	mul_taun(272,120)
	add_product()

	mul_taun(288,96)
	add_product()

	mul_taun(304,72)
	add_product()

	mul_taun(320,48)
	add_product()

	mul_taun(336,24)
	add_product()

	mul_tau(352,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(368,72)

	jmp	.LB0

.LB25:
	mul_taun(0,552)
	add_product()

	mul_taun(16,528)
	add_product()

	mul_taun(32,504)
	add_product()

	mul_taun(48,480)
	add_product()

	mul_taun(64,456)
	add_product()

	mul_taun(80,432)
	add_product()

	mul_taun(96,408)
	add_product()

	mul_taun(112,384)
	add_product()

	mul_taun(128,360)
	add_product()

	mul_taun(144,336)
	add_product()

	mul_taun(160,312)
	add_product()

	mul_taun(176,288)
	add_product()

	mul_taun(192,264)
	add_product()

	mul_taun(208,240)
	add_product()

	mul_taun(224,216)
	add_product()

	mul_taun(240,192)
	add_product()

	mul_taun(256,168)
	add_product()

	mul_taun(272,144)
	add_product()

	mul_taun(288,120)
	add_product()

	mul_taun(304,96)
	add_product()

	mul_taun(320,72)
	add_product()

	mul_taun(336,48)
	add_product()

	mul_taun(352,24)
	add_product()

	mul_tau(368,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(384,72)

	jmp	.LB0

.LB26:
	mul_taun(0,576)
	add_product()

	mul_taun(16,552)
	add_product()

	mul_taun(32,528)
	add_product()

	mul_taun(48,504)
	add_product()

	mul_taun(64,480)
	add_product()

	mul_taun(80,456)
	add_product()

	mul_taun(96,432)
	add_product()

	mul_taun(112,408)
	add_product()

	mul_taun(128,384)
	add_product()

	mul_taun(144,360)
	add_product()

	mul_taun(160,336)
	add_product()

	mul_taun(176,312)
	add_product()

	mul_taun(192,288)
	add_product()

	mul_taun(208,264)
	add_product()

	mul_taun(224,240)
	add_product()

	mul_taun(240,216)
	add_product()

	mul_taun(256,192)
	add_product()

	mul_taun(272,168)
	add_product()

	mul_taun(288,144)
	add_product()

	mul_taun(304,120)
	add_product()

	mul_taun(320,96)
	add_product()

	mul_taun(336,72)
	add_product()

	mul_taun(352,48)
	add_product()

	mul_taun(368,24)
	add_product()

	mul_tau(384,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(400,72)

	jmp	.LB0

.LB27:
	mul_taun(0,600)
	add_product()

	mul_taun(16,576)
	add_product()

	mul_taun(32,552)
	add_product()

	mul_taun(48,528)
	add_product()

	mul_taun(64,504)
	add_product()

	mul_taun(80,480)
	add_product()

	mul_taun(96,456)
	add_product()

	mul_taun(112,432)
	add_product()

	mul_taun(128,408)
	add_product()

	mul_taun(144,384)
	add_product()

	mul_taun(160,360)
	add_product()

	mul_taun(176,336)
	add_product()

	mul_taun(192,312)
	add_product()

	mul_taun(208,288)
	add_product()

	mul_taun(224,264)
	add_product()

	mul_taun(240,240)
	add_product()

	mul_taun(256,216)
	add_product()

	mul_taun(272,192)
	add_product()

	mul_taun(288,168)
	add_product()

	mul_taun(304,144)
	add_product()

	mul_taun(320,120)
	add_product()

	mul_taun(336,96)
	add_product()

	mul_taun(352,72)
	add_product()

	mul_taun(368,48)
	add_product()

	mul_taun(384,24)
	add_product()

	mul_tau(400,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(416,72)

	jmp	.LB0

.LB28:
	mul_taun(0,624)
	add_product()

	mul_taun(16,600)
	add_product()

	mul_taun(32,576)
	add_product()

	mul_taun(48,552)
	add_product()

	mul_taun(64,528)
	add_product()

	mul_taun(80,504)
	add_product()

	mul_taun(96,480)
	add_product()

	mul_taun(112,456)
	add_product()

	mul_taun(128,432)
	add_product()

	mul_taun(144,408)
	add_product()

	mul_taun(160,384)
	add_product()

	mul_taun(176,360)
	add_product()

	mul_taun(192,336)
	add_product()

	mul_taun(208,312)
	add_product()

	mul_taun(224,288)
	add_product()

	mul_taun(240,264)
	add_product()

	mul_taun(256,240)
	add_product()

	mul_taun(272,216)
	add_product()

	mul_taun(288,192)
	add_product()

	mul_taun(304,168)
	add_product()

	mul_taun(320,144)
	add_product()

	mul_taun(336,120)
	add_product()

	mul_taun(352,96)
	add_product()

	mul_taun(368,72)
	add_product()

	mul_taun(384,48)
	add_product()

	mul_taun(400,24)
	add_product()

	mul_tau(416,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(432,72)

	jmp	.LB0

.LB29:
	mul_taun(0,648)
	add_product()

	mul_taun(16,624)
	add_product()

	mul_taun(32,600)
	add_product()

	mul_taun(48,576)
	add_product()

	mul_taun(64,552)
	add_product()

	mul_taun(80,528)
	add_product()

	mul_taun(96,504)
	add_product()

	mul_taun(112,480)
	add_product()

	mul_taun(128,456)
	add_product()

	mul_taun(144,432)
	add_product()

	mul_taun(160,408)
	add_product()

	mul_taun(176,384)
	add_product()

	mul_taun(192,360)
	add_product()

	mul_taun(208,336)
	add_product()

	mul_taun(224,312)
	add_product()

	mul_taun(240,288)
	add_product()

	mul_taun(256,264)
	add_product()

	mul_taun(272,240)
	add_product()

	mul_taun(288,216)
	add_product()

	mul_taun(304,192)
	add_product()

	mul_taun(320,168)
	add_product()

	mul_taun(336,144)
	add_product()

	mul_taun(352,120)
	add_product()

	mul_taun(368,96)
	add_product()

	mul_taun(384,72)
	add_product()

	mul_taun(400,48)
	add_product()

	mul_taun(416,24)
	add_product()

	mul_tau(432,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(448,72)

	jmp	.LB0

.LB30:
	mul_taun(0,672)
	add_product()

	mul_taun(16,648)
	add_product()

	mul_taun(32,624)
	add_product()

	mul_taun(48,600)
	add_product()

	mul_taun(64,576)
	add_product()

	mul_taun(80,552)
	add_product()

	mul_taun(96,528)
	add_product()

	mul_taun(112,504)
	add_product()

	mul_taun(128,480)
	add_product()

	mul_taun(144,456)
	add_product()

	mul_taun(160,432)
	add_product()

	mul_taun(176,408)
	add_product()

	mul_taun(192,384)
	add_product()

	mul_taun(208,360)
	add_product()

	mul_taun(224,336)
	add_product()

	mul_taun(240,312)
	add_product()

	mul_taun(256,288)
	add_product()

	mul_taun(272,264)
	add_product()

	mul_taun(288,240)
	add_product()

	mul_taun(304,216)
	add_product()

	mul_taun(320,192)
	add_product()

	mul_taun(336,168)
	add_product()

	mul_taun(352,144)
	add_product()

	mul_taun(368,120)
	add_product()

	mul_taun(384,96)
	add_product()

	mul_taun(400,72)
	add_product()

	mul_taun(416,48)
	add_product()

	mul_taun(432,24)
	add_product()

	mul_tau(448,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(464,72)

	jmp	.LB0

.LB31:
	mul_taun(0,696)
	add_product()

	mul_taun(16,672)
	add_product()

	mul_taun(32,648)
	add_product()

	mul_taun(48,624)
	add_product()

	mul_taun(64,600)
	add_product()

	mul_taun(80,576)
	add_product()

	mul_taun(96,552)
	add_product()

	mul_taun(112,528)
	add_product()

	mul_taun(128,504)
	add_product()

	mul_taun(144,480)
	add_product()

	mul_taun(160,456)
	add_product()

	mul_taun(176,432)
	add_product()

	mul_taun(192,408)
	add_product()

	mul_taun(208,384)
	add_product()

	mul_taun(224,360)
	add_product()

	mul_taun(240,336)
	add_product()

	mul_taun(256,312)
	add_product()

	mul_taun(272,288)
	add_product()

	mul_taun(288,264)
	add_product()

	mul_taun(304,240)
	add_product()

	mul_taun(320,216)
	add_product()

	mul_taun(336,192)
	add_product()

	mul_taun(352,168)
	add_product()

	mul_taun(368,144)
	add_product()

	mul_taun(384,120)
	add_product()

	mul_taun(400,96)
	add_product()

	mul_taun(416,72)
	add_product()

	mul_taun(432,48)
	add_product()

	mul_taun(448,24)
	add_product()

	mul_tau(464,0)
	add_product()

	reduce_5limb()
	reduce_3limb()

	add_msg_block(480,72)

.LB0:
	xorq 	%rdx,%rdx
	movq	64(%rsp),%rax
	movq	$8,%rcx
	divq	%rcx

	cmpq	$0,%rdx
	jg	.L0

	/* if the last block is full */
	cmpq	$0,72(%rsp)
	jg	.L0

	/* if the last block is partial */
	subq	96(%rsp),%r10

.L0:
	mul_taur(0)

	reduce_5limb()
	reduce_3limb()

	jmp	.LF

.LB1:
	movq    0(%rdx),%r14
	movq    8(%rdx),%r15

	movq    0(%rsi),%r13
	movq    8(%rsi),%rax

	cmpq    $1,72(%rsp)
	je      .L2

	cmpq    $8,80(%rsp)
	jle     .L1

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

	reduce_4limb()

	jmp     .LF

.L1:
	xorq    %r11,%r11
	movq    %r13,%rdx

	mulx    %r14,%r8,%r9
	mulx    %r15,%rbx,%r10
	adcx    %rbx,%r9
	adcx    %r11,%r10

	reduce_3limb()

	jmp     .LF

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

	xorq    %rax,%rax

	adcx    %r14,%r10
	adox    %r15,%r11
	adox    %rax,%r12

	reduce_5limb()

.LF:
	reduce_3limb()

	make_unique()

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
