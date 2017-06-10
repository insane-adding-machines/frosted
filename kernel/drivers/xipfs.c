/*
 *      This file is part of frosted.
 *
 *      frosted is free software: you can redistribute it and/or modify
 *      it under the terms of the GNU General Public License version 2, as
 *      published by the Free Software Foundation.
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
 *      Authors: Daniele Lacamera <root@danielinux.net>
 *
 */


/*** XIPfs == eXecution In Place Filesystem ***/
/*
 * Applies to any blob. See frosted-headers/include/sys/fs/xipfs.h for the
 * physical structure on disk.
 *
 */
 
#include "frosted.h"
#include <string.h>
#include "bflt.h"
#include "kprintf.h"
#include "sys/fs/xipfs.h"
#include "vfs.h"
#include "locks.h"

#define GDB_PATH "frosted-userland/gdb/"

static struct fnode *xipfs;
static struct module mod_xipfs;
static uint8_t *xipfs_base;
static uint8_t *xipfs_end;

struct xipfs_fnode {
    struct fnode *fnode;
    void (*init)(void *);
    uint8_t *temp;
    uint32_t temp_size;
    struct flash_commit_op { 
        uint8_t *buf; 
        int len; 
        uint8_t *ptr;
        sem_t xfer_done;
    } commit;
};




#define SECTOR_SIZE (512)
 
/* 
 * - Write once, read always.
 * - Read operations always possible.
 * - Files stored are write-protected.
 * - Creation of new files allowed:
 *     * Temporarly stored in memory
 *     * Committed to the block device on close()
 *     * Once committed, it is write protected.
 */
static int xipfs_open(const char *path, int flags)
{
    int ret;
    struct fnode *f;
    if ((flags & O_WRONLY) == 0) {
        /* Read mode. */
        f = fno_search(path);
        if (f) {
            return task_filedesc_add(f);
        } else {
            return -ENOENT;
        }
    } else {
        /* Write-once mode. */
        if (fno_search(path)) {
            return -EEXIST;
        }
    }
    f = fno_create_file(path);
    return task_filedesc_add(f);
}

static int xipfs_creat(struct fnode *f)
{
    struct xipfs_fnode *xip;
    if (!f)
        return -EBADF;
    xip = kalloc(sizeof(struct xipfs_fnode));
    if (!xip)
        return -ENOMEM;
    xip->fnode = f;
    xip->fnode->priv = xip;
    /* Make executable */
    xip->fnode->flags |= FL_EXEC;
    xip->init = NULL;
    xip->temp = NULL;
    return 0;
}

static int xipfs_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct xipfs_fnode *xfno;
    if (len <= 0)
        return len;

    xfno = FNO_MOD_PRIV(fno, &mod_xipfs);
    if (!xfno)
        return -1;

    if (xfno->init == NULL) {
        return -EBUSY;
    }

    if (fno->size <= (fno->off))
        return -1;

    if (len > (fno->size - fno->off))
        len = fno->size - fno->off;

    memcpy(buf, ((char *)xfno->init) + fno->off, len);
    fno->off += len;
    return len;
}

static int xipfs_block_read(struct fnode *fno, void *buf, uint32_t sector, int offset, int count)
{
    fno->off = sector * SECTOR_SIZE + offset;
    if (fno->off > fno->size) {
        fno->off = 0;
        return -1;
    }
    if (xipfs_read(fno, buf, count) == count)
        return 0;
    return -1;
}



static int xipfs_write(struct fnode *fno, const void *buf, unsigned int len)
{
    struct xipfs_fnode *xip = (struct xipfs_fnode *)fno->priv;
    if (len <= 0)
        return len;

    if (!xip)
        return -1;

    if (xip->init) {
        return -EPERM; /* This file has already been committed. */
    }

    if (fno->size < (fno->off + len)) {
        xip->temp = krealloc(xip->temp, fno->off + len);
    }
    if (!xip->temp)
        return -1; /* OOM: file is corrupted forever. */
    memcpy(xip->temp + fno->off, buf, len);
    fno->off += len;
    if (fno->size < fno->off)
        fno->size = fno->off;
    return len;
}

static int xipfs_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    return 1;
}

static int xipfs_seek(struct fnode *fno, int off, int whence)
{
    struct xipfs_fnode *xip = (struct xipfs_fnode *)fno->priv;
    int new_off;
    if (!xip)
        return -1;
    if (xip->init) {
        return -1;
    }
    switch(whence) {
        case SEEK_CUR:
            new_off = fno->off + off;
            break;
        case SEEK_SET:
            new_off = off;
            break;
        case SEEK_END:
            new_off = fno->size + off;
            break;
        default:
            return -1;
    }

    if (new_off < 0)
        new_off = 0;

    if (new_off > fno->size) {
        xip->temp = krealloc(xip->temp, new_off);
        memset(xip->temp + fno->size, 0, new_off - fno->size);
        fno->size = new_off;
    }
    fno->off = new_off;
    return fno->off;
}

#ifdef STM32F4
#include <unicore-mx/stm32/flash.h>
struct flash_sector { int id; uint32_t base; uint32_t size; };

const struct flash_sector Flash[12] = {
    {0, 0x08000000, 16 * 1024  },
    {1, 0x08004000, 16 * 1024  },
    {2, 0x08008000, 16 * 1024  },
    {3, 0x0800C000, 16 * 1024  },
    {4, 0x08010000, 64 * 1024  },
    {5, 0x08020000, 128 * 1024 },
    {6, 0x08040000, 128 * 1024 },
    {7, 0x08060000, 128 * 1024 },
    {8, 0x08080000, 128 * 1024 },
    {9, 0x080A0000, 128 * 1024 },
    {10, 0x080C0000, 128 * 1024 },
    {11, 0x080E0000, 128 * 1024 },
};

#define SWAP_SECTOR (11)



static const struct flash_sector *flash_sector_find(uint8_t *addr)
{
    int i;
    for (i = 0; i < 12; i++)
       if ((Flash[i].base <= (uint32_t)addr) && ((Flash[i].base + Flash[i].size) > (uint32_t)addr))
           return &Flash[i];
    return NULL;
}

void flash_wait_for_last_operation(void) {
	while ((FLASH_SR & FLASH_SR_BSY) == FLASH_SR_BSY)
        kthread_yield();
}

static void flash_commit(void *arg)
{
    struct xipfs_fnode *xip = (struct xipfs_fnode *)arg;
    const struct xipfs_fat *stored_fat = (const struct xipfs_fat *)xipfs_base;
    const struct flash_sector *fat_sector = flash_sector_find(xipfs_base);
    struct xipfs_fat fat = {};
    struct xipfs_fhdr f = {};
    int wr = 0;

    if (!stored_fat)
        goto xfer_done;

    if (!xip || !xip->commit.buf)
        goto xfer_done;
   
    flash_unlock();

    while(wr < xip->commit.len) {
        const struct flash_sector *cur = flash_sector_find(xip->commit.ptr + wr);
        uint32_t off = ((uint32_t)xip->commit.ptr + wr) - cur->base;
        int w = xip->commit.len;
        if (w > cur->size - off)
            w = cur->size - off;

        /* Make a swap copy on sector SWAP_SECTOR */
        flash_erase_sector(SWAP_SECTOR, 0);
        flash_program(Flash[SWAP_SECTOR].base, (uint8_t *)cur->base, cur->size);
        
        /* Erase current sector */
        flash_erase_sector(cur->id, 0);

        /* Restore pre-buffer part */
        if (off > 0)
            flash_program(cur->base, (uint8_t *)Flash[SWAP_SECTOR].base, off);
        /* Actual buffer write */
        flash_program(cur->base + off, xip->commit.buf, w);
        /* Restore post-buffer part */
        if ((off + w) < cur->size)
            flash_program(cur->base + off + w, (uint8_t*)(Flash[SWAP_SECTOR].base + off + w), cur->size - off - w);
        wr += w;
    }
    
    
    /* Update Fat */
    memcpy(&fat, stored_fat, sizeof(struct xipfs_fat));
    flash_erase_sector(SWAP_SECTOR, 0);
    flash_program(Flash[SWAP_SECTOR].base, (uint8_t *)fat_sector->base, fat_sector->size);
    flash_erase_sector(fat_sector->id, 0);
    fat.fs_files++;
    flash_program(fat_sector->base, (uint8_t *)&fat, sizeof(struct xipfs_fat));
    flash_program(fat_sector->base + sizeof(struct xipfs_fat), (uint8_t *)Flash[SWAP_SECTOR].base + sizeof(struct xipfs_fat), fat_sector->size - sizeof(struct xipfs_fat));
    flash_lock();
    xip->init = (void *)xip->commit.ptr;
    xipfs_end = xip->commit.ptr + xip->fnode->size;
    kfree(xip->temp);
    xip->temp = NULL;
    sem_destroy(&xip->commit.xfer_done);
    xip->commit.buf = NULL;

xfer_done:
    sem_post(&xip->commit.xfer_done);
}
#endif

static int xipfs_close(struct fnode *fno)
{
    struct xipfs_fnode *xip = (struct xipfs_fnode *)fno->priv;
    if (!xip)
        return -1;

    if (xip->init)
        return 0;


    xip->commit.buf = xip->temp;
    xip->commit.len = fno->size;
    xip->commit.ptr = xipfs_end;
    if (sem_init(&xip->commit.xfer_done, 0) != 0)
        return -ENOMEM;

#   ifdef STM32F4 
    kthread_create(flash_commit, xip);

    if (sem_trywait(&xip->commit.xfer_done)) {
        task_suspend();
        return SYS_CALL_AGAIN;
    }
#endif
    return 0;
}

static void *xipfs_exe(struct fnode *fno, void *arg)
{
    struct xipfs_fnode *xip = (struct xipfs_fnode *)fno->priv;
    void *reloc_text, *reloc_data, *reloc_bss;
    size_t stack_size;
    void *init = NULL;
    struct vfs_info *vfsi = NULL;


    if (!xip)
        return NULL;
    if (xip->init == NULL) {
        return NULL;
    }

    vfsi = f_calloc(MEM_KERNEL, 1u, sizeof(struct vfs_info));
    if (!vfsi)
        return NULL;

    /* note: xip->init is bFLT load address! */
    if (bflt_load((uint8_t*)xip->init, &reloc_text, &reloc_data, &reloc_bss, &init, &stack_size, (uint32_t *)&vfsi->pic, &vfsi->text_size, &vfsi->data_size))
    {
        kprintf("xipfs: bFLT loading failed.\n");
        return NULL;
    }

    kprintf("xipfs: GDB: add-symbol-file %s%s.gdb 0x%p -s .data 0x%p -s .bss 0x%p\n", GDB_PATH, fno->fname, reloc_text, reloc_data, reloc_bss);

    vfsi->type = VFS_TYPE_BFLT;
    vfsi->allocated = reloc_data;
    vfsi->init = init;

    return (void*)vfsi;
}

static int xipfs_unlink(struct fnode *fno)
{
    return -1; /* Cannot unlink */
}

static int xip_add(const char *name, const void (*init), uint32_t size)
{
    struct xipfs_fnode *xip = kalloc(sizeof(struct xipfs_fnode));
    if (!xip)
        return -1;
    xip->fnode = fno_create(&mod_xipfs, name, fno_search("/bin"));
    if (!xip->fnode) {
        kfree(xip);
        return -1;
    }
    xip->fnode->priv = xip;

    /* Make executable */
    xip->fnode->flags |= FL_EXEC;
    xip->fnode->size = size;
    xip->init = init;
    return 0;
}

static int xipfs_parse_blob(const uint8_t *blob)
{
    const struct xipfs_fat *fat = (const struct xipfs_fat *)blob;
    const struct xipfs_fhdr *f;
    int i, offset;
    if (!fat || fat->fs_magic != XIPFS_MAGIC)
        return -1;

    offset = sizeof(struct xipfs_fat);
    for (i = 0; i < fat->fs_files; i++) {
        f = (const struct xipfs_fhdr *) (blob + offset);
        if (f->magic != XIPFS_MAGIC)
            return -1;
        xip_add(f->name, f->payload, f->len);
        offset += f->len + sizeof(struct xipfs_fhdr);
    }
    xipfs_end = xipfs_base + offset;
    return 0;
}

static int xipfs_mount(char *source, char *tgt, uint32_t flags, void *arg)
{
    struct fnode *tgt_dir = NULL;
    /* Source must NOT be NULL */
    if (!source)
        return -1;

    /* Target must be a valid dir */
    if (!tgt)
        return -1;

    tgt_dir = fno_search(tgt);

    if (!tgt_dir || ((tgt_dir->flags & FL_DIR) == 0)) {
        /* Not a valid mountpoint. */
        return -1;
    }

    tgt_dir->owner = &mod_xipfs;
    xipfs_base = (uint8_t *)source;

    if (xipfs_parse_blob((uint8_t *)source) < 0)
        return -1;

    return 0;
}



void xipfs_init(void)
{
    mod_xipfs.family = FAMILY_FILE;
    mod_xipfs.ops.open = xipfs_open;
    mod_xipfs.mount = xipfs_mount;
    strcpy(mod_xipfs.name,"xipfs");
    mod_xipfs.ops.read = xipfs_read;
    mod_xipfs.ops.poll = xipfs_poll;
    mod_xipfs.ops.write = xipfs_write;
    mod_xipfs.ops.seek = xipfs_seek;
    mod_xipfs.ops.creat = xipfs_creat;
    mod_xipfs.ops.unlink = xipfs_unlink;
    mod_xipfs.ops.close = xipfs_close;
    mod_xipfs.ops.exe = xipfs_exe;

    mod_xipfs.ops.block_read = xipfs_block_read;
    register_module(&mod_xipfs);
}
