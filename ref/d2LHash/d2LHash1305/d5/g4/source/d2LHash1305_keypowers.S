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
	tau_squaren(4)

	/* tau^16 */
	tau_squaren(5)

	/* gamma = tau^32 */
	tau_squaren(6)

	/* gamma^2 */
	tau_squaren(7)

	/* gamma^4 */
	tau_squaren(9)

	/* tau^3 */
	mul_tau(1,2)

	/* gamma^3 */
	mul(6,7,8)

	movq 	0(%rsp),%r11
	movq 	8(%rsp),%r12
	movq 	16(%rsp),%r13
	movq 	24(%rsp),%r14
	movq 	32(%rsp),%r15
	movq 	40(%rsp),%rbx
	movq 	48(%rsp),%rbp

	movq 	%r11,%rsp

	ret
