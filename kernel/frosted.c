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


int bflt_load(uint8_t* from, void **mem_ptr, size_t *mem_size, int (**entry_address_ptr)(int,char*[])) {
    struct flat_hdr hdr;
    void * mem = NULL;
	uint32_t text_len, data_len, bss_len, stack_len, flags, extra;
	uint32_t full_data;
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
    /* start of text */
    *mem_ptr = from + long_be(hdr.entry);

	/*
	 * calculate the extra space we need to map in
	 */
	extra = bss_len + stack_len;
    if (relocs * sizeof(unsigned long) > extra)
        extra = relocs * sizeof(unsigned long);

    //printf("Extra size needed: %d\n", extra);
    size_t binary_size = hdr.bss_end - hdr.entry;

	/*
	 * there are a couple of cases here,  the separate code/data
	 * case,  and then the fully copied to RAM case which lumps
	 * it all together.
	 */
#if 0
	if ((flags & (FLAT_FLAG_RAM|FLAT_FLAG_GZIP)) == 0) {
		/*
		 * this should give us a ROM ptr,  but if it doesn't we don't
		 * really care
		 */
		//DBG_FLT("BINFMT_FLAT: ROM mapping of file (we hope)\n");

		textpos = vm_mmap(bprm->file, 0, text_len, PROT_READ|PROT_EXEC,
				  MAP_PRIVATE|MAP_EXECUTABLE, 0);
		if (!textpos || IS_ERR_VALUE(textpos)) {
			if (!textpos)
				textpos = (unsigned long) -ENOMEM;
			printk("Unable to mmap process text, errno %d\n", (int)-textpos);
			ret = textpos;
			goto err;
		}

		len = data_len + extra + MAX_SHARED_LIBS * sizeof(unsigned long);
		len = PAGE_ALIGN(len);
		realdatastart = vm_mmap(0, 0, len,
			PROT_READ|PROT_WRITE|PROT_EXEC, MAP_PRIVATE, 0);

		if (realdatastart == 0 || IS_ERR_VALUE(realdatastart)) {
			if (!realdatastart)
				realdatastart = (unsigned long) -ENOMEM;
			printk("Unable to allocate RAM for process data, errno %d\n",
					(int)-realdatastart);
			vm_munmap(textpos, text_len);
			ret = realdatastart;
			goto err;
		}
		datapos = ALIGN(realdatastart +
				MAX_SHARED_LIBS * sizeof(unsigned long),
				FLAT_DATA_ALIGN);

		DBG_FLT("BINFMT_FLAT: Allocated data+bss+stack (%d bytes): %x\n",
				(int)(data_len + bss_len + stack_len), (int)datapos);

		fpos = ntohl(hdr->data_start);
#ifdef CONFIG_BINFMT_ZFLAT
		if (flags & FLAT_FLAG_GZDATA) {
			result = decompress_exec(bprm, fpos, (char *) datapos, 
						 full_data, 0);
		} else
#endif
		{
			result = read_code(bprm->file, datapos, fpos,
					full_data);
		}
		if (IS_ERR_VALUE(result)) {
			printk("Unable to read data+bss, errno %d\n", (int)-result);
			vm_munmap(textpos, text_len);
			vm_munmap(realdatastart, len);
			ret = result;
			goto err;
		}

		reloc = (unsigned long *) (datapos+(ntohl(hdr->reloc_start)-text_len));
		memp = realdatastart;
		memp_size = len;
	} else {
        /* GZIP or FULL RAM bFLTs not supported for now */
	}
#endif

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
    void *program_entry;
    int (*entry_point)(int, char*[]);
    size_t bin_size;

    /* Create "init" task */
    klog(LOG_INFO, "Loading BFLT executable\n");

    bflt_load(flt_file, &program_entry, &bin_size, &entry_point);

    klog(LOG_INFO, "Starting Init task\n");
    if (task_create(program_entry, (void *)0, 2) < 0)
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

