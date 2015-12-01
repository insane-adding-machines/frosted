#include "frosted.h"
#include <string.h>

static struct fnode *xipfs;
static struct module mod_xipfs;

struct xipfs_fnode {
    struct fnode *fnode;
    void (*init)(void *);
    uint16_t pid;
};

static int xipfs_read(int fd, void *buf, unsigned int len)
{
    return -1;
}

static int xipfs_write(int fd, const void *buf, unsigned int len)
{
    return -1; /* Cannot write! */
}

static int xipfs_poll(int fd, uint16_t events, uint16_t *revents)
{
    return -1;
}

static int xipfs_seek(int fd, int off, int whence)
{
    return -1;
}

static int xipfs_close(int fd)
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

int xip_add(const char *name, void (*init))
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

static int xipfs_mount(char *source, char *tgt, uint32_t flags, char *arg)
{
    struct fnode *tgt_dir = NULL;
    /* Source must be NULL */
    if (source)
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



