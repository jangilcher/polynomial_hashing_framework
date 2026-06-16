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

	/* tau^3 */
	mul_tau_taun(16,32)

	/* tau^5 */
	mul_tau_taun(48,64)

	/* tau^6 */
	mul_tau_taun(64,80)

	/* tau^7 */
	mul_tau_taun(80,96)

	movq 	0(%rsp),%r11
	movq 	8(%rsp),%r12
	movq 	16(%rsp),%r13
	movq 	24(%rsp),%r14
	movq 	32(%rsp),%r15
	movq 	40(%rsp),%rbx
	movq 	48(%rsp),%rbp

	movq 	%r11,%rsp

	ret
