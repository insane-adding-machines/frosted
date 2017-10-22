/*
 * bFLT header
 */

#ifndef _BFLT_H_
#define _BFLT_H_

#include <frosted_api.h>
#include "flat.h"

#include <stdint.h>

struct bflt_info {
    void *entry_point;
    void *orig_text;
    void *orig_data;

    void *reloc_text;
    void *reloc_data; /* also location of static base (a.k.a r9/v6 register for -msingle-pic-base) */
    void *reloc_bss;

    uint32_t *relocations_start;
    int relocations_count;

    void *text_offset;
    void *data_offset;
    void *data_offset_end;
    void *bss_offset_end;

    uint32_t stack_size;
    uint32_t text_len;
    uint32_t data_len;
    uint32_t bss_len;
    struct process_data_table *pdt;
};

int bflt_load(const void *from, struct bflt_info *info);

#endif
