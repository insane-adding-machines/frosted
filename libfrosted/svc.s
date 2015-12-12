    .syntax unified
    .thumb
    .text
    .align 2                // Align the function code to a 4-byte (2^n) word boundary.
    .thumb                  // Use THUMB-2 instrctions instead of ARM.
    .globl _syscall         // Make the function globally accessible.
    .thumb_func // Use THUMB-2 for the following function.

_syscall:
    #save_r0
    push {r0};
    #save_context
    mrs r0, PSP
    stmdb r0!, {r4-r11}
    msr PSP, r0
    #restore_r0
    add r0, r0, #32
    ldr r0, [r0] 
    #actual SV Call
    isb
    svc 0
    #restore_context
    mrs r1, PSP
    ldmfd r1!, {r4-r11}
    #compensate_for_push
    add r1,r1, #4
    msr PSP, r1

    bx  lr      // Jump back to the caller.

