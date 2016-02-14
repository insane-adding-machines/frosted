#include "frosted.h"
#include "string.h"

static struct module mod_memfs;


struct memfs_fnode {
    struct fnode *fnode;
    uint8_t *content;
};

static int memfs_read(struct fnode *fno, void *buf, unsigned int len)
{
    struct memfs_fnode *mfno;
    if (len <= 0)
        return len;

    mfno = FNO_MOD_PRIV(fno, &mod_memfs);
    if (!mfno)
        return -1;

    if (fno->size <= (fno->off))
        return -1;

    if (len > (fno->size - fno->off))
        len = fno->size - fno->off;

    memcpy(buf, mfno->content + fno->off, len);
    fno->off += len;
    return len;
}


static int memfs_write(struct fnode *fno, const void *buf, unsigned int len)
{
    struct memfs_fnode *mfno;
    if (len <= 0)
        return len;

    mfno = FNO_MOD_PRIV(fno, &mod_memfs);
    if (!mfno)
        return -1;

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

static int memfs_poll(struct fnode *fno, uint16_t events, uint16_t *revents)
{
    *revents = events;
    return 1;
}

static int memfs_seek(struct fnode *fno, int off, int whence)
{
    struct memfs_fnode *mfno;
    int new_off;
    mfno = FNO_MOD_PRIV(fno, &mod_memfs);
    if (!mfno)
        return -1;
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

static int memfs_close(struct fnode *fno)
{
    struct memfs_fnode *mfno;
    mfno = FNO_MOD_PRIV(fno, &mod_memfs);
    if (!mfno)
        return -1;
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

static int memfs_mount(char *source, char *tgt, uint32_t flags, void *arg)
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

    /* TODO: Check empty dir 
    if (tgt_dir->children) {
        return -1;
    }
    */
    tgt_dir->owner = &mod_memfs;
    return 0;
}

void memfs_init(void)
{
    mod_memfs.family = FAMILY_FILE;
    strcpy(mod_memfs.name,"memfs");

    mod_memfs.mount = memfs_mount;

    mod_memfs.ops.read = memfs_read; 
    mod_memfs.ops.poll = memfs_poll;
    mod_memfs.ops.write = memfs_write;
    mod_memfs.ops.seek = memfs_seek;
    mod_memfs.ops.creat = memfs_creat;
    mod_memfs.ops.unlink = memfs_unlink;
    mod_memfs.ops.close = memfs_close;
    register_module(&mod_memfs);
}



