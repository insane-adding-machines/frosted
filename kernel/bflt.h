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
    void *orig_text;  /* address in ROM of the .text start */
    void *reloc_text; /* relocated address of .text (if relocated), otherwise same as orig_text (for XIP) */
    void *reloc_data; /* relocated address of .data (always relocated)
                         also location of static base (a.k.a r9/v6 register for -msingle-pic-base) */
    void *reloc_bss;  /* relocated address of .bss (always relocated) */

    uint32_t *relocations_start;
    int relocations_count;

    void *text_offset;      /* offset of .text inside bFLT file */
    void *data_offset;      /* offset of .data inside bFLT file */
    void *data_offset_end;  /* offset of end of .data inside bFLT file */
    void *bss_offset_end;   /* offset of end of .bss inside bFLT file */

    uint32_t stack_size;
    uint32_t text_len;
    uint32_t data_len;
    uint32_t bss_len;

    struct process_data_table *pdt;
};

int bflt_load(const void *from, struct bflt_info *info);

#endif
