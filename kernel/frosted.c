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

#define IDLE() while(1){do{}while(0);}

/* The following needs to be defined by
 * the application code
 */
void (*init)(void *arg) = (void (*)(void*))(FLASH_ORIGIN + APPS_ORIGIN);
uint8_t * flt_file = (void (*)(void*))(FLASH_ORIGIN + APPS_ORIGIN);
void (*_start)(void *arg) = (void (*)(void*))(FLASH_ORIGIN + APPS_ORIGIN + 0x70);

static int (*_klog_write)(int, const void *, unsigned int) = NULL;
    

void klog_set_write(int (*wr)(int, const void *, unsigned int))
{
    _klog_write = wr;
}

int klog_write(int file, char *ptr, int len)
{
    if (_klog_write) {
        _klog_write(file, ptr, len);
    }
    return len;
}

void hard_fault_handler(void)
{
    /*
    volatile uint32_t hfsr = GET_REG(SYSREG_HFSR);
    volatile uint32_t bfsr = GET_REG(SYSREG_BFSR);
    volatile uint32_t bfar = GET_REG(SYSREG_BFAR);
    volatile uint32_t afsr = GET_REG(SYSREG_AFSR);
    */
    while(1);
}

void mem_manage_handler(void)
{
    while(1);
}

void bus_fault_handler(void)
{
    while(1);
}

void usage_fault_handler(void)
{
    while(1);
}

void machine_init(struct fnode * dev);

static void hw_init(void)
{
    machine_init(fno_search("/dev"));
    SysTick_Config(CONFIG_SYS_CLOCK / 1000);
}

void frosted_init(void)
{
    extern void * _k__syscall__;
    volatile void * vector = &_k__syscall__;
    (void)vector;

    vfs_init();
    devnull_init(fno_search("/dev"));

    /* Set up system */

    /* ktimers must be enabled before systick */
    ktimer_init();

    hw_init();
            
    syscalls_init();

    memfs_init();
    xipfs_init();
    sysfs_init();

    vfs_mount(NULL, "/mem", "memfs", 0, NULL);
    vfs_mount(NULL, "/bin", "xipfs", 0, NULL);
    vfs_mount(NULL, "/sys", "sysfs", 0, NULL);

    kernel_task_init();

#ifdef UNIX    
    socket_un_init();
#endif

    frosted_scheduler_on();
}

static void tasklet_test(void *arg)
{
    klog(LOG_INFO, "Tasklet executed\n");
}

static void ktimer_test(uint32_t time, void *arg)
{
    tasklet_add(tasklet_test, NULL);
}




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

static uint8_t * calc_reloc(uint8_t * base, uint32_t offset)
{
    /* the library id is in top byte of offset */
    int id = (offset >> 24) & 0x000000FFu;
    if (id)
    {
        klog(LOG_ERR, "No shared library support\n");
        return RELOC_FAILED;
    }
    return (uint8_t*)(base + (offset & 0x00FFFFFFu));
}

int process_GOT_relocs(uint8_t * base, unsigned long * got_start)
{
    unsigned long * rp = got_start;
    for (rp; *rp != 0xffffffff; rp++) {
        if (*rp) {
            unsigned long addr = calc_reloc(base, *rp);
            if (addr == RELOC_FAILED) {
                //errno = -ENOEXEC;
                return -1;
            }
            *rp = addr;
        }
    }
    return 0;
}

void process_relocs(int rev, uint8_t *relocs_start)
{
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
	if (rev > OLD_FLAT_VERSION) {
		//unsigned long persistent = 0;
		//for (i=0; i < relocs; i++) {
		//	unsigned long addr, relval;

		//	/* Get the address of the pointer to be
		//	   relocated (of course, the address has to be
		//	   relocated first).  */
		//	relval = long_be(reloc[i]);
		//	if (flat_set_persistent (relval, &persistent))
		//		continue;
		//	addr = flat_get_relocate_addr(relval);
		//	rp = (unsigned long *) calc_reloc(addr, libinfo, id, 1);
		//	if (rp == (unsigned long *)RELOC_FAILED) {
		//		ret = -ENOEXEC;
		//		goto err;
		//	}

		//	/* Get the pointer's value.  */
		//	addr = flat_get_addr_from_rp(rp, relval, flags,
		//					&persistent);
		//	if (addr != 0) {
		//		/*
		//		 * Do the relocation.  PIC relocs in the data section are
		//		 * already in target order
		//		 */
		//		if ((flags & FLAT_FLAG_GOTPIC) == 0)
		//			addr = long_be(addr);
		//		addr = calc_reloc(addr, libinfo, id, 0);
		//		if (addr == RELOC_FAILED) {
		//			ret = -ENOEXEC;
		//			goto err;
		//		}

		//		/* Write back the relocated pointer.  */
		//		flat_put_addr_at_rp(rp, addr, relval);
		//	}
		//}
	} else {
        /* no support for OLD relocs for now */
		//for (i=0; i < relocs; i++)
		//	old_reloc(long_be(reloc[i]));
	}
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

int bflt_load(uint8_t* from, void **mem_ptr, size_t *mem_size, int (**entry_address_ptr)(int,char*[]), size_t *stack_size) {
    struct flat_hdr hdr;
    void * mem = NULL;
	uint32_t text_len, data_len, bss_len, stack_len, flags, alloc_len;
	uint32_t full_data;
    uint8_t *data_start, *data_dest, *relocs_start;
    int relocs;
    int rev;

    klog(LOG_INFO, "Begin loading");

    if (!from) {
        goto error;
        /// ("Recieved bad file pointer");
    }

    load_header(&hdr, (struct flat_hdr *)from);

    if (check_header(&hdr) != 0) {
        klog(LOG_ERR, "Bad FLT header\n");
        goto error;    
    }

    /* Calculate all the sizes */
	text_len  = long_be(hdr.data_start);
	data_len  = long_be(hdr.data_end) - long_be(hdr.data_start);
	bss_len   = long_be(hdr.bss_end) - long_be(hdr.data_end);
	stack_len = long_be(hdr.stack_size);
	relocs    = long_be(hdr.reloc_count);
	flags     = long_be(hdr.flags);
	rev       = long_be(hdr.rev);
	full_data = data_len + relocs * sizeof(unsigned long);
    data_start = from + long_be(hdr.data_start);
    /* start of text */
    *entry_address_ptr = from + long_be(hdr.entry);
	*stack_size = stack_len;

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
        data_dest = kalloc(alloc_len);
        if (!(data_dest))
        {
            klog(LOG_ERR, "Could not allocate enough memory for process");
            goto error;
        }
        *mem_ptr = data_dest;

        /* copy segments .data and .bss */
        memcpy(data_dest, data_start, data_len);    /* init .data */
        memset(data_dest + data_len, 0, bss_len);   /* zero .bss  */

		relocs_start = (unsigned long *) (data_dest+(long_be(hdr.reloc_start)));
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
        process_GOT_relocs(from, data_dest);
	}

	/*
	 * Now run through the relocation entries.
     */
    process_relocs(rev, relocs_start);

    klog(LOG_INFO, "Successfully loaded bFLT executable to memory");
    return 0;

error:
    if (mem) kfree(mem);
    *mem_ptr = NULL;
    *entry_address_ptr = NULL;
    *mem_size = 0;
    klog(LOG_ERR, "Caught error - exiting");
}


void frosted_kernel(void)
{
    void *bin_mem;
    int (*entry_point)(int, char*[]);
    size_t bin_size;
    unsigned int stack_size;

    /* Create "init" task */
    klog(LOG_INFO, "Loading BFLT executable\n");

    bflt_load(flt_file, &bin_mem, &bin_size, &entry_point, &stack_size);

    klog(LOG_INFO, "Starting Init task\n");
    if (task_create(entry_point, (void *)0, 2) < 0)
        IDLE();

    ktimer_add(1000, ktimer_test, NULL);

    while(1) {
        check_tasklets();
    }
}

/* OS entry point */
void main(void) 
{
    frosted_init();
    frosted_kernel(); /* never returns */
}

