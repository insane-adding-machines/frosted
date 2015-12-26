#include "frosted.h"
#include <string.h>
#include "xipfs.h"

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

static void *xipfs_exe(struct fnode *fno, void *arg)
{
    int pid;
    struct xipfs_fnode *xip = fno->priv;
    if (!xip)
        return NULL;
    return xip->init;
}

static int xipfs_unlink(struct fnode *fno)
{
    return -1; /* Cannot unlink */
}

static int xip_add(const char *name, void (*init))
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
    return 0;
}

static int xipfs_parse_blob(uint8_t *blob)
{
    struct xipfs_fat *fat = (struct xipfs_fat *)blob;
    struct xipfs_fhdr *f;
    int i, offset;
    if (fat->fs_magic != XIPFS_MAGIC)
        return -1;

    offset = sizeof(struct xipfs_fhdr);
    for (i = 0; i < fat->fs_files; i++) {
        f = (struct xipfs_fhdr *) (blob + offset);
        if (f->magic != XIPFS_MAGIC)
            return -1;
        f->name[55] = (char) 0;
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

    if (tgt_dir->children) {
        /* Only allowed to mount on empty directory */
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



