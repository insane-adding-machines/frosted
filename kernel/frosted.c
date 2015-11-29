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
void (*_start)(void *arg) = (void (*)(void*))(FLASH_ORIGIN + APPS_ORIGIN + 0x70);
uint8_t * flt_file = (void (*)(void*))(FLASH_ORIGIN + APPS_ORIGIN);

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

static inline void endian_fix32(uint32_t * tofix, size_t count) {
    /* bFLT is big endian */

    /* endianness test */
    union {
        uint16_t int_val;
        uint8_t  char_val[2];
    } endian;
    endian.int_val = 1;

    if (endian.char_val[0]) {
        /* we are little endian, do a byteswap */
        size_t i;
        for (i=0; i<count; i++) {
            tofix[i] = long_be(tofix[i]);
        }
    }

}

static void load_header(struct flat_hdr * from_hdr, struct flat_hdr * to_hdr) {
    memcpy((uint8_t*)to_hdr, (uint8_t*)from_hdr, sizeof(struct flat_hdr));
    endian_fix32(&to_hdr->rev, ( &to_hdr->build_date - &to_hdr->rev ) + 1);
}

int check_header(struct flat_hdr * header) {
    if (memcmp(header->magic, "bFLT", 4) != 0) {
        klog(LOG_INFO,"Magic number does not match");
        return -1;
    }
    if (header->rev != FLAT_VERSION){
        klog(LOG_INFO,"Version number does not match");
        return -1;
    }

    /* check for unsupported flags */
    if (header->flags & (FLAT_FLAG_GZIP | FLAT_FLAG_GZDATA)) {
        klog(LOG_INFO,"Unsupported flags detected - GZip'd data is not supported");
        return -1;
    }
    return 0;
}


int bflt_fload(uint8_t* from, void **mem_ptr, size_t *mem_size, int (**entry_address_ptr)(int,char*[])) {
    void * mem = NULL;
    struct flat_hdr header;

    klog(LOG_INFO, "Begin loading");

    if (!from) {
        goto error;
        /// ("Recieved bad file pointer");
    }

    read_header(&header, (struct flat_hdr *)from);

    if (check_header(&header) != 0) {
        klog(LOG_ERR, "Bad FLT header\n");
        goto error;    
    }

    size_t binary_size = header.bss_end - header.entry;
    klog(LOG_INFO, "Attempting to alloc %u bytes",binary_size);
    mem = kalloc(binary_size);
    if (!mem) 
    {
        klog(LOG_ERR, "Failed to alloc binary memory");
        goto error;
    }

    //if (copy_segments(from, &header, mem, binary_size) != 0) error_goto_error("Failed to copy segments");
    //if (process_relocs(from, &header, mem) != 0) error_goto_error("Failed to relocate");

    /* only attempt to process GOT if the flags tell us a GOT exists AND
       if the Ndless startup file is not already doing so */
    //if (header.flags & FLAT_FLAG_GOTPIC && memcmp(mem, "PRG\0", 4) != 0) {
    //    if (process_got(&header, mem) != 0) error_goto_error("Failed to process got");
    //}else{
    //    klog(LOG_INFO, "No need to process got - skipping");
    //}

    *mem_ptr = mem;
    *mem_size = binary_size;

    //if (memcmp(mem, "PRG\0", 4) == 0) {
    //    klog(LOG_INFO, "Detected as ndless program packaged in a bFLT file");
    //    *entry_address_ptr = (int (*)(int,char*[]))((char*)mem + 4);
    //}else{
    //    klog(LOG_INFO, "Detected as ordinary bFLT executable");
    //    *entry_address_ptr = (int (*)(int,char*[]))mem;
    //}

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

    /* Create "init" task */
    klog(LOG_INFO, "Loading BFLT executable\n");

    bflt_fload(flt_file, &bin_mem, &bin_size, &entry_point);

    //klog(LOG_INFO, "Starting Init task\n");
    //if (task_create(init, (void *)0, 2) < 0)
    //    IDLE();

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

