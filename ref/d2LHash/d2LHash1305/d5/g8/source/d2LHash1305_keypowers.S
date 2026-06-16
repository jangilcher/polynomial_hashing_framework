/* assembly to compute the key powers */

#include "d2LHash1305_macro.h"

	.p2align 5
	.globl d2LHash1305keypowers

d2LHash1305keypowers:

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
	tau_squaren(8)

	/* gamma = tau^32 */
	tau_squaren(9)

	/* gamma^2 */
	tau_squaren(10)

	/* gamma^4 */
	tau_squaren(12)

	/* gamma^8 */
	tau_squaren(16)

	/* tau^3 */
	mul_tau(1,2)

	/* tau^5 */
	mul_tau(3,4)

	/* tau^6 */
	mul_taur(5)

	/* tau^7 */
	mul_taur(6)

	/* gamma^3 */
	mul(9,10,11)

	/* gamma^5 */
	mul(9,12,13)

	/* gamma^6 */
	mul(9,13,14)

	/* gamma^7 */
	mul(9,14,15)

	movq 	0(%rsp),%r11
	movq 	8(%rsp),%r12
	movq 	16(%rsp),%r13
	movq 	24(%rsp),%r14
	movq 	32(%rsp),%r15
	movq 	40(%rsp),%rbx
	movq 	48(%rsp),%rbp

	movq 	%r11,%rsp

	ret
