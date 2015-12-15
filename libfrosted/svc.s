    .syntax unified
    .thumb
    .text
    .align 2                // Align the function code to a 4-byte (2^n) word boundary.
    .thumb                  // Use THUMB-2 instrctions instead of ARM.
    .globl _syscall         // Make the function globally accessible.
    .thumb_func // Use THUMB-2 for the following function.

_syscall:
    #save_context
    push {r4-r11}

    #actual SV Call
    isb
    svc 0

    #restore_context
    pop {r4-r11}

    bx  lr      // Jump back to the caller.

