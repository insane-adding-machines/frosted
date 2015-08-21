
.syntax unified

.global _sem_wait
_sem_wait:
   LDREX   r1, [r0]
   CMP         r1, #0        // Test if semaphore holds the value 0
   BEQ     _sem_wait_fail    // If it does, return 0
   SUB     r1, #1            // If not, decrement temporary copy
   STREX   r2, r1, [r0]      // Attempt Store-Exclusive
   CMP     r2, #0            // Check if Store-Exclusive succeeded
   BNE     _sem_wait         // If Store-Exclusive failed, retry from start
   DMB                       // Required before accessing protected resource
   MOV     r0, r1
   BX      lr
_sem_wait_fail:
   DMB
   MOV     r0, #0
   BX      lr


.global _sem_post
_sem_post:
   LDREX   r1, [r0]
   ADD     r1, #1           // Increment temporary copy
   STREX   r2, r1, [r0]     // Attempt Store-Exclusive
   CMP     r2, #0           // Check if Store-Exclusive succeeded
   BNE     _sem_post        // Store failed - retry immediately
   CMP     r0, #1           // Store successful - test if incremented from zero
   DMB                      // Required before releasing protected resource
   BGE     _sem_signal_up   // If initial value was 0, signal update
   MOV     r0, #0
   BX      lr
_sem_signal_up:
   MOV     r0, #1
   BX      lr

