#include "frosted.h"
#include <string.h>

static struct fnode *memfs;
static struct module mod_memfs;


struct memfs_fnode {
    struct fnode *fnode;
    uint8_t *content;
    uint32_t size;
    int is_dir;
    uint32_t off;
};

static int memfs_check_fd(int fd, struct fnode **fno)
{
    *fno = fno_get(fd);
    
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

    if (mfno->size <= (mfno->off))
        return -1;

    if (len > (mfno->size - mfno->off))
        len = mfno->size - mfno->off;

    memcpy(buf, mfno->content + mfno->off, len);
    mfno->off += len;
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

    if (mfno->size < (mfno->off + len)) {
        mfno->content = krealloc(mfno->content, mfno->off + len);
    }
    if (!mfno->content)
        return -1;
    memcpy(mfno->content + mfno->off, buf, len);
    mfno->off += len;
    if (mfno->size < mfno->off)
        mfno->size = mfno->off;
    return len;
}

static int memfs_poll(int fd, uint16_t events)
{
    return 1;
}

static int memfs_seek(int fd, int off)
{
    struct fnode *fno;
    struct memfs_fnode *mfno;
    int new_off;
    if (memfs_check_fd(fd, &fno))
        return -1;

    mfno = fno->priv;
    new_off = mfno->off + off;

    if (new_off < 0)
        new_off = 0;

    if (new_off > mfno->size) {
        mfno->content = krealloc(mfno->content, new_off);
        memset(mfno->content + mfno->size, 0, new_off - mfno->size);
        mfno->size = new_off;
    }
    mfno->off = new_off;
    return 0;
}

static int memfs_close(int fd)
{
    struct fnode *fno;
    struct memfs_fnode *mfno;
    if (memfs_check_fd(fd, &fno))
        return -1;
    mfno = fno->priv;
    mfno->off = 0;
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
    struct memfs_fnode *mfno = fno->priv;
    if (mfno && mfno->content)
        kfree(mfno->content);
    return 0;
}

void memfs_init(void)
{
    struct fnode *test_file;
    mod_memfs.family = FAMILY_FILE;
    mod_memfs.ops.read = memfs_read; 
    mod_memfs.ops.poll = memfs_poll;
    mod_memfs.ops.write = memfs_write;
    mod_memfs.ops.seek = memfs_seek;
    mod_memfs.ops.creat = memfs_creat;
    mod_memfs.ops.unlink = memfs_unlink;
    mod_memfs.ops.close = memfs_close;

    memfs = fno_create(&mod_memfs, "mem", NULL);
    test_file = fno_create(&mod_memfs, "test", memfs);

    register_module(&mod_memfs);
}



