/* assembly to compute the key powers */

#include "poly1305_macro.h"

	.p2align 5
	.globl poly1305keypowers

poly1305keypowers:

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

	movq    0(%rsi),%r14
	movq    8(%rsi),%r15

	/* tau */
	movq    %r14,0(%rdi)
	movq    %r15,8(%rdi)
	movq    $0,16(%rdi)

	/* tau^2 */
	tau_square()

	/* tau^4 */
	tau_squaren(3)

	/* tau^8 */
	tau_squaren(7)

	/* tau^16 */
	tau_squaren(15)

	/* tau^3 */
	mul_tau_taun(1,2)

	/* tau^5 */
	mul_tau_taun(3,4)

	/* tau^6 */
	mul_tau_taun(4,5)

	/* tau^7 */
	mul_tau_taun(5,6)

	/* tau^9 */
	mul_tau_taun(7,8)

	/* tau^10 */
	mul_tau_taun(8,9)

	/* tau^11 */
	mul_tau_taun(9,10)

	/* tau^12 */
	mul_tau_taun(10,11)

	/* tau^13 */
	mul_tau_taun(11,12)

	/* tau^14 */
	mul_tau_taun(12,13)

	/* tau^15 */
	mul_tau_taun(13,14)

	movq 	0(%rsp),%r11
	movq 	8(%rsp),%r12
	movq 	16(%rsp),%r13
	movq 	24(%rsp),%r14
	movq 	32(%rsp),%r15
	movq 	40(%rsp),%rbx
	movq 	48(%rsp),%rbp

	movq 	%r11,%rsp

	ret
