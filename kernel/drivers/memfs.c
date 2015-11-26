#include "frosted.h"
#include <string.h>

static struct fnode *memfs;
static struct module mod_memfs;


struct memfs_fnode {
    struct fnode *fnode;
    uint8_t *content;
};

static int memfs_check_fd(int fd, struct fnode **fno)
{
    *fno = task_filedesc_get(fd);
    
    if (!fno)
        return -1;

    if (fd < 0)
        return -1;
    if ((*fno)->owner != &mod_memfs)
        return -1;
    if ((*fno)->priv == NULL)
        return -1;

    return 0;
}

static int memfs_read(int fd, void *buf, unsigned int len)
{
    struct fnode *fno;
    struct memfs_fnode *mfno;
    if (len <= 0)
        return len;

    if (memfs_check_fd(fd, &fno))
        return -1;

    mfno = fno->priv;

    if (fno->size <= (fno->off))
        return -1;

    if (len > (fno->size - fno->off))
        len = fno->size - fno->off;

    memcpy(buf, mfno->content + fno->off, len);
    fno->off += len;
    return len;
}


static int memfs_write(int fd, const void *buf, unsigned int len)
{
    struct fnode *fno;
    struct memfs_fnode *mfno;
    if (len <= 0)
        return len;

    if (memfs_check_fd(fd, &fno))
        return -1;

    mfno = fno->priv;

    if (fno->size < (fno->off + len)) {
        mfno->content = krealloc(mfno->content, fno->off + len);
    }
    if (!mfno->content)
        return -1;
    memcpy(mfno->content + fno->off, buf, len);
    fno->off += len;
    if (fno->size < fno->off)
        fno->size = fno->off;
    return len;
}

static int memfs_poll(int fd, uint16_t events, uint16_t *revents)
{
    *revents = events;
    return 1;
}

static int memfs_seek(int fd, int off, int whence)
{
    struct fnode *fno;
    struct memfs_fnode *mfno;
    int new_off;
    if (memfs_check_fd(fd, &fno))
        return -1;

    mfno = fno->priv;
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
        mfno->content = krealloc(mfno->content, new_off);
        memset(mfno->content + fno->size, 0, new_off - fno->size);
        fno->size = new_off;
    }
    fno->off = new_off;
    return 0;
}

static int memfs_close(int fd)
{
    struct fnode *fno;
    struct memfs_fnode *mfno;
    if (memfs_check_fd(fd, &fno))
        return -1;
    mfno = fno->priv;
    fno->off = 0;
    return 0;
}

static int memfs_creat(struct fnode *fno)
{
    struct memfs_fnode *mfs = kalloc(sizeof(struct memfs_fnode));
    if (mfs) {
        mfs->fnode = fno;
        fno->priv = mfs;
        return 0;
    }
    return -1;
    
}

static int memfs_unlink(struct fnode *fno)
{
    struct memfs_fnode *mfno;
    if (!fno)
        return -1;
    mfno = fno->priv;
    if (mfno && mfno->content)
        kfree(mfno->content);
    kfree(mfno);
    return 0;
}

void memfs_init(void)
{
    mod_memfs.family = FAMILY_FILE;
    strcpy(mod_memfs.name,"memfs");
    mod_memfs.ops.read = memfs_read; 
    mod_memfs.ops.poll = memfs_poll;
    mod_memfs.ops.write = memfs_write;
    mod_memfs.ops.seek = memfs_seek;
    mod_memfs.ops.creat = memfs_creat;
    mod_memfs.ops.unlink = memfs_unlink;
    mod_memfs.ops.close = memfs_close;

    memfs = fno_mkdir(&mod_memfs, "mem", NULL);
    register_module(&mod_memfs);
}



