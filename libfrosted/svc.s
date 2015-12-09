.syntax unified

.text

.global _syscall
_syscall:
save_r0:
    push {r0};

save_context:
    mrs r0, PSP
    stmdb r0!, {r4-r11}
    msr PSP, r0

restore_r0:
    add r0, r0, #32
    ldr r0, [r0] 

    isb
    svc 0

restore_context:
    mrs r1, PSP
    ldmfd r1!, {r4-r11}
compensate_for_push:
    add r1,r1, #4
    msr PSP, r1

    isb
    bx lr

