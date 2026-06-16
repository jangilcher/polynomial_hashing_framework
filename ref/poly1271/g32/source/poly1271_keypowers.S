/* assembly to compute the key powers */

#include "poly1271_macro.h"

	.p2align 5
	.globl poly1271keypowers

poly1271keypowers:

	movq 	%rsp,%r11
	andq    $-32,%rsp
	subq 	$56,%rsp

	movq 	%r11,0(%rsp)
	movq 	%r12,8(%rsp)
	movq 	%r13,16(%rsp)
	movq 	%r14,24(%rsp)
	movq 	%r15,32(%rsp)
	movq 	%rbx,40(%rsp)
	movq 	%rbp,48(%rsp)

	movq    0(%rsi),%rax
	movq    8(%rsi),%rcx

	/* tau */
	movq    %rax,0(%rdi)
	movq    %rcx,8(%rdi)

	/* tau^2 */
	tau_squaren(16)

	/* tau^4 */
	tau_squaren(48)

	/* tau^8 */
	tau_squaren(112)

	/* tau^16 */
	tau_squaren(240)

	/* tau^32 */
	tau_squaren(496)

	/* tau^3 */
	mul_tau_taun(16,32)

	/* tau^5 */
	mul_tau_taun(48,64)

	/* tau^6 */
	mul_tau_taun(64,80)

	/* tau^7 */
	mul_tau_taun(80,96)

	/* tau^9 */
	mul_tau_taun(112,128)

	/* tau^10 */
	mul_tau_taun(128,144)

	/* tau^11 */
	mul_tau_taun(144,160)

	/* tau^12 */
	mul_tau_taun(160,176)

	/* tau^13 */
	mul_tau_taun(176,192)

	/* tau^14 */
	mul_tau_taun(192,208)

	/* tau^15 */
	mul_tau_taun(208,224)

	/* tau^17 */
	mul_tau_taun(240,256)

	/* tau^18 */
	mul_tau_taun(256,272)

	/* tau^19 */
	mul_tau_taun(272,288)

	/* tau^20 */
	mul_tau_taun(288,304)

	/* tau^21 */
	mul_tau_taun(304,320)

	/* tau^22 */
	mul_tau_taun(320,336)

	/* tau^23 */
	mul_tau_taun(336,352)

	/* tau^24 */
	mul_tau_taun(352,368)

	/* tau^25 */
	mul_tau_taun(368,384)

	/* tau^26 */
	mul_tau_taun(384,400)

	/* tau^27 */
	mul_tau_taun(400,416)

	/* tau^28 */
	mul_tau_taun(416,432)

	/* tau^29 */
	mul_tau_taun(432,448)

	/* tau^30 */
	mul_tau_taun(448,464)

	/* tau^31 */
	mul_tau_taun(464,480)

	movq 	0(%rsp),%r11
	movq 	8(%rsp),%r12
	movq 	16(%rsp),%r13
	movq 	24(%rsp),%r14
	movq 	32(%rsp),%r15
	movq 	40(%rsp),%rbx
	movq 	48(%rsp),%rbp

	movq 	%r11,%rsp

	ret
