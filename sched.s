.syntax unified

.global _syscall
_syscall:
    push {r0-r3, r7, r12}
    svc 0
    pop {r0-r3, r7, r12}
    bx lr
