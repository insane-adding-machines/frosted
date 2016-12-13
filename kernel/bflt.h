 /*
 */

#include <frosted_api.h>
#include "flat.h"

#ifndef _BFLT_H_
#define _BFLT_H_

int bflt_load(uint8_t* from, void **reloc_text, void **reloc_data, void **reloc_bss,
              void **entry_point, size_t *stack_size, uint32_t *got_loc, uint32_t *text_len, uint32_t *data_len);

#endif
