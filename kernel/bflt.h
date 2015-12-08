 /*
 */

#include <frosted_api.h>
#include "flat.h"

#ifndef _BFLT_H_
#define _BFLT_H_

int bflt_load(uint8_t* from, void **mem_ptr, size_t *mem_size, int (**entry_point)(int,char*[]), size_t *stack_size, uint32_t *got_loc);

#endif
