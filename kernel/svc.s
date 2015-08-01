.syntax unified

.global _syscall
_syscall:
    nop
    svc 0
//    mov r0, r9 /* return value (r0) is put in r9 by SVC_Handler */
    nop
    bx lr
