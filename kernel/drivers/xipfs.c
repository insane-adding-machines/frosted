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

static int xipfs_exec(struct fnode *fno, void *arg)
{
    int pid;
    struct xipfs_fnode *xip = fno->priv;
    pid = task_create(xip->init, arg, 1);
    if (pid < 0)
        return -1;
    return pid;
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


void xipfs_init(void)
{
    mod_xipfs.family = FAMILY_FILE;
    mod_xipfs.ops.read = xipfs_read; 
    mod_xipfs.ops.poll = xipfs_poll;
    mod_xipfs.ops.write = xipfs_write;
    mod_xipfs.ops.seek = xipfs_seek;
    mod_xipfs.ops.creat = xipfs_creat;
    mod_xipfs.ops.unlink = xipfs_unlink;
    mod_xipfs.ops.close = xipfs_close;
    xipfs = fno_mkdir(&mod_xipfs, "bin", NULL);
    //xip_add("sleep", sleep_task);
    register_module(&mod_xipfs);
}



