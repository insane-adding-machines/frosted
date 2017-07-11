
.syntax unified

/* Lock function.
 * On success, return 0. 
 * On failure, return -1 (Locked, try again later).
 */

.global _mutex_lock
_mutex_lock:
#ifdef __ARCH_V7M__
   LDREX   r1, [r0]
#elif defined(__ARCH_V6M__)
   CPSID   i
   LDR     r1, [r0]
#endif
   CMP     r1, #0             // Test if mutex holds the value 0
   BEQ     _mutex_lock_fail   // If it does, return 0
   SUBS    r1, #1             // If not, decrement temporary copy
#ifdef __ARM_ARCH_7M__
   STREX   r2, r1, [r0]       // Attempt Store-Exclusive
   CMP     r2, #0             // Check if Store-Exclusive succeeded
   BNE     _mutex_lock        // If Store-Exclusive failed, retry from start
   DMB                        // Required before accessing protected resource
#elif defined(__ARM_ARCH_6M__)
   STR     r1, [r0]
   CPSIE   i
#endif
   MOVS    r0, #0             // Successfully locked.
   BX      lr
_mutex_lock_fail:
#ifdef __ARM_ARCH_7M__
   DMB
   MOV     r0, #-1              // Already locked!
#elif defined(__ARM_ARCH_6M__)
   CPSIE   i
   MOVS    r2, #0               // Already locked!
   MVNS    r0, r2
#endif
   BX      lr

/* Unlock mutex. 
 * On success, return 0. 
 * On failure, return -1 (Already unlocked!).
 */

.global _mutex_unlock
_mutex_unlock:
#ifdef __ARM_ARCH_7M__
   LDREX   r1, [r0]
#elif defined(__ARM_ARCH_6M__)
   CPSID    i
   LDR      r1, [r0]
#endif
   CMP     r1, #0               // Test if mutex holds the value 0
   BNE     _mutex_unlock_fail   // If it does not, it's already locked!
   ADDS    r1, #1               // Increment temporary copy
#ifdef __ARM_ARCH_7M__
   STREX   r2, r1, [r0]         // Attempt Store-Exclusive
   CMP     r2, #0               // Check if Store-Exclusive succeeded
   BNE     _mutex_unlock        // Store failed - retry immediately
   DMB                          // Required before releasing protected resource
#elif defined(__ARM_ARCH_6M__)
   STR     r1, [r0]
   CPSIE   i
#endif
   MOVS    r0, #0               // Successfully unlocked.
   BX      lr
_mutex_unlock_fail:
#ifdef __ARM_ARCH_7M__
   DMB
   MOV     r0, #-1              // Already locked!
#elif defined(__ARM_ARCH_6M__)
   CPSIE   i
   MOVS    r2, #0               // Already locked!
   MVNS    r0, r2
#endif
   BX      lr

