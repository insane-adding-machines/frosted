.syntax unified

.global _syscall
_syscall:
    nop
    svc 0
    nop
    bx lr
