#include <stdint.h>

/* Syscall table is fixed at kernel start + 1K */
int (** __syscall__)( uint32_t, uint32_t, uint32_t, uint32_t, uint32_t ) = (int (**)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t)) (FLASH_ORIGIN + 0x400);

