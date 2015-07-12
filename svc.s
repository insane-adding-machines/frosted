.syntax unified

.global _syscallp
_syscallp:
    push {r0-r3, r7, r12}
    svc 0
    pop {r0-r3, r7, r12}
    bx lr

.global _syscall
_syscall:
    nop
    svc 0
    nop
    bx lr
