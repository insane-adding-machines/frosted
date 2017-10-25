/*  
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as 
 *      published by the free Software Foundation.
 *      
 *
 *      frosted is distributed in the hope that it will be useful,
 *      but WITHOUT ANY WARRANTY; without even the implied warranty of
 *      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *      GNU General Public License for more details.
 *
 *      You should have received a copy of the GNU General Public License
 *      along with frosted.  If not, see <http://www.gnu.org/licenses/>.
 *
 *      Authors: Daniele Lacamera, Maxime Vincent
 *
 */  
#include "frosted.h"
#include "flat.h"
#include "bflt.h"
#include "kprintf.h"
#include "vfs.h"
#include "libopencmsis/core_cm3.h"
#include "unicore-mx/cm3/systick.h"

#include <stdio.h>

/*************************
 * bFLT start 
 *************************/
#define RELOC_FAILED 0xff00ff01     /* Relocation incorrect somewhere */

#ifndef CONFIG_BINFMT_SHARED_FLAT
#define CONFIG_BINFMT_SHARED_FLAT   /* TODO: make optional */
#endif

#define GDB_PATH "frosted-userland/gdb/"

/* Process Data Table (PDT) is used by the shared lib infrastructure
 * to find the static_base (r9 single-pic-base) value for each library,
 * and it's text_base (start of code) value.
 * Both are required in the asm veneers, which are inserted when calling
 * libraries from a bFLT executable.
 */
struct __attribute__((packed)) shared_lib {
    void *base;
    uint32_t build_date;
};

struct __attribute__((packed)) base_addr {
    void * static_base;
    void * text_base;
    void * scratch_lr;
    void * padding;
};

struct __attribute__((packed)) process_data_table {
    struct base_addr base[MAX_SHARED_LIBS];
};

/* lib 0 = reserved for the running app, so in the cache, all the indices are -1 */
static struct shared_lib lib_cache[MAX_SHARED_LIBS-1];

static inline uint32_t long_be(uint32_t le)
{
    uint8_t *b = (uint8_t *)&le;
    uint32_t be = 0;
    uint32_t b0, b1, b2;
    b0 = b[0];
    b1 = b[1];
    b2 = b[2];
    be = b[3] + (b2 << 8) + (b1 << 16) + (b0 << 24);
    return be;
}

static void load_header(struct flat_hdr * to_hdr, struct flat_hdr * from_hdr) {
    memcpy((uint8_t*)to_hdr, (uint8_t*)from_hdr, sizeof(struct flat_hdr));
}

static int check_header(struct flat_hdr * header) {
    if (memcmp(header->magic, "bFLT", 4) != 0) {
        kprintf("bFLT: Magic number does not match\r\n");
        return -1;
    }
    if (long_be(header->rev) != FLAT_VERSION){
        kprintf("bFLT: Version number does not match\r\n");
        return -1;
    }

    /* check for unsupported flags */
    if (long_be(header->flags) & (FLAT_FLAG_GZIP | FLAT_FLAG_GZDATA)) {
        kprintf("bFLT: Unsupported flags detected - GZip'd data is not supported\r\n");
        return -1;
    }
    return 0;
}

static uint32_t * calc_reloc(const uint8_t * base, uint32_t offset)
{
    /* the library id is in top byte of offset */
    int id = (offset >> 24) & 0x000000FFu;
    if (id)
    {
        kprintf("bFLT: Found reloc to shared library id %d\r\n", id);
        return (uint32_t *)RELOC_FAILED;
    }
    return (uint32_t *)(base + (offset & 0x00FFFFFFu));
}

static void * load_shared_lib(struct bflt_info *info, int lib_id)
{
    /* TODO: Re-load a new DATA and BSS for each instance! */
    struct bflt_info libinfo = {};
    struct fnode *f;
    size_t stack_size;
    char path[MAX_FILE];
    uint8_t *bflt_file_base;

    if ( (lib_id >= MAX_SHARED_LIBS) || (lib_id <= 0) ) {
        kprintf("Invalid lib_id specified: %d\r\n", lib_id);
        return NULL;
    }

    /* Open /lib/lib[lib_id].so */
    ksprintf(path, "/lib/lib%d.so", lib_id);
    f = fno_search(path);
    if (!f) {
        ksprintf(path, "/bin/lib%d.so", lib_id);
        f = fno_search(path);
    }
    if (!f || !f->owner || !f->owner->ops.mmap) {
        return NULL;
    }

    bflt_file_base = f->owner->ops.mmap(f);
    if (!bflt_file_base)
        return NULL;

    libinfo.pdt = info->pdt; /* lib loading should use the main executable's PDT */
    if (bflt_load(bflt_file_base, &libinfo))
        return NULL;

    kprintf("bflt_lib: GDB: add-symbol-file %slib%d.so.gdb 0x%p -s .data 0x%p -s .bss 0x%p\n", GDB_PATH, lib_id, libinfo.reloc_text, libinfo.reloc_data, libinfo.reloc_bss);

    /* fill in static_base (PIC register value) and text_base of this library instance in PDT */
    info->pdt->base[lib_id].static_base = libinfo.reloc_data;
    info->pdt->base[lib_id].text_base = libinfo.reloc_text;

    return libinfo.reloc_text;
}

static int process_got_relocs(struct bflt_info *info)
{
    /*
     * Addresses in header are relative to start of FILE (so including flat_hdr)
     * Addresses in the relocs are relative to start of .text (so excluding flat_hdr)
     */
    uint32_t * rp;

    for (rp = (uint32_t *)info->reloc_data; *rp != 0xffffffff; rp++) {
        uint32_t reloc = *rp;
        /* this will remap pointers starting from address 0x0,
         * to wherever they are actually loaded in the memory map (.text reloc) */
        if (reloc) {
            uint32_t addr = RELOC_FAILED;
            int lib_id = (reloc >> 24u) & 0xFF;
            if (lib_id)
            {
                uint8_t * lib_base = lib_cache[lib_id - 1].base;
                kprintf("bFLT: Found GOT reloc to shared library id %d\r\n", lib_id);

                /* Load shared library if needed */
                if (lib_base == NULL)
                {
                    lib_base = load_shared_lib(info, lib_id);
                    if (!lib_base) {
                        kprintf("Library lib%d.so could not be loaded\r\n", lib_id);
                        return -2;
                    }
                    lib_cache[lib_id -1].base = lib_base;
                    /* TODO: add date to cache + compare with executable build date! */

                    /* TODO:
                     * - Do NOT reload same lib twice for one executable
                     * - DO reload the same lib again for another executable!
                     */
                }

                /* perform reloc (.text only for now) */
                reloc &= 0x00FFFFFFu;
                /* perform .text reloc */
                addr = (uint32_t)calc_reloc(lib_base, reloc);
                // XXX FIXME: will break if there's a reloc to library's .data or .bss!! */
                addr |= 0x00000001u; /* relocs in .text must have the bit0 set! */
                /* XXX: what about .data and .bss relocs form exec ->> shared lib?? */
                // XXX need something like this: if (reloc < (uint32_t)info->data_offset) {
            } else {
                if (reloc < (uint32_t)info->data_offset) {
                    /* reloc is in .text section: BASE == text_start  -- addr == relative to .text */
                    addr = (uint32_t)calc_reloc(info->reloc_text, reloc);
                } else if (reloc < (uint32_t)info->bss_offset_end) {
                    /* reloc is in .data section: BASE == data_start  -- addr == relative to .text - (start of data) */
                    addr = (uint32_t)calc_reloc(info->reloc_data, reloc - (uint32_t)info->data_offset);
                }
            }
            if (addr == RELOC_FAILED) {
                //errno = -ENOEXEC;
                return -1;
            }
            *rp = addr; /* Store translated address in GOT */
        }
    }
    return 0;
}

/* 
 * Works only for FLAT v4
 * reloc starts just after data_end
 *
 * Relocations in the table point to an address,
 * at that address(1), there is an adress(2) that needs fixup,
 * and needs to be written at it's original address(1), after it's been fixed
 *
 * Addresses in header are relative to start of FILE (so including flat_hdr)
 * Addresses in the relocs are relative to start of .text (so excluding flat_hdr)
 */
int process_relocs(struct bflt_info *info)
{
    int i;
    /*
     * Now run through the relocation entries.
     * We've got to be careful here as C++ produces relocatable zero
     * entries in the constructor and destructor tables which are then
     * tested for being not zero (which will always occur unless we're
     * based from address zero).  This causes an endless loop as __start
     * is at zero.  The solution used is to not relocate zero addresses.
     * This has the negative side effect of not allowing a global data
     * reference to be statically initialised to _stext (I've moved
     * __start to address 4 so that is okay).
     */
    for (i=0; i < info->relocations_count; i++) {
        uint32_t addr, *fixup_addr;
        uint32_t *relocd_addr = (uint32_t *)RELOC_FAILED;
        
        /* Get the address of the pointer to be
           relocated (of course, the address has to be
           relocated first).  */
        fixup_addr = (uint32_t *)long_be(info->relocations_start[i]);
        /* two cases: reloc_addr < text_end: For now we only support GOTPIC, which cannot have relocations in .text segment!
         *        or  reloc_addr > data_start
         */
        /* TODO: Make common between GOT and regular relocs ? */
        if ((uint32_t)fixup_addr < (uint32_t)info->data_offset)
        {
            /* FAIL -- non GOTPIC, cannot write to ROM/.text */
            return -1;
        } else if ((uint32_t)fixup_addr < (uint32_t)info->data_offset_end) {
            /* Reloc is in .data section (must be for GOTPIC), now make this point to the .data source (in the DEST ram!), and dereference */
            fixup_addr = calc_reloc((uint8_t *)((uint32_t)info->reloc_data - (uint32_t)info->data_offset), (uint32_t)fixup_addr);
            if (fixup_addr == (uint32_t *)RELOC_FAILED)
                return -1;

            /* Again 2 cases: reloc points to .text -- or to .data/.bss */
            if (*fixup_addr < (uint32_t)info->data_offset) {
                /* reloc is in .text section: BASE == text_start  -- addr == relative to .text */
                relocd_addr = calc_reloc((uint8_t *)info->orig_text, *fixup_addr);
            } else if (*fixup_addr < (uint32_t)info->bss_offset_end) {
                /* reloc is in .data section: BASE == data_start  -- addr == relative to .text - (start of data) */
                relocd_addr = calc_reloc((uint8_t *)info->reloc_data, *fixup_addr - (uint32_t)info->data_offset);
            } else {
                relocd_addr = (uint32_t *)RELOC_FAILED;
                return -1;
            }
            /* write the relocated/offsetted value back were it was read */
            *fixup_addr = (uint32_t)relocd_addr;
        }

        if (relocd_addr == (uint32_t *)RELOC_FAILED) {
            kprintf("bFLT: Unable to calculate relocation address\r\n");
            return -1;
        }

    }

    return 0;
}


/* BFLT file structure:
 *
 * +------------------------+   0x0
 * | BFLT header            |
 * +------------------------+
 * | padding                |
 * +------------------------+   entry
 * | .text section          |
 * |                        |
 * +------------------------+   data_start
 * | .data section          |
 * |                        |   
 * +------------------------+   data_end, relocs_start, bss_start
 * | relocations (and .bss) |
 * |........................|   relocs_end   <- BFLT ends here
 * | (.bss section)         |
 * +------------------------+   bss_end
 */

/****
 * XXX TODO:
 * - shared libs now require a different r9 pointer, because their GOT table is located elsewhere.
 * - this is a problem, because this means we should change the r9 pointer every time the code execution jumps from exec -> lib -> exec, etc...
 * - it's possible to introduce a veneer to do this, but this does not solve the problem in case the library want to call a call-back function, located in the application and not in the lib...
 */

int bflt_load(const void *bflt_src, struct bflt_info *info)
{
    struct flat_hdr hdr;
    uint32_t flags, alloc_len, entry_point_offset;
    int32_t rev;
    void * mem = NULL;

    if ((!info) || (!bflt_src)) {
        goto error;
    }

    load_header(&hdr, (struct flat_hdr *)bflt_src);
    if (check_header(&hdr) != 0) {
        kprintf("bFLT: Bad FLT header\r\n");
        goto error;
    }

    /* Calculate all the sizes */
    info->orig_text         = (void *)((uint32_t)bflt_src + sizeof(struct flat_hdr));
    info->text_offset       = (void *)sizeof(struct flat_hdr);
    info->data_offset       = (void *)(long_be(hdr.data_start) - sizeof(struct flat_hdr));
    info->data_offset_end   = (void *)(long_be(hdr.data_end) - sizeof(struct flat_hdr));
    info->bss_offset_end    = (void *)(long_be(hdr.bss_end) - sizeof(struct flat_hdr));
    info->text_len          = long_be(hdr.data_start) - sizeof(struct flat_hdr);
    info->data_len          = long_be(hdr.data_end) - long_be(hdr.data_start);
    info->bss_len           = long_be(hdr.bss_end) - long_be(hdr.data_end);
    info->stack_size        = long_be(hdr.stack_size);
    flags                   = long_be(hdr.flags);
    rev                     = long_be(hdr.rev);
    /* relocation data */
    info->relocations_start = (void *)((uint32_t)bflt_src + long_be(hdr.reloc_start));
    info->relocations_count = long_be(hdr.reloc_count);
    /* Calculate source addresses */
    entry_point_offset      = (long_be(hdr.entry) & 0xFFFFFFFE) - sizeof(struct flat_hdr); /* offset inside .text + reset THUMB bit */

    /*
     * calculate the extra space we need to malloc
     */
    /* relocs are located in the .bss part of the BFLT binary, so we need whichever is biggest */
    if ((info->relocations_count * sizeof(uint32_t)) > info->bss_len)
        alloc_len = info->relocations_count * sizeof(uint32_t);
    else
        alloc_len = info->bss_len;
    alloc_len += info->data_len;

    /* extra space needed for the shared lib static-base pointers (r9/v6)
     * a.k.a. "process data table" */
#ifdef CONFIG_BINFMT_SHARED_FLAT
    /* for the pointer to PDT */
    alloc_len += sizeof(void *);
    /* for the actual PDT (only main executable, not for shared libs) */
    if (info->pdt == NULL) {
        alloc_len += sizeof(struct process_data_table);
    }
#endif

    /*
     * there are a couple of cases here:
     *  -> the fully copied to RAM case which lumps it all together (RAM flag)
     *  -> the separate code/data case (GOTPIC flag, w/o RAM flag)
     */
    if (flags & FLAT_FLAG_GZIP) {
        kprintf("bFLT: GZIP compression not supported\r\n");
        goto error;
    }

    if (flags & FLAT_FLAG_GOTPIC) {
        uint8_t  *mem, *copy_src, *copy_dst;
        uint32_t data_offset = 0;
        uint32_t copy_len = info->data_len;

        if (flags & FLAT_FLAG_RAM) {
            alloc_len += info->text_len;
            copy_len += info->text_len;
            data_offset = info->text_len;
        }

        /* Allocate enough memory for .data, .bss and possibly PDT and .text */
        mem = u_malloc(alloc_len);
        if (!mem)
        {
            kprintf("bFLT: Could not allocate enough memory for process\r\n");
            goto error;
        }
        copy_dst = mem;

#ifdef CONFIG_BINFMT_SHARED_FLAT
        /* move copy_dst after PDT pointer */
        copy_dst += sizeof(void *);
        /* move copy_dst after PDT table and PDT pointer (only for main executable, not for shared libs) */
        if (info->pdt == NULL) {
            copy_dst += sizeof(struct process_data_table);
        }
#endif

        /* .text is only relocated when RAM flag is set */
        if (flags & FLAT_FLAG_RAM) {
            info->reloc_text = copy_dst;
            copy_src = (uint8_t *)info->orig_text;
        } else {
            info->reloc_text = (uint8_t *)info->orig_text;
            copy_src = (uint8_t *)info->orig_text + info->text_len;
        }

#ifdef CONFIG_BINFMT_SHARED_FLAT
        if (info->pdt == NULL) {
            /* main executable */
            struct process_data_table ** pdt_ptr = (void *)(mem + sizeof(struct process_data_table));
            info->pdt = (void *)mem;
            *pdt_ptr = info->pdt;
            memset(info->pdt, 0, sizeof(struct process_data_table));
            info->pdt->base[0].static_base = copy_dst + data_offset; /* save static base in PDT */
            info->pdt->base[0].text_base = info->reloc_text;
        } else {
            /* shared lib: PDT is not modified, this is done by the caller of this function, using the 'info' struct */
            struct process_data_table ** pdt_ptr = (void *)mem;
            *pdt_ptr = info->pdt;
        }
#endif

        /* .data is always relocated */
        info->reloc_data = copy_dst + data_offset;
        info->reloc_bss = (uint8_t *)info->reloc_data + info->data_len;
        info->entry_point = (uint8_t *)info->reloc_text + entry_point_offset;

        /* copy segments .data segment and possibly .text */
        memcpy(copy_dst, copy_src, copy_len);
        /* zero-init .bss */
        memset(info->reloc_bss, 0, info->bss_len);
    } else {
        /* GZIP or FULL RAM bFLTs not supported for now */
        kprintf("bFLT: Only GOTPIC bFLT binaries are supported\r\n");
        goto error;
    }


    /*
     * Two sections of relocation entries:
     * 1) the GOT which resides at the beginning of the data segment
     *    and is terminated with a -1.  This one can be relocated in place.
     * 2) Extra relocation entries tacked after the image's data segment.
     *    These require a little more processing as the entry is
     *    really an offset into the image which contains an offset into the
     *    image.
     */

    /* 1. GOT relocations */
    if (flags & FLAT_FLAG_GOTPIC) {
        if (process_got_relocs(info)) goto error;
    }

    /* 2. Extra relocations */
    process_relocs(info);

    return 0;

error:
    if (mem) kfree(mem);
    info->reloc_text  = NULL;
    info->reloc_data  = NULL;
    info->reloc_bss   = NULL;
    info->entry_point = NULL;
    kprintf("bFLT: Caught error - exiting\r\n");
    return -1;
}

