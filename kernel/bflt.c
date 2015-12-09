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
#include "libopencmsis/core_cm3.h"
#include "libopencm3/cm3/systick.h"

/*************************
 * bFLT start 
 *************************/
#define RELOC_FAILED 0xff00ff01		/* Relocation incorrect somewhere */

static inline uint16_t short_be(uint16_t le)
{
    return (uint16_t)(((le & 0xFFu) << 8) | ((le >> 8u) & 0xFFu));
}

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

int check_header(struct flat_hdr * header) {
    if (memcmp(header->magic, "bFLT", 4) != 0) {
        klog(LOG_INFO,"Magic number does not match");
        return -1;
    }
    if (long_be(header->rev) != FLAT_VERSION){
        klog(LOG_INFO,"Version number does not match");
        return -1;
    }

    /* check for unsupported flags */
    if (long_be(header->flags) & (FLAT_FLAG_GZIP | FLAT_FLAG_GZDATA)) {
        klog(LOG_INFO,"Unsupported flags detected - GZip'd data is not supported");
        return -1;
    }
    return 0;
}

static unsigned long * calc_reloc(uint8_t * base, uint32_t offset)
{
    /* the library id is in top byte of offset */
    int id = (offset >> 24) & 0x000000FFu;
    if (id)
    {
        klog(LOG_ERR, "No shared library support\n");
        return RELOC_FAILED;
    }
    return (unsigned long *)(base + (offset & 0x00FFFFFFu));
}

int process_GOT_relocs(uint8_t * base, unsigned long * got_start)
{
    unsigned long * rp = got_start;
    for (rp; *rp != 0xffffffff; rp++) {
        if (*rp) {
            /* this will remap pointers starting from address 0x0, to wherever they are actually loaded in the memory map (.text reloc) */
            unsigned long addr = (unsigned long)calc_reloc(base + sizeof(struct flat_hdr), *rp);
            if (addr == RELOC_FAILED) {
                //errno = -ENOEXEC;
                return -1;
            }
            *rp = addr;
        }
    }
    return 0;
}

/* works only for FLAT v4 */
/* reloc starts just after data_end */
int process_relocs(struct flat_hdr * hdr, unsigned long * base,  unsigned long *reloc, int relocs)
{
    int i;
    /* Relocations in the table point to an address,
     * at that address(1), there is an adress(2) that needs fixup,
     * and needs to be written at it's original address(1), after it's been fixed
     */
    unsigned long data_start = long_be(hdr->data_start);
    unsigned long data_end = long_be(hdr->data_end);
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
        if ((unsigned long)fixup_addr < data_start)
        {
            /* FAIL -- non GOTPIC */
            //relocd_addr = calc_reloc(text_base, fixme);
            return -1;
        } else if ((unsigned long)fixup_addr < data_end) {
            /* Reloc is in .data section, now make this point to the .data source (from the binary), and dereference */
            fixup_addr = ((unsigned long)base + (unsigned long)fixup_addr); // relocate the location of the address
            relocd_addr = calc_reloc(base, *fixup_addr);    // read the value at that adress, and add an offset
            *fixup_addr = relocd_addr;                      // write the relocated/offsetted value back were it was read
        }

        if (relocd_addr == (unsigned long *)RELOC_FAILED) {
            klog(LOG_ERR, "Unable to calculate relocation address");
            return;
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

int bflt_load(uint8_t* from, void **mem_ptr, size_t *mem_size, int (**entry_point)(int,char*[]), size_t *stack_size, uint32_t *got_loc) {
    struct flat_hdr hdr;
    void * mem = NULL;
	uint32_t text_len, data_len, bss_len, stack_len, flags, alloc_len, start_of_file;
	uint32_t full_data;
    uint8_t *data_src_start, *data_dest_start, *relocs_src_start, *address_zero = from;
    int relocs;
    int rev;

    klog(LOG_INFO, "Begin loading");

    if (!address_zero) {
        goto error;
        /// ("Recieved bad file pointer");
    }

    load_header(&hdr, (struct flat_hdr *)address_zero);

    if (check_header(&hdr) != 0) {
        klog(LOG_ERR, "Bad FLT header\n");
        goto error;    
    }

    /* Calculate all the sizes */
	text_len            = long_be(hdr.data_start);
	data_len            = long_be(hdr.data_end) - long_be(hdr.data_start);
	bss_len             = long_be(hdr.bss_end) - long_be(hdr.data_end);
	stack_len           = long_be(hdr.stack_size);
	relocs              = long_be(hdr.reloc_count);
	flags               = long_be(hdr.flags);
	rev                 = long_be(hdr.rev);
	full_data           = data_len + relocs * sizeof(unsigned long);
    /* Calculate source addresses */
    //text_src_start      = address_zero + sizeof(struct flat_hdr);   // UNUSED?
    data_src_start      = address_zero + long_be(hdr.data_start);
    relocs_src_start    = address_zero + long_be(hdr.reloc_start);
    *entry_point        = address_zero + (long_be(hdr.entry) & 0xFFFFFFFE); /* entrypoint - reset THUMB bit */
	*stack_size         = stack_len;

	/*
	 * calculate the extra space we need to malloc
	 */
    /* relocs are located in the .bss part of the BFLT binary, so we need whichever is biggest */
    if ((relocs * sizeof(unsigned long)) > bss_len)
        alloc_len = relocs * sizeof(unsigned long);
    else
        alloc_len = bss_len;
    alloc_len += data_len;


	/*
	 * there are a couple of cases here,  the separate code/data
	 * case,  and then the fully copied to RAM case which lumps
	 * it all together.
	 */
	if ((flags & (FLAT_FLAG_RAM|FLAT_FLAG_GZIP)) == 0) {
		/*
		 * this should give us a ROM ptr,  but if it doesn't we don't
		 * really care
		 */
		//DBG_FLT("BINFMT_FLAT: ROM mapping of file (we hope)\n");
        
        /* Allocate enough memory for .data and .bss */
        data_dest_start = f_malloc(MEM_USER, alloc_len);
        if (!(data_dest_start))
        {
            klog(LOG_ERR, "Could not allocate enough memory for process");
            goto error;
        }
        *mem_ptr = data_dest_start;

        /* copy segments .data and .bss */
        memcpy(data_dest_start, data_src_start, data_len);    /* init .data */
        memset(data_dest_start + data_len, 0, bss_len);   /* zero .bss  */
	} else {
        /* GZIP or FULL RAM bFLTs not supported for now */
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
        //printf("GOT-PIC!\n");
        process_GOT_relocs(address_zero, data_dest_start); // .data section is beginning of GOT
        *got_loc = data_dest_start;
	}

	/*
	 * Now run through the relocation entries.
     */
    process_relocs(&hdr, address_zero,  relocs_src_start, relocs);

    klog(LOG_INFO, "Successfully loaded bFLT executable to memory");
    return 0;

error:
    if (mem) kfree(mem);
    *mem_ptr = NULL;
    *entry_point = NULL;
    *mem_size = 0;
    klog(LOG_ERR, "Caught error - exiting");
}

