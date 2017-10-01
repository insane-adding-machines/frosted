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
#include "kprintf.h"
#include "vfs.h"
#include "libopencmsis/core_cm3.h"
#include "unicore-mx/cm3/systick.h"

#include <stdio.h>

/*************************
 * bFLT start 
 *************************/
#define RELOC_FAILED 0xff00ff01     /* Relocation incorrect somewhere */
#define MAX_SHARED_LIB_ID (254)

#define GDB_PATH "frosted-userland/gdb/"

struct shared_lib {
    void *base;
    unsigned long build_date;
};

int bflt_load(char* from, void **reloc_text, void **reloc_data, void **reloc_bss,
              void ** entry_point, size_t *stack_size, unsigned long *got_loc, unsigned long *text_len, unsigned long *data_len);

/* lib 0 = reserved for the running app, so in the cache, all the indices are -1 */
static struct shared_lib lib_cache[MAX_SHARED_LIB_ID-1];

static inline unsigned long long_be(unsigned long le)
{
    char *b = (char *)&le;
    unsigned long be = 0;
    unsigned long b0, b1, b2;
    b0 = b[0];
    b1 = b[1];
    b2 = b[2];
    be = b[3] + (b2 << 8) + (b1 << 16) + (b0 << 24);
    return be;
}

static void load_header(struct flat_hdr * to_hdr, struct flat_hdr * from_hdr) {
    memcpy((char*)to_hdr, (char*)from_hdr, sizeof(struct flat_hdr));
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

static unsigned long * calc_reloc(char * base, unsigned long offset)
{
    /* the library id is in top byte of offset */
    int id = (offset >> 24) & 0x000000FFu;
    if (id)
    {
        kprintf("bFLT: Found reloc to shared library id %d\r\n", id);
        return (unsigned long *)RELOC_FAILED;
    }
    return (unsigned long *)(base + (offset & 0x00FFFFFFu));
}

static void * load_shared_lib(int lib_id)
{
    /* Should we load TEXT + DATA + BSS?  -- HOW TO MANAGER DIFFERENT RELOCS? */
    /* Re-load DATA and BSS for each instance? */
    unsigned long * reloc_text, reloc_data, reloc_bss, entry_point;
    unsigned long text_len, got_loc, data_len;
    struct fnode *f;
    size_t stack_size;
    char path[MAX_FILE];
    char *bflt_file_base; // Open the file, first!

    if ( (lib_id >= MAX_SHARED_LIB_ID) || (lib_id <= 0) ) {
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

    if (bflt_load(bflt_file_base, (void **)&reloc_text, (void **)&reloc_data, (void **)&reloc_bss, (void **)&entry_point, &stack_size, &got_loc, &text_len, &data_len))
        return NULL;

    kprintf("bflt_lib: GDB: add-symbol-file %s/lib%d.so.gdb 0x%p -s .data 0x%p -s .bss 0x%p\n", GDB_PATH, lib_id, reloc_text, reloc_data, reloc_bss);

    return reloc_text;
}

static int process_got_relocs(struct flat_hdr * hdr, char * base, char * got_start)
{
    /*
     * Addresses in header are relative to start of FILE (so including flat_hdr)
     * Addresses in the relocs are relative to start of .text (so excluding flat_hdr)
     */
    unsigned long data_start = long_be(hdr->data_start) - sizeof(struct flat_hdr);
    unsigned long bss_end = long_be(hdr->bss_end) - sizeof(struct flat_hdr);
    unsigned long * rp;
    char * text_start_dest = base + sizeof(struct flat_hdr);
    char * data_start_dest = got_start;

    for (rp = (unsigned long *)got_start; *rp != 0xffffffff; rp++) {
        unsigned long reloc = *rp;
        /* this will remap pointers starting from address 0x0, to wherever they are actually loaded in the memory map (.text reloc) */
        if (reloc) {
            unsigned long addr = RELOC_FAILED;
            int lib_id = (reloc >> 24u) & 0xFF;
            if (lib_id)
            {
                /* Load shared library if needed */
                kprintf("bFLT: Found GOT reloc to shared library id %d\r\n", lib_id);
                if (lib_cache[lib_id -1].base == NULL)
                {
                    /* Needs loading */
                    char * lib_base = load_shared_lib(lib_id);
                    if (!lib_base) {
                        kprintf("Library lib%d.so could not be loaded\r\n", lib_id);
                        return -2;
                    }
                    lib_cache[lib_id -1].base = lib_base;
                    /* TODO: add date to cache + compare with executable build date! */

                    reloc &= 0x00FFFFFFu;

                    /* perform .text reloc */
                    addr = (unsigned long)calc_reloc(lib_base, reloc);
                    /* XXX: what about .data and .bss relocs form exec ->> shared lib?? */
                }
            } else {
                if (reloc < data_start) {
                    /* reloc is in .text section: BASE == text_start  -- addr == relative to .text */
                    addr = (unsigned long)calc_reloc(text_start_dest, reloc);
                } else if (reloc < bss_end) {
                    /* reloc is in .data section: BASE == data_start  -- addr == relative to .text - (start of data) */
                    addr = (unsigned long)calc_reloc(data_start_dest, reloc - data_start);
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

/* works only for FLAT v4 */
/* reloc starts just after data_end */
int process_relocs(struct flat_hdr * hdr, unsigned long * base, unsigned long data_start_dest,  unsigned long *reloc, int relocs)
{
    int i;
    /* Relocations in the table point to an address,
     * at that address(1), there is an adress(2) that needs fixup,
     * and needs to be written at it's original address(1), after it's been fixed
     */
    /*
     * Addresses in header are relative to start of FILE (so including flat_hdr)
     * Addresses in the relocs are relative to start of .text (so excluding flat_hdr)
     */
    unsigned long data_start = long_be(hdr->data_start) - sizeof(struct flat_hdr);
    unsigned long data_end = long_be(hdr->data_end) - sizeof(struct flat_hdr); /* relocs must be located in .data segment for GOTPIC */
    unsigned long bss_end = long_be(hdr->bss_end) - sizeof(struct flat_hdr);
    unsigned long text_start_dest = ((unsigned long)base) + sizeof(struct flat_hdr); /* original RELOC is relative to text_start (.bss in ROM/Flash/source) */
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
    for (i=0; i < relocs; i++) {
        unsigned long addr, *fixup_addr;
        unsigned long *relocd_addr = (unsigned long *)RELOC_FAILED;
        
        /* Get the address of the pointer to be
           relocated (of course, the address has to be
           relocated first).  */
        fixup_addr = (unsigned long *)long_be(reloc[i]);
        /* two cases: reloc_addr < text_end: For now we only support GOTPIC, which cannot have relocations in .text segment!
         *        or  reloc_addr > data_start
         */
        /* TODO: Make common between GOT and regular relocs ? */



        if ((unsigned long)fixup_addr < data_start)
        {
            /* FAIL -- non GOTPIC, cannot write to ROM/.text */
            return -1;
        } else if ((unsigned long)fixup_addr < data_end) {
            /* Reloc is in .data section (must be for GOTPIC), now make this point to the .data source (in the DEST ram!), and dereference */
            fixup_addr = (unsigned long *)calc_reloc((char *)((unsigned long)data_start_dest - (unsigned long)data_start), (unsigned long)fixup_addr);
            if (fixup_addr == (unsigned long *)RELOC_FAILED)
                return -1;

            /* Again 2 cases: reloc points to .text -- or to .data/.bss */
            if (*fixup_addr < data_start) {
                /* reloc is in .text section: BASE == text_start  -- addr == relative to .text */
                relocd_addr = (unsigned long *)calc_reloc((char *)text_start_dest, *fixup_addr);
            } else if (*fixup_addr < bss_end) {
                /* reloc is in .data section: BASE == data_start  -- addr == relative to .text - (start of data) */
                relocd_addr = (unsigned long *)calc_reloc((char *)data_start_dest, *fixup_addr - data_start);
            } else {
                relocd_addr = (unsigned long *)RELOC_FAILED;
                return -1;
            }
            /* write the relocated/offsetted value back were it was read */
            *fixup_addr = (unsigned long)relocd_addr;
        }

        if (relocd_addr == (unsigned long *)RELOC_FAILED) {
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

int bflt_load(char* from, void **reloc_text, void **reloc_data, void **reloc_bss,
              void ** entry_point, size_t *stack_size, unsigned long *got_loc, unsigned long *text_len, unsigned long *data_len)
{
    struct flat_hdr hdr;
    void * mem = NULL;
    unsigned long bss_len, stack_len, flags, alloc_len, entry_point_offset;
    char *relocs_src, *text_src, *data_dest;
    char *address_zero = from;
    int32_t relocs, rev;

    //kprintf("bFLT: Loading from 0x%p\r\n", from);

    if (!address_zero) {
        goto error;
    }

    load_header(&hdr, (struct flat_hdr *)address_zero);
    if (check_header(&hdr) != 0) {
        kprintf("bFLT: Bad FLT header\r\n");
        goto error;
    }

    /* Calculate all the sizes */
    *text_len           = long_be(hdr.data_start) - sizeof(struct flat_hdr);
    *data_len           = long_be(hdr.data_end) - long_be(hdr.data_start);
    bss_len             = long_be(hdr.bss_end) - long_be(hdr.data_end);
    stack_len           = long_be(hdr.stack_size);
    relocs              = long_be(hdr.reloc_count);
    flags               = long_be(hdr.flags);
    rev                 = long_be(hdr.rev);
    /* Calculate source addresses */
    text_src            = address_zero + sizeof(struct flat_hdr);
    relocs_src          = address_zero + long_be(hdr.reloc_start);
    entry_point_offset  = (long_be(hdr.entry) & 0xFFFFFFFE) - sizeof(struct flat_hdr); /* offset inside .text + reset THUMB bit */
    *stack_size         = stack_len;

    /*
     * calculate the extra space we need to malloc
     */
    /* relocs are located in the .bss part of the BFLT binary, so we need whichever is biggest */
    if ((relocs * sizeof(unsigned long)) > bss_len)
        alloc_len = relocs * sizeof(unsigned long);
    else
        alloc_len = bss_len;
    alloc_len += *data_len;


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
        char  *mem, *copy_src;
        unsigned long data_offset = 0;
        unsigned long copy_len = *data_len;

        if (flags & FLAT_FLAG_RAM) {
            alloc_len += *text_len;
            copy_len += *text_len;
            data_offset = *text_len;
        }

        /* Allocate enough memory for .data, .bss and possibly .text */
        mem = u_malloc(alloc_len);
        if (!mem)
        {
            kprintf("bFLT: Could not allocate enough memory for process\r\n");
            goto error;
        }

        /* .text is only relocated when RAM flag is set */
        if (flags & FLAT_FLAG_RAM) {
            *reloc_text = mem;
            copy_src = text_src;
        } else {
            *reloc_text = text_src;
            copy_src = text_src + *text_len;
        }
        /* .data is always relocated */
        data_dest = mem + data_offset;

        *entry_point = *reloc_text + entry_point_offset;
        *reloc_data = data_dest;
        *reloc_bss = data_dest + *data_len;

        /* copy segments .data segment and possibly .text */
        memcpy(mem, copy_src, copy_len);
        /* zero-init .bss */
        memset(data_dest + *data_len, 0, bss_len);
    } else {
        /* GZIP or FULL RAM bFLTs not supported for now */
        kprintf("bFLT: Only GOTPIC bFLT binaries are supported\r\n");
        goto error;
    }


    /*
     * We just load the allocations into some temporary memory to
     * help simplify all this mumbo jumbo
     *
     * We've got two different sections of relocation entries.
     * The first is the GOT which resides at the beginning of the data segment
     * and is terminated with a -1.  This one can be relocated in place.
     * The second is the extra relocation entries tacked after the image's
     * data segment. These require a little more processing as the entry is
     * really an offset into the image which contains an offset into the
     * image.
     */

    /* init relocations */
    if (flags & FLAT_FLAG_GOTPIC) {
        if (process_got_relocs(&hdr, address_zero, data_dest)) // .data section is beginning of GOT
            goto error;
        *got_loc = (unsigned long)data_dest;
    }

    /*
     * Now run through the relocation entries.
     */
    process_relocs(&hdr, (unsigned long *)address_zero, (unsigned long)data_dest, (unsigned long *)relocs_src, relocs);

    return 0;

error:
    if (mem) kfree(mem);
    *reloc_text  = NULL;
    *reloc_data  = NULL;
    *reloc_bss   = NULL;
    *entry_point = NULL;
    kprintf("bFLT: Caught error - exiting\r\n");
    return -1;
}

