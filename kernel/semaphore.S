
.syntax unified

/* Try to decrease the semaphore. 
 * On success, return 0. 
 * On failure, return -1 (sem == 0, try again later).
 */

.global _sem_wait
_sem_wait:
#ifdef __ARM_ARCH_7M__
   LDREX   r1, [r0]
#elif defined(__ARM_ARCH_6M__)
  CPSID    i
  LDR      r1, [r0]
#endif
   CMP      r1, #0           // Test if semaphore holds the value 0
   BEQ     _sem_wait_fail    // If it does, return 0
   SUBS    r1, #1            // If not, decrement temporary copy
#ifdef __ARM_ARCH_7M__
   STREX   r2, r1, [r0]      // Attempt Store-Exclusive
   CMP     r2, #0            // Check if Store-Exclusive succeeded
   BNE     _sem_wait         // If Store-Exclusive failed, retry from start
   DMB                       // Required before accessing protected resource
#elif defined(__ARM_ARCH_6M__)
   STR     r1, [r0]
   CPSIE   i
#endif
   MOVS    r0, #0            // Successfully acquired.
   BX      lr
_sem_wait_fail:
#ifdef __ARM_ARCH_7M__
   DMB
   MOV     r0, #-1           // Already locked!
#elif defined(__ARM_ARCH_6M__)
   CPSIE   i
   MOVS    r2, #0            // Already locked!
   MVNS    r0, r2
#endif
   BX      lr

/* Increase the semaphore value. 
 * If the value was 0, return 1 (send a signal to the listeners)
 * If a non-zero semaphore is incremented, return 0. (do not signal).
 */

.global _sem_post
_sem_post:
#ifdef __ARM_ARCH_7M__
   LDREX   r1, [r0]
#elif defined(__ARM_ARCH_6M__)
   CPSID    i
   LDR      r1, [r0]
#endif
   ADDS    r1, #1           // Increment temporary copy
#ifdef __ARM_ARCH_7M__
   STREX   r2, r1, [r0]     // Attempt Store-Exclusive
   CMP     r2, #0           // Check if Store-Exclusive succeeded
   BNE     _sem_post        // Store failed - retry immediately
#elif defined(__ARM_ARCH_6M__)
   STR     r1, [r0]
   CPSIE   i
#endif
   CMP     r0, #1           // Store successful - test if incremented from zero
   DMB                      // Required before releasing protected resource
   BGE     _sem_signal_up   // If initial value was 0, signal update
   MOVS    r0, #0
   BX      lr
_sem_signal_up:
   MOVS    r0, #1           // Semaphore was 0, send a signal to listeners.
   BX      lr

