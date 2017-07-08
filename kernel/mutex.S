
.syntax unified

/* Lock function.
 * On success, return 0. 
 * On failure, return -1 (Locked, try again later).
 */

.global _mutex_lock
_mutex_lock:
   LDREX   r1, [r0]
   CMP     r1, #0             // Test if mutex holds the value 0
   BEQ     _mutex_lock_fail   // If it does, return 0
   SUB     r1, #1             // If not, decrement temporary copy
   STREX   r2, r1, [r0]       // Attempt Store-Exclusive
   CMP     r2, #0             // Check if Store-Exclusive succeeded
   BNE     _mutex_lock        // If Store-Exclusive failed, retry from start
   DMB                        // Required before accessing protected resource
   MOV     r0, #0             // Successfully locked.
   BX      lr
_mutex_lock_fail:
   DMB
   MOV     r0, #-1            // Try again.
   BX      lr

/* Unlock mutex. 
 * On success, return 0. 
 * On failure, return -1 (Already unlocked!).
 */

.global _mutex_unlock
_mutex_unlock:
   LDREX   r1, [r0]
   CMP     r1, #0               // Test if mutex holds the value 0
   BNE     _mutex_unlock_fail   // If it does not, it's already locked!
   ADD     r1, #1               // Increment temporary copy
   STREX   r2, r1, [r0]         // Attempt Store-Exclusive
   CMP     r2, #0               // Check if Store-Exclusive succeeded
   BNE     _mutex_unlock        // Store failed - retry immediately
   DMB                          // Required before releasing protected resource
   MOV     r0, #0               // Successfully unlocked.
   BX      lr
_mutex_unlock_fail:
   MOV     r0, #-1              // Already locked!
   BX      lr

