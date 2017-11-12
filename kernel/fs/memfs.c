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
 *      Authors:
 *
 */

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
    uint32_t off;
    if (len <= 0)
        return len;

    mfno = FNO_MOD_PRIV(fno, &mod_memfs);
    if (!mfno)
        return -ENOENT;

    off = task_fd_get_off(fno);

    if (fno->size <= off)
        return 0;

    if (len > (fno->size - off))
        len = fno->size - off;

    memcpy(buf, mfno->content + off, len);
    off+=len;
    task_fd_set_off(fno, off);
    return len;
}


static int memfs_write(struct fnode *fno, const void *buf, unsigned int len)
{
    struct memfs_fnode *mfno;
    uint32_t off;
    if (len <= 0)
        return len;

    mfno = FNO_MOD_PRIV(fno, &mod_memfs);
    if (!mfno)
        return -ENOENT;

    off = task_fd_get_off(fno);

    if (fno->size < (off+len)) {
        mfno->content = krealloc(mfno->content, off + len);
    }
    if (!mfno->content)
        return -ENOMEM;
    memcpy(mfno->content + off, buf, len);
    off += len;
    if (fno->size < off)
        fno->size = off;
    task_fd_set_off(fno, off);
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
            new_off = task_fd_get_off(fno) + off;
            break;
        case SEEK_SET:
            new_off = off;
            break;
        case SEEK_END:
            new_off = fno->size + off;
            break;
        default:
            return -EINVAL;
    }

    if (new_off < 0)
        new_off = 0;

    if (new_off > fno->size) {
        mfno->content = krealloc(mfno->content, new_off);
        memset(mfno->content + fno->size, 0, new_off - fno->size);
        fno->size = new_off;
    }
    task_fd_set_off(fno, new_off);
    return new_off;
}

static int memfs_close(struct fnode *fno)
{
    struct memfs_fnode *mfno;
    mfno = FNO_MOD_PRIV(fno, &mod_memfs);
    if (!mfno)
        return -1;
    return 0;
}

static int memfs_creat(struct fnode *fno)
{
    struct memfs_fnode *mfs = kalloc(sizeof(struct memfs_fnode));
    if (mfs) {
        mfs->fnode = fno;
        mfs->content = NULL;
        fno->priv = mfs;
        return 0;
    }
    return -1;

}

static int memfs_unlink(struct fnode *fno)
{
    struct memfs_fnode *mfno;
    if (!fno)
        return -ENOENT;
    mfno = fno->priv;
    if (mfno && mfno->content)
        kfree(mfno->content);
    kfree(mfno);
    return 0;
}

static int memfs_truncate(struct fnode *fno, unsigned int newsize)
{
    struct memfs_fnode *mfno;
    if (!fno)
        return -ENOENT;
    mfno = fno->priv;
    if (mfno) {
        if (fno->size <= newsize) {
            /* Nothing to do here. */
            return 0;
        }
        if (newsize == 0) {
            fno->size = 0;
            kfree(mfno->content);
            mfno->content = NULL;
        } else {
            mfno->content = krealloc(mfno->content, newsize);
            fno->size = newsize;
        }
        return 0;
    }
    return -EFAULT;
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
    mod_memfs.ops.truncate = memfs_truncate;
    register_module(&mod_memfs);
}
