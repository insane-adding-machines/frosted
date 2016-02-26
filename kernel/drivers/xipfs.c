#include "frosted.h"
#include <string.h>
#include "bflt.h"
#include "kprintf.h"
#include "xipfs.h"
#include "vfs.h"

static struct fnode *xipfs;
static struct module mod_xipfs;

struct xipfs_fnode {
    struct fnode *fnode;
    void (*init)(void *);
    uint16_t pid;
};

static int xipfs_read(struct fnode *fno, void *buf, unsigned int len)
{
    return -1;
}

static int xipfs_write(struct fnode *fno, const void *buf, unsigned int len)
{
    return -1; /* Cannot write! */
}

static int xipfs_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    return -1;
}

static int xipfs_seek(struct fnode *fno, int off, int whence)
{
    return -1;
}

static int xipfs_close(struct fnode *fno)
{
    return 0;
}

static int xipfs_creat(struct fnode *fno)
{
    return -1;
    
}

static void *xipfs_exe(struct fnode *fno, void *arg, uint32_t *pic)
{
    int pid;
    struct xipfs_fnode *xip = (struct xipfs_fnode *)fno->priv;
    void *reloc_text, *reloc_data, *reloc_bss;
    size_t stack_size;
    void *init = NULL;
    struct vfs_info *vfsi = NULL;

    if (!xip)
        return NULL;

    vfsi = f_calloc(MEM_KERNEL, 1u, sizeof(struct vfs_info));
    if (!vfsi)
        return NULL;

    /* note: xip->init is bFLT load address! */
    bflt_load((uint8_t*)xip->init, &reloc_text, &reloc_data, &reloc_bss, &init, &stack_size, pic);
    kprintf("xipfs: GDB: add-symbol-file %s.gdb 0x%p -s .data 0x%p -s .bss 0x%p\r\n", fno->fname, reloc_text, reloc_data, reloc_bss);

    vfsi->type = VFS_TYPE_BFLT;
    vfsi->allocated = reloc_data;
    vfsi->init = init;

    return (void*)vfsi;
}

static int xipfs_unlink(struct fnode *fno)
{
    return -1; /* Cannot unlink */
}

static int xip_add(const char *name, const void (*init))
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
        xip_add(f->name, f->payload);
        offset += f->len + sizeof(struct xipfs_fhdr);
    }
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
    if (xipfs_parse_blob((uint8_t *)source) < 0)
        return -1;

    return 0;
}


void xipfs_init(void)
{
    mod_xipfs.family = FAMILY_FILE;
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
    register_module(&mod_xipfs);
}



